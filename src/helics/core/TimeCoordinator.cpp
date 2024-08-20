/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "TimeCoordinator.hpp"

#include "flagOperations.hpp"
#include "helics_definitions.hpp"

#include <algorithm>
#include <fmt/format.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace helics {

void TimeCoordinator::enteringExecMode(IterationRequest mode)
{
    if (executionMode) {
        return;
    }
    iterating = mode;
    auto res = dependencies.checkForIssues(info.wait_for_current_time_updates);
    if (res.first != 0) {
        ActionMessage ge(CMD_GLOBAL_ERROR);
        ge.dest_id = parent_broker_id;
        ge.source_id = mSourceId;
        ge.messageID = res.first;
        ge.payload = res.second;
        sendMessageFunction(ge);
        return;
    }
    sendTimingInfo();
    checkingExec = true;
    ActionMessage execreq(CMD_EXEC_REQUEST);

    execreq.source_id = mSourceId;
    if (iterating != IterationRequest::NO_ITERATIONS) {
        setIterationFlags(execreq, iterating);
        ++sequenceCounter;
        execreq.counter = sequenceCounter;
        if (!hasInitUpdates) {
            const auto& mfed = getExecEntryMinFederate(dependencies, mSourceId);
            execreq.setExtraData(mfed.fedID.baseValue());
        }
    }
    if (info.wait_for_current_time_updates) {
        setActionFlag(execreq, delayed_timing_flag);
    }
    transmitTimingMessages(execreq);
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

        bye.source_id = mSourceId;
        if (dependencies.size() == 1) {
            auto& dep = *dependencies.begin();
            if ((dep.dependency && dep.next < Time::maxVal()) || dep.dependent) {
                bye.dest_id = dep.fedID;
                if (bye.dest_id == mSourceId) {
                    processTimeMessage(bye);
                } else {
                    sendMessageFunction(bye);
                }
            }

        } else {
            ActionMessage multi(CMD_MULTI_MESSAGE);
            for (const auto& dep : dependencies) {
                if ((dep.dependency && dep.next < Time::maxVal()) || dep.dependent) {
                    bye.dest_id = dep.fedID;
                    if (dep.fedID == mSourceId) {
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
    ++sequenceCounter;
    updateTimeFactors();

    if (!dependencies.empty()) {
        sendTimeRequest(GlobalFederateId{});
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
            if (time_exec > time_granted) {
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
    if (valueUpdateTime <= time_granted) {
        hasIterationData = true;
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
                    sendTimeRequest(GlobalFederateId{});
                }
            }
        }
    }
}

void TimeCoordinator::generateConfig(nlohmann::json& base) const
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

void TimeCoordinator::generateDebuggingTimeInfo(nlohmann::json& base) const
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
    nlohmann::json upBlock;
    generateJsonOutputTimeData(upBlock, upstream);

    base["upstream"] = upBlock;
    nlohmann::json tblock;
    generateJsonOutputTimeData(tblock, total);

    base["total"] = tblock;

    nlohmann::json sent;
    generateJsonOutputTimeData(sent, lastSend);

    base["last_send"] = sent;
    BaseTimeCoordinator::generateDebuggingTimeInfo(base);
    // now add any time blocks that may be present
    base["blocks"] = nlohmann::json::array();
    for (const auto& blk : timeBlocks) {
        nlohmann::json timeblock;
        timeblock["time"] = static_cast<double>(blk.first);
        timeblock["id"] = blk.second;
        base["blocks"].push_back(std::move(timeblock));
    }
}

void TimeCoordinator::enterInitialization()
{
    if (dynamicJoining) {
        ActionMessage timeUpdateRequest(CMD_REQUEST_CURRENT_TIME);
        timeUpdateRequest.source_id = mSourceId;
        for (const auto& dep : dependencies) {
            // send to all dependencies
            if (dep.dependency) {
                if (dep.fedID == mSourceId) {
                    continue;
                }
                timeUpdateRequest.dest_id = dep.fedID;
                sendMessageFunction(timeUpdateRequest);
            }
        }
    }
}

Time TimeCoordinator::getNextTime() const
{
    return getGrantedTime();
}

void TimeCoordinator::requestTimeCheck()
{
    if (dynamicJoining) {
        ActionMessage timeUpdateRequest(CMD_REQUEST_CURRENT_TIME);
        timeUpdateRequest.source_id = mSourceId;
        for (const auto& dep : dependencies) {
            // send to all dependencies
            if (dep.dependency) {
                if (dep.fedID == mSourceId) {
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
    if (messageUpdateTime <= time_granted) {
        hasIterationData = true;
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
                    sendTimeRequest(GlobalFederateId{});
                }
            }
        }
    }
}

bool TimeCoordinator::updateTimeFactors()
{
    total = generateMinTimeTotal(dependencies,
                                 info.restrictive_time_policy || globalTime,
                                 GlobalFederateId{},
                                 NoIgnoredFederates,
                                 sequenceCounter);
    upstream = generateMinTimeUpstream(dependencies,
                                       info.restrictive_time_policy || globalTime,
                                       GlobalFederateId{},
                                       NoIgnoredFederates,
                                       sequenceCounter);
    if (globalTime && dependencies.size() == 1) {
        upstream = total;
        upstream.minFed = GlobalFederateId{};
    }

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
    if (!globalTime && (info.event_triggered || time_requested >= cBigTime)) {
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

MessageProcessingResult TimeCoordinator::checkTimeGrant(GlobalFederateId triggerFed)
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
    if (time_block <= time_exec && time_block < Time::maxVal()) {
        if (triggerFed.isValid()) {
            if (triggerFed != mSourceId) {
                sendTimeRequest(triggerFed);
            }
        }
        return MessageProcessingResult::CONTINUE_PROCESSING;
    }
    if ((nonGranting && time_exec < time_requested)) {
        if (triggerFed.isValid()) {
            if (triggerFed != mSourceId) {
                sendTimeRequest(triggerFed);
            }
        }
        return MessageProcessingResult::CONTINUE_PROCESSING;
    }
    // if ((iterating == IterationRequest::NO_ITERATIONS) ||
    //     (time_exec > time_granted && iterating == IterationRequest::ITERATE_IF_NEEDED)) {
    //
    //     if (time_allow > time_exec) {
    //         iteration = 0;
    //         updateTimeGrant();
    //         return MessageProcessingResult::NEXT_STEP;
    //     }
    //     if (time_allow == time_exec) {
    //         if (!info.wait_for_current_time_updates) {
    //             if (time_requested <= time_exec) {
    //                 // this is the non interrupted case
    //                 iteration = 0;
    //                 updateTimeGrant();
    //                 return MessageProcessingResult::NEXT_STEP;
    //             }
    //             if (dependencies.checkIfReadyForTimeGrant(false, time_exec)) {
    //                 iteration = 0;
    //                 updateTimeGrant();
    //                 return MessageProcessingResult::NEXT_STEP;
    //             }
    //         } else {
    //             // if the wait_for_current_time_updates flag is set then time_allow must be
    //             greater
    //             // than time_exec
    //         }
    //     }
    // } else {
    //     if (time_allow > time_exec) {
    //         ++iteration;
    //         updateTimeGrant();
    //         return MessageProcessingResult::ITERATING;
    //     }
    //     if (time_allow == time_exec)  // time_allow==time_exec==time_granted
    //     {
    //         if (dependencies.checkIfReadyForTimeGrant(true, time_exec)) {
    //             ++iteration;
    //             updateTimeGrant();
    //             return MessageProcessingResult::ITERATING;
    //         }
    //     }
    // }
    auto ret = MessageProcessingResult::CONTINUE_PROCESSING;
    bool sendAll{needSendAll};
    needSendAll = false;
    switch (iterating) {
        case IterationRequest::NO_ITERATIONS:
            if (time_allow > time_exec) {
                iteration = 0;
                sequenceCounter = 0;
                hasIterationData = false;
                updateTimeGrant();
                return MessageProcessingResult::NEXT_STEP;
            }
            if (time_allow == time_exec) {
                if (!info.wait_for_current_time_updates) {
                    if (time_requested <= time_exec) {
                        // this is the non interrupted case
                        iteration = 0;
                        sequenceCounter = 0;
                        hasIterationData = false;
                        updateTimeGrant();
                        return MessageProcessingResult::NEXT_STEP;
                    }
                }

                auto delayMode =
                    getDelayMode(info.wait_for_current_time_updates, (time_requested > time_exec));
                if (dependencies.checkIfReadyForTimeGrant(false, time_exec, delayMode)) {
                    iteration = 0;
                    sequenceCounter = 0;
                    hasIterationData = false;
                    updateTimeGrant();
                    return MessageProcessingResult::NEXT_STEP;
                }

                // if the wait_for_current_time_updates flag is set then time_allow must be greater
                // than time_exec
            }
            break;
        case IterationRequest::ITERATE_IF_NEEDED:
        case IterationRequest::FORCE_ITERATION:
            if (time_allow > time_exec) {
                if (time_exec <= time_granted || hasIterationData) {
                    ++iteration;
                    hasIterationData = false;
                    updateTimeGrant();
                    return MessageProcessingResult::ITERATING;
                }
                if (iterating == IterationRequest::FORCE_ITERATION) {
                    ++iteration;
                } else {
                    iteration = 0;
                    sequenceCounter = 0;
                }

                updateTimeGrant();
                return (iterating == IterationRequest::FORCE_ITERATION) ?
                    MessageProcessingResult::ITERATING :
                    MessageProcessingResult::NEXT_STEP;
            }

            if (time_allow == time_exec) {
                if (time_allow == time_requested) {
                    if (!info.wait_for_current_time_updates) {
                        if (time_requested <= time_exec) {
                            // this is the non interrupted case
                            ret = MessageProcessingResult::NEXT_STEP;
                            break;
                        }
                        if (dependencies.checkIfReadyForTimeGrant(false,
                                                                  time_exec,
                                                                  GrantDelayMode::NONE)) {
                            ret = MessageProcessingResult::NEXT_STEP;
                            break;
                        }
                    }
                    ret = MessageProcessingResult::CONTINUE_PROCESSING;
                    break;
                }
                if (dependencies.checkIfReadyForTimeGrant(
                        true, time_exec, getDelayMode(info.wait_for_current_time_updates, false))) {
                    if (hasIterationData) {
                        ret = MessageProcessingResult::ITERATING;
                        break;
                    }
                    if (time_exec > time_granted) {
                        ret = MessageProcessingResult::NEXT_STEP;
                        break;
                    }
                    // time_
                    bool allowed{!info.wait_for_current_time_updates};
                    bool restricted{info.restrictive_time_policy};
                    bool restrictionAdvance{restricted};
                    int restrictionLevel{50};
                    if (allowed) {
                        for (const auto& dep : dependencies) {
                            if (!dep.dependency) {
                                continue;
                            }
                            if (dep.next > time_exec || dep.connection == ConnectionType::SELF) {
                                continue;
                            }

                            if (dep.minFed != mSourceId) {
                                allowed = false;
                            }
                            if (dep.responseSequenceCounter == sequenceCounter) {
                                if (restricted) {
                                    restrictionLevel =
                                        (std::min)(restrictionLevel,
                                                   static_cast<int>(dep.restrictionLevel));
                                }

                            } else {
                                restrictionAdvance = false;
                                allowed = false;
                                break;
                            }
                        }
                    }
                    if (allowed) {
                        if (restricted) {
                            if (restrictionLevel >= 1) {
                                ret = MessageProcessingResult::NEXT_STEP;
                            } else {
                                if (currentRestrictionLevel != restrictionLevel + 1) {
                                    currentRestrictionLevel = restrictionLevel + 1;
                                    sendAll = true;
                                    ++sequenceCounter;
                                }

                                ret = MessageProcessingResult::CONTINUE_PROCESSING;
                            }
                        } else {
                            ret = MessageProcessingResult::NEXT_STEP;
                        }

                    } else {
                        if (restrictionAdvance) {
                            currentRestrictionLevel = restrictionLevel + 1;
                            sendAll = true;
                            ++sequenceCounter;
                        }
                        ret = MessageProcessingResult::CONTINUE_PROCESSING;
                    }
                }
            }

            break;
        default:
            break;
    }

    switch (ret) {
        case MessageProcessingResult::CONTINUE_PROCESSING:
            if (triggerFed.isValid() || sendAll) {
                if (sendAll) {
                    sendTimeRequest(GlobalFederateId{});
                } else {
                    if (triggerFed != mSourceId) {
                        sendTimeRequest(triggerFed);
                    }
                }
            }
            break;
        case MessageProcessingResult::ITERATING:
            ++iteration;
            hasIterationData = false;
            updateTimeGrant();
            break;
        case MessageProcessingResult::NEXT_STEP:
            if (iterating == IterationRequest::FORCE_ITERATION) {
                ++iteration;
                hasIterationData = false;
                time_exec = time_granted;
                updateTimeGrant();
                ret = MessageProcessingResult::ITERATING;
            } else {
                iteration = 0;
                sequenceCounter = 0;
                hasIterationData = false;
                updateTimeGrant();
            }
            break;
        default:
            break;
    }

    return ret;
}

std::pair<bool, bool> TimeCoordinator::checkAndSendTimeRequest(ActionMessage& upd,
                                                               GlobalFederateId skipFed) const
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
    if (lastSend.mTimeState != TimeState::time_requested) {
        changed = true;
    }
    if (lastSend.sequenceCounter != sequenceCounter) {
        changed = true;
    }
    if (lastSend.interrupted != checkActionFlag(upd, interrupted_flag)) {
        changed = true;
    }
    if (changed) {
        lastSend.next = upd.actionTime;
        lastSend.minDe = upd.Tdemin;
        lastSend.Te = upd.Te;
        lastSend.sequenceCounter = sequenceCounter;
        lastSend.minFed = GlobalFederateId(upd.getExtraData());
        lastSend.mTimeState = TimeState::time_requested;
        lastSend.interrupted = checkActionFlag(upd, interrupted_flag);
        return {true, transmitTimingMessages(upd, skipFed)};
    }
    return {false, false};
}

static inline Time checkAdd(Time val, Time increment)
{
    return (val < Time::maxVal() - increment) ? val + increment : Time::maxVal();
}

void TimeCoordinator::sendTimeRequest(GlobalFederateId triggerFed) const
{
    ActionMessage upd(CMD_TIME_REQUEST);
    upd.source_id = mSourceId;
    upd.actionTime = time_next;
    upd.counter = sequenceCounter;
    if (nonGranting) {
        setActionFlag(upd, non_granting_flag);
    }
    if (info.wait_for_current_time_updates) {
        setActionFlag(upd, delayed_timing_flag);
    } else if (time_requested > time_next) {
        setActionFlag(upd, interrupted_flag);
    }
    upd.Te = checkAdd(time_exec, info.outputDelay);
    if (!globalTime && (info.event_triggered || time_requested >= cBigTime)) {
        upd.Te = std::min(upd.Te, checkAdd(upstream.Te, info.outputDelay));
        if (upd.Te < timeZero) {
            upd.Te = timeZero;
        }
        upd.actionTime = std::min(upd.actionTime, upd.Te);
    }
    upd.Tdemin = std::min(checkAdd(upstream.Te, info.outputDelay), upd.Te);
    if (!globalTime && (info.event_triggered || time_requested >= cBigTime)) {
        upd.Tdemin = std::min(upd.Tdemin, checkAdd(upstream.minDe, info.outputDelay));
        if (upd.Tdemin < timeZero) {
            upd.Tdemin = timeZero;
        }
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
    }
    upd.counter = sequenceCounter;
    if (triggered) {
        setActionFlag(upd, destination_target);
    }
    auto check = checkAndSendTimeRequest(upd, upstream.minFed);
    if (check.first) {
        if (check.second) {
            if (upstream.minFed.isValid()) {
                upd.dest_id = upstream.minFed;
                upd.setExtraData(GlobalFederateId{}.baseValue());
                upd.setExtraDestData(upstream.responseSequenceCounter);
                if (!globalTime && (info.event_triggered || time_requested >= cBigTime)) {
                    upd.Te = checkAdd(time_exec, info.outputDelay);
                    upd.Te = std::min(upd.Te, checkAdd(upstream.TeAlt, info.outputDelay));
                }
                upd.Tdemin = std::min(upstream.TeAlt, upd.Te);
                sendMessageFunction(upd);
            }
        }
    } else if (triggerFed.isValid()) {
        upd.dest_id = triggerFed;
        const auto* dep = dependencies.getDependencyInfo(triggerFed);
        if (dep->dependent) {
            upd.setExtraDestData(dep->sequenceCounter);
            sendMessageFunction(upd);
        }
    }

    //    printf("%d next=%f, exec=%f, Tdemin=%f\n", mSourceId, static_cast<double>(time_next),
    // static_cast<double>(time_exec), static_cast<double>(time_minDe));
}

void TimeCoordinator::updateTimeGrant()
{
    if (iterating != IterationRequest::FORCE_ITERATION) {
        time_granted = time_exec;
        time_grantBase = time_granted;
    }
    ++sequenceCounter;
    ActionMessage treq(CMD_TIME_GRANT);
    treq.source_id = mSourceId;
    treq.actionTime = time_granted;
    treq.counter = sequenceCounter;
    if (static_cast<std::int32_t>(treq.counter) != sequenceCounter) {
        sequenceCounter = 0;
    }
    if (iterating != IterationRequest::NO_ITERATIONS) {
        dependencies.resetIteratingTimeRequests(time_exec);
    }
    lastSend.next = treq.actionTime;
    lastSend.Te = treq.actionTime;
    lastSend.minDe = treq.actionTime;
    lastSend.mTimeState = TimeState::time_granted;
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

bool TimeCoordinator::addDependency(GlobalFederateId fedID)
{
    if (BaseTimeCoordinator::addDependency(fedID)) {
        dependency_federates.lock()->push_back(fedID);
        return true;
    }
    return false;
}

bool TimeCoordinator::addDependent(GlobalFederateId fedID)
{
    if (BaseTimeCoordinator::addDependent(fedID)) {
        dependent_federates.lock()->push_back(fedID);
        return true;
    }
    return false;
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
            if (msg.action() == CMD_EXEC_REQUEST || msg.action() == CMD_TIME_REQUEST) {
                msg.setExtraDestData(dep.sequenceCounter);
            }
            sendMessageFunction(msg);
        }
    }
    return skipped;
}

void TimeCoordinator::sendUpdatedExecRequest(GlobalFederateId target,
                                             GlobalFederateId minFed,
                                             std::int32_t responseSequenceCounter)
{
    if (!minFed.isValid()) {
        const auto& mfed = getExecEntryMinFederate(dependencies, mSourceId);
        minFed = mfed.fedID;
        responseSequenceCounter = mfed.sequenceCounter;
    }

    ActionMessage execreq(CMD_EXEC_REQUEST);
    execreq.source_id = mSourceId;
    setIterationFlags(execreq, iterating);
    execreq.counter = sequenceCounter;
    execreq.setExtraData(minFed.baseValue());

    execreq.messageID = currentRestrictionLevel;
    if (info.wait_for_current_time_updates) {
        setActionFlag(execreq, delayed_timing_flag);
    }
    if (target.isValid()) {
        execreq.dest_id = target;
        execreq.setExtraDestData(responseSequenceCounter);
        sendMessageFunction(execreq);
    } else {
        for (const auto& dep : dependencies) {
            if (dep.dependent && dep.mTimeState < TimeState::time_granted) {
                execreq.dest_id = dep.fedID;
                execreq.setExtraDestData(dep.sequenceCounter);
                sendMessageFunction(execreq);
            }
        }
    }
}

MessageProcessingResult TimeCoordinator::checkExecEntry(GlobalFederateId triggerFed)
{
    auto ret = MessageProcessingResult::CONTINUE_PROCESSING;
    if (time_block <= timeZero) {
        return ret;
    }
    if (!dependencies.checkIfReadyForExecEntry(iterating != IterationRequest::NO_ITERATIONS,
                                               info.wait_for_current_time_updates)) {
        if (!hasInitUpdates) {
            if (triggerFed.isValid() && iterating != IterationRequest::NO_ITERATIONS) {
                if (dependencies.checkIfReadyForExecEntry(false, false)) {
                    // if we are just continuing but would have granted in other circumstances
                    const auto& mfed = getExecEntryMinFederate(dependencies, mSourceId);
                    if (mfed.fedID == triggerFed) {
                        sendUpdatedExecRequest(triggerFed,
                                               GlobalFederateId{},
                                               mfed.sequenceCounter);
                    } else {
                        const auto* tfed = dependencies.getDependencyInfo(triggerFed);
                        if (tfed->dependent) {
                            sendUpdatedExecRequest(triggerFed,
                                                   GlobalFederateId{},
                                                   tfed->sequenceCounter);
                        }
                    }
                } else {
                    needSendAll = true;
                }
            }
        }
        return ret;
    }
    bool sendAll{needSendAll};
    needSendAll = false;
    switch (iterating) {
        case IterationRequest::NO_ITERATIONS:
            if (!info.wait_for_current_time_updates) {
                ret = MessageProcessingResult::NEXT_STEP;
            } else {
                // on wait for current time flag all other federates must have entered exec mode
                total = generateMinTimeTotal(dependencies,
                                             info.restrictive_time_policy || globalTime,
                                             mSourceId,
                                             mSourceId,
                                             sequenceCounter);
                if (total.next > timeZero) {
                    ret = MessageProcessingResult::NEXT_STEP;
                }
            }
            break;
        case IterationRequest::ITERATE_IF_NEEDED:
        case IterationRequest::FORCE_ITERATION:
            if (iteration >= info.maxIterations) {
                if (iterating == IterationRequest::FORCE_ITERATION) {
                    ret = MessageProcessingResult::ITERATING;
                } else {
                    ret = MessageProcessingResult::NEXT_STEP;
                }
                break;
            }
            if (hasInitUpdates) {
                ret = MessageProcessingResult::ITERATING;
            } else {
                if ((dependencies.checkIfReadyForExecEntry(false,
                                                           info.wait_for_current_time_updates)) ||
                    (info.wait_for_current_time_updates &&
                     dependencies.checkIfAllDependenciesArePastExec(true))) {
                    ret = (iterating == IterationRequest::FORCE_ITERATION) ?
                        MessageProcessingResult::ITERATING :
                        MessageProcessingResult::NEXT_STEP;
                } else {
                    bool allowed{!info.wait_for_current_time_updates};
                    bool restricted{info.restrictive_time_policy};
                    bool restrictionAdvance{restricted};
                    int restrictionLevel{50};
                    if (allowed) {
                        for (const auto& dep : dependencies) {
                            if (!dep.dependency) {
                                continue;
                            }
                            if (dep.mTimeState == TimeState::initialized) {
                                allowed = false;
                                restrictionAdvance = false;
                                break;
                            }
                            if (dep.mTimeState >= TimeState::exec_requested) {
                                continue;
                            }
                            if (dep.minFed != mSourceId) {
                                allowed = false;
                            }
                            if (dep.responseSequenceCounter == sequenceCounter) {
                                if (restricted) {
                                    restrictionLevel =
                                        (std::min)(restrictionLevel,
                                                   static_cast<int>(dep.restrictionLevel));
                                }

                            } else {
                                restrictionAdvance = false;
                                allowed = false;
                                break;
                            }
                        }
                    }
                    if (allowed) {
                        if (restricted) {
                            if (restrictionLevel >= 1) {
                                ret = (iterating == IterationRequest::FORCE_ITERATION) ?
                                    MessageProcessingResult::ITERATING :
                                    MessageProcessingResult::NEXT_STEP;
                            } else {
                                if (currentRestrictionLevel != restrictionLevel + 1) {
                                    currentRestrictionLevel = restrictionLevel + 1;
                                    sendAll = true;
                                    ++sequenceCounter;
                                }

                                ret = MessageProcessingResult::CONTINUE_PROCESSING;
                            }
                        } else {
                            ret = (iterating == IterationRequest::FORCE_ITERATION) ?
                                MessageProcessingResult::ITERATING :
                                MessageProcessingResult::NEXT_STEP;
                        }

                    } else {
                        if (restrictionAdvance) {
                            currentRestrictionLevel = restrictionLevel + 1;
                            sendAll = true;
                            ++sequenceCounter;
                        }
                        ret = MessageProcessingResult::CONTINUE_PROCESSING;
                    }
                }
            }
            break;
        default:
            break;
    }
    if (!dynamicJoining) {
        if (ret == MessageProcessingResult::NEXT_STEP) {
            time_granted = timeZero;
            time_grantBase = time_granted;
            executionMode = true;
            iteration = 0;
            currentRestrictionLevel = 0;
            ActionMessage execgrant(CMD_EXEC_GRANT);
            execgrant.source_id = mSourceId;
            execgrant.setExtraDestData(TIME_COORDINATOR_VERSION);  // version
            transmitTimingMessages(execgrant);
        } else if (ret == MessageProcessingResult::ITERATING) {
            dependencies.resetIteratingExecRequests();
            hasInitUpdates = false;
            ++iteration;
            ActionMessage execgrant(CMD_EXEC_GRANT);
            execgrant.source_id = mSourceId;
            execgrant.counter = iteration;
            execgrant.setExtraDestData(TIME_COORDINATOR_VERSION);  // version
            setActionFlag(execgrant, iteration_requested_flag);
            transmitTimingMessages(execgrant);
            currentRestrictionLevel = 0;
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
            currentRestrictionLevel = 0;
            ActionMessage execgrant(time_granted > timeZero ? CMD_TIME_GRANT : CMD_EXEC_GRANT);
            execgrant.source_id = mSourceId;
            execgrant.actionTime = time_granted;
            execgrant.setExtraDestData(TIME_COORDINATOR_VERSION);  // version
            transmitTimingMessages(execgrant);
        }
    }
    if (triggerFed.isValid() && ret == MessageProcessingResult::CONTINUE_PROCESSING &&
        iterating != IterationRequest::NO_ITERATIONS) {
        // if we are just continuing
        const auto& mfed = getExecEntryMinFederate(dependencies, mSourceId);
        if (sendAll) {
            sendUpdatedExecRequest(GlobalFederateId{}, mfed.fedID, mfed.sequenceCounter);
        } else {
            if (triggerFed != mSourceId) {
                if (triggerFed == mfed.fedID && mfed.dependent) {
                    sendUpdatedExecRequest(triggerFed, mfed.fedID, mfed.sequenceCounter);
                } else {
                    const auto* tfed = dependencies.getDependencyInfo(triggerFed);
                    if (tfed->dependent) {
                        sendUpdatedExecRequest(triggerFed, mfed.fedID, tfed->sequenceCounter);
                    }
                }
            }
        }
    }
    return ret;
}

static bool isDelayableMessage(const ActionMessage& cmd, GlobalFederateId localId)
{
    return (cmd.action() == CMD_TIME_GRANT && cmd.source_id != localId);
}

std::pair<GlobalFederateId, Time> TimeCoordinator::getMinGrantedDependency() const
{
    Time minTime = Time::maxVal();
    GlobalFederateId minID;
    for (const auto& dep : dependencies) {
        if (!dep.dependency) {
            continue;
        }
        if (dep.mTimeState != TimeState::time_requested) {
            if (dep.next < minTime) {
                minTime = dep.next;
                minID = dep.fedID;
            }
        }
    }
    return {minID, minTime};
}

TimeProcessingResult TimeCoordinator::processTimeMessage(const ActionMessage& cmd)
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
                treq.source_id = mSourceId;
                treq.actionTime = time_granted;
                lastSend.next = time_granted;
                lastSend.Te = time_granted;
                lastSend.minDe = time_granted;
                lastSend.mTimeState = TimeState::time_granted;
                transmitTimingMessages(treq);
                return TimeProcessingResult::PROCESSED;
            }
            return TimeProcessingResult::NOT_PROCESSED;
        case CMD_DISCONNECT:
        case CMD_BROADCAST_DISCONNECT:
        case CMD_DISCONNECT_CORE:
        case CMD_DISCONNECT_FED:
        case CMD_DISCONNECT_BROKER:
        case CMD_GLOBAL_ERROR:
        case CMD_LOCAL_ERROR:
            // this command requires removing dependents as well as dealing with dependency
            // processing
            removeDependent(cmd.source_id);
            break;
        case CMD_REQUEST_CURRENT_TIME:
            if (disconnected || lastSend.mTimeState == TimeState::error) {
                ActionMessage treq(CMD_DISCONNECT, mSourceId, cmd.source_id);
                treq.setExtraDestData(cmd.counter);
                sendMessageFunction(treq);
            } else {
                auto resp = generateTimeRequest(lastSend, cmd.source_id, cmd.counter);
                if (triggered) {
                    setActionFlag(resp, destination_target);
                    if (cmd.source_id == gRootBrokerID) {
                        triggered = false;
                    }
                }
                sendMessageFunction(resp);
                dependencies.updateTime(cmd);
            }

            return TimeProcessingResult::PROCESSED;
        default:
            break;
    }
    if (isDelayableMessage(cmd, mSourceId)) {
        auto* dep = dependencies.getDependencyInfo(cmd.source_id);
        if (dep == nullptr) {
            return TimeProcessingResult::NOT_PROCESSED;
        }
        switch (dep->mTimeState) {
            case TimeState::time_requested:
                if (dep->next > time_exec) {
                    //     return TimeProcessingResult::delay_processing;
                }
                break;
            case TimeState::time_requested_iterative:
                if (dep->next > time_exec) {
                    //         return TimeProcessingResult::delay_processing;
                }
                if ((iterating != IterationRequest::NO_ITERATIONS) && (time_exec == dep->next)) {
                    //       return TimeProcessingResult::delay_processing;
                }
                break;
            case TimeState::exec_requested_iterative:
                if ((iterating != IterationRequest::NO_ITERATIONS) && (checkingExec) &&
                    (dep->hasData)) {
                    //          return TimeProcessingResult::delay_processing;
                }
                break;
            default:
                break;
        }
    }
    auto procRes = dependencies.updateTime(cmd);
    if (procRes == TimeProcessingResult::PROCESSED_AND_CHECK) {
        auto checkRes = dependencies.checkForIssues(info.wait_for_current_time_updates);
        if (checkRes.first != 0) {
            ActionMessage ge(CMD_GLOBAL_ERROR);
            ge.dest_id = parent_broker_id;
            ge.source_id = mSourceId;
            ge.messageID = checkRes.first;
            ge.payload = checkRes.second;
            sendMessageFunction(ge);
        }
        procRes = TimeProcessingResult::PROCESSED;
    }

    return procRes;
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

TimeProcessingResult TimeCoordinator::processTimeBlockMessage(const ActionMessage& cmd)
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
        return TimeProcessingResult::PROCESSED;
    }
    time_block = ltime;
    return TimeProcessingResult::NOT_PROCESSED;
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

int TimeCoordinator::getIntegerProperty(int intProperty) const
{
    switch (intProperty) {  // NOLINT
        case defs::Properties::MAX_ITERATIONS:
            return info.maxIterations;
        case defs::Properties::CURRENT_ITERATION:
            return iteration.load();
        default:
            return HELICS_INVALID_PROPERTY_VALUE;
    }
}

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
