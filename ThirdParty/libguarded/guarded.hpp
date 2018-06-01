/***********************************************************************
*
* Copyright (c) 2015-2018 Ansel Sermersheim
* All rights reserved.
*
* This file is part of libguarded
*
* libguarded is free software, released under the BSD 2-Clause license.
* For license details refer to LICENSE provided with this project.
*
***********************************************************************/

/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was modified by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

additions include load store operations and operator= functions
*/
#ifndef LIBGUARDED_GUARDED_HPP
#define LIBGUARDED_GUARDED_HPP

#include <memory>
#include <mutex>
#include <type_traits>
#include "handles.hpp"

namespace libguarded
{

/**
   \headerfile guarded.hpp <libguarded/guarded.hpp>

   This templated class wraps an object and allows only one thread at a
   time to access the protected object.

   This class will use std::mutex for the internal locking mechanism by
   default. Other classes which are useful for the mutex type are
   std::recursive_mutex, std::timed_mutex, and
   std::recursive_timed_mutex.

   The handle returned by the various lock methods is moveable but not
   copyable.
*/
template <typename T, typename M = std::mutex>
class guarded
{
private:
    using handle = lock_handle<T, M>;
  public:
    /**
     Construct a guarded object. This constructor will accept any
     number of parameters, all of which are forwarded to the
     constructor of T.
    */
    template <typename... Us>
    guarded(Us &&... data);

    /**
     Acquire a handle to the protected object. As a side effect, the
     protected object will be locked from access by any other
     thread. The lock will be automatically released when the handle
     is destroyed.
    */
    handle lock();

    /**
     Attempt to acquire a handle to the protected object. Returns a
     null handle if the object is already locked. As a side effect,
     the protected object will be locked from access by any other
     thread. The lock will be automatically released when the handle
     is destroyed.
    */
    handle try_lock();

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
    handle try_lock_for(const Duration &duration);

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
    handle try_lock_until(const TimePoint &timepoint);

    /** generate a copy of the protected object
    */
    std::conditional_t<std::is_copy_assignable<T>::value,T,void> load()
    {
        std::lock_guard<M> glock(m_mutex);
        auto retObj= std::conditional_t<std::is_copy_assignable<T>::value, T, void>(m_obj);
        return retObj;

    }

    /** store an updated value into the object*/
    template <typename objType>
     void store(objType &&newObj)
    { //uses a forwarding reference
        std::lock_guard<M> glock(m_mutex);
        m_obj = std::forward<objType>(newObj);
    }

    /** store an updated value into the object*/
    template <typename objType>
    void operator=(objType &&newObj)
    { //uses a forwarding reference
        std::lock_guard<M> glock(m_mutex);
        m_obj = std::forward<objType>(newObj);
    }

  private:
    T m_obj;
    M m_mutex;
};

template <typename T, typename M>
template <typename... Us>
guarded<T, M>::guarded(Us &&... data) : m_obj(std::forward<Us>(data)...)
{
}

template <typename T, typename M>
auto guarded<T, M>::lock() -> handle
{
    return handle(&m_obj, m_mutex);
}

template <typename T, typename M>
auto guarded<T, M>::try_lock() -> handle
{
    std::unique_lock<M> glock(m_mutex, std::try_to_lock);

    if (glock.owns_lock()) {
        return handle(&m_obj, std::move(glock));
    } else {
        return handle(nullptr, std::move(glock));
    }
}

template <typename T, typename M>
template <typename Duration>
auto guarded<T, M>::try_lock_for(const Duration &d) ->handle
{
    std::unique_lock<M> glock(m_mutex, d);

    if (glock.owns_lock()) {
        return handle(&m_obj, std::move(glock));
    } else {
        return handle(nullptr, std::move(glock));
    }
}

template <typename T, typename M>
template <typename TimePoint>
auto guarded<T, M>::try_lock_until(const TimePoint &tp) ->handle
{
    std::unique_lock<M> glock(m_mutex, tp);

    if (glock.owns_lock()) {
        return handle(&m_obj, std::move(glock));
    } else {
        return handle(nullptr, std::move(glock));
    }
}

}

#endif
