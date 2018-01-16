/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "TimeCoordinator.h"
#include "../flag-definitions.h"
#include <algorithm>
#include <boost/format.hpp>

namespace helics
{
TimeCoordinator::TimeCoordinator (const CoreFederateInfo &info_) : info (info_)
{
    if (info.timeDelta <= timeZero)
    {
        info.timeDelta = timeEpsilon;
    }
}

void TimeCoordinator::enteringExecMode (helics_iteration_request mode)
{
    if (executionMode)
    {
        return;
    }
    iterating = (mode != helics_iteration_request::no_iterations);
    checkingExec = true;
    if ((!dependents.empty ()) && (sendMessageFunction))
    {
        ActionMessage execreq (CMD_EXEC_REQUEST);
        execreq.source_id = source_id;
        if (iterating)
        {
            SET_ACTION_FLAG (execreq, iterationRequested);
        }
        sendMessageFunction (execreq);
    }
}

void TimeCoordinator::timeRequest (Time nextTime,
                                   helics_iteration_request iterate,
                                   Time newValueTime,
                                   Time newMessageTime)
{
    iterating = (iterate != helics_iteration_request::no_iterations);
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
        if (iterating)
        {
            SET_ACTION_FLAG (treq, iterationRequested);
        }
        treq.source_id = source_id;
        treq.actionTime = time_next;
        treq.Te = time_exec + info.outputDelay;
        treq.Tdemin = time_minDe;
        sendMessageFunction (treq);
    }
}

void TimeCoordinator::updateNextExecutionTime ()
{
    time_exec = std::min (time_message, time_value);
    if (time_exec < Time::maxVal ())
    {
        time_exec += info.inputDelay;
    }
    time_exec = std::min (time_requested, time_exec);
    if (time_exec <= time_granted)
    {
        time_exec = (iterating) ? time_granted : (time_granted + info.timeDelta);
    }
    if (((time_exec - time_granted) > 0.0) && (info.period > timeEpsilon))
    {
        auto blk = static_cast<int> (std::ceil ((time_exec - time_granted) / info.period));
        time_exec = time_granted + blk * info.period;
    }
}

void TimeCoordinator::updateNextPossibleEventTime ()
{
    if (!iterating)
    {
        time_next = time_granted + info.timeDelta + info.outputDelay;
        // the next time must fall on a period boundary
        if (info.period > timeEpsilon)
        {
            auto blk = static_cast<int> (std::ceil ((time_next - time_granted) / info.period));
            time_next = time_granted + blk * info.period;
        }
    }
    else
    {
        time_next = time_granted + info.outputDelay;
    }
    time_next = std::max (time_next, time_minminDe + info.inputDelay + info.outputDelay);
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
    Time minminDe = std::min (time_value, time_message);
    Time minDe = minminDe;
    for (auto &dep : dependencies)
    {
        if (dep.Tnext < minNext)
        {
            minNext = dep.Tnext;
        }
        if ((dep.Tdemin < minDe) && (dep.Tdemin >= dep.Tnext))
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
    time_allow = info.inputDelay + minNext;
    updateNextExecutionTime ();
    return update;
}

iteration_state TimeCoordinator::checkTimeGrant ()
{
    bool update = updateTimeFactors ();
    if ((!iterating) || (time_exec > time_granted))
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
            // printf("%d GRANT allow=%f next=%f, exec=%f, Tdemin=%f\n", source_id,
            // static_cast<double>(time_allow), static_cast<double>(time_next), static_cast<double>(time_exec),
            // static_cast<double>(time_minDe));
            return iteration_state::next_step;
        }
        if (time_allow == time_exec)
        {
            if (time_requested <= time_exec)
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
            if (dependencies.checkIfReadyForTimeGrant (false, time_exec))
            {
                time_granted = time_exec;
                if ((!dependents.empty ()) && (sendMessageFunction))
                {
                    ActionMessage treq (CMD_TIME_GRANT);
                    treq.source_id = source_id;
                    treq.actionTime = time_granted;
                    SET_ACTION_FLAG (treq, iterationRequested);
                    sendMessageFunction (treq);
                }
                return iteration_state::next_step;
            }
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
        if (time_allow == time_exec)  // time_allow==time_exec==time_granted
        {
            if (dependencies.checkIfReadyForTimeGrant (true, time_exec))
            {
                dependencies.resetIteratingTimeRequests (time_exec);
                if ((!dependents.empty ()) && (sendMessageFunction))
                {
                    ActionMessage treq (CMD_TIME_GRANT);
                    treq.source_id = source_id;
                    treq.actionTime = time_granted;
                    SET_ACTION_FLAG (treq, iterationRequested);
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
        upd.Te = time_exec;
        upd.Tdemin = time_minDe;
        if (iterating)
        {
            SET_ACTION_FLAG (upd, iterationRequested);
        }
        sendMessageFunction (upd);

        //	printf("%d next=%f, exec=%f, Tdemin=%f\n", source_id, static_cast<double>(time_next),
        // static_cast<double>(time_exec), static_cast<double>(time_minDe));
    }
    return iteration_state::continue_processing;
}

std::string TimeCoordinator::printTimeStatus () const
{
    return (boost::format ("exec=%f allow=%f, value=%f, message=%f, minDe=%f minminDe=%f") %
            static_cast<double> (time_exec) % static_cast<double> (time_allow) % static_cast<double> (time_value) %
            static_cast<double> (time_message) % static_cast<double> (time_minDe) %
            static_cast<double> (time_minminDe))
      .str ();
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

std::vector<Core::federate_id_t> TimeCoordinator::getDependencies () const
{
    std::vector<Core::federate_id_t> deps;
    for (auto &dep : dependencies)
    {
        deps.push_back (dep.fedID);
    }
    return deps;
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
            if (iteration > info.maxIterations)
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
            sendMessageFunction (execgrant);
        }
    }
    else if (ret == iteration_state::iterating)
    {
        dependencies.resetIteratingExecRequests ();
        hasInitUpdates = false;
        if (sendMessageFunction)
        {
            ActionMessage execgrant (CMD_EXEC_GRANT);
            execgrant.source_id = source_id;
            SET_ACTION_FLAG (execgrant, iterationRequested);
            sendMessageFunction (execgrant);
        }
    }
    return ret;
}

bool TimeCoordinator::processTimeMessage (const ActionMessage &cmd) { return dependencies.updateTime (cmd); }

void TimeCoordinator::processDependencyUpdateMessage (const ActionMessage &cmd)
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

void TimeCoordinator::processConfigUpdateMessage (const ActionMessage &cmd, bool initMode)
{
    switch (cmd.index)
    {
    case UPDATE_outputDelay:
        info.outputDelay = cmd.actionTime;
        break;
    case UPDATE_IMPACT_WINDOW:
        info.inputDelay = cmd.actionTime;
        break;
    case UPDATE_MINDELTA:
        info.timeDelta = cmd.actionTime;
        if (info.timeDelta <= timeZero)
        {
            info.timeDelta = timeEpsilon;
        }
        break;
    case UPDATE_PERIOD:
        info.period = cmd.actionTime;
        break;
    case UPDATE_OFFSET:
        info.offset = cmd.actionTime;
        break;
    case UPDATE_MAX_ITERATION:
        info.maxIterations = static_cast<int16_t> (cmd.dest_id);
        break;
    case UPDATE_LOG_LEVEL:
        info.logLevel = static_cast<int> (cmd.dest_id);
        break;
    case UPDATE_FLAG:
        switch (cmd.dest_id)
        {
        case UNINTERRUPTIBLE_FLAG:
            info.uninterruptible = CHECK_ACTION_FLAG (cmd, indicator_flag);
            break;
        case ONLY_TRANSMIT_ON_CHANGE_FLAG:
            info.only_transmit_on_change = CHECK_ACTION_FLAG (cmd, indicator_flag);
            break;
        case ONLY_UPDATE_ON_CHANGE_FLAG:
            info.only_update_on_change = CHECK_ACTION_FLAG (cmd, indicator_flag);
            break;
        case WAIT_FOR_CURRENT_TIME_UPDATE_FLAG:
            info.wait_for_current_time_updates = CHECK_ACTION_FLAG (cmd, indicator_flag);
            break;
        case SOURCE_ONLY_FLAG:
            if (initMode)
            {
                info.source_only = CHECK_ACTION_FLAG (cmd, indicator_flag);
            }
            break;
        case OBSERVER_FLAG:
            if (initMode)
            {
                info.observer = CHECK_ACTION_FLAG (cmd, indicator_flag);
            }
            break;
        default:
            break;
        }
    }
}

}  // namespace helics