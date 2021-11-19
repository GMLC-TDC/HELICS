/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "TimeCoordinator.hpp"

#include "../common/fmt_format.h"
#include "flagOperations.hpp"
#include "helics_definitions.hpp"

#include "json/json.h"
#include <algorithm>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace helics {

static const Time bigTime(HELICS_BIG_NUMBER);

static auto nullMessageFunction = [](const ActionMessage& /*unused*/) {};
TimeCoordinator::TimeCoordinator(): sendMessageFunction(nullMessageFunction) {}

TimeCoordinator::TimeCoordinator(std::function<void(const ActionMessage&)> userSendMessageFunction):
    sendMessageFunction(std::move(userSendMessageFunction))
{
    if (!sendMessageFunction) {
        sendMessageFunction = nullMessageFunction;
    }
}

void TimeCoordinator::setMessageSender(
    std::function<void(const ActionMessage&)> userSendMessageFunction)
{
    sendMessageFunction = std::move(userSendMessageFunction);
    if (!sendMessageFunction) {
        sendMessageFunction = nullMessageFunction;
    }
}

void TimeCoordinator::enteringExecMode(IterationRequest mode)
{
    if (executionMode) {
        return;
    }
    iterating = mode;
    checkingExec = true;
    ActionMessage execreq(CMD_EXEC_REQUEST);
    execreq.source_id = source_id;
    if (iterating != IterationRequest::NO_ITERATIONS) {
        setIterationFlags(execreq, iterating);
    }
    if (info.wait_for_current_time_updates) {
        setActionFlag(execreq, delayed_timing_flag);
    }
    transmitTimingMessages(execreq);
}

void TimeCoordinator::disconnect()
{
    if (sendMessageFunction) {
        if (dependencies.empty()) {
            return;
        }
        ActionMessage bye(CMD_DISCONNECT);
        bye.source_id = source_id;
        if (dependencies.size() == 1) {
            auto& dep = *dependencies.begin();
            if ((dep.dependency && dep.next < Time::maxVal()) || dep.dependent) {
                bye.dest_id = dep.fedID;
                if (bye.dest_id == source_id) {
                    processTimeMessage(bye);
                } else {
                    sendMessageFunction(bye);
                }
            }

        } else {
            ActionMessage multi(CMD_MULTI_MESSAGE);
            for (auto dep : dependencies) {
                if ((dep.dependency && dep.next < Time::maxVal()) || dep.dependent) {
                    bye.dest_id = dep.fedID;
                    if (dep.fedID == source_id) {
                        processTimeMessage(bye);
                    } else {
                        appendMessage(multi, bye);
                    }
                }
            }
            sendMessageFunction(multi);
        }
    }
    disconnected = true;
}

void TimeCoordinator::localError()
{
    if (disconnected) {
        return;
    }
    time_granted = Time::maxVal();
    time_grantBase = Time::maxVal();
    if (sendMessageFunction) {
        if (dependencies.empty()) {
            return;
        }
        ActionMessage bye(CMD_LOCAL_ERROR);

        bye.source_id = source_id;
        if (dependencies.size() == 1) {
            auto& dep = *dependencies.begin();
            if ((dep.dependency && dep.next < Time::maxVal()) || dep.dependent) {
                bye.dest_id = dep.fedID;
                if (bye.dest_id == source_id) {
                    processTimeMessage(bye);
                } else {
                    sendMessageFunction(bye);
                }
            }

        } else {
            ActionMessage multi(CMD_MULTI_MESSAGE);
            for (auto dep : dependencies) {
                if ((dep.dependency && dep.next < Time::maxVal()) || dep.dependent) {
                    bye.dest_id = dep.fedID;
                    if (dep.fedID == source_id) {
                        processTimeMessage(bye);
                    } else {
                        appendMessage(multi, bye);
                    }
                }
            }
            sendMessageFunction(multi);
        }
    }
    disconnected = true;
}

void TimeCoordinator::timeRequest(Time nextTime,
                                  IterationRequest iterate,
                                  Time newValueTime,
                                  Time newMessageTime)
{
    iterating = iterate;

    if (iterating != IterationRequest::NO_ITERATIONS) {
        if (nextTime < time_granted || iterating == IterationRequest::FORCE_ITERATION) {
            nextTime = time_granted;
        }
    } else {
        time_next = getNextPossibleTime();
        if (nextTime < time_next) {
            nextTime = time_next;
        }
        if (info.uninterruptible) {
            time_next = generateAllowedTime(nextTime);
        }
    }
    time_requested = nextTime;
    if (iterating != IterationRequest::NO_ITERATIONS) {
        time_value = (newValueTime > time_granted) ? newValueTime : time_granted;
        time_message = (newMessageTime > time_granted) ? newMessageTime : time_granted;
    } else {
        time_value = (newValueTime > time_next) ? newValueTime : time_next;
        time_message = (newMessageTime > time_next) ? newMessageTime : time_next;
    }

    time_exec = std::min({time_value, time_message, time_requested});
    if (info.uninterruptible) {
        if (time_exec > time_granted || iterating == IterationRequest::NO_ITERATIONS) {
            time_exec = time_requested;
        }
    }
    dependencies.resetDependentEvents(time_granted);
    updateTimeFactors();

    if (!dependencies.empty()) {
        sendTimeRequest();
    }
}

bool TimeCoordinator::updateNextExecutionTime()
{
    auto cexec = time_exec;
    if (info.uninterruptible) {
        if (iterating == IterationRequest::NO_ITERATIONS) {
            time_exec = generateAllowedTime(time_requested);
        } else {
            time_exec = std::min(time_message, time_value);
            if (time_exec < Time::maxVal()) {
                time_exec += info.inputDelay;
            }
            time_exec =
                (time_exec <= time_granted) ? time_granted : generateAllowedTime(time_requested);
        }
    } else {
        time_exec = std::min(time_message, time_value);
        if (time_exec < Time::maxVal()) {
            time_exec += info.inputDelay;
        }
        time_exec = std::min(time_requested, time_exec);
        if (time_exec <= time_granted) {
            time_exec = (iterating == IterationRequest::NO_ITERATIONS) ? getNextPossibleTime() :
                                                                         time_granted;
        }
        if (time_granted < Time::maxVal()) {
            if ((time_exec - time_granted) > timeZero) {
                time_exec = generateAllowedTime(time_exec);
            }
        } else {
            time_exec = generateAllowedTime(time_exec);
        }
    }

    return (time_exec != cexec);
}

void TimeCoordinator::updateNextPossibleEventTime()
{
    time_next =
        (iterating == IterationRequest::NO_ITERATIONS) ? getNextPossibleTime() : time_granted;

    if (info.uninterruptible) {
        if (iterating == IterationRequest::NO_ITERATIONS) {
            time_next = generateAllowedTime(time_requested) + info.outputDelay;
        } else {
            if (time_minminDe < Time::maxVal() && !info.restrictive_time_policy) {
                if (time_minminDe + info.inputDelay > time_next) {
                    time_next = generateAllowedTime(time_requested);
                }
            }
            time_next = std::min(time_next, time_exec) + info.outputDelay;
        }
    } else {
        if (time_minminDe < Time::maxVal() && !info.restrictive_time_policy) {
            if (time_minminDe + info.inputDelay > time_next) {
                time_next = time_minminDe + info.inputDelay;
                time_next = generateAllowedTime(time_next);
            }
        }
        time_next = std::min(time_next, time_exec) + info.outputDelay;
    }
}

void TimeCoordinator::updateValueTime(Time valueUpdateTime, bool allowRequestSend)
{
    if (!executionMode)  // updates before exec mode
    {
        if (valueUpdateTime < timeZero) {
            hasInitUpdates = true;
        }
        return;
    }
    if (valueUpdateTime < time_value) {
        auto ptime = time_value;
        if (iterating != IterationRequest::NO_ITERATIONS) {
            time_value = (valueUpdateTime <= time_granted) ? time_granted : valueUpdateTime;
        } else {
            auto nextPossibleTime = getNextPossibleTime();
            if (valueUpdateTime <= nextPossibleTime) {
                time_value = nextPossibleTime;
            } else {
                time_value = valueUpdateTime;
            }
        }
        if (time_value < ptime && !disconnected) {
            if (updateNextExecutionTime()) {
                if (allowRequestSend) {
                    sendTimeRequest();
                }
            }
        }
    }
}

void TimeCoordinator::generateConfig(Json::Value& base) const
{
    base["uninterruptible"] = info.uninterruptible;
    base["wait_for_current_time_updates"] = info.wait_for_current_time_updates;
    base["restrictive_time_policy"] = info.restrictive_time_policy;
    base["event_triggered"] = info.event_triggered;
    base["max_iterations"] = info.maxIterations;

    if (info.period > timeZero) {
        base["period"] = static_cast<double>(info.period);
    }
    if (info.offset != timeZero) {
        base["offset"] = static_cast<double>(info.offset);
    }
    if (info.timeDelta > Time::epsilon()) {
        base["time_delta"] = static_cast<double>(info.timeDelta);
    }
    if (info.outputDelay > timeZero) {
        base["output_delay"] = static_cast<double>(info.outputDelay);
    }
    if (info.inputDelay > timeZero) {
        base["intput_delay"] = static_cast<double>(info.inputDelay);
    }
}

void TimeCoordinator::generateDebuggingTimeInfo(Json::Value& base) const
{
    generateConfig(base);
    base["granted"] = static_cast<double>(time_granted);
    base["requested"] = static_cast<double>(time_requested);
    base["exec"] = static_cast<double>(time_exec);
    base["allow"] = static_cast<double>(time_allow);
    base["value"] = static_cast<double>(time_value);
    base["message"] = static_cast<double>(time_message);
    base["minde"] = static_cast<double>(time_minDe);
    base["minminde"] = static_cast<double>(time_minminDe);

    Json::Value upBlock;
    generateJsonOutputTimeData(upBlock, upstream);

    base["upstream"] = upBlock;
    Json::Value tblock;
    generateJsonOutputTimeData(tblock, total);

    base["total"] = tblock;

    Json::Value sent;
    generateJsonOutputTimeData(sent, lastSend);

    base["last_send"] = sent;
    base["dependencies"] = Json::arrayValue;
    for (const auto& dep : dependencies) {
        if (dep.dependency) {
            Json::Value depblock;
            generateJsonOutputDependency(depblock, dep);
            base["dependencies"].append(depblock);
        }
        if (dep.dependent) {
            base["dependents"].append(dep.fedID.baseValue());
        }
    }
    // now add any time blocks that may be present
    base["blocks"] = Json::arrayValue;
    for (const auto& blk : timeBlocks) {
        Json::Value timeblock;
        timeblock["time"] = static_cast<double>(blk.first);
        timeblock["id"] = blk.second;
        base["blocks"].append(timeblock);
    }
}

bool TimeCoordinator::hasActiveTimeDependencies() const
{
    return dependencies.hasActiveTimeDependencies();
}

int TimeCoordinator::dependencyCount() const
{
    return dependencies.activeDependencyCount();
}

/** get a count of the active dependencies*/
GlobalFederateId TimeCoordinator::getMinDependency() const
{
    return dependencies.getMinDependency();
}

void TimeCoordinator::enterInitialization()
{
    if (dynamicJoining) {
        ActionMessage timeUpdateRequest(CMD_REQUEST_CURRENT_TIME);
        timeUpdateRequest.source_id = source_id;
        for (const auto& dep : dependencies) {
            // send to all dependencies
            if (dep.dependency) {
                if (dep.fedID == source_id) {
                    continue;
                }
                timeUpdateRequest.dest_id = dep.fedID;
                sendMessageFunction(timeUpdateRequest);
            }
        }
    }
}

void TimeCoordinator::requestTimeCheck()
{
    if (dynamicJoining) {
        ActionMessage timeUpdateRequest(CMD_REQUEST_CURRENT_TIME);
        timeUpdateRequest.source_id = source_id;
        for (const auto& dep : dependencies) {
            // send to all dependencies
            if (dep.dependency) {
                if (dep.fedID == source_id) {
                    continue;
                }
                // only send the request if it is blocking the current grant
                if (dep.next < time_exec) {
                    timeUpdateRequest.dest_id = dep.fedID;
                    sendMessageFunction(timeUpdateRequest);
                }
            }
        }
    }
}

Time TimeCoordinator::getNextPossibleTime() const
{
    if (time_granted == timeZero) {
        if (info.offset > info.timeDelta) {
            return info.offset;
        }
        if (info.offset == timeZero) {
            return generateAllowedTime(std::max(info.timeDelta, info.period));
        }
        if (info.period <= Time::epsilon()) {
            return info.timeDelta;
        }
        Time retTime = info.offset + info.period;
        while (retTime < info.timeDelta) {
            retTime += info.period;
        }
        return retTime;
    }
    if (time_grantBase >= Time::maxVal() - std::max(info.timeDelta, info.period)) {
        return Time::maxVal();
    }
    return generateAllowedTime(time_grantBase + std::max(info.timeDelta, info.period));
}

Time TimeCoordinator::generateAllowedTime(Time testTime) const
{
    if (info.period > timeEpsilon) {
        if (testTime == Time::maxVal()) {
            return testTime;
        }
        auto timeBase = time_grantBase;
        if (time_grantBase < info.offset) {
            timeBase = info.offset;
            if (testTime <= info.offset) {
                return info.offset;
            }
        }
        if (testTime - timeBase > info.period) {
            auto blk = std::ceil((testTime - timeBase) / info.period);
            testTime = timeBase + blk * info.period;
        } else {
            testTime = timeBase + info.period;
        }
    }
    return testTime;
}

void TimeCoordinator::updateMessageTime(Time messageUpdateTime, bool allowRequestSend)
{
    if (!executionMode)  // updates before exec mode
    {
        if (messageUpdateTime < timeZero) {
            hasInitUpdates = true;
        }
        return;
    }

    if (messageUpdateTime < time_message) {
        auto ptime = time_message;
        if (iterating != IterationRequest::NO_ITERATIONS) {
            time_message = (messageUpdateTime <= time_granted) ? time_granted : messageUpdateTime;
        } else {
            auto nextPossibleTime = getNextPossibleTime();
            if (messageUpdateTime <= nextPossibleTime) {
                time_message = nextPossibleTime;
            } else {
                time_message = messageUpdateTime;
            }
        }
        if (time_message < ptime && !disconnected) {
            if (updateNextExecutionTime()) {
                if (allowRequestSend) {
                    sendTimeRequest();
                }
            }
        }
    }
}

bool TimeCoordinator::updateTimeFactors()
{
    total = generateMinTimeTotal(dependencies, info.restrictive_time_policy, GlobalFederateId{});
    upstream =
        generateMinTimeUpstream(dependencies, info.restrictive_time_policy, GlobalFederateId{});

    maxTime = Time::maxVal() - info.outputDelay - (std::max)(info.period, info.timeDelta);
    bool update = false;
    time_minminDe = total.minDe;
    Time prev_next = time_next;
    updateNextPossibleEventTime();

    //    printf("%d UPDATE next=%f, minminDE=%f, Tdemin=%f\n", source_id,
    //    static_cast<double>(time_next),
    // static_cast<double>(minminDe), static_cast<double>(minDe));
    if (prev_next != time_next) {
        update = true;
    }
    if (total.minDe < maxTime) {
        total.minDe = generateAllowedTime(total.minDe) + info.outputDelay;
    }
    if (upstream.minDe < maxTime && upstream.minDe > total.minDe) {
        upstream.minDe = generateAllowedTime(upstream.minDe) + info.outputDelay;
    }
    if (info.event_triggered || time_requested >= bigTime) {
        if (upstream.Te < maxTime) {
            upstream.Te = generateAllowedTime(upstream.minDe);
        }
    }
    if (total.minDe != time_minDe) {
        update = true;
        time_minDe = total.minDe;
    }
    time_allow = (total.next < maxTime) ? info.inputDelay + total.next : Time::maxVal();

    updateNextExecutionTime();
    return update;
}

MessageProcessingResult TimeCoordinator::checkTimeGrant()
{
    updateTimeFactors();
    if (time_exec == Time::maxVal()) {
        if (time_allow == Time::maxVal()) {
            time_granted = Time::maxVal();
            time_grantBase = Time::maxVal();
            disconnect();
            return MessageProcessingResult::HALTED;
        }
    }
    if ((time_block <= time_exec && time_block < Time::maxVal()) ||
        (nonGranting && time_exec < time_requested)) {
        return MessageProcessingResult::CONTINUE_PROCESSING;
    }
    if ((iterating == IterationRequest::NO_ITERATIONS) ||
        (time_exec > time_granted && iterating == IterationRequest::ITERATE_IF_NEEDED)) {
        iteration = 0;
        if (time_allow > time_exec) {
            updateTimeGrant();
            return MessageProcessingResult::NEXT_STEP;
        }
        if (time_allow == time_exec) {
            if (!info.wait_for_current_time_updates) {
                if (time_requested <= time_exec) {
                    // this is the non interrupted case
                    updateTimeGrant();
                    return MessageProcessingResult::NEXT_STEP;
                }
                if (dependencies.checkIfReadyForTimeGrant(false, time_exec)) {
                    updateTimeGrant();
                    return MessageProcessingResult::NEXT_STEP;
                }
            } else {
                // if the wait_for_current_time_updates flag is set then time_allow must be greater
                // than time_exec
            }
        }
    } else {
        if (time_allow > time_exec) {
            ++iteration;
            updateTimeGrant();
            return MessageProcessingResult::ITERATING;
        }
        if (time_allow == time_exec)  // time_allow==time_exec==time_granted
        {
            if (dependencies.checkIfReadyForTimeGrant(true, time_exec)) {
                ++iteration;
                updateTimeGrant();
                return MessageProcessingResult::ITERATING;
            }
        }
    }

    // if we haven't returned we may need to update the time messages
    if ((!dependencies.empty())) {
        sendTimeRequest();
    }
    return MessageProcessingResult::CONTINUE_PROCESSING;
}

bool TimeCoordinator::checkAndSendTimeRequest(ActionMessage& upd, GlobalFederateId skipFed) const
{
    bool changed{false};
    if (lastSend.next != upd.actionTime) {
        changed = true;
    }
    if (lastSend.minDe != upd.Tdemin) {
        changed = true;
    }
    if (lastSend.Te != upd.Te) {
        changed = true;
    }
    if (lastSend.minFed != GlobalFederateId(upd.getExtraData())) {
        changed = true;
    }
    if (lastSend.time_state != time_state_t::time_requested) {
        changed = true;
    }
    if (changed) {
        lastSend.next = upd.actionTime;
        lastSend.minDe = upd.Tdemin;
        lastSend.Te = upd.Te;
        lastSend.minFed = GlobalFederateId(upd.getExtraData());
        lastSend.time_state = time_state_t::time_requested;
        return transmitTimingMessages(upd, skipFed);
    }
    return false;
}

static inline Time checkAdd(Time val, Time increment)
{
    return (val < Time::maxVal() - increment) ? val + increment : Time::maxVal();
}

void TimeCoordinator::sendTimeRequest() const
{
    ActionMessage upd(CMD_TIME_REQUEST);
    upd.source_id = source_id;
    upd.actionTime = time_next;
    if (nonGranting) {
        setActionFlag(upd, non_granting_flag);
    }
    if (info.wait_for_current_time_updates) {
        setActionFlag(upd, delayed_timing_flag);
    }
    upd.Te = checkAdd(time_exec, info.outputDelay);
    if (info.event_triggered || time_requested >= bigTime) {
        upd.Te = std::min(upd.Te, checkAdd(upstream.Te, info.outputDelay));
        upd.actionTime = std::min(upd.actionTime, upd.Te);
    }
    upd.Tdemin = std::min(checkAdd(upstream.Te, info.outputDelay), upd.Te);
    if (info.event_triggered || time_requested >= bigTime) {
        upd.Tdemin = std::min(upd.Tdemin, checkAdd(upstream.minDe, info.outputDelay));

        if (upd.Tdemin < upd.actionTime) {
            upd.actionTime = upd.Tdemin;
        }
    }
    upd.setExtraData(upstream.minFed.baseValue());

    if (upd.Tdemin < upd.actionTime) {
        upd.Tdemin = upd.actionTime;
    }

    if (iterating != IterationRequest::NO_ITERATIONS) {
        setIterationFlags(upd, iterating);
        upd.counter = iteration;
    }
    if (checkAndSendTimeRequest(upd, upstream.minFed)) {
        if (upstream.minFed.isValid()) {
            upd.dest_id = upstream.minFed;
            upd.setExtraData(GlobalFederateId{}.baseValue());
            if (info.event_triggered || time_requested >= bigTime) {
                upd.Te = checkAdd(time_exec, info.outputDelay);
                upd.Te = std::min(upd.Te, checkAdd(upstream.TeAlt, info.outputDelay));
            }
            upd.Tdemin = std::min(upstream.TeAlt, upd.Te);
            sendMessageFunction(upd);
        }
    }

    //    printf("%d next=%f, exec=%f, Tdemin=%f\n", source_id, static_cast<double>(time_next),
    // static_cast<double>(time_exec), static_cast<double>(time_minDe));
}

void TimeCoordinator::updateTimeGrant()
{
    if (iterating != IterationRequest::FORCE_ITERATION) {
        time_granted = time_exec;
        time_grantBase = time_granted;
    }
    ActionMessage treq(CMD_TIME_GRANT);
    treq.source_id = source_id;
    treq.actionTime = time_granted;
    treq.counter = iteration;
    if (iterating != IterationRequest::NO_ITERATIONS) {
        dependencies.resetIteratingTimeRequests(time_exec);
    }
    lastSend.next = treq.actionTime;
    lastSend.Te = treq.actionTime;
    lastSend.minDe = treq.actionTime;
    lastSend.time_state = time_state_t::time_granted;
    transmitTimingMessages(treq);
    // printf("%d GRANT allow=%f next=%f, exec=%f, Tdemin=%f\n", source_id,
    // static_cast<double>(time_allow), static_cast<double>(time_next),
    // static_cast<double>(time_exec), static_cast<double>(time_minDe));
}
std::string TimeCoordinator::printTimeStatus() const
{
    return fmt::format(
        R"raw({{"granted_time":{},"requested_time":{}, "exec":{}, "allow":{}, "value":{}, "message":{}, "minDe":{}, "minminDe":{}}})raw",
        static_cast<double>(time_granted),
        static_cast<double>(time_requested),
        static_cast<double>(time_exec),
        static_cast<double>(time_allow),
        static_cast<double>(time_value),
        static_cast<double>(time_message),
        static_cast<double>(time_minDe),
        static_cast<double>(time_minminDe));
}

bool TimeCoordinator::isDependency(GlobalFederateId ofed) const
{
    return dependencies.isDependency(ofed);
}

bool TimeCoordinator::addDependency(GlobalFederateId fedID)
{
    if (dependencies.addDependency(fedID)) {
        if (fedID == source_id) {
            auto* dep = dependencies.getDependencyInfo(fedID);
            if (dep != nullptr) {
                dep->connection = ConnectionType::self;
            }
        }

        dependency_federates.lock()->push_back(fedID);
        return true;
    }
    return false;
}

bool TimeCoordinator::addDependent(GlobalFederateId fedID)
{
    if (dependencies.addDependent(fedID)) {
        dependent_federates.lock()->push_back(fedID);
        return true;
    }
    return false;
}

void TimeCoordinator::setAsChild(GlobalFederateId fedID)
{
    if (fedID == source_id) {
        return;
    }
    auto* dep = dependencies.getDependencyInfo(fedID);
    if (dep != nullptr) {
        dep->connection = ConnectionType::child;
    }
}

void TimeCoordinator::setAsParent(GlobalFederateId fedID)
{
    if (fedID == source_id) {
        return;
    }
    auto* dep = dependencies.getDependencyInfo(fedID);
    if (dep != nullptr) {
        dep->connection = ConnectionType::parent;
    }
}

void TimeCoordinator::removeDependency(GlobalFederateId fedID)
{
    dependencies.removeDependency(fedID);
    // remove the thread safe version
    auto dlock = dependency_federates.lock();
    auto res = std::find(dlock.begin(), dlock.end(), fedID);
    if (res != dlock.end()) {
        dlock->erase(res);
    }
}

void TimeCoordinator::removeDependent(GlobalFederateId fedID)
{
    dependencies.removeDependent(fedID);
    // remove the thread safe version
    auto dlock = dependent_federates.lock();
    auto res = std::find(dlock.begin(), dlock.end(), fedID);
    if (res != dlock.end()) {
        dlock->erase(res);
    }
}

DependencyInfo* TimeCoordinator::getDependencyInfo(GlobalFederateId ofed)
{
    return dependencies.getDependencyInfo(ofed);
}

std::vector<GlobalFederateId> TimeCoordinator::getDependencies() const
{
    return *dependency_federates.lock_shared();
}

bool TimeCoordinator::transmitTimingMessages(ActionMessage& msg, GlobalFederateId skipFed) const
{
    bool skipped{false};
    for (const auto& dep : dependencies) {
        if (dep.dependent) {
            if (dep.fedID == skipFed) {
                skipped = true;
                continue;
            }
            msg.dest_id = dep.fedID;
            sendMessageFunction(msg);
        }
    }
    return skipped;
}

MessageProcessingResult TimeCoordinator::checkExecEntry()
{
    auto ret = MessageProcessingResult::CONTINUE_PROCESSING;
    if (time_block <= timeZero) {
        return ret;
    }
    if (!dependencies.checkIfReadyForExecEntry(iterating != IterationRequest::NO_ITERATIONS)) {
        return ret;
    }

    // check for timing deadlock with wait_for_current_time_flag
    if (info.wait_for_current_time_updates) {
        for (auto& dep : dependencies) {
            if (dep.dependency && dep.dependent && dep.delayedTiming && dep.fedID != source_id) {
                ActionMessage ge(CMD_GLOBAL_ERROR);
                ge.dest_id = parent_broker_id;
                ge.source_id = source_id;
                ge.messageID = multiple_wait_for_current_time_flags;
                ge.payload =
                    "Multiple federates declaring wait_for_current_time flag will result in deadlock";
                sendMessageFunction(ge);
                return MessageProcessingResult::ERROR_RESULT;
            }
        }
    }

    switch (iterating) {
        case IterationRequest::NO_ITERATIONS:
            if (!info.wait_for_current_time_updates) {
                ret = MessageProcessingResult::NEXT_STEP;
            } else {
                total = generateMinTimeTotal(dependencies, info.restrictive_time_policy, source_id);
                if (total.next > timeZero) {
                    ret = MessageProcessingResult::NEXT_STEP;
                }
            }
            break;
        case IterationRequest::ITERATE_IF_NEEDED:
            if (hasInitUpdates) {
                if (iteration >= info.maxIterations) {
                    ret = MessageProcessingResult::NEXT_STEP;
                } else {
                    ret = MessageProcessingResult::ITERATING;
                }
            } else {
                ret = MessageProcessingResult::NEXT_STEP;
            }
            break;
        case IterationRequest::FORCE_ITERATION:
            ret = MessageProcessingResult::ITERATING;
            break;
    }
    if (!dynamicJoining) {
        if (ret == MessageProcessingResult::NEXT_STEP) {
            time_granted = timeZero;
            time_grantBase = time_granted;
            executionMode = true;
            iteration = 0;

            ActionMessage execgrant(CMD_EXEC_GRANT);
            execgrant.source_id = source_id;
            transmitTimingMessages(execgrant);
        } else if (ret == MessageProcessingResult::ITERATING) {
            dependencies.resetIteratingExecRequests();
            hasInitUpdates = false;
            ++iteration;
            ActionMessage execgrant(CMD_EXEC_GRANT);
            execgrant.source_id = source_id;
            execgrant.counter = iteration;
            setActionFlag(execgrant, iteration_requested_flag);
            transmitTimingMessages(execgrant);
        }
    } else {
        if (ret == MessageProcessingResult::NEXT_STEP) {
            updateTimeFactors();
            if (dependencyCount() > 0) {
                time_granted =
                    generateAllowedTime(total.next) - (std::max)(info.period, info.timeDelta);
            } else {
                time_granted = timeZero;
            }
            time_grantBase = time_granted;
            executionMode = true;
            iteration = 0;

            ActionMessage execgrant(time_granted > timeZero ? CMD_TIME_GRANT : CMD_EXEC_GRANT);
            execgrant.source_id = source_id;
            execgrant.actionTime = time_granted;
            transmitTimingMessages(execgrant);
        }
    }
    return ret;
}

static bool isDelayableMessage(const ActionMessage& cmd, GlobalFederateId localId)
{
    return (((cmd.action() == CMD_TIME_GRANT) || (cmd.action() == CMD_EXEC_GRANT)) &&
            (cmd.source_id != localId));
}

std::pair<GlobalFederateId, Time> TimeCoordinator::getMinGrantedDependency() const
{
    Time minTime = Time::maxVal();
    GlobalFederateId minID;
    for (auto& dep : dependencies) {
        if (!dep.dependency) {
            continue;
        }
        if (dep.time_state != time_state_t::time_requested) {
            if (dep.next < minTime) {
                minTime = dep.next;
                minID = dep.fedID;
            }
        }
    }
    return {minID, minTime};
}

message_process_result TimeCoordinator::processTimeMessage(const ActionMessage& cmd)
{
    switch (cmd.action()) {
        case CMD_TIME_BLOCK:
        case CMD_TIME_UNBLOCK:
        case CMD_TIME_BARRIER:
        case CMD_TIME_BARRIER_CLEAR:
            return processTimeBlockMessage(cmd);
        case CMD_FORCE_TIME_GRANT:
            if (time_granted < cmd.actionTime) {
                time_granted = cmd.actionTime;
                time_grantBase = time_granted;

                ActionMessage treq(CMD_TIME_GRANT);
                treq.source_id = source_id;
                treq.actionTime = time_granted;
                lastSend.next = time_granted;
                lastSend.Te = time_granted;
                lastSend.minDe = time_granted;
                lastSend.time_state = time_state_t::time_granted;
                transmitTimingMessages(treq);
                return message_process_result::processed;
            }
            return message_process_result::no_effect;
        case CMD_DISCONNECT:
        case CMD_BROADCAST_DISCONNECT:
        case CMD_DISCONNECT_CORE:
        case CMD_DISCONNECT_FED:
        case CMD_DISCONNECT_BROKER:
        case CMD_GLOBAL_ERROR:
        case CMD_LOCAL_ERROR:
            // this command requires removing dependents as well as dealing with dependency
            // processing
            removeDependent(GlobalFederateId(cmd.source_id));
            break;
        case CMD_REQUEST_CURRENT_TIME:
            if (disconnected) {
                ActionMessage treq(CMD_DISCONNECT, source_id, cmd.source_id);
                sendMessageFunction(treq);
            } else if (lastSend.time_state == time_state_t::time_granted) {
                ActionMessage treq(CMD_TIME_GRANT, source_id, cmd.source_id);
                treq.actionTime = lastSend.next;
                sendMessageFunction(treq);
            } else {
                ActionMessage treq(CMD_TIME_REQUEST, source_id, cmd.source_id);
                treq.actionTime = lastSend.next;
                treq.Tdemin = lastSend.minDe;
                treq.Te = lastSend.Te;
                treq.setExtraData(lastSend.minFed.baseValue());
                sendMessageFunction(treq);
            }
            return message_process_result::processed;
        default:
            break;
    }
    if (isDelayableMessage(cmd, source_id)) {
        auto* dep = dependencies.getDependencyInfo(GlobalFederateId(cmd.source_id));
        if (dep == nullptr) {
            return message_process_result::no_effect;
        }
        switch (dep->time_state) {
            case time_state_t::time_requested:
                if (dep->next > time_exec) {
                    return message_process_result::delay_processing;
                }
                break;
            case time_state_t::time_requested_iterative:
                if (dep->next > time_exec) {
                    return message_process_result::delay_processing;
                }
                if ((iterating != IterationRequest::NO_ITERATIONS) && (time_exec == dep->next)) {
                    return message_process_result::delay_processing;
                }
                break;
            case time_state_t::exec_requested_iterative:
                if ((iterating != IterationRequest::NO_ITERATIONS) && (checkingExec)) {
                    return message_process_result::delay_processing;
                }
                break;
            default:
                break;
        }
    }
    return (dependencies.updateTime(cmd)) ? message_process_result::processed :
                                            message_process_result::no_effect;
}

Time TimeCoordinator::updateTimeBlocks(int32_t blockId, Time newTime)
{
    auto blk = std::find_if(timeBlocks.begin(), timeBlocks.end(), [blockId](const auto& block) {
        return (block.second == blockId);
    });
    if (blk != timeBlocks.end()) {
        blk->first = newTime;
    } else {
        timeBlocks.emplace_back(newTime, blockId);
    }
    auto res = std::min_element(timeBlocks.begin(),
                                timeBlocks.end(),
                                [](const auto& blk1, const auto& blk2) {
                                    return (blk1.first < blk2.first);
                                });
    return res->first;
}

message_process_result TimeCoordinator::processTimeBlockMessage(const ActionMessage& cmd)
{
    Time ltime = Time::maxVal();
    switch (cmd.action()) {
        case CMD_TIME_BLOCK:
        case CMD_TIME_BARRIER:
            ltime = updateTimeBlocks(cmd.messageID, cmd.actionTime);
            break;
        case CMD_TIME_UNBLOCK:
        case CMD_TIME_BARRIER_CLEAR:
            if (!timeBlocks.empty()) {
                ltime = updateTimeBlocks(cmd.messageID, Time::maxVal());
            }
            break;
        default:
            break;
    }
    if (ltime > time_block) {
        time_block = ltime;
        return message_process_result::processed;
    }
    time_block = ltime;
    return message_process_result::no_effect;
}

void TimeCoordinator::processDependencyUpdateMessage(const ActionMessage& cmd)
{
    bool added{false};
    switch (cmd.action()) {
        case CMD_ADD_DEPENDENCY:
            added = addDependency(cmd.source_id);
            break;
        case CMD_REMOVE_DEPENDENCY:
            removeDependency(cmd.source_id);
            break;
        case CMD_ADD_DEPENDENT:
            addDependent(cmd.source_id);
            break;
        case CMD_REMOVE_DEPENDENT:
            removeDependent(cmd.source_id);
            break;
        case CMD_ADD_INTERDEPENDENCY:
            added = addDependency(cmd.source_id);
            addDependent(cmd.source_id);
            break;
        case CMD_REMOVE_INTERDEPENDENCY:
            removeDependency(cmd.source_id);
            removeDependent(cmd.source_id);
            break;
        default:
            break;
    }
    if (added) {
        if (checkActionFlag(cmd, child_flag)) {
            setAsChild(cmd.source_id);
        }
        if (checkActionFlag(cmd, parent_flag)) {
            setAsParent(cmd.source_id);
        }
    }
}

/** set a timeProperty for a the coordinator*/
void TimeCoordinator::setProperty(int timeProperty, Time propertyVal)
{
    switch (timeProperty) {
        case defs::Properties::OUTPUT_DELAY:
            info.outputDelay = propertyVal;
            break;
        case defs::Properties::INPUT_DELAY:
            info.inputDelay = propertyVal;
            break;
        case defs::Properties::TIME_DELTA:
            info.timeDelta = propertyVal;
            if (info.timeDelta <= timeZero) {
                info.timeDelta = timeEpsilon;
            }
            break;
        case defs::Properties::PERIOD:
            info.period = propertyVal;
            break;
        case defs::Properties::OFFSET:
            info.offset = propertyVal;
            break;
        default:
            break;
    }
}

/** set a timeProperty for a the coordinator*/
void TimeCoordinator::setProperty(int intProperty, int propertyVal)
{
    if (intProperty == defs::Properties::MAX_ITERATIONS) {
        info.maxIterations = propertyVal;
    } else {
        setProperty(intProperty, Time(static_cast<double>(propertyVal)));
    }
}

/** set an option Flag for a the coordinator*/
void TimeCoordinator::setOptionFlag(int optionFlag, bool value)
{
    switch (optionFlag) {
        case defs::Flags::UNINTERRUPTIBLE:
            info.uninterruptible = value;
            break;
        case defs::Flags::WAIT_FOR_CURRENT_TIME_UPDATE:
            info.wait_for_current_time_updates = value;
            break;
        case defs::Flags::RESTRICTIVE_TIME_POLICY:
            info.restrictive_time_policy = value;
            break;
        case defs::Flags::EVENT_TRIGGERED:
            info.event_triggered = value;
            break;
        default:
            break;
    }
}
/** get a time Property*/
Time TimeCoordinator::getTimeProperty(int timeProperty) const
{
    switch (timeProperty) {
        case defs::Properties::OUTPUT_DELAY:
            return info.outputDelay;
        case defs::Properties::INPUT_DELAY:
            return info.inputDelay;
        case defs::Properties::TIME_DELTA:
            return info.timeDelta;
        case defs::Properties::PERIOD:
            return info.period;
        case defs::Properties::OFFSET:
            return info.offset;
        default:
            return Time::minVal();
    }
}

/** get a time Property*/
int TimeCoordinator::getIntegerProperty(int intProperty) const
{
    switch (intProperty) {  // NOLINT
        case defs::Properties::MAX_ITERATIONS:
            return info.maxIterations;
        default:
            return HELICS_INVALID_PROPERTY_VALUE;
    }
}

/** get an option flag value*/
bool TimeCoordinator::getOptionFlag(int optionFlag) const
{
    switch (optionFlag) {
        case defs::Flags::UNINTERRUPTIBLE:
            return info.uninterruptible;
        case defs::Flags::INTERRUPTIBLE:
            return !info.uninterruptible;
        case defs::Flags::WAIT_FOR_CURRENT_TIME_UPDATE:
            return info.wait_for_current_time_updates;
        case defs::Flags::RESTRICTIVE_TIME_POLICY:
            return info.restrictive_time_policy;
        case defs::Flags::EVENT_TRIGGERED:
            return info.event_triggered;
        default:
            throw(std::invalid_argument("flag not recognized"));
    }
}

void TimeCoordinator::processConfigUpdateMessage(const ActionMessage& cmd)
{
    switch (cmd.action()) {
        case CMD_FED_CONFIGURE_TIME:
            setProperty(cmd.messageID, cmd.actionTime);
            break;
        case CMD_FED_CONFIGURE_INT:
            setProperty(cmd.messageID, cmd.counter);
            break;
        case CMD_FED_CONFIGURE_FLAG:
            setOptionFlag(cmd.messageID, checkActionFlag(cmd, indicator_flag));
            break;
        default:
            break;
    }
}

}  // namespace helics
