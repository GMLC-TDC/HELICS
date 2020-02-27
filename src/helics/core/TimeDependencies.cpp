/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "TimeDependencies.hpp"

#include "ActionMessage.hpp"
#include "flagOperations.hpp"

#include <algorithm>
#include <cassert>

namespace helics {
bool DependencyInfo::ProcessMessage(const ActionMessage& m)
{
    switch (m.action()) {
        case CMD_EXEC_REQUEST:
            time_state = checkActionFlag(m, iteration_requested_flag) ?
                time_state_t::exec_requested_iterative :
                time_state_t::exec_requested;
            break;
        case CMD_EXEC_GRANT:
            if (!checkActionFlag(m, iteration_requested_flag)) {
                time_state = time_state_t::time_granted;
                Tnext = timeZero;
                Tdemin = timeZero;
                Te = timeZero;
            } else {
                time_state = time_state_t::initialized;
            }
            break;
        case CMD_TIME_REQUEST:
            time_state = checkActionFlag(m, iteration_requested_flag) ?
                time_state_t::time_requested_iterative :
                time_state_t::time_requested;
            //   printf("%d Request from %d time %f, te=%f, Tdemin=%f\n", fedID, m.source_id,
            //   static_cast<double>(m.actionTime), static_cast<double>(m.Te), static_cast<double>(m.Tdemin));
            //   assert(m.actionTime >= Tnext);
            Tnext = m.actionTime;
            Te = m.Te;
            Tdemin = m.Tdemin;
            if (forwardEvent < Te) {
                Te = forwardEvent;
            }
            if (Te < Tdemin) {
                Tdemin = Te;
            }
            forwardEvent = Time::maxVal();
            minFed = global_federate_id(m.source_handle.baseValue());
            break;
        case CMD_TIME_GRANT:
            time_state = time_state_t::time_granted;
            //    printf("%d Grant from %d time %f\n", fedID, m.source_id, static_cast<double>(m.actionTime));
            //   assert(m.actionTime >= Tnext);
            Tnext = m.actionTime;
            Te = Tnext;
            Tdemin = Tnext;
            minFed = global_federate_id(m.source_handle.baseValue());
            break;
        case CMD_DISCONNECT:
        case CMD_PRIORITY_DISCONNECT:
        case CMD_BROADCAST_DISCONNECT:
        case CMD_DISCONNECT_CORE:
        case CMD_DISCONNECT_BROKER:
        case CMD_DISCONNECT_FED:
            time_state = time_state_t::time_granted;
            //   printf("%d disconnect from %d\n", fedID, m.source_id);
            Tnext = Time::maxVal();
            Te = Time::maxVal();
            Tdemin = Time::maxVal();
            minFed = global_federate_id{};
            break;
        case CMD_LOCAL_ERROR:
        case CMD_GLOBAL_ERROR:
            time_state = time_state_t::error;
            //   printf("%d disconnect from %d\n", fedID, m.source_id);
            Tnext = Time::maxVal();
            Te = Time::maxVal();
            Tdemin = Time::maxVal();
            minFed = global_federate_id{};
            break;
        case CMD_SEND_MESSAGE:
            if (time_state == time_state_t::time_granted) {
                if (m.actionTime < forwardEvent) {
                    forwardEvent = m.actionTime;
                }
                return false;
            }
            if (m.actionTime >= Tnext) {
                if (m.actionTime < Te) {
                    Te = std::max(Tnext, m.actionTime);
                    if (Te < Tdemin) {
                        Tdemin = Te;
                    }
                    return true;
                }
            } else {
                if (Tnext < Te) {
                    Te = Tnext;
                    if (Te < Tdemin) {
                        Tdemin = Te;
                    }
                    return true;
                }
            }

            return false;
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
    return (res->fedID == ofed);
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
        return true;
    }
    auto dep = std::lower_bound(dependencies.begin(), dependencies.end(), id, dependencyCompare);
    if (dep == dependencies.end()) {
        dependencies.emplace_back(id);
    } else {
        if (dep->fedID == id) {
            // the dependency is already present
            return false;
        }
        dependencies.emplace(dep, id);
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

bool TimeDependencies::updateTime(const ActionMessage& m)
{
    auto dependency_id = (m.action() != CMD_SEND_MESSAGE) ? m.source_id : m.dest_id;

    auto depInfo = getDependencyInfo(global_federate_id(dependency_id));
    if (depInfo == nullptr) {
        return false;
    }
    return depInfo->ProcessMessage(m);
}

bool TimeDependencies::checkIfReadyForExecEntry(bool iterating) const
{
    if (iterating) {
        return std::none_of(dependencies.begin(), dependencies.end(), [](const auto& dep) {
            return (dep.time_state == DependencyInfo::time_state_t::initialized);
        });
    }
    return std::none_of(dependencies.begin(), dependencies.end(), [](const auto& dep) {
        return (dep.time_state < DependencyInfo::time_state_t::exec_requested);
    });
}

bool TimeDependencies::hasActiveTimeDependencies() const
{
    return std::any_of(dependencies.begin(), dependencies.end(), [](const auto& dep) {
        return ((dep.fedID.isFederate()) && (dep.Tnext < Time::maxVal()));
    });
}

void TimeDependencies::resetIteratingExecRequests()
{
    for (auto& dep : dependencies) {
        if (dep.time_state == DependencyInfo::time_state_t::exec_requested_iterative) {
            dep.time_state = DependencyInfo::time_state_t::initialized;
        }
    }
}

bool TimeDependencies::checkIfReadyForTimeGrant(bool iterating, Time desiredGrantTime) const
{
    if (iterating) {
        for (auto& dep : dependencies) {
            if (dep.Tnext < desiredGrantTime) {
                return false;
            }
            if ((dep.Tnext == desiredGrantTime) &&
                (dep.time_state == DependencyInfo::time_state_t::time_granted)) {
                return false;
            }
        }
    } else {
        for (auto& dep : dependencies) {
            if (dep.Tnext < desiredGrantTime) {
                return false;
            }
            if (dep.Tnext == desiredGrantTime) {
                if (dep.time_state == DependencyInfo::time_state_t::time_granted) {
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
        if (dep.time_state == DependencyInfo::time_state_t::time_requested_iterative) {
            if (dep.Tnext == requestTime) {
                dep.time_state = DependencyInfo::time_state_t::time_granted;
                dep.Te = requestTime;
                dep.Tdemin = requestTime;
            }
        }
    }
}

void TimeDependencies::resetDependentEvents(helics::Time grantTime)
{
    for (auto& dep : dependencies) {
        dep.Te = (std::max)(dep.Tnext, grantTime);
        dep.Tdemin = dep.Te;
    }
}

} // namespace helics
