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
        if (m_handle_lock.owns_lock())
		{
            m_handle_lock.unlock ();
		};
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
    auto begin()
    {
        return std::begin(*data);
    }
    auto end()
    {
        return std::end(*data);
    }
private:
    //this is a non owning pointer
    pointer data;
    std::unique_lock<M> m_handle_lock;
};

template <typename T, typename M>
lock_handle<T, M> try_lock_handle(T *obj, M &gmutex)
{
    typename lock_handle<T, M>::lock_type glock(gmutex, std::try_to_lock);
    if (glock.owns_lock()) {
        return lock_handle<T, M>(obj, std::move(glock));
    }
    else {
        return lock_handle<T, M>(nullptr, std::move(glock));
    }
}

template <typename T, typename M, typename Duration>
lock_handle<T, M> try_lock_handle_for(T *obj, M &gmutex,const Duration & d)
{
    typename lock_handle<T, M>::lock_type glock(gmutex, d);
    if (glock.owns_lock()) {
        return lock_handle<T,M>(obj, std::move(glock));
    }
    else {
        return lock_handle<T, M>(nullptr, std::move(glock));
    }
}

template <typename T, typename M, typename TimePoint>
lock_handle<T, M> try_lock_handle_until(T *obj, M &gmutex, const TimePoint & tp)
{
    typename lock_handle<T, M>::lock_type glock(gmutex, tp);
    if (glock.owns_lock()) {
        return lock_handle<T,M>(obj, std::move(glock));
    }
    else {
        return lock_handle<T,M>(nullptr, std::move(glock));
    }
}

/** template SFINAE if lock shared is available*/
template < class T >                                                             
class Has_lock_shared                                                         
{                                                                                
private:                                                                         
    using Yes = char[2];                                                         
    using  No = char[1];                                                         
                                                                                 
    struct Fallback { int member; };                                             
    struct Derived : T, Fallback { };                                            
                                                                                 
    template < class U >                                                         
    static No& test ( decltype(U::lock_shared)* );                                    
    template < typename U >                                                      
    static Yes& test ( U* );                                                     
                                                                                  
public:                                                                           
    static constexpr bool RESULT = sizeof(test<Derived>(nullptr)) == sizeof(Yes); 
};                                                                                
                                                                                  
template < class T >                                                              
struct has_lock_shared                                                       
: public std::integral_constant<bool, Has_lock_shared<T>::RESULT>              
{                                                                                 
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
        if (m_handle_lock.owns_lock())
        {
            m_handle_lock.unlock ();
		}
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
    auto begin() const
    {
        return std::begin(*data);
    }
    auto end() const
    {
        return std::end(*data);
    }
private:
    //this is a non owning pointer
    pointer data;
    lock_type m_handle_lock;
};

template <typename T, typename M>
shared_lock_handle<T, M> try_lock_shared_handle(const T *obj, M &smutex)
{
    typename shared_lock_handle<T, M>::lock_type slock(smutex, std::try_to_lock);
    if (slock.owns_lock()) {
        return shared_lock_handle<T, M>(obj, std::move(slock));
    }
    else {
        return shared_lock_handle<T, M>(nullptr, std::move(slock));
    }
}

template <typename T, typename M, typename Duration>
shared_lock_handle<T, M> try_lock_shared_handle_for(const T *obj, M &smutex, const Duration & d)
{
    typename shared_lock_handle<T, M>::lock_type slock(smutex, d);
    if (slock.owns_lock()) {
        return shared_lock_handle<T, M>(obj, std::move(slock));
    }
    else {
        return shared_lock_handle<T, M>(nullptr, std::move(slock));
    }
}

template <typename T, typename M, typename TimePoint>
shared_lock_handle<T, M> try_lock_shared_handle_until(const T *obj, M &smutex, const TimePoint & tp)
{
    typename shared_lock_handle<T, M>::lock_type slock(smutex, tp);
    if (slock.owns_lock()) {
        return shared_lock_handle<T,M>(obj, std::move(slock));
    }
    else {
        return shared_lock_handle<T,M>(nullptr, std::move(slock));
    }
}
}

#endif // LIBGUARDED_HANDLES_HPP