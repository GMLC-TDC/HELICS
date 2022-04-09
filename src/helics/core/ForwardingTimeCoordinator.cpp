/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "ForwardingTimeCoordinator.hpp"

#include "../common/fmt_format.h"
#include "flagOperations.hpp"
#include "helics_definitions.hpp"

#include "json/json.h"
#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace helics {

void ForwardingTimeCoordinator::updateTimeFactors()
{
    auto mTimeUpstream = generateMinTimeUpstream(
        dependencies, restrictive_time_policy, mSourceId, NoIgnoredFederates, 0);
    auto mTimeDownstream = (noParent) ? mTimeUpstream :
                                        generateMinTimeDownstream(dependencies,
                                                                  restrictive_time_policy,
                                                                  mSourceId,
                                                                  NoIgnoredFederates,
                                                                  0);

    bool updateUpstream = upstream.update(mTimeUpstream);

    bool updateDownstream = downstream.update(mTimeDownstream);

    if (upstream.mTimeState == TimeState::time_requested) {
        if (upstream.minDe < downstream.minDe) {
            downstream.minDe = upstream.minDe;
        }
        if (upstream.Te < downstream.Te) {
            downstream.Te = upstream.Te;
        }
    }
    if (!restrictive_time_policy && upstream.minDe < Time::maxVal()) {
        if (downstream.minDe > downstream.next) {
            //     downstream.next = downstream.minminDe;
        }
    }

    if (updateUpstream) {
        auto upd = generateTimeRequest(upstream, GlobalFederateId{});
        if (upd.action() != CMD_IGNORE) {
            transmitTimingMessagesUpstream(upd);
        }
    }
    if (updateDownstream) {
        if (dependencies.hasDelayedDependency() &&
            downstream.minFed == dependencies.delayedDependency()) {
            auto upd = generateTimeRequest(downstream, GlobalFederateId{});
            if (upd.action() != CMD_IGNORE) {
                transmitTimingMessagesDownstream(upd, downstream.minFed);
            }
            auto td = generateMinTimeUpstream(
                dependencies, restrictive_time_policy, mSourceId, downstream.minFed, 0);
            DependencyInfo di;
            di.update(td);
            auto upd_delayed = generateTimeRequest(di, downstream.minFed);
            if (sendMessageFunction) {
                sendMessageFunction(upd_delayed);
            }
        } else {
            auto upd = generateTimeRequest(downstream, GlobalFederateId{});
            if (upd.action() != CMD_IGNORE) {
                transmitTimingMessagesDownstream(upd);
            }
        }
    }
}

void ForwardingTimeCoordinator::generateDebuggingTimeInfo(Json::Value& base) const
{
    base["type"] = "forwarding";
    Json::Value upBlock;
    generateJsonOutputTimeData(upBlock, upstream);

    base["upstream"] = upBlock;
    Json::Value downBlock;
    generateJsonOutputTimeData(downBlock, downstream);
    base["downstream"] = downBlock;
    BaseTimeCoordinator::generateDebuggingTimeInfo(base);
}

std::string ForwardingTimeCoordinator::printTimeStatus() const
{
    return fmt::format(R"raw({{"time_next":{}, "Te":{}, "minDe":{}}})raw",
                       static_cast<double>(downstream.next),
                       static_cast<double>(downstream.Te),
                       static_cast<double>(downstream.minDe));
}

MessageProcessingResult ForwardingTimeCoordinator::checkExecEntry()
{
    auto ret = MessageProcessingResult::CONTINUE_PROCESSING;

    if (!dependencies.checkIfReadyForExecEntry(false, false)) {
        bool allowed{false};
        if (downstream.mTimeState == TimeState::exec_requested_iterative) {
            allowed = true;
            for (auto& dep : dependencies) {
                if (dep.dependency) {
                    if (dep.minFed != mSourceId) {
                        allowed = false;
                        break;
                    }
                    if (dep.responseSequenceCounter != downstream.sequenceCounter) {
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

    downstream.next = timeZero;
    downstream.mTimeState = TimeState::time_granted;
    downstream.minDe = timeZero;

    ActionMessage execgrant(CMD_EXEC_GRANT);
    execgrant.source_id = mSourceId;
    transmitTimingMessagesDownstream(execgrant);
    transmitTimingMessagesUpstream(execgrant);
    return ret;
}

void ForwardingTimeCoordinator::transmitTimingMessagesUpstream(ActionMessage& msg) const
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

void ForwardingTimeCoordinator::transmitTimingMessagesDownstream(ActionMessage& msg,
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
