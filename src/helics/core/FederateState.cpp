/*
Copyright (c) 2017-2024,
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
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

#ifndef HELICS_DISABLE_ASIO
#    include "MessageTimer.hpp"
#else
namespace helics {
class MessageTimer {};
}  // namespace helics
#endif

#include <fmt/format.h>

// NOLINTNEXTLINE
static const std::string gHelicsEmptyStr;
#define LOG_ERROR(message) logMessage(HELICS_LOG_LEVEL_ERROR, gHelicsEmptyStr, message)
#define LOG_WARNING(message) logMessage(HELICS_LOG_LEVEL_WARNING, gHelicsEmptyStr, message)

#ifdef HELICS_ENABLE_LOGGING

#    define LOG_SUMMARY(message)                                                                   \
        do {                                                                                       \
            if (maxLogLevel >= HELICS_LOG_LEVEL_SUMMARY) {                                         \
                logMessage(HELICS_LOG_LEVEL_SUMMARY, gHelicsEmptyStr, message);                    \
            }                                                                                      \
        } while (false)

#    define LOG_INTERFACES(message)                                                                \
        do {                                                                                       \
            if (maxLogLevel >= HELICS_LOG_LEVEL_INTERFACES) {                                      \
                logMessage(HELICS_LOG_LEVEL_INTERFACES, gHelicsEmptyStr, message);                 \
            }                                                                                      \
        } while (false)

#    ifdef HELICS_ENABLE_DEBUG_LOGGING
#        define LOG_TIMING(message)                                                                \
            do {                                                                                   \
                if (maxLogLevel >= HELICS_LOG_LEVEL_TIMING) {                                      \
                    logMessage(HELICS_LOG_LEVEL_TIMING, gHelicsEmptyStr, message);                 \
                }                                                                                  \
            } while (false)

#        define LOG_DATA(message)                                                                  \
            do {                                                                                   \
                if (maxLogLevel >= HELICS_LOG_LEVEL_DATA) {                                        \
                    logMessage(HELICS_LOG_LEVEL_DATA, gHelicsEmptyStr, message);                   \
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
                    logMessage(HELICS_LOG_LEVEL_TRACE, gHelicsEmptyStr, message);                  \
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
        [this](ActionMessage&& message) { mParent->addActionMessage(std::move(message)); });
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
        case FederateStates::ERRORED:
        case FederateStates::FINISHED:
        case FederateStates::CREATED:
        case FederateStates::TERMINATING:
            state = newState;
            break;
        case FederateStates::INITIALIZING: {
            auto reqState = FederateStates::CREATED;
            state.compare_exchange_strong(reqState, newState);
            break;
        }
        case FederateStates::EXECUTING: {
            auto reqState = FederateStates::INITIALIZING;
            state.compare_exchange_strong(reqState, newState);
            break;
        }
        case FederateStates::UNKNOWN:
        default:
            break;
    }
}

void FederateState::reset(const CoreFederateInfo& fedInfo)
{
    state = FederateStates::CREATED;
    queue.clear();
    delayQueues.clear();
    interfaceInformation.reset();

    timeCoord =
        std::make_unique<TimeCoordinator>([this](const ActionMessage& msg) { routeMessage(msg); });

    only_transmit_on_change = false;
    realtime = false;
    observer = false;
    // reentrant should stay the same
    mSourceOnly = false;
    mCallbackBased = false;
    strict_input_type_checking = false;
    ignore_unit_mismatch = false;
    mSlowResponding = false;
    // allow remote control needs to remain the same

    mLogManager = std::make_unique<LogManager>();
    maxLogLevel = HELICS_LOG_LEVEL_NO_PRINT;
    init_transmitted = false;

    wait_for_current_time = false;
    ignore_time_mismatch_warnings = false;
    mProfilerActive = false;
    mLocalProfileCapture = false;
    errorCode = 0;
    // leave mParent alone
    // CommonCore* mParent{nullptr};  //!< pointer to the higher level;
    errorString.clear();
    rt_lag = timeZero;
    rt_lead = timeZero;
    grantTimeOutPeriod = timeZero;
    realTimeTimerIndex = -1;
    grantTimeoutTimeIndex = -1;
    initRequested = false;
    requestingMode = false;
    initIterating = false;

    iterating = false;
    timeGranted_mode = false;
    terminate_on_error = false;
    lastIterationRequest = IterationRequest::NO_ITERATIONS;
    timeMethod = TimeSynchronizationMethod::DISTRIBUTED;
    mGrantCount = 0;
    commandQueue.clear();
    interfaceFlags = 0;

    events.clear();
    eventMessages.clear();
    delayedFederates.clear();
    time_granted = startupTime;
    allowed_send_time = startupTime;

    queryCallbacks.clear();
    fedCallbacks = nullptr;
    tags.clear();
    // now update with the new properties
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
        [this](ActionMessage&& message) { mParent->addActionMessage(std::move(message)); });
    maxLogLevel = mLogManager->getMaxLevel();
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
    const std::lock_guard<FederateState> plock(*this);
    // this function could be called externally in a multi-threaded context
    auto* pub = interfaceInformation.getPublication(pub_id);
    auto res = pub->CheckSetValue(data, len, time_granted, only_transmit_on_change);
    return res;
}

void FederateState::generateConfig(nlohmann::json& base) const
{
    base["only_transmit_on_change"] = only_transmit_on_change;
    base["realtime"] = realtime;
    base["observer"] = observer;
    base["reentrant"] = reentrant;
    base["source_only"] = mSourceOnly;
    base["strict_input_type_checking"] = strict_input_type_checking;
    base["slow_responding"] = mSlowResponding;
    if (!mAllowRemoteControl) {
        base["disable_remote_control"] = !mAllowRemoteControl;
    }

    if (rt_lag > timeZero) {
        base["rt_lag"] = static_cast<double>(rt_lag);
    }
    if (rt_lead > timeZero) {
        base["rt_lead"] = static_cast<double>(rt_lead);
    }
}

uint64_t FederateState::getQueueSize(InterfaceHandle hid) const
{
    const auto* epI = interfaceInformation.getEndpoint(hid);
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

std::unique_ptr<Message> FederateState::receive(InterfaceHandle hid)
{
    auto* epI = interfaceInformation.getEndpoint(hid);
    if (epI != nullptr) {
        return epI->getMessage(time_granted);
    }
    return nullptr;
}

std::unique_ptr<Message> FederateState::receiveAny(InterfaceHandle& hid)
{
    Time earliest_time = Time::maxVal();
    EndpointInfo* endpointI = nullptr;
    auto elock = interfaceInformation.getEndpoints();
    // Find the end point with the earliest message time
    for (const auto& end_point : elock) {
        auto firstTime = end_point->firstMessageTime();
        if (firstTime < earliest_time) {
            earliest_time = firstTime;
            endpointI = end_point.get();
        }
    }
    if (endpointI == nullptr) {
        return nullptr;
    }
    // Return the message found and remove from the queue
    if (earliest_time <= time_granted) {
        auto result = endpointI->getMessage(time_granted);
        hid = (result) ? endpointI->id.handle : InterfaceHandle{};

        return result;
    }
    hid = InterfaceHandle();
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

std::pair<SmallBuffer, Time> FederateState::getPublishedValue(InterfaceHandle handle)
{
    auto* pub = interfaces().getPublication(handle);
    if (pub != nullptr) {
        return {pub->data, pub->lastPublishTime};
    }
    return {SmallBuffer{}, Time::minVal()};
}

void FederateState::routeMessage(const ActionMessage& msg)
{
    if (mParent != nullptr) {
        if (msg.action() == CMD_TIME_REQUEST && !requestingMode) {
            LOG_ERROR("sending time request in invalid state");
        }
        if (msg.action() == CMD_TIME_GRANT) {
            requestingMode.store(false);
        }
        mParent->addActionMessage(msg);
    } else {
        addAction(msg);
    }
}

void FederateState::routeMessage(ActionMessage&& msg)
{
    if (mParent != nullptr) {
        if (msg.action() == CMD_TIME_REQUEST && !requestingMode) {
            LOG_ERROR("sending time request in invalid state");
        }
        if (msg.action() == CMD_TIME_GRANT) {
            requestingMode.store(false);
        }
        mParent->addActionMessage(std::move(msg));
    } else {
        addAction(std::move(msg));
    }
}

void FederateState::addAction(const ActionMessage& action)
{
    if (action.action() != CMD_IGNORE) {
        queue.push(action);
        if (mCallbackBased) {
            callbackProcessing();
        }
    }
}

void FederateState::addAction(ActionMessage&& action)
{
    if (action.action() != CMD_IGNORE) {
        queue.push(std::move(action));
        if (mCallbackBased) {
            callbackProcessing();
        }
    }
}

void FederateState::createInterface(InterfaceType htype,
                                    InterfaceHandle handle,
                                    std::string_view key,
                                    std::string_view type,
                                    std::string_view units,
                                    uint16_t flags)
{
    const std::lock_guard<FederateState> plock(*this);
    // this function could be called externally in a multi-threaded context
    switch (htype) {
        case InterfaceType::PUBLICATION:
            interfaceInformation.createPublication(handle, key, type, units, flags);

            break;
        case InterfaceType::INPUT:
            interfaceInformation.createInput(handle, key, type, units, flags);
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
            break;
        case InterfaceType::ENDPOINT:
        case InterfaceType::SINK:
            interfaceInformation.createEndpoint(handle, key, type, flags);
            break;
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
                for (const auto& sub : pub->subscribers) {
                    rem.setDestination(sub.id);
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
    // this function can fail try_lock gracefully

    const std::lock_guard<FederateState> fedlock(*this);
    IterationResult ret;
    switch (getState()) {
        case FederateStates::CREATED: {  // we are still in the created state
            return waitSetup();
        }
        case FederateStates::ERRORED:
            ret = IterationResult::ERROR_RESULT;
            break;
        case FederateStates::FINISHED:
            ret = IterationResult::HALTED;
            break;
        default:
            ret = IterationResult::NEXT_STEP;
            break;
    }

    return ret;
}

IterationResult FederateState::enterInitializingMode(IterationRequest request)
{
    if (try_lock()) {  // only enter this loop once per federate
        auto ret = processQueue();
        unlock();
        initIterating = false;
        switch (ret) {
            case MessageProcessingResult::NEXT_STEP:
                time_granted = initialTime;
                allowed_send_time = initialTime;
                break;
            case MessageProcessingResult::ITERATING:
            default:
                break;
        }
        return static_cast<IterationResult>(ret);
    }
    // this function can handle try_lock fail gracefully
    sleeplock();
    IterationResult ret;
    switch (getState()) {
        case FederateStates::ERRORED:
            ret = IterationResult::ERROR_RESULT;
            break;
        case FederateStates::FINISHED:
            ret = IterationResult::HALTED;
            break;
        case FederateStates::CREATED:
            unlock();
            return enterInitializingMode(request);
        default:  // everything >= INITIALIZING
            ret = IterationResult::NEXT_STEP;
            break;
    }
    unlock();
    return ret;
}

iteration_time FederateState::enterExecutingMode(IterationRequest iterate, bool sendRequest)
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
        updateDataForExecEntry(ret, iterate);
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
        return {time_granted, static_cast<IterationResult>(ret)};
    }

    // if this is not true then try again the core may have been handing something short so try
    // again
    if (!queueProcessing.load()) {
        std::this_thread::yield();
        if (!queueProcessing.load()) {
            return enterExecutingMode(iterate, sendRequest);
        }
    }

    // the following code is for a situation in which this method has been called multiple times
    // from different threads, which really shouldn't be done but it isn't really an error so we
    // need to deal with it.
    const std::lock_guard<FederateState> plock(*this);
    IterationResult ret;
    switch (getState()) {
        case FederateStates::ERRORED:
            ret = IterationResult::ERROR_RESULT;
            break;
        case FederateStates::FINISHED:
            ret = IterationResult::HALTED;
            break;
        case FederateStates::CREATED:
        case FederateStates::INITIALIZING:
        default:
            ret = IterationResult::ITERATING;
            break;
        case FederateStates::EXECUTING:
            ret = IterationResult::NEXT_STEP;
            break;
    }
    return {time_granted, ret};
}

void FederateState::updateDataForExecEntry(MessageProcessingResult result, IterationRequest iterate)
{
    ++mGrantCount;
    if (result == MessageProcessingResult::NEXT_STEP) {
        time_granted = timeCoord->getGrantedTime();
        allowed_send_time = timeCoord->allowedSendTime();
    } else if (result == MessageProcessingResult::ITERATING) {
        time_granted = initializationTime;
        allowed_send_time = initializationTime;
    }
    if (result != MessageProcessingResult::ERROR_RESULT) {
        switch (iterate) {
            case IterationRequest::FORCE_ITERATION:
                fillEventVectorNextIteration(time_granted);
                break;
            case IterationRequest::ITERATE_IF_NEEDED:
                if (result == MessageProcessingResult::NEXT_STEP) {
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
            default:
                break;
        }
    }
}

std::vector<GlobalHandle> FederateState::getSubscribers(InterfaceHandle handle)
{
    const std::lock_guard<FederateState> fedlock(*this);
    std::vector<GlobalHandle> subs;
    auto* pubInfo = interfaceInformation.getPublication(handle);
    if (pubInfo != nullptr) {
        for (const auto& sub : pubInfo->subscribers) {
            subs.emplace_back(sub.id);
        }
    }
    return subs;
}

std::vector<std::pair<GlobalHandle, std::string_view>>
    FederateState::getMessageDestinations(InterfaceHandle handle)
{
    const std::lock_guard<FederateState> fedlock(*this);
    const auto* eptInfo = interfaceInformation.getEndpoint(handle);
    if (eptInfo != nullptr) {
        return eptInfo->getTargets();
    }
    return {};
}
iteration_time FederateState::requestTime(Time nextTime, IterationRequest iterate, bool sendRequest)
{
    if (try_lock()) {  // only enter this loop once per federate
        const Time lastTime = timeCoord->getGrantedTime();
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
        updateDataForTimeReturn(ret, nextTime, iterate);
        iteration_time retTime = {time_granted, static_cast<IterationResult>(ret)};
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

    // if this is not true then try again the core may have been handling something short so try
    // again
    if (!queueProcessing.load()) {
        std::this_thread::yield();
        if (!queueProcessing.load()) {
            return requestTime(nextTime, iterate, sendRequest);
        }
    }
    LOG_WARNING("duplicate locking attempted");
    // this would not be good practice to get into this part of the function
    // but the area must protect itself against the possibility and should return something sensible
    const std::lock_guard<FederateState> fedlock(*this);
    IterationResult ret = iterating ? IterationResult::ITERATING : IterationResult::NEXT_STEP;
    if (state == FederateStates::FINISHED) {
        ret = IterationResult::HALTED;
    } else if (state == FederateStates::ERRORED) {
        ret = IterationResult::ERROR_RESULT;
    }
    return {time_granted, ret};
}

void FederateState::updateDataForTimeReturn(MessageProcessingResult result,
                                            Time nextTime,
                                            IterationRequest iterate)
{
    ++mGrantCount;
    if (result == MessageProcessingResult::HALTED) {
        time_granted = Time::maxVal();
        allowed_send_time = Time::maxVal();
        iterating = false;
    } else {
        time_granted = timeCoord->getGrantedTime();
        allowed_send_time = timeCoord->allowedSendTime();
        iterating = (result == MessageProcessingResult::ITERATING);
    }

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
        default:
            break;
    }
}

void FederateState::fillEventVectorUpTo(Time currentTime)
{
    events.clear();
    eventMessages.clear();
    for (const auto& ipt : interfaceInformation.getInputs()) {
        if (ipt->updateTimeUpTo(currentTime)) {
            events.push_back(ipt->id.handle);
        }
    }
    for (const auto& ept : interfaceInformation.getEndpoints()) {
        if (ept->updateTimeUpTo(currentTime)) {
            eventMessages.push_back(ept->id.handle);
        }
    }
}

void FederateState::fillEventVectorInclusive(Time currentTime)
{
    events.clear();
    for (const auto& ipt : interfaceInformation.getInputs()) {
        if (ipt->updateTimeInclusive(currentTime)) {
            events.push_back(ipt->id.handle);
        }
    }
    eventMessages.clear();
    for (const auto& ept : interfaceInformation.getEndpoints()) {
        if (ept->updateTimeInclusive(currentTime)) {
            eventMessages.push_back(ept->id.handle);
        }
    }
}

void FederateState::fillEventVectorNextIteration(Time currentTime)
{
    events.clear();
    for (const auto& ipt : interfaceInformation.getInputs()) {
        if (ipt->updateTimeNextIteration(currentTime)) {
            events.push_back(ipt->id.handle);
        }
    }
    eventMessages.clear();
    for (const auto& ept : interfaceInformation.getEndpoints()) {
        if (ept->updateTimeNextIteration(currentTime)) {
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
    // if this is not true then try again the core may have been handling something short so try
    // again
    if (!queueProcessing.load()) {
        std::this_thread::yield();
        if (!queueProcessing.load()) {
            return genericUnspecifiedQueueProcess(busyReturn);
        }
    }
    if (busyReturn) {
        return MessageProcessingResult::BUSY;
    }
    sleeplock();
    MessageProcessingResult ret;
    switch (getState()) {
        case FederateStates::ERRORED:
            ret = MessageProcessingResult::ERROR_RESULT;
            break;
        case FederateStates::FINISHED:
            ret = MessageProcessingResult::HALTED;
            break;
        default:  // everything >= INITIALIZING
            ret = MessageProcessingResult::NEXT_STEP;
            break;
    }
    unlock();
    return ret;
}

void FederateState::finalize()
{
    if ((state == FederateStates::FINISHED) || (state == FederateStates::ERRORED)) {
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

// const std::vector<InterfaceHandle> emptyHandles;

const std::vector<InterfaceHandle>& FederateState::getEvents() const
{
    return events;
}

MessageProcessingResult FederateState::processDelayQueue() noexcept
{
    delayedFederates.clear();
    auto ret_code = MessageProcessingResult::CONTINUE_PROCESSING;
    if (!delayQueues.empty()) {
        for (auto& dqueue : delayQueues) {
            auto& tempQueue = dqueue.second;
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

void FederateState::addFederateToDelay(GlobalFederateId gid)
{
    if ((delayedFederates.empty()) || (gid > delayedFederates.back())) {
        delayedFederates.push_back(gid);
        return;
    }
    auto res = std::lower_bound(delayedFederates.begin(), delayedFederates.end(), gid);
    if (res == delayedFederates.end()) {
        delayedFederates.push_back(gid);
        return;
    }
    if (*res != gid) {
        delayedFederates.insert(res, gid);
    }
}

bool FederateState::messageShouldBeDelayed(const ActionMessage& cmd) const noexcept
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
    const std::string message = fmt::format(
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
        if (mParent != nullptr) {
            ActionMessage prof(CMD_PROFILER_DATA, global_id.load(), parent_broker_id);
            prof.payload = message;
            mParent->addActionMessage(std::move(prof));
        }
    }
}

void FederateState::generateProfilingMessage(bool enterHelicsCode)
{
    auto ctime = std::chrono::steady_clock::now();
    static constexpr std::string_view entry_string("ENTRY");
    static constexpr std::string_view exit_string("EXIT");
    const std::string message = fmt::format(
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
        if (mParent != nullptr) {
            ActionMessage prof(CMD_PROFILER_DATA, global_id.load(), parent_broker_id);
            prof.payload = message;
            mParent->addActionMessage(std::move(prof));
        }
    }
}

void FederateState::initCallbackProcessing()
{
    auto initIter = fedCallbacks->initializeOperations();
    switch (initIter) {
        case IterationRequest::NO_ITERATIONS:
        case IterationRequest::ITERATE_IF_NEEDED:
        case IterationRequest::FORCE_ITERATION:
        default: {
            ActionMessage exec(CMD_EXEC_REQUEST);
            exec.source_id = global_id.load();
            exec.dest_id = global_id.load();
            setIterationFlags(exec, initIter);
            setActionFlag(exec, indicator_flag);
            mParent->addActionMessage(exec);
        } break;
        case IterationRequest::HALT_OPERATIONS: {
            ActionMessage bye(CMD_DISCONNECT);
            bye.source_id = global_id.load();
            bye.dest_id = bye.source_id;
            mParent->addActionMessage(bye);
        } break;
        case IterationRequest::ERROR_CONDITION:
            ActionMessage bye(CMD_LOCAL_ERROR);
            bye.source_id = global_id.load();
            bye.dest_id = bye.source_id;
            bye.messageID = HELICS_USER_EXCEPTION;
            bye.payload = "Callback federate unspecified error condition in initializing callback";
            mParent->addActionMessage(bye);
            break;
    }
    lastIterationRequest = initIter;
}

void FederateState::execCallbackProcessing(IterationResult result)
{
    auto execIter = fedCallbacks->operate({grantedTime(), result});
    switch (execIter.second) {
        case IterationRequest::NO_ITERATIONS:
        case IterationRequest::ITERATE_IF_NEEDED:
        case IterationRequest::FORCE_ITERATION:
        default: {
            ActionMessage treq(CMD_TIME_REQUEST);
            treq.source_id = global_id.load();
            treq.dest_id = treq.source_id;
            treq.actionTime = execIter.first;
            setIterationFlags(treq, execIter.second);
            setActionFlag(treq, indicator_flag);
            mParent->addActionMessage(treq);
        } break;
        case IterationRequest::HALT_OPERATIONS: {
            ActionMessage bye(CMD_DISCONNECT);
            bye.source_id = global_id.load();
            bye.dest_id = bye.source_id;
            mParent->addActionMessage(bye);
        } break;
        case IterationRequest::ERROR_CONDITION:
            ActionMessage bye(CMD_LOCAL_ERROR);
            bye.source_id = global_id.load();
            bye.dest_id = bye.source_id;
            bye.messageID = HELICS_USER_EXCEPTION;
            bye.payload = "Callback federate unspecified error condition in executing callback";
            mParent->addActionMessage(bye);
            break;
    }
    lastIterationRequest = execIter.second;
}

void FederateState::callbackReturnResult(FederateStates lastState,
                                         MessageProcessingResult result,
                                         FederateStates newState) noexcept
{
    try {
        // handle some general new states
        if (lastState != newState) {
            switch (newState) {
                case FederateStates::TERMINATING:
                    break;
                case FederateStates::FINISHED:
                    if (lastState != FederateStates::ERRORED) {
                        fedCallbacks->finalize();
                    }
                    return;
                case FederateStates::ERRORED:
                    if (lastState != FederateStates::FINISHED) {
                        fedCallbacks->error_handler(lastErrorCode(), lastErrorString());
                    }
                    return;
                default:
                    break;
            }
        }
        switch (result) {
            case MessageProcessingResult::ITERATING:
            case MessageProcessingResult::NEXT_STEP:
                // these are the only 2 results that warrant further processing
                break;
            default:
                return;
        }
        switch (lastState) {
            case FederateStates::CREATED:
                // this is the only valid transition that hasn't been dealt with yet
                initCallbackProcessing();
                break;
            case FederateStates::INITIALIZING:
                if (newState == FederateStates::INITIALIZING) {
                    updateDataForExecEntry(result, lastIterationRequest);
                    initCallbackProcessing();
                } else {
                    updateDataForExecEntry(result, lastIterationRequest);
                    execCallbackProcessing(IterationResult::NEXT_STEP);
                }
                break;
            case FederateStates::EXECUTING:
                updateDataForTimeReturn(result,
                                        timeCoord->getRequestedTime(),
                                        lastIterationRequest);
                execCallbackProcessing(result == MessageProcessingResult::ITERATING ?
                                           IterationResult::ITERATING :
                                           IterationResult::NEXT_STEP);
                break;
            default:
                break;
        }
    }
    catch (const std::exception& e) {
        if (newState != FederateStates::ERRORED && newState != FederateStates::FINISHED) {
            ActionMessage bye(CMD_LOCAL_ERROR);
            bye.source_id = global_id.load();
            bye.dest_id = bye.source_id;
            bye.messageID = HELICS_USER_EXCEPTION;
            bye.payload = e.what();
            mParent->addActionMessage(bye);
        }
    }
    catch (...) {
        if (newState != FederateStates::ERRORED && newState != FederateStates::FINISHED) {
            ActionMessage bye(CMD_LOCAL_ERROR);
            bye.source_id = global_id.load();
            bye.dest_id = bye.source_id;
            bye.messageID = HELICS_USER_EXCEPTION;
            bye.payload = "unrecognized exception thrown in federate callback";
            mParent->addActionMessage(bye);
        }
    }
}

void FederateState::callbackProcessing() noexcept
{
    if (state == FederateStates::FINISHED) {
        return;
    }
    auto initError = (state == FederateStates::ERRORED);
    bool error_cmd{false};

    if (!initRequested) {
        // don't run callback processing before the user calls enterInit
        return;
    }
    auto cState = state.load();
    // process the delay Queue first
    auto ret_code = processDelayQueue();
    while (returnableResult(ret_code)) {
        callbackReturnResult(cState, ret_code, state.load());
        cState = state.load();
        ret_code = processDelayQueue();
    }
    auto cmd = queue.try_pop();
    while (cmd) {
        if (messageShouldBeDelayed(*cmd)) {
            delayQueues[cmd->source_id].push_back(*cmd);
            cmd = queue.try_pop();
            continue;
        }
        ret_code = processActionMessage(*cmd);

        if (ret_code == MessageProcessingResult::DELAY_MESSAGE) {
            delayQueues[static_cast<GlobalFederateId>(cmd->source_id)].push_back(*cmd);
        }
        if (ret_code == MessageProcessingResult::ERROR_RESULT &&
            cmd->action() == CMD_GLOBAL_ERROR) {
            error_cmd = true;
        }
        if (ret_code == MessageProcessingResult::ERROR_RESULT && state == FederateStates::ERRORED) {
            if (!initError && !error_cmd) {
                if (mParent != nullptr) {
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
                    mParent->addActionMessage(std::move(gError));
                }
            }
        }
        if (initError) {
            ret_code = MessageProcessingResult::ERROR_RESULT;
        }
        if (returnableResult(ret_code)) {
            callbackReturnResult(cState, ret_code, state.load());
            cState = state.load();
        }
        cmd = queue.try_pop();
    }
}

MessageProcessingResult FederateState::processQueue() noexcept
{
    if (state == FederateStates::FINISHED) {
        return MessageProcessingResult::HALTED;
    }
    auto initError = (state == FederateStates::ERRORED);
    bool error_cmd{false};
    const bool profilerActive{mProfilerActive};
    queueProcessing.store(true);
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
        ret_code = processActionMessage(cmd);

        if (ret_code == MessageProcessingResult::DELAY_MESSAGE) {
            delayQueues[static_cast<GlobalFederateId>(cmd.source_id)].push_back(cmd);
        }
        if (ret_code == MessageProcessingResult::ERROR_RESULT && cmd.action() == CMD_GLOBAL_ERROR) {
            error_cmd = true;
        }
    }

    if (ret_code == MessageProcessingResult::ERROR_RESULT && state == FederateStates::ERRORED) {
        if (!initError && !error_cmd) {
            if (mParent != nullptr) {
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

                mParent->addActionMessage(std::move(gError));
            }
        }
    }
    if (initError) {
        ret_code = MessageProcessingResult::ERROR_RESULT;
    }
    queueProcessing.store(false);
    if (profilerActive) {
        generateProfilingMessage(false);
    }
    return ret_code;
}

std::optional<MessageProcessingResult> FederateState::checkProcResult(
    std::tuple<FederateStates, MessageProcessingResult, bool>& proc_result,
    ActionMessage& cmd)
{
    timeGranted_mode = std::get<2>(proc_result);

    if (getState() != std::get<0>(proc_result)) {
        setState(std::get<0>(proc_result));
        switch (std::get<0>(proc_result)) {
            case FederateStates::INITIALIZING:
                LOG_TIMING("Granting Initialization");
                if (checkInterfaces() != defs::Errors::OK) {
                    setState(FederateStates::ERRORED);
                    return MessageProcessingResult::ERROR_RESULT;
                }
                timeCoord->enterInitialization();
                break;
            case FederateStates::EXECUTING:
                timeCoord->updateTimeFactors();
                LOG_TIMING("Granting Execution");
                break;
            case FederateStates::FINISHED:
                LOG_TIMING("Terminating");
                break;
            case FederateStates::ERRORED:
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
                        LOG_WARNING(fmt::format("forced Granted Time={}",
                                                static_cast<double>(time_granted)));
                    }
                } else {
                    LOG_TIMING(fmt::format("Granted Time={}", static_cast<double>(time_granted)));
                }
            }
            return (std::get<1>(proc_result));
    }
    return std::nullopt;
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

    auto result = checkProcResult(proc_result, cmd);
    if (result) {
        return *result;
    }

    switch (cmd.action()) {
        case CMD_IGNORE:
        default:
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
            if ((state != FederateStates::FINISHED) && (state != FederateStates::TERMINATING)) {
                timeCoord->disconnect();
                cmd.dest_id = parent_broker_id;
                if (state != FederateStates::ERRORED) {
                    setState(FederateStates::TERMINATING);
                }
                routeMessage(cmd);
            }
            break;
        case CMD_DISCONNECT_FED:
        case CMD_DISCONNECT:
            if (cmd.source_id == global_id.load()) {
                if ((state != FederateStates::FINISHED) && (state != FederateStates::TERMINATING)) {
                    timeCoord->disconnect();
                    cmd.dest_id = parent_broker_id;
                    if (state != FederateStates::ERRORED) {
                        setState(FederateStates::TERMINATING);
                    }
                    routeMessage(cmd);
                }
            } else {
                const Time lastTime = timeCoord->getLastGrant(cmd.source_id);
                switch (timeCoord->processTimeMessage(cmd)) {
                    case TimeProcessingResult::DELAY_PROCESSING:
                        addFederateToDelay(cmd.source_id);
                        return MessageProcessingResult::DELAY_MESSAGE;
                    case TimeProcessingResult::NOT_PROCESSED:
                        return MessageProcessingResult::CONTINUE_PROCESSING;
                    default:
                        break;
                }
                if (state != FederateStates::EXECUTING) {
                    break;
                }

                interfaceInformation.disconnectFederate(cmd.source_id, lastTime);
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
        case CMD_SEND_MESSAGE:
        case CMD_PUB:
            processDataMessage(cmd);
            break;
        case CMD_LOG:
        case CMD_REMOTE_LOG:
        case CMD_WARNING:
        case CMD_SET_PROFILER_FLAG:
        case CMD_TIMEOUT_DISCONNECT:
            processLoggingMessage(cmd);
            break;
        case CMD_GRANT_TIMEOUT_CHECK:
            timeoutCheck(cmd);
            break;
        case CMD_ADD_PUBLISHER:
        case CMD_ADD_SUBSCRIBER:
        case CMD_ADD_ENDPOINT:
        case CMD_REMOVE_NAMED_PUBLICATION:
        case CMD_REMOVE_PUBLICATION:
        case CMD_REMOVE_SUBSCRIBER:
        case CMD_REMOVE_ENDPOINT:
        case CMD_CLOSE_INTERFACE:
            processDataConnectionMessage(cmd);
            break;

        case CMD_FED_ACK:
            if (state != FederateStates::CREATED) {
                break;
            }
            if (cmd.name() == name) {
                if (checkActionFlag(cmd, error_flag)) {
                    setState(FederateStates::ERRORED);
                    errorString = commandErrorString(cmd.messageID);
                    return MessageProcessingResult::ERROR_RESULT;
                }
                if (checkActionFlag(cmd, global_timing_flag)) {
                    if (checkActionFlag(cmd, async_timing_flag)) {
                        timeMethod = TimeSynchronizationMethod::ASYNC;
                    } else {
                        timeMethod = TimeSynchronizationMethod::GLOBAL;
                        addDependent(gRootBrokerID);
                        addDependency(gRootBrokerID);
                        timeCoord->setAsParent(gRootBrokerID);
                    }
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

        case CMD_QUERY_ORDERED:
        case CMD_QUERY: {
            ActionMessage queryResp(cmd.action() == CMD_QUERY ? CMD_QUERY_REPLY :
                                                                CMD_QUERY_REPLY_ORDERED);
            queryResp.dest_id = cmd.source_id;
            queryResp.source_id = cmd.dest_id;
            queryResp.messageID = cmd.messageID;
            queryResp.counter = cmd.counter;

            queryResp.payload = processQueryActual(cmd.payload.to_string());
            routeMessage(std::move(queryResp));
        } break;
    }
    return MessageProcessingResult::CONTINUE_PROCESSING;
}

void FederateState::timeoutCheck(ActionMessage& cmd)
{
    if (timeGranted_mode && cmd.actionTime != Time::maxVal()) {
        return;
    }
    if (mGrantCount != static_cast<std::uint32_t>(cmd.getExtraData())) {
        // time has been granted since this was triggered
        return;
    }
    if (cmd.counter == 0) {
        auto blockFed = timeCoord->getMinGrantedDependency();
        if (blockFed.first.isValid()) {
            LOG_WARNING(fmt::format("grant timeout exceeded sim time {} waiting on {}",
                                    static_cast<double>(time_granted),
                                    blockFed.first.baseValue()));
        } else {
            LOG_WARNING(fmt::format("grant timeout exceeded sim time {}",
                                    static_cast<double>(time_granted)));
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
            ActionMessage brokerTimeoutCheck{cmd};
            brokerTimeoutCheck.source_id = global_id.load();
            brokerTimeoutCheck.dest_id = parentID;
            routeMessage(brokerTimeoutCheck);
            LOG_WARNING(fmt::format("sending grant time out check to {}", parentID.baseValue()));
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
}
void FederateState::processDataConnectionMessage(ActionMessage& cmd)
{
    switch (cmd.action()) {
        case CMD_ADD_PUBLISHER: {
            auto* subI = interfaceInformation.getInput(cmd.dest_handle);
            if (subI != nullptr) {
                if (subI->addSource(cmd.getSource(),
                                    cmd.name(),
                                    cmd.getString(typeStringLoc),
                                    cmd.getString(unitStringLoc))) {
                    if (timeMethod == TimeSynchronizationMethod::DISTRIBUTED) {
                        addDependency(cmd.source_id);
                    }
                }
            } else {
                auto* eptI = interfaceInformation.getEndpoint(cmd.dest_handle);
                if (eptI != nullptr) {
                    eptI->addSource(cmd.getSource(), cmd.name(), cmd.getString(typeStringLoc));
                    if (timeMethod == TimeSynchronizationMethod::DISTRIBUTED) {
                        addDependency(cmd.source_id);
                    }
                }
            }
        } break;
        case CMD_ADD_SUBSCRIBER: {
            auto* pubI = interfaceInformation.getPublication(cmd.dest_handle);
            if (pubI != nullptr) {
                if (pubI->addSubscriber(cmd.getSource(), cmd.name())) {
                    if (timeMethod == TimeSynchronizationMethod::DISTRIBUTED) {
                        addDependent(cmd.source_id);
                    }
                }

                if (getState() > FederateStates::CREATED) {
                    if (getState() == FederateStates::EXECUTING &&
                        timeMethod == TimeSynchronizationMethod::DISTRIBUTED) {
                        resetDependency(cmd.source_id);
                        addDependent(cmd.source_id);
                    }
                    if (!pubI->data.empty() && pubI->lastPublishTime > Time::minVal()) {
                        ActionMessage pub(CMD_PUB);
                        pub.setSource(pubI->id);
                        pub.setDestination(cmd.getSource());
                        pub.counter = static_cast<uint16_t>(getCurrentIteration());
                        pub.payload = pubI->data;
                        pub.actionTime = pubI->lastPublishTime;
                        routeMessage(std::move(pub));
                    }
                }
            }
        } break;
        case CMD_ADD_ENDPOINT: {
            auto* eptI = interfaceInformation.getEndpoint(cmd.dest_handle);
            if (eptI != nullptr) {
                if (checkActionFlag(cmd, destination_target)) {
                    eptI->addDestination(cmd.getSource(), cmd.name(), cmd.getString(typeStringLoc));
                    if (eptI->targetedEndpoint) {
                        if (timeMethod == TimeSynchronizationMethod::DISTRIBUTED) {
                            addDependent(cmd.source_id);
                        }
                    }
                } else {
                    eptI->addSource(cmd.getSource(), cmd.name(), cmd.getString(typeStringLoc));
                    if (eptI->targetedEndpoint) {
                        if (timeMethod == TimeSynchronizationMethod::DISTRIBUTED) {
                            addDependency(cmd.source_id);
                            if (getState() == FederateStates::EXECUTING) {
                                minimumReceiveTime = time_granted;
                            }
                        }
                    }
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
        case CMD_CLOSE_INTERFACE:
            if (cmd.source_id == global_id.load()) {
                closeInterface(cmd.source_handle, static_cast<InterfaceType>(cmd.counter));
            }
            break;

        case CMD_REMOVE_ENDPOINT:
        default:
            break;
    }
}

void FederateState::processDataMessage(ActionMessage& cmd)
{
    switch (cmd.action()) {
        case CMD_SEND_MESSAGE: {
            auto* epi = interfaceInformation.getEndpoint(cmd.dest_handle);
            if (epi != nullptr && !epi->sourceOnly) {
                if (cmd.actionTime < minimumReceiveTime) {
                    cmd.actionTime = minimumReceiveTime;
                }
                // if (!epi->not_interruptible)
                {
                    timeCoord->updateMessageTime(cmd.actionTime, !timeGranted_mode);
                }
                LOG_DATA(fmt::format("receive_message {}", prettyPrintString(cmd)));
                if (cmd.actionTime < time_granted &&
                    timeMethod != TimeSynchronizationMethod::ASYNC) {
                    LOG_WARNING(
                        fmt::format("received message {} at time({}) earlier than granted time({})",
                                    prettyPrintString(cmd),
                                    static_cast<double>(cmd.actionTime),
                                    static_cast<double>(time_granted)));
                    auto qres = processQueryActual("global_time_debugging");
                    qres.insert(0, "TIME DEBUGGING::");
                    LOG_WARNING(qres);
                }

                if (state <= FederateStates::EXECUTING) {
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
                    if (cmd.actionTime < time_granted &&
                        timeMethod != TimeSynchronizationMethod::ASYNC) {
                        LOG_WARNING(fmt::format(
                            "received message {} at time({}) earlier than granted time({})",
                            prettyPrintString(cmd),
                            static_cast<double>(cmd.actionTime),
                            static_cast<double>(time_granted)));
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
                    if (state <= FederateStates::EXECUTING) {
                        timeCoord->processTimeMessage(cmd);
                    }
                }
                break;
            }
            for (auto& src : subI->input_sources) {
                auto valueTime = cmd.actionTime;
                if (timeMethod == TimeSynchronizationMethod::ASYNC) {
                    if (valueTime < time_granted) {
                        valueTime = time_granted;
                    }
                }
                if ((cmd.source_id == src.fed_id) && (cmd.source_handle == src.handle)) {
                    if (subI->addData(src,
                                      valueTime,
                                      cmd.counter,
                                      std::make_shared<const SmallBuffer>(
                                          std::move(cmd.payload)))) {
                        if (!subI->not_interruptible) {
                            timeCoord->updateValueTime(valueTime, !timeGranted_mode);
                            LOG_TRACE(timeCoord->printTimeStatus());
                        }
                        LOG_DATA(fmt::format("receive PUBLICATION {} from {}",
                                             prettyPrintString(cmd),
                                             subI->getSourceName(src)));
                    }
                    // this can only match once
                    break;
                }
            }
            if (state <= FederateStates::EXECUTING) {
                timeCoord->processTimeMessage(cmd);
            }
        } break;
        default:
            break;
    }
}

void FederateState::processLoggingMessage(ActionMessage& cmd)
{
    switch (cmd.action()) {
        case CMD_LOG:
        case CMD_REMOTE_LOG:
            logMessage(cmd.messageID,
                       cmd.getString(0),
                       cmd.payload.to_string(),
                       cmd.action() == CMD_REMOTE_LOG);
            break;

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
        case CMD_SET_PROFILER_FLAG:
            setOptionFlag(defs::PROFILING, checkActionFlag(cmd, indicator_flag));
            break;
        case CMD_TIMEOUT_DISCONNECT: {
            auto qres = processQueryActual("global_time_debugging");
            qres.insert(0, "TIME DEBUGGING::");
            LOG_WARNING(qres);
        } break;
        default:
            break;
    }
}

void FederateState::setProperties(const ActionMessage& cmd)
{
    if (state == FederateStates::CREATED) {
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
                if (getState() >= FederateStates::INITIALIZING && grantTimeOutPeriod > timeZero) {
                    if (!mTimer) {
                        if (!mTimer) {
                            mTimer = std::make_shared<MessageTimer>([this](ActionMessage&& mess) {
                                return this->addAction(std::move(mess));
                            });
                        }
                    }
                }
                // if we are currently waiting for a grant trigger the timer
                if (getState() == FederateStates::EXECUTING && !timeGranted_mode) {
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
        case defs::Properties::INDEX_GROUP:
            indexGroup = (propertyVal > 16) ? 16 : ((propertyVal < 0) ? 0 : propertyVal);
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
            mSlowResponding = value;
            break;
        case defs::Flags::PROFILING:
            if (value && !mProfilerActive) {
                generateProfilingMarker();
            }
            mProfilerActive = value;
            break;
        case defs::Flags::ALLOW_REMOTE_CONTROL:
            mAllowRemoteControl = value;
            break;
        case defs::Flags::DISABLE_REMOTE_CONTROL:
            mAllowRemoteControl = !value;
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
                if (state < FederateStates::EXECUTING) {
                    realtime = true;
                }
            } else {
                realtime = false;
            }

            break;
        case defs::Flags::SOURCE_ONLY:
            if (state == FederateStates::CREATED) {
                mSourceOnly = value;
                if (value) {
                    observer = false;
                }
            }
            break;
        case defs::Flags::OBSERVER:
            if (state == FederateStates::CREATED) {
                observer = value;
                if (value) {
                    mSourceOnly = false;
                }
            }
            break;
        case defs::Flags::REENTRANT:
            if (state == FederateStates::CREATED) {
                reentrant = value;
            }
            break;
        case defs::Flags::CALLBACK_FEDERATE:
            if (state == FederateStates::CREATED) {
                mCallbackBased = value;
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
        case defs::Options::RECONNECTABLE:
            if (value) {
                interfaceFlags |= make_flags(reconnectable_flag);
            } else {
                interfaceFlags &= ~(make_flags(reconnectable_flag));
            }
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
        case defs::Flags::REENTRANT:
            return reentrant;
        case defs::Flags::CALLBACK_FEDERATE:
            return mCallbackBased;
        case defs::Flags::SOURCE_ONLY:
            return mSourceOnly;
        case defs::Flags::SLOW_RESPONDING:
        case defs::Flags::DEBUGGING:
            return mSlowResponding;
        case defs::Flags::TERMINATE_ON_ERROR:
            return terminate_on_error;
        case defs::Flags::CONNECTIONS_REQUIRED:
            return ((interfaceFlags.load() & make_flags(required_flag)) != 0);
        case defs::Flags::CONNECTIONS_OPTIONAL:
            return ((interfaceFlags.load() & make_flags(optional_flag)) != 0);
        case defs::Options::RECONNECTABLE:
            return ((interfaceFlags.load() & make_flags(reconnectable_flag)) != 0);
        case defs::Flags::STRICT_INPUT_TYPE_CHECKING:
            return strict_input_type_checking;
        case defs::Flags::IGNORE_INPUT_UNIT_MISMATCH:
            return ignore_unit_mismatch;
        case defs::Flags::IGNORE_TIME_MISMATCH_WARNINGS:
            return ignore_time_mismatch_warnings;
        case defs::Properties::LOG_BUFFER:
            return (mLogManager->getLogBuffer().capacity() > 0);
            // NOTE: the allow remote control property is purposely not retrievable and should not
            // be in config generation
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
        case defs::Properties::CURRENT_ITERATION:
            return timeCoord->getCurrentIteration();
        case defs::Properties::LOG_LEVEL:
        case defs::Properties::FILE_LOG_LEVEL:
        case defs::Properties::CONSOLE_LOG_LEVEL:
            return mLogManager->getConsoleLevel();
        case defs::Properties::LOG_BUFFER:
            return static_cast<int>(mLogManager->getLogBuffer().capacity());
        case defs::Properties::INDEX_GROUP:
            return indexGroup;
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

void FederateState::resetDependency(GlobalFederateId gid)
{
    timeCoord->resetDependency(gid);
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
    for (const auto& endpoint : interfaceInformation.getEndpoints()) {
        auto messageTime = endpoint->firstMessageTime();
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
    mParent = parent;
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
    auto currentTime = grantedTime();
    std::string timeString;
    if (currentTime < timeZero) {
        timeString = fmt::format("[{}]", fedStateString(getState()));
    } else if (currentTime == Time::maxVal()) {
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
    static const std::string created{"created"};
    static const std::string estate{"error"};
    static const std::string init{"initializing"};
    static const std::string dis{"disconnected"};
    static const std::string exec{"executing"};
    static const std::string term{"terminating"};
    static const std::string unk{"unknown"};

    switch (state) {
        case FederateStates::CREATED:
            return created;
        case FederateStates::INITIALIZING:
            return init;
        case FederateStates::EXECUTING:
            return exec;
        case FederateStates::TERMINATING:
            return term;
        case FederateStates::FINISHED:
            return dis;
        case FederateStates::ERRORED:
            return estate;
        case FederateStates::UNKNOWN:
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

    if (res[0] == "terminate" || res[0] == "halt") {
        if (mAllowRemoteControl) {
            if (mParent != nullptr) {
                ActionMessage bye(CMD_DISCONNECT);
                bye.source_id = global_id.load();
                bye.dest_id = bye.source_id;
                mParent->addActionMessage(bye);
            }
        } else {
            if (mParent != nullptr) {
                ActionMessage response(command.action());
                response.payload =
                    fmt::format("log {} does not allow remote termination", getIdentifier());
                response.dest_id = command.source_id;
                response.source_id = global_id.load();
                response.setString(targetStringLoc, command.getString(sourceStringLoc));
                response.setString(sourceStringLoc, getIdentifier());

                mParent->addActionMessage(std::move(response));
            }
        }
    } else if (res[0] == "echo") {
        if (mParent != nullptr) {
            ActionMessage response(command.action());
            response.payload = "echo_reply";
            response.dest_id = command.source_id;
            response.source_id = global_id.load();
            response.setString(targetStringLoc, command.getString(sourceStringLoc));
            response.setString(sourceStringLoc, getIdentifier());

            mParent->addActionMessage(std::move(response));
        }
    } else if (res[0] == "command_status") {
        if (mParent != nullptr) {
            ActionMessage response(command.action());
            response.payload = fmt::format("\"{} unprocessed commands\"", commandQueue.size());
            response.dest_id = command.source_id;
            response.source_id = global_id.load();
            response.setString(targetStringLoc, command.getString(sourceStringLoc));
            response.setString(sourceStringLoc, getIdentifier());

            mParent->addActionMessage(std::move(response));
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
    } else if ((res[0] == "set") && (res.size() > 2 && res[1] == "barrier")) {
        ActionMessage barrier(CMD_TIME_BARRIER);
        barrier.dest_id = global_id;
        barrier.actionTime = gmlc::utilities::numeric_conversionComplete<double>(res[2], 0.0);
        if (res.size() >= 4) {
            barrier.messageID =
                gmlc::utilities::numeric_conversionComplete<std::int32_t>(res[3], 0);
        }

        queue.push(barrier);
    } else if ((res[0] == "clear") && (res.size() >= 2 && res[1] == "barrier")) {
        ActionMessage barrier(CMD_TIME_BARRIER_CLEAR);
        barrier.dest_id = global_id;
        if (res.size() >= 3) {
            barrier.messageID =
                gmlc::utilities::numeric_conversionComplete<std::int32_t>(res[2], 0);
        }
        queue.push(barrier);
    } else {
        commandQueue.emplace(cmd, command.getString(sourceStringLoc));
    }
}

std::pair<std::string, std::string> FederateState::getCommand()
{
    auto val = commandQueue.try_pop();
    while (val && val->first == "notify") {
        if (mParent != nullptr) {
            mParent->sendCommand(val->second,
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
        if (mParent != nullptr) {
            mParent->sendCommand(val.second,
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
    auto addHeader = [this](nlohmann::json& base) {
        nlohmann::json att = nlohmann::json::object();
        att["name"] = getIdentifier();
        att["id"] = global_id.load().baseValue();
        att["parent"] = mParent->getGlobalId().baseValue();
        base["attributes"] = att;
    };

    auto qres = generateInterfaceQueryResults(query, interfaceInformation, addHeader);
    if (!qres.empty()) {
        return qres;
    }
    if (query == "global_flush") {
        return "{\"status\":true}";
    }
    if (query == "subscriptions") {
        std::ostringstream subs;
        subs << "[";
        auto ipts = interfaceInformation.getInputs();
        for (const auto& ipt : ipts) {
            for (const auto& isrc : ipt->input_sources) {
                subs << isrc.fed_id << ':' << isrc.handle << ';';
            }
        }
        ipts.unlock();
        unlock();
        auto str = subs.str();
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
        nlohmann::json base;
        addHeader(base);
        base["state"] = fedStateString(state.load());
        base["publications"] = publicationCount();
        base["input"] = inputCount();
        base["endpoints"] = endpointCount();
        base["granted_time"] = static_cast<double>(grantedTime());
        return fileops::generateJsonString(base);
    }
    if (query == "barriers") {
        nlohmann::json base;
        addHeader(base);

        base["barriers"] = nlohmann::json::array();
        for (const auto& barrier : timeCoord->getBarriers()) {
            nlohmann::json br1 = nlohmann::json::object();
            br1["id"] = barrier.second;
            br1["time"] = static_cast<double>(barrier.first);
            base["barriers"].push_back(std::move(br1));
        }
        return fileops::generateJsonString(base);
    }
    if (query == "global_state") {
        nlohmann::json base;
        addHeader(base);
        base["state"] = fedStateString(state.load());
        return fileops::generateJsonString(base);
    }
    if (query == "global_time_debugging") {
        nlohmann::json base;
        addHeader(base);
        base["federate_state"] = fedStateString(state.load());
        base["granted_mode"] = timeGranted_mode;
        if (!timeCoord->empty()) {
            timeCoord->generateDebuggingTimeInfo(base);
        }
        return fileops::generateJsonString(base);
    }
    if (query == "timeconfig") {
        nlohmann::json base;
        timeCoord->generateConfig(base);
        generateConfig(base);
        return fileops::generateJsonString(base);
    }
    if (query == "config") {
        nlohmann::json base;
        timeCoord->generateConfig(base);
        generateConfig(base);
        interfaceInformation.generateInferfaceConfig(base);
        addFederateTags(base, this);
        return fileops::generateJsonString(base);
    }
    if (query == "unconnected_interfaces") {
        nlohmann::json base;
        addHeader(base);
        interfaceInformation.getUnconnectedInterfaces(base);

        if (!tags.empty()) {
            nlohmann::json tagBlock = nlohmann::json::object();
            for (const auto& tag : tags) {
                tagBlock[tag.first] = tag.second;
            }
            base["tags"] = tagBlock;
        }
        if (!queryCallbacks.empty()) {
            for (const auto& queryCallback : queryCallbacks) {
                if (!queryCallback) {
                    continue;
                }
                auto potential = queryCallback("potential_interfaces");
                if (!potential.empty()) {
                    try {
                        auto json = fileops::loadJsonStr(potential);

                        if (!json.contains("error")) {
                            base["potential_interfaces"] = json;
                            break;
                        }
                    }
                    catch (const std::invalid_argument&) {
                        ;
                    }
                }
            }
        }
        return fileops::generateJsonString(base);
    }
    if (query == "tags") {
        nlohmann::json tagBlock = nlohmann::json::object();
        for (const auto& tag : tags) {
            tagBlock[tag.first] = tag.second;
        }
        return fileops::generateJsonString(tagBlock);
    }
    if (query.compare(0, 4, "tag/") == 0) {
        std::string_view keyTag = query;
        keyTag.remove_prefix(4);
        for (const auto& tag : tags) {
            if (keyTag == tag.first) {
                return generateJsonQuotedString(tag.second);
            }
        }
        return "\"\"";
    }
    if (query == "dependents") {
        return generateStringVector(timeCoord->getDependents(),
                                    [](auto& dep) { return std::to_string(dep.baseValue()); });
    }
    if (query == "logs") {
        nlohmann::json base;
        addHeader(base);
        bufferToJson(mLogManager->getLogBuffer(), base);
        return fileops::generateJsonString(base);
    }
    if (query == "data_flow_graph") {
        nlohmann::json base;
        addHeader(base);
        interfaceInformation.generateDataFlowGraph(base);
        return fileops::generateJsonString(base);
    }
    if (query == "global_time" || query == "global_status") {
        nlohmann::json base;
        addHeader(base);
        base["granted_time"] = static_cast<double>(timeCoord->getGrantedTime());
        base["send_time"] = static_cast<double>(timeCoord->allowedSendTime());
        return fileops::generateJsonString(base);
    }
    if (query == "dependency_graph") {
        nlohmann::json base;
        addHeader(base);
        base["dependents"] = nlohmann::json::array();
        for (auto& dep : timeCoord->getDependents()) {
            base["dependents"].push_back(dep.baseValue());
        }
        base["dependencies"] = nlohmann::json::array();
        for (auto& dep : timeCoord->getDependencies()) {
            base["dependencies"].push_back(dep.baseValue());
        }
        return fileops::generateJsonString(base);
    }

    if (!queryCallbacks.empty()) {
        for (const auto& queryCallback : queryCallbacks) {
            if (!queryCallback) {
                continue;
            }
            auto val = queryCallback(query);
            if (!val.empty()) {
                return val;
            }
        }
    }
    // check existingTag value for a matching string
    for (const auto& tag : tags) {
        if (tag.first == query) {
            return generateJsonQuotedString(tag.second);
        }
    }
    return generateJsonErrorResponse(JsonErrorCodes::BAD_REQUEST, "unrecognized Federate query");
}

std::string FederateState::processQuery(std::string_view query, bool force_ordering) const
{
    std::string qstring;
    if (!force_ordering &&
        (query == "publications" || query == "inputs" || query == "endpoints" ||
         query == "global_state")) {  // these never need to be locked
        qstring = processQueryActual(query);
    } else if ((query == "queries") || (query == "available_queries")) {
        qstring =
            R"("publications","inputs","logs","endpoints","subscriptions","current_state","global_state","dependencies","timeconfig","config","dependents","current_time","global_time","global_status","unconnected_interfaces")";
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

void FederateState::setTag(std::string_view tag, std::string_view value)
{
    spinlock();
    for (auto& testTag : tags) {
        if (testTag.first == tag) {
            unlock();
            testTag.second = value;
            return;
        }
    }
    tags.emplace_back(tag, value);
    unlock();
}

const std::string& FederateState::getTag(std::string_view tag) const
{
    spinlock();
    for (const auto& existingTag : tags) {
        if (existingTag.first == tag) {
            unlock();
            return existingTag.second;
        }
    }
    unlock();
    return gHelicsEmptyStr;
}
}  // namespace helics
