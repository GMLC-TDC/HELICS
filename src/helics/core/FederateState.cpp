/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "FederateState.hpp"

#include "../common/JsonGeneration.hpp"
#include "../common/JsonProcessingFunctions.hpp"
#include "../common/LogBuffer.hpp"
#include "../common/logging.hpp"
#include "CommonCore.hpp"
#include "CoreFederateInfo.hpp"
#include "EndpointInfo.hpp"
#include "InputInfo.hpp"
#include "LogManager.hpp"
#include "PublicationInfo.hpp"
#include "TimeCoordinator.hpp"
#include "TimeCoordinatorProcessing.hpp"
#include "TimeDependencies.hpp"
#include "gmlc/utilities/string_viewConversion.h"
#include "helics/helics-config.h"
#include "helics_definitions.hpp"
#include "queryHelpers.hpp"

#include <algorithm>
#include <chrono>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <utility>

#ifndef HELICS_DISABLE_ASIO
#    include "MessageTimer.hpp"
#else
namespace helics {
class MessageTimer {};
}  // namespace helics
#endif

#include "../common/fmt_format.h"
static const std::string gEmptyStr;
#define LOG_ERROR(message) logMessage(HELICS_LOG_LEVEL_ERROR, gEmptyStr, message)
#define LOG_WARNING(message) logMessage(HELICS_LOG_LEVEL_WARNING, gEmptyStr, message)

#ifdef HELICS_ENABLE_LOGGING

#    define LOG_SUMMARY(message)                                                                   \
        do {                                                                                       \
            if (maxLogLevel >= HELICS_LOG_LEVEL_SUMMARY) {                                         \
                logMessage(HELICS_LOG_LEVEL_SUMMARY, gEmptyStr, message);                          \
            }                                                                                      \
        } while (false)

#    define LOG_INTERFACES(message)                                                                \
        do {                                                                                       \
            if (maxLogLevel >= HELICS_LOG_LEVEL_INTERFACES) {                                      \
                logMessage(HELICS_LOG_LEVEL_INTERFACES, gEmptyStr, message);                       \
            }                                                                                      \
        } while (false)

#    ifdef HELICS_ENABLE_DEBUG_LOGGING
#        define LOG_TIMING(message)                                                                \
            do {                                                                                   \
                if (maxLogLevel >= HELICS_LOG_LEVEL_TIMING) {                                      \
                    logMessage(HELICS_LOG_LEVEL_TIMING, gEmptyStr, message);                       \
                }                                                                                  \
            } while (false)

#        define LOG_DATA(message)                                                                  \
            do {                                                                                   \
                if (maxLogLevel >= HELICS_LOG_LEVEL_DATA) {                                        \
                    logMessage(HELICS_LOG_LEVEL_DATA, gEmptyStr, message);                         \
                }                                                                                  \
            } while (false)
#    else
#        define LOG_TIMING(message)
#        define LOG_DATA(message)
#    endif

#    ifdef HELICS_ENABLE_TRACE_LOGGING
#        define LOG_TRACE(message)                                                                 \
            do {                                                                                   \
                if (maxLogLevel >= HELICS_LOG_LEVEL_TRACE) {                                       \
                    logMessage(HELICS_LOG_LEVEL_TRACE, gEmptyStr, message);                        \
                }                                                                                  \
            } while (false)
#    else
#        define LOG_TRACE(message) ((void)0)
#    endif
#else  // LOGGING_DISABLED
#    define LOG_SUMMARY(message) ((void)0)
#    define LOG_INTERFACES(message) ((void)0)
#    define LOG_TIMING(message) ((void)0)
#    define LOG_DATA(message) ((void)0)
#    define LOG_TRACE(message) ((void)0)
#endif  // LOGGING_DISABLED

using namespace std::chrono_literals;  // NOLINT

namespace helics {
FederateState::FederateState(const std::string& fedName, const CoreFederateInfo& fedInfo):
    name(fedName),
    timeCoord(new TimeCoordinator([this](const ActionMessage& msg) { routeMessage(msg); })),
    global_id{GlobalFederateId()}, mLogManager(std::make_unique<LogManager>())
{
    for (const auto& prop : fedInfo.timeProps) {
        setProperty(prop.first, prop.second);
    }
    for (const auto& prop : fedInfo.intProps) {
        setProperty(prop.first, prop.second);
    }
    for (const auto& prop : fedInfo.flagProps) {
        setOptionFlag(prop.first, prop.second);
    }
    mLogManager->setTransmitCallback(
        [this](ActionMessage&& m) { parent_->addActionMessage(std::move(m)); });
    maxLogLevel = mLogManager->getMaxLevel();
}

FederateState::~FederateState() = default;

// define the allowable state transitions for a federate
void FederateState::setState(FederateStates newState)
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
    global_id = GlobalFederateId();
    interfaceInformation.setGlobalId(GlobalFederateId());
    local_id = LocalFederateId();
    state = HELICS_CREATED;
    queue.clear();
    delayQueues.clear();
    // TODO(PT): this probably needs to do a lot more
}
/** reset the federate to the initializing state*/
void FederateState::reInit()
{
    state = HELICS_INITIALIZING;
    queue.clear();
    delayQueues.clear();
    // TODO(PT): this needs to reset a bunch of stuff as well as check a few things
}
FederateStates FederateState::getState() const
{
    return state.load();
}

int32_t FederateState::getCurrentIteration() const
{
    return timeCoord->getCurrentIteration();
}

bool FederateState::checkAndSetValue(InterfaceHandle pub_id, const char* data, uint64_t len)
{
    if (!only_transmit_on_change) {
        return true;
    }
    std::lock_guard<FederateState> plock(*this);
    // this function could be called externally in a multi-threaded context
    auto* pub = interfaceInformation.getPublication(pub_id);
    auto res = pub->CheckSetValue(data, len);
    return res;
}

void FederateState::generateConfig(Json::Value& base) const
{
    base["only_transmit_on_change"] = only_transmit_on_change;
    base["realtime"] = realtime;
    base["observer"] = observer;
    base["source_only"] = source_only;
    base["strict_input_type_checking"] = source_only;
    base["slow_responding"] = slow_responding;
    if (rt_lag > timeZero) {
        base["rt_lag"] = static_cast<double>(rt_lag);
    }
    if (rt_lead > timeZero) {
        base["rt_lead"] = static_cast<double>(rt_lead);
    }
}

uint64_t FederateState::getQueueSize(InterfaceHandle id) const
{
    const auto* epI = interfaceInformation.getEndpoint(id);
    return (epI != nullptr) ? epI->availableMessages() : 0;
}

uint64_t FederateState::getQueueSize() const
{
    uint64_t cnt = 0;
    for (const auto& end_point : interfaceInformation.getEndpoints()) {
        cnt += end_point->availableMessages();
    }
    return cnt;
}

void FederateState::setLogger(
    std::function<void(int, std::string_view, std::string_view)> logFunction)
{
    mLogManager->setLoggerFunction(std::move(logFunction));
}

std::unique_ptr<Message> FederateState::receive(InterfaceHandle id)
{
    auto* epI = interfaceInformation.getEndpoint(id);
    if (epI != nullptr) {
        return epI->getMessage(time_granted);
    }
    return nullptr;
}

std::unique_ptr<Message> FederateState::receiveAny(InterfaceHandle& id)
{
    Time earliest_time = Time::maxVal();
    EndpointInfo* endpointI = nullptr;
    auto elock = interfaceInformation.getEndpoints();
    // Find the end point with the earliest message time
    for (const auto& end_point : elock) {
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
        id = (result) ? endpointI->id.handle : InterfaceHandle{};

        return result;
    }
    id = InterfaceHandle();
    return nullptr;
}

const std::shared_ptr<const SmallBuffer>& FederateState::getValue(InterfaceHandle handle,
                                                                  uint32_t* inputIndex)
{
    return interfaces().getInput(handle)->getData(inputIndex);
}

const std::vector<std::shared_ptr<const SmallBuffer>>&
    FederateState::getAllValues(InterfaceHandle handle)
{
    return interfaces().getInput(handle)->getAllData();
}

void FederateState::routeMessage(const ActionMessage& msg)
{
    if (parent_ != nullptr) {
        if (msg.action() == CMD_TIME_REQUEST && !requestingMode) {
            LOG_ERROR("sending time request in invalid state");
        }
        if (msg.action() == CMD_TIME_GRANT) {
            requestingMode.store(false);
        }
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

void FederateState::createInterface(InterfaceType htype,
                                    InterfaceHandle handle,
                                    const std::string& key,
                                    const std::string& type,
                                    const std::string& units)
{
    std::lock_guard<FederateState> plock(*this);
    // this function could be called externally in a multi-threaded context
    switch (htype) {
        case InterfaceType::PUBLICATION: {
            interfaceInformation.createPublication(handle, key, type, units);
            if (checkActionFlag(getInterfaceFlags(), required_flag)) {
                interfaceInformation.setPublicationProperty(handle,
                                                            defs::Options::CONNECTION_REQUIRED,
                                                            1);
            }
            if (checkActionFlag(getInterfaceFlags(), optional_flag)) {
                interfaceInformation.setPublicationProperty(handle,
                                                            defs::Options::CONNECTION_OPTIONAL,
                                                            1);
            }
        } break;
        case InterfaceType::INPUT: {
            interfaceInformation.createInput(handle, key, type, units);
            if (strict_input_type_checking) {
                interfaceInformation.setInputProperty(handle,
                                                      defs::Options::STRICT_TYPE_CHECKING,
                                                      1);
            }
            if (ignore_unit_mismatch) {
                interfaceInformation.setInputProperty(handle,
                                                      defs::Options::IGNORE_UNIT_MISMATCH,
                                                      1);
            }
            if (checkActionFlag(getInterfaceFlags(), required_flag)) {
                interfaceInformation.setInputProperty(handle,
                                                      defs::Options::CONNECTION_REQUIRED,
                                                      1);
            }
            if (checkActionFlag(getInterfaceFlags(), optional_flag)) {
                interfaceInformation.setInputProperty(handle,
                                                      defs::Options::CONNECTION_OPTIONAL,
                                                      1);
            }
        } break;
        case InterfaceType::ENDPOINT: {
            interfaceInformation.createEndpoint(handle, key, type);
        }
        default:
            break;
    }
}

void FederateState::closeInterface(InterfaceHandle handle, InterfaceType type)
{
    switch (type) {
        case InterfaceType::PUBLICATION: {
            auto* pub = interfaceInformation.getPublication(handle);
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
        case InterfaceType::ENDPOINT: {
            auto* ept = interfaceInformation.getEndpoint(handle);
            if (ept != nullptr) {
                ept->clearQueue();
            }
        } break;
        case InterfaceType::INPUT: {
            auto* ipt = interfaceInformation.getInput(handle);
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

std::optional<ActionMessage>
    FederateState::processPostTerminationAction(const ActionMessage& action)  // NOLINT
{
    std::optional<ActionMessage> optAct;
    switch (action.action()) {
        case CMD_REQUEST_CURRENT_TIME:
            optAct = ActionMessage(CMD_DISCONNECT, global_id.load(), action.source_id);
            break;
        default:
            break;
    }
    return optAct;
}

void FederateState::forceProcessMessage(ActionMessage& action)
{
    if (try_lock()) {
        processActionMessage(action);
        unlock();
    } else {
        addAction(action);
    }
}

IterationResult FederateState::waitSetup()
{
    if (try_lock()) {  // only enter this loop once per federate
        auto ret = processQueue();
        unlock();
        return static_cast<IterationResult>(ret);
    }

    std::lock_guard<FederateState> fedlock(*this);
    IterationResult ret;
    switch (getState()) {
        case HELICS_CREATED: {  // we are still in the created state
            return waitSetup();
        }
        case HELICS_ERROR:
            ret = IterationResult::ERROR_RESULT;
            break;
        case HELICS_FINISHED:
            ret = IterationResult::HALTED;
            break;
        default:
            ret = IterationResult::NEXT_STEP;
            break;
    }

    return ret;
}

IterationResult FederateState::enterInitializingMode()
{
    if (try_lock()) {  // only enter this loop once per federate
        auto ret = processQueue();
        unlock();
        if (ret == MessageProcessingResult::NEXT_STEP) {
            time_granted = initialTime;
            allowed_send_time = initialTime;
        }
        return static_cast<IterationResult>(ret);
    }

    sleeplock();
    IterationResult ret;
    switch (getState()) {
        case HELICS_ERROR:
            ret = IterationResult::ERROR_RESULT;
            break;
        case HELICS_FINISHED:
            ret = IterationResult::HALTED;
            break;
        case HELICS_CREATED:
            unlock();
            return enterInitializingMode();
        default:  // everything >= HELICS_INITIALIZING
            ret = IterationResult::NEXT_STEP;
            break;
    }
    unlock();
    return ret;
}

IterationResult FederateState::enterExecutingMode(IterationRequest iterate, bool sendRequest)
{
    if (try_lock()) {  // only enter this loop once per federate
        // timeCoord->enteringExecMode (iterate);
        if (sendRequest) {
            ActionMessage exec(CMD_EXEC_REQUEST);
            exec.source_id = global_id.load();
            setIterationFlags(exec, iterate);
            setActionFlag(exec, indicator_flag);
            addAction(exec);
        }

        auto ret = processQueue();
        ++mGrantCount;
        if (ret == MessageProcessingResult::NEXT_STEP) {
            time_granted = timeCoord->getGrantedTime();
            allowed_send_time = timeCoord->allowedSendTime();
        } else if (ret == MessageProcessingResult::ITERATING) {
            time_granted = initializationTime;
            allowed_send_time = initializationTime;
        }
        if (ret != MessageProcessingResult::ERROR_RESULT) {
            switch (iterate) {
                case IterationRequest::FORCE_ITERATION:
                    fillEventVectorNextIteration(time_granted);
                    break;
                case IterationRequest::ITERATE_IF_NEEDED:
                    if (ret == MessageProcessingResult::NEXT_STEP) {
                        fillEventVectorUpTo(time_granted);
                    } else {
                        fillEventVectorNextIteration(time_granted);
                    }
                    break;
                case IterationRequest::NO_ITERATIONS:
                    if (wait_for_current_time) {
                        fillEventVectorInclusive(time_granted);
                    } else {
                        fillEventVectorUpTo(time_granted);
                    }
                    break;
            }
        }
        unlock();
#ifndef HELICS_DISABLE_ASIO
        if ((realtime) && (ret == MessageProcessingResult::NEXT_STEP)) {
            if (!mTimer) {
                mTimer = std::make_shared<MessageTimer>(
                    [this](ActionMessage&& mess) { return this->addAction(std::move(mess)); });
            }
            start_clock_time = std::chrono::steady_clock::now();
        } else if (grantTimeOutPeriod > timeZero) {
            if (!mTimer) {
                mTimer = std::make_shared<MessageTimer>(
                    [this](ActionMessage&& mess) { return this->addAction(std::move(mess)); });
            }
        }
#endif
        return static_cast<IterationResult>(ret);
    }
    // the following code is for a situation in which this method has been called multiple times
    // from different threads, which really shouldn't be done but it isn't really an error so we
    // need to deal with it.
    std::lock_guard<FederateState> plock(*this);
    IterationResult ret;
    switch (getState()) {
        case HELICS_ERROR:
            ret = IterationResult::ERROR_RESULT;
            break;
        case HELICS_FINISHED:
            ret = IterationResult::HALTED;
            break;
        case HELICS_CREATED:
        case HELICS_INITIALIZING:
        default:
            ret = IterationResult::ITERATING;
            break;
        case HELICS_EXECUTING:
            ret = IterationResult::NEXT_STEP;
            break;
    }
    return ret;
}

std::vector<GlobalHandle> FederateState::getSubscribers(InterfaceHandle handle)
{
    std::lock_guard<FederateState> fedlock(*this);
    auto* pubInfo = interfaceInformation.getPublication(handle);
    if (pubInfo != nullptr) {
        return pubInfo->subscribers;
    }
    return {};
}

std::vector<std::pair<GlobalHandle, std::string_view>>
    FederateState::getMessageDestinations(InterfaceHandle handle)
{
    std::lock_guard<FederateState> fedlock(*this);
    const auto* eptInfo = interfaceInformation.getEndpoint(handle);
    if (eptInfo != nullptr) {
        return eptInfo->getTargets();
    }
    return {};
}
iteration_time FederateState::requestTime(Time nextTime, IterationRequest iterate, bool sendRequest)
{
    if (try_lock()) {  // only enter this loop once per federate
        Time lastTime = timeCoord->getGrantedTime();
        events.clear();  // clear the event queue
        LOG_TRACE(timeCoord->printTimeStatus());
        // timeCoord->timeRequest (nextTime, iterate, nextValueTime (), nextMessageTime ());

        if (sendRequest) {
            ActionMessage treq(CMD_TIME_REQUEST);
            treq.source_id = global_id.load();
            treq.actionTime = nextTime;
            setIterationFlags(treq, iterate);
            setActionFlag(treq, indicator_flag);
            addAction(treq);
            LOG_TRACE(timeCoord->printTimeStatus());
        }

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
                    mTimer->updateTimer(realTimeTimerIndex,
                                        current_clock_time + current_lead,
                                        std::move(tforce));
                }
            } else {
                ActionMessage tforce(CMD_FORCE_TIME_GRANT);
                tforce.source_id = global_id.load();
                tforce.actionTime = nextTime;
                addAction(tforce);
            }
        } else if (grantTimeOutPeriod > timeZero) {
            ActionMessage grantCheck(CMD_GRANT_TIMEOUT_CHECK);
            grantCheck.setExtraData(static_cast<std::int32_t>(mGrantCount));
            grantCheck.counter = 0;
            if (grantTimeoutTimeIndex < 0) {
                grantTimeoutTimeIndex =
                    mTimer->addTimerFromNow(grantTimeOutPeriod.to_ms(), std::move(grantCheck));
            } else {
                mTimer->updateTimerFromNow(realTimeTimerIndex,
                                           grantTimeOutPeriod.to_ms(),
                                           std::move(grantCheck));
            }
        }
#endif
        auto ret = processQueue();
        ++mGrantCount;
        if (ret == MessageProcessingResult::HALTED) {
            time_granted = Time::maxVal();
            allowed_send_time = Time::maxVal();
            iterating = false;
        } else {
            time_granted = timeCoord->getGrantedTime();
            allowed_send_time = timeCoord->allowedSendTime();
            iterating = (ret == MessageProcessingResult::ITERATING);
        }

        iteration_time retTime = {time_granted, static_cast<IterationResult>(ret)};
        // now fill the event vector so external systems know what has been updated
        switch (iterate) {
            case IterationRequest::FORCE_ITERATION:
                fillEventVectorNextIteration(time_granted);
                break;
            case IterationRequest::ITERATE_IF_NEEDED:
                if (time_granted < nextTime || wait_for_current_time) {
                    fillEventVectorNextIteration(time_granted);
                } else {
                    fillEventVectorUpTo(time_granted);
                }
                break;
            case IterationRequest::NO_ITERATIONS:
                if (time_granted < nextTime || wait_for_current_time) {
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
            if (ret == MessageProcessingResult::NEXT_STEP) {
                auto current_clock_time = std::chrono::steady_clock::now();
                auto timegap = current_clock_time - start_clock_time;
                if (time_granted - Time(timegap) > rt_lead) {
                    auto current_lead = (time_granted - rt_lead).to_ns() - timegap;
                    if (current_lead > std::chrono::milliseconds(5)) {
                        std::this_thread::sleep_for(current_lead);
                    }
                }
            }
        } else if (grantTimeOutPeriod > timeZero) {
            mTimer->cancelTimer(grantTimeoutTimeIndex);
        }
#endif

        unlock();
        if (retTime.grantedTime > nextTime && nextTime > lastTime &&
            retTime.grantedTime < Time::maxVal()) {
            if (!ignore_time_mismatch_warnings) {
                LOG_WARNING(fmt::format(
                    "Time mismatch detected: granted time greater than requested time {} vs {}",
                    static_cast<double>(retTime.grantedTime),
                    static_cast<double>(nextTime)));
            }
        }
        return retTime;
    }
    // this would not be good practice to get into this part of the function
    // but the area must protect itself against the possibility and should return something sensible
    std::lock_guard<FederateState> fedlock(*this);
    IterationResult ret = iterating ? IterationResult::ITERATING : IterationResult::NEXT_STEP;
    if (state == HELICS_FINISHED) {
        ret = IterationResult::HALTED;
    } else if (state == HELICS_ERROR) {
        ret = IterationResult::ERROR_RESULT;
    }
    return {time_granted, ret};
}

void FederateState::fillEventVectorUpTo(Time currentTime)
{
    events.clear();
    eventMessages.clear();
    for (const auto& ipt : interfaceInformation.getInputs()) {
        bool updated = ipt->updateTimeUpTo(currentTime);
        if (updated) {
            events.push_back(ipt->id.handle);
        }
    }
    for (const auto& ept : interfaceInformation.getEndpoints()) {
        bool updated = ept->updateTimeUpTo(currentTime);
        if (updated) {
            eventMessages.push_back(ept->id.handle);
        }
    }
}

void FederateState::fillEventVectorInclusive(Time currentTime)
{
    events.clear();
    for (const auto& ipt : interfaceInformation.getInputs()) {
        bool updated = ipt->updateTimeInclusive(currentTime);
        if (updated) {
            events.push_back(ipt->id.handle);
        }
    }
    eventMessages.clear();
    for (const auto& ept : interfaceInformation.getEndpoints()) {
        bool updated = ept->updateTimeInclusive(currentTime);
        if (updated) {
            eventMessages.push_back(ept->id.handle);
        }
    }
}

void FederateState::fillEventVectorNextIteration(Time currentTime)
{
    events.clear();
    for (const auto& ipt : interfaceInformation.getInputs()) {
        bool updated = ipt->updateTimeNextIteration(currentTime);
        if (updated) {
            events.push_back(ipt->id.handle);
        }
    }
    eventMessages.clear();
    for (const auto& ept : interfaceInformation.getEndpoints()) {
        bool updated = ept->updateTimeNextIteration(currentTime);
        if (updated) {
            eventMessages.push_back(ept->id.handle);
        }
    }
}

MessageProcessingResult FederateState::genericUnspecifiedQueueProcess(bool busyReturn)
{
    if (try_lock()) {  // only 1 thread can enter this loop once per federate
        auto ret = processQueue();
        if (ret != MessageProcessingResult::USER_RETURN) {
            time_granted = timeCoord->getGrantedTime();
            allowed_send_time = timeCoord->allowedSendTime();
        }
        unlock();
        return ret;
    }

    if (busyReturn) {
        return MessageProcessingResult::BUSY;
    }
    sleeplock();
    MessageProcessingResult ret;
    switch (getState()) {
        case HELICS_ERROR:
            ret = MessageProcessingResult::ERROR_RESULT;
            break;
        case HELICS_FINISHED:
            ret = MessageProcessingResult::HALTED;
            break;
        default:  // everything >= HELICS_INITIALIZING
            ret = MessageProcessingResult::NEXT_STEP;
            break;
    }
    unlock();
    return ret;
}

void FederateState::finalize()
{
    if ((state == FederateStates::HELICS_FINISHED) || (state == FederateStates::HELICS_ERROR)) {
        return;
    }
    auto ret = MessageProcessingResult::NEXT_STEP;
#ifndef HELICS_DISABLE_ASIO
    if (grantTimeOutPeriod > timeZero) {
        ActionMessage grantCheck(CMD_GRANT_TIMEOUT_CHECK);
        grantCheck.setExtraData(static_cast<std::int32_t>(mGrantCount));
        grantCheck.counter = 0;
        grantCheck.actionTime = Time::maxVal();
        if (grantTimeoutTimeIndex < 0) {
            grantTimeoutTimeIndex =
                mTimer->addTimerFromNow(grantTimeOutPeriod.to_ms(), std::move(grantCheck));
        } else {
            mTimer->updateTimerFromNow(realTimeTimerIndex,
                                       grantTimeOutPeriod.to_ms(),
                                       std::move(grantCheck));
        }
    }
#endif
    while (ret != MessageProcessingResult::HALTED) {
        ret = genericUnspecifiedQueueProcess(false);
        if (ret == MessageProcessingResult::ERROR_RESULT) {
            break;
        }
    }
#ifndef HELICS_DISABLE_ASIO
    ++mGrantCount;
    if (grantTimeOutPeriod > timeZero) {
        mTimer->cancelTimer(grantTimeoutTimeIndex);
    }
#endif
}

void FederateState::processCommunications(std::chrono::milliseconds period)
{
    ActionMessage treq(CMD_USER_RETURN);
    treq.source_id = global_id.load();
    // the user return should only be for this thread, other threads will ignore it
    treq.messageID = static_cast<int32_t>(std::hash<std::thread::id>{}(std::this_thread::get_id()));
    addAction(treq);
    auto starttime = std::chrono::steady_clock::now();
    auto ret = MessageProcessingResult::CONTINUE_PROCESSING;
    while (ret != MessageProcessingResult::USER_RETURN) {
        ret = genericUnspecifiedQueueProcess(true);
        if (ret == MessageProcessingResult::BUSY) {
            return;
        }
    }
    if (period >= std::chrono::milliseconds(10)) {
        auto ctime = std::chrono::steady_clock::now();
        if (period - (ctime - starttime) > std::chrono::milliseconds(10)) {
            std::this_thread::sleep_for(period - (ctime - starttime));
            processCommunications(std::chrono::milliseconds(0));
        }
    }
}

const std::vector<InterfaceHandle> emptyHandles;

const std::vector<InterfaceHandle>& FederateState::getEvents() const
{
    return events;
}

MessageProcessingResult FederateState::processDelayQueue() noexcept
{
    delayedFederates.clear();
    auto ret_code = MessageProcessingResult::CONTINUE_PROCESSING;
    if (!delayQueues.empty()) {
        for (auto& dQ : delayQueues) {
            auto& tempQueue = dQ.second;
            ret_code = MessageProcessingResult::CONTINUE_PROCESSING;
            // we specifically want to stop the loop on a delay_message return
            while ((ret_code == MessageProcessingResult::CONTINUE_PROCESSING) &&
                   (!tempQueue.empty())) {
                auto& cmd = tempQueue.front();
                if (messageShouldBeDelayed(cmd)) {
                    ret_code = MessageProcessingResult::DELAY_MESSAGE;
                    continue;
                }

                ret_code = processActionMessage(cmd);
                if (ret_code == MessageProcessingResult::DELAY_MESSAGE) {
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

void FederateState::addFederateToDelay(GlobalFederateId id)
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
            return ((cmd.source_id == delayedFederates.front()) ||
                    (cmd.source_id == delayedFederates.back()));
        default: {
            auto res =
                std::lower_bound(delayedFederates.begin(), delayedFederates.end(), cmd.source_id);
            return ((res != delayedFederates.end()) && (*res == cmd.source_id));
        }
    }
}

void FederateState::generateProfilingMarker()
{
    auto ctime = std::chrono::steady_clock::now();
    auto gtime = std::chrono::system_clock::now();
    std::string message = fmt::format(
        "<PROFILING>{}[{}]({})MARKER<{}|{}>[t={}]</PROFILING>",
        name,
        global_id.load().baseValue(),
        fedStateString(getState()),
        std::chrono::duration_cast<std::chrono::nanoseconds>(ctime.time_since_epoch()).count(),
        std::chrono::duration_cast<std::chrono::nanoseconds>(gtime.time_since_epoch()).count(),
        static_cast<double>(time_granted));

    if (mLocalProfileCapture) {
        logMessage(HELICS_LOG_LEVEL_PROFILING, name, message);
    } else {
        if (parent_ != nullptr) {
            ActionMessage prof(CMD_PROFILER_DATA, global_id.load(), parent_broker_id);
            prof.payload = message;
            parent_->addActionMessage(std::move(prof));
        }
    }
}

void FederateState::generateProfilingMessage(bool enterHelicsCode)
{
    auto ctime = std::chrono::steady_clock::now();
    static constexpr std::string_view entry_string("ENTRY");
    static constexpr std::string_view exit_string("EXIT");
    std::string message = fmt::format(
        "<PROFILING>{}[{}]({})HELICS CODE {}<{}>[t={}]</PROFILING>",
        name,
        global_id.load().baseValue(),
        fedStateString(getState()),
        (enterHelicsCode ? entry_string : exit_string),
        std::chrono::duration_cast<std::chrono::nanoseconds>(ctime.time_since_epoch()).count(),
        static_cast<double>(time_granted));
    if (mLocalProfileCapture) {
        logMessage(HELICS_LOG_LEVEL_PROFILING, name, message);
    } else {
        if (parent_ != nullptr) {
            ActionMessage prof(CMD_PROFILER_DATA, global_id.load(), parent_broker_id);
            prof.payload = message;
            parent_->addActionMessage(std::move(prof));
        }
    }
}

MessageProcessingResult FederateState::processQueue() noexcept
{
    if (state == HELICS_FINISHED) {
        return MessageProcessingResult::HALTED;
    }
    auto initError = (state == HELICS_ERROR);
    bool error_cmd{false};
    bool profilerActive{mProfilerActive};
    if (profilerActive) {
        generateProfilingMessage(true);
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
        if (ret_code == MessageProcessingResult::DELAY_MESSAGE) {
            delayQueues[static_cast<GlobalFederateId>(cmd.source_id)].push_back(cmd);
        }
        if (ret_code == MessageProcessingResult::ERROR_RESULT && cmd.action() == CMD_GLOBAL_ERROR) {
            error_cmd = true;
        }
    }
    if (ret_code == MessageProcessingResult::ERROR_RESULT && state == HELICS_ERROR) {
        if (!initError && !error_cmd) {
            if (parent_ != nullptr) {
                ActionMessage gError(CMD_LOCAL_ERROR);
                if (terminate_on_error) {
                    gError.setAction(CMD_GLOBAL_ERROR);
                } else {
                    timeCoord->localError();
                }
                gError.source_id = global_id.load();
                gError.dest_id = parent_broker_id;
                gError.messageID = errorCode;
                gError.payload = errorString;

                parent_->addActionMessage(std::move(gError));
            }
        }
    }
    if (initError) {
        ret_code = MessageProcessingResult::ERROR_RESULT;
    }
    if (profilerActive) {
        generateProfilingMessage(false);
    }
    return ret_code;
}

MessageProcessingResult FederateState::processActionMessage(ActionMessage& cmd)
{
    LOG_TRACE(fmt::format("processing command {}", prettyPrintString(cmd)));

    if (cmd.action() == CMD_TIME_REQUEST) {
        if ((cmd.source_id == global_id.load()) &&
            checkActionFlag(cmd, indicator_flag)) {  // this sets up a time request
            requestingMode.store(true);
            IterationRequest iterate = IterationRequest::NO_ITERATIONS;
            if (checkActionFlag(cmd, iteration_requested_flag)) {
                iterate = (checkActionFlag(cmd, required_flag)) ?
                    IterationRequest::FORCE_ITERATION :
                    IterationRequest::ITERATE_IF_NEEDED;
            }
            timeCoord->timeRequest(cmd.actionTime, iterate, nextValueTime(), nextMessageTime());
            timeGranted_mode = false;
            auto ret = processDelayQueue();
            if (returnableResult(ret)) {
                return ret;
            }
            cmd.setAction(CMD_TIME_CHECK);
        }
    }
    auto proc_result = processCoordinatorMessage(
        cmd, timeCoord.get(), getState(), timeGranted_mode, global_id.load());

    timeGranted_mode = std::get<2>(proc_result);

    if (getState() != std::get<0>(proc_result)) {
        setState(std::get<0>(proc_result));
        switch (std::get<0>(proc_result)) {
            case HELICS_INITIALIZING:
                LOG_TIMING("Granting Initialization");
                if (checkInterfaces() != defs::Errors::OK) {
                    setState(HELICS_ERROR);
                    return MessageProcessingResult::ERROR_RESULT;
                }
                timeCoord->enterInitialization();
                break;
            case HELICS_EXECUTING:
                timeCoord->updateTimeFactors();
                LOG_TIMING("Granting Execution");
                break;
            case HELICS_FINISHED:
                LOG_TIMING("Terminating");
                break;
            case HELICS_ERROR:
                if (cmd.payload.empty()) {
                    errorString = commandErrorString(cmd.messageID);
                    if (errorString == "unknown") {
                        errorString += " code:" + std::to_string(cmd.messageID);
                    }
                } else {
                    errorString = cmd.payload.to_string();
                }
                errorCode = cmd.messageID;
                LOG_ERROR(errorString);
                break;
            default:
                break;
        }
    }

    switch (std::get<1>(proc_result)) {
        case MessageProcessingResult::CONTINUE_PROCESSING:
            break;
        case MessageProcessingResult::REPROCESS_MESSAGE:
            if (cmd.dest_id != global_id.load()) {
                routeMessage(cmd);
                return MessageProcessingResult::CONTINUE_PROCESSING;
            }
            return processActionMessage(cmd);
        case MessageProcessingResult::DELAY_MESSAGE:
            addFederateToDelay(GlobalFederateId(cmd.source_id));
            return MessageProcessingResult::DELAY_MESSAGE;
        default:
            if (timeGranted_mode) {
                time_granted = timeCoord->getGrantedTime();
                allowed_send_time = timeCoord->allowedSendTime();
                if (cmd.action() == CMD_FORCE_TIME_GRANT) {
                    if (!ignore_time_mismatch_warnings) {
                        LOG_WARNING(fmt::format("forced Granted Time={}", time_granted));
                    }
                } else {
                    LOG_TIMING(fmt::format("Granted Time={}", time_granted));
                }
            }
            return (std::get<1>(proc_result));
    }

    switch (cmd.action()) {
        case CMD_IGNORE:
        default:
            break;
        case CMD_LOG:
        case CMD_REMOTE_LOG:
            logMessage(cmd.messageID,
                       cmd.getString(0),
                       cmd.payload.to_string(),
                       cmd.action() == CMD_REMOTE_LOG);
            break;

        case CMD_EXEC_REQUEST:
            if ((cmd.source_id == global_id.load()) &&
                checkActionFlag(cmd, indicator_flag)) {  // this sets up a time request
                auto ret = processDelayQueue();
                if (returnableResult(ret)) {
                    return ret;
                }
                cmd.setAction(CMD_EXEC_CHECK);
                return processActionMessage(cmd);
            }
            break;
        case CMD_GLOBAL_DISCONNECT:
        case CMD_USER_DISCONNECT:
            if ((state != HELICS_FINISHED) && (state != HELICS_TERMINATING)) {
                timeCoord->disconnect();
                cmd.dest_id = parent_broker_id;
                if (state != HELICS_ERROR) {
                    setState(HELICS_TERMINATING);
                }
                routeMessage(cmd);
            }
            break;
        case CMD_DISCONNECT_FED:
        case CMD_DISCONNECT:
            if (cmd.source_id == global_id.load()) {
                if ((state != HELICS_FINISHED) && (state != HELICS_TERMINATING)) {
                    timeCoord->disconnect();
                    cmd.dest_id = parent_broker_id;
                    if (state != HELICS_ERROR) {
                        setState(HELICS_TERMINATING);
                    }
                    routeMessage(cmd);
                }
            } else {
                switch (timeCoord->processTimeMessage(cmd)) {
                    case message_process_result::delay_processing:
                        addFederateToDelay(GlobalFederateId(cmd.source_id));
                        return MessageProcessingResult::DELAY_MESSAGE;
                    case message_process_result::no_effect:
                        return MessageProcessingResult::CONTINUE_PROCESSING;
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
        case CMD_CLOSE_INTERFACE:
            if (cmd.source_id == global_id.load()) {
                closeInterface(cmd.source_handle, static_cast<InterfaceType>(cmd.counter));
            }
            break;

        case CMD_SEND_MESSAGE: {
            auto* epi = interfaceInformation.getEndpoint(cmd.dest_handle);
            if (epi != nullptr) {
                // if (!epi->not_interruptible)
                {
                    timeCoord->updateMessageTime(cmd.actionTime, !timeGranted_mode);
                }
                LOG_DATA(fmt::format("receive_message {}", prettyPrintString(cmd)));
                if (cmd.actionTime < time_granted) {
                    LOG_WARNING(
                        fmt::format("received message {} at time({}) earlier than granted time({})",
                                    prettyPrintString(cmd),
                                    cmd.actionTime,
                                    time_granted));
                    auto qres = processQueryActual("global_time_debugging");
                    qres.insert(0, "TIME DEBUGGING::");
                    LOG_WARNING(qres);
                }

                if (state <= HELICS_EXECUTING) {
                    timeCoord->processTimeMessage(cmd);
                }
                epi->addMessage(createMessageFromCommand(std::move(cmd)));
            }
        } break;
        case CMD_PUB: {
            auto* subI = interfaceInformation.getInput(InterfaceHandle(cmd.dest_handle));
            if (subI == nullptr) {
                auto* eptI = interfaceInformation.getEndpoint(cmd.dest_handle);
                if (eptI != nullptr) {
                    // if (!epi->not_interruptible)
                    {
                        timeCoord->updateMessageTime(cmd.actionTime, !timeGranted_mode);
                    }
                    LOG_DATA(fmt::format("receive_message {}", prettyPrintString(cmd)));
                    if (cmd.actionTime < time_granted) {
                        LOG_WARNING(fmt::format(
                            "received message {} at time({}) earlier than granted time({})",
                            prettyPrintString(cmd),
                            cmd.actionTime,
                            time_granted));
                    }
                    auto mess = std::make_unique<Message>();
                    mess->data = std::move(cmd.payload);
                    mess->dest = eptI->key;
                    mess->flags = cmd.flags;
                    mess->time = cmd.actionTime;
                    mess->counter = cmd.counter;
                    mess->messageID = cmd.messageID;
                    mess->original_dest = eptI->key;
                    eptI->addMessage(std::move(mess));
                    if (state <= HELICS_EXECUTING) {
                        timeCoord->processTimeMessage(cmd);
                    }
                }
                break;
            }
            for (auto& src : subI->input_sources) {
                if ((cmd.source_id == src.fed_id) && (cmd.source_handle == src.handle)) {
                    subI->addData(src,
                                  cmd.actionTime,
                                  cmd.counter,
                                  std::make_shared<const SmallBuffer>(std::move(cmd.payload)));
                    if (!subI->not_interruptible) {
                        timeCoord->updateValueTime(cmd.actionTime, !timeGranted_mode);
                        LOG_TRACE(timeCoord->printTimeStatus());
                    }
                    LOG_DATA(fmt::format("receive PUBLICATION {} from {}",
                                         prettyPrintString(cmd),
                                         subI->getSourceName(src)));
                }
            }
            if (state <= HELICS_EXECUTING) {
                timeCoord->processTimeMessage(cmd);
            }
        } break;
        case CMD_WARNING:
            if (cmd.payload.empty()) {
                cmd.payload = commandErrorString(cmd.messageID);
                if (cmd.payload.to_string() == "unknown") {
                    cmd.payload.append(" code:");
                    cmd.payload.append(std::to_string(cmd.messageID));
                }
            }
            LOG_WARNING(cmd.payload.to_string());
            break;
        case CMD_GRANT_TIMEOUT_CHECK:
            if (timeGranted_mode && cmd.actionTime != Time::maxVal()) {
                break;
            }
            if (mGrantCount != static_cast<std::uint32_t>(cmd.getExtraData())) {
                // time has been granted since this was triggered
                break;
            }
            if (cmd.counter == 0) {
                auto blockFed = timeCoord->getMinGrantedDependency();
                if (blockFed.first.isValid()) {
                    LOG_WARNING(fmt::format("grant timeout exceeded sim time {} waiting on {}",
                                            time_granted,
                                            blockFed.first.baseValue()));
                } else {
                    LOG_WARNING(fmt::format("grant timeout exceeded sim time {}", time_granted));
                }

            } else if (cmd.counter == 3) {
                LOG_WARNING("grant timeout stage 2 requesting time resend");
                timeCoord->requestTimeCheck();
            } else if (cmd.counter == 6) {
                LOG_WARNING("grant timeout stage 3 diagnostics");
                auto qres = processQueryActual("global_time_debugging");
                qres.insert(0, "TIME DEBUGGING::");
                LOG_WARNING(qres);
                auto parentID = timeCoord->getParent();
                if (parentID.isValid()) {
                    auto brokerTimeoutCheck = cmd;
                    brokerTimeoutCheck.source_id = global_id.load();
                    brokerTimeoutCheck.dest_id = parentID;
                    routeMessage(brokerTimeoutCheck);
                    LOG_WARNING(
                        fmt::format("sending grant time out check to {}", parentID.baseValue()));
                }
            } else if (cmd.counter == 10) {
                if (cmd.actionTime == Time::maxVal()) {
                    LOG_WARNING("finalize blocking");
                } else {
                    LOG_WARNING("grant timeout stage 4 error actions (none available)");
                }
            }
#ifndef HELICS_DISABLE_ASIO
            if (mTimer) {
                ++cmd.counter;
                mTimer->updateTimerFromNow(grantTimeoutTimeIndex,
                                           grantTimeOutPeriod.to_ms(),
                                           std::move(cmd));
            }
#endif
            break;
        case CMD_ADD_PUBLISHER: {
            auto* subI = interfaceInformation.getInput(cmd.dest_handle);
            if (subI != nullptr) {
                if (subI->addSource(cmd.getSource(),
                                    std::string(cmd.name()),
                                    cmd.getString(typeStringLoc),
                                    cmd.getString(unitStringLoc))) {
                    if (!usingGlobalTime) {
                        addDependency(cmd.source_id);
                    }
                }
            } else {
                auto* eptI = interfaceInformation.getEndpoint(cmd.dest_handle);
                if (eptI != nullptr) {
                    eptI->addSourceTarget(cmd.getSource(),
                                          std::string(cmd.name()),
                                          cmd.getString(typeStringLoc));
                    if (!usingGlobalTime) {
                        addDependency(cmd.source_id);
                    }
                }
            }
        } break;
        case CMD_ADD_SUBSCRIBER: {
            auto* pubI = interfaceInformation.getPublication(cmd.dest_handle);
            if (pubI != nullptr) {
                if (pubI->addSubscriber(cmd.getSource())) {
                    if (!usingGlobalTime) {
                        addDependent(cmd.source_id);
                    }
                }
            }
        } break;
        case CMD_ADD_ENDPOINT: {
            auto* eptI = interfaceInformation.getEndpoint(cmd.dest_handle);
            if (eptI != nullptr) {
                if (checkActionFlag(cmd, destination_target)) {
                    eptI->addDestinationTarget(cmd.getSource(),
                                               std::string(cmd.name()),
                                               cmd.getString(typeStringLoc));
                } else {
                    eptI->addSourceTarget(cmd.getSource(),
                                          std::string(cmd.name()),
                                          cmd.getString(typeStringLoc));
                }
            }
        } break;
        case CMD_REMOVE_NAMED_PUBLICATION: {
            auto* subI = interfaceInformation.getInput(cmd.source_handle);
            if (subI != nullptr) {
                subI->removeSource(std::string(cmd.name()),
                                   (cmd.actionTime != timeZero) ? cmd.actionTime : time_granted);
            }
            break;
        }
        case CMD_REMOVE_PUBLICATION: {
            auto* subI = interfaceInformation.getInput(cmd.dest_handle);
            if (subI != nullptr) {
                subI->removeSource(cmd.getSource(),
                                   (cmd.actionTime != timeZero) ? cmd.actionTime : time_granted);
            }
            break;
        }
        case CMD_REMOVE_SUBSCRIBER: {
            auto* pubI = interfaceInformation.getPublication(cmd.dest_handle);
            if (pubI != nullptr) {
                pubI->removeSubscriber(cmd.getSource());
            }
        } break;
        case CMD_REMOVE_ENDPOINT:
            break;
        case CMD_SET_PROFILER_FLAG:
            setOptionFlag(defs::PROFILING, checkActionFlag(cmd, indicator_flag));
            break;
        case CMD_FED_ACK:
            if (state != HELICS_CREATED) {
                break;
            }
            if (cmd.name() == name) {
                if (checkActionFlag(cmd, error_flag)) {
                    setState(HELICS_ERROR);
                    errorString = commandErrorString(cmd.messageID);
                    return MessageProcessingResult::ERROR_RESULT;
                }
                if (checkActionFlag(cmd, indicator_flag)) {
                    usingGlobalTime = true;
                    addDependent(gRootBrokerID);
                    addDependency(gRootBrokerID);
                    timeCoord->setAsParent(gRootBrokerID);
                    timeCoord->globalTime = true;
                }
                global_id = cmd.dest_id;
                interfaceInformation.setGlobalId(cmd.dest_id);
                timeCoord->setSourceId(global_id);
                return MessageProcessingResult::NEXT_STEP;
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
        case CMD_SEND_COMMAND:
        case CMD_SEND_COMMAND_ORDERED:
            sendCommand(cmd);
            break;
        case CMD_TIMEOUT_DISCONNECT: {
            auto qres = processQueryActual("global_time_debugging");
            qres.insert(0, "TIME DEBUGGING::");
            LOG_WARNING(qres);
        } break;
        case CMD_QUERY_ORDERED:
        case CMD_QUERY: {
            std::string repStr;
            ActionMessage queryResp(cmd.action() == CMD_QUERY ? CMD_QUERY_REPLY :
                                                                CMD_QUERY_REPLY_ORDERED);
            queryResp.dest_id = cmd.source_id;
            queryResp.source_id = cmd.dest_id;
            queryResp.messageID = cmd.messageID;
            queryResp.counter = cmd.counter;

            queryResp.payload = processQueryActual(cmd.payload.to_string());
            routeMessage(queryResp);
        } break;
    }
    return MessageProcessingResult::CONTINUE_PROCESSING;
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
            used = interfaceInformation.setInputProperty(cmd.dest_handle,
                                                         cmd.messageID,
                                                         checkActionFlag(cmd, indicator_flag) ?
                                                             cmd.getExtraDestData() :
                                                             0);
            if (!used) {
                auto* ipt = interfaceInformation.getInput(cmd.dest_handle);
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
            used =
                interfaceInformation.setPublicationProperty(cmd.dest_handle,
                                                            cmd.messageID,
                                                            checkActionFlag(cmd, indicator_flag) ?
                                                                cmd.getExtraDestData() :
                                                                0);
            if (!used) {
                auto* pub = interfaceInformation.getPublication(cmd.dest_handle);
                if (pub != nullptr) {
                    LOG_WARNING(fmt::format("property {} not used on publication {}",
                                            cmd.messageID,
                                            pub->key));
                } else {
                    LOG_WARNING(fmt::format("property {} not used on due to unknown publication",
                                            cmd.messageID));
                }
            }
            break;
        case 'e':
            used = interfaceInformation.setEndpointProperty(cmd.dest_handle,
                                                            cmd.messageID,
                                                            checkActionFlag(cmd, indicator_flag) ?
                                                                cmd.getExtraDestData() :
                                                                0);
            if (!used) {
                auto* ept = interfaceInformation.getEndpoint(cmd.dest_handle);
                if (ept != nullptr) {
                    LOG_WARNING(fmt::format("property {} not used on endpoint {}",
                                            cmd.messageID,
                                            ept->key));
                } else {
                    LOG_WARNING(fmt::format("property {} not used on due to unknown endpoint",
                                            cmd.messageID));
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
        case defs::Properties::RT_LAG:
            rt_lag = propertyVal;
            break;
        case defs::Properties::RT_LEAD:
            rt_lead = propertyVal;
            break;
        case defs::Properties::RT_TOLERANCE:
            rt_lag = propertyVal;
            rt_lead = propertyVal;
            break;
        case defs::Properties::GRANT_TIMEOUT: {
#ifndef HELICS_DISABLE_ASIO
            auto prevTimeout = grantTimeOutPeriod;
            grantTimeOutPeriod = propertyVal;
            if (prevTimeout == timeZero) {
                if (getState() >= HELICS_INITIALIZING && grantTimeOutPeriod > timeZero) {
                    if (!mTimer) {
                        if (!mTimer) {
                            mTimer = std::make_shared<MessageTimer>([this](ActionMessage&& mess) {
                                return this->addAction(std::move(mess));
                            });
                        }
                    }
                }
                // if we are currently waiting for a grant trigger the timer
                if (getState() == HELICS_EXECUTING && !timeGranted_mode) {
                    ActionMessage grantCheck(CMD_GRANT_TIMEOUT_CHECK);
                    grantCheck.setExtraData(static_cast<std::int32_t>(mGrantCount));
                    grantCheck.counter = 0;
                    if (grantTimeoutTimeIndex < 0) {
                        grantTimeoutTimeIndex = mTimer->addTimerFromNow(grantTimeOutPeriod.to_ms(),
                                                                        std::move(grantCheck));
                    }
                }
            } else if (grantTimeOutPeriod <= timeZero && grantTimeoutTimeIndex >= 0) {
                mTimer->cancelTimer(grantTimeoutTimeIndex);
            }
#else
            grantTimeOutPeriod = propertyVal;
#endif
        } break;
        default:
            timeCoord->setProperty(timeProperty, propertyVal);
            break;
    }
}

/** set a timeProperty for a the coordinator*/
void FederateState::setProperty(int intProperty, int propertyVal)
{
    switch (intProperty) {
        case defs::Properties::LOG_LEVEL:
        case defs::Properties::FILE_LOG_LEVEL:
        case defs::Properties::CONSOLE_LOG_LEVEL:
            mLogManager->setLogLevel(propertyVal);
            maxLogLevel = mLogManager->getMaxLevel();
            break;
        case defs::Properties::RT_LAG:
            rt_lag = helics::Time(static_cast<double>(propertyVal));
            break;
        case defs::Properties::RT_LEAD:
            rt_lead = helics::Time(static_cast<double>(propertyVal));
            break;
        case defs::Properties::RT_TOLERANCE:
            rt_lag = helics::Time(static_cast<double>(propertyVal));
            rt_lead = rt_lag;
            break;
        case defs::Properties::LOG_BUFFER:
            mLogManager->getLogBuffer().resize(
                (propertyVal <= 0) ? 0UL : static_cast<std::size_t>(propertyVal));
            break;
        default:
            timeCoord->setProperty(intProperty, propertyVal);
    }
}

/** set an option Flag for a the coordinator*/
void FederateState::setOptionFlag(int optionFlag, bool value)
{
    switch (optionFlag) {
        case defs::Flags::ONLY_TRANSMIT_ON_CHANGE:
        case defs::Options::HANDLE_ONLY_TRANSMIT_ON_CHANGE:
            only_transmit_on_change = value;
            break;
        case defs::Flags::ONLY_UPDATE_ON_CHANGE:
        case defs::Options::HANDLE_ONLY_UPDATE_ON_CHANGE:
            interfaceInformation.setChangeUpdateFlag(value);
            break;
        case defs::Flags::STRICT_INPUT_TYPE_CHECKING:
            strict_input_type_checking = value;
            break;
        case defs::Flags::IGNORE_INPUT_UNIT_MISMATCH:
            ignore_unit_mismatch = value;
            break;
        case defs::Flags::SLOW_RESPONDING:
        case defs::Flags::DEBUGGING:
            slow_responding = value;
            break;
        case defs::Flags::PROFILING:
            if (value && !mProfilerActive) {
                generateProfilingMarker();
            }
            mProfilerActive = value;
            break;
        case defs::Flags::PROFILING_MARKER:
            if (value && mProfilerActive) {
                generateProfilingMarker();
            }
            break;
        case defs::Flags::LOCAL_PROFILING_CAPTURE:
            mLocalProfileCapture = value;
            break;
        case defs::Flags::TERMINATE_ON_ERROR:
            terminate_on_error = value;
            break;
        case defs::Flags::REALTIME:
            if (value) {
                if (state < HELICS_EXECUTING) {
                    realtime = true;
                }
            } else {
                realtime = false;
            }

            break;
        case defs::Flags::SOURCE_ONLY:
            if (state == HELICS_CREATED) {
                source_only = value;
                if (value) {
                    observer = false;
                }
            }
            break;
        case defs::Flags::OBSERVER:
            if (state == HELICS_CREATED) {
                observer = value;
                if (value) {
                    source_only = false;
                }
            }
            break;
        case defs::Flags::IGNORE_TIME_MISMATCH_WARNINGS:
            ignore_time_mismatch_warnings = value;
            break;
        case defs::Flags::WAIT_FOR_CURRENT_TIME_UPDATE:
            // this flag is needed in both locations
            wait_for_current_time = value;
            timeCoord->setOptionFlag(optionFlag, value);
            break;
        case defs::Options::BUFFER_DATA:
            break;
        case defs::Flags::CONNECTIONS_REQUIRED:
            if (value) {
                interfaceFlags |= make_flags(required_flag);
            } else {
                interfaceFlags &= ~(make_flags(required_flag));
            }
            break;
        case defs::Flags::CONNECTIONS_OPTIONAL:
            if (value) {
                interfaceFlags |= make_flags(optional_flag);
            } else {
                interfaceFlags &= ~(make_flags(optional_flag));
            }
            break;
        case defs::Properties::LOG_BUFFER:
            mLogManager->getLogBuffer().enable(value);
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
        case defs::Properties::RT_LAG:
        case defs::Properties::RT_TOLERANCE:
            return rt_lag;
        case defs::Properties::RT_LEAD:
            return rt_lead;
        case defs::Properties::GRANT_TIMEOUT:
            return grantTimeOutPeriod;
        default:
            return timeCoord->getTimeProperty(timeProperty);
    }
}

/** get an option flag value*/
bool FederateState::getOptionFlag(int optionFlag) const
{
    switch (optionFlag) {
        case defs::Flags::ONLY_TRANSMIT_ON_CHANGE:
        case defs::Options::HANDLE_ONLY_TRANSMIT_ON_CHANGE:
            return only_transmit_on_change;
        case defs::Flags::ONLY_UPDATE_ON_CHANGE:
        case defs::Options::HANDLE_ONLY_UPDATE_ON_CHANGE:
            return interfaceInformation.getChangeUpdateFlag();
        case defs::Flags::REALTIME:
            return realtime;
        case defs::Flags::OBSERVER:
            return observer;
        case defs::Flags::SOURCE_ONLY:
            return source_only;
        case defs::Flags::SLOW_RESPONDING:
        case defs::Flags::DEBUGGING:
            return slow_responding;
        case defs::Flags::TERMINATE_ON_ERROR:
            return terminate_on_error;
        case defs::Flags::CONNECTIONS_REQUIRED:
            return ((interfaceFlags.load() & make_flags(required_flag)) != 0);
        case defs::Flags::CONNECTIONS_OPTIONAL:
            return ((interfaceFlags.load() & make_flags(optional_flag)) != 0);
        case defs::Flags::STRICT_INPUT_TYPE_CHECKING:
            return strict_input_type_checking;
        case defs::Flags::IGNORE_INPUT_UNIT_MISMATCH:
            return ignore_unit_mismatch;
        case defs::Flags::IGNORE_TIME_MISMATCH_WARNINGS:
            return ignore_time_mismatch_warnings;
        case defs::Properties::LOG_BUFFER:
            return (mLogManager->getLogBuffer().capacity() > 0);
        default:
            return timeCoord->getOptionFlag(optionFlag);
    }
}

int32_t FederateState::getHandleOption(InterfaceHandle handle, char iType, int32_t option) const
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
    return 0;
}

/** get an option flag value*/
int FederateState::getIntegerProperty(int intProperty) const
{
    switch (intProperty) {
        case defs::Properties::LOG_LEVEL:
        case defs::Properties::FILE_LOG_LEVEL:
        case defs::Properties::CONSOLE_LOG_LEVEL:
            return mLogManager->getConsoleLevel();
        case defs::Properties::LOG_BUFFER:
            return static_cast<int>(mLogManager->getLogBuffer().capacity());
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

std::vector<GlobalFederateId> FederateState::getDependencies() const
{
    return timeCoord->getDependencies();
}

std::vector<GlobalFederateId> FederateState::getDependents() const
{
    return timeCoord->getDependents();
}

void FederateState::addDependency(GlobalFederateId fedToDependOn)
{
    timeCoord->addDependency(fedToDependOn);
}

void FederateState::addDependent(GlobalFederateId fedThatDependsOnThis)
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
            case defs::Errors::CONNECTION_FAILURE:
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
    for (const auto& inp : interfaceInformation.getInputs()) {
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
    for (const auto& ep : interfaceInformation.getEndpoints()) {
        auto messageTime = ep->firstMessageTime();
        if (messageTime < time_granted) {
            messageTime = time_granted;
        }
        if (messageTime < firstMessageTime) {
            firstMessageTime = messageTime;
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

void FederateState::logMessage(int level,
                               std::string_view logMessageSource,
                               std::string_view message,
                               bool fromRemote) const
{
    if (level > maxLogLevel && !fromRemote) {
        return;
    }
    std::string header;
    auto t = grantedTime();
    std::string timeString;
    if (t < timeZero) {
        timeString = fmt::format("[{}]", fedStateString(getState()));
    } else if (t == Time::maxVal()) {
        timeString = "[MAXTIME]";
    } else {
        timeString = fmt::format("[{}]", static_cast<double>(grantedTime()));
    }
    if (logMessageSource.empty()) {
        header = fmt::format("{} ({}){}", name, global_id.load().baseValue(), timeString);
    } else if (logMessageSource.back() == ']') {
        header = logMessageSource;
    } else {
        header = fmt::format("{}{}", logMessageSource, timeString);
    }

    mLogManager->sendToLogger(level, header, message, fromRemote);
}

const std::string& fedStateString(FederateStates state)
{
    static const std::string c1{"created"};
    static const std::string estate{"error"};
    static const std::string init{"initializing"};
    static const std::string dis{"disconnected"};
    static const std::string exec{"executing"};
    static const std::string term{"terminating"};
    static const std::string unk{"unknown"};

    switch (state) {
        case FederateStates::HELICS_CREATED:
            return c1;
        case FederateStates::HELICS_INITIALIZING:
            return init;
        case FederateStates::HELICS_EXECUTING:
            return exec;
        case FederateStates::HELICS_TERMINATING:
            return term;
        case FederateStates::HELICS_FINISHED:
            return dis;
        case FederateStates::HELICS_ERROR:
            return estate;
        case FederateStates::HELICS_UNKNOWN:
        default:
            return unk;
    }
}

void FederateState::sendCommand(ActionMessage& command)
{
    auto cmd = command.payload.to_string();
    auto commentLoc = cmd.find('#');
    if (commentLoc != std::string_view::npos) {
        cmd = cmd.substr(0, commentLoc - 1);
    }
    gmlc::utilities::string_viewOps::trimString(cmd);
    auto res = gmlc::utilities::string_viewOps::splitlineQuotes(
        cmd,
        " ",
        gmlc::utilities::string_viewOps::default_quote_chars,
        gmlc::utilities::string_viewOps::delimiter_compression::on);
    if (res.empty()) {
        return;
    }

    if (res[0] == "terminate") {
        if (parent_ != nullptr) {
            ActionMessage bye(CMD_DISCONNECT);
            bye.source_id = global_id.load();
            bye.dest_id = bye.source_id;
            parent_->addActionMessage(bye);
        }
    } else if (res[0] == "echo") {
        if (parent_ != nullptr) {
            ActionMessage response(command.action());
            response.payload = "echo_reply";
            response.dest_id = command.source_id;
            response.source_id = global_id.load();
            response.setString(targetStringLoc, command.getString(sourceStringLoc));
            response.setString(sourceStringLoc, getIdentifier());

            parent_->addActionMessage(response);
        }
    } else if (res[0] == "command_status") {
        if (parent_ != nullptr) {
            ActionMessage response(command.action());
            response.payload = fmt::format("\"{} unprocessed commands\"", commandQueue.size());
            response.dest_id = command.source_id;
            response.source_id = global_id.load();
            response.setString(targetStringLoc, command.getString(sourceStringLoc));
            response.setString(sourceStringLoc, getIdentifier());

            parent_->addActionMessage(response);
        }
    } else if (res[0] == "logbuffer") {
        if (res.size() > 1) {
            if (res[1] == "stop") {
                mLogManager->getLogBuffer().enable(false);
            } else {
                mLogManager->getLogBuffer().resize(gmlc::utilities::numeric_conversion<std::size_t>(
                    res[1], LogBuffer::cDefaultBufferSize));
            }
        } else {
            mLogManager->getLogBuffer().enable(true);
        }
    } else if (res[0] == "remotelog") {
        if (res.size() > 1) {
            if (res[1] == "stop") {
                mLogManager->updateRemote(command.source_id, HELICS_LOG_LEVEL_NO_PRINT);
            } else {
                int newLevel{HELICS_LOG_LEVEL_NO_PRINT};
                if (isdigit(res[1][0]) != 0) {
                    newLevel =
                        gmlc::utilities::numeric_conversion<int>(res[1],
                                                                 mLogManager->getConsoleLevel());
                } else {
                    newLevel = logLevelFromString(res[1]);
                }
                mLogManager->updateRemote(command.source_id, newLevel);
            }
        } else {
            mLogManager->updateRemote(command.source_id, mLogManager->getConsoleLevel());
        }
        maxLogLevel = mLogManager->getMaxLevel();
    } else if (res[0] == "timeout_monitor") {
        setProperty(defs::Properties::GRANT_TIMEOUT, command.actionTime);
    } else if (res[0] == "log") {
        logMessage(HELICS_LOG_LEVEL_SUMMARY,
                   command.getString(sourceStringLoc),
                   command.payload.to_string().substr(4));
    } else {
        commandQueue.emplace(cmd, command.getString(sourceStringLoc));
    }
}

std::pair<std::string, std::string> FederateState::getCommand()
{
    auto val = commandQueue.try_pop();
    while (val && val->first == "notify") {
        if (parent_ != nullptr) {
            parent_->sendCommand(val->second,
                                 "notify_response",
                                 name,
                                 HelicsSequencingModes::HELICS_SEQUENCING_MODE_FAST);
        }
        val = commandQueue.try_pop();
    }
    return (val) ? *val : std::pair<std::string, std::string>{std::string{}, std::string{}};
}

std::pair<std::string, std::string> FederateState::waitCommand()
{
    auto val = commandQueue.pop();
    if (val.first == "notify") {
        if (parent_ != nullptr) {
            parent_->sendCommand(val.second,
                                 "notify_response",
                                 name,
                                 HelicsSequencingModes::HELICS_SEQUENCING_MODE_FAST);
        }
        val = commandQueue.pop();
    }
    return val;
}

std::string FederateState::processQueryActual(std::string_view query) const
{
    auto addHeader = [this](Json::Value& base) {
        base["name"] = getIdentifier();
        base["id"] = global_id.load().baseValue();
        base["parent"] = parent_->getGlobalId().baseValue();
    };

    auto qres = generateInterfaceQueryResults(query, interfaceInformation, addHeader);
    if (query == "global_flush") {
        return "{\"status\":true}";
    }
    if (query == "subscriptions") {
        std::ostringstream s;
        s << "[";
        auto ipts = interfaceInformation.getInputs();
        for (const auto& ipt : ipts) {
            for (const auto& isrc : ipt->input_sources) {
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
        return generateStringVector(timeCoord->getDependencies(),
                                    [](auto& dep) { return std::to_string(dep.baseValue()); });
    }
    if (query == "current_time") {
        return timeCoord->printTimeStatus();
    }
    if (query == "current_state") {
        Json::Value base;
        addHeader(base);
        base["state"] = fedStateString(state.load());
        base["publications"] = publicationCount();
        base["input"] = inputCount();
        base["endpoints"] = endpointCount();
        base["granted_time"] = static_cast<double>(grantedTime());
        return fileops::generateJsonString(base);
    }
    if (query == "global_state") {
        Json::Value base;
        addHeader(base);
        base["state"] = fedStateString(state.load());
        return fileops::generateJsonString(base);
    }
    if (query == "global_time_debugging") {
        Json::Value base;
        addHeader(base);
        base["state"] = fedStateString(state.load());
        if (!timeCoord->empty()) {
            timeCoord->generateDebuggingTimeInfo(base);
        }
        return fileops::generateJsonString(base);
    }
    if (query == "timeconfig") {
        Json::Value base;
        timeCoord->generateConfig(base);
        generateConfig(base);
        return fileops::generateJsonString(base);
    }
    if (query == "config") {
        Json::Value base;
        timeCoord->generateConfig(base);
        generateConfig(base);
        interfaceInformation.generateInferfaceConfig(base);
        addFederateTags(base, this);
        return fileops::generateJsonString(base);
    }
    if (query == "tags") {
        Json::Value tagBlock = Json::objectValue;
        for (const auto& tg : tags) {
            tagBlock[tg.first] = tg.second;
        }
        return fileops::generateJsonString(tagBlock);
    }
    if (query.compare(0, 4, "tag/") == 0) {
        std::string_view tag = query;
        tag.remove_prefix(4);
        for (const auto& tg : tags) {
            if (tag == tg.first) {
                return Json::valueToQuotedString(tg.second.c_str());
            }
        }
        return "\"\"";
    }
    if (query == "dependents") {
        return generateStringVector(timeCoord->getDependents(),
                                    [](auto& dep) { return std::to_string(dep.baseValue()); });
    }
    if (query == "logs") {
        Json::Value base;
        addHeader(base);
        bufferToJson(mLogManager->getLogBuffer(), base);
        return fileops::generateJsonString(base);
    }
    if (query == "data_flow_graph") {
        Json::Value base;
        addHeader(base);
        interfaceInformation.GenerateDataFlowGraph(base);
        return fileops::generateJsonString(base);
    }
    if (query == "global_time" || query == "global_status") {
        Json::Value base;
        addHeader(base);
        base["granted_time"] = static_cast<double>(timeCoord->getGrantedTime());
        base["send_time"] = static_cast<double>(timeCoord->allowedSendTime());
        return fileops::generateJsonString(base);
    }
    if (query == "dependency_graph") {
        Json::Value base;
        addHeader(base);
        base["dependents"] = Json::arrayValue;
        for (auto& dep : timeCoord->getDependents()) {
            base["dependents"].append(dep.baseValue());
        }
        base["dependencies"] = Json::arrayValue;
        for (auto& dep : timeCoord->getDependencies()) {
            base["dependencies"].append(dep.baseValue());
        }
        return fileops::generateJsonString(base);
    }

    if (queryCallback) {
        auto val = queryCallback(query);
        if (!val.empty()) {
            return val;
        }
    }
    // check tag value for a matching string
    for (const auto& tg : tags) {
        if (tg.first == query) {
            return Json::valueToQuotedString(tg.second.c_str());
        }
    }
    return generateJsonErrorResponse(JsonErrorCodes::BAD_REQUEST, "unrecognized Federate query");
}

std::string FederateState::processQuery(const std::string& query, bool force_ordering) const
{
    std::string qstring;
    if (!force_ordering &&
        (query == "publications" || query == "inputs" || query == "endpoints" ||
         query == "global_state")) {  // these never need to be locked
        qstring = processQueryActual(query);
    } else if ((query == "queries") || (query == "available_queries")) {
        qstring =
            R"("publications","inputs","logs","endpoints","subscriptions","current_state","global_state","dependencies","timeconfig","config","dependents","current_time","global_time","global_status")";
    } else if (query == "state") {
        qstring = fmt::format("\"{}\"", fedStateString(getState()));
    } else {  // the rest might need be locked to prevent a race condition
        if (try_lock()) {
            qstring = processQueryActual(query);
            unlock();
        } else {
            qstring = "#wait";
        }
    }
    return qstring;
}

int FederateState::loggingLevel() const
{
    return mLogManager->getConsoleLevel();
}

void FederateState::setTag(const std::string& tag, const std::string& value)
{
    spinlock();
    for (auto& tg : tags) {
        if (tg.first == tag) {
            unlock();
            tg.second = value;
            return;
        }
    }
    tags.emplace_back(tag, value);
    unlock();
}

static const std::string emptyStr;

const std::string& FederateState::getTag(const std::string& tag) const
{
    spinlock();
    for (const auto& tg : tags) {
        if (tg.first == tag) {
            unlock();
            return tg.second;
        }
    }
    unlock();
    return emptyStr;
}
}  // namespace helics
