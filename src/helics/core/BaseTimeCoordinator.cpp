/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "BaseTimeCoordinator.hpp"

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
void BaseTimeCoordinator::enteringExecMode()
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
            ge.source_id = mSourceId;
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

void BaseTimeCoordinator::disconnect()
{
    if (sendMessageFunction) {
        if (dependencies.empty()) {
            return;
        }
        ActionMessage bye(CMD_DISCONNECT);
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
}


void BaseTimeCoordinator::generateDebuggingTimeInfo(Json::Value& base) const
{
    
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


Json::Value BaseTimeCoordinator::grantTimeoutCheck(const ActionMessage& cmd)
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

bool BaseTimeCoordinator::isDependency(GlobalFederateId ofed) const
{
    return dependencies.isDependency(ofed);
}

bool BaseTimeCoordinator::addDependency(GlobalFederateId fedID)
{
    return dependencies.addDependency(fedID);
}

bool BaseTimeCoordinator::addDependent(GlobalFederateId fedID)
{
    return dependencies.addDependent(fedID);
}

void BaseTimeCoordinator::setAsChild(GlobalFederateId fedID)
{
    auto* dep = dependencies.getDependencyInfo(fedID);
    if (dep != nullptr) {
        dep->connection = ConnectionType::child;
    }
}

void BaseTimeCoordinator::setAsParent(GlobalFederateId fedID)
{
    auto* dep = dependencies.getDependencyInfo(fedID);
    if (dep != nullptr) {
        dep->connection = ConnectionType::parent;
        noParent = false;
    }
}

void BaseTimeCoordinator::removeDependency(GlobalFederateId fedID)
{
    dependencies.removeDependency(fedID);
}

void BaseTimeCoordinator::removeDependent(GlobalFederateId fedID)
{
    dependencies.removeDependent(fedID);
}

const DependencyInfo* BaseTimeCoordinator::getDependencyInfo(GlobalFederateId ofed) const
{
    return dependencies.getDependencyInfo(ofed);
}

std::vector<GlobalFederateId> BaseTimeCoordinator::getDependencies() const
{
    std::vector<GlobalFederateId> deps;
    for (const auto& dep : dependencies) {
        if (dep.dependency) {
            deps.push_back(dep.fedID);
        }
    }
    return deps;
}

std::vector<GlobalFederateId> BaseTimeCoordinator::getDependents() const
{
    std::vector<GlobalFederateId> deps;
    for (const auto& dep : dependencies) {
        if (dep.dependent) {
            deps.push_back(dep.fedID);
        }
    }
    return deps;
}

bool BaseTimeCoordinator::hasActiveTimeDependencies() const
{
    return dependencies.hasActiveTimeDependencies();
}

int BaseTimeCoordinator::dependencyCount() const
{
    return dependencies.activeDependencyCount();
}
/** get a count of the active dependencies*/
GlobalFederateId BaseTimeCoordinator::getMinDependency() const
{
    return dependencies.getMinDependency();
}



ActionMessage BaseTimeCoordinator::generateTimeRequest(const DependencyInfo& dep,
                                                             GlobalFederateId fed) const
{
    ActionMessage nTime(CMD_TIME_REQUEST);
    nTime.source_id = mSourceId;
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
            nTime.counter = dep.sequenceCounter;
            nTime.setExtraDestData(dep.responseSequenceCounter);
            break;
        case TimeState::time_requested_require_iteration:
            nTime.setExtraData(dep.minFed.baseValue());
            setIterationFlags(nTime, IterationRequest::FORCE_ITERATION);
            nTime.Tdemin = std::min(dep.minDe, dep.Te);
            nTime.counter = dep.sequenceCounter;
            nTime.Te = dep.Te;
            nTime.setExtraDestData(dep.responseSequenceCounter);
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
            nTime.counter = dep.sequenceCounter;
            break;
        case TimeState::exec_requested_require_iteration:
            nTime.setAction(CMD_EXEC_REQUEST);
            setIterationFlags(nTime, IterationRequest::FORCE_ITERATION);
            nTime.setExtraData(dep.minFed.baseValue());
            nTime.counter = dep.sequenceCounter;
            break;
        case TimeState::initialized:
            if (dep.responseSequenceCounter == 0) {
                nTime.setAction(CMD_IGNORE);
            } else {
                nTime.setAction(CMD_EXEC_GRANT);
                nTime.setExtraData(dep.minFed.baseValue());
                setIterationFlags(nTime, IterationRequest::ITERATE_IF_NEEDED);
                nTime.counter = dep.sequenceCounter;
            }

            break;
    }

    return nTime;
}


bool BaseTimeCoordinator::processTimeMessage(const ActionMessage& cmd)
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
                ge.source_id = mSourceId;
                ge.messageID = checkRes.first;
                ge.payload = checkRes.second;
                sendMessageFunction(ge);
            }
            return true;
        }
    }
}

void BaseTimeCoordinator::processDependencyUpdateMessage(const ActionMessage& cmd)
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