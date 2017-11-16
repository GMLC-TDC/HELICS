/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "TimeCoordinator.h"

#include <algorithm>

namespace helics
{
TimeCoordinator::TimeCoordinator (const CoreFederateInfo &info_) : info (info_)
{
    if (info.timeDelta <= timeZero)
    {
        info.timeDelta = timeEpsilon;
    }
}

void TimeCoordinator::enteringExecMode (iteration_request mode)
{
    if (executionMode)
    {
        return;
    }
    iterating = (mode != iteration_request::no_iterations);
    checkingExec = true;
    if ((!dependents.empty ()) && (sendMessageFunction))
    {
        ActionMessage execreq (CMD_EXEC_REQUEST);
        execreq.source_id = source_id;
        execreq.iterationComplete = !iterating;
        sendMessageFunction (execreq);
    }
}

void TimeCoordinator::timeRequest (Time nextTime,
                                   iteration_request iterate,
                                   Time newValueTime,
                                   Time newMessageTime)
{
    iterating = (iterate != iteration_request::no_iterations);
    if (nextTime <= time_granted)
    {
        nextTime = time_granted + info.timeDelta;
    }
    time_requested = nextTime;

    time_value = newValueTime;
    time_message = newMessageTime;
    updateNextExecutionTime ();
    updateNextPossibleEventTime ();
    if ((!dependents.empty ()) && (sendMessageFunction))
    {
        ActionMessage treq (CMD_TIME_REQUEST);
        treq.iterationComplete = !iterating;
        treq.source_id = source_id;
        treq.actionTime = time_next;
        treq.info ().Te = time_exec + info.lookAhead;
        treq.info ().Tdemin = time_minDe;
        sendMessageFunction (treq);
    }
}

void TimeCoordinator::updateNextExecutionTime ()
{
    time_exec = std::min (time_message, time_value) + info.impactWindow;
    time_exec = std::min (time_requested, time_exec);
    if (time_exec <= time_granted)
    {
        time_exec = (iterating) ? time_granted : (time_granted + info.timeDelta);
    }
    if (info.period > timeEpsilon)
    {
        auto blk = static_cast<int> (std::ceil ((time_exec - time_granted) / info.period));
        time_exec = time_granted + blk * info.period;
    }
}

void TimeCoordinator::updateNextPossibleEventTime ()
{
    if (!iterating)
    {
        time_next = time_granted + info.timeDelta + info.lookAhead;
    }
    else
    {
        time_next = time_granted + info.lookAhead;
    }
    time_next = std::max (time_next, time_minminDe + info.impactWindow + info.lookAhead);
    time_next = std::min (time_next, time_exec);
}
void TimeCoordinator::updateValueTime (Time valueUpdateTime)
{
    if (!executionMode)  // updates before exec mode
    {
        if (valueUpdateTime < timeZero)
        {
            hasInitUpdates = true;
        }
        return;
    }
    valueUpdateTime += info.impactWindow;
    if (valueUpdateTime < time_value)
    {
        if (iterating)
        {
            time_value = (valueUpdateTime <= time_granted) ? time_granted : valueUpdateTime;
        }
        else
        {
            if (valueUpdateTime <= time_granted + info.timeDelta)
            {
                time_value = time_granted + info.timeDelta;
            }
            else
            {
                time_value = valueUpdateTime;
            }
        }
    }
}

void TimeCoordinator::updateMessageTime (Time messageUpdateTime)
{
    if (!executionMode)  // updates before exec mode
    {
        if (messageUpdateTime < timeZero)
        {
            hasInitUpdates = true;
        }
        return;
    }
    messageUpdateTime += info.impactWindow;
    if (messageUpdateTime < time_message)
    {
        if (iterating)
        {
            time_message = (messageUpdateTime <= time_granted) ? time_granted : messageUpdateTime;
        }
        else
        {
            if (messageUpdateTime <= time_granted + info.timeDelta)
            {
                time_message = time_granted + info.timeDelta;
            }
            else
            {
                time_message = messageUpdateTime;
            }
        }
    }
}

bool TimeCoordinator::updateTimeFactors ()
{
    Time minNext = Time::maxVal ();
    Time minminDe = Time::maxVal ();
    Time minDe = Time::maxVal ();
    for (auto &dep : dependencies)
    {
        if (dep.Tnext < minNext)
        {
            minNext = dep.Tnext;
        }
        if (dep.Tdemin < minDe)
        {
            minminDe = dep.Tdemin;
        }
        if (dep.Te < minDe)
        {
            minDe = dep.Te;
        }
    }
    bool update = false;
    time_minminDe = std::min (minDe, minminDe);
    Time prev_next = time_next;
    updateNextPossibleEventTime ();

    //	printf("%d UDPATE next=%f, minminDE=%f, Tdemin=%f\n", source_id, static_cast<double>(time_next),
    // static_cast<double>(minminDe), static_cast<double>(minDe));
    if (prev_next != time_next)
    {
        update = true;
    }

    if (minDe != time_minDe)
    {
        update = true;
        time_minDe = minDe;
    }
    time_allow = info.impactWindow + minNext;
    updateNextExecutionTime ();
    return update;
}

iteration_state TimeCoordinator::checkTimeGrant ()
{
    bool update = updateTimeFactors ();
    if ((!iterating) || (time_exec > time_granted))
    {
        if (time_allow >= time_exec)
        {
            time_granted = time_exec;
            if ((!dependents.empty ()) && (sendMessageFunction))
            {
                ActionMessage treq (CMD_TIME_GRANT);
                treq.source_id = source_id;
                treq.actionTime = time_granted;
                sendMessageFunction (treq);
            }
            // printf("%d GRANT allow=%f next=%f, exec=%f, Tdemin=%f\n", source_id,
            // static_cast<double>(time_allow), static_cast<double>(time_next), static_cast<double>(time_exec),
            // static_cast<double>(time_minDe));
            return iteration_state::next_step;
        }
    }
    else
    {
        if (time_allow > time_exec)
        {
            time_granted = time_exec;
            if ((!dependents.empty ()) && (sendMessageFunction))
            {
                ActionMessage treq (CMD_TIME_GRANT);
                treq.source_id = source_id;
                treq.actionTime = time_granted;
                sendMessageFunction (treq);
            }
            return iteration_state::iterating;
        }
        else if (time_allow == time_exec)  // time_allow==time_exec==time_granted
        {
            if (dependencies.checkIfReadyForTimeGrant (true, time_exec))
            {
                dependencies.ResetIteratingTimeRequests (time_exec);
                if ((!dependents.empty ()) && (sendMessageFunction))
                {
                    ActionMessage treq (CMD_TIME_GRANT);
                    treq.source_id = source_id;
                    treq.actionTime = time_granted;
                    sendMessageFunction (treq);
                }
                return iteration_state::iterating;
            }
        }
    }

    // if we haven't returned we need to update the time messages
    if ((!dependents.empty ()) && (update))
    {
        ActionMessage upd (CMD_TIME_REQUEST);
        upd.source_id = source_id;
        upd.actionTime = time_next;
        upd.info ().Te = time_exec;
        upd.info ().Tdemin = time_minDe;
        sendMessageFunction (upd);

        //	printf("%d next=%f, exec=%f, Tdemin=%f\n", source_id, static_cast<double>(time_next),
        // static_cast<double>(time_exec), static_cast<double>(time_minDe));
    }
    return iteration_state::continue_processing;
}

bool TimeCoordinator::isDependency (Core::federate_id_t ofed) const { return dependencies.isDependency (ofed); }

bool TimeCoordinator::addDependency (Core::federate_id_t fedID) { return dependencies.addDependency (fedID); }

bool TimeCoordinator::addDependent (Core::federate_id_t fedID)
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

void TimeCoordinator::removeDependency (Core::federate_id_t fedID) { dependencies.removeDependency (fedID); }

void TimeCoordinator::removeDependent (Core::federate_id_t fedID)
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

DependencyInfo *TimeCoordinator::getDependencyInfo (Core::federate_id_t ofed)
{
    return dependencies.getDependencyInfo (ofed);
}

iteration_state TimeCoordinator::checkExecEntry ()
{
    auto ret = iteration_state::continue_processing;
    if (!dependencies.checkIfReadyForExecEntry (iterating))
    {
        return ret;
    }
    if (iterating)
    {
        if (hasInitUpdates)
        {
            if (iteration > info.max_iterations)
            {
                ret = iteration_state::next_step;
            }
            else
            {
                ret = iteration_state::iterating;
            }
        }
        else
        {
            ret = iteration_state::next_step;  // todo add a check for updates and iteration limit
        }
    }
    else
    {
        ret = iteration_state::next_step;
    }

    if (ret == iteration_state::next_step)
    {
        time_granted = timeZero;
        executionMode = true;

        if (sendMessageFunction)
        {
            ActionMessage execgrant (CMD_EXEC_GRANT);
            execgrant.source_id = source_id;
            execgrant.iterationComplete = true;
            sendMessageFunction (execgrant);
        }
    }
    else if (ret == iteration_state::iterating)
    {
        dependencies.ResetIteratingExecRequests ();
        hasInitUpdates = false;
        if (sendMessageFunction)
        {
            ActionMessage execgrant (CMD_EXEC_GRANT);
            execgrant.source_id = source_id;
            execgrant.iterationComplete = false;
            sendMessageFunction (execgrant);
        }
    }
    return ret;
}

bool TimeCoordinator::processTimeMessage (ActionMessage &cmd) { return dependencies.updateTime (cmd); }
}  // namespace helics