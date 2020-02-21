/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "ForwardingTimeCoordinator.hpp"

#include "../common/fmt_format.h"
#include "flagOperations.hpp"
#include "helics_definitions.hpp"

#include <algorithm>
#include <set>
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

void ForwardingTimeCoordinator::disconnect()
{
    if (sendMessageFunction) {
        std::set<global_federate_id> connections(dependents.begin(), dependents.end());
        for (auto dep : dependencies) {
            if (dep.Tnext < Time::maxVal()) {
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

class minTimeSet {
  public:
    Time minNext = Time::maxVal();
    Time minminDe = Time::maxVal();
    Time minDe = minminDe;
    global_federate_id minFed;
    DependencyInfo::time_state_t tState = DependencyInfo::time_state_t::time_requested;
};

static minTimeSet generateMinTimeSet(
    const TimeDependencies& dependencies,
    bool restricted,
    global_federate_id ignore = global_federate_id())
{
    minTimeSet mTime;
    for (auto& dep : dependencies) {
        if (dep.fedID == ignore) {
            continue;
        }
        if (dep.Tnext < mTime.minNext) {
            mTime.minNext = dep.Tnext;
            mTime.tState = dep.time_state;
        } else if (dep.Tnext == mTime.minNext) {
            if (dep.time_state == DependencyInfo::time_state_t::time_granted) {
                mTime.tState = dep.time_state;
            }
        }
        if (dep.Tdemin >= dep.Tnext) {
            if (dep.Tdemin < mTime.minminDe) {
                mTime.minminDe = dep.Tdemin;
                mTime.minFed = dep.fedID;
            } else if (dep.Tdemin == mTime.minminDe) {
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
        if (mTime.minminDe > mTime.minNext) {
            mTime.minNext = mTime.minminDe;
        }
    }
    return mTime;
}

void ForwardingTimeCoordinator::updateTimeFactors()
{
    auto mTime = generateMinTimeSet(dependencies, restrictive_time_policy);

    bool update = (time_state != mTime.tState);
    time_state = mTime.tState;

    Time prev_next = time_next;
    time_next = mTime.minNext;

    if (mTime.minDe != time_minDe) {
        update = true;
        time_minDe = mTime.minDe;
    }
    if (mTime.minminDe != time_minminDe) {
        time_minminDe = mTime.minminDe;
        update = true;
    }

    if (!restrictive_time_policy && time_minminDe < Time::maxVal()) {
        if (time_minminDe > time_next) {
            time_next = time_minminDe;
        }
    }
    //	printf("%d UDPATE next=%f, minminDE=%f, Tdemin=%f\n", source_id, static_cast<double>(time_next),
    // static_cast<double>(minminDe), static_cast<double>(minDe));
    if (prev_next != time_next) {
        update = true;
    }

    if (mTime.minFed != lastMinFed) {
        lastMinFed = mTime.minFed;
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
    if (time_state == DependencyInfo::time_state_t::time_granted) {
        ActionMessage upd(CMD_TIME_GRANT);
        upd.source_id = source_id;
        //    upd.source_handle = lastMinFed;
        upd.actionTime = time_next;
        if (iterating) {
            setActionFlag(upd, iteration_requested_flag);
        }
        transmitTimingMessage(upd);
    } else {
        ActionMessage upd(CMD_TIME_REQUEST);
        upd.source_id = source_id;
        //    upd.source_handle = lastMinFed;
        upd.actionTime = time_next;
        upd.Te = time_minDe;
        upd.Tdemin = time_minminDe;
        if (iterating) {
            setActionFlag(upd, iteration_requested_flag);
        }
        transmitTimingMessage(upd);

        //	printf("%d next=%f, exec=%f, Tdemin=%f\n", source_id, static_cast<double>(time_next),
        // static_cast<double>(time_exec), static_cast<double>(time_minDe));
    }
}

std::string ForwardingTimeCoordinator::printTimeStatus() const
{
    return fmt::format(
        " minDe={} minminDe={}",
        static_cast<double>(time_minDe),
        static_cast<double>(time_minminDe));
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
    time_next = timeZero;
    time_state = DependencyInfo::time_state_t::time_granted;
    time_minDe = timeZero;
    time_minminDe = timeZero;

    ActionMessage execgrant(CMD_EXEC_GRANT);
    execgrant.source_id = source_id;
    transmitTimingMessage(execgrant);

    return ret;
}

ActionMessage ForwardingTimeCoordinator::generateTimeRequestIgnoreDependency(
    const ActionMessage& msg,
    global_federate_id iFed) const
{
    auto mTime = generateMinTimeSet(dependencies, restrictive_time_policy, iFed);
    ActionMessage nTime(msg);

    nTime.actionTime = mTime.minNext;
    nTime.Tdemin = mTime.minminDe;
    nTime.Te = mTime.minDe;
    nTime.dest_id = iFed;

    if (mTime.tState == DependencyInfo::time_state_t::time_granted) {
        nTime.setAction(CMD_TIME_GRANT);
    } else if (mTime.tState == DependencyInfo::time_state_t::time_requested) {
        nTime.setAction(CMD_TIME_REQUEST);
        clearActionFlag(nTime, iteration_requested_flag);
    } else if (mTime.tState == DependencyInfo::time_state_t::time_requested_iterative) {
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
                        if ((di->Tnext == msg.actionTime) || (di->fedID == lastMinFed)) {
                            sendMessageFunction(generateTimeRequestIgnoreDependency(msg, dep));
                            continue;
                        }
                    }
                }
                auto di = getDependencyInfo(dep);
                if (di != nullptr) {
                    if (di->Tnext > msg.actionTime) {
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

} // namespace helics
