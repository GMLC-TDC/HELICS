/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

/***********************************************************************
 *
 * Copyright (c) 2015-2017 Ansel Sermersheim
 * All rights reserved.
 *
 * This file is part of libguarded
 *
 * libguarded is free software, released under the BSD 2-Clause license.
 * For license details refer to LICENSE provided with this project.
 *
 ***********************************************************************/

#ifndef STAGED_GUARDED_HPP
#define STAGED_GUARDED_HPP

#include <atomic>
#include <memory>
#include <mutex>
#include <type_traits>
#include "handles.hpp"

namespace libguarded
{
/**
   \headerfile staged_guarded.hpp <libguarded/staged_guarded.hpp>

   This templated class wraps an object and allows only one thread at a
   time to access the protected object until it is locked as a constant after which the structure is const

   This class will use std::mutex for the internal locking mechanism by
   default. Other classes which are useful for the mutex type are
   std::recursive_mutex, std::timed_mutex, and
   std::recursive_timed_mutex.

   The handle returned by the various lock methods is moveable but not
   copyable.
*/
template <typename T, typename M = std::mutex>
class staged_guarded
{
public:
    /**
     Construct a guarded object. This constructor will accept any
     number of parameters, all of which are forwarded to the
     constructor of T.
    */
    template <typename... Us>
    staged_guarded (Us &&... data);

    /**
     Acquire a handle to the protected object. As a side effect, the
     protected object will be locked from access by any other
     thread. The lock will be automatically released when the handle
     is destroyed.
    */
    handle lock ();
    shared_handle lock() const;

    /**
     Attempt to acquire a handle to the protected object. Returns a
     null handle if the object is already locked. As a side effect,
     the protected object will be locked from access by any other
     thread. The lock will be automatically released when the handle
     is destroyed.
    */
    handle try_lock ();

    /**
     Attempt to acquire a handle to the protected object. As a side
     effect, the protected object will be locked from access by any
     other thread. The lock will be automatically released when the
     handle is destroyed.

     Returns a null handle if the object is already locked, and does
     not become available for locking before the time duration has
     elapsed.

     Calling this method requires that the underlying mutex type M
     supports the try_lock_for method.  This is not true if M is the
     default std::mutex.
    */
    template <class Duration>
    handle try_lock_for (const Duration &duration);

    /**
     Attempt to acquire a handle to the protected object.  As a side
     effect, the protected object will be locked from access by any other
     thread. The lock will be automatically released when the handle is
     destroyed.

     Returns a null handle if the object is already locked, and does not
     become available for locking before reaching the specified timepoint.

     Calling this method requires that the underlying mutex type M
     supports the try_lock_until method.  This is not true if M is the
     default std::mutex.
    */
    template <class TimePoint>
    handle try_lock_until (const TimePoint &timepoint);

    // shared access, note "shared" in method names
    shared_handle lock_shared() const;
    shared_handle try_lock_shared() const;

    template <class Duration>
    shared_handle try_lock_shared_for(const Duration & duration) const;

    template <class TimePoint>
    shared_handle try_lock_shared_until(const TimePoint & timepoint) const;

    void transition ()
    {
        if (!constant)
        {
            //acquire the lock then alter the constant variable
            std::lock_guard<M> lock(m_mutex);
            constant = true;
        }
       
    }

  private:
    T m_obj;
    M m_mutex;
    std::atomic<bool> constant{false};
};

template <typename T, typename M>
template <typename... Us>
staged_guarded<T, M>::staged_guarded (Us &&... data) : m_obj (std::forward<Us> (data)...)
{
}

template <typename T, typename M>
auto staged_guarded<T, M>::lock () -> handle
{
    std::unique_lock<M> lock =
      (constant) ? std::unique_lock<M> (m_mutex, std::defer_lock) : std::unique_lock<M> (m_mutex);
    return handle (&m_obj, std::move (lock));
}

template <typename T, typename M>
auto staged_guarded<T, M>::lock() const -> shared_handle
{
    std::unique_lock<M> lock =
        (constant) ? std::unique_lock<M>(m_mutex, std::defer_lock) : std::unique_lock<M>(m_mutex);
    return shared_handle(&m_obj, std::move(lock));
}

template <typename T, typename M>
auto staged_guarded<T, M>::lock_shared() const -> shared_handle
{
    using locktype = typename shared_lock_handle<T, M>::locker_type;
     auto lock =(constant) ? locktype(m_mutex, std::defer_lock) : locktype(m_mutex);
    return shared_handle(&m_obj,std::move(lock));
}

template <typename T, typename M>
auto staged_guarded<T, M>::try_lock () -> handle
{
    if (!constant)
    {
        try_lock_handle(&m_obj, m_mutex);
    }
    else
    {
        return handle (&m_obj,std::unique_lock<M> (m_mutex, std::defer_lock));
    }
}


template <typename T, typename M>
auto staged_guarded<T, M>::try_lock_shared() const -> shared_handle
{
    if (!constant)
    {
        try_lock_handle_shared(&m_obj, m_mutex);
    }
    else
    {
        using locktype = typename shared_lock_handle<T, M>::locker_type;
        return handle(&m_obj, locktype(m_mutex, std::defer_lock)));
    }
}

template <typename T, typename M>
template <typename Duration>
auto staged_guarded<T, M>::try_lock_for (const Duration &d) -> handle
{
    if (!constant)
    {
        try_lock_handle_for(&m_obj, m_mutex, d);
    }
    else
    {
        return handle (&m_obj, std::unique_lock<M> (m_mutex, std::defer_lock));
    }
}

template <typename T, typename M>
template <typename TimePoint>
auto staged_guarded<T, M>::try_lock_until (const TimePoint &tp) -> handle
{
    if (!constant)
    {
        try_lock_handle_until(&m_obj, m_mutex, tp);
    }
    else
    {
        return handle (&m_obj, std::unique_lock<M> (m_mutex, std::defer_lock));
    }
}

template <typename T, typename M>
template <typename Duration>
auto staged_guarded<T, M>::try_lock_shared_for(const Duration & d) const -> shared_handle
{
    if (!constant)
    {
        try_lock_shared_handle_for(&m_obj, m_mutex, d);
    }
    else
    {
        using locktype = typename shared_lock_handle<T, M>::locker_type;
        return handle(&m_obj, locktype(m_mutex, std::defer_lock)));
    }
}

template <typename T, typename M>
template <typename TimePoint>
auto staged_guarded<T, M>::try_lock_shared_until(const TimePoint & tp) const -> shared_handle
{
    if (!constant)
    {
        try_lock_shared_handle_until(&m_obj, m_mutex, tp);
    }
    else
    {
        using locktype = typename shared_lock_handle<T, M>::locker_type;
        return handle(&m_obj, locktype(m_mutex, std::defer_lock)));
    }
}

}

#endif /*STAGED_GUARDED_HPP*/
