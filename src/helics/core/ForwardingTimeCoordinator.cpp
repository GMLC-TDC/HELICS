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
void ForwardingTimeCoordinator::enteringExecMode()
{
    if (executionMode) {
        return;
    }
    checkingExec = true;
    if (!dependencies.empty()) {
        updateTimeFactors();
        auto res = dependencies.checkForIssues(false);
        if (res.first != 0) {
            ActionMessage ge(CMD_GLOBAL_ERROR);
            ge.dest_id = parent_broker_id;
            ge.source_id = source_id;
            ge.messageID = res.first;
            ge.payload = res.second;
            sendMessageFunction(ge);
            return;
        }
    }
    bool fedOnly = true;
    noParent = true;
    for (const auto& dep : dependencies) {
        if (dep.connection == ConnectionType::parent) {
            fedOnly = false;
            noParent = false;
            break;
        }
        if (dep.connection == ConnectionType::child && dep.fedID.isBroker()) {
            fedOnly = false;
        }
    }
    federatesOnly = fedOnly;
}

void ForwardingTimeCoordinator::disconnect()
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
            for (const auto& dep : dependencies) {
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
}

void ForwardingTimeCoordinator::updateTimeFactors()
{
    auto mTimeUpstream = generateMinTimeUpstream(dependencies, restrictive_time_policy, source_id);
    auto mTimeDownstream = (noParent) ?
        mTimeUpstream :
        generateMinTimeDownstream(dependencies, restrictive_time_policy, source_id);

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
            auto td = generateMinTimeUpstream(dependencies,
                                              restrictive_time_policy,
                                              source_id,
                                              downstream.minFed);
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

    base["dependencies"] = Json::arrayValue;
    base["federatesonly"] = federatesOnly;
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
}

std::string ForwardingTimeCoordinator::printTimeStatus() const
{
    return fmt::format(R"raw({{"time_next":{}, "Te":{}, "minDe":{}}})raw",
                       static_cast<double>(downstream.next),
                       static_cast<double>(downstream.Te),
                       static_cast<double>(downstream.minDe));
}

Json::Value ForwardingTimeCoordinator::grantTimeoutCheck(const ActionMessage& cmd)
{
    for (auto& dep : dependencies) {
        if (dep.fedID == cmd.source_id) {
            dep.timeoutCount = cmd.counter;
            if (cmd.counter == 6) {
                Json::Value base;
                generateDebuggingTimeInfo(base);
                return base;
            }
        }
    }
    return Json::nullValue;
}

bool ForwardingTimeCoordinator::isDependency(GlobalFederateId ofed) const
{
    return dependencies.isDependency(ofed);
}

bool ForwardingTimeCoordinator::addDependency(GlobalFederateId fedID)
{
    return dependencies.addDependency(fedID);
}

bool ForwardingTimeCoordinator::addDependent(GlobalFederateId fedID)
{
    return dependencies.addDependent(fedID);
}

void ForwardingTimeCoordinator::setAsChild(GlobalFederateId fedID)
{
    auto* dep = dependencies.getDependencyInfo(fedID);
    if (dep != nullptr) {
        dep->connection = ConnectionType::child;
    }
}

void ForwardingTimeCoordinator::setAsParent(GlobalFederateId fedID)
{
    auto* dep = dependencies.getDependencyInfo(fedID);
    if (dep != nullptr) {
        dep->connection = ConnectionType::parent;
        noParent = false;
    }
}

void ForwardingTimeCoordinator::removeDependency(GlobalFederateId fedID)
{
    dependencies.removeDependency(fedID);
}

void ForwardingTimeCoordinator::removeDependent(GlobalFederateId fedID)
{
    dependencies.removeDependent(fedID);
}

const DependencyInfo* ForwardingTimeCoordinator::getDependencyInfo(GlobalFederateId ofed) const
{
    return dependencies.getDependencyInfo(ofed);
}

std::vector<GlobalFederateId> ForwardingTimeCoordinator::getDependencies() const
{
    std::vector<GlobalFederateId> deps;
    for (const auto& dep : dependencies) {
        if (dep.dependency) {
            deps.push_back(dep.fedID);
        }
    }
    return deps;
}

std::vector<GlobalFederateId> ForwardingTimeCoordinator::getDependents() const
{
    std::vector<GlobalFederateId> deps;
    for (const auto& dep : dependencies) {
        if (dep.dependent) {
            deps.push_back(dep.fedID);
        }
    }
    return deps;
}

bool ForwardingTimeCoordinator::hasActiveTimeDependencies() const
{
    return dependencies.hasActiveTimeDependencies();
}

int ForwardingTimeCoordinator::dependencyCount() const
{
    return dependencies.activeDependencyCount();
}
/** get a count of the active dependencies*/
GlobalFederateId ForwardingTimeCoordinator::getMinDependency() const
{
    return dependencies.getMinDependency();
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
                    if (dep.minFed != source_id) {
                        allowed = false;
                        break;
                    }
                    if (dep.minFedIteration != downstream.requestIteration) {
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
    execgrant.source_id = source_id;
    transmitTimingMessagesDownstream(execgrant);
    transmitTimingMessagesUpstream(execgrant);
    return ret;
}

ActionMessage ForwardingTimeCoordinator::generateTimeRequest(const DependencyInfo& dep,
                                                             GlobalFederateId fed) const
{
    ActionMessage nTime(CMD_TIME_REQUEST);
    nTime.source_id = source_id;
    nTime.dest_id = fed;
    nTime.actionTime = dep.next;
    if (dep.delayedTiming) {
        setActionFlag(nTime, delayed_timing_flag);
    }
    switch (dep.mTimeState) {
        case TimeState::time_granted:
            nTime.setAction(CMD_TIME_GRANT);
            break;
        case TimeState::time_requested:
            nTime.setExtraData(dep.minFed.baseValue());
            nTime.Tdemin = std::min(dep.minDe, dep.Te);
            nTime.Te = dep.Te;
            break;
        case TimeState::time_requested_iterative:
            nTime.setExtraData(dep.minFed.baseValue());
            setIterationFlags(nTime, IterationRequest::ITERATE_IF_NEEDED);
            nTime.Tdemin = std::min(dep.minDe, dep.Te);
            nTime.Te = dep.Te;
            nTime.counter = dep.requestIteration;
            break;
        case TimeState::time_requested_require_iteration:
            nTime.setExtraData(dep.minFed.baseValue());
            setIterationFlags(nTime, IterationRequest::FORCE_ITERATION);
            nTime.Tdemin = std::min(dep.minDe, dep.Te);
            nTime.counter = dep.requestIteration;
            nTime.Te = dep.Te;
            break;
        case TimeState::exec_requested:
            nTime.setAction(CMD_EXEC_REQUEST);
            nTime.actionTime = Time::zeroVal();
            break;
        case TimeState::error:
            nTime.setAction(CMD_IGNORE);
            // no need to send updates for this
            break;
        case TimeState::exec_requested_iterative:
            nTime.setAction(CMD_EXEC_REQUEST);
            setIterationFlags(nTime, IterationRequest::ITERATE_IF_NEEDED);
            nTime.setExtraData(dep.minFed.baseValue());
            nTime.setExtraDestData(dep.minFedIteration);
            nTime.counter = dep.requestIteration;
            break;
        case TimeState::exec_requested_require_iteration:
            nTime.setAction(CMD_EXEC_REQUEST);
            setIterationFlags(nTime, IterationRequest::FORCE_ITERATION);
            nTime.setExtraData(dep.minFed.baseValue());
            nTime.setExtraDestData(dep.minFedIteration);
            nTime.counter = dep.requestIteration;
            break;
        case TimeState::initialized:
            if (dep.minFedIteration == 0) {
                nTime.setAction(CMD_IGNORE);
            } else {
                nTime.setAction(CMD_EXEC_GRANT);
                nTime.setExtraData(dep.minFed.baseValue());
                nTime.setExtraDestData(dep.minFedIteration);
                setIterationFlags(nTime, IterationRequest::ITERATE_IF_NEEDED);
                nTime.counter = dep.requestIteration;
            }

            break;
    }

    return nTime;
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
                msg.dest_id = dep.fedID;
                sendMessageFunction(msg);
            }
        }
    }
}
bool ForwardingTimeCoordinator::processTimeMessage(const ActionMessage& cmd)
{
    switch (cmd.action()) {
        case CMD_DISCONNECT:
        case CMD_DISCONNECT_BROKER:
        case CMD_DISCONNECT_FED:
        case CMD_DISCONNECT_CORE:
        case CMD_BROADCAST_DISCONNECT:
            removeDependent(cmd.source_id);
            break;
        default:
            break;
    }
    auto procRes = dependencies.updateTime(cmd);
    switch (procRes) {
        case DependencyProcessingResult::NOT_PROCESSED:
        default:
            return false;
        case DependencyProcessingResult::PROCESSED:
            return true;
        case DependencyProcessingResult::PROCESSED_AND_CHECK: {
            auto checkRes = dependencies.checkForIssues(false);
            if (checkRes.first != 0) {
                ActionMessage ge(CMD_GLOBAL_ERROR);
                ge.dest_id = parent_broker_id;
                ge.source_id = source_id;
                ge.messageID = checkRes.first;
                ge.payload = checkRes.second;
                sendMessageFunction(ge);
            }
            return true;
        }
    }
}

void ForwardingTimeCoordinator::processDependencyUpdateMessage(const ActionMessage& cmd)
{
    switch (cmd.action()) {
        case CMD_ADD_DEPENDENCY:
            addDependency(cmd.source_id);
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
            addDependency(cmd.source_id);
            addDependent(cmd.source_id);
            break;
        case CMD_REMOVE_INTERDEPENDENCY:
            removeDependency(cmd.source_id);
            removeDependent(cmd.source_id);
            break;
        default:
            break;
    }
    if (checkActionFlag(cmd, child_flag)) {
        setAsChild(cmd.source_id);
    }
    if (checkActionFlag(cmd, parent_flag)) {
        setAsParent(cmd.source_id);
    }
}

}  // namespace helics
