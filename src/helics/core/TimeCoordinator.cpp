/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "TimeCoordinator.hpp"

#include "../common/fmt_format.h"
#include "flagOperations.hpp"
#include "helics_definitions.hpp"

#include "json/json.h"
#include <algorithm>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace helics {
static auto nullMessageFunction = [](const ActionMessage& /*unused*/) {};
TimeCoordinator::TimeCoordinator(): sendMessageFunction(nullMessageFunction) {}

TimeCoordinator::TimeCoordinator(std::function<void(const ActionMessage&)> userSendMessageFunction):
    sendMessageFunction(std::move(userSendMessageFunction))
{
    if (!sendMessageFunction) {
        sendMessageFunction = nullMessageFunction;
    }
}

void TimeCoordinator::setMessageSender(
    std::function<void(const ActionMessage&)> userSendMessageFunction)
{
    sendMessageFunction = std::move(userSendMessageFunction);
    if (!sendMessageFunction) {
        sendMessageFunction = nullMessageFunction;
    }
}

void TimeCoordinator::enteringExecMode(IterationRequest mode)
{
    if (executionMode) {
        return;
    }
    iterating = mode;
    checkingExec = true;
    ActionMessage execreq(CMD_EXEC_REQUEST);
    execreq.source_id = source_id;
    if (iterating != IterationRequest::NO_ITERATIONS) {
        setIterationFlags(execreq, iterating);
    }
    transmitTimingMessage(execreq);
}

void TimeCoordinator::disconnect()
{
    time_granted = Time::maxVal();
    time_grantBase = Time::maxVal();
    if (sendMessageFunction) {
        std::set<GlobalFederateId> connections(dependents.begin(), dependents.end());
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

void TimeCoordinator::localError()
{
    if (disconnected) {
        return;
    }
    time_granted = Time::maxVal();
    time_grantBase = Time::maxVal();
    if (sendMessageFunction) {
        std::set<GlobalFederateId> connections(dependents.begin(), dependents.end());
        for (auto dep : dependencies) {
            if (dep.Tnext < Time::maxVal()) {
                connections.insert(dep.fedID);
            }
        }
        if (connections.empty()) {
            return;
        }
        ActionMessage bye(CMD_LOCAL_ERROR);

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

void TimeCoordinator::timeRequest(Time nextTime,
                                  IterationRequest iterate,
                                  Time newValueTime,
                                  Time newMessageTime)
{
    iterating = iterate;

    if (iterating != IterationRequest::NO_ITERATIONS) {
        if (nextTime < time_granted || iterating == IterationRequest::FORCE_ITERATION) {
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
            time_exec = (iterating == IterationRequest::NO_ITERATIONS) ? getNextPossibleTime() :
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
        (iterating == IterationRequest::NO_ITERATIONS) ? getNextPossibleTime() : time_granted;

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
    if (!executionMode)  // updates before exec mode
    {
        if (valueUpdateTime < timeZero) {
            hasInitUpdates = true;
        }
        return;
    }
    if (valueUpdateTime < time_value) {
        auto ptime = time_value;
        if (iterating != IterationRequest::NO_ITERATIONS) {
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

void TimeCoordinator::generateConfig(Json::Value& base) const
{
    base["uninterruptible"] = info.uninterruptible;
    base["wait_for_current_time_updates"] = info.wait_for_current_time_updates;
    base["restrictive_time_policy"] = info.restrictive_time_policy;
    base["max_iterations"] = info.maxIterations;

    if (info.period > timeZero) {
        base["period"] = static_cast<double>(info.period);
    }
    if (info.offset != timeZero) {
        base["offset"] = static_cast<double>(info.offset);
    }
    if (info.timeDelta > Time::epsilon()) {
        base["time_delta"] = static_cast<double>(info.timeDelta);
    }
    if (info.outputDelay > timeZero) {
        base["output_delay"] = static_cast<double>(info.outputDelay);
    }
    if (info.inputDelay > timeZero) {
        base["intput_delay"] = static_cast<double>(info.inputDelay);
    }
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
        }
        if (info.offset == timeZero) {
            return generateAllowedTime(std::max(info.timeDelta, info.period));
        }
        if (info.period <= Time::epsilon()) {
            return info.timeDelta;
        }
        Time retTime = info.offset + info.period;
        while (retTime < info.timeDelta) {
            retTime += info.period;
        }
        return retTime;
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
    if (!executionMode)  // updates before exec mode
    {
        if (messageUpdateTime < timeZero) {
            hasInitUpdates = true;
        }
        return;
    }

    if (messageUpdateTime < time_message) {
        auto ptime = time_message;
        if (iterating != IterationRequest::NO_ITERATIONS) {
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

    //    printf("%d UPDATE next=%f, minminDE=%f, Tdemin=%f\n", source_id,
    //    static_cast<double>(time_next),
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

MessageProcessingResult TimeCoordinator::checkTimeGrant()
{
    bool update = updateTimeFactors();
    if (time_exec == Time::maxVal()) {
        if (time_allow == Time::maxVal()) {
            time_granted = Time::maxVal();
            time_grantBase = Time::maxVal();
            disconnect();
            return MessageProcessingResult::HALTED;
        }
    }
    if (time_block <= time_exec) {
        return MessageProcessingResult::CONTINUE_PROCESSING;
    }
    if ((iterating == IterationRequest::NO_ITERATIONS) ||
        (time_exec > time_granted && iterating == IterationRequest::ITERATE_IF_NEEDED)) {
        iteration = 0;
        if (time_allow > time_exec) {
            updateTimeGrant();
            return MessageProcessingResult::NEXT_STEP;
        }
        if (time_allow == time_exec) {
            if (time_requested > time_exec || !info.wait_for_current_time_updates) {
                if (time_requested <= time_exec) {
                    updateTimeGrant();
                    return MessageProcessingResult::NEXT_STEP;
                }
                if (dependencies.checkIfReadyForTimeGrant(false, time_exec)) {
                    updateTimeGrant();
                    return MessageProcessingResult::NEXT_STEP;
                }
            }
        }
    } else {
        if (time_allow > time_exec) {
            ++iteration;
            updateTimeGrant();
            return MessageProcessingResult::ITERATING;
        }
        if (time_allow == time_exec)  // time_allow==time_exec==time_granted
        {
            if (dependencies.checkIfReadyForTimeGrant(true, time_exec)) {
                ++iteration;
                updateTimeGrant();
                return MessageProcessingResult::ITERATING;
            }
        }
    }

    // if we haven't returned we may need to update the time messages
    if ((!dependents.empty()) && (update)) {
        sendTimeRequest();
    }
    return MessageProcessingResult::CONTINUE_PROCESSING;
}

void TimeCoordinator::sendTimeRequest() const
{
    ActionMessage upd(CMD_TIME_REQUEST);
    upd.source_id = source_id;
    upd.actionTime = time_next;
    upd.Te = (time_exec != Time::maxVal()) ? time_exec + info.outputDelay : time_exec;
    upd.Tdemin = (time_minDe < time_next) ? time_next : time_minDe;

    if (iterating != IterationRequest::NO_ITERATIONS) {
        setIterationFlags(upd, iterating);
        upd.counter = iteration;
    }
    transmitTimingMessage(upd);
    //    printf("%d next=%f, exec=%f, Tdemin=%f\n", source_id, static_cast<double>(time_next),
    // static_cast<double>(time_exec), static_cast<double>(time_minDe));
}

void TimeCoordinator::updateTimeGrant()
{
    if (iterating != IterationRequest::FORCE_ITERATION) {
        time_granted = time_exec;
        time_grantBase = time_granted;
    }
    ActionMessage treq(CMD_TIME_GRANT);
    treq.source_id = source_id;
    treq.actionTime = time_granted;
    treq.counter = iteration;
    if (iterating != IterationRequest::NO_ITERATIONS) {
        dependencies.resetIteratingTimeRequests(time_exec);
    }
    transmitTimingMessage(treq);
    // printf("%d GRANT allow=%f next=%f, exec=%f, Tdemin=%f\n", source_id,
    // static_cast<double>(time_allow), static_cast<double>(time_next),
    // static_cast<double>(time_exec), static_cast<double>(time_minDe));
}
std::string TimeCoordinator::printTimeStatus() const
{
    return fmt::format(
        R"raw({{"granted_time":{},"requested_time":{}, "exec":{}, "allow":{}, "value":{}, "message":{}, "minDe":{}, "minminDe":{}}})raw",
        static_cast<double>(time_granted),
        static_cast<double>(time_requested),
        static_cast<double>(time_exec),
        static_cast<double>(time_allow),
        static_cast<double>(time_value),
        static_cast<double>(time_message),
        static_cast<double>(time_minDe),
        static_cast<double>(time_minminDe));
}

bool TimeCoordinator::isDependency(GlobalFederateId ofed) const
{
    return dependencies.isDependency(ofed);
}

bool TimeCoordinator::addDependency(GlobalFederateId fedID)
{
    if (dependencies.addDependency(fedID)) {
        dependency_federates.lock()->push_back(fedID);
        return true;
    }
    return false;
}

bool TimeCoordinator::addDependent(GlobalFederateId fedID)
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

void TimeCoordinator::removeDependency(GlobalFederateId fedID)
{
    dependencies.removeDependency(fedID);
    // remove the thread safe version
    auto dlock = dependency_federates.lock();
    auto res = std::find(dlock.begin(), dlock.end(), fedID);
    if (res != dlock.end()) {
        dlock->erase(res);
    }
}

void TimeCoordinator::removeDependent(GlobalFederateId fedID)
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

DependencyInfo* TimeCoordinator::getDependencyInfo(GlobalFederateId ofed)
{
    return dependencies.getDependencyInfo(ofed);
}

std::vector<GlobalFederateId> TimeCoordinator::getDependencies() const
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

MessageProcessingResult TimeCoordinator::checkExecEntry()
{
    auto ret = MessageProcessingResult::CONTINUE_PROCESSING;
    if (time_block <= timeZero) {
        return ret;
    }
    if (!dependencies.checkIfReadyForExecEntry(iterating != IterationRequest::NO_ITERATIONS)) {
        return ret;
    }
    switch (iterating) {
        case IterationRequest::NO_ITERATIONS:
            ret = MessageProcessingResult::NEXT_STEP;
            break;
        case IterationRequest::ITERATE_IF_NEEDED:
            if (hasInitUpdates) {
                if (iteration >= info.maxIterations) {
                    ret = MessageProcessingResult::NEXT_STEP;
                } else {
                    ret = MessageProcessingResult::ITERATING;
                }
            } else {
                ret = MessageProcessingResult::NEXT_STEP;
            }
            break;
        case IterationRequest::FORCE_ITERATION:
            ret = MessageProcessingResult::ITERATING;
            break;
    }

    if (ret == MessageProcessingResult::NEXT_STEP) {
        time_granted = timeZero;
        time_grantBase = time_granted;
        executionMode = true;
        iteration = 0;

        ActionMessage execgrant(CMD_EXEC_GRANT);
        execgrant.source_id = source_id;
        transmitTimingMessage(execgrant);
    } else if (ret == MessageProcessingResult::ITERATING) {
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

static bool isDelayableMessage(const ActionMessage& cmd, GlobalFederateId localId)
{
    return (((cmd.action() == CMD_TIME_GRANT) || (cmd.action() == CMD_EXEC_GRANT)) &&
            (cmd.source_id != localId));
}

message_process_result TimeCoordinator::processTimeMessage(const ActionMessage& cmd)
{
    switch (cmd.action()) {
        case CMD_TIME_BLOCK:
        case CMD_TIME_UNBLOCK:
        case CMD_TIME_BARRIER:
        case CMD_TIME_BARRIER_CLEAR:
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
            // this command requires removing dependents as well as dealing with dependency
            // processing
            removeDependent(GlobalFederateId(cmd.source_id));
            break;

        default:
            break;
    }
    if (isDelayableMessage(cmd, source_id)) {
        auto* dep = dependencies.getDependencyInfo(GlobalFederateId(cmd.source_id));
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
                if ((iterating != IterationRequest::NO_ITERATIONS) && (time_exec == dep->Tnext)) {
                    return message_process_result::delay_processing;
                }
                break;
            case DependencyInfo::time_state_t::exec_requested_iterative:
                if ((iterating != IterationRequest::NO_ITERATIONS) && (checkingExec)) {
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

Time TimeCoordinator::updateTimeBlocks(int32_t blockId, Time newTime)
{
    auto blk = std::find_if(timeBlocks.begin(), timeBlocks.end(), [blockId](const auto& block) {
        return (block.second == blockId);
    });
    if (blk != timeBlocks.end()) {
        blk->first = newTime;
    } else {
        timeBlocks.emplace_back(newTime, blockId);
    }
    auto res = std::min_element(timeBlocks.begin(),
                                timeBlocks.end(),
                                [](const auto& blk1, const auto& blk2) {
                                    return (blk1.first < blk2.first);
                                });
    return res->first;
}

message_process_result TimeCoordinator::processTimeBlockMessage(const ActionMessage& cmd)
{
    Time ltime = Time::maxVal();
    switch (cmd.action()) {
        case CMD_TIME_BLOCK:
        case CMD_TIME_BARRIER:
            ltime = updateTimeBlocks(cmd.messageID, cmd.actionTime);
            break;
        case CMD_TIME_UNBLOCK:
        case CMD_TIME_BARRIER_CLEAR:
            if (!timeBlocks.empty()) {
                ltime = updateTimeBlocks(cmd.messageID, Time::maxVal());
            }
            break;
        default:
            break;
    }
    if (ltime > time_block) {
        time_block = ltime;
        return message_process_result::processed;
    }
    time_block = ltime;
    return message_process_result::no_effect;
}

void TimeCoordinator::processDependencyUpdateMessage(const ActionMessage& cmd)
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

/** set a timeProperty for a the coordinator*/
void TimeCoordinator::setProperty(int timeProperty, Time propertyVal)
{
    switch (timeProperty) {
        case defs::Properties::OUTPUT_DELAY:
            info.outputDelay = propertyVal;
            break;
        case defs::Properties::INPUT_DELAY:
            info.inputDelay = propertyVal;
            break;
        case defs::Properties::TIME_DELTA:
            info.timeDelta = propertyVal;
            if (info.timeDelta <= timeZero) {
                info.timeDelta = timeEpsilon;
            }
            break;
        case defs::Properties::PERIOD:
            info.period = propertyVal;
            break;
        case defs::Properties::OFFSET:
            info.offset = propertyVal;
            break;
        default:
            break;
    }
}

/** set a timeProperty for a the coordinator*/
void TimeCoordinator::setProperty(int intProperty, int propertyVal)
{
    if (intProperty == defs::Properties::MAX_ITERATIONS) {
        info.maxIterations = propertyVal;
    } else {
        setProperty(intProperty, Time(static_cast<double>(propertyVal)));
    }
}

/** set an option Flag for a the coordinator*/
void TimeCoordinator::setOptionFlag(int optionFlag, bool value)
{
    switch (optionFlag) {
        case defs::Flags::UNINTERRUPTIBLE:
            info.uninterruptible = value;
            break;
        case defs::Flags::WAIT_FOR_CURRENT_TIME_UPDATE:
            info.wait_for_current_time_updates = value;
            break;
        case defs::Flags::RESTRICTIVE_TIME_POLICY:
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
        case defs::Properties::OUTPUT_DELAY:
            return info.outputDelay;
        case defs::Properties::INPUT_DELAY:
            return info.inputDelay;
        case defs::Properties::TIME_DELTA:
            return info.timeDelta;
        case defs::Properties::PERIOD:
            return info.period;
        case defs::Properties::OFFSET:
            return info.offset;
        default:
            return Time::minVal();
    }
}

/** get a time Property*/
int TimeCoordinator::getIntegerProperty(int intProperty) const
{
    switch (intProperty) {  // NOLINT
        case defs::Properties::MAX_ITERATIONS:
            return info.maxIterations;
        default:
            // TODO(PT): make this something consistent
            return -972;
    }
}

/** get an option flag value*/
bool TimeCoordinator::getOptionFlag(int optionFlag) const
{
    switch (optionFlag) {
        case defs::Flags::UNINTERRUPTIBLE:
            return info.uninterruptible;
        case defs::Flags::INTERRUPTIBLE:
            return !info.uninterruptible;
        case defs::Flags::WAIT_FOR_CURRENT_TIME_UPDATE:
            return info.wait_for_current_time_updates;
        case defs::Flags::RESTRICTIVE_TIME_POLICY:
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

}  // namespace helics
