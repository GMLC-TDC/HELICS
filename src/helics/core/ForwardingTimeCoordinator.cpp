/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "ForwardingTimeCoordinator.hpp"
#include "../flag-definitions.h"
#include <algorithm>
#include "../common/fmt_format.h"

namespace helics
{
void ForwardingTimeCoordinator::enteringExecMode ()
{
    if (executionMode)
    {
        return;
    }
    checkingExec = true;
    ActionMessage execreq (CMD_EXEC_REQUEST);
    execreq.source_id = source_id;
    transmitTimingMessage (execreq);
}

static inline bool isBroker (Core::federate_id_t id) { return ((id == 1) || (id >= 0x7000'0000)); }

void ForwardingTimeCoordinator::updateTimeFactors ()
{
    Time minNext = Time::maxVal ();
    Time minminDe = Time::maxVal ();
    Time minDe = minminDe;
    Core::federate_id_t minFed = invalid_fed_id;
    DependencyInfo::time_state_t tState = DependencyInfo::time_state_t::time_requested;
    for (auto &dep : dependencies)
    {
        if (dep.minFed == source_id)
        {
            continue;
        }
        if (dep.Tnext < minNext)
        {
            minNext = dep.Tnext;
            tState = dep.time_state;
        }
        else if (dep.Tnext == minNext)
        {
            if (dep.time_state == DependencyInfo::time_state_t::time_granted)
            {
                tState = dep.time_state;
            }
        }
        if (dep.Tdemin >= dep.Tnext)
        {
            if (dep.Tdemin < minminDe)
            {
                minminDe = dep.Tdemin;
                minFed = dep.fedID;
            }
            else if (dep.Tdemin == minminDe)
            {
                minFed = invalid_fed_id;
            }
        }
        else
        {
            // this minimum dependent event time received was invalid and can't be trusted
            // therefore it can't be used to determine a time grant
            minminDe = -1;
        }

        if (dep.Te < minDe)
        {
            minDe = dep.Te;
        }
    }

    minminDe = std::min (minDe, minminDe);

    bool update = (time_state != tState);
    time_state = tState;

    Time prev_next = time_next;
    time_next = minNext;

    if (minDe != time_minDe)
    {
        update = true;
        time_minDe = minDe;
    }
    if (minminDe != time_minminDe)
    {
        time_minminDe = minminDe;
        update = true;
    }

    if (time_minminDe < Time::maxVal ())
    {
        if (time_minminDe > time_next)
        {
            time_next = time_minminDe;
        }
    }
    //	printf("%d UDPATE next=%f, minminDE=%f, Tdemin=%f\n", source_id, static_cast<double>(time_next),
    // static_cast<double>(minminDe), static_cast<double>(minDe));
    if (prev_next != time_next)
    {
        update = true;
    }

    if (minFed != lastMinFed)
    {
        lastMinFed = minFed;
        if (isBroker (minFed))
        {
            update = true;
        }
    }
    if (update)
    {
        sendTimeRequest ();
    }
}
/*

message_processing_result ForwardingTimeCoordinator::checkTimeGrant()
{
    bool update = updateTimeFactors();
    if ((!iterating) || (time_exec > time_granted))
    {
        if (time_allow > time_exec)
        {
            updateTimeGrant();
            return message_processing_result::next_step;
        }
        if (time_allow == time_exec)
        {
            if (time_requested <= time_exec)
            {
                updateTimeGrant();
                return message_processing_result::next_step;
            }
            if (dependencies.checkIfReadyForTimeGrant(false, time_exec))
            {
                updateTimeGrant();
                return message_processing_result::next_step;
            }
        }
    }
    else
    {
        if (time_allow > time_exec)
        {
            dependencies.resetIteratingTimeRequests(time_exec);
            updateTimeGrant();
            return message_processing_result::iterating;
        }
        if (time_allow == time_exec)  // time_allow==time_exec==time_granted
        {
            if (dependencies.checkIfReadyForTimeGrant(true, time_exec))
            {
                dependencies.resetIteratingTimeRequests(time_exec);
                updateTimeGrant();
                return message_processing_result::iterating;
            }
        }
    }

    // if we haven't returned we need to update the time messages
    if ((!dependents.empty()) && (update))
    {
        sendTimeRequest();
    }
    return message_processing_result::continue_processing;
}

*/
void ForwardingTimeCoordinator::sendTimeRequest () const
{
    if (!sendMessageFunction)
    {
        return;
    }
    if (time_state == DependencyInfo::time_state_t::time_granted)
    {
        ActionMessage upd (CMD_TIME_GRANT);
        upd.source_id = source_id;
        upd.source_handle = lastMinFed;
        upd.actionTime = time_next;
        if (iterating)
        {
            setActionFlag (upd, iteration_requested_flag);
        }
        transmitTimingMessage (upd);
    }
    else
    {
        ActionMessage upd (CMD_TIME_REQUEST);
        upd.source_id = source_id;
        upd.source_handle = lastMinFed;
        upd.actionTime = time_next;
        upd.Te = time_minDe;
        upd.Tdemin = time_minminDe;
        if (iterating)
        {
            setActionFlag (upd, iteration_requested_flag);
        }
        transmitTimingMessage (upd);

        //	printf("%d next=%f, exec=%f, Tdemin=%f\n", source_id, static_cast<double>(time_next),
        // static_cast<double>(time_exec), static_cast<double>(time_minDe));
    }
}

std::string ForwardingTimeCoordinator::printTimeStatus () const
{
    return fmt::format (" minDe={} minminDe={}", static_cast<double> (time_minDe),
            static_cast<double> (time_minminDe));
}

bool ForwardingTimeCoordinator::isDependency (Core::federate_id_t ofed) const
{
    return dependencies.isDependency (ofed);
}

bool ForwardingTimeCoordinator::addDependency (Core::federate_id_t fedID)
{
    return dependencies.addDependency (fedID);
}

bool ForwardingTimeCoordinator::addDependent (Core::federate_id_t fedID)
{
    if (dependents.empty ())
    {
        dependents.push_back (fedID);
        return true;
    }
    auto dep = std::lower_bound (dependents.begin (), dependents.end (), fedID);
    if (dep == dependents.end ())
    {
        dependents.push_back (fedID);
    }
    else
    {
        if (*dep == fedID)
        {
            return false;
        }
        dependents.insert (dep, fedID);
    }
    return true;
}

void ForwardingTimeCoordinator::removeDependency (Core::federate_id_t fedID)
{
    dependencies.removeDependency (fedID);
}

void ForwardingTimeCoordinator::removeDependent (Core::federate_id_t fedID)
{
    auto dep = std::lower_bound (dependents.begin (), dependents.end (), fedID);
    if (dep != dependents.end ())
    {
        if (*dep == fedID)
        {
            dependents.erase (dep);
        }
    }
}

const DependencyInfo *ForwardingTimeCoordinator::getDependencyInfo (Core::federate_id_t ofed) const
{
    return dependencies.getDependencyInfo (ofed);
}

std::vector<Core::federate_id_t> ForwardingTimeCoordinator::getDependencies () const
{
    std::vector<Core::federate_id_t> deps;
    for (auto &dep : dependencies)
    {
        deps.push_back (dep.fedID);
    }
    return deps;
}

bool ForwardingTimeCoordinator::hasActiveTimeDependencies () const
{
    return dependencies.hasActiveTimeDependencies ();
}

message_processing_result ForwardingTimeCoordinator::checkExecEntry ()
{
    auto ret = message_processing_result::continue_processing;
    if (!dependencies.checkIfReadyForExecEntry (false))
    {
        return ret;
    }

    ret = message_processing_result::next_step;

    executionMode = true;
    time_next = timeZero;
    time_state = DependencyInfo::time_state_t::time_granted;
    time_minDe = timeZero;
    time_minminDe = timeZero;

    ActionMessage execgrant (CMD_EXEC_GRANT);
    execgrant.source_id = source_id;
    transmitTimingMessage (execgrant);

    return ret;
}

ActionMessage ForwardingTimeCoordinator::generateTimeRequestIgnoreDependency (const ActionMessage &msg,
                                                                              Core::federate_id_t iFed) const
{
    ActionMessage nTime (msg);
    Time minNext = Time::maxVal ();
    Time minminDe = Time::maxVal ();
    Time minDe = minminDe;
    for (auto &dep : dependencies)
    {
        if ((dep.minFed == source_id) || (dep.fedID == iFed))
        {
            continue;
        }
        if (dep.Tnext < minNext)
        {
            minNext = dep.Tnext;
        }
        if (dep.Tdemin >= dep.Tnext)
        {
            if (dep.Tdemin < minminDe)
            {
                minminDe = dep.Tdemin;
            }
        }
        else
        {
            // this minimum dependent event time received was invalid and can't be trusted
            // therefore it can't be used to determine a time grant
            minminDe = -1;
        }

        if (dep.Te < minDe)
        {
            minDe = dep.Te;
        }
    }

    minminDe = std::min (minDe, minminDe);

    if (minminDe < Time::maxVal ())
    {
        if (minminDe > minNext)
        {
            minNext = minminDe;
        }
    }
    nTime.actionTime = minNext;
    nTime.Tdemin = minminDe;
    nTime.Te = minDe;
    nTime.dest_id = iFed;
    return nTime;
}

void ForwardingTimeCoordinator::transmitTimingMessage (ActionMessage &msg) const
{
    if (sendMessageFunction)
    {
        if (msg.action () == CMD_TIME_REQUEST)
        {
            for (auto dep : dependents)
            {
                if (isBroker (dep))
                {
                    auto di = getDependencyInfo (dep);
                    if (di != nullptr)
                    {
                        if (di->Tnext == msg.actionTime)
                        {
                            sendMessageFunction (generateTimeRequestIgnoreDependency (msg, dep));
                            continue;
                        }
                    }
                }

                msg.dest_id = dep;
                sendMessageFunction (msg);
            }
        }
        else
        {
            for (auto dep : dependents)
            {
                msg.dest_id = dep;
                sendMessageFunction (msg);
            }
        }
    }
}

bool ForwardingTimeCoordinator::processTimeMessage (const ActionMessage &cmd)
{
    return dependencies.updateTime (cmd);
}

void ForwardingTimeCoordinator::processDependencyUpdateMessage (const ActionMessage &cmd)
{
    switch (cmd.action ())
    {
    case CMD_ADD_DEPENDENCY:
        addDependency (cmd.source_id);
        break;
    case CMD_REMOVE_DEPENDENCY:
        removeDependency (cmd.source_id);
        break;
    case CMD_ADD_DEPENDENT:
        addDependent (cmd.source_id);
        break;
    case CMD_REMOVE_DEPENDENT:
        removeDependent (cmd.source_id);
        break;
    case CMD_ADD_INTERDEPENDENCY:
        addDependency (cmd.source_id);
        addDependent (cmd.source_id);
        break;
    case CMD_REMOVE_INTERDEPENDENCY:
        removeDependency (cmd.source_id);
        removeDependent (cmd.source_id);
        break;
    default:
        break;
    }
}

}  // namespace helics
