/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "GlobalTimeCoordinator.hpp"

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

static Time findNextTriggerEvent(const TimeDependencies& deps)
{
    Time me{Time::maxVal()};
    for (const auto& dep : deps) {
        if (!dep.nonGranting) {
            if (dep.Te < me) {
                me = dep.Te;
            }
        }
    }
    return me;
}

static std::pair<bool, Time> checkForTriggered(const TimeDependencies& deps, Time nextEvent)
{
    Time me{Time::maxVal()};
    bool triggered{false};
    for (const auto& dep : deps) {
        if (dep.next > nextEvent) {
            if (dep.Te < me) {
                me = dep.Te;
            }
            continue;
        }
        if (dep.nonGranting) {
            if (dep.triggered) {
                triggered = true;
            }
        } else {
            if (dep.Te < me) {
                me = dep.Te;
            }
        }
    }
    return {triggered, me};
}

void GlobalTimeCoordinator::sendTimeUpdateRequest(Time triggerTime)
{
    ActionMessage updateTime(CMD_REQUEST_CURRENT_TIME, mSourceId, mSourceId);
    updateTime.counter = sequenceCounter;
    for (auto& dep : dependencies) {
        if (dep.next <= triggerTime && dep.next < cBigTime) {
            updateTime.dest_id = dep.fedID;
            updateTime.setExtraDestData(dep.sequenceCounter);
            dep.updateRequested = true;
            dep.grantedIteration = sequenceCounter;
            sendMessageFunction(updateTime);
        }
    }
}

bool GlobalTimeCoordinator::updateTimeFactors()
{
    auto timeStream = generateMinTimeUpstream(dependencies, true, mSourceId, NoIgnoredFederates, 0);
    if (timeStream.mTimeState == TimeState::time_granted) {
        currentTimeState = TimeState::time_granted;
        currentMinTime = timeStream.next;
        nextEvent = timeStream.next;
        return false;
    }
    if (timeStream.mTimeState == TimeState::time_requested) {
        if (currentTimeState == TimeState::time_granted) {
            currentTimeState = TimeState::time_requested;
            currentMinTime = timeStream.next;
            nextEvent = findNextTriggerEvent(dependencies);
            ++sequenceCounter;
            auto trigTime = (nextEvent < cBigTime) ? nextEvent + Time::epsilon() : nextEvent;
            mNewRequest = false;
            sendTimeUpdateRequest(trigTime);
            return true;
        }
        if (currentTimeState == TimeState::time_requested) {
            auto trigTime = (nextEvent < cBigTime) ? nextEvent + Time::epsilon() : nextEvent;
            if (dependencies.verifySequenceCounter(trigTime, sequenceCounter)) {
                auto trig = checkForTriggered(dependencies, trigTime);
                bool verified{trig.second <= nextEvent};
                nextEvent = trig.second;
                trigTime = (nextEvent < cBigTime) ? nextEvent + Time::epsilon() : nextEvent;
                if (!verified) {
                    verified = dependencies.verifySequenceCounter(trigTime, sequenceCounter);
                }

                if (trig.first || !verified || mNewRequest) {
                    ++sequenceCounter;
                    mNewRequest = false;
                    sendTimeUpdateRequest(trigTime);
                    return true;
                }
                ActionMessage updateTime(CMD_TIME_REQUEST, mSourceId, mSourceId);
                updateTime.actionTime = trigTime;
                updateTime.Te = trigTime;
                updateTime.Tdemin = trigTime;

                ++sequenceCounter;
                updateTime.counter = sequenceCounter;
                for (const auto& dep : dependencies) {
                    if (dep.next <= trigTime && dep.next < cBigTime) {
                        updateTime.dest_id = dep.fedID;
                        updateTime.setExtraDestData(dep.sequenceCounter);
                        sendMessageFunction(updateTime);
                    }
                }
                currentTimeState = TimeState::time_granted;
                currentMinTime = timeStream.Te;
                nextEvent = timeStream.Te;
            } else {
                for (auto& dep : dependencies) {
                    if (dep.updateRequested) {
                        continue;
                    }
                    if (dep.next <= trigTime && dep.next < cBigTime) {
                        if (!checkSequenceCounter(dep, trigTime, sequenceCounter)) {
                            std::cerr << "sequence check but no request" << std::endl;
                            /* ActionMessage updateTime(CMD_REQUEST_CURRENT_TIME,
                                                       mSourceId,
                                                       mSourceId);
                              updateTime.counter = sequenceCounter;

                              updateTime.dest_id = dep.fedID;
                              updateTime.setExtraDestData(dep.sequenceCounter);
                              dep.updateRequested = true;
                              sendMessageFunction(updateTime);
                              */
                        }
                    }
                }
            }
        }
    }
    return true;
}

TimeProcessingResult GlobalTimeCoordinator::processTimeMessage(const ActionMessage& cmd)
{
    auto res = BaseTimeCoordinator::processTimeMessage(cmd);
    if (res == TimeProcessingResult::PROCESSED_NEW_REQUEST) {
        mNewRequest = true;
    }
    return res;
}

void GlobalTimeCoordinator::generateDebuggingTimeInfo(nlohmann::json& base) const
{
    base["type"] = "global";
    base["nextEvent"] = static_cast<double>(nextEvent);
    addTimeState(base, currentTimeState);
    base["minTime"] = static_cast<double>(currentMinTime);
    base["executing"] = executionMode;
    BaseTimeCoordinator::generateDebuggingTimeInfo(base);
}

std::string GlobalTimeCoordinator::printTimeStatus() const
{
    return fmt::format(R"raw({{"time_next":{}, "Te":{}}})raw",
                       static_cast<double>(currentMinTime),
                       static_cast<double>(nextEvent));
}

MessageProcessingResult GlobalTimeCoordinator::checkExecEntry(GlobalFederateId /*triggerFed*/)
{
    if (!checkingExec) {
        if (sendMessageFunction) {
            ActionMessage logcmd(CMD_LOG);
            logcmd.messageID = HELICS_LOG_LEVEL_WARNING;
            logcmd.dest_id = mSourceId;
            logcmd.source_id = mSourceId;
            logcmd.setString(
                0, "calling checkExecEntry without first calling enterExec this is probably a bug");
            sendMessageFunction(logcmd);
        }
        return MessageProcessingResult::CONTINUE_PROCESSING;
    }
    auto ret = MessageProcessingResult::CONTINUE_PROCESSING;
    if (!dependencies.checkIfReadyForExecEntry(false, false)) {
        bool allowed{false};
        if (currentTimeState == TimeState::exec_requested_iterative) {
            allowed = true;
            for (const auto& dep : dependencies) {
                if (dep.dependency) {
                    if (dep.minFed != mSourceId) {
                        allowed = false;
                        break;
                    }
                    if (dep.responseSequenceCounter != sequenceCounter) {
                        allowed = false;
                        break;
                    }
                }
            }
        }
        if (!allowed) {
            return ret;
        }
    }
    executionMode = true;
    ret = MessageProcessingResult::NEXT_STEP;

    currentMinTime = timeZero;
    currentTimeState = TimeState::time_granted;
    nextEvent = timeZero;

    ActionMessage execgrant(CMD_EXEC_GRANT);
    execgrant.source_id = mSourceId;
    transmitTimingMessagesDownstream(execgrant);
    transmitTimingMessagesUpstream(execgrant);
    return ret;
}

void GlobalTimeCoordinator::transmitTimingMessagesUpstream(ActionMessage& msg) const
{
    if (!sendMessageFunction) {
        return;
    }

    for (const auto& dep : dependencies) {
        if (dep.connection == ConnectionType::CHILD) {
            continue;
        }
        if (!dep.dependent) {
            continue;
        }
        msg.dest_id = dep.fedID;
        if (msg.action() == CMD_EXEC_REQUEST) {
            msg.setExtraDestData(dep.sequenceCounter);
        }
        sendMessageFunction(msg);
    }
}

void GlobalTimeCoordinator::transmitTimingMessagesDownstream(ActionMessage& msg,
                                                             GlobalFederateId skipFed) const
{
    if (!sendMessageFunction) {
        return;
    }
    if ((msg.action() == CMD_TIME_REQUEST || msg.action() == CMD_TIME_GRANT)) {
        for (const auto& dep : dependencies) {
            if (dep.connection != ConnectionType::CHILD) {
                continue;
            }
            if (!dep.dependent) {
                continue;
            }
            if (dep.fedID == skipFed) {
                continue;
            }
            if (dep.dependency) {
                if (dep.next > msg.actionTime) {
                    continue;
                }
            }
            msg.dest_id = dep.fedID;
            sendMessageFunction(msg);
        }
    } else {
        for (const auto& dep : dependencies) {
            if (dep.dependent) {
                if (dep.fedID == skipFed) {
                    continue;
                }
                if (!dep.dependent) {
                    continue;
                }
                if (msg.action() == CMD_EXEC_REQUEST) {
                    msg.setExtraDestData(dep.sequenceCounter);
                }
                msg.dest_id = dep.fedID;
                sendMessageFunction(msg);
            }
        }
    }
}

}  // namespace helics
