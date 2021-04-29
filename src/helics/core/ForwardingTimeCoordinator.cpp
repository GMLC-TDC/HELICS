/*
Copyright (c) 2017-2021,
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
    ActionMessage execreq(CMD_EXEC_REQUEST);
    execreq.source_id = source_id;
    transmitTimingMessagesUpstream(execreq);
    transmitTimingMessagesDownstream(execreq);
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
}

/*
void ForwardingTimeCoordinator::disconnect()
{
    if (sendMessageFunction) {
        std::set<global_federate_id> connections(dependents.begin(), dependents.end());
        for (auto dep : dependencies) {
            if (dep.next < Time::maxVal()) {
                connections.insert(dep.fedID);
            }
        }
        if (connections.empty()) {
            return;
        }
        ActionMessage bye(CMD_DISCONNECT);

        bye.source_id = source_id;
        if (connections.size() == 1) {
            bye.dest_id = *connections.begin();
            if (bye.dest_id == source_id) {
                processTimeMessage(bye);
            } else {
                sendMessageFunction(bye);
            }
        } else {
            ActionMessage multi(CMD_MULTI_MESSAGE);
            for (auto fed : connections) {
                bye.dest_id = fed;
                if (fed == source_id) {
                    processTimeMessage(bye);
                } else {
                    appendMessage(multi, bye);
                }
            }
            sendMessageFunction(multi);
        }
    }
}
*/

void ForwardingTimeCoordinator::updateTimeFactors()
{
    auto mTimeUpstream = generateMinTimeUpstream(dependencies, restrictive_time_policy, source_id);
    auto mTimeDownstream = (noParent) ?
        mTimeUpstream :
        generateMinTimeDownstream(dependencies, restrictive_time_policy, source_id);

    bool updateUpstream = upstream.update(mTimeUpstream);

    bool updateDownstream = downstream.update(mTimeDownstream);

    if (!restrictive_time_policy && upstream.minDe < Time::maxVal()) {
        if (downstream.minDe > downstream.next) {
            //     downstream.next = downstream.minminDe;
        }
    }

    if (updateUpstream) {
        auto upd = generateTimeRequest(upstream, global_federate_id{});
        transmitTimingMessagesUpstream(upd);
    }
    if (updateDownstream) {
        auto upd = generateTimeRequest(downstream, global_federate_id{});
        transmitTimingMessagesDownstream(upd);
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
    for (auto dep : dependencies) {
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

bool ForwardingTimeCoordinator::isDependency(global_federate_id ofed) const
{
    return dependencies.isDependency(ofed);
}

bool ForwardingTimeCoordinator::addDependency(global_federate_id fedID)
{
    return dependencies.addDependency(fedID);
}

bool ForwardingTimeCoordinator::addDependent(global_federate_id fedID)
{
    return dependencies.addDependent(fedID);
}

void ForwardingTimeCoordinator::setAsChild(global_federate_id fedID)
{
    auto* dep = dependencies.getDependencyInfo(fedID);
    if (dep != nullptr) {
        dep->connection = ConnectionType::child;
    }
}

void ForwardingTimeCoordinator::setAsParent(global_federate_id fedID)
{
    auto* dep = dependencies.getDependencyInfo(fedID);
    if (dep != nullptr) {
        dep->connection = ConnectionType::parent;
    }
}

void ForwardingTimeCoordinator::removeDependency(global_federate_id fedID)
{
    dependencies.removeDependency(fedID);
}

void ForwardingTimeCoordinator::removeDependent(global_federate_id fedID)
{
    dependencies.removeDependent(fedID);
}

const DependencyInfo* ForwardingTimeCoordinator::getDependencyInfo(global_federate_id ofed) const
{
    return dependencies.getDependencyInfo(ofed);
}

std::vector<global_federate_id> ForwardingTimeCoordinator::getDependencies() const
{
    std::vector<global_federate_id> deps;
    for (auto& dep : dependencies) {
        if (dep.dependency) {
            deps.push_back(dep.fedID);
        }
    }
    return deps;
}

std::vector<global_federate_id> ForwardingTimeCoordinator::getDependents() const
{
    std::vector<global_federate_id> deps;
    for (auto& dep : dependencies) {
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
global_federate_id ForwardingTimeCoordinator::getMinDependency() const
{
    return dependencies.getMinDependency();
}

message_processing_result ForwardingTimeCoordinator::checkExecEntry()
{
    auto ret = message_processing_result::continue_processing;
    if (!dependencies.checkIfReadyForExecEntry(false)) {
        return ret;
    }

    ret = message_processing_result::next_step;

    executionMode = true;
    downstream.next = timeZero;
    downstream.time_state = time_state_t::time_granted;
    downstream.minDe = timeZero;

    ActionMessage execgrant(CMD_EXEC_GRANT);
    execgrant.source_id = source_id;
    transmitTimingMessagesDownstream(execgrant);
    transmitTimingMessagesUpstream(execgrant);
    return ret;
}

ActionMessage ForwardingTimeCoordinator::generateTimeRequest(const DependencyInfo& dep,
                                                             global_federate_id fed) const
{
    ActionMessage nTime(CMD_TIME_REQUEST);
    nTime.source_id = source_id;
    nTime.dest_id = fed;
    nTime.actionTime = dep.next;

    if (dep.time_state == time_state_t::time_granted) {
        nTime.setAction(CMD_TIME_GRANT);
    } else if (dep.time_state == time_state_t::time_requested) {
        nTime.setExtraData(dep.minFed.baseValue());
        nTime.Tdemin = std::min(dep.minDe, dep.Te);
        nTime.Te = dep.Te;
    } else if (dep.time_state == time_state_t::time_requested_iterative) {
        nTime.setExtraData(dep.minFed.baseValue());
        setActionFlag(nTime, iteration_requested_flag);
        nTime.Tdemin = std::min(dep.minDe, dep.Te);
        nTime.Te = dep.Te;
    }
    return nTime;
}

void ForwardingTimeCoordinator::transmitTimingMessagesUpstream(ActionMessage& msg) const
{
    if (sendMessageFunction) {
        for (auto dep : dependencies) {
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
}

void ForwardingTimeCoordinator::transmitTimingMessagesDownstream(ActionMessage& msg) const
{
    if (sendMessageFunction) {
        if ((msg.action() == CMD_TIME_REQUEST || msg.action() == CMD_TIME_GRANT)) {
            for (auto dep : dependencies) {
                if (dep.connection != ConnectionType::child) {
                    continue;
                }
                if (!dep.dependent) {
                    continue;
                }

                if (dep.dependency) {
                    if (dep.next > msg.actionTime) {
                        continue;
                    }
                }
                msg.dest_id = dep.fedID;
                /*if (msg.dest_id == global_federate_id(131074)) {
                    if (msg.actionTime > timeZero) {
                        printf("sending TR to 131074 for next=%f\n",
                               static_cast<double>(msg.actionTime));
                    }
                }
                */
                sendMessageFunction(msg);
            }
        } else {
            for (auto dep : dependencies) {
                if (dep.dependent) {
                    msg.dest_id = dep.fedID;
                    sendMessageFunction(msg);
                }
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
    return dependencies.updateTime(cmd);
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
