/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "TimeCoordinator.hpp"

#include "../common/fmt_format.h"
#include "flagOperations.hpp"
#include "helics_definitions.hpp"

#include <algorithm>
#include <set>
#include <sstream>

namespace helics {
static auto nullMessageFunction = [](const ActionMessage&) {};
TimeCoordinator::TimeCoordinator(): sendMessageFunction(nullMessageFunction) {}

TimeCoordinator::TimeCoordinator(std::function<void(const ActionMessage&)> sendMessageFunction_):
    sendMessageFunction(std::move(sendMessageFunction_))
{
    if (!sendMessageFunction) {
        sendMessageFunction = nullMessageFunction;
    }
}

void TimeCoordinator::setMessageSender(
    std::function<void(const ActionMessage&)> sendMessageFunction_)
{
    sendMessageFunction = std::move(sendMessageFunction_);
    if (!sendMessageFunction) {
        sendMessageFunction = nullMessageFunction;
    }
}

void TimeCoordinator::enteringExecMode(iteration_request mode)
{
    if (executionMode) {
        return;
    }
    iterating = mode;
    checkingExec = true;
    ActionMessage execreq(CMD_EXEC_REQUEST);
    execreq.source_id = source_id;
    if (iterating != iteration_request::no_iterations) {
        setIterationFlags(execreq, iterating);
    }
    transmitTimingMessage(execreq);
}

void TimeCoordinator::disconnect()
{
    time_granted = Time::maxVal();
    time_grantBase = Time::maxVal();
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
    disconnected = true;
}

void TimeCoordinator::timeRequest(
    Time nextTime,
    iteration_request iterate,
    Time newValueTime,
    Time newMessageTime)
{
    iterating = iterate;

    if (iterating != iteration_request::no_iterations) {
        if (nextTime < time_granted || iterating == iteration_request::force_iteration) {
            nextTime = time_granted;
        }
    } else {
        time_next = getNextPossibleTime();
        if (nextTime < time_next) {
            nextTime = time_next;
        }
        if (info.uninterruptible) {
            time_next = nextTime;
        }
    }
    time_requested = nextTime;
    time_value = (newValueTime > time_next) ? newValueTime : time_next;
    time_message = (newMessageTime > time_next) ? newMessageTime : time_next;
    time_exec = std::min({time_value, time_message, time_requested});
    if (info.uninterruptible) {
        time_exec = time_requested;
    }
    dependencies.resetDependentEvents(time_granted);
    updateTimeFactors();

    if (!dependents.empty()) {
        sendTimeRequest();
    }
}

bool TimeCoordinator::updateNextExecutionTime()
{
    auto cexec = time_exec;
    if (info.uninterruptible) {
        time_exec = time_requested;
    } else {
        time_exec = std::min(time_message, time_value);
        if (time_exec < Time::maxVal()) {
            time_exec += info.inputDelay;
        }
        time_exec = std::min(time_requested, time_exec);
        if (time_exec <= time_granted) {
            time_exec = (iterating == iteration_request::no_iterations) ? getNextPossibleTime() :
                                                                          time_granted;
        }
        if ((time_exec - time_granted) > timeZero) {
            time_exec = generateAllowedTime(time_exec);
        }
    }

    return (time_exec != cexec);
}

void TimeCoordinator::updateNextPossibleEventTime()
{
    time_next =
        (iterating == iteration_request::no_iterations) ? getNextPossibleTime() : time_granted;

    if (info.uninterruptible) {
        time_next = time_requested;
    } else {
        if (time_minminDe < Time::maxVal() && !info.restrictive_time_policy) {
            if (time_minminDe + info.inputDelay > time_next) {
                time_next = time_minminDe + info.inputDelay;
                time_next = generateAllowedTime(time_next);
            }
        }
        time_next = std::min(time_next, time_exec) + info.outputDelay;
    }
}

void TimeCoordinator::updateValueTime(Time valueUpdateTime)
{
    if (!executionMode) // updates before exec mode
    {
        if (valueUpdateTime < timeZero) {
            hasInitUpdates = true;
        }
        return;
    }
    if (valueUpdateTime < time_value) {
        auto ptime = time_value;
        if (iterating != iteration_request::no_iterations) {
            time_value = (valueUpdateTime <= time_granted) ? time_granted : valueUpdateTime;
        } else {
            auto nextPossibleTime = getNextPossibleTime();
            if (valueUpdateTime <= nextPossibleTime) {
                time_value = nextPossibleTime;
            } else {
                time_value = valueUpdateTime;
            }
        }
        if (time_value < ptime && !disconnected) {
            if (updateNextExecutionTime()) {
                sendTimeRequest();
            }
        }
    }
}

std::string TimeCoordinator::generateConfig() const
{
    std::stringstream s;
    s << "\"uninterruptible\":" << ((info.uninterruptible) ? " true,\n" : "false,\n");
    s << "\"wait_for_current_time_updates\":"
      << ((info.wait_for_current_time_updates) ? " true,\n" : "false,\n");
    if (info.restrictive_time_policy) {
        s << "\"restrictive_time_policy\":true,\n";
    }
    s << "\"max_iterations\":" << info.maxIterations;
    if (info.period > timeZero) {
        s << ",\n\"period\":" << static_cast<double>(info.period);
    }
    if (info.offset != timeZero) {
        s << ",\n\"offset\":" << static_cast<double>(info.offset);
    }
    if (info.timeDelta > Time::epsilon()) {
        s << ",\n\"time_delta\":" << static_cast<double>(info.timeDelta);
    }
    if (info.outputDelay > timeZero) {
        s << ",\n\"output_delay\":" << static_cast<double>(info.outputDelay);
    }
    if (info.inputDelay > timeZero) {
        s << ",\n\"intput_delay\":" << static_cast<double>(info.inputDelay);
    }
    return s.str();
}
bool TimeCoordinator::hasActiveTimeDependencies() const
{
    return dependencies.hasActiveTimeDependencies();
}

Time TimeCoordinator::getNextPossibleTime() const
{
    if (time_granted == timeZero) {
        if (info.offset > info.timeDelta) {
            return info.offset;
        } else if (info.offset == timeZero) {
            return generateAllowedTime(std::max(info.timeDelta, info.period));
        } else if (info.period <= Time::epsilon()) {
            return info.timeDelta;
        } else {
            Time retTime = info.offset + info.period;
            while (retTime < info.timeDelta) {
                retTime += info.period;
            }
            return retTime;
        }
    }
    return generateAllowedTime(time_grantBase + std::max(info.timeDelta, info.period));
}

Time TimeCoordinator::generateAllowedTime(Time testTime) const
{
    if (info.period > timeEpsilon) {
        if (testTime == Time::maxVal()) {
            return testTime;
        }
        if (testTime - time_grantBase > info.period) {
            auto blk = std::ceil((testTime - time_grantBase) / info.period);
            testTime = time_grantBase + blk * info.period;
        } else {
            testTime = time_grantBase + info.period;
        }
    }
    return testTime;
}

void TimeCoordinator::updateMessageTime(Time messageUpdateTime)
{
    if (!executionMode) // updates before exec mode
    {
        if (messageUpdateTime < timeZero) {
            hasInitUpdates = true;
        }
        return;
    }

    if (messageUpdateTime < time_message) {
        auto ptime = time_message;
        if (iterating != iteration_request::no_iterations) {
            time_message = (messageUpdateTime <= time_granted) ? time_granted : messageUpdateTime;
        } else {
            auto nextPossibleTime = getNextPossibleTime();
            if (messageUpdateTime <= nextPossibleTime) {
                time_message = nextPossibleTime;
            } else {
                time_message = messageUpdateTime;
            }
        }
        if (time_message < ptime && !disconnected) {
            if (updateNextExecutionTime()) {
                sendTimeRequest();
            }
        }
    }
}

bool TimeCoordinator::updateTimeFactors()
{
    Time minNext = Time::maxVal();
    Time minminDe = std::min(time_value, time_message);
    Time minDe = minminDe;
    for (auto& dep : dependencies) {
        if (dep.Tnext < minNext) {
            minNext = dep.Tnext;
        }
        if (dep.Tdemin >= dep.Tnext) {
            if (dep.Tdemin < minminDe) {
                minminDe = dep.Tdemin;
            }
        } else {
            // this minimum dependent event time received was invalid and can't be trusted
            // therefore it can't be used to determine a time grant
            minminDe = -1;
        }

        if (dep.Te < minDe) {
            minDe = dep.Te;
        }
    }

    bool update = false;
    time_minminDe = std::min(minDe, minminDe);
    Time prev_next = time_next;
    updateNextPossibleEventTime();

    //	printf("%d UDPATE next=%f, minminDE=%f, Tdemin=%f\n", source_id, static_cast<double>(time_next),
    // static_cast<double>(minminDe), static_cast<double>(minDe));
    if (prev_next != time_next) {
        update = true;
    }
    if (minDe < Time::maxVal()) {
        minDe = generateAllowedTime(minDe) + info.outputDelay;
    }
    if (minDe != time_minDe) {
        update = true;
        time_minDe = minDe;
    }
    if (minNext < Time::maxVal()) {
        time_allow = info.inputDelay + minNext;
    } else {
        time_allow = Time::maxVal();
    }
    updateNextExecutionTime();
    return update;
}

message_processing_result TimeCoordinator::checkTimeGrant()
{
    bool update = updateTimeFactors();
    if (time_exec == Time::maxVal()) {
        if (time_allow == Time::maxVal()) {
            time_granted = Time::maxVal();
            time_grantBase = Time::maxVal();
            disconnect();
            return message_processing_result::halted;
        }
    }
    if (time_block <= time_exec) {
        return message_processing_result::continue_processing;
    }
    if ((iterating == iteration_request::no_iterations) ||
        (time_exec > time_granted && iterating == iteration_request::iterate_if_needed)) {
        iteration = 0;
        if (time_allow > time_exec) {
            updateTimeGrant();
            return message_processing_result::next_step;
        }
        if (time_allow == time_exec) {
            if (time_requested <= time_exec) {
                updateTimeGrant();
                return message_processing_result::next_step;
            }
            if (dependencies.checkIfReadyForTimeGrant(false, time_exec)) {
                updateTimeGrant();
                return message_processing_result::next_step;
            }
        }
    } else {
        if (time_allow > time_exec) {
            ++iteration;
            updateTimeGrant();
            return message_processing_result::iterating;
        }
        if (time_allow == time_exec) // time_allow==time_exec==time_granted
        {
            if (dependencies.checkIfReadyForTimeGrant(true, time_exec)) {
                ++iteration;
                updateTimeGrant();
                return message_processing_result::iterating;
            }
        }
    }

    // if we haven't returned we may need to update the time messages
    if ((!dependents.empty()) && (update)) {
        sendTimeRequest();
    }
    return message_processing_result::continue_processing;
}

void TimeCoordinator::sendTimeRequest() const
{
    ActionMessage upd(CMD_TIME_REQUEST);
    upd.source_id = source_id;
    upd.actionTime = time_next;
    upd.Te = (time_exec != Time::maxVal()) ? time_exec + info.outputDelay : time_exec;
    upd.Tdemin = (time_minDe < time_next) ? time_next : time_minDe;

    if (iterating != iteration_request::no_iterations) {
        setIterationFlags(upd, iterating);
        upd.counter = iteration;
    }
    transmitTimingMessage(upd);
    //	printf("%d next=%f, exec=%f, Tdemin=%f\n", source_id, static_cast<double>(time_next),
    // static_cast<double>(time_exec), static_cast<double>(time_minDe));
}

void TimeCoordinator::updateTimeGrant()
{
    if (iterating != iteration_request::force_iteration) {
        time_granted = time_exec;
        time_grantBase = time_granted;
    }
    ActionMessage treq(CMD_TIME_GRANT);
    treq.source_id = source_id;
    treq.actionTime = time_granted;
    treq.counter = iteration;
    if (iterating != iteration_request::no_iterations) {
        dependencies.resetIteratingTimeRequests(time_exec);
    }
    transmitTimingMessage(treq);
    // printf("%d GRANT allow=%f next=%f, exec=%f, Tdemin=%f\n", source_id,
    // static_cast<double>(time_allow), static_cast<double>(time_next), static_cast<double>(time_exec),
    // static_cast<double>(time_minDe));
}
std::string TimeCoordinator::printTimeStatus() const
{
    return fmt::format(
        "exec={} allow={}, value={}, message={}, minDe={} minminDe={}",
        static_cast<double>(time_exec),
        static_cast<double>(time_allow),
        static_cast<double>(time_value),
        static_cast<double>(time_message),
        static_cast<double>(time_minDe),
        static_cast<double>(time_minminDe));
}

bool TimeCoordinator::isDependency(global_federate_id ofed) const
{
    return dependencies.isDependency(ofed);
}

bool TimeCoordinator::addDependency(global_federate_id fedID)
{
    if (dependencies.addDependency(fedID)) {
        dependency_federates.lock()->push_back(fedID);
        return true;
    }
    return false;
}

bool TimeCoordinator::addDependent(global_federate_id fedID)
{
    if (dependents.empty()) {
        dependents.push_back(fedID);
        dependent_federates.lock()->push_back(fedID);
        return true;
    }
    auto dep = std::lower_bound(dependents.begin(), dependents.end(), fedID);
    if (dep == dependents.end()) {
        dependents.push_back(fedID);
        dependent_federates.lock()->push_back(fedID);
    } else {
        if (*dep == fedID) {
            return false;
        }
        dependents.insert(dep, fedID);
        dependent_federates.lock()->push_back(fedID);
    }
    return true;
}

void TimeCoordinator::removeDependency(global_federate_id fedID)
{
    dependencies.removeDependency(fedID);
    // remove the thread safe version
    auto dlock = dependency_federates.lock();
    auto res = std::find(dlock.begin(), dlock.end(), fedID);
    if (res != dlock.end()) {
        dlock->erase(res);
    }
}

void TimeCoordinator::removeDependent(global_federate_id fedID)
{
    auto dep = std::lower_bound(dependents.begin(), dependents.end(), fedID);
    if (dep != dependents.end()) {
        if (*dep == fedID) {
            dependents.erase(dep);
            // remove the thread safe version
            auto dlock = dependent_federates.lock();
            auto res = std::find(dlock.begin(), dlock.end(), fedID);
            if (res != dlock.end()) {
                dlock->erase(res);
            }
        }
    }
}

DependencyInfo* TimeCoordinator::getDependencyInfo(global_federate_id ofed)
{
    return dependencies.getDependencyInfo(ofed);
}

std::vector<global_federate_id> TimeCoordinator::getDependencies() const
{
    return *dependency_federates.lock_shared();
}

void TimeCoordinator::transmitTimingMessage(ActionMessage& msg) const
{
    for (auto dep : dependents) {
        msg.dest_id = dep;
        sendMessageFunction(msg);
    }
}

message_processing_result TimeCoordinator::checkExecEntry()
{
    auto ret = message_processing_result::continue_processing;
    if (time_block <= timeZero) {
        return ret;
    }
    if (!dependencies.checkIfReadyForExecEntry(iterating != iteration_request::no_iterations)) {
        return ret;
    }
    switch (iterating) {
        case iteration_request::no_iterations:
            ret = message_processing_result::next_step;
            break;
        case iteration_request::iterate_if_needed:
            if (hasInitUpdates) {
                if (iteration >= info.maxIterations) {
                    ret = message_processing_result::next_step;
                } else {
                    ret = message_processing_result::iterating;
                }
            } else {
                ret = message_processing_result::next_step;
            }
            break;
        case iteration_request::force_iteration:
            ret = message_processing_result::iterating;
            break;
    }

    if (ret == message_processing_result::next_step) {
        time_granted = timeZero;
        time_grantBase = time_granted;
        executionMode = true;
        iteration = 0;

        ActionMessage execgrant(CMD_EXEC_GRANT);
        execgrant.source_id = source_id;
        transmitTimingMessage(execgrant);
    } else if (ret == message_processing_result::iterating) {
        dependencies.resetIteratingExecRequests();
        hasInitUpdates = false;
        ++iteration;
        ActionMessage execgrant(CMD_EXEC_GRANT);
        execgrant.source_id = source_id;
        execgrant.counter = iteration;
        setActionFlag(execgrant, iteration_requested_flag);
        transmitTimingMessage(execgrant);
    }
    return ret;
}

static bool isDelayableMessage(const ActionMessage& cmd, global_federate_id localId)
{
    return (
        ((cmd.action() == CMD_TIME_GRANT) || (cmd.action() == CMD_EXEC_GRANT)) &&
        (cmd.source_id != localId));
}

message_process_result TimeCoordinator::processTimeMessage(const ActionMessage& cmd)
{
    switch (cmd.action()) {
        case CMD_TIME_BLOCK:
        case CMD_TIME_UNBLOCK:
            return processTimeBlockMessage(cmd);
        case CMD_FORCE_TIME_GRANT:
            if (time_granted < cmd.actionTime) {
                time_granted = cmd.actionTime;
                time_grantBase = time_granted;

                ActionMessage treq(CMD_TIME_GRANT);
                treq.source_id = source_id;
                treq.actionTime = time_granted;
                transmitTimingMessage(treq);
                return message_process_result::processed;
            }
            return message_process_result::no_effect;
        case CMD_DISCONNECT:
        case CMD_BROADCAST_DISCONNECT:
        case CMD_DISCONNECT_CORE:
        case CMD_DISCONNECT_FED:
        case CMD_DISCONNECT_BROKER:
        case CMD_GLOBAL_ERROR:
        case CMD_LOCAL_ERROR:
            // this command requires removing dependents as well as dealing with dependency processing
            removeDependent(global_federate_id(cmd.source_id));
            break;

        default:
            break;
    }
    if (isDelayableMessage(cmd, source_id)) {
        auto dep = dependencies.getDependencyInfo(global_federate_id(cmd.source_id));
        if (dep == nullptr) {
            return message_process_result::no_effect;
        }
        switch (dep->time_state) {
            case DependencyInfo::time_state_t::time_requested:
                if (dep->Tnext > time_exec) {
                    return message_process_result::delay_processing;
                }
                break;
            case DependencyInfo::time_state_t::time_requested_iterative:
                if (dep->Tnext > time_exec) {
                    return message_process_result::delay_processing;
                }
                if ((iterating != iteration_request::no_iterations) && (time_exec == dep->Tnext)) {
                    return message_process_result::delay_processing;
                }
                break;
            case DependencyInfo::time_state_t::exec_requested_iterative:
                if ((iterating != iteration_request::no_iterations) && (checkingExec)) {
                    return message_process_result::delay_processing;
                }
                break;
            default:
                break;
        }
    }
    return (dependencies.updateTime(cmd)) ? message_process_result::processed :
                                            message_process_result::no_effect;
}

message_process_result TimeCoordinator::processTimeBlockMessage(const ActionMessage& cmd)
{
    if (cmd.action() == CMD_TIME_BLOCK) {
        timeBlocks.emplace_back(cmd.actionTime, cmd.messageID);
        if (cmd.actionTime < time_block) {
            time_block = cmd.actionTime;
        }
    } else if (cmd.action() == CMD_TIME_UNBLOCK) {
        if (!timeBlocks.empty()) {
            auto ltime = Time::maxVal();
            if (timeBlocks.front().second == cmd.messageID) {
                ltime = timeBlocks.front().first;
                timeBlocks.pop_front();
            } else {
                auto blk =
                    std::find_if(timeBlocks.begin(), timeBlocks.end(), [&cmd](const auto& block) {
                        return (block.second == cmd.messageID);
                    });
                if (blk != timeBlocks.end()) {
                    ltime = blk->first;
                    timeBlocks.erase(blk);
                }
            }
            if (ltime <= time_block) {
                if (!timeBlocks.empty()) {
                    auto res = std::min_element(
                        timeBlocks.begin(),
                        timeBlocks.end(),
                        [](const auto& blk1, const auto& blk2) {
                            return (blk1.first < blk2.first);
                        });
                    time_block = res->first;
                } else {
                    time_block = Time::maxVal();
                }
                return message_process_result::processed;
            }
        }
    }
    return message_process_result::no_effect;
}

void TimeCoordinator::processDependencyUpdateMessage(const ActionMessage& cmd)
{
    switch (cmd.action()) {
        case CMD_ADD_DEPENDENCY:
            addDependency(global_federate_id(cmd.source_id));
            break;
        case CMD_REMOVE_DEPENDENCY:
            removeDependency(global_federate_id(cmd.source_id));
            break;
        case CMD_ADD_DEPENDENT:
            addDependent(global_federate_id(cmd.source_id));
            break;
        case CMD_REMOVE_DEPENDENT:
            removeDependent(global_federate_id(cmd.source_id));
            break;
        case CMD_ADD_INTERDEPENDENCY:
            addDependency(global_federate_id(cmd.source_id));
            addDependent(global_federate_id(cmd.source_id));
            break;
        case CMD_REMOVE_INTERDEPENDENCY:
            removeDependency(global_federate_id(cmd.source_id));
            removeDependent(global_federate_id(cmd.source_id));
            break;
        default:
            break;
    }
}

/** set a timeProperty for a the coordinator*/
void TimeCoordinator::setProperty(int timeProperty, Time propertyVal)
{
    switch (timeProperty) {
        case defs::properties::output_delay:
            info.outputDelay = propertyVal;
            break;
        case defs::properties::input_delay:
            info.inputDelay = propertyVal;
            break;
        case defs::properties::time_delta:
            info.timeDelta = propertyVal;
            if (info.timeDelta <= timeZero) {
                info.timeDelta = timeEpsilon;
            }
            break;
        case defs::properties::period:
            info.period = propertyVal;
            break;
        case defs::properties::offset:
            info.offset = propertyVal;
            break;
    }
}

/** set a timeProperty for a the coordinator*/
void TimeCoordinator::setProperty(int intProperty, int propertyVal)
{
    if (intProperty == defs::properties::max_iterations) {
        info.maxIterations = propertyVal;
    } else {
        setProperty(intProperty, Time(static_cast<double>(propertyVal)));
    }
}

/** set an option Flag for a the coordinator*/
void TimeCoordinator::setOptionFlag(int optionFlag, bool value)
{
    switch (optionFlag) {
        case defs::flags::uninterruptible:
            info.uninterruptible = value;
            break;
        case defs::flags::wait_for_current_time_update:
            info.wait_for_current_time_updates = value;
            break;
        case defs::flags::restrictive_time_policy:
            info.restrictive_time_policy = value;
            break;
        default:
            break;
    }
}
/** get a time Property*/
Time TimeCoordinator::getTimeProperty(int timeProperty) const
{
    switch (timeProperty) {
        case defs::properties::output_delay:
            return info.outputDelay;
        case defs::properties::input_delay:
            return info.inputDelay;
        case defs::properties::time_delta:
            return info.timeDelta;
        case defs::properties::period:
            return info.period;
        case defs::properties::offset:
            return info.offset;
        default:
            return Time::minVal();
    }
}

/** get a time Property*/
int TimeCoordinator::getIntegerProperty(int intProperty) const
{
    switch (intProperty) {
        case defs::properties::max_iterations:
            return info.maxIterations;
        default:
            // TODO: make this something consistent
            return -972;
    }
}

/** get an option flag value*/
bool TimeCoordinator::getOptionFlag(int optionFlag) const
{
    switch (optionFlag) {
        case defs::flags::uninterruptible:
            return info.uninterruptible;
        case defs::flags::interruptible:
            return !info.uninterruptible;
        case defs::flags::wait_for_current_time_update:
            return info.wait_for_current_time_updates;
        case defs::flags::restrictive_time_policy:
            return info.restrictive_time_policy;
        default:
            throw(std::invalid_argument("flag not recognized"));
    }
}

void TimeCoordinator::processConfigUpdateMessage(const ActionMessage& cmd)
{
    switch (cmd.action()) {
        case CMD_FED_CONFIGURE_TIME:
            setProperty(cmd.messageID, cmd.actionTime);
            break;
        case CMD_FED_CONFIGURE_INT:
            setProperty(cmd.messageID, cmd.counter);
            break;
        case CMD_FED_CONFIGURE_FLAG:
            setOptionFlag(cmd.messageID, checkActionFlag(cmd, indicator_flag));
            break;
        default:
            break;
    }
}

} // namespace helics
