/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

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
private:
    class deleter;
    class shared_deleter;

public:
    using handle = std::unique_ptr<T, deleter>;
    using shared_handle = std::unique_ptr<const T, shared_deleter>;

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

    void transition ()
    {
        if (!constant.load(std::memory_order_acquire)
        {
            //acquire the lock then alter the 
            std::lock_guard<M> lock(m_mutex);
            if (!constant.load(std::memory_order_relaxed))
            {
                m_objConst = std::move(m_obj);
                constant.store(true, std::memory_order_release);
            }
        }
    }

  private:
    class deleter
    {
      public:
        using pointer = T *;

        deleter (std::unique_lock<M> lock) : m_lock (std::move (lock)) {}

        void operator() (T *ptr)
        {
            if (m_lock.owns_lock ())
            {
                m_lock.unlock ();
            }
        }

      private:
        std::unique_lock<M> m_lock;
    };

    class shared_deleter
    {
    public:
        using pointer = const T *;

        shared_deleter(M & mutex) : m_deleter_mutex(mutex)
        {
        }

        void operator()(const T * ptr)
        {
            if (ptr) {
                m_deleter_mutex.unlock_shared();
            }
        }

    private:
        M & m_deleter_mutex;
    };

    T m_obj;
    T m_objConst;
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
    if (constant.load(std::memory_order_acquire))
    {
        throw(std::exception("data has transitioned"));
    }
    else
    {
        auto lock = std::unique_lock<M>(m_mutex);
        return handle(&m_obj, deleter(std::move(lock)));
    }
   
   
}

template <typename T, typename M>
auto staged_guarded<T, M>::lock() const -> shared_handle
{
    return lock_shared();
   
}

template <typename T, typename M>
auto staged_guarded<T, M>::lock_shared() const -> shared_handle
{
    if (constant.load(std::memory_order_acquire))
    {
        return shared_handle(&m_objConst, deleter(std::unique_lock<M>(m_mutex, std::defer_lock)));
    }
    else
    {
        return shared_handle(&m_obj, deleter(std::unique_lock<M>(m_mutex)));
    }
}

template <typename T, typename M>
auto staged_guarded<T, M>::try_lock () -> handle
{
    if (!constant.load(std::memory_order_acquire))
    {
        std::unique_lock<M> lock (m_mutex, std::try_to_lock);

        if (lock.owns_lock ())
        {
            return handle (&m_obj, deleter (std::move (lock)));
        }
        else
        {
            return handle (nullptr, deleter (std::move (lock)));
        }
    }
    else
    {
        return handle (&m_objConst, deleter (std::unique_lock<M> (m_mutex, std::defer_lock)));
    }
}


template <typename T, typename M>
auto staged_guarded<T, M>::try_lock_shared() const -> shared_handle
{
    if (!constant.load(std::memory_order_acquire))
    {
        std::unique_lock<M> lock(m_mutex, std::try_to_lock);

        if (lock.owns_lock())
        {
            return handle(&m_obj, deleter(std::move(lock)));
        }
        else
        {
            return handle(nullptr, deleter(std::move(lock)));
        }
    }
    else
    {
        return handle(&m_objConst, deleter(std::unique_lock<M>(m_mutex, std::defer_lock)));
    }
}

template <typename T, typename M>
template <typename Duration>
auto staged_guarded<T, M>::try_lock_for (const Duration &d) -> handle
{
    if (!constant.load(std::memory_order_acquire))
    {
        std::unique_lock<M> lock (m_mutex, d);

        if (lock.owns_lock ())
        {
            return handle (&m_obj, deleter (std::move (lock)));
        }
        else
        {
            return handle (nullptr, deleter (std::move (lock)));
        }
    }
    else
    {
        return handle (&m_objConst, deleter (std::unique_lock<M> (m_mutex, std::defer_lock)));
    }
}

template <typename T, typename M>
template <typename TimePoint>
auto staged_guarded<T, M>::try_lock_until (const TimePoint &tp) -> handle
{
    if (!constant.load(std::memory_order_acquire))
    {
        std::unique_lock<M> lock (m_mutex, tp);

        if (lock.owns_lock ())
        {
            return handle (&m_obj, deleter (std::move (lock)));
        }
        else
        {
            return handle (nullptr, deleter (std::move (lock)));
        }
    }
    else
    {
        return handle (&m_objConst, deleter (std::unique_lock<M> (m_mutex, std::defer_lock)));
    }
}
}

#endif /*STAGED_GUARDED_HPP*/
