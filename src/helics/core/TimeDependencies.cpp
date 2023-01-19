/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "TimeDependencies.hpp"

#include "ActionMessage.hpp"
#include "flagOperations.hpp"

#include "json/json.h"
#include <algorithm>
#include <cassert>
#include <string>

namespace helics {

static TimeProcessingResult processMessage(const ActionMessage& m, DependencyInfo& dep)
{
    TimeProcessingResult res{TimeProcessingResult::PROCESSED};
    bool delayed{false};
    switch (m.action()) {
        case CMD_EXEC_REQUEST:
            dep.mTimeState = checkActionFlag(m, iteration_requested_flag) ?
                (checkActionFlag(m, required_flag) ? TimeState::exec_requested_require_iteration :
                                                     TimeState::exec_requested_iterative) :
                TimeState::exec_requested;
            delayed = checkActionFlag(m, delayed_timing_flag);
            if (delayed && !dep.delayedTiming) {
                res = TimeProcessingResult::PROCESSED_AND_CHECK;
            }
            dep.delayedTiming = delayed;
            dep.restrictionLevel = m.messageID;
            dep.sequenceCounter = m.counter;
            dep.minFed = GlobalFederateId(m.getExtraData());
            dep.responseSequenceCounter = m.getExtraDestData();
            if (dep.connection == ConnectionType::SELF) {
                dep.responseSequenceCounter = dep.sequenceCounter;
            }
            if (dep.responseSequenceCounter == dep.grantedIteration) {
                dep.updateRequested = false;
            }
            break;
        case CMD_EXEC_GRANT:
            if (!checkActionFlag(m, iteration_requested_flag)) {
                dep.mTimeState = TimeState::time_granted;
                dep.next = timeZero;
                dep.minDe = timeZero;
                dep.timeoutCount = 0;
                dep.Te = timeZero;
                dep.sequenceCounter = 0;
                dep.responseSequenceCounter = 0;
                dep.grantedIteration = 0;
                dep.restrictionLevel = 0;
                dep.minFed = GlobalFederateId{};
            } else {
                dep.mTimeState = TimeState::initialized;
                dep.sequenceCounter = m.counter;
                dep.responseSequenceCounter = 0;
                dep.restrictionLevel = 0;
            }
            dep.hasData = false;
            break;
        case CMD_TIME_REQUEST:
            if (dep.mTimeState == TimeState::time_granted) {
                dep.lastGrant = dep.next;
                res = TimeProcessingResult::PROCESSED_NEW_REQUEST;
            }
            dep.mTimeState = checkActionFlag(m, iteration_requested_flag) ?
                (checkActionFlag(m, required_flag) ? TimeState::time_requested_require_iteration :
                                                     TimeState::time_requested_iterative) :
                TimeState::time_requested;
            //   printf("%d Request from %d time %f, te=%f, Tdemin=%f\n", fedID, m.source_id,
            //   static_cast<double>(m.actionTime), static_cast<double>(m.Te),
            //   static_cast<double>(m.Tdemin)); assert(m.actionTime >= Tnext);
            dep.next = m.actionTime;
            dep.Te = m.Te;
            dep.minDe = m.Tdemin;

            if (dep.Te < dep.minDe) {
                dep.minDe = dep.Te;
            }

            dep.minFed = GlobalFederateId(m.getExtraData());
            dep.interrupted = checkActionFlag(m, interrupted_flag);

            // NEXT version this gets moved out of here
            if (checkActionFlag(m, non_granting_flag)) {
                dep.nonGranting = true;
            }

            delayed = checkActionFlag(m, delayed_timing_flag);
            if (delayed && !dep.delayedTiming) {
                res = TimeProcessingResult::PROCESSED_AND_CHECK;
            }
            if (delayed) {
                dep.delayedTiming = delayed;
            }
            // END remove block

            dep.triggered = checkActionFlag(m, destination_target);
            dep.sequenceCounter = m.counter;
            dep.responseSequenceCounter = (dep.connection != ConnectionType::SELF) ?
                m.getExtraDestData() :
                dep.sequenceCounter;
            if (dep.responseSequenceCounter == dep.grantedIteration) {
                dep.updateRequested = false;
            }
            break;
        case CMD_TIME_GRANT:
            dep.mTimeState = TimeState::time_granted;
            //    printf("%d Grant from %d time %f\n", fedID, m.source_id,
            //    static_cast<double>(m.actionTime));
            //   assert(m.actionTime >= Tnext);
            dep.next = m.actionTime;
            dep.Te = dep.next;
            dep.minDe = dep.next;
            dep.minFed = GlobalFederateId{};
            dep.timeoutCount = 0;
            dep.interrupted = false;
            dep.sequenceCounter = m.counter;
            dep.hasData = false;
            break;
        case CMD_DISCONNECT:
        case CMD_PRIORITY_DISCONNECT:
        case CMD_BROADCAST_DISCONNECT:
        case CMD_DISCONNECT_CORE:
        case CMD_DISCONNECT_BROKER:
        case CMD_DISCONNECT_FED:
            dep.mTimeState = TimeState::time_granted;
            //   printf("%d disconnect from %d\n", fedID, m.source_id);
            dep.next = Time::maxVal();
            dep.Te = Time::maxVal();
            dep.minDe = Time::maxVal();
            dep.minFed = GlobalFederateId{};
            dep.timeoutCount = 0;
            dep.hasData = false;
            dep.updateRequested = false;
            break;
        case CMD_TIMING_INFO:
            dep.nonGranting = checkActionFlag(m, non_granting_flag);
            dep.delayedTiming = checkActionFlag(m, delayed_timing_flag);
            dep.timingVersion = static_cast<std::uint8_t>(m.getExtraData());
            res = TimeProcessingResult::PROCESSED_AND_CHECK;
            break;
        case CMD_LOCAL_ERROR:
        case CMD_GLOBAL_ERROR:
            dep.mTimeState = TimeState::error;
            //   printf("%d disconnect from %d\n", fedID, m.source_id);
            dep.next = Time::maxVal();
            dep.Te = Time::maxVal();
            dep.minDe = Time::maxVal();
            dep.minFed = GlobalFederateId{};
            dep.timeoutCount = 0;
            break;
        case CMD_SEND_MESSAGE:
        case CMD_PUB:
            dep.hasData = true;
            break;
        case CMD_REQUEST_CURRENT_TIME:
            dep.sequenceCounter = m.counter;
            break;
        default:
            res = TimeProcessingResult::NOT_PROCESSED;
            break;
    }
    return res;
}

bool TimeData::update(const TimeData& update)
{
    bool updated = (mTimeState != update.mTimeState);
    mTimeState = update.mTimeState;

    Time prev_next = next;
    next = update.next;

    if (update.Te != Te) {
        updated = true;
        Te = update.Te;
    }

    if (update.minDe != minDe) {
        updated = true;
        minDe = update.minDe;
    }
    if (update.TeAlt != TeAlt) {
        updated = true;
        TeAlt = update.TeAlt;
    }

    if (prev_next != next) {
        updated = true;
    }

    if (update.minFed != minFed) {
        minFed = update.minFed;
        updated = true;
    }
    if (update.sequenceCounter != sequenceCounter) {
        sequenceCounter = update.sequenceCounter;
    }

    if (update.interrupted != interrupted) {
        interrupted = update.interrupted;
        updated = true;
    }

    if (update.responseSequenceCounter != responseSequenceCounter) {
        responseSequenceCounter = update.responseSequenceCounter;
        updated = true;
    }
    if (update.minFedActual != minFedActual) {
        minFedActual = update.minFedActual;
        updated = true;
    }

    return updated;
}

static std::string_view timeStateString(TimeState state)
{
    static constexpr std::string_view init{"initialized"};
    static constexpr std::string_view granted{"granted"};
    static constexpr std::string_view errorString{"error"};
    static constexpr std::string_view execReq{"exec requested"};
    static constexpr std::string_view execReqIt{"exec requested iterative"};
    static constexpr std::string_view execMustIt{"exec requested required iteration"};
    static constexpr std::string_view timeReq{"time requested"};
    static constexpr std::string_view timeReqIterative{"time requested iterative"};
    static constexpr std::string_view timeReqMustIt{"time requested required iteration"};
    // static constexpr std::string_view disconnected{"disconnected"};
    static constexpr std::string_view other{"other"};
    switch (state) {
        case TimeState::initialized:
            return init;
        case TimeState::time_granted:
            return granted;
        case TimeState::error:
            return errorString;
        case TimeState::exec_requested:
            return execReq;
        case TimeState::exec_requested_iterative:
            return execReqIt;
        case TimeState::exec_requested_require_iteration:
            return execMustIt;
        case TimeState::time_requested:
            return timeReq;
        case TimeState::time_requested_iterative:
            return timeReqIterative;
        case TimeState::time_requested_require_iteration:
            return timeReqMustIt;
        default:
            return other;
    }
}

void addTimeState(Json::Value& output, const TimeState state)
{
    auto sstring = timeStateString(state);
    output["state"] = Json::Value(sstring.data(), sstring.data() + sstring.size());
}

void generateJsonOutputTimeData(Json::Value& output, const TimeData& dep, bool includeAggregates)
{
    output["next"] = static_cast<double>(dep.next);
    output["te"] = static_cast<double>(dep.Te);
    output["minde"] = static_cast<double>(dep.minDe);
    output["minfed"] = dep.minFed.baseValue();
    output["responseSequence"] = dep.responseSequenceCounter;
    addTimeState(output, dep.mTimeState);
    output["iteration"] = dep.sequenceCounter;
    output["granted_iteration"] = dep.grantedIteration;
    output["sequenceCounter"] = dep.sequenceCounter;
    output["interrupted"] = dep.interrupted;
    output["delayed"] = dep.delayedTiming;
    if (includeAggregates) {
        output["minde_alt"] = static_cast<double>(dep.minDe);
        output["minfedActual"] = dep.minFedActual.baseValue();
    }
}

void generateJsonOutputDependency(Json::Value& output, const DependencyInfo& dep)
{
    output["id"] = dep.fedID.baseValue();
    generateJsonOutputTimeData(output, dep, false);
    switch (dep.connection) {
        case ConnectionType::CHILD:
            output["connection"] = "child";
            break;
        case ConnectionType::PARENT:
            output["connection"] = "parent";
            break;
        case ConnectionType::INDEPENDENT:
            output["connection"] = "independent";
            break;
        case ConnectionType::SELF:
            output["connection"] = "self";
            break;
        case ConnectionType::NONE:
        default:
            output["connection"] = "none";
            break;
    }
}

// comparison helper lambda for comparing dependencies
static auto dependencyCompare = [](const auto& dep, auto& target) { return (dep.fedID < target); };

bool TimeDependencies::isDependency(GlobalFederateId ofed) const
{
    auto res = std::lower_bound(dependencies.begin(), dependencies.end(), ofed, dependencyCompare);
    if (res == dependencies.end()) {
        return false;
    }
    return (res->fedID == ofed) ? res->dependency : false;
}

bool TimeDependencies::isDependent(GlobalFederateId ofed) const
{
    auto res = std::lower_bound(dependencies.begin(), dependencies.end(), ofed, dependencyCompare);
    if (res == dependencies.end()) {
        return false;
    }
    return (res->fedID == ofed) ? res->dependent : false;
}

const DependencyInfo* TimeDependencies::getDependencyInfo(GlobalFederateId id) const
{
    auto res = std::lower_bound(dependencies.cbegin(), dependencies.cend(), id, dependencyCompare);
    if ((res == dependencies.cend()) || (res->fedID != id)) {
        return nullptr;
    }

    return &(*res);
}

DependencyInfo* TimeDependencies::getDependencyInfo(GlobalFederateId id)
{
    auto res = std::lower_bound(dependencies.begin(), dependencies.end(), id, dependencyCompare);
    if ((res == dependencies.end()) || (res->fedID != id)) {
        return nullptr;
    }

    return &(*res);
}

bool TimeDependencies::addDependency(GlobalFederateId id)

{
    if (dependencies.empty()) {
        dependencies.emplace_back(id);
        dependencies.back().dependency = true;
        return true;
    }
    auto dep = std::lower_bound(dependencies.begin(), dependencies.end(), id, dependencyCompare);
    if (dep == dependencies.end()) {
        dependencies.emplace_back(id);
        dependencies.back().dependency = true;
    } else {
        if (dep->fedID == id) {
            auto rval = dep->dependency;
            dep->dependency = true;
            // the dependency is already present
            return !rval;
        }
        auto it = dependencies.emplace(dep, id);
        it->dependency = true;
    }
    return true;
}

void TimeDependencies::removeDependency(GlobalFederateId id)
{
    auto dep = std::lower_bound(dependencies.begin(), dependencies.end(), id, dependencyCompare);
    if (dep != dependencies.end()) {
        if (dep->fedID == id) {
            dep->dependency = false;
            if (!dep->dependent) {
                dependencies.erase(dep);
            }
        }
    }
}

bool TimeDependencies::addDependent(GlobalFederateId id)

{
    if (dependencies.empty()) {
        dependencies.emplace_back(id);
        dependencies.back().dependent = true;
        return true;
    }
    auto dep = std::lower_bound(dependencies.begin(), dependencies.end(), id, dependencyCompare);
    if (dep == dependencies.end()) {
        dependencies.emplace_back(id);
        dependencies.back().dependent = true;
    } else {
        if (dep->fedID == id) {
            auto rval = dep->dependent;
            dep->dependent = true;
            // the dependency is already present
            return !rval;
        }
        auto it = dependencies.emplace(dep, id);
        it->dependent = true;
    }
    return true;
}

void TimeDependencies::removeDependent(GlobalFederateId id)
{
    auto dep = std::lower_bound(dependencies.begin(), dependencies.end(), id, dependencyCompare);
    if (dep != dependencies.end()) {
        if (dep->fedID == id) {
            dep->dependent = false;
            if (!dep->dependency) {
                dependencies.erase(dep);
            }
        }
    }
}

void TimeDependencies::removeInterdependence(GlobalFederateId id)
{
    auto dep = std::lower_bound(dependencies.begin(), dependencies.end(), id, dependencyCompare);
    if (dep != dependencies.end()) {
        if (dep->fedID == id) {
            dependencies.erase(dep);
        }
    }
}

TimeProcessingResult TimeDependencies::updateTime(const ActionMessage& m)
{
    auto* depInfo = getDependencyInfo(m.source_id);
    if (depInfo == nullptr || !depInfo->dependency) {
        return TimeProcessingResult::NOT_PROCESSED;
    }
    return processMessage(m, *depInfo);
}

bool TimeDependencies::checkIfAllDependenciesArePastExec(bool iterating) const
{
    auto minstate =
        iterating ? TimeState::time_requested_require_iteration : TimeState::time_requested;

    return std::all_of(dependencies.begin(), dependencies.end(), [minstate](const auto& dep) {
        return ((!dep.dependency) || (dep.connection == ConnectionType::SELF) ||
                (dep.mTimeState >= minstate) ||
                (dep.mTimeState == TimeState::time_granted && dep.next > timeZero));
    });

    /* return std::none_of(dependencies.begin(), dependencies.end(), [minstate](const auto& dep) {
         return (dep.dependency && dep.connection != ConnectionType::SELF &&
                 (dep.mTimeState < minstate));
     });*/
}

bool TimeDependencies::checkIfReadyForExecEntry(bool iterating, bool waiting) const
{
    if (iterating) {
        if (waiting) {
            for (const auto& dep : dependencies) {
                if (!dep.dependency) {
                    continue;
                }
                if (dep.connection == ConnectionType::SELF) {
                    continue;
                }
                if (dep.mTimeState == TimeState::initialized) {
                    if (dep.grantedIteration == 0) {
                        return false;
                    }
                }
                if (dep.mTimeState == TimeState::exec_requested_iterative ||
                    dep.mTimeState == TimeState::exec_requested_require_iteration) {
                    if (dep.sequenceCounter < dep.grantedIteration) {
                        return false;
                    }
                }
            }
            return true;
        }
        return std::none_of(dependencies.begin(), dependencies.end(), [](const auto& dep) {
            return (dep.dependency && dep.mTimeState == TimeState::initialized);
        });
    }
    if (waiting) {
        return checkIfAllDependenciesArePastExec(false);
    }
    return std::none_of(dependencies.begin(), dependencies.end(), [](const auto& dep) {
        return (dep.dependency &&
                (!(dep.mTimeState >= TimeState::exec_requested ||
                   (dep.connection == ConnectionType::SELF &&
                    dep.mTimeState >= TimeState::initialized))));
    });
}

bool TimeDependencies::checkIfReadyForTimeGrant(bool iterating,
                                                Time desiredGrantTime,
                                                GrantDelayMode delayMode) const
{
    if (iterating) {
        for (const auto& dep : dependencies) {
            if (!dep.dependency || dep.next >= cBigTime) {
                continue;
            }
            if (dep.connection == ConnectionType::SELF) {
                continue;
            }
            if (dep.next < desiredGrantTime) {
                return false;
            }
            if ((dep.next == desiredGrantTime) && (dep.mTimeState == TimeState::time_granted)) {
                return false;
            }
            if (delayMode == GrantDelayMode::WAITING) {
                if (dep.mTimeState == TimeState::time_requested_iterative ||
                    dep.mTimeState == TimeState::time_requested_require_iteration) {
                    if (dep.sequenceCounter < dep.grantedIteration) {
                        return false;
                    }
                }
            }
        }
        return true;
    }
    switch (delayMode) {
        case GrantDelayMode::NONE:
            for (const auto& dep : dependencies) {
                if (!dep.dependency || dep.next >= cBigTime) {
                    continue;
                }
                if (dep.connection == ConnectionType::SELF) {
                    continue;
                }
                if (dep.next < desiredGrantTime) {
                    return false;
                }
                if (dep.next == desiredGrantTime) {
                    if (dep.mTimeState == TimeState::time_granted) {
                        return false;
                    }
                    if (dep.mTimeState == TimeState::time_requested && dep.nonGranting) {
                        return false;
                    }
                }
            }
            break;
        case GrantDelayMode::INTERRUPTED:
            for (const auto& dep : dependencies) {
                if (!dep.dependency || dep.next >= cBigTime) {
                    continue;
                }
                if (dep.connection == ConnectionType::SELF) {
                    continue;
                }
                if (dep.next < desiredGrantTime) {
                    return false;
                }
                if (dep.next == desiredGrantTime) {
                    if (dep.mTimeState == TimeState::time_granted) {
                        return false;
                    }
                    if (dep.mTimeState == TimeState::time_requested && dep.nonGranting) {
                        return false;
                    }

                    if (dep.interrupted || dep.delayedTiming) {
                        continue;
                    }
                    return false;
                }
            }
            break;
        case GrantDelayMode::WAITING:
            for (const auto& dep : dependencies) {
                if (!dep.dependency || dep.next >= cBigTime) {
                    continue;
                }
                if (dep.connection == ConnectionType::SELF) {
                    continue;
                }
                if (dep.next <= desiredGrantTime) {
                    return false;
                }
            }
            break;
    }

    return true;
}

bool TimeDependencies::hasActiveTimeDependencies() const
{
    return std::any_of(dependencies.begin(), dependencies.end(), [](const auto& dep) {
        return (dep.dependency && (dep.fedID.isFederate()) && (dep.next < cBigTime));
    });
}

bool TimeDependencies::verifySequenceCounter(Time tmin, std::int32_t sq)
{
    return std::all_of(dependencies.begin(), dependencies.end(), [tmin, sq](const auto& dep) {
        return checkSequenceCounter(dep, tmin, sq);
    });
}
int TimeDependencies::activeDependencyCount() const
{
    return std::count_if(dependencies.begin(), dependencies.end(), [](const auto& dep) {
        return (dep.dependency && (dep.fedID.isFederate()) && (dep.next < cBigTime));
    });
}
/** get a count of the active dependencies*/
GlobalFederateId TimeDependencies::getMinDependency() const
{
    GlobalFederateId minID;
    Time minTime(Time::maxVal());
    for (const auto& dep : dependencies) {
        if (dep.dependency && (dep.fedID.isFederate()) && (dep.next < cBigTime)) {
            if (dep.next < minTime) {
                minTime = dep.next;
                minID = dep.fedID;
            }
        }
    }
    return minID;
}

void TimeDependencies::resetIteratingExecRequests()
{
    for (auto& dep : dependencies) {
        if (dep.dependency && dep.mTimeState <= TimeState::exec_requested_iterative) {
            dep.mTimeState = TimeState::initialized;
            dep.grantedIteration = dep.sequenceCounter;
            dep.sequenceCounter = 0;
            dep.responseSequenceCounter = 0;
            dep.restrictionLevel = 0;
            dep.minFed = GlobalFederateId();
        }
    }
}

void TimeDependencies::resetIteratingTimeRequests(helics::Time requestTime)
{
    for (auto& dep : dependencies) {
        if (dep.dependency && dep.mTimeState == TimeState::time_requested_iterative) {
            if (dep.next == requestTime) {
                dep.mTimeState = TimeState::time_granted;
                dep.Te = requestTime;
                dep.minDe = requestTime;
            }
        }
    }
}

void TimeDependencies::resetDependentEvents(helics::Time grantTime)
{
    for (auto& dep : dependencies) {
        if (dep.dependency) {
            dep.Te = (std::max)(dep.next, grantTime);
            dep.minDe = dep.Te;
        }
    }
}

std::pair<int, std::string> TimeDependencies::checkForIssues(bool waiting) const
{
    // check for timing deadlock with wait_for_current_time_flag

    bool hasDelayedTiming = waiting;
    for (const auto& dep : dependencies) {
        if (dep.dependency && dep.dependent && dep.delayedTiming &&
            dep.connection != ConnectionType::SELF) {
            mDelayedDependency = dep.fedID;
            if (hasDelayedTiming) {
                return {
                    multiple_wait_for_current_time_flags,
                    "Multiple federates declaring wait_for_current_time flag will result in deadlock"};
            }
            hasDelayedTiming = true;
        }
    }
    return {0, ""};
}

static void generateMinTimeImplementation(TimeData& mTime,
                                          const DependencyInfo& dep,
                                          GlobalFederateId ignore,
                                          std::int32_t sequenceCode)
{
    if (dep.mTimeState < TimeState::time_granted) {
        if (dep.fedID == ignore) {
            return;
        }
        if (dep.mTimeState < mTime.mTimeState) {
            mTime.minFed = dep.fedID;
            mTime.mTimeState = dep.mTimeState;
            mTime.delayedTiming = dep.delayedTiming;
            mTime.restrictionLevel = dep.restrictionLevel;
            mTime.sequenceCounter = dep.sequenceCounter;
            mTime.responseSequenceCounter = dep.responseSequenceCounter;
        } else if (dep.mTimeState == mTime.mTimeState) {
            if (dep.restrictionLevel < mTime.restrictionLevel) {
                mTime.minFed = dep.fedID;
                mTime.delayedTiming = dep.delayedTiming;
                mTime.restrictionLevel = dep.restrictionLevel;
                mTime.sequenceCounter = dep.sequenceCounter;
                mTime.responseSequenceCounter = dep.sequenceCounter;
            } else if (dep.restrictionLevel == mTime.restrictionLevel && dep.fedID < mTime.minFed) {
                mTime.minFed = dep.fedID;
                mTime.delayedTiming = dep.delayedTiming;
                mTime.sequenceCounter = dep.sequenceCounter;
                mTime.responseSequenceCounter = dep.sequenceCounter;
            }
        }
        mTime.next = initializationTime;
        mTime.Te = timeZero;
        mTime.minDe = timeZero;
        return;
    }

    if (dep.fedID == ignore) {
        if (dep.fedID.isBroker()) {
            if (dep.Te < mTime.minDe) {
                mTime.minDe = dep.Te;
            }
        }

        return;
    }

    if (dep.connection != ConnectionType::SELF &&
        (sequenceCode == 0 || dep.responseSequenceCounter == sequenceCode ||
         dep.timingVersion == 0 || !dep.dependent)) {
        if (dep.minDe >= dep.next) {
            if (dep.minDe < mTime.minDe) {
                mTime.minDe = dep.minDe;
            }
        } else {
            // this minimum dependent event time received was invalid and can't be trusted
            // therefore it can't be used to determine a time grant
            mTime.minDe = -1;
        }
    } else if (dep.responseSequenceCounter == sequenceCode && dep.dependent) {
        if (dep.minDe >= dep.next && dep.minDe < mTime.minDe) {
            mTime.minDe = dep.minDe;
        }
    } else {
        if (dep.next < mTime.minDe) {
            mTime.minDe = dep.next;
        }
    }
    if (dep.next < mTime.next) {
        mTime.next = dep.next;
        mTime.mTimeState = dep.mTimeState;
        if (dep.responseSequenceCounter == sequenceCode && dep.dependent) {
            mTime.interrupted = dep.interrupted;
        } else {
            mTime.interrupted = false;
        }
    } else if (dep.next == mTime.next) {
        if (dep.mTimeState == TimeState::time_granted) {
            mTime.mTimeState = dep.mTimeState;
            mTime.interrupted = false;
        } else if (!dep.interrupted) {
            mTime.interrupted = false;
        }
    }
    // if (dep.connection != ConnectionType::self) {
    if (dep.Te < mTime.Te) {
        mTime.TeAlt = mTime.Te;
        mTime.Te = dep.Te;
        mTime.minFed = dep.fedID;
        mTime.sequenceCounter = dep.sequenceCounter;
        mTime.responseSequenceCounter = dep.sequenceCounter;

        if (dep.minFed.isValid()) {
            mTime.minFedActual = dep.minFed;
        } else {
            mTime.minFed = dep.fedID;
        }
    } else if (dep.Te == mTime.Te) {
        mTime.minFed = GlobalFederateId{};
        mTime.TeAlt = mTime.Te;
    }
    // }
}

const DependencyInfo& getExecEntryMinFederate(const TimeDependencies& dependencies,
                                              GlobalFederateId self,
                                              ConnectionType ignoreType,
                                              GlobalFederateId ignore)
{
    static DependencyInfo maxDep{Time::maxVal(), TimeState::initialized, 50U};

    const DependencyInfo* minDep = &maxDep;

    for (const auto& dep : dependencies) {
        if (!dep.dependency) {
            continue;
        }
        if (dep.fedID == ignore) {
            continue;
        }
        if (dep.connection == ConnectionType::SELF || dep.connection == ignoreType) {
            continue;
        }
        if (self.isValid() && dep.minFedActual == self) {
            continue;
        }
        if (dep.mTimeState > TimeState::exec_requested_iterative) {
            continue;
        }
        if (dep.restrictionLevel > minDep->restrictionLevel) {
            continue;
        }
        if (!minDep->fedID.isValid() || dep.fedID < minDep->fedID) {
            minDep = &dep;
            if (minDep->mTimeState == TimeState::initialized) {
                minDep = &maxDep;
                break;
            }
        }
    }
    return *minDep;
}

TimeData generateMinTimeUpstream(const TimeDependencies& dependencies,
                                 bool restricted,
                                 GlobalFederateId self,
                                 GlobalFederateId ignore,
                                 std::int32_t responseCode)
{
    TimeData mTime(Time::maxVal(), TimeState::error);
    std::int32_t iterationCount{0};
    for (const auto& dep : dependencies) {
        if (!dep.dependency) {
            continue;
        }
        if (dep.connection == ConnectionType::PARENT) {
            continue;
        }
        if (self.isValid() && dep.minFedActual == self) {
            continue;
        }
        iterationCount += dep.sequenceCounter;
        generateMinTimeImplementation(mTime, dep, ignore, responseCode);
    }
    if (mTime.Te < mTime.minDe) {
        mTime.minDe = mTime.Te;
    }

    if (!restricted) {
        if (mTime.minDe > mTime.next) {
            mTime.next = mTime.minDe;
        }
    }
    mTime.sequenceCounter = iterationCount;
    if (mTime.mTimeState < TimeState::time_granted) {
        mTime.next = initializationTime;
        mTime.minDe = initializationTime;
        mTime.Te = initializationTime;
    }
    return mTime;
}

TimeData generateMinTimeDownstream(const TimeDependencies& dependencies,
                                   bool restricted,
                                   GlobalFederateId self,
                                   GlobalFederateId ignore,
                                   std::int32_t responseCode)
{
    TimeData mTime(Time::maxVal(), TimeState::error);
    for (const auto& dep : dependencies) {
        if (!dep.dependency) {
            continue;
        }
        if (dep.connection != ConnectionType::PARENT) {
            continue;
        }
        if (self.isValid() && dep.minFedActual == self) {
            continue;
        }
        generateMinTimeImplementation(mTime, dep, ignore, responseCode);
    }
    if (mTime.Te < mTime.minDe) {
        mTime.minDe = mTime.Te;
    }

    if (!restricted) {
        if (mTime.minDe > mTime.next) {
            mTime.next = mTime.minDe;
        }
    }
    if (mTime.mTimeState < TimeState::time_granted) {
        mTime.next = initializationTime;
        mTime.minDe = initializationTime;
        mTime.Te = initializationTime;

        if (mTime.mTimeState < TimeState::exec_requested) {
            const auto& res =
                getExecEntryMinFederate(dependencies, self, ConnectionType::CHILD, ignore);
            mTime.minFed = res.fedID;
        }
    }
    return mTime;
}

TimeData generateMinTimeTotal(const TimeDependencies& dependencies,
                              bool restricted,
                              GlobalFederateId self,
                              GlobalFederateId ignore,
                              std::int32_t responseCode)
{
    TimeData mTime(Time::maxVal(), TimeState::error);
    for (const auto& dep : dependencies) {
        if (!dep.dependency) {
            continue;
        }

        if (self.isValid() && dep.minFedActual == self) {
            continue;
        }
        generateMinTimeImplementation(mTime, dep, ignore, responseCode);
    }

    if (mTime.Te < mTime.minDe) {
        mTime.minDe = mTime.Te;
    }

    if (!restricted) {
        if (mTime.minDe > mTime.next) {
            mTime.next = mTime.minDe;
        }
    }
    if (mTime.mTimeState < TimeState::time_granted) {
        mTime.next = initializationTime;
        mTime.minDe = initializationTime;
        mTime.Te = initializationTime;
    }
    return mTime;
}
}  // namespace helics
