/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "TimeCoordinator.hpp"
#include "../flag-definitions.h"
#include <algorithm>
#include <boost/format.hpp>

namespace helics
{
static auto nullMessageFunction = [](const ActionMessage &) {};
TimeCoordinator::TimeCoordinator () : sendMessageFunction (nullMessageFunction) {}
TimeCoordinator::TimeCoordinator (const CoreFederateInfo &info_) : TimeCoordinator (info_, nullMessageFunction) {}

TimeCoordinator::TimeCoordinator (const CoreFederateInfo &info_,
                                  std::function<void(const ActionMessage &)> sendMessageFunction_)
    : info (info_), sendMessageFunction (std::move (sendMessageFunction_))
{
    if (info.timeDelta <= timeZero)
    {
        info.timeDelta = timeEpsilon;
    }
    if (!sendMessageFunction)
    {
        sendMessageFunction = nullMessageFunction;
    }
}

void TimeCoordinator::setMessageSender (std::function<void(const ActionMessage &)> sendMessageFunction_)
{
    sendMessageFunction = std::move (sendMessageFunction_);
    if (!sendMessageFunction)
    {
        sendMessageFunction = nullMessageFunction;
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
    ActionMessage execreq (CMD_EXEC_REQUEST);
    execreq.source_id = source_id;
    if (iterating)
    {
        setActionFlag (execreq, iteration_requested_flag);
    }
    transmitTimingMessage (execreq);
}

void TimeCoordinator::disconnect ()
{
    ActionMessage bye (CMD_DISCONNECT);
    bye.source_id = source_id;
    transmitTimingMessage (bye);
}

void TimeCoordinator::timeRequest (Time nextTime,
                                   iteration_request iterate,
                                   Time newValueTime,
                                   Time newMessageTime)
{
    iterating = (iterate != iteration_request::no_iterations);
    if (iterating)
    {
        if (nextTime < time_granted)
        {
            nextTime = time_granted;
        }
    }
    else
    {
        if (nextTime < getNextPossibleTime ())
        {
            nextTime = getNextPossibleTime ();
        }
    }

    time_requested = nextTime;
    time_value = newValueTime;
    time_message = newMessageTime;
    updateTimeFactors ();

    if (!dependents.empty ())
    {
        sendTimeRequest ();
    }
}

bool TimeCoordinator::updateNextExecutionTime ()
{
    auto cexec = time_exec;
    time_exec = std::min (time_message, time_value);
    if (time_exec < Time::maxVal ())
    {
        time_exec += info.inputDelay;
    }
    time_exec = std::min (time_requested, time_exec);
    if (time_exec <= time_granted)
    {
        time_exec = (iterating) ? time_granted : getNextPossibleTime ();
    }
    if ((time_exec - time_granted) > 0.0)
    {
        time_exec = generateAllowedTime (time_exec);
    }
    return (time_exec != cexec);
}

void TimeCoordinator::updateNextPossibleEventTime ()
{
    if (!iterating)
    {
        time_next = getNextPossibleTime ();
    }
    else
    {
        time_next = time_granted;
    }
    if (time_minminDe < Time::maxVal ())
    {
        if (time_minminDe + info.inputDelay > time_next)
        {
            time_next = time_minminDe + info.inputDelay;
            time_next = generateAllowedTime (time_next);
        }
    }
    time_next = std::min (time_next, time_exec) + info.outputDelay;
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
        auto ptime = time_value;
        if (iterating)
        {
            time_value = (valueUpdateTime <= time_granted) ? time_granted : valueUpdateTime;
        }
        else
        {
            auto nextPossibleTime = getNextPossibleTime ();
            if (valueUpdateTime <= nextPossibleTime)
            {
                time_value = nextPossibleTime;
            }
            else
            {
                time_value = valueUpdateTime;
            }
        }
        if (time_value < ptime)
        {
            if (updateNextExecutionTime ())
            {
                sendTimeRequest ();
            }
        }
    }
}

bool TimeCoordinator::hasActiveTimeDependencies () const { return dependencies.hasActiveTimeDependencies (); }

Time TimeCoordinator::getNextPossibleTime () const
{
    if (time_granted == timeZero)
    {
        if (info.offset > info.timeDelta)
        {
            return info.offset;
        }
        else if (info.offset == timeZero)
        {
            return generateAllowedTime (std::max (info.timeDelta, info.period));
        }
        else if (info.period <= Time::epsilon ())
        {
            return info.timeDelta;
        }
        else
        {
            Time retTime = info.offset + info.period;
            while (retTime < info.timeDelta)
            {
                retTime += info.period;
            }
            return retTime;
        }
    }
    return generateAllowedTime (time_grantBase + std::max (info.timeDelta, info.period));
}

Time TimeCoordinator::generateAllowedTime (Time testTime) const
{
    if (info.period > timeEpsilon)
    {
        if (testTime == Time::maxVal ())
        {
            return testTime;
        }
        if (testTime - time_grantBase > info.period)
        {
            auto blk = static_cast<int> (std::ceil ((testTime - time_grantBase) / info.period));
            testTime = time_grantBase + blk * info.period;
        }
        else
        {
            testTime = time_grantBase + info.period;
        }
    }
    return testTime;
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
        auto ptime = time_message;
        if (iterating)
        {
            time_message = (messageUpdateTime <= time_granted) ? time_granted : messageUpdateTime;
        }
        else
        {
            auto nextPossibleTime = getNextPossibleTime ();
            if (messageUpdateTime <= nextPossibleTime)
            {
                time_message = nextPossibleTime;
            }
            else
            {
                time_message = messageUpdateTime;
            }
        }
        if (time_message < ptime)
        {
            if (updateNextExecutionTime ())
            {
                sendTimeRequest ();
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
    if (minDe < Time::maxVal ())
    {
        minDe = generateAllowedTime (minDe) + info.outputDelay;
    }
    if (minDe != time_minDe)
    {
        update = true;
        time_minDe = minDe;
    }
    if (minNext < Time::maxVal ())
    {
        time_allow = info.inputDelay + minNext;
    }
    else
    {
        time_allow = Time::maxVal ();
    }
    updateNextExecutionTime ();
    return update;
}

iteration_state TimeCoordinator::checkTimeGrant ()
{
    bool update = updateTimeFactors ();
    if (time_exec == Time::maxVal())
    {
        if (time_allow == Time::maxVal())
        {
            time_granted = Time::maxVal();
            time_grantBase = Time::maxVal();

            ActionMessage treq(CMD_DISCONNECT);
            treq.source_id = source_id;
            transmitTimingMessage(treq);
            return iteration_state::halted;
        }
    }
    if (time_block <= time_exec)
    {
        return iteration_state::continue_processing;
    }
    if ((!iterating) || (time_exec > time_granted))
    {
        iteration = 0;
        if (time_allow > time_exec)
        {
            updateTimeGrant ();
            return iteration_state::next_step;
        }
        if (time_allow == time_exec)
        {
            if (time_requested <= time_exec)
            {
                updateTimeGrant ();
                return iteration_state::next_step;
            }
            if (dependencies.checkIfReadyForTimeGrant (false, time_exec))
            {
                updateTimeGrant ();
                return iteration_state::next_step;
            }
        }
    }
    else
    {
        if (time_allow > time_exec)
        {
            ++iteration;
            updateTimeGrant ();
            return iteration_state::iterating;
        }
        if (time_allow == time_exec)  // time_allow==time_exec==time_granted
        {
            if (dependencies.checkIfReadyForTimeGrant (true, time_exec))
            {
                ++iteration;
                updateTimeGrant ();
                return iteration_state::iterating;
            }
        }
    }

    // if we haven't returned we may need to update the time messages
    if ((!dependents.empty ()) && (update))
    {
        sendTimeRequest ();
    }
    return iteration_state::continue_processing;
}

void TimeCoordinator::sendTimeRequest () const
{
    ActionMessage upd (CMD_TIME_REQUEST);
    upd.source_id = source_id;
    upd.actionTime = time_next;
    upd.Te = (time_exec != Time::maxVal ()) ? time_exec + info.outputDelay : time_exec;
    upd.Tdemin = (time_minDe < time_next) ? time_next : time_minDe;

    if (iterating)
    {
        setActionFlag (upd, iteration_requested_flag);
        upd.counter = iteration;
    }
    transmitTimingMessage (upd);
    //	printf("%d next=%f, exec=%f, Tdemin=%f\n", source_id, static_cast<double>(time_next),
    // static_cast<double>(time_exec), static_cast<double>(time_minDe));
}

void TimeCoordinator::updateTimeGrant ()
{
    time_granted = time_exec;
    time_grantBase = time_granted;

    ActionMessage treq (CMD_TIME_GRANT);
    treq.source_id = source_id;
    treq.actionTime = time_granted;
    treq.counter = iteration;
    if (iterating)
    {
        dependencies.resetIteratingTimeRequests (time_exec);
    }
    transmitTimingMessage (treq);
    // printf("%d GRANT allow=%f next=%f, exec=%f, Tdemin=%f\n", source_id,
    // static_cast<double>(time_allow), static_cast<double>(time_next), static_cast<double>(time_exec),
    // static_cast<double>(time_minDe));
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
    deps.reserve (dependencies.size ());
    for (auto &dep : dependencies)
    {
        deps.push_back (dep.fedID);
    }
    return deps;
}

void TimeCoordinator::transmitTimingMessage (ActionMessage &msg) const
{
    for (auto dep : dependents)
    {
        msg.dest_id = dep;
        sendMessageFunction (msg);
    }
}

iteration_state TimeCoordinator::checkExecEntry ()
{
    auto ret = iteration_state::continue_processing;
    if (time_block <= timeZero)
    {
        return ret;
    }
    if (!dependencies.checkIfReadyForExecEntry (iterating))
    {
        return ret;
    }
    if (iterating)
    {
        if (hasInitUpdates)
        {
            if (iteration >= info.maxIterations)
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
        time_grantBase = time_granted;
        executionMode = true;
        iteration = 0;

        ActionMessage execgrant (CMD_EXEC_GRANT);
        execgrant.source_id = source_id;
        transmitTimingMessage (execgrant);
    }
    else if (ret == iteration_state::iterating)
    {
        dependencies.resetIteratingExecRequests ();
        hasInitUpdates = false;
        ++iteration;
        ActionMessage execgrant (CMD_EXEC_GRANT);
        execgrant.source_id = source_id;
        execgrant.counter = iteration;
        setActionFlag (execgrant, iteration_requested_flag);
        transmitTimingMessage (execgrant);
    }
    return ret;
}

static bool isDelayableMessage (const ActionMessage &cmd, Core::federate_id_t localId)
{
    return (((cmd.action () == CMD_TIME_GRANT) || (cmd.action () == CMD_EXEC_GRANT)) &&
            (cmd.source_id != localId));
}

message_process_result TimeCoordinator::processTimeMessage (const ActionMessage &cmd)
{
    if ((cmd.action () == CMD_TIME_BLOCK) || (cmd.action () == CMD_TIME_UNBLOCK))
    {
        return processTimeBlockMessage (cmd);
    }
    if (isDelayableMessage (cmd, source_id))
    {
        auto dep = dependencies.getDependencyInfo (cmd.source_id);
        if (dep == nullptr)
        {
            return message_process_result::no_effect;
        }
        switch (dep->time_state)
        {
        case DependencyInfo::time_state_t::time_requested:
            if (dep->Tnext > time_exec)
            {
                return message_process_result::delay_processing;
            }
            break;
        case DependencyInfo::time_state_t::time_requested_iterative:
            if (dep->Tnext > time_exec)
            {
                return message_process_result::delay_processing;
            }
            if ((iterating) && (time_exec == dep->Tnext))
            {
                return message_process_result::delay_processing;
            }
            break;
        case DependencyInfo::time_state_t::exec_requested_iterative:
            if ((iterating) && (checkingExec))
            {
                return message_process_result::delay_processing;
            }
            break;
        default:
            break;
        }
    }
    return (dependencies.updateTime (cmd)) ? message_process_result::processed : message_process_result::no_effect;
}

message_process_result TimeCoordinator::processTimeBlockMessage (const ActionMessage &cmd)
{
    if (cmd.action () == CMD_TIME_BLOCK)
    {
        timeBlocks.emplace_back (cmd.actionTime, cmd.index);
        if (cmd.actionTime < time_block)
        {
            time_block = cmd.actionTime;
        }
    }
    else if (cmd.action () == CMD_TIME_UNBLOCK)
    {
        if (!timeBlocks.empty ())
        {
            auto ltime = Time::maxVal ();
            if (timeBlocks.front ().second == cmd.index)
            {
                ltime = timeBlocks.front ().first;
                timeBlocks.pop_front ();
            }
            else
            {
                auto blk = std::find_if (timeBlocks.begin (), timeBlocks.end (),
                                         [&cmd](const auto &block) { return (block.second == cmd.index); });
                if (blk != timeBlocks.end ())
                {
                    ltime = blk->first;
                    timeBlocks.erase (blk);
                }
            }
            if (ltime <= time_block)
            {
                if (!timeBlocks.empty ())
                {
                    auto res = std::min_element (timeBlocks.begin (), timeBlocks.end (),
                                                 [](const auto &blk1, const auto &blk2) {
                                                     return (blk1.first < blk2.first);
                                                 });
                    time_block = res->first;
                }
                else
                {
                    time_block = Time::maxVal ();
                }
                return message_process_result::processed;
            }
        }
    }
    return message_process_result::no_effect;
}

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
    case UPDATE_OUTPUT_DELAY:
        info.outputDelay = cmd.actionTime;
        break;
    case UPDATE_INPUT_DELAY:
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
            info.uninterruptible = checkActionFlag (cmd, indicator_flag);
            break;
        case ONLY_TRANSMIT_ON_CHANGE_FLAG:
            info.only_transmit_on_change = checkActionFlag (cmd, indicator_flag);
            break;
        case ONLY_UPDATE_ON_CHANGE_FLAG:
            info.only_update_on_change = checkActionFlag (cmd, indicator_flag);
            break;
        case WAIT_FOR_CURRENT_TIME_UPDATE_FLAG:
            info.wait_for_current_time_updates = checkActionFlag (cmd, indicator_flag);
            break;
        case SOURCE_ONLY_FLAG:
            if (initMode)
            {
                info.source_only = checkActionFlag (cmd, indicator_flag);
            }
            break;
        case OBSERVER_FLAG:
            if (initMode)
            {
                info.observer = checkActionFlag (cmd, indicator_flag);
            }
            break;
        default:
            break;
        }
    }
}

}  // namespace helics
