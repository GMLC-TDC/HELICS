/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "TimeDependencies.hpp"

#include "ActionMessage.hpp"
#include "flagOperations.hpp"

#include <algorithm>
#include <cassert>

namespace helics {
static bool ProcessMessage(const ActionMessage& m, DependencyInfo &dep)
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
            dep.minDe= m.Tdemin;

            if (dep.Te < dep.minDe) {
                dep.minDe= dep.Te;
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
            dep.minDe= dep.next;
            dep.minFed = global_federate_id(m.getExtraData());
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
            dep.minDe= Time::maxVal();
            dep.minFed = global_federate_id{};
            break;
        case CMD_LOCAL_ERROR:
        case CMD_GLOBAL_ERROR:
            dep.time_state = time_state_t::error;
            //   printf("%d disconnect from %d\n", fedID, m.source_id);
            dep.next = Time::maxVal();
            dep.Te = Time::maxVal();
            dep.minDe= Time::maxVal();
            dep.minFed = global_federate_id{};
            break;
        default:
            return false;
    }
    return true;
}

// comparison helper lambda for comparing dependencies
static auto dependencyCompare = [](const auto& dep, auto& target) { return (dep.fedID < target); };

bool TimeDependencies::isDependency(global_federate_id ofed) const
{
    auto res = std::lower_bound(dependencies.begin(), dependencies.end(), ofed, dependencyCompare);
    if (res == dependencies.end()) {
        return false;
    }
    return (res->fedID == ofed)?res->dependency:false;
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
        auto it=dependencies.emplace(dep, id);
        it->dependency = true;
    }
    return true;
}

void TimeDependencies::removeDependency(global_federate_id id)
{
    auto dep = std::lower_bound(dependencies.begin(), dependencies.end(), id, dependencyCompare);
    if (dep != dependencies.end()) {
        if (dep->fedID == id) {
            dependencies.erase(dep);
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
        }
    }
}

void TimeDependencies::removeInterdependence(global_federate_id id)
{
    auto dep = std::lower_bound(dependencies.begin(), dependencies.end(), id, dependencyCompare);
    if (dep != dependencies.end()) {
        if (dep->fedID == id) {
            dep->dependent = false;
            dep->dependency = false;
        }
    }
}

bool TimeDependencies::updateTime(const ActionMessage& m)
{
    auto dependency_id = (m.action() != CMD_SEND_MESSAGE) ? m.source_id : m.dest_id;

    auto depInfo = getDependencyInfo(global_federate_id(dependency_id));
    if (depInfo == nullptr||!depInfo->dependency) {
        return false;
    }
    return ProcessMessage(m,*depInfo);
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
        return (dep.dependency &&  (dep.fedID.isFederate()) && (dep.next < Time::maxVal()));
    });
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
            if (!dep.dependency)
            {
                continue;
            }
            if (dep.next < desiredGrantTime) {
                return false;
            }
            if ((dep.next == desiredGrantTime) &&
                (dep.time_state == time_state_t::time_granted)) {
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
        if (dep.dependency)
        {
        dep.Te = (std::max)(dep.next, grantTime);
        dep.minDe = dep.Te;
    }
}
}

}  // namespace helics
