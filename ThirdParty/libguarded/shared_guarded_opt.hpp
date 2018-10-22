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
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
/*
this entire file was added to the library to meet a need of a shared type where the locking could be disabled at construction
*/

#ifndef LIBGUARDED_SHARED_GUARDED_OPT_HPP
#define LIBGUARDED_SHARED_GUARDED_OPT_HPP

#include <memory>
#include <type_traits>

#include <shared_mutex>

#include "handles.hpp"
namespace libguarded
{

/**
   \headerfile shared_guarded.hpp <libguarded/shared_guarded.hpp>

   This templated class wraps an object and allows only one thread at
   a time to modify the protected object.

   This class will use std::shared_timed_mutex for the internal
   locking mechanism by default. In C++17 the std::shared_mutex class
   is also available.

   The handle returned by the various lock methods is moveable but not
   copyable.
*/
#ifdef LIBGUARDED_NO_DEFAULT
template <typename T, typename M>
class shared_guarded
#else
template <typename T, typename M = std::shared_timed_mutex>
class shared_guarded_opt
#endif
{
private:
    using handle = lock_handle<T, M>;
    using shared_handle = shared_lock_handle<T, M>;
public:
    explicit shared_guarded_opt (bool enableLocking = true)
      : enabled (enableLocking){}
    template <typename... Us>
    shared_guarded_opt(bool enableLocking, Us &&... data);

    // exclusive access
    handle lock();
    shared_handle lock() const;
    handle try_lock();

    template <class Duration>
    handle try_lock_for(const Duration & duration);

    template <class TimePoint>
    handle try_lock_until(const TimePoint & timepoint);

    // shared access, note "shared" in method names
    shared_handle lock_shared() const;
    shared_handle try_lock_shared() const;

    template <class Duration>
    shared_handle try_lock_shared_for(const Duration & duration) const;

    template <class TimePoint>
    shared_handle try_lock_shared_until(const TimePoint & timepoint) const;

private:
    T         m_obj;
    mutable M m_mutex;
    const bool enabled{true};
};

template <typename T, typename M>
template <typename... Us>
shared_guarded_opt<T, M>::shared_guarded_opt (bool enableLocking, Us &&... data)
    : m_obj (std::forward<Us> (data)...), enabled (enableLocking)
{
}

template <typename T, typename M>
auto shared_guarded_opt<T, M>::lock() ->handle
{
    return (enabled) ? handle (&m_obj, m_mutex) : handle (&m_obj, std::unique_lock<M> ());
}

template <typename T, typename M>
auto shared_guarded_opt<T, M>::lock() const ->shared_handle
{
    return (enabled) ? shared_handle (&m_obj, m_mutex) : shared_handle (&m_obj, shared_locker<M>::locker_type ());
}

template <typename T, typename M>
auto shared_guarded_opt<T, M>::try_lock() ->handle
{
    return (enabled) ? try_lock_handle (&m_obj, m_mutex):handle (&m_obj, std::unique_lock<M> ());
}

template <typename T, typename M>
template <typename Duration>
auto shared_guarded_opt<T, M>::try_lock_for(const Duration & duration) ->handle
{
    return (enabled) ? try_lock_handle_for (&m_obj, m_mutex, duration) : handle (&m_obj, std::unique_lock<M> ());
}

template <typename T, typename M>
template <typename TimePoint>
auto shared_guarded_opt<T, M>::try_lock_until(const TimePoint & timepoint) ->handle
{
    return (enabled) ? try_lock_handle_until (&m_obj, m_mutex, timepoint) :
                       handle (&m_obj, std::unique_lock<M> ());
}

template <typename T, typename M>
auto shared_guarded_opt<T, M>::lock_shared() const ->shared_handle
{
    return (enabled) ? shared_handle (&m_obj, m_mutex) : shared_handle (&m_obj, shared_locker<M>::locker_type ());
}

template <typename T, typename M>
auto shared_guarded_opt<T, M>::try_lock_shared() const ->shared_handle
{
    return (enabled) ? try_lock_shared_handle (&m_obj, m_mutex) :
                       shared_handle (&m_obj, shared_locker<M>::locker_type ());
}

template <typename T, typename M>
template <typename Duration>
auto shared_guarded_opt<T, M>::try_lock_shared_for(const Duration & d) const ->shared_handle
{
    return (enabled) ? try_lock_shared_handle_for (&m_obj, m_mutex, d) :
                       shared_handle (&m_obj, shared_locker<M>::locker_type ());
}

template <typename T, typename M>
template <typename TimePoint>
auto shared_guarded_opt<T, M>::try_lock_shared_until(const TimePoint & tp) const -> shared_handle
{
    return (enabled) ? try_lock_shared_handle_until (&m_obj, m_mutex, tp) :
                       shared_handle (&m_obj, shared_locker<M>::locker_type ());
}
} //namespace libguarded
#endif
