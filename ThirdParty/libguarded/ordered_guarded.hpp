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
additions include load store operations and use of handles.hpp
*/

#ifndef LIBGUARDED_ORDERED_GUARDED_HPP
#define LIBGUARDED_ORDERED_GUARDED_HPP

#include <memory>
#include <mutex>
#include <type_traits>

#include <shared_mutex>

namespace libguarded
{

/**
   \headerfile ordered_guarded.hpp <libguarded/ordered_guarded.hpp>

   This templated class wraps an object. The protected object may be
   read by any number of threads simultaneously, but only one thread
   may modify the object at a time.

   This class will use std::shared_timed_mutex for the internal
   locking mechanism by default. In C++17, the class std::shared_mutex
   is available as well.

   The handle returned by the various lock methods is moveable but not
   copyable.
*/
#ifdef LIBGUARDED_NO_DEFAULT
template <typename T, typename M>
class ordered_guarded
#else
template <typename T, typename M = std::shared_timed_mutex>
class ordered_guarded
#endif
{
  public:
      using shared_handle = shared_lock_handle<T, M>;

    /**
     Construct a guarded object. This constructor will accept any number
     of parameters, all of which are forwarded to the constructor of T.
    */
    template <typename... Us>
    ordered_guarded(Us &&... data);

    template <typename Func>
    typename std::enable_if<
        std::is_same<decltype(std::declval<Func>()(std::declval<T &>())), void>::value, void>::type
    modify(Func &&func);

    template <typename Func>
    typename std::enable_if<
        !std::is_same<decltype(std::declval<Func>()(std::declval<T &>())), void>::value,
        decltype(std::declval<Func>()(std::declval<T &>()))>::type
    modify(Func &&func);

    template <typename Func>
    typename std::enable_if<
        std::is_same<decltype(std::declval<Func>()(std::declval<const T &>())), void>::value,
        void>::type
    read(Func &&func) const;

    template <typename Func>
    typename std::enable_if<
        !std::is_same<decltype(std::declval<Func>()(std::declval<const T &>())), void>::value,
        decltype(std::declval<Func>()(std::declval<const T &>()))>::type
    read(Func &&func) const;

    shared_handle lock_shared() const;
    shared_handle try_lock_shared() const;

    template <class Duration>
    shared_handle try_lock_shared_for(const Duration & duration) const;

    template <class TimePoint>
    shared_handle try_lock_shared_until(const TimePoint & timepoint) const;

	/** generate a copy of the protected object
	*/
	std::enable_if_t<std::is_copy_constructible<T>::value, T> load() const
	{
        auto handle = lock_shared();
        T newObj(*handle);
        return newObj;
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
    T         m_obj;
    mutable M m_mutex;
};

template <typename T, typename M>
template <typename... Us>
ordered_guarded<T, M>::ordered_guarded(Us &&... data) : m_obj(std::forward<Us>(data)...)
{
}

template <typename T, typename M>
template <typename Func>
typename std::enable_if<
    std::is_same<decltype(std::declval<Func>()(std::declval<T &>())), void>::value, void>::type
ordered_guarded<T, M>::modify(Func &&func)
{
    std::lock_guard<M> lock(m_mutex);
    func(m_obj);
}

template <typename T, typename M>
template <typename Func>
typename std::enable_if<
    !std::is_same<decltype(std::declval<Func>()(std::declval<T &>())), void>::value,
    decltype(std::declval<Func>()(std::declval<T &>()))>::type
ordered_guarded<T, M>::modify(Func &&func)
{
    std::lock_guard<M> lock(m_mutex);

    return func(m_obj);
}

template <typename T, typename M>
template <typename Func>
typename std::enable_if<
    std::is_same<decltype(std::declval<Func>()(std::declval<const T &>())), void>::value,
    void>::type
ordered_guarded<T, M>::read(Func &&func) const
{
    typename shared_handle::lock_type glock(m_mutex);
    func(m_obj);
}

template <typename T, typename M>
template <typename Func>
typename std::enable_if<
    !std::is_same<decltype(std::declval<Func>()(std::declval<const T &>())), void>::value,
    decltype(std::declval<Func>()(std::declval<const T &>()))>::type
ordered_guarded<T, M>::read(Func &&func) const
{
    typename shared_handle::lock_type glock(m_mutex);

    return func(m_obj);
}

template <typename T, typename M>
auto ordered_guarded<T, M>::lock_shared() const -> shared_handle
{
    return shared_handle(&m_obj, m_mutex);
}

template <typename T, typename M>
auto ordered_guarded<T, M>::try_lock_shared() const -> shared_handle
{
    return try_lock_shared_handle(&m_obj, m_mutex);
}

template <typename T, typename M>
template <typename Duration>
auto ordered_guarded<T, M>::try_lock_shared_for(const Duration & duration) const -> shared_handle
{
    return try_lock_shared_handle_for(&m_obj, m_mutex, duration);
}

template <typename T, typename M>
template <typename TimePoint>
auto ordered_guarded<T, M>::try_lock_shared_until(const TimePoint & timepoint) const
    -> shared_handle
{
    return try_lock_shared_handle_until(&m_obj, m_mutex, timepoint);
}

}
#endif
