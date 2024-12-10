/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "AsyncTimeCoordinator.hpp"

#include "flagOperations.hpp"
#include "helics_definitions.hpp"
#include "nlohmann/json.hpp"

#include <algorithm>
#include <fmt/format.h>
#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace helics {

bool AsyncTimeCoordinator::updateTimeFactors()
{
    auto timeStream = generateMinTimeTotal(dependencies, true, mSourceId, NoIgnoredFederates, 0);
    currentMinTime = timeStream.next;
    currentTimeState = timeStream.mTimeState;

    return false;
}

void AsyncTimeCoordinator::generateDebuggingTimeInfo(nlohmann::json& base) const
{
    base["type"] = "global";
    base["nextEvent"] = static_cast<double>(nextEvent);
    addTimeState(base, currentTimeState);
    base["minTime"] = static_cast<double>(currentMinTime);
    BaseTimeCoordinator::generateDebuggingTimeInfo(base);
}

std::string AsyncTimeCoordinator::printTimeStatus() const
{
    return fmt::format(R"raw({{"time_next":{}, "Te":{}}})raw",
                       static_cast<double>(currentMinTime),
                       static_cast<double>(nextEvent));
}

MessageProcessingResult AsyncTimeCoordinator::checkExecEntry(GlobalFederateId /*triggerFed*/)
{
    auto ret = MessageProcessingResult::CONTINUE_PROCESSING;

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

void AsyncTimeCoordinator::transmitTimingMessagesUpstream(ActionMessage& msg) const
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

void AsyncTimeCoordinator::transmitTimingMessagesDownstream(ActionMessage& msg,
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
