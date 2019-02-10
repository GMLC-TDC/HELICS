/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include <atomic>

#include <chrono>
#include <condition_variable>
#include <mutex>

class TriggerVariable
{
  public:
    explicit TriggerVariable (bool active = false) : triggered (false), activated (active){};
    /** activate the trigger to the ready state
    @return true if the Trigger was activated false if it was already active
    */
    bool activate ();
    /** trigger the variable
    @return true if the trigger was successful, false if the trigger has not been activated yet*/
    bool trigger ();
    /** check if the variable has been triggered*/
    bool isTriggered () const { return triggered.load (std::memory_order_acquire); }
    /** wait for the variable to trigger*/
    void wait () const;
    /** wait for a period of time for the value to trigger*/
    bool wait_for (const std::chrono::milliseconds &duration) const;
    /** wait on the Trigger becoming active*/
    void waitActivation () const;
    /** wait for a period of time for the value to trigger*/
    bool wait_forActivation (const std::chrono::milliseconds &duration) const;
    /** reset the trigger Variable to the inactive state*/
    void reset ();
    /** check if the variable is active*/
    bool isActive () const;

  private:
    std::atomic<bool> triggered;  //!< the state of the trigger
    bool activated;  //!< variable controlling if the trigger has been activated
    mutable std::mutex stateLock;  //!< mutex protecting the trigger
    mutable std::condition_variable cv_trigger;  //!< semaphore for the trigger
    mutable std::condition_variable cv_active;  //!< semaphore for the activation
};
