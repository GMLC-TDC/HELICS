/*
Copyright (c) 2017-2020,
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

namespace helics {
static bool ProcessMessage(const ActionMessage& m, DependencyInfo& dep)
{
    switch (m.action()) {
        case CMD_EXEC_REQUEST:
            dep.time_state = checkActionFlag(m, iteration_requested_flag) ?
                time_state_t::exec_requested_iterative :
                time_state_t::exec_requested;
            break;
        case CMD_EXEC_GRANT:
            if (!checkActionFlag(m, iteration_requested_flag)) {
                dep.time_state = time_state_t::time_granted;
                dep.next = timeZero;
                dep.minDe = timeZero;
                dep.Te = timeZero;
            } else {
                dep.time_state = time_state_t::initialized;
            }
            break;
        case CMD_TIME_REQUEST:
            dep.time_state = checkActionFlag(m, iteration_requested_flag) ?
                time_state_t::time_requested_iterative :
                time_state_t::time_requested;
            //   printf("%d Request from %d time %f, te=%f, Tdemin=%f\n", fedID, m.source_id,
            //   static_cast<double>(m.actionTime), static_cast<double>(m.Te),
            //   static_cast<double>(m.Tdemin)); assert(m.actionTime >= Tnext);
            dep.next = m.actionTime;
            dep.Te = m.Te;
            dep.minDe = m.Tdemin;

            if (dep.Te < dep.minDe) {
                dep.minDe = dep.Te;
            }

            dep.minFed = global_federate_id(m.getExtraData());
            break;
        case CMD_TIME_GRANT:
            dep.time_state = time_state_t::time_granted;
            //    printf("%d Grant from %d time %f\n", fedID, m.source_id,
            //    static_cast<double>(m.actionTime));
            //   assert(m.actionTime >= Tnext);
            dep.next = m.actionTime;
            dep.Te = dep.next;
            dep.minDe = dep.next;
            dep.minFed = global_federate_id{};
            break;
        case CMD_DISCONNECT:
        case CMD_PRIORITY_DISCONNECT:
        case CMD_BROADCAST_DISCONNECT:
        case CMD_DISCONNECT_CORE:
        case CMD_DISCONNECT_BROKER:
        case CMD_DISCONNECT_FED:
            dep.time_state = time_state_t::time_granted;
            //   printf("%d disconnect from %d\n", fedID, m.source_id);
            dep.next = Time::maxVal();
            dep.Te = Time::maxVal();
            dep.minDe = Time::maxVal();
            dep.minFed = global_federate_id{};
            break;
        case CMD_LOCAL_ERROR:
        case CMD_GLOBAL_ERROR:
            dep.time_state = time_state_t::error;
            //   printf("%d disconnect from %d\n", fedID, m.source_id);
            dep.next = Time::maxVal();
            dep.Te = Time::maxVal();
            dep.minDe = Time::maxVal();
            dep.minFed = global_federate_id{};
            break;
        default:
            return false;
    }
    return true;
}

bool TimeData::update(const TimeData& update)
{
    bool updated = (time_state != update.time_state);
    time_state = update.time_state;

    Time prev_next = next;
    next = update.next;

    if (update.minDe != minDe) {
        updated = true;
        minDe = update.minDe;
    }
    if (update.minminDe != minminDe) {
        minminDe = update.minminDe;
        updated = true;
    }

    if (prev_next != next) {
        updated = true;
    }

    if (update.minFed != minFed) {
        minFed = update.minFed;
        updated = true;
    }
    if (update.minFedActual != minFedActual) {
        minFedActual = update.minFedActual;
        updated = true;
    }

    return updated;
}

static const std::string& timeStateString(time_state_t state)
{
    static const std::string init{"initialized"};
    static const std::string granted{"granted"};
    static const std::string errorString{"error"};
    static const std::string execReq{"exec requested"};
    static const std::string timeReq{"time requested"};
    static const std::string disconnected{"disconnected"};
    static const std::string other{"other"};
    switch (state)
    {
        case time_state_t::initialized:
            return init;
        case time_state_t::time_granted:
            return granted;
        case time_state_t::error:
            return errorString;
        case time_state_t::exec_requested:
            return execReq;
        case time_state_t::time_requested:
            return timeReq;
        default:
            return other;
    }
}

void generateJsonOutputTimeData(Json::Value & output, const TimeData& dep, bool includeAggregates)
    {
    output["next"] = static_cast<double>(dep.next);
    output["te"] = static_cast<double>(dep.Te);
    output["minde"] = static_cast<double>(dep.minDe);
    output["minfed"] = dep.minFed.baseValue();
    output["state"] = timeStateString(dep.time_state);
    if (includeAggregates)
    {
        output["minminde"] = static_cast<double>(dep.minminDe);
        output["minfedActual"] = dep.minFedActual.baseValue();
    }
    
}

void generateJsonOutputDependency(Json::Value& output, const DependencyInfo& dep)
{
    output["id"] = dep.fedID.baseValue();
    generateJsonOutputTimeData(output, dep,false);
    switch (dep.connection) {
        case ConnectionType::child:
            output["connection"] = "child";
            break;
        case ConnectionType::parent:
            output["connection"] = "parent";
            break;
        case ConnectionType::independent:
            output["connection"] = "independent";
            break;
        case ConnectionType::self:
            output["connection"] = "self";
            break;
        case ConnectionType::none:
        default:
            output["connection"] = "none";
            break;
    }
}

// comparison helper lambda for comparing dependencies
static auto dependencyCompare = [](const auto& dep, auto& target) { return (dep.fedID < target); };

bool TimeDependencies::isDependency(global_federate_id ofed) const
{
    auto res = std::lower_bound(dependencies.begin(), dependencies.end(), ofed, dependencyCompare);
    if (res == dependencies.end()) {
        return false;
    }
    return (res->fedID == ofed) ? res->dependency : false;
}

bool TimeDependencies::isDependent(global_federate_id ofed) const
{
    auto res = std::lower_bound(dependencies.begin(), dependencies.end(), ofed, dependencyCompare);
    if (res == dependencies.end()) {
        return false;
    }
    return (res->fedID == ofed) ? res->dependent : false;
}

const DependencyInfo* TimeDependencies::getDependencyInfo(global_federate_id id) const
{
    auto res = std::lower_bound(dependencies.cbegin(), dependencies.cend(), id, dependencyCompare);
    if ((res == dependencies.cend()) || (res->fedID != id)) {
        return nullptr;
    }

    return &(*res);
}

DependencyInfo* TimeDependencies::getDependencyInfo(global_federate_id id)
{
    auto res = std::lower_bound(dependencies.begin(), dependencies.end(), id, dependencyCompare);
    if ((res == dependencies.end()) || (res->fedID != id)) {
        return nullptr;
    }

    return &(*res);
}

bool TimeDependencies::addDependency(global_federate_id id)

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

void TimeDependencies::removeDependency(global_federate_id id)
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

bool TimeDependencies::addDependent(global_federate_id id)

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

void TimeDependencies::removeDependent(global_federate_id id)
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

void TimeDependencies::removeInterdependence(global_federate_id id)
{
    auto dep = std::lower_bound(dependencies.begin(), dependencies.end(), id, dependencyCompare);
    if (dep != dependencies.end()) {
        if (dep->fedID == id) {
            dependencies.erase(dep);
        }
    }
}

bool TimeDependencies::updateTime(const ActionMessage& m)
{
    auto dependency_id = (m.action() != CMD_SEND_MESSAGE) ? m.source_id : m.dest_id;

    auto depInfo = getDependencyInfo(global_federate_id(dependency_id));
    if (depInfo == nullptr || !depInfo->dependency) {
        return false;
    }
    return ProcessMessage(m, *depInfo);
}

bool TimeDependencies::checkIfReadyForExecEntry(bool iterating) const
{
    if (iterating) {
        return std::none_of(dependencies.begin(), dependencies.end(), [](const auto& dep) {
            return (dep.dependency && dep.time_state == time_state_t::initialized);
        });
    }
    return std::none_of(dependencies.begin(), dependencies.end(), [](const auto& dep) {
        return (dep.dependency && dep.time_state < time_state_t::exec_requested);
    });
}

bool TimeDependencies::hasActiveTimeDependencies() const
{
    return std::any_of(dependencies.begin(), dependencies.end(), [](const auto& dep) {
        return (dep.dependency && (dep.fedID.isFederate()) && (dep.next < Time::maxVal()));
    });
}

int TimeDependencies::activeDependencyCount() const
{
    return std::count_if(dependencies.begin(), dependencies.end(), [](const auto& dep) {
        return (dep.dependency && (dep.fedID.isFederate()) && (dep.next < Time::maxVal()));
    });
}
/** get a count of the active dependencies*/
global_federate_id TimeDependencies::getMinDependency() const {
    global_federate_id minID;
    Time minTime(Time::maxVal());
    for (auto dep : dependencies)
    {
        if (dep.dependency && (dep.fedID.isFederate()) && (dep.next < Time::maxVal()))
        {
            if (dep.next < minTime)
            {
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
        if (dep.dependency && dep.time_state == time_state_t::exec_requested_iterative) {
            dep.time_state = time_state_t::initialized;
        }
    }
}

bool TimeDependencies::checkIfReadyForTimeGrant(bool iterating, Time desiredGrantTime) const
{
    if (iterating) {
        for (auto& dep : dependencies) {
            if (!dep.dependency) {
                continue;
            }
            if (dep.next < desiredGrantTime) {
                return false;
            }
            if ((dep.next == desiredGrantTime) && (dep.time_state == time_state_t::time_granted)) {
                return false;
            }
        }
    } else {
        for (auto& dep : dependencies) {
            if (!dep.dependency) {
                continue;
            }
            if (dep.next < desiredGrantTime) {
                return false;
            }
            if (dep.next == desiredGrantTime) {
                if (dep.time_state == time_state_t::time_granted) {
                    return false;
                }
            }
        }
    }
    return true;
}

void TimeDependencies::resetIteratingTimeRequests(helics::Time requestTime)
{
    for (auto& dep : dependencies) {
        if (dep.dependency && dep.time_state == time_state_t::time_requested_iterative) {
            if (dep.next == requestTime) {
                dep.time_state = time_state_t::time_granted;
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

static void generateMinTimeImplementation(TimeData& mTime,
                                          const DependencyInfo& dep,
                                          global_federate_id ignore)
{
    if (dep.fedID == ignore) {
        if (dep.fedID.isBroker()) {
            if (dep.Te < mTime.minDe) {
                mTime.minDe = dep.Te;
            }
            if (mTime.minDe < mTime.minminDe) {
                mTime.minminDe = mTime.minDe;
            }
        }

        return;
    }
    if (dep.connection != ConnectionType::self) {
    if (dep.minDe >= dep.next) {
        if (dep.minDe < mTime.minminDe) {
            mTime.minminDe = dep.minDe;
            mTime.minFed = dep.fedID;
            if (dep.minFed.isValid()) {
                mTime.minFedActual = dep.minFed;
            } else {
                mTime.minFed = dep.fedID;
            }
        } else if (dep.minDe == mTime.minminDe) {
            mTime.minFedActual = global_federate_id();
        }
    } else {
        // this minimum dependent event time received was invalid and can't be trusted
        // therefore it can't be used to determine a time grant
        mTime.minminDe = -1;
    }
    }
    if (dep.next < mTime.next) {
        mTime.next = dep.next;
        mTime.time_state = dep.time_state;

    } else if (dep.next == mTime.next) {
        if (dep.time_state == time_state_t::time_granted) {
            mTime.time_state = dep.time_state;
        }
    }
    if (dep.connection != ConnectionType::self)
    {
        if (dep.Te < mTime.Te) {
            mTime.Te = dep.Te;
        }
        if (dep.minDe < mTime.minDe) {
            mTime.minDe = dep.minDe;
        }
    }
    
}

TimeData generateMinTimeUpstream(const TimeDependencies& dependencies,
                                       bool restricted,
                                       global_federate_id self,
                                       global_federate_id ignore)
{
    TimeData mTime(Time::maxVal());
    for (auto& dep : dependencies) {
        if (!dep.dependency) {
            continue;
        }
        if (dep.connection == ConnectionType::parent) {
            continue;
        }
        if (self.isValid() && dep.minFedActual == self) {
            continue;
        }
        generateMinTimeImplementation(mTime, dep, ignore);
    }
    if (mTime.Te < mTime.minDe) {
        mTime.minDe = mTime.Te;
    }
    mTime.minminDe = std::min(mTime.minDe, mTime.minminDe);

    if (!restricted ) {
        if (mTime.minminDe > mTime.next) {
            mTime.next = mTime.minminDe;
        }
    }

    return mTime;
}

TimeData generateMinTimeDownstream(const TimeDependencies& dependencies,
                                         bool restricted,
                                         global_federate_id self,
                                         global_federate_id ignore)
{
    TimeData mTime(Time::maxVal());
    for (auto& dep : dependencies) {
        if (dep.dependency == false) {
            continue;
        }
        if (dep.connection == ConnectionType::parent) {
            continue;
        }
        if (self.isValid() && dep.minFedActual == self) {
            continue;
        }
        generateMinTimeImplementation(mTime, dep,ignore);
    }
    if (mTime.Te < mTime.minDe) {
        mTime.minDe = mTime.Te;
    }
    mTime.minminDe = std::min(mTime.minDe, mTime.minminDe);

    if (!restricted ) {
        if (mTime.minminDe > mTime.next) {
            mTime.next = mTime.minminDe;
        }
    }

    return mTime;
}

TimeData generateMinTimeTotal(const TimeDependencies& dependencies,
                                    bool restricted,
                                    global_federate_id self,
                                    global_federate_id ignore)
{
    TimeData mTime(Time::maxVal());
    for (auto& dep : dependencies) {
        if (!dep.dependency) {
            continue;
        }

        if (self.isValid() && dep.minFedActual == self) {
            continue;
        }
        generateMinTimeImplementation(mTime, dep, ignore);
    }

    if (mTime.Te < mTime.minDe) {
        mTime.minDe = mTime.Te;
    }
    mTime.minminDe = std::min(mTime.minDe, mTime.minminDe);

    if (!restricted ) {
        if (mTime.minminDe > mTime.next) {
            mTime.next = mTime.minminDe;
        }
    }

    return mTime;
}
}  // namespace helics
