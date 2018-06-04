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

#ifndef LIBGUARDED_DEFERRED_GUARDED_HPP
#define LIBGUARDED_DEFERRED_GUARDED_HPP

#include <libguarded/guarded.hpp>

#include <atomic>
#include <future>
#include <memory>
#include <vector>
#include <shared_mutex>
#include "handles.hpp"

namespace libguarded
{

template <class T>
typename std::add_lvalue_reference<T>::type declref();

/**
   \headerfile deferred_guarded.hpp <libguarded/deferred_guarded.hpp>

   This templated
   class wraps an object and allows only one thread at a time to
   modify the protected object.

   This class will use std::shared_timed_mutex for the internal locking mechanism by
   default. In C++17, the class std::shared_mutex is available as well.

   The shared_handle returned by the various lock methods is moveable
   but not copyable.
*/
#ifdef LIBGUARDED_NO_DEFAULT
template <typename T, typename M>
class deferred_guarded
#else
template <typename T, typename M = std::shared_timed_mutex>
class deferred_guarded
#endif
{
  
  public:
    template <typename... Us>
    deferred_guarded(Us &&... data);

    template <typename Func>
    void modify_detach(Func && func);

    template <typename Func>
    auto modify_async(Func && func) ->
        typename std::future<decltype(std::declval<Func>()(declref<T>()))>;

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
  private:
   
    void do_pending_writes() const;

    mutable T m_obj;
    mutable M m_mutex;

    mutable std::atomic<bool>                                   m_pendingWrites;
    mutable guarded<std::vector<std::packaged_task<void(T &)>>> m_pendingList;
};

template <typename T, typename M>
template <typename... Us>
deferred_guarded<T, M>::deferred_guarded(Us &&... data)
    : m_obj(std::forward<Us>(data)...), m_pendingWrites(false)
{
}

template <typename T, typename M>
template <typename Func>
void deferred_guarded<T, M>::modify_detach(Func && func)
{
    std::unique_lock<M> lock(m_mutex, std::try_to_lock);

    if (lock.owns_lock()) {
        // consider looser memory ordering
        if (m_pendingWrites.load()) {
            std::vector<std::packaged_task<void(T &)>> localPending;
            m_pendingWrites.store(false);
            swap(localPending, *(m_pendingList.lock()));

            for (auto & f : localPending) {
                f(m_obj);
            }
        }

        func(m_obj);
    } else {
        m_pendingList.lock()->push_back(std::packaged_task<void(T &)>(std::forward<Func>(func)));
        m_pendingWrites.store(true);
    }
}

template <typename Ret, typename Func, typename T>
auto call_returning_future(Func & func, T & data) ->
    typename std::enable_if<!std::is_same<Ret, void>::value, std::future<Ret>>::type
{
    std::promise<Ret> promise;

    try {
        promise.set_value(func(data));
    } catch (...) {
        promise.set_exception(std::current_exception());
    }

    return promise.get_future();
}

template <typename Ret, typename Func, typename T>
auto call_returning_future(Func & func, T & data) ->
    typename std::enable_if<std::is_same<Ret, void>::value, std::future<Ret>>::type
{
    std::promise<Ret> promise;

    try {
        func(data);
        promise.set_value();
    } catch (...) {
        promise.set_exception(std::current_exception());
    }

    return promise.get_future();
}

template <typename Ret, typename T, typename Func>
auto package_task_void(Func && func) ->
    typename std::enable_if<std::is_same<Ret, void>::value,
                            std::pair<std::packaged_task<void(T &)>, std::future<void>>>::type
{
    std::packaged_task<void(T &)> task(std::forward<Func>(func));
    std::future<void>             task_future(task.get_future());

    return std::make_pair(std::move(task), std::move(task_future));
}

template <typename Ret, typename T, typename Func>
auto package_task_void(Func && func) ->
    typename std::enable_if<!std::is_same<Ret, void>::value,
                            std::pair<std::packaged_task<void(T &)>, std::future<T>>>::type
{
    std::packaged_task<Ret(T &)> task(std::forward<Func>(func));
    std::future<Ret>             task_future(task.get_future());

    return std::make_pair(std::packaged_task<void(T &)>(std::move(task)), std::move(task_future));
}

template <typename T, typename M>
template <typename Func>
auto deferred_guarded<T, M>::modify_async(Func && func) ->
    typename std::future<decltype(std::declval<Func>()(declref<T>()))>
{
    using return_t = decltype(func(m_obj));
    using future_t = std::future<decltype(func(m_obj))>;
    future_t retval;

    std::unique_lock<M> lock(m_mutex, std::try_to_lock);

    if (lock.owns_lock()) {
        if (m_pendingWrites.load()) {
            std::vector<std::packaged_task<void(T &)>> localPending;

            m_pendingWrites.store(false);
            swap(localPending, *(m_pendingList.lock()));
            for (auto & f : localPending) {
                f(m_obj);
            }
        }

        retval = call_returning_future<return_t>(func, m_obj);

    } else {
        std::pair<std::packaged_task<void(T &)>, std::future<return_t>> task_future =
            package_task_void<return_t, T>(std::forward<Func>(func));

        retval = std::move(task_future.second);

        m_pendingList.lock()->push_back(std::move(task_future.first));
        m_pendingWrites.store(true);
    }

    return retval;
}

template <typename T, typename M>
void deferred_guarded<T, M>::do_pending_writes() const
{
    if (m_pendingWrites.load()) {

        std::unique_lock<M> lock(m_mutex, std::try_to_lock);

        if (lock.owns_lock()) {
            if (m_pendingWrites.load()) {
                std::vector<std::packaged_task<void(T &)>> localPending;

                m_pendingWrites.store(false);
                swap(localPending, *(m_pendingList.lock()));

                for (auto & f : localPending) {
                    f(m_obj);
                }
            }
        }
    }
}

template <typename T, typename M>
auto deferred_guarded<T, M>::lock_shared() const -> shared_handle
{
    do_pending_writes();
    return shared_handle(&m_obj, m_mutex);
}

template <typename T, typename M>
auto deferred_guarded<T, M>::try_lock_shared() const -> shared_handle
{
    do_pending_writes();
    return try_lock_shared_handle(&m_obj, m_mutex);
}

template <typename T, typename M>
template <typename Duration>
auto deferred_guarded<T, M>::try_lock_shared_for(const Duration & d) const -> shared_handle
{
    do_pending_writes();
    return try_lock_shared_handle_for(&m_obj, m_mutex, d);
}

template <typename T, typename M>
template <typename TimePoint>
auto deferred_guarded<T, M>::try_lock_shared_until(const TimePoint & tp) const -> shared_handle
{
    do_pending_writes();
    return try_lock_shared_handle_until(&m_obj, m_mutex, timepoint);
}
}
#endif
