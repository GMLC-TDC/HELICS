/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "TriggerVariable.hpp"

bool TriggerVariable::activate ()
{
    std::lock_guard<std::mutex> lock (stateLock);
    if (activated)
    {
        // we are already activated so this function did nothing so return false
        return false;
    }
    activated = true;
    cv_active.notify_all ();
    return true;
}

bool TriggerVariable::trigger ()
{
    std::lock_guard<std::mutex> lock (stateLock);
    if (activated)
    {
        triggered.store (true, std::memory_order_release);
        cv_trigger.notify_all ();
        return true;
    }

    return false;
}

void TriggerVariable::wait () const
{
    std::unique_lock<std::mutex> lk (stateLock);
    if (activated && (!triggered.load (std::memory_order_acquire)))
    {
        cv_trigger.wait (lk, [this] { return triggered.load (std::memory_order_acquire); });
    }
}

bool TriggerVariable::wait_for (const std::chrono::milliseconds &duration) const
{
    std::unique_lock<std::mutex> lk (stateLock);
    if (activated && (!triggered.load (std::memory_order_acquire)))
    {
        return cv_trigger.wait_for (lk, duration, [this] { return triggered.load (std::memory_order_acquire); });
    }
    return true;
}

void TriggerVariable::waitActivation () const
{
    std::unique_lock<std::mutex> lk (stateLock);
    if (!activated)
    {
        cv_active.wait (lk, [this] { return activated; });
    }
}

bool TriggerVariable::wait_forActivation (const std::chrono::milliseconds &duration) const
{
    std::unique_lock<std::mutex> lk (stateLock);
    if (!activated)
    {
        return cv_active.wait_for (lk, duration, [this] { return activated; });
    }
    return true;
}

void TriggerVariable::reset ()
{
    std::unique_lock<std::mutex> lk (stateLock);
    if (activated)
    {
        while (!triggered.load (std::memory_order_acquire))
        {
            lk.unlock ();
            trigger ();
            lk.lock ();
        }
    }
    activated = false;
}

bool TriggerVariable::isActive () const
{
    std::lock_guard<std::mutex> lock (stateLock);
    return activated;
}
