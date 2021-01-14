/*
Copyright (c) 2017-2020,
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
    transmitTimingMessages(execreq);
    bool fedOnly = true;
    for (const auto& dep : dependencies) {
        if (dep.parent) {
            fedOnly = false;
            break;
        }
        if (dep.child && dep.fedID.isBroker()) {
            fedOnly = false;
            break;
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

static DependencyInfo generateMinTimeSet(const TimeDependencies& dependencies,
                                         bool restricted,
                                         global_federate_id self,
                                         global_federate_id ignore = global_federate_id())
{
    DependencyInfo mTime(Time::maxVal());
    for (auto& dep : dependencies) {
        if (dep.dependency == false) {
            continue;
        }

        if (dep.minFedActualMinDe == self) {
            continue;
        }
        if (dep.fedID == ignore) {
            if (dep.fedID.isBroker()) {
                if (dep.Te < mTime.minDe) {
                    mTime.minDe = dep.Te;
                }
                if (mTime.minDe < mTime.minminDe) {
                    mTime.minminDe = mTime.minDe;
                }
            }

            continue;
        }

        if (dep.minDe >= dep.next) {
            if (dep.minDe < mTime.minminDe) {
                mTime.minminDe = dep.minDe;
                mTime.minFedMinDe = dep.fedID;
                if (dep.minFedMinDe.isValid()) {
                    mTime.minFedActualMinDe = dep.minFedActualMinDe;
                } else {
                    mTime.minFedActualMinDe = dep.fedID;
                }
            } else if (dep.minDe == mTime.minminDe) {
                mTime.minFedActualMinDe = global_federate_id();
            }
        } else {
            // this minimum dependent event time received was invalid and can't be trusted
            // therefore it can't be used to determine a time grant
            mTime.minminDe = -1;
        }

        if (dep.next < mTime.next) {
            mTime.next = dep.next;
            mTime.time_state = dep.time_state;
            mTime.minFedNext = dep.fedID;
            if (dep.minFedNext.isValid()) {
                mTime.minFedActualNext = dep.minFedActualNext;
            } else {
                mTime.minFedActualNext = dep.fedID;
            }
        } else if (dep.next == mTime.next) {
            if (dep.time_state == time_state_t::time_granted) {
                mTime.time_state = dep.time_state;
                mTime.minState = dep.fedID;
            }
        }
        if (dep.Te < mTime.minDe) {
            mTime.minDe = dep.Te;
            mTime.minFedEvent = dep.fedID;
            if (dep.minFedEvent.isValid()) {
                mTime.minFedActualEvent = dep.minFedActualEvent;
            } else {
                mTime.minFedActualEvent = dep.fedID;
            }
        }
    }

    mTime.minminDe = std::min(mTime.minDe, mTime.minminDe);

    if (!restricted && mTime.minminDe < Time::maxVal()) {
        if (mTime.minminDe > mTime.next) {
            mTime.next = mTime.minminDe;
        }
    }
    if (mTime.minFedMinDe.isValid()) {
        if (mTime.minFedEvent == mTime.minFedMinDe) {
            mTime.minFedEvent = global_federate_id{};
        }
        if (mTime.minFedNext == mTime.minFedMinDe) {
            mTime.minFedNext = global_federate_id{};
        }
        if (mTime.minState == mTime.minFedMinDe) {
            mTime.minState = global_federate_id{};
        }
    }

    if (mTime.minFedEvent.isValid()) {
        if (mTime.minFedNext == mTime.minFedEvent) {
            mTime.minFedNext = global_federate_id{};
        }
        if (mTime.minState == mTime.minFedEvent) {
            mTime.minState = global_federate_id{};
        }
    }

    if (mTime.minFedNext.isValid()) {
        if (mTime.minState == mTime.minFedNext) {
            mTime.minState = global_federate_id{};
        }
    }
    return mTime;
}

void ForwardingTimeCoordinator::updateTimeFactors()
{
    auto mTime = generateMinTimeSet(dependencies, restrictive_time_policy, source_id);

    bool update = main.update(mTime);
    bool minUpdateNext = false;
    bool minUpdateEvent = false;
    bool minUpdateMinDe = false;
    bool minUpdateState = false;
    if (!restrictive_time_policy && main.minminDe < Time::maxVal()) {
        if (main.minminDe > main.next) {
            main.next = main.minminDe;
        }
    }

    if (main.minFedNext.isValid()) {
        auto minTime =
            generateMinTimeSet(dependencies, restrictive_time_policy, source_id, main.minFedNext);
        minUpdateNext = minNext.update(minTime);
    }

    if (main.minFedEvent.isValid()) {
        auto minTime =
            generateMinTimeSet(dependencies, restrictive_time_policy, source_id, main.minFedEvent);
        minUpdateEvent = minEvent.update(minTime);
    }

    if (main.minFedMinDe.isValid()) {
        auto minTime =
            generateMinTimeSet(dependencies, restrictive_time_policy, source_id, main.minFedMinDe);
        minUpdateMinDe = minMinDe.update(minTime);
    }
    if (main.minState.isValid()) {
        auto minTime =
            generateMinTimeSet(dependencies, restrictive_time_policy, source_id, main.minState);
        minUpdateState = minState.update(minTime);
    }

    if (update) {
        auto upd = generateTimeRequest(main, global_federate_id{});
        transmitTimingMessages(upd);
    } else {
        if (minUpdateNext) {
            if (sendMessageFunction) {
                sendMessageFunction(generateTimeRequest(minNext, main.minFedNext));
            }
        }
        if (minUpdateEvent) {
            if (sendMessageFunction) {
                sendMessageFunction(generateTimeRequest(minEvent, main.minFedEvent));
            }
        }
        if (minUpdateMinDe) {
            if (sendMessageFunction) {
                sendMessageFunction(generateTimeRequest(minMinDe, main.minFedMinDe));
            }
        }

        if (minUpdateState) {
            if (sendMessageFunction) {
                sendMessageFunction(generateTimeRequest(minState, main.minState));
            }
        }
    }
}

void ForwardingTimeCoordinator::generateDebuggingTimeInfo(Json::Value& base) const
{
    base["type"] = "forwarding";
    Json::Value mainBlock;
    mainBlock["next"] = static_cast<double>(main.next);
    mainBlock["minde"] = static_cast<double>(main.minDe);
    mainBlock["minminde"] = static_cast<double>(main.minminDe);
    mainBlock["minfed"] = main.minFedMinDe.baseValue();
    mainBlock["state"] = static_cast<int32_t>(main.time_state);
    mainBlock["minfedActual"] = main.minFedActualMinDe.baseValue();
    base["main"] = mainBlock;
    Json::Value minBlock;
    minBlock["next"] = static_cast<double>(minMinDe.next);
    minBlock["minde"] = static_cast<double>(minMinDe.minDe);
    minBlock["minminde"] = static_cast<double>(minMinDe.minminDe);
    minBlock["minfed"] = minMinDe.minFedMinDe.baseValue();
    minBlock["minfedActual"] = minMinDe.minFedActualMinDe.baseValue();
    minBlock["state"] = static_cast<int32_t>(minMinDe.time_state);
    base["excl"] = minBlock;

    base["dependencies"] = Json::arrayValue;
    base["federatesonly"] = federatesOnly;
    for (auto dep : dependencies) {
        if (dep.dependency) {
            Json::Value depblock;
            depblock["id"] = dep.fedID.baseValue();
            depblock["state"] = static_cast<int>(dep.time_state);
            depblock["next"] = static_cast<double>(dep.next);
            depblock["te"] = static_cast<double>(dep.Te);
            depblock["minde"] = static_cast<double>(dep.minDe);
            depblock["minfed"] = dep.minFedMinDe.baseValue();
            depblock["parent"] = dep.parent;
            depblock["child"] = dep.child;
            base["dependencies"].append(depblock);
        }
        if (dep.dependent) {
            base["dependents"].append(dep.fedID.baseValue());
        }
    }
}

std::string ForwardingTimeCoordinator::printTimeStatus() const
{
    return fmt::format(R"raw({{"time_next":{}, "minDe":{}, "minminDe":{}}})raw",
                       static_cast<double>(main.next),
                       static_cast<double>(main.minDe),
                       static_cast<double>(main.minminDe));
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
        dep->child = true;
    }
}

void ForwardingTimeCoordinator::setAsParent(global_federate_id fedID)
{
    auto* dep = dependencies.getDependencyInfo(fedID);
    if (dep != nullptr) {
        dep->parent = true;
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

message_processing_result ForwardingTimeCoordinator::checkExecEntry()
{
    auto ret = message_processing_result::continue_processing;
    if (!dependencies.checkIfReadyForExecEntry(false)) {
        return ret;
    }

    ret = message_processing_result::next_step;

    executionMode = true;
    main.next = timeZero;
    main.time_state = time_state_t::time_granted;
    main.minDe = timeZero;
    main.minminDe = timeZero;

    ActionMessage execgrant(CMD_EXEC_GRANT);
    execgrant.source_id = source_id;
    transmitTimingMessages(execgrant);

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
        nTime.setExtraData(dep.minFedMinDe.baseValue());
        nTime.Tdemin = dep.minminDe;
        nTime.Te = dep.minDe;
    } else if (dep.time_state == time_state_t::time_requested_iterative) {
        nTime.setExtraData(dep.minFedMinDe.baseValue());
        setActionFlag(nTime, iteration_requested_flag);
        nTime.Tdemin = dep.minminDe;
        nTime.Te = dep.minDe;
    }
    return nTime;
}

void ForwardingTimeCoordinator::transmitTimingMessages(ActionMessage& msg) const
{
    if (sendMessageFunction) {
        if ((msg.action() == CMD_TIME_REQUEST)) {
            for (auto dep : dependencies) {
                if ((dep.child || dep.parent) && (!ignoreMinFed) && (!federatesOnly)) {
                    if (dep.dependency) {
                        if (dep.fedID == main.minFedMinDe)
                        {
                            sendMessageFunction(generateTimeRequest(minMinDe, dep.fedID));
                            continue;
                        }
                        if (dep.fedID == main.minFedNext) {
                            sendMessageFunction(generateTimeRequest(minNext, dep.fedID));
                            continue;
                        }
                        if (dep.fedID == main.minFedEvent) {
                            sendMessageFunction(generateTimeRequest(minEvent, dep.fedID));
                            continue;
                        }
                        if (dep.fedID == main.minState) {
                            sendMessageFunction(generateTimeRequest(minState, dep.fedID));
                            continue;
                        }
                    }
                }
                if (dep.dependency) {
                    if (dep.next > msg.actionTime) {
                        continue;
                    }
                }

                msg.dest_id = dep.fedID;
                sendMessageFunction(msg);
            }
        } else if (msg.action() == CMD_TIME_GRANT) {
            for (auto dep : dependencies) {
                if (dep.dependency) {
                    if (dep.next > msg.actionTime) {
                        continue;
                    }
                }
                msg.dest_id = dep.fedID;
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
