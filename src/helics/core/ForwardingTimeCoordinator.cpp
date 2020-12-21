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

#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <vector>
#include "json/json.h"

namespace helics {
void ForwardingTimeCoordinator::enteringExecMode()
{
    if (executionMode) {
        return;
    }
    checkingExec = true;
    ActionMessage execreq(CMD_EXEC_REQUEST);
    execreq.source_id = source_id;
    transmitTimingMessage(execreq);
}
/*
void ForwardingTimeCoordinator::disconnect()
{
if (sendMessageFunction) {
        if (dependencies.empty())
        {
            return;
        }
        ActionMessage bye(CMD_DISCONNECT);
        bye.source_id = source_id;
        if (dependencies.size() == 1)
        {
            auto& dep = *dependencies.begin();
            if ((dep.dependency && dep.next < Time::maxVal()) || dep.dependent)
            {
                bye.dest_id = dep.fedID;
                if (bye.dest_id == source_id) {
                    processTimeMessage(bye);
                } else {
                    sendMessageFunction(bye);
                }
            }

        }
        else
        {
            ActionMessage multi(CMD_MULTI_MESSAGE);
            for (auto dep : dependencies) {
                if ((dep.dependency && dep.next < Time::maxVal())||dep.dependent) {
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
*/
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

static inline bool isBroker(global_federate_id id)
{
    return ((id.baseValue() == 1) || (id.baseValue() >= 0x7000'0000));
}

static DependencyInfo generateMinTimeSet(const TimeDependencies& dependencies,
                                     bool restricted,
                                     global_federate_id ignore = global_federate_id())
{
    DependencyInfo mTime(Time::maxVal());
    for (auto& dep : dependencies) {
        if (dep.fedID == ignore) {
            continue;
        }
        if (dep.next < mTime.next) {
            mTime.next = dep.next;
            mTime.time_state = dep.time_state;
        } else if (dep.next == mTime.next) {
            if (dep.time_state == time_state_t::time_granted) {
                mTime.time_state = dep.time_state;
            }
        }
        if (dep.minDe >= dep.next) {
            if (dep.minDe < mTime.minminDe) {
                mTime.minminDe = dep.minDe;
                mTime.minFed = dep.fedID;
            } else if (dep.minDe == mTime.minminDe) {
                mTime.minFed = global_federate_id();
            }
        } else {
            // this minimum dependent event time received was invalid and can't be trusted
            // therefore it can't be used to determine a time grant
            mTime.minminDe = -1;
        }

        if (dep.Te < mTime.minDe) {
            mTime.minDe = dep.Te;
        }
    }

    mTime.minminDe = std::min(mTime.minDe, mTime.minminDe);

    if (!restricted && mTime.minminDe < Time::maxVal()) {
        if (mTime.minminDe > mTime.next) {
            mTime.next = mTime.minminDe;
        }
    }
    return mTime;
}

void ForwardingTimeCoordinator::updateTimeFactors()
{
    auto mTime = generateMinTimeSet(dependencies, restrictive_time_policy);

    bool update = (main.time_state != mTime.time_state);
    main.time_state = mTime.time_state;

    Time prev_next = main.next;
    main.next = mTime.next;

    if (mTime.minDe != main.minDe) {
        update = true;
        main.minDe = mTime.minDe;
    }
    if (mTime.minminDe != main.minminDe) {
        main.minminDe = mTime.minminDe;
        update = true;
    }

    if (!restrictive_time_policy && main.minminDe < Time::maxVal()) {
        if (main.minminDe > main.next) {
            main.next = main.minminDe;
        }
    }
    //    printf("%d UPDATE next=%f, minminDE=%f, Tdemin=%f\n", source_id,
    //    static_cast<double>(time_next),
    // static_cast<double>(minminDe), static_cast<double>(minDe));
    if (prev_next != main.next) {
        update = true;
    }

    if (mTime.minFed != main.minFed) {
        main.minFed = mTime.minFed;
        if (isBroker(mTime.minFed)) {
            update = true;
        }
    }
    if (update) {
        sendTimeRequest();
    }
}

void ForwardingTimeCoordinator::sendTimeRequest() const
{
    if (!sendMessageFunction) {
        return;
    }
    if (main.time_state == time_state_t::time_granted) {
        ActionMessage upd(CMD_TIME_GRANT);
        upd.source_id = source_id;
        //    upd.source_handle = lastMinFed;
        upd.actionTime = main.next;
        if (iterating) {
            setActionFlag(upd, iteration_requested_flag);
        }
        transmitTimingMessage(upd);
    } else {
        ActionMessage upd(CMD_TIME_REQUEST);
        upd.source_id = source_id;
        //    upd.source_handle = lastMinFed;
        upd.actionTime = main.next;
        upd.Te = main.minDe;
        upd.Tdemin = main.minminDe;
        upd.setExtraData(main.minFed.baseValue());
        if (iterating) {
            setActionFlag(upd, iteration_requested_flag);
        }
        transmitTimingMessage(upd);

        //    printf("%d next=%f, exec=%f, Tdemin=%f\n", source_id, static_cast<double>(time_next),
        // static_cast<double>(time_exec), static_cast<double>(time_minDe));
    }
}


void ForwardingTimeCoordinator::generateDebuggingTimeInfo(Json::Value& base) const
{
    base["type"] = "forwarding";
    base["next"] = static_cast<double>(main.next);
    base["minde"] = static_cast<double>(main.minDe);
    base["minminde"] = static_cast<double>(main.minminDe);
    base["minfed"] = main.minFed.baseValue();
    base["minfedActual"] = main.minFedActual.baseValue();
    base["dependencies"] = Json::arrayValue;
    for (auto dep : dependencies) {
        Json::Value depblock;
        depblock["id"] = dep.fedID.baseValue();
        depblock["state"] = static_cast<int>(dep.time_state);
        depblock["next"] = static_cast<double>(dep.next);
        depblock["te"] = static_cast<double>(dep.Te);
        depblock["minde"] = static_cast<double>(dep.minDe);
        depblock["minfed"] = dep.minFed.baseValue();
        base["dependencies"].append(depblock);
    }
    for (auto dep : dependents) {
        base["dependents"].append(dep.baseValue());
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
    dependencies.addDependent(fedID);

    if (dependents.empty()) {
        dependents.push_back(fedID);
        return true;
    }
    auto dep = std::lower_bound(dependents.begin(), dependents.end(), fedID);
    if (dep == dependents.end()) {
        dependents.push_back(fedID);
    } else {
        if (*dep == fedID) {
            return false;
        }
        dependents.insert(dep, fedID);
    }
    return true;
}

void ForwardingTimeCoordinator::removeDependency(global_federate_id fedID)
{
    dependencies.removeDependency(fedID);
}

void ForwardingTimeCoordinator::removeDependent(global_federate_id fedID)
{
    dependencies.removeDependent(fedID);
    auto dep = std::lower_bound(dependents.begin(), dependents.end(), fedID);
    if (dep != dependents.end()) {
        if (*dep == fedID) {
            dependents.erase(dep);
        }
    }
}

const DependencyInfo* ForwardingTimeCoordinator::getDependencyInfo(global_federate_id ofed) const
{
    return dependencies.getDependencyInfo(ofed);
}

std::vector<global_federate_id> ForwardingTimeCoordinator::getDependencies() const
{
    std::vector<global_federate_id> deps;
    for (auto& dep : dependencies) {
        deps.push_back(dep.fedID);
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
    transmitTimingMessage(execgrant);

    return ret;
}

ActionMessage
    ForwardingTimeCoordinator::generateTimeRequestIgnoreDependency(const ActionMessage& msg,
                                                                   global_federate_id iFed) const
{
    auto mTime = generateMinTimeSet(dependencies, restrictive_time_policy, iFed);
    ActionMessage nTime(msg);

    nTime.actionTime = mTime.next;
    nTime.Tdemin = mTime.minminDe;
    nTime.Te = mTime.minDe;
    nTime.dest_id = iFed;
    nTime.setExtraData(mTime.minFed.baseValue());
    if (mTime.time_state == time_state_t::time_granted) {
        nTime.setAction(CMD_TIME_GRANT);
    } else if (mTime.time_state == time_state_t::time_requested) {
        nTime.setAction(CMD_TIME_REQUEST);
        clearActionFlag(nTime, iteration_requested_flag);
    } else if (mTime.time_state == time_state_t::time_requested_iterative) {
        nTime.setAction(CMD_TIME_REQUEST);
        setActionFlag(nTime, iteration_requested_flag);
    }
    return nTime;
}

void ForwardingTimeCoordinator::transmitTimingMessage(ActionMessage& msg) const
{
    if (sendMessageFunction) {
        if ((msg.action() == CMD_TIME_REQUEST) || (msg.action() == CMD_TIME_GRANT)) {
            for (auto dep : dependents) {
                if ((isBroker(dep)) && (!ignoreMinFed)) {
                    auto di = getDependencyInfo(dep);
                    if (di != nullptr) {
                        if ((di->next == msg.actionTime) || (di->fedID == main.minFed)) {
                            sendMessageFunction(generateTimeRequestIgnoreDependency(msg, dep));
                            continue;
                        }
                    }
                }
                auto di = getDependencyInfo(dep);
                if (di != nullptr) {
                    if (di->next > msg.actionTime) {
                        continue;
                    }
                }

                msg.dest_id = dep;
                sendMessageFunction(msg);
            }
        } else {
            for (auto dep : dependents) {
                msg.dest_id = dep;
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
}

}  // namespace helics
