/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../common/fmt_format.h"
#include "GlobalTimeCoordinator.hpp"
#include "flagOperations.hpp"
#include "helics_definitions.hpp"

#include "json/json.h"
#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace helics {


void GlobalTimeCoordinator::updateTimeFactors()
    {
    auto timeStream = generateMinTimeUpstream(dependencies, true, mSourceId);
    if (timeStream.mTimeState == TimeState::time_granted) {
        currentTimeState = TimeState::time_granted;
        currentMinTime = timeStream.next;
        nextEvent = timeStream.next;
        return;
    }
    if (timeStream.mTimeState == TimeState::time_requested) {
        if (currentTimeState==TimeState::time_granted) {
            currentTimeState = TimeState::time_requested;
            currentMinTime = timeStream.next;
            nextEvent = timeStream.Te;
            ActionMessage updateTime(CMD_REQUEST_CURRENT_TIME, mSourceId,mSourceId);
            ++sequenceCounter;
            updateTime.counter = sequenceCounter;
            for (const auto &dep:dependencies) {
                if (dep.next<=nextEvent) {
                    updateTime.dest_id = dep.fedID;
                    updateTime.setExtraDestData(dep.sequenceCounter);
                    sendMessageFunction(updateTime);
                }
            }
            return;
        }
        if (currentTimeState==TimeState::time_requested) {
            if (dependencies.verifySequenceCounter(nextEvent,sequenceCounter)) {
                ActionMessage updateTime(CMD_TIME_REQUEST, mSourceId, mSourceId);
                updateTime.actionTime = timeStream.Te+Time::epsilon();
                updateTime.Te = timeStream.Te + Time::epsilon();
                updateTime.Tdemin = timeStream.Te + Time::epsilon();
                updateTime.counter = sequenceCounter;
                for (const auto& dep : dependencies) {
                    if (dep.next <= nextEvent) {
                        updateTime.dest_id = dep.fedID;
                        updateTime.setExtraDestData(dep.sequenceCounter);
                        sendMessageFunction(updateTime);
                    }
                }
                currentTimeState = TimeState::time_granted;
                currentMinTime = timeStream.Te;
                nextEvent = timeStream.Te;
            }
        }
    }
}

void GlobalTimeCoordinator::generateDebuggingTimeInfo(Json::Value& base) const
{
    base["type"] = "global";
    
    
    BaseTimeCoordinator::generateDebuggingTimeInfo(base);
}

std::string GlobalTimeCoordinator::printTimeStatus() const
{
    return fmt::format(R"raw({{"time_next":{}, "Te":{}}})raw",
                       static_cast<double>(currentMinTime),
                       static_cast<double>(nextEvent)
                       );
}

MessageProcessingResult GlobalTimeCoordinator::checkExecEntry()
{
    auto ret = MessageProcessingResult::CONTINUE_PROCESSING;

    if (!dependencies.checkIfReadyForExecEntry(false, false)) {
        bool allowed{false};
        if (currentTimeState == TimeState::exec_requested_iterative) {
            allowed = true;
            for (auto& dep : dependencies) {
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
        if (dep.connection == ConnectionType::child) {
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
            if (dep.connection != ConnectionType::child) {
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
