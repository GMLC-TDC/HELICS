/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#ifndef LIBGUARDED_HANDLES_HPP
#define LIBGUARDED_HANDLES_HPP
#include <mutex>
#include <shared_mutex>
#include <utility>

namespace libguarded
{

template <typename T, typename M>
class lock_handle
{
public:
    using pointer = T *;

    lock_handle(pointer val, std::unique_lock<M> lock) : data(val), m_handle_lock(std::move(lock))
    {
    }
    lock_handle(pointer val, M &mut) : data(val), m_handle_lock(mut)
    {
    }
    lock_handle(lock_handle &&) = default;
    lock_handle &operator=(lock_handle &&) = default;
    lock_handle(const lock_handle &) = delete;
    lock_handle &operator=(const lock_handle &) = delete;
    /** once unlocked can't be used for data access again*/ 
    void unlock()
    {
        data = nullptr;
        m_handle_lock.unlock();
    }
    T* operator-> () const noexcept
    {  // return pointer to class object
        return data;
    }
    T &operator* () const noexcept
    {  // return pointer to class object
        return *data;
    }
    operator bool() const noexcept
    {
        return (data != nullptr);
    }
private:
    //this is a non owning pointer
    pointer data;
    std::unique_lock<M> m_handle_lock;
};

template <typename M>
class shared_locker
{
public:
    using locker_type = std::shared_lock<M>;

    static locker_type generate_lock(M &mut)
    {
        return locker_type(mut);
    }
};

template <>
class shared_locker<std::mutex>
{
public:
    using locker_type = std::unique_lock<std::mutex>;

    static locker_type generate_lock(std::mutex &mut)
    {
        return locker_type(mut);
    }
};

template <>
class shared_locker<std::timed_mutex>
{
public:
    using locker_type = std::unique_lock<std::timed_mutex>;

    static locker_type generate_lock(std::timed_mutex &mut)
    {
        return locker_type(mut);
    }
};


template <typename T, typename M>
class shared_lock_handle
{
public:
    using pointer = const T *;
    using lock_type = typename shared_locker<M>::locker_type;
    shared_lock_handle(pointer val, lock_type lock) : data(val), m_handle_lock(std::move(lock))
    {
    }
    shared_lock_handle(pointer val, M& smutex) : data(val), m_handle_lock(smutex)
    {
    }
    shared_lock_handle(shared_lock_handle &&) = default;
    shared_lock_handle &operator=(shared_lock_handle &&) = default;
    shared_lock_handle(const shared_lock_handle &) = delete;
    shared_lock_handle &operator=(const shared_lock_handle &) = delete;
    /** once unlocked can't be used for data access again*/
    void unlock()
    {
        data = nullptr;
        m_handle_lock.unlock();
    }
    const T* operator-> () const noexcept
    {  // return pointer to class object
        return data;
    }
    const T &operator* () const noexcept
    {  // return pointer to class object
        return *data;
    }
    operator bool() const noexcept
    {
        return (data != nullptr);
    }
private:
    //this is a non owning pointer
    pointer data;
    lock_type m_handle_lock;
};
}

#endif // LIBGUARD_HANDLES_HPP