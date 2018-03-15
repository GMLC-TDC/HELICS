/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "TimeDependencies.hpp"
#include "ActionMessage.hpp"
#include <algorithm>
#include <cassert>

namespace helics
{
bool DependencyInfo::ProcessMessage (const ActionMessage &m)
{
    switch (m.action ())
    {
    case CMD_EXEC_REQUEST:
        time_state = checkActionFlag (m, iterationRequested) ? time_state_t::exec_requested_iterative :
                                                                 time_state_t::exec_requested;
        break;
    case CMD_EXEC_GRANT:
        if (!checkActionFlag (m, iterationRequested))
        {
            time_state = time_state_t::time_granted;
            Tnext = timeZero;
            Tdemin = timeZero;
            Te = timeZero;
        }
        else
        {
            time_state = time_state_t::initialized;
        }
        break;
    case CMD_TIME_REQUEST:
        time_state = checkActionFlag (m, iterationRequested) ? time_state_t::time_requested_iterative :
                                                                 time_state_t::time_requested;
        //   printf("%d Request from %d time %f, te=%f, Tdemin=%f\n", fedID, m.source_id,
        //   static_cast<double>(m.actionTime), static_cast<double>(m.Te), static_cast<double>(m.Tdemin));
        //   assert(m.actionTime >= Tnext);
        Tnext = m.actionTime;
        Te = m.Te;
        Tdemin = m.Tdemin;
        if (forwardEvent < Te)
        {
            Te = forwardEvent;
        }
        if (Te < Tdemin)
        {
            Tdemin = Te;
        }
        forwardEvent = Time::maxVal();
        minFed = m.source_handle;
        break;
    case CMD_TIME_GRANT:
        time_state = time_state_t::time_granted;
        //    printf("%d Grant from %d time %f\n", fedID, m.source_id, static_cast<double>(m.actionTime));
        //   assert(m.actionTime >= Tnext);
        Tnext = m.actionTime;
        Te = Tnext;
        Tdemin = Tnext;
        minFed = m.source_handle;
        break;
    case CMD_DISCONNECT:
    case CMD_PRIORITY_DISCONNECT:
        time_state = time_state_t::time_granted;
        //   printf("%d disconnect from %d\n", fedID, m.source_id);
        Tnext = Time::maxVal ();
        Te = Time::maxVal ();
        Tdemin = Time::maxVal ();
        minFed = invalid_fed_id;
        break;
    case CMD_SEND_MESSAGE:
        if (time_state == time_state_t::time_granted)
        {
            if (m.actionTime < forwardEvent)
            {
                forwardEvent = m.actionTime;
            }
            return false;
        }
        if (m.actionTime >= Tnext)
        {
            if (m.actionTime < Te)
            {
                Te = std::max(Tnext, m.actionTime);
                if (Te < Tdemin)
                {
                    Tdemin = Te;
                }
                return true;
            }
        }
        else
        {
            if (Tnext < Te)
            {
                Te = Tnext;
                if (Te < Tdemin)
                {
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
static auto dependencyCompare = [](const auto &dep, auto &target) { return (dep.fedID < target); };

bool TimeDependencies::isDependency (Core::federate_id_t ofed) const
{
    auto res = std::lower_bound (dependencies.begin (), dependencies.end (), ofed, dependencyCompare);
    if (res == dependencies.end ())
    {
        return false;
    }
    return (res->fedID == ofed);
}

DependencyInfo *TimeDependencies::getDependencyInfo (Core::federate_id_t ofed)
{
    auto res = std::lower_bound (dependencies.begin (), dependencies.end (), ofed, dependencyCompare);
    if ((res == dependencies.end ()) || (res->fedID != ofed))
    {
        return nullptr;
    }

    return &(*res);
}

bool TimeDependencies::addDependency (Core::federate_id_t id)

{
    if (dependencies.empty ())
    {
        dependencies.emplace_back (id);
        return true;
    }
    auto dep = std::lower_bound (dependencies.begin (), dependencies.end (), id, dependencyCompare);
    if (dep == dependencies.end ())
    {
        dependencies.emplace_back (id);
    }
    else
    {
        if (dep->fedID == id)
        {
            // the dependency is already present
            return false;
        }
        dependencies.emplace (dep, id);
    }
    return true;
}

void TimeDependencies::removeDependency (Core::federate_id_t id)
{
    auto dep = std::lower_bound (dependencies.begin (), dependencies.end (), id, dependencyCompare);
    if (dep != dependencies.end ())
    {
        if (dep->fedID == id)
        {
            dependencies.erase (dep);
        }
    }
}

bool TimeDependencies::updateTime (const ActionMessage &m)
{
    if (m.action() == CMD_SEND_MESSAGE)
    {
        auto depInfo = getDependencyInfo(m.dest_id);
        if (depInfo == nullptr)
        {
            return false;
        }
        return depInfo->ProcessMessage(m);
    }
    else
    {
        auto depInfo = getDependencyInfo(m.source_id);
        if (depInfo == nullptr)
        {
            return false;
        }
        return depInfo->ProcessMessage(m);
    }

}

bool TimeDependencies::checkIfReadyForExecEntry (bool iterating) const
{
    if (iterating)
    {
        for (auto &dep : dependencies)
        {
            if (dep.time_state == DependencyInfo::time_state_t::initialized)
            {
                return false;
            }
        }
    }
    else
    {
        for (auto &dep : dependencies)
        {
            // if exec mode has not been requesting
            if (dep.time_state < DependencyInfo::time_state_t::exec_requested)
            {
                return false;
            }
        }
    }
    return true;
}

void TimeDependencies::resetIteratingExecRequests ()
{
    for (auto &dep : dependencies)
    {
        if (dep.time_state == DependencyInfo::time_state_t::exec_requested_iterative)
        {
            dep.time_state = DependencyInfo::time_state_t::initialized;
        }
    }
}

bool TimeDependencies::checkIfReadyForTimeGrant (bool iterating, Time desiredGrantTime) const
{
    if (iterating)
    {
        for (auto &dep : dependencies)
        {
            if (dep.Tnext < desiredGrantTime)
            {
                return false;
            }
            if ((dep.Tnext == desiredGrantTime) && (dep.time_state == DependencyInfo::time_state_t::time_granted))
            {
                return false;
            }
        }
    }
    else
    {
        for (auto &dep : dependencies)
        {
            if (dep.Tnext < desiredGrantTime)
            {
                return false;
            }
            if (dep.Tnext == desiredGrantTime)
            {
                if (dep.time_state == DependencyInfo::time_state_t::time_granted)
                {
                    return false;
                }
            }
        }
    }
    return true;
}

void TimeDependencies::resetIteratingTimeRequests (helics::Time requestTime)
{
    for (auto &dep : dependencies)
    {
        if (dep.time_state == DependencyInfo::time_state_t::time_requested_iterative)
        {
            if (dep.Tnext == requestTime)
            {
                dep.time_state = DependencyInfo::time_state_t::time_granted;
                dep.Te = requestTime;
                dep.Tdemin = requestTime;
            }
        }
    }
}

}  // namespace helics

