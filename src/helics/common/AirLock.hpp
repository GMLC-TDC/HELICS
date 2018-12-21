/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "helics_includes/optional.hpp"
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <type_traits>

/** NOTES:: PT Went with unlocking after signaling on the basis of this page
http://www.domaigne.com/blog/computing/condvars-signal-with-mutex-locked-or-not/
will check performance at a later time
*/
/** class implementing a airlock
@details this class is used to transfer an object from a thread safe context to a single thread so it can be
accessed without locks
*/
template <typename T>
class AirLock
{
  public:
    /** default constructor */
    AirLock () = default;
    /** destructor*/
    ~AirLock () = default;
    /** try to load the airlock
    @return true if successful, false if not*/
    template <class Z>
    bool try_load (Z &&val)
    { //all modifications to loaded should be insde the mutex otherwise this will contain race conditions
        if (!loaded.load(std::memory_order_acquire))
        {
            std::lock_guard<std::mutex> lock (door);
            //We can use relaxed here since we are behind the mutex
            if (!loaded.load(std::memory_order_relaxed))
            {
                data = std::forward<Z> (val);
                loaded.store(true, std::memory_order_release);
                return true;
            }
        }
        return false;
    }
    /** load the airlock,
    @details the call will block until the airlock is ready to be loaded
    */
    template <class Z>
    void load (Z &&val)
    {
        std::unique_lock<std::mutex> lock (door);
        if (!loaded.load(std::memory_order_relaxed))
        {
            data = std::forward<Z> (val);
            loaded.store(true, std::memory_order_release);
        }
        else
        {
            while (loaded.load(std::memory_order_acquire))
            {
                condition.wait (lock);
            }
            data = std::forward<Z> (val);
            loaded.store(true, std::memory_order_release);
        }
    }

    /** unload the airlock,
    @return the value is returned in an optional which needs to be checked if it contains a value
    */
    stx::optional<T> try_unload ()
    {
        if (loaded.load(std::memory_order_acquire))
        {
            std::lock_guard<std::mutex> lock (door);
            //can use relaxed since we are behind a mutex
            if (loaded.load(std::memory_order_relaxed))
            {
                stx::optional<T> val{std::move (data)};
                loaded.store(false, std::memory_order_release);
                condition.notify_one ();
                return val;
            }
        }
        return stx::nullopt;
    }
    /** check if the airlock is loaded
    @details this may or may  not mean anything depending on usage
    it is correct but may be incorrect immediately after the call
    */
    bool isLoaded () const { return loaded.load(std::memory_order_acquire); }

  private:
    std::atomic_bool loaded{false};  //!< flag if the airlock is loaded with cargo
    std::mutex door;  //!< check if one of the doors to the airlock is open
    T data;  //!< the data to be stored in the airlock
    std::condition_variable condition;  //!< condition variable for notification of new data
};
