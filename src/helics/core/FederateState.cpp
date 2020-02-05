/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "FederateState.hpp"

#include "CommonCore.hpp"
#include "CoreFederateInfo.hpp"
#include "EndpointInfo.hpp"
#include "NamedInputInfo.hpp"
#include "PublicationInfo.hpp"
#include "TimeCoordinator.hpp"
#include "TimeDependencies.hpp"
#include "helics/helics-config.h"
#include "helics_definitions.hpp"
#include "queryHelpers.hpp"

#include <algorithm>
#include <chrono>
#include <mutex>
#include <sstream>
#include <thread>

#ifndef HELICS_DISABLE_ASIO
#    include "MessageTimer.hpp"
#else
namespace helics {
class MessageTimer {
};
} // namespace helics
#endif

#include "../common/fmt_format.h"
static const std::string emptyStr;
#define LOG_ERROR(message) logMessage(helics_log_level_error, emptyStr, message)
#define LOG_WARNING(message) logMessage(helics_log_level_warning, emptyStr, message)

#ifdef HELICS_ENABLE_LOGGING

#    define LOG_SUMMARY(message)                                                                   \
        do {                                                                                       \
            if (logLevel >= helics_log_level_summary) {                                            \
                logMessage(Hhelics_log_level_summary, emptyStr, message);                          \
            }                                                                                      \
        } while (false)

#    define LOG_INTERFACES(message)                                                                \
        do {                                                                                       \
            if (logLevel >= helics_log_level_interfaces) {                                         \
                logMessage(helics_log_level_interfaces, emptyStr, message);                        \
            }                                                                                      \
        } while (false)

#    ifdef HELICS_ENABLE_DEBUG_LOGGING
#        define LOG_TIMING(message)                                                                \
            do {                                                                                   \
                if (logLevel >= helics_log_level_timing) {                                         \
                    logMessage(helics_log_level_timing, emptyStr, message);                        \
                }                                                                                  \
            } while (false)

#        define LOG_DATA(message)                                                                  \
            do {                                                                                   \
                if (logLevel >= helics_log_level_data) {                                           \
                    logMessage(helics_log_level_data, emptyStr, message);                          \
                }                                                                                  \
            } while (false)
#    else
#        define LOG_TIMING(message)
#        define LOG_DATA(message)
#    endif

#    ifdef HELICS_ENABLE_TRACE_LOGGING
#        define LOG_TRACE(message)                                                                 \
            do {                                                                                   \
                if (logLevel >= helics_log_level_trace) {                                          \
                    logMessage(helics_log_level_trace, emptyStr, message);                         \
                }                                                                                  \
            } while (false)
#    else
#        define LOG_TRACE(message) ((void)0)
#    endif
#else // LOGGING_DISABLED
#    define LOG_SUMMARY(message) ((void)0)
#    define LOG_INTERFACES(message) ((void)0)
#    define LOG_TIMING(message) ((void)0)
#    define LOG_DATA(message) ((void)0)
#    define LOG_TRACE(message) ((void)0)
#endif // LOGGING_DISABLED

using namespace std::chrono_literals;

namespace helics {
FederateState::FederateState(const std::string& name_, const CoreFederateInfo& info_):
    name(name_),
    timeCoord(new TimeCoordinator([this](const ActionMessage& msg) { routeMessage(msg); })),
    global_id{global_federate_id()}
{
    for (const auto& prop : info_.timeProps) {
        setProperty(prop.first, prop.second);
    }
    for (const auto& prop : info_.intProps) {
        setProperty(prop.first, prop.second);
    }
    for (const auto& prop : info_.flagProps) {
        setOptionFlag(prop.first, prop.second);
    }
}

FederateState::~FederateState() = default;

// define the allowable state transitions for a federate
void FederateState::setState(federate_state newState)
{
    if (state == newState) {
        return;
    }
    switch (newState) {
        case HELICS_ERROR:
        case HELICS_FINISHED:
        case HELICS_CREATED:
        case HELICS_TERMINATING:
            state = newState;
            break;
        case HELICS_INITIALIZING: {
            auto reqState = HELICS_CREATED;
            state.compare_exchange_strong(reqState, newState);
            break;
        }
        case HELICS_EXECUTING: {
            auto reqState = HELICS_INITIALIZING;
            state.compare_exchange_strong(reqState, newState);
            break;
        }
        case HELICS_UNKNOWN:
        default:
            break;
    }
}

void FederateState::reset()
{
    global_id = global_federate_id();
    interfaceInformation.setGlobalId(global_federate_id());
    local_id = local_federate_id();
    state = HELICS_CREATED;
    queue.clear();
    delayQueues.clear();
    // TODO:: this probably needs to do a lot more
}
/** reset the federate to the initializing state*/
void FederateState::reInit()
{
    state = HELICS_INITIALIZING;
    queue.clear();
    delayQueues.clear();
    // TODO:: this needs to reset a bunch of stuff as well as check a few things
}
federate_state FederateState::getState() const
{
    return state;
}

int32_t FederateState::getCurrentIteration() const
{
    return timeCoord->getCurrentIteration();
}

bool FederateState::checkAndSetValue(interface_handle pub_id, const char* data, uint64_t len)
{
    if (!only_transmit_on_change) {
        return true;
    }
    std::lock_guard<FederateState> plock(*this);
    // this function could be called externally in a multi-threaded context
    auto pub = interfaceInformation.getPublication(pub_id);
    auto res = pub->CheckSetValue(data, len);
    return res;
}

std::string FederateState::generateConfig() const
{
    static const std::string truestr{"true"};
    static const std::string falsestr{"false"};

    std::stringstream s;
    s << "\"only_transmit_on_change\":" << ((only_transmit_on_change) ? truestr : falsestr);
    s << ",\n\"realtime\":" << ((realtime) ? truestr : falsestr);
    s << ",\n\"observer\":" << ((observer) ? truestr : falsestr);
    s << ",\n\"source_only\":" << ((source_only) ? truestr : falsestr);
    s << ",\n\"strict_input_type_checking\":" << ((source_only) ? truestr : falsestr);
    s << ",\n\"slow_responding\":" << ((slow_responding) ? truestr : falsestr);
    if (rt_lag > timeZero) {
        s << ",\n\"rt_lag\":" << static_cast<double>(rt_lag);
    }
    if (rt_lead > timeZero) {
        s << ",\n\"rt_lead\":" << static_cast<double>(rt_lead);
    }
    return s.str();
}

uint64_t FederateState::getQueueSize(interface_handle handle_) const
{
    auto epI = interfaceInformation.getEndpoint(handle_);
    if (epI != nullptr) {
        return epI->queueSize(time_granted);
    }
    return 0;
}

uint64_t FederateState::getQueueSize() const
{
    uint64_t cnt = 0;
    for (const auto& end_point : interfaceInformation.getEndpoints()) {
        cnt += end_point->queueSize(time_granted);
    }
    return cnt;
}

std::unique_ptr<Message> FederateState::receive(interface_handle handle_)
{
    auto epI = interfaceInformation.getEndpoint(handle_);
    if (epI != nullptr) {
        return epI->getMessage(time_granted);
    }
    return nullptr;
}

std::unique_ptr<Message> FederateState::receiveAny(interface_handle& id)
{
    Time earliest_time = Time::maxVal();
    EndpointInfo* endpointI = nullptr;
    auto elock = interfaceInformation.getEndpoints();
    // Find the end point with the earliest message time
    for (auto& end_point : elock) {
        auto t = end_point->firstMessageTime();
        if (t < earliest_time) {
            earliest_time = t;
            endpointI = end_point.get();
        }
    }
    if (endpointI == nullptr) {
        return nullptr;
    }
    // Return the message found and remove from the queue
    if (earliest_time <= time_granted) {
        auto result = endpointI->getMessage(time_granted);
        id = endpointI->id.handle;
        return result;
    }
    id = interface_handle();
    return nullptr;
}

void FederateState::routeMessage(const ActionMessage& msg)
{
    if (parent_ != nullptr) {
        parent_->addActionMessage(msg);
    } else {
        queue.push(msg);
    }
}

void FederateState::addAction(const ActionMessage& action)
{
    if (action.action() != CMD_IGNORE) {
        queue.push(action);
    }
}

void FederateState::addAction(ActionMessage&& action)
{
    if (action.action() != CMD_IGNORE) {
        queue.push(std::move(action));
    }
}

void FederateState::createInterface(
    handle_type htype,
    interface_handle handle,
    const std::string& key,
    const std::string& type,
    const std::string& units)
{
    std::lock_guard<FederateState> plock(*this);
    // this function could be called externally in a multi-threaded context
    switch (htype) {
        case handle_type::publication: {
            interfaceInformation.createPublication(handle, key, type, units);
            if (checkActionFlag(getInterfaceFlags(), required_flag)) {
                interfaceInformation.setPublicationProperty(
                    handle, defs::options::connection_required, true);
            }
            if (checkActionFlag(getInterfaceFlags(), optional_flag)) {
                interfaceInformation.setPublicationProperty(
                    handle, defs::options::connection_optional, true);
            }
        } break;
        case handle_type::input: {
            interfaceInformation.createInput(handle, key, type, units);
            if (strict_input_type_checking) {
                interfaceInformation.setInputProperty(
                    handle, defs::options::strict_type_checking, true);
            }
            if (ignore_unit_mismatch) {
                interfaceInformation.setInputProperty(
                    handle, defs::options::ignore_unit_mismatch, true);
            }
            if (checkActionFlag(getInterfaceFlags(), required_flag)) {
                interfaceInformation.setInputProperty(
                    handle, defs::options::connection_required, true);
            }
            if (checkActionFlag(getInterfaceFlags(), optional_flag)) {
                interfaceInformation.setInputProperty(
                    handle, defs::options::connection_optional, true);
            }
        } break;
        case handle_type::endpoint: {
            interfaceInformation.createEndpoint(handle, key, type);
        }
        default:
            break;
    }
}

void FederateState::closeInterface(interface_handle handle, handle_type type)
{
    switch (type) {
        case handle_type::publication: {
            auto pub = interfaceInformation.getPublication(handle);
            if (pub != nullptr) {
                ActionMessage rem(CMD_REMOVE_PUBLICATION);
                rem.setSource(pub->id);
                rem.actionTime = time_granted;
                for (auto& sub : pub->subscribers) {
                    rem.setDestination(sub);
                    routeMessage(rem);
                }
                pub->subscribers.clear();
            }
        } break;
        case handle_type::endpoint: {
            auto ept = interfaceInformation.getEndpoint(handle);
            if (ept != nullptr) {
                ept->clearQueue();
            }
        } break;
        case handle_type::input: {
            auto ipt = interfaceInformation.getInput(handle);
            if (ipt != nullptr) {
                ActionMessage rem(CMD_REMOVE_SUBSCRIBER);
                rem.setSource(ipt->id);
                rem.actionTime = time_granted;
                for (auto& pub : ipt->input_sources) {
                    rem.setDestination(pub);
                    routeMessage(rem);
                }
                ipt->input_sources.clear();
                ipt->clearFutureData();
            }
        } break;
        default:
            break;
    }
}

stx::optional<ActionMessage>
    FederateState::processPostTerminationAction(const ActionMessage& /*action*/)
{
    return stx::nullopt;
}

iteration_result FederateState::waitSetup()
{
    if (try_lock()) { // only enter this loop once per federate
        auto ret = processQueue();
        unlock();
        return static_cast<iteration_result>(ret);
    }

    std::lock_guard<FederateState> fedlock(*this);
    iteration_result ret;
    switch (getState()) {
        case HELICS_CREATED: { // we are still in the created state
            return waitSetup();
        }
        case HELICS_ERROR:
            ret = iteration_result::error;
            break;
        case HELICS_FINISHED:
            ret = iteration_result::halted;
            break;
        default:
            ret = iteration_result::next_step;
            break;
    }

    return ret;
}

iteration_result FederateState::enterInitializingMode()
{
    if (try_lock()) { // only enter this loop once per federate
        auto ret = processQueue();
        unlock();
        if (ret == message_processing_result::next_step) {
            time_granted = initialTime;
            allowed_send_time = initialTime;
        }
        return static_cast<iteration_result>(ret);
    }

    std::lock_guard<FederateState> fedlock(*this);
    iteration_result ret;
    switch (getState()) {
        case HELICS_ERROR:
            ret = iteration_result::error;
            break;
        case HELICS_FINISHED:
            ret = iteration_result::halted;
            break;
        case HELICS_CREATED: {
            return enterInitializingMode();
        }
        default: // everything >= HELICS_INITIALIZING
            ret = iteration_result::next_step;
            break;
    }
    return ret;
}

iteration_result FederateState::enterExecutingMode(iteration_request iterate)
{
    if (try_lock()) { // only enter this loop once per federate
        // timeCoord->enteringExecMode (iterate);
        ActionMessage exec(CMD_EXEC_REQUEST);
        exec.source_id = global_id.load();
        switch (iterate) {
            case iteration_request::force_iteration:
                setActionFlag(exec, iteration_requested_flag);
                setActionFlag(exec, required_flag);
                break;
            case iteration_request::iterate_if_needed:
                setActionFlag(exec, iteration_requested_flag);
                break;
            case iteration_request::no_iterations:
                break;
        }

        addAction(exec);

        auto ret = processQueue();
        if (ret == message_processing_result::next_step) {
            time_granted = timeZero;
            allowed_send_time = timeCoord->allowedSendTime();
        }
        switch (iterate) {
            case iteration_request::force_iteration:
                fillEventVectorNextIteration(time_granted);
                break;
            case iteration_request::iterate_if_needed:
                if (ret == message_processing_result::next_step) {
                    fillEventVectorUpTo(time_granted);
                } else {
                    fillEventVectorNextIteration(time_granted);
                }
                break;
            case iteration_request::no_iterations:
                fillEventVectorUpTo(time_granted);
                break;
        }

        unlock();
#ifndef HELICS_DISABLE_ASIO
        if ((realtime) && (ret == message_processing_result::next_step)) {
            if (!mTimer) {
                mTimer = std::make_shared<MessageTimer>(
                    [this](ActionMessage&& mess) { return this->addAction(std::move(mess)); });
            }
            start_clock_time = std::chrono::steady_clock::now();
        }
#endif
        return static_cast<iteration_result>(ret);
    }
    // the following code is for situation which this has been called multiple times, which really shouldn't be
    // done but it isn't really an error so we need to deal with it.
    std::lock_guard<FederateState> plock(*this);
    iteration_result ret;
    switch (getState()) {
        case HELICS_ERROR:
            ret = iteration_result::error;
            break;
        case HELICS_FINISHED:
            ret = iteration_result::halted;
            break;
        case HELICS_CREATED:
        case HELICS_INITIALIZING:
        default:
            ret = iteration_result::iterating;
            break;
        case HELICS_EXECUTING:
            ret = iteration_result::next_step;
            break;
    }
    return ret;
}

std::vector<global_handle> FederateState::getSubscribers(interface_handle handle)
{
    std::lock_guard<FederateState> fedlock(*this);
    auto pubInfo = interfaceInformation.getPublication(handle);
    if (pubInfo != nullptr) {
        return pubInfo->subscribers;
    }
    return {};
}

iteration_time FederateState::requestTime(Time nextTime, iteration_request iterate)
{
    if (try_lock()) { // only enter this loop once per federate
        Time lastTime = timeCoord->getGrantedTime();
        events.clear(); // clear the event queue
        LOG_TRACE(timeCoord->printTimeStatus());
        // timeCoord->timeRequest (nextTime, iterate, nextValueTime (), nextMessageTime ());

        ActionMessage treq(CMD_TIME_REQUEST);
        treq.source_id = global_id.load();
        treq.actionTime = nextTime;
        switch (iterate) {
            case iteration_request::force_iteration:
                setActionFlag(treq, iteration_requested_flag);
                setActionFlag(treq, required_flag);
                break;
            case iteration_request::iterate_if_needed:
                setActionFlag(treq, iteration_requested_flag);
                break;
            case iteration_request::no_iterations:
                break;
        }

        addAction(treq);
        LOG_TRACE(timeCoord->printTimeStatus());
// timeCoord->timeRequest (nextTime, iterate, nextValueTime (), nextMessageTime ());
#ifndef HELICS_DISABLE_ASIO
        if ((realtime) && (rt_lag < Time::maxVal())) {
            auto current_clock_time = std::chrono::steady_clock::now();
            auto timegap = current_clock_time - start_clock_time;
            auto current_lead = (nextTime + rt_lag).to_ns() - timegap;
            if (current_lead > std::chrono::milliseconds(0)) {
                ActionMessage tforce(CMD_FORCE_TIME_GRANT);
                tforce.source_id = global_id.load();
                tforce.actionTime = nextTime;
                if (realTimeTimerIndex < 0) {
                    realTimeTimerIndex =
                        mTimer->addTimer(current_clock_time + current_lead, std::move(tforce));
                } else {
                    mTimer->updateTimer(
                        realTimeTimerIndex, current_clock_time + current_lead, std::move(tforce));
                }
            } else {
                ActionMessage tforce(CMD_FORCE_TIME_GRANT);
                tforce.source_id = global_id.load();
                tforce.actionTime = nextTime;
                addAction(tforce);
            }
        }
#endif
        auto ret = processQueue();
        time_granted = timeCoord->getGrantedTime();
        allowed_send_time = timeCoord->allowedSendTime();
        iterating = (ret == message_processing_result::iterating);

        iteration_time retTime = {time_granted, static_cast<iteration_result>(ret)};
        // now fill the event vector so external systems know what has been updated
        switch (iterate) {
            case iteration_request::force_iteration:
                fillEventVectorNextIteration(time_granted);
                break;
            case iteration_request::iterate_if_needed:
                if (time_granted < nextTime) {
                    fillEventVectorNextIteration(time_granted);
                } else {
                    fillEventVectorUpTo(time_granted);
                }
                break;
            case iteration_request::no_iterations:
                if (time_granted < nextTime) {
                    fillEventVectorInclusive(time_granted);
                } else {
                    fillEventVectorUpTo(time_granted);
                }

                break;
        }
#ifndef HELICS_DISABLE_ASIO
        if (realtime) {
            if (rt_lag < Time::maxVal()) {
                mTimer->cancelTimer(realTimeTimerIndex);
            }
            if (ret == message_processing_result::next_step) {
                auto current_clock_time = std::chrono::steady_clock::now();
                auto timegap = current_clock_time - start_clock_time;
                if (time_granted - Time(timegap) > rt_lead) {
                    auto current_lead = (time_granted - rt_lead).to_ns() - timegap;
                    if (current_lead > std::chrono::milliseconds(5)) {
                        std::this_thread::sleep_for(current_lead);
                    }
                }
            }
        }
#endif

        unlock();
        if ((retTime.grantedTime > nextTime) && (nextTime > lastTime)) {
            if (!ignore_time_mismatch_warnings) {
                LOG_WARNING(fmt::format(
                    "Time mismatch detected granted time >requested time {} vs {}",
                    static_cast<double>(retTime.grantedTime),
                    static_cast<double>(nextTime)));
            }
        }
        return retTime;
    }
    // this would not be good practice to get into this part of the function
    // but the area must protect itself and should return something sensible
    std::lock_guard<FederateState> fedlock(*this);
    iteration_result ret = iterating ? iteration_result::iterating : iteration_result::next_step;
    if (state == HELICS_FINISHED) {
        ret = iteration_result::halted;
    } else if (state == HELICS_ERROR) {
        ret = iteration_result::error;
    }
    iteration_time retTime = {time_granted, ret};
    return retTime;
}

void FederateState::fillEventVectorUpTo(Time currentTime)
{
    events.clear();
    for (auto& ipt : interfaceInformation.getInputs()) {
        bool updated = ipt->updateTimeUpTo(currentTime);
        if (updated) {
            events.push_back(ipt->id.handle);
        }
    }
}

void FederateState::fillEventVectorInclusive(Time currentTime)
{
    events.clear();
    for (auto& ipt : interfaceInformation.getInputs()) {
        bool updated = ipt->updateTimeInclusive(currentTime);
        if (updated) {
            events.push_back(ipt->id.handle);
        }
    }
}

void FederateState::fillEventVectorNextIteration(Time currentTime)
{
    events.clear();
    for (auto& ipt : interfaceInformation.getInputs()) {
        bool updated = ipt->updateTimeNextIteration(currentTime);
        if (updated) {
            events.push_back(ipt->id.handle);
        }
    }
}

iteration_result FederateState::genericUnspecifiedQueueProcess()
{
    if (try_lock()) { // only 1 thread can enter this loop once per federate
        auto ret = processQueue();
        time_granted = timeCoord->getGrantedTime();
        allowed_send_time = timeCoord->allowedSendTime();
        unlock();
        return static_cast<iteration_result>(ret);
    }

    std::lock_guard<FederateState> fedlock(*this);
    return iteration_result::next_step;
}

void FederateState::finalize()
{
    if ((state == federate_state::HELICS_FINISHED) || (state == federate_state::HELICS_ERROR)) {
        return;
    }
    iteration_result ret = iteration_result::next_step;
    while (ret != iteration_result::halted) {
        ret = genericUnspecifiedQueueProcess();
        if (ret == iteration_result::error) {
            break;
        }
    }
}

const std::vector<interface_handle> emptyHandles;

const std::vector<interface_handle>& FederateState::getEvents() const
{
    return events;
}

message_processing_result FederateState::processDelayQueue()
{
    delayedFederates.clear();
    auto ret_code = message_processing_result::continue_processing;
    if (!delayQueues.empty()) {
        for (auto& dQ : delayQueues) {
            auto& tempQueue = dQ.second;
            ret_code = message_processing_result::continue_processing;
            // we specifically want to stop the loop on a delay_message return
            while ((ret_code == message_processing_result::continue_processing) &&
                   (!tempQueue.empty())) {
                auto& cmd = tempQueue.front();
                if (messageShouldBeDelayed(cmd)) {
                    ret_code = message_processing_result::delay_message;
                    continue;
                }

                ret_code = processActionMessage(cmd);
                if (ret_code == message_processing_result::delay_message) {
                    continue;
                }
                tempQueue.pop_front();
            }
            if (returnableResult(ret_code)) {
                break;
            }
        }
    }
    return ret_code;
}

void FederateState::addFederateToDelay(global_federate_id id)
{
    if ((delayedFederates.empty()) || (id > delayedFederates.back())) {
        delayedFederates.push_back(id);
        return;
    }
    auto res = std::lower_bound(delayedFederates.begin(), delayedFederates.end(), id);
    if (res == delayedFederates.end()) {
        delayedFederates.push_back(id);
        return;
    }
    if (*res != id) {
        delayedFederates.insert(res, id);
    }
}

bool FederateState::messageShouldBeDelayed(const ActionMessage& cmd) const
{
    switch (delayedFederates.size()) {
        case 0:
            return false;
        case 1:
            return (cmd.source_id == delayedFederates.front());
        case 2:
            return (
                (cmd.source_id == delayedFederates.front()) ||
                (cmd.source_id == delayedFederates.back()));
        default: {
            auto res =
                std::lower_bound(delayedFederates.begin(), delayedFederates.end(), cmd.source_id);
            return ((res != delayedFederates.end()) && (*res == cmd.source_id));
        }
    }
}

message_processing_result FederateState::processQueue()
{
    if (state == HELICS_FINISHED) {
        return message_processing_result::halted;
    }
    if (state == HELICS_ERROR) {
        return message_processing_result::error;
    }
    // process the delay Queue first
    auto ret_code = processDelayQueue();

    while (!(returnableResult(ret_code))) {
        auto cmd = queue.pop();
        if (messageShouldBeDelayed(cmd)) {
            delayQueues[cmd.source_id].push_back(cmd);
            continue;
        }
        //    messLog.push_back(cmd);
        ret_code = processActionMessage(cmd);
        if (ret_code == message_processing_result::delay_message) {
            delayQueues[static_cast<global_federate_id>(cmd.source_id)].push_back(cmd);
        }
    }
    return ret_code;
}

message_processing_result FederateState::processActionMessage(ActionMessage& cmd)
{
    LOG_TRACE(fmt::format("processing cmd {}", prettyPrintString(cmd)));
    switch (cmd.action()) {
        case CMD_IGNORE:
        default:
            break;
        case CMD_LOG: {
            if (cmd.getStringData().empty()) {
                logMessage(cmd.messageID, emptyStr, cmd.payload);
            } else {
                logMessage(cmd.messageID, cmd.getStringData()[0], cmd.payload);
            }
        }

        break;
        case CMD_TIME_BLOCK:
        case CMD_TIME_UNBLOCK: {
            auto processed = timeCoord->processTimeMessage(cmd);
            if (processed == message_process_result::processed) {
                if (!timeGranted_mode) {
                    if (state == HELICS_INITIALIZING) {
                        cmd.setAction(CMD_EXEC_CHECK);
                        return processActionMessage(cmd);
                    }
                    if (state == HELICS_EXECUTING) {
                        cmd.setAction(CMD_TIME_CHECK);
                        return processActionMessage(cmd);
                    }
                }
            }
            break;
        }
        case CMD_INIT_GRANT:
            if (state == HELICS_CREATED) {
                setState(HELICS_INITIALIZING);
                LOG_TIMING("Granting Initialization");
                timeGranted_mode = true;
                int pcode = checkInterfaces();
                if (pcode != 0) {
                    return message_processing_result::error;
                }
                return message_processing_result::next_step;
            }
            break;
        case CMD_EXEC_REQUEST:
            if ((cmd.source_id == global_id.load()) &&
                (cmd.dest_id == parent_broker_id)) { // this sets up a time request
                iteration_request iterate = iteration_request::no_iterations;
                if (checkActionFlag(cmd, iteration_requested_flag)) {
                    iterate = (checkActionFlag(cmd, required_flag)) ?
                        iteration_request::force_iteration :
                        iteration_request::iterate_if_needed;
                }
                timeCoord->enteringExecMode(iterate);
                timeGranted_mode = false;
                auto ret = processDelayQueue();
                if (returnableResult(ret)) {
                    return ret;
                }
                cmd.setAction(CMD_EXEC_CHECK);
                return processActionMessage(cmd);
            }
            FALLTHROUGH
            /* FALLTHROUGH */
        case CMD_EXEC_GRANT:
            switch (timeCoord->processTimeMessage(cmd)) {
                case message_process_result::delay_processing:
                    addFederateToDelay(global_federate_id(cmd.source_id));
                    return message_processing_result::delay_message;
                case message_process_result::no_effect:
                    return message_processing_result::continue_processing;
                default:
                    break;
            }
            FALLTHROUGH
            /* FALLTHROUGH */
        case CMD_EXEC_CHECK: // just check the time for entry
        {
            if (state != HELICS_INITIALIZING) {
                break;
            }
            if (!timeGranted_mode) {
                auto grant = timeCoord->checkExecEntry();
                switch (grant) {
                    case message_processing_result::iterating:
                        timeGranted_mode = true;
                        return grant;
                    case message_processing_result::next_step:
                        setState(HELICS_EXECUTING);
                        LOG_TIMING("Granting Execution");
                        timeGranted_mode = true;
                        return grant;
                    case message_processing_result::continue_processing:
                        break;
                    default:
                        timeGranted_mode = true;
                        return grant;
                }
            }
        } break;
        case CMD_TERMINATE_IMMEDIATELY:
            setState(HELICS_FINISHED);
            LOG_TIMING("Terminating");
            return message_processing_result::halted;
        case CMD_STOP:
            setState(HELICS_FINISHED);
            LOG_TIMING("Terminating");
            timeCoord->disconnect();
            return message_processing_result::halted;
        case CMD_DISCONNECT_FED_ACK:
            if ((cmd.dest_id == global_id.load()) && (cmd.source_id == parent_broker_id)) {
                if ((state != HELICS_FINISHED) && (state != HELICS_TERMINATING)) {
                    timeCoord->disconnect();
                }
                setState(HELICS_FINISHED);
                return message_processing_result::halted;
            }
            break;
        case CMD_DISCONNECT_FED:
        case CMD_DISCONNECT:
            if (cmd.source_id == global_id.load()) {
                if ((state != HELICS_FINISHED) && (state != HELICS_TERMINATING)) {
                    timeCoord->disconnect();
                    cmd.dest_id = parent_broker_id;
                    setState(HELICS_TERMINATING);
                    routeMessage(cmd);
                }
            } else {
                switch (timeCoord->processTimeMessage(cmd)) {
                    case message_process_result::delay_processing:
                        addFederateToDelay(global_federate_id(cmd.source_id));
                        return message_processing_result::delay_message;
                    case message_process_result::no_effect:
                        return message_processing_result::continue_processing;
                    default:
                        break;
                }
                if (state != HELICS_EXECUTING) {
                    break;
                }
                if (!timeGranted_mode) {
                    auto ret = timeCoord->checkTimeGrant();
                    if (returnableResult(ret)) {
                        time_granted = timeCoord->getGrantedTime();
                        allowed_send_time = timeCoord->allowedSendTime();
                        timeGranted_mode = true;
                        return ret;
                    }
                }
            }
            break;
        case CMD_BROADCAST_DISCONNECT:
        case CMD_DISCONNECT_BROKER:
        case CMD_DISCONNECT_CORE:
            switch (timeCoord->processTimeMessage(cmd)) {
                case message_process_result::delay_processing:
                    addFederateToDelay(global_federate_id(cmd.source_id));
                    return message_processing_result::delay_message;
                case message_process_result::no_effect:
                    return message_processing_result::continue_processing;
                default:
                    break;
            }
            if (state != HELICS_EXECUTING) {
                break;
            }
            if (!timeGranted_mode) {
                auto ret = timeCoord->checkTimeGrant();
                if (returnableResult(ret)) {
                    time_granted = timeCoord->getGrantedTime();
                    allowed_send_time = timeCoord->allowedSendTime();
                    timeGranted_mode = true;
                    return ret;
                }
            }
            break;
        case CMD_CLOSE_INTERFACE:
            if (cmd.source_id == global_id.load()) {
                closeInterface(cmd.source_handle, static_cast<handle_type>(cmd.counter));
            }
            break;
        case CMD_TIME_REQUEST:
            if ((cmd.source_id == global_id.load()) &&
                (cmd.dest_id == parent_broker_id)) { // this sets up a time request
                iteration_request iterate = iteration_request::no_iterations;
                if (checkActionFlag(cmd, iteration_requested_flag)) {
                    iterate = (checkActionFlag(cmd, required_flag)) ?
                        iteration_request::force_iteration :
                        iteration_request::iterate_if_needed;
                }
                timeCoord->timeRequest(cmd.actionTime, iterate, nextValueTime(), nextMessageTime());
                timeGranted_mode = false;
                auto ret = processDelayQueue();
                if (returnableResult(ret)) {
                    return ret;
                }
                cmd.setAction(CMD_TIME_CHECK);
                return processActionMessage(cmd);
            }
            FALLTHROUGH
            /* FALLTHROUGH */
        case CMD_TIME_GRANT:
            switch (timeCoord->processTimeMessage(cmd)) {
                case message_process_result::delay_processing:
                    addFederateToDelay(global_federate_id(cmd.source_id));
                    return message_processing_result::delay_message;
                case message_process_result::no_effect:
                    return message_processing_result::continue_processing;
                default:
                    break;
            }
            FALLTHROUGH
            /* FALLTHROUGH */
        case CMD_TIME_CHECK: {
            if (state != HELICS_EXECUTING) {
                break;
            }
            if (!timeGranted_mode) {
                auto ret = timeCoord->checkTimeGrant();
                if (returnableResult(ret)) {
                    time_granted = timeCoord->getGrantedTime();
                    allowed_send_time = timeCoord->allowedSendTime();
                    LOG_TIMING(fmt::format("Granted Time={}", time_granted));
                    timeGranted_mode = true;
                    return ret;
                }
            }
        } break;
        case CMD_FORCE_TIME_GRANT: {
            if (cmd.actionTime < time_granted) {
                break;
            }
            timeCoord->processTimeMessage(cmd);
            time_granted = timeCoord->getGrantedTime();
            allowed_send_time = timeCoord->allowedSendTime();
            if (!ignore_time_mismatch_warnings) {
                LOG_WARNING(fmt::format("forced Granted Time={}", time_granted));
            }
            timeGranted_mode = true;
            return message_processing_result::next_step;
        }
        case CMD_SEND_MESSAGE: {
            auto epi = interfaceInformation.getEndpoint(cmd.dest_handle);
            if (epi != nullptr) {
                timeCoord->updateMessageTime(cmd.actionTime);
                LOG_DATA(fmt::format("receive_message {}", prettyPrintString(cmd)));
                epi->addMessage(createMessageFromCommand(std::move(cmd)));
            }
        } break;
        case CMD_PUB: {
            auto subI = interfaceInformation.getInput(interface_handle(cmd.dest_handle));
            if (subI == nullptr) {
                break;
            }
            for (auto& src : subI->input_sources) {
                if ((cmd.source_id == src.fed_id) && (cmd.source_handle == src.handle)) {
                    subI->addData(
                        src,
                        cmd.actionTime,
                        cmd.counter,
                        std::make_shared<const data_block>(std::move(cmd.payload)));
                    if (!subI->not_interruptible) {
                        timeCoord->updateValueTime(cmd.actionTime);
                        LOG_TRACE(timeCoord->printTimeStatus());
                    }
                    LOG_DATA(fmt::format("receive publication {}", prettyPrintString(cmd)));
                }
            }
        } break;
        case CMD_WARNING:
            if (cmd.payload.empty()) {
                cmd.payload = commandErrorString(cmd.messageID);
                if (cmd.payload == "unknown") {
                    cmd.payload += " code:" + std::to_string(cmd.messageID);
                }
            }
            LOG_WARNING(cmd.payload);
            break;
        case CMD_ERROR:
            setState(HELICS_ERROR);
            if (cmd.payload.empty()) {
                errorString = commandErrorString(cmd.messageID);
                if (errorString == "unknown") {
                    errorString += " code:" + std::to_string(cmd.messageID);
                }
            } else {
                errorString = cmd.payload;
            }
            errorCode = cmd.messageID;
            return message_processing_result::error;
        case CMD_ADD_PUBLISHER: {
            auto subI = interfaceInformation.getInput(cmd.dest_handle);
            if (subI != nullptr) {
                subI->addSource(
                    cmd.getSource(),
                    cmd.name,
                    cmd.getString(typeStringLoc),
                    cmd.getString(unitStringLoc));
                addDependency(cmd.source_id);
            }
        } break;
        case CMD_ADD_SUBSCRIBER: {
            auto pubI = interfaceInformation.getPublication(cmd.dest_handle);
            if (pubI != nullptr) {
                pubI->subscribers.emplace_back(cmd.source_id, cmd.source_handle);
                addDependent(cmd.source_id);
            }
        } break;
        case CMD_ADD_DEPENDENCY:
        case CMD_REMOVE_DEPENDENCY:
        case CMD_ADD_DEPENDENT:
        case CMD_REMOVE_DEPENDENT:
        case CMD_ADD_INTERDEPENDENCY:
        case CMD_REMOVE_INTERDEPENDENCY:
            if (cmd.dest_id == global_id.load()) {
                timeCoord->processDependencyUpdateMessage(cmd);
            }

            break;
        case CMD_REMOVE_NAMED_PUBLICATION: {
            auto subI = interfaceInformation.getInput(cmd.source_handle);
            if (subI != nullptr) {
                subI->removeSource(
                    cmd.name, (cmd.actionTime != timeZero) ? cmd.actionTime : time_granted);
            }
            break;
        }
        case CMD_REMOVE_PUBLICATION: {
            auto subI = interfaceInformation.getInput(cmd.dest_handle);
            if (subI != nullptr) {
                subI->removeSource(
                    cmd.getSource(), (cmd.actionTime != timeZero) ? cmd.actionTime : time_granted);
            }
            break;
        }
        case CMD_REMOVE_SUBSCRIBER: {
            auto pubI = interfaceInformation.getPublication(cmd.dest_handle);
            if (pubI != nullptr) {
                pubI->removeSubscriber(cmd.getSource());
            }
        } break;
        case CMD_REMOVE_ENDPOINT:
            break;
        case CMD_FED_ACK:
            if (state != HELICS_CREATED) {
                break;
            }
            if (cmd.name == name) {
                if (checkActionFlag(cmd, error_flag)) {
                    setState(HELICS_ERROR);
                    errorString = commandErrorString(cmd.messageID);
                    return message_processing_result::error;
                }
                global_id = cmd.dest_id;
                interfaceInformation.setGlobalId(cmd.dest_id);
                timeCoord->source_id = global_id;
                return message_processing_result::next_step;
            }
            break;
        case CMD_FED_CONFIGURE_TIME:
            setProperty(cmd.messageID, cmd.actionTime);
            break;
        case CMD_FED_CONFIGURE_INT:
            setProperty(cmd.messageID, cmd.getExtraData());
            break;
        case CMD_FED_CONFIGURE_FLAG:
            setOptionFlag(cmd.messageID, checkActionFlag(cmd, indicator_flag));
            break;
        case CMD_INTERFACE_CONFIGURE:
            setInterfaceProperty(cmd);
            break;
        case CMD_QUERY: {
            std::string repStr;
            ActionMessage queryResp(CMD_QUERY_REPLY);
            queryResp.dest_id = cmd.source_id;
            queryResp.source_id = cmd.dest_id;
            queryResp.messageID = cmd.messageID;
            queryResp.counter = cmd.counter;

            queryResp.payload = processQueryActual(cmd.payload);
            routeMessage(queryResp);
        } break;
    }
    return message_processing_result::continue_processing;
}

void FederateState::setProperties(const ActionMessage& cmd)
{
    if (state == HELICS_CREATED) {
        switch (cmd.action()) {
            case CMD_FED_CONFIGURE_FLAG:
                spinlock();
                setOptionFlag(cmd.messageID, checkActionFlag(cmd, indicator_flag));
                unlock();
                break;
            case CMD_FED_CONFIGURE_TIME:
                spinlock();
                setProperty(cmd.messageID, cmd.actionTime);
                unlock();
                break;
            case CMD_FED_CONFIGURE_INT:
                spinlock();
                setProperty(cmd.messageID, cmd.getExtraData());
                unlock();
                break;
            case CMD_INTERFACE_CONFIGURE:
                spinlock();
                setInterfaceProperty(cmd);
                unlock();
                break;
            default:
                break;
        }
    } else {
        switch (cmd.action()) {
            case CMD_FED_CONFIGURE_FLAG:
            case CMD_FED_CONFIGURE_TIME:
            case CMD_FED_CONFIGURE_INT:
            case CMD_INTERFACE_CONFIGURE:
                addAction(cmd);
                break;
            default:
                break;
        }
    }
}

void FederateState::setInterfaceProperty(const ActionMessage& cmd)
{
    if (cmd.action() != CMD_INTERFACE_CONFIGURE) {
        return;
    }
    bool used = false;
    switch (static_cast<char>(cmd.counter)) {
        case 'i':
            used = interfaceInformation.setInputProperty(
                cmd.dest_handle, cmd.messageID, checkActionFlag(cmd, indicator_flag));
            if (!used) {
                auto ipt = interfaceInformation.getInput(cmd.dest_handle);
                if (ipt != nullptr) {
                    LOG_WARNING(
                        fmt::format("property {} not used on input {}", cmd.messageID, ipt->key));
                } else {
                    LOG_WARNING(
                        fmt::format("property {} not used on due to unknown input", cmd.messageID));
                }
            }
            break;
        case 'p':
            used = interfaceInformation.setPublicationProperty(
                cmd.dest_handle, cmd.messageID, checkActionFlag(cmd, indicator_flag));
            if (!used) {
                auto pub = interfaceInformation.getPublication(cmd.dest_handle);
                if (pub != nullptr) {
                    LOG_WARNING(fmt::format(
                        "property {} not used on Publication {}", cmd.messageID, pub->key));
                } else {
                    LOG_WARNING(fmt::format(
                        "property {} not used on due to unknown Publication", cmd.messageID));
                }
            }
            break;
        case 'e':
            used = interfaceInformation.setInputProperty(
                cmd.dest_handle, cmd.messageID, checkActionFlag(cmd, indicator_flag));
            if (!used) {
                auto ept = interfaceInformation.getEndpoint(cmd.dest_handle);
                if (ept != nullptr) {
                    LOG_WARNING(fmt::format(
                        "property {} not used on Endpoint {}", cmd.messageID, ept->key));
                } else {
                    LOG_WARNING(fmt::format(
                        "property {} not used on due to unknown Endpoint", cmd.messageID));
                }
            }
            break;
        default:
            break;
    }
}

void FederateState::setProperty(int timeProperty, Time propertyVal)
{
    switch (timeProperty) {
        case defs::properties::rt_lag:
            rt_lag = propertyVal;
            break;
        case defs::properties::rt_lead:
            rt_lead = propertyVal;
            break;
        case defs::properties::rt_tolerance:
            rt_lag = propertyVal;
            rt_lead = propertyVal;
            break;
        default:
            timeCoord->setProperty(timeProperty, propertyVal);
            break;
    }
}

/** set a timeProperty for a the coordinator*/
void FederateState::setProperty(int intProperty, int propertyVal)
{
    switch (intProperty) {
        case defs::properties::log_level:
        case defs::properties::file_log_level:
        case defs::properties::console_log_level:
            logLevel = propertyVal;
            break;
        case defs::properties::rt_lag:
            rt_lag = helics::Time(static_cast<double>(propertyVal));
            break;
        case defs::properties::rt_lead:
            rt_lead = helics::Time(static_cast<double>(propertyVal));
            break;
        case defs::properties::rt_tolerance:
            rt_lag = helics::Time(static_cast<double>(propertyVal));
            rt_lead = rt_lag;
            break;
        default:
            timeCoord->setProperty(intProperty, propertyVal);
    }
}

/** set an option Flag for a the coordinator*/
void FederateState::setOptionFlag(int optionFlag, bool value)
{
    switch (optionFlag) {
        case defs::flags::only_transmit_on_change:
            only_transmit_on_change = value;
            break;
        case defs::flags::only_update_on_change:
            interfaceInformation.setChangeUpdateFlag(value);
            break;
        case defs::flags::strict_input_type_checking:
            strict_input_type_checking = value;
            break;
        case defs::flags::ignore_input_unit_mismatch:
            ignore_unit_mismatch = value;
            break;
        case defs::flags::slow_responding:
            slow_responding = value;
            break;
        case defs::flags::realtime:
            if (value) {
                if (state < HELICS_EXECUTING) {
                    realtime = true;
                }
            } else {
                realtime = false;
            }

            break;
        case defs::flags::source_only:
            if (state == HELICS_CREATED) {
                source_only = value;
                if (value) {
                    observer = false;
                }
            }
            break;
        case defs::flags::observer:
            if (state == HELICS_CREATED) {
                observer = value;
                if (value) {
                    source_only = false;
                }
            }
            break;
        case defs::flags::ignore_time_mismatch_warnings:
            ignore_time_mismatch_warnings = value;
            break;
        case defs::options::buffer_data:
            break;
        case defs::flags::connections_required:
            if (value) {
                interfaceFlags |= make_flags(required_flag);
            } else {
                interfaceFlags &= ~(make_flags(required_flag));
            }
            break;
        case defs::flags::connections_optional:
            if (value) {
                interfaceFlags |= make_flags(optional_flag);
            } else {
                interfaceFlags &= ~(make_flags(optional_flag));
            }
            break;
        default:
            timeCoord->setOptionFlag(optionFlag, value);
            break;
    }
}

/** get a time Property*/
Time FederateState::getTimeProperty(int timeProperty) const
{
    switch (timeProperty) {
        case defs::properties::rt_lag:
        case defs::properties::rt_tolerance:
            return rt_lag;
        case defs::properties::rt_lead:
            return rt_lead;
        default:
            return timeCoord->getTimeProperty(timeProperty);
    }
}

/** get an option flag value*/
bool FederateState::getOptionFlag(int optionFlag) const
{
    switch (optionFlag) {
        case defs::flags::only_transmit_on_change:
            return only_transmit_on_change;
        case defs::flags::only_update_on_change:
            return interfaceInformation.getChangeUpdateFlag();
        case defs::flags::realtime:
            return realtime;
        case defs::flags::observer:
            return observer;
        case defs::flags::source_only:
            return source_only;
        case defs::flags::slow_responding:
            return slow_responding;
        case defs::flags::connections_required:
            return ((interfaceFlags.load() & make_flags(required_flag)) != 0);
        case defs::flags::connections_optional:
            return ((interfaceFlags.load() & make_flags(optional_flag)) != 0);
        case defs::flags::strict_input_type_checking:
            return strict_input_type_checking;
        case defs::flags::ignore_input_unit_mismatch:
            return ignore_unit_mismatch;
        case defs::flags::ignore_time_mismatch_warnings:
            return ignore_time_mismatch_warnings;
        default:
            return timeCoord->getOptionFlag(optionFlag);
    }
}

bool FederateState::getHandleOption(interface_handle handle, char iType, int32_t option) const
{
    switch (iType) {
        case 'i':
            return interfaceInformation.getInputProperty(handle, option);
        case 'p':
            return interfaceInformation.getPublicationProperty(handle, option);
        case 'e':
            return interfaceInformation.getEndpointProperty(handle, option);
        default:
            break;
    }
    return false;
}

/** get an option flag value*/
int FederateState::getIntegerProperty(int intProperty) const
{
    switch (intProperty) {
        case defs::properties::log_level:
        case defs::properties::file_log_level:
        case defs::properties::console_log_level:
            return logLevel;
        default:
            return timeCoord->getIntegerProperty(intProperty);
    }
}

int FederateState::publicationCount() const
{
    return static_cast<int>(interfaceInformation.getPublications()->size());
}

int FederateState::endpointCount() const
{
    return static_cast<int>(interfaceInformation.getEndpoints()->size());
}

int FederateState::inputCount() const
{
    return static_cast<int>(interfaceInformation.getInputs()->size());
}

std::vector<global_federate_id> FederateState::getDependencies() const
{
    return timeCoord->getDependencies();
}

std::vector<global_federate_id> FederateState::getDependents() const
{
    return timeCoord->getDependents();
}

void FederateState::addDependency(global_federate_id fedToDependOn)
{
    timeCoord->addDependency(fedToDependOn);
}

void FederateState::addDependent(global_federate_id fedThatDependsOnThis)
{
    timeCoord->addDependent(fedThatDependsOnThis);
}

int FederateState::checkInterfaces()
{
    auto issues = interfaceInformation.checkInterfacesForIssues();
    if (issues.empty()) {
        return 0;
    }
    errorCode = issues.front().first;
    errorString = issues.front().second;
    for (auto& issue : issues) {
        switch (issue.first) {
            case defs::errors::connection_failure:
                LOG_ERROR(fmt::format("Connection Error: {}", issue.second));
                break;
            default:
                LOG_ERROR(fmt::format("error code {}: {}", issue.first, issue.second));
                break;
        }
    }
    return errorCode;
}
Time FederateState::nextValueTime() const
{
    auto firstValueTime = Time::maxVal();
    for (auto& inp : interfaceInformation.getInputs()) {
        auto nvt = inp->nextValueTime();
        if (nvt >= time_granted) {
            if (nvt < firstValueTime) {
                firstValueTime = nvt;
            }
        }
    }
    return firstValueTime;
}

/** find the next Message Event*/
Time FederateState::nextMessageTime() const
{
    auto firstMessageTime = Time::maxVal();
    for (auto& ep : interfaceInformation.getEndpoints()) {
        auto messageTime = ep->firstMessageTime();
        if (messageTime >= time_granted) {
            if (messageTime < firstMessageTime) {
                firstMessageTime = messageTime;
            }
        }
    }
    return firstMessageTime;
}

void FederateState::setCoreObject(CommonCore* parent)
{
    spinlock();
    parent_ = parent;
    unlock();
}

void FederateState::logMessage(
    int level,
    const std::string& logMessageSource,
    const std::string& message) const
{
    if ((loggerFunction) && (level <= logLevel)) {
        loggerFunction(
            level,
            (logMessageSource.empty()) ?
                fmt::format("{} ({})", name, global_id.load().baseValue()) :
                logMessageSource,
            message);
    }
}

std::string FederateState::processQueryActual(const std::string& query) const
{
    if (query == "publications") {
        return generateStringVector(interfaceInformation.getPublications(), [](auto& pub) {
            return pub->key;
        });
    }
    if (query == "inputs") {
        return generateStringVector(interfaceInformation.getInputs(), [](auto& inp) {
            return inp->key;
        });
    }
    if (query == "endpoints") {
        return generateStringVector(interfaceInformation.getEndpoints(), [](auto& ept) {
            return ept->key;
        });
    }
    if (query == "interfaces") {
        return "{" + interfaceInformation.generateInferfaceConfig() + "}";
    }
    if (query == "subscriptions") {
        std::ostringstream s;
        s << "[";
        auto ipts = interfaceInformation.getInputs();
        for (auto& ipt : ipts) {
            for (auto& isrc : ipt->input_sources) {
                s << isrc.fed_id << ':' << isrc.handle << ';';
            }
        }
        ipts.unlock();
        unlock();
        auto str = s.str();
        if (str.back() == ';') {
            str.pop_back();
        }
        str.push_back(']');
        return str;
    }
    if (query == "dependencies") {
        return generateStringVector(timeCoord->getDependencies(), [](auto& dep) {
            return std::to_string(dep.baseValue());
        });
    }
    if (query == "timeconfig") {
        std::ostringstream s;
        s << "{\n" << timeCoord->generateConfig();
        s << ",\n" << generateConfig();
        s << "}";
        return s.str();
    }
    if (query == "config") {
        std::ostringstream s;
        s << "{\n" << timeCoord->generateConfig();
        s << ",\n" << generateConfig();
        s << ",\n" << interfaceInformation.generateInferfaceConfig();
        s << "}";
        return s.str();
    }
    if (query == "dependents") {
        return generateStringVector(timeCoord->getDependents(), [](auto& dep) {
            return std::to_string(dep.baseValue());
        });
    }
    if (queryCallback) {
        return queryCallback(query);
    }
    return "#invalid";
}

std::string FederateState::processQuery(const std::string& query) const
{
    std::string qstring;
    if (query == "publications" || query == "inputs" ||
        query == "endpoints") { // these never need to be locked
        qstring = processQueryActual(query);
    } else if ((query == "queries") || (query == "available_queries")) {
        qstring =
            "publications;inputs;endpoints;interfaces;subscriptions;dependencies;timeconfig;config;dependents";
    } else { // the rest might to prevent a race condition
        if (try_lock()) {
            qstring = processQueryActual(query);
            unlock();
        } else {
            qstring = "#wait";
        }
    }
    return qstring;
}
} // namespace helics
