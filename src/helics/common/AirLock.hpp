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
    {
        if (!loaded)
        {
            std::lock_guard<std::mutex> lock (door);
            if (!loaded)
            {
                data = std::forward<Z> (val);
                loaded = true;
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
        if (!loaded)
        {
            data = std::forward<Z> (val);
            loaded = true;
        }
        else
        {
            while (loaded)
            {
                condition.wait (lock);
            }
            data = std::forward<Z> (val);
            loaded = true;
        }
    }

    /** unload the airlock,
    @return the value is returned in an optional which needs to be checked if it contains a value
    */
    stx::optional<T> try_unload ()
    {
        if (loaded)
        {
            std::lock_guard<std::mutex> lock (door);
            if (loaded)
            {
                stx::optional<T> val{std::move (data)};
                loaded = false;
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
    bool isLoaded () const { return loaded; }

  private:
    std::atomic_bool loaded{false};  //!< flag if the airlock is loaded with cargo
    std::mutex door;  //!< check if one of the doors to the airlock is open
    T data;  //!< the data to be stored in the airlock
    std::condition_variable condition;  //!< condition variable for notification of new data
};
