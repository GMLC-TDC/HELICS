/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "BaseTimeCoordinator.hpp"

#include "flagOperations.hpp"
#include "helics_definitions.hpp"
#include "nlohmann/json.hpp"

#include <algorithm>
#include <fmt/format.h>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace helics {

static auto nullMessageFunction = [](const ActionMessage& /*unused*/) {};

BaseTimeCoordinator::BaseTimeCoordinator(): sendMessageFunction(nullMessageFunction) {}
BaseTimeCoordinator::BaseTimeCoordinator(
    std::function<void(const ActionMessage&)> userSendMessageFunction):
    sendMessageFunction(std::move(userSendMessageFunction))
{
    if (!sendMessageFunction) {
        sendMessageFunction = nullMessageFunction;
    }
}

void BaseTimeCoordinator::setMessageSender(
    std::function<void(const ActionMessage&)> userSendMessageFunction)
{
    sendMessageFunction = std::move(userSendMessageFunction);
    if (!sendMessageFunction) {
        sendMessageFunction = nullMessageFunction;
    }
}

void BaseTimeCoordinator::enteringExecMode(IterationRequest /*mode*/)
{
    if (executionMode) {
        return;
    }
    checkingExec = true;
    if (!dependencies.empty()) {
        updateTimeFactors();
        auto res = dependencies.checkForIssues(false);
        if (res.first != 0) {
            ActionMessage error(CMD_GLOBAL_ERROR);
            error.dest_id = parent_broker_id;
            error.source_id = mSourceId;
            error.messageID = res.first;
            error.payload = res.second;
            sendMessageFunction(error);
            return;
        }
    }
    bool fedOnly = true;
    noParent = true;
    for (const auto& dep : dependencies) {
        if (dep.connection == ConnectionType::PARENT) {
            fedOnly = false;
            noParent = false;
            break;
        }
        if (dep.connection == ConnectionType::CHILD && dep.fedID.isBroker()) {
            fedOnly = false;
        }
    }
    federatesOnly = fedOnly;
    sendTimingInfo();
}

void BaseTimeCoordinator::disconnect()
{
    if (disconnected) {
        return;
    }

    if (dependencies.empty()) {
        disconnected = true;
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
        bool hasLocal{false};
        for (const auto& dep : dependencies) {
            if ((dep.dependency && dep.next < Time::maxVal()) || dep.dependent) {
                if (dep.fedID == mSourceId) {
                    hasLocal = true;
                } else {
                    bye.dest_id = dep.fedID;
                    appendMessage(multi, bye);
                }
            }
        }
        if (hasLocal) {
            bye.dest_id = mSourceId;
            processTimeMessage(bye);
        }
        sendMessageFunction(multi);
    }
    disconnected = true;
}

void BaseTimeCoordinator::generateDebuggingTimeInfo(nlohmann::json& base) const
{
    base["dependencies"] = nlohmann::json::array();
    base["federatesonly"] = federatesOnly;
    base["sequenceCounter"] = sequenceCounter;
    base["id"] = mSourceId.baseValue();
    for (const auto& dep : dependencies) {
        if (dep.dependency) {
            nlohmann::json depblock;
            generateJsonOutputDependency(depblock, dep);
            base["dependencies"].push_back(depblock);
        }
        if (dep.dependent) {
            base["dependents"].push_back(dep.fedID.baseValue());
        }
    }
}

nlohmann::json BaseTimeCoordinator::grantTimeoutCheck(const ActionMessage& cmd)
{
    for (auto& dep : dependencies) {
        if (dep.fedID == cmd.source_id) {
            dep.timeoutCount = cmd.counter;
            if (cmd.counter == 6) {
                nlohmann::json base;
                generateDebuggingTimeInfo(base);
                return base;
            }
        }
    }
    return nlohmann::json::object();
}

bool BaseTimeCoordinator::isDependency(GlobalFederateId ofed) const
{
    return dependencies.isDependency(ofed);
}

bool BaseTimeCoordinator::addDependency(GlobalFederateId fedID)
{
    if (dependencies.addDependency(fedID)) {
        if (fedID == mSourceId) {
            auto* dep = dependencies.getDependencyInfo(fedID);
            if (dep != nullptr) {
                dep->connection = ConnectionType::SELF;
            }
        }
        return true;
    }
    return false;
}

bool BaseTimeCoordinator::addDependent(GlobalFederateId fedID)
{
    return dependencies.addDependent(fedID);
}

void BaseTimeCoordinator::setAsChild(GlobalFederateId fedID)
{
    if (fedID == mSourceId) {
        return;
    }
    auto* dep = dependencies.getDependencyInfo(fedID);
    if (dep != nullptr) {
        dep->connection = ConnectionType::CHILD;
    }
}

void BaseTimeCoordinator::setAsParent(GlobalFederateId fedID)
{
    if (fedID == mSourceId) {
        return;
    }
    auto* dep = dependencies.getDependencyInfo(fedID);
    if (dep != nullptr) {
        dep->connection = ConnectionType::PARENT;
        noParent = false;
    }
}

void BaseTimeCoordinator::setVersion(GlobalFederateId fedID, std::int8_t version)
{
    auto* dep = dependencies.getDependencyInfo(fedID);
    if (dep != nullptr) {
        dep->timingVersion = version;
    }
}

GlobalFederateId BaseTimeCoordinator::getParent() const
{
    for (const auto& dep : dependencies) {
        if (dep.connection == ConnectionType::PARENT) {
            return dep.fedID;
        }
    }
    return {};
}

void BaseTimeCoordinator::removeDependency(GlobalFederateId fedID)
{
    dependencies.removeDependency(fedID);
}

void BaseTimeCoordinator::removeDependent(GlobalFederateId fedID)
{
    dependencies.removeDependent(fedID);
}

void BaseTimeCoordinator::resetDependency(GlobalFederateId fedID)
{
    dependencies.resetDependency(fedID);
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

Time BaseTimeCoordinator::getLastGrant(GlobalFederateId fedId) const
{
    const auto* dep = dependencies.getDependencyInfo(fedId);
    return (dep == nullptr) ? timeZero : dep->lastGrant;
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

void BaseTimeCoordinator::sendTimingInfo()
{
    ActionMessage tinfo(CMD_TIMING_INFO);
    tinfo.source_id = mSourceId;
    if (nonGranting) {
        setActionFlag(tinfo, non_granting_flag);
    }
    if (delayedTiming) {
        setActionFlag(tinfo, delayed_timing_flag);
    }
    tinfo.setExtraData(TIME_COORDINATOR_VERSION);

    for (const auto& dep : dependencies) {
        if (dep.dependent) {
            tinfo.dest_id = dep.fedID;
            sendMessageFunction(tinfo);
        }
    }
}

ActionMessage BaseTimeCoordinator::generateTimeRequest(const TimeData& dep,
                                                       GlobalFederateId fed,
                                                       std::int32_t responseCode) const
{
    ActionMessage nTime(CMD_TIME_REQUEST);
    nTime.source_id = mSourceId;
    nTime.dest_id = fed;
    nTime.actionTime = dep.next;
    if (dep.interrupted) {
        setActionFlag(nTime, interrupted_flag);
    }
    switch (dep.mTimeState) {
        case TimeState::time_granted:
            nTime.setAction(CMD_TIME_GRANT);
            break;
        case TimeState::time_requested:
            nTime.setExtraData(dep.minFed.baseValue());
            nTime.Tdemin = std::min(dep.minDe, dep.Te);
            nTime.Te = dep.Te;
            nTime.counter = sequenceCounter;
            nTime.setExtraDestData(responseCode);
            break;
        case TimeState::time_requested_iterative:
            nTime.setExtraData(dep.minFed.baseValue());
            setIterationFlags(nTime, IterationRequest::ITERATE_IF_NEEDED);
            nTime.Tdemin = std::min(dep.minDe, dep.Te);
            nTime.Te = dep.Te;
            nTime.counter = sequenceCounter;
            nTime.setExtraDestData(responseCode);
            break;
        case TimeState::time_requested_require_iteration:
            nTime.setExtraData(dep.minFed.baseValue());
            setIterationFlags(nTime, IterationRequest::FORCE_ITERATION);
            nTime.Tdemin = std::min(dep.minDe, dep.Te);
            nTime.counter = sequenceCounter;
            nTime.Te = dep.Te;
            nTime.setExtraDestData(responseCode);
            break;
        case TimeState::exec_requested:
            nTime.setAction(CMD_EXEC_REQUEST);
            nTime.actionTime = Time::zeroVal();
            nTime.counter = sequenceCounter;
            nTime.setExtraDestData(responseCode);
            break;
        case TimeState::error:
            nTime.setAction(CMD_IGNORE);
            // no need to send updates for this
            break;
        case TimeState::exec_requested_iterative:
            nTime.setAction(CMD_EXEC_REQUEST);
            setIterationFlags(nTime, IterationRequest::ITERATE_IF_NEEDED);
            nTime.setExtraData(dep.minFed.baseValue());
            nTime.counter = sequenceCounter;
            nTime.setExtraDestData(responseCode);
            break;
        case TimeState::exec_requested_require_iteration:
            nTime.setAction(CMD_EXEC_REQUEST);
            setIterationFlags(nTime, IterationRequest::FORCE_ITERATION);
            nTime.setExtraData(dep.minFed.baseValue());
            nTime.counter = sequenceCounter;
            nTime.setExtraDestData(responseCode);
            break;
        case TimeState::initialized:
            if (dep.responseSequenceCounter == 0) {
                nTime.setAction(CMD_IGNORE);
            } else {
                nTime.setAction(CMD_EXEC_GRANT);
                nTime.setExtraData(dep.minFed.baseValue());
                setIterationFlags(nTime, IterationRequest::ITERATE_IF_NEEDED);
                nTime.counter = sequenceCounter;
                nTime.setExtraDestData(responseCode);
            }

            break;
    }

    return nTime;
}

TimeProcessingResult BaseTimeCoordinator::processTimeMessage(const ActionMessage& cmd)
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
    if (procRes == TimeProcessingResult::PROCESSED_AND_CHECK) {
        auto checkRes = dependencies.checkForIssues(false);
        if (checkRes.first != 0) {
            ActionMessage error(CMD_GLOBAL_ERROR);
            error.dest_id = parent_broker_id;
            error.source_id = mSourceId;
            error.messageID = checkRes.first;
            error.payload = checkRes.second;
            sendMessageFunction(error);
        }
    }
    return procRes;
}

void BaseTimeCoordinator::processDependencyUpdateMessage(const ActionMessage& cmd)
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
        case CMD_TIMING_INFO:
            dependencies.updateTime(cmd);
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
        if (cmd.counter > 0) {
            setVersion(cmd.source_id, cmd.counter);
        }
    }
}

}  // namespace helics
