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
additions include overloads for std::mutex and std::timed_mutex
*/

#ifndef LIBGUARDED_SHARED_GUARDED_HPP
#define LIBGUARDED_SHARED_GUARDED_HPP

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
class shared_guarded
#endif
{
private:
    using handle = lock_handle<T, M>;
    using shared_handle = shared_lock_handle<T, M>;
public:
    template <typename... Us>
    shared_guarded(Us &&... data);

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
};

template <typename T, typename M>
template <typename... Us>
shared_guarded<T, M>::shared_guarded(Us &&... data) : m_obj(std::forward<Us>(data)...)
{
}

template <typename T, typename M>
auto shared_guarded<T, M>::lock() ->handle
{
    return handle(&m_obj, m_mutex);
}

template <typename T, typename M>
auto shared_guarded<T, M>::lock() const ->shared_handle
{
    return shared_handle(&m_obj, m_mutex);
}

template <typename T, typename M>
auto shared_guarded<T, M>::try_lock() ->handle
{
    return try_lock_handle(&m_obj, m_mutex);
}

template <typename T, typename M>
template <typename Duration>
auto shared_guarded<T, M>::try_lock_for(const Duration & duration) ->handle
{
    return try_lock_handle_for(&m_obj, m_mutex, duration);
}

template <typename T, typename M>
template <typename TimePoint>
auto shared_guarded<T, M>::try_lock_until(const TimePoint & timepoint) ->handle
{
    return try_lock_handle_until(&m_obj, m_mutex, timepoint);
}

template <typename T, typename M>
auto shared_guarded<T, M>::lock_shared() const ->shared_handle
{
    return shared_handle(&m_obj, m_mutex);
}

template <typename T, typename M>
auto shared_guarded<T, M>::try_lock_shared() const ->shared_handle
{
    return try_lock_shared_handle(&m_obj, m_mutex);
}

template <typename T, typename M>
template <typename Duration>
auto shared_guarded<T, M>::try_lock_shared_for(const Duration & d) const ->shared_handle
{
    return try_lock_shared_handle_for(&m_obj, m_mutex, d);
}

template <typename T, typename M>
template <typename TimePoint>
auto shared_guarded<T, M>::try_lock_shared_until(const TimePoint & tp) const -> shared_handle
{
    return try_lock_shared_handle_until(&m_obj, m_mutex, tp);
}
} //namespace libguarded
#endif
