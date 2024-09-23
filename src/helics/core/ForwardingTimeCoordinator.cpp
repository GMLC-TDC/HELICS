/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "ForwardingTimeCoordinator.hpp"

#include "flagOperations.hpp"
#include "helics_definitions.hpp"

#include <algorithm>
#include <fmt/format.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <vector>

namespace helics {

bool ForwardingTimeCoordinator::updateTimeFactors()
{
    auto mTimeUpstream = generateMinTimeUpstream(
        dependencies, restrictive_time_policy, mSourceId, NoIgnoredFederates, sequenceCounter);
    auto mTimeDownstream = (noParent) ? mTimeUpstream :
                                        generateMinTimeDownstream(dependencies,
                                                                  restrictive_time_policy,
                                                                  mSourceId,
                                                                  NoIgnoredFederates,
                                                                  sequenceCounter);

    bool updateUpStream{false};
    bool updateDownStream{false};
    if (mTimeUpstream.mTimeState > TimeState::exec_requested || !executionMode) {
        updateUpStream = upstream.update(mTimeUpstream);
    }
    if (mTimeDownstream.mTimeState > TimeState::exec_requested || !executionMode) {
        updateDownStream = downstream.update(mTimeDownstream);
    }

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

    sequenceCounter = upstream.sequenceCounter + sequenceModifier;
    if (updateUpStream || updateDownStream) {
        auto upd =
            generateTimeRequest(upstream, GlobalFederateId{}, upstream.responseSequenceCounter);
        if (upd.action() != CMD_IGNORE) {
            transmitTimingMessagesUpstream(upd);
        }
    }
    if (updateDownStream) {
        if (dependencies.hasDelayedDependency() &&
            downstream.minFed == dependencies.delayedDependency()) {
            auto upd = generateTimeRequest(downstream, GlobalFederateId{}, 0);
            if (upd.action() != CMD_IGNORE) {
                transmitTimingMessagesDownstream(upd, downstream.minFed);
            }
            auto td = generateMinTimeUpstream(
                dependencies, restrictive_time_policy, mSourceId, downstream.minFed, 0);
            DependencyInfo dependency;
            dependency.update(td);
            auto upd_delayed = generateTimeRequest(dependency,
                                                   downstream.minFed,
                                                   dependency.responseSequenceCounter);
            if (sendMessageFunction) {
                sendMessageFunction(upd_delayed);
            }
        } else {
            auto upd = generateTimeRequest(downstream, GlobalFederateId{}, 0);
            if (upd.action() != CMD_IGNORE) {
                transmitTimingMessagesDownstream(upd);
            }
        }
    } else if (dependencies.hasDelayedDependency() &&
               mTimeDownstream.minFed == dependencies.delayedDependency() && executionMode) {
        auto minTimeUpstream = generateMinTimeUpstream(
            dependencies, restrictive_time_policy, mSourceId, mTimeDownstream.minFed, 0);
        DependencyInfo dependency;
        dependency.update(minTimeUpstream);
        auto upd_delayed = generateTimeRequest(dependency,
                                               mTimeDownstream.minFed,
                                               dependency.responseSequenceCounter);
        if (sendMessageFunction) {
            sendMessageFunction(upd_delayed);
        }
    }
    return (updateUpStream || updateDownStream);
}

void ForwardingTimeCoordinator::generateDebuggingTimeInfo(nlohmann::json& base) const
{
    base["type"] = "forwarding";
    nlohmann::json upBlock;
    generateJsonOutputTimeData(upBlock, upstream);

    base["upstream"] = upBlock;
    nlohmann::json downBlock;
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

TimeProcessingResult ForwardingTimeCoordinator::processTimeMessage(const ActionMessage& cmd)
{
    auto res = BaseTimeCoordinator::processTimeMessage(cmd);
    if (res == TimeProcessingResult::PROCESSED_NEW_REQUEST) {
        sequenceModifier += mSequenceIncrement;
        if (sequenceModifier > 16000) {
            sequenceModifier = mSequenceIncrement;
        }
        sequenceCounter = upstream.sequenceCounter + sequenceModifier;
    }
    return res;
}

MessageProcessingResult ForwardingTimeCoordinator::checkExecEntry(GlobalFederateId /*triggerFed*/)
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
        if (dep.connection == ConnectionType::CHILD) {
            continue;
        }
        if (!dep.dependent) {
            continue;
        }
        msg.dest_id = dep.fedID;
        if (msg.action() == CMD_EXEC_REQUEST || msg.action() == CMD_TIME_REQUEST) {
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
                if (dep.next > msg.actionTime && dep.next < cBigTime) {
                    continue;
                }
            }
            if (msg.action() == CMD_TIME_REQUEST) {
                msg.setExtraDestData(dep.sequenceCounter);
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
