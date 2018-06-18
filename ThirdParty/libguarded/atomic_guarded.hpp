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

/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
/*
this file is a new addition to the library
*/
#ifndef ATOMIC_GUARDED_HPP
#define ATOMIC_GUARDED_HPP

#include <memory>
#include <mutex>
#include <type_traits>

namespace libguarded
{

/**
   \headerfile atomic_guarded.hpp <libguarded/atomic_guarded.hpp>

   This templated class wraps an object in an atomic like construct
   this is useful for classes where std::atomic doesn't work but atomic like operations are desired

   This class will use std::mutex for the internal locking mechanism.

*/
template <typename T, typename M = std::mutex>
class atomic_guarded
{
  
  public:

    /**
     Construct a guarded object. This constructor will accept any
     number of parameters, all of which are forwarded to the
     constructor of T.
    */
    template <typename... Us>
    atomic_guarded(Us &&... data) : m_obj(std::forward<Us>(data)...)
    {
        static_assert(std::is_copy_constructible<T>::value
            && std::is_copy_assignable<T>::value,
            "classes used must be copy constructible and assignable");
    }

    /** generate a copy of the protected object
    */
    T load() const
    {
        std::lock_guard<M> glock(m_mutex);
        return m_obj;
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

    /** cast operator so the class can work like T newT=Obj*/
    operator T() const 
    { 
        std::lock_guard<M> glock(m_mutex);
        return m_obj;
    }

    /** exchange the current object and replace it with the specified object*/
    T exchange(T newValue)
    {
        std::lock_guard<M> glock(m_mutex);
        std::swap(newValue, m_obj);
        return newValue;
    }

    /** do a compare and exchage operation 
    if the object is equal to the expected value it is replaced with desired
    and true is returned otherwise expected will contain the current value and false is returned
    */
    template <typename objType>
    bool compare_exchange(T &expected, objType &&desired)
    {
        std::lock_guard<M> glock(m_mutex);
        if (m_obj == expected)
        {
            m_obj = std::forward<objType>(desired);
            return true;
        }
        expected = m_obj;
        return false;
    }
  private:
    
    T m_obj; //!< primary object
    mutable M m_mutex; //!< mutex protecting object
};

}

#endif
