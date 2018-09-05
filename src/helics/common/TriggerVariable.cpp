/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "TriggerVariable.hpp"

bool TriggerVariable::activate ()
{
    std::unique_lock<std::mutex> lock (stateLock);
    if (activated)
    {
        // we are already activated so this function did nothing so return false
        return false;
    }
    activated = true;
    lock.unlock ();
    cv_active.notify_all ();
    return true;
}

bool TriggerVariable::trigger ()
{
    std::unique_lock<std::mutex> lock (stateLock);
    if (activated)
    {
        triggered.store (true);
        lock.unlock ();
        cv_trigger.notify_all ();
        return true;
    }

    return false;
}

void TriggerVariable::wait () const
{
    std::unique_lock<std::mutex> lk (stateLock);
    if (activated && (!triggered.load ()))
    {
        cv_trigger.wait (lk, [this] { return triggered.load (); });
    }
}

bool TriggerVariable::wait_for (const std::chrono::milliseconds &duration) const
{
    std::unique_lock<std::mutex> lk (stateLock);
    if (activated && (!triggered.load ()))
    {
        return cv_trigger.wait_for (lk, duration, [this] { return triggered.load (); });
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
        while (!triggered.load ())
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
