/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#pragma once
#ifndef SIMPLE_QUEUE_H_
#define SIMPLE_QUEUE_H_

#include <algorithm>
#include <mutex>
#include <type_traits>
#include <vector>

#include <extra_includes/optional.h>

/** class for very simple thread safe queue
@details  uses two vectors for the operations,  once the pull vector is empty it swaps the vectors
and reverses it so it can pop from the back
@tparam X the base class of the queue*/
template <class X>
class simpleQueue
{
  private:
    mutable std::mutex m_pushLock;  //!< lock for operations on the pushElements vector
    mutable std::mutex m_pullLock;  //!< lock for elements on the pullLock vector
    std::vector<X> pushElements;  //!< vector of elements being added
    std::vector<X> pullElements;  //!< vector of elements waiting extraction
  public:
    /** default constructor */
    simpleQueue () noexcept= default;
    /** constructor with a reservation size
    @param[in] capacity  the initial storage capacity of the queue*/
    simpleQueue (size_t capacity)
    {  // don't need to lock since we aren't out of the constructor yet
        pushElements.reserve (capacity);
        pullElements.reserve (capacity);
    }
    /** enable the move constructor not the copy constructor*/
    simpleQueue (simpleQueue &&sq) noexcept
        : pushElements (std::move (sq.pushElements)), pullElements (std::move (sq.pullElements))
    {
    }

    /** enable the move assignment not the copy assignement*/
    simpleQueue &operator= (simpleQueue &&sq)
    {
        std::lock_guard<std::mutex> pullLock (m_pullLock);  // first pullLock
        std::lock_guard<std::mutex> pushLock (m_pushLock);  // second pushLock
        pushElements = std::move (sq.pushElements);
        pullElements = std::move (sq.pullElements);
        return *this;
    }
    // const functions should be thread safe
    /** check whether there are any elements in the queue*/
    bool empty () const 
	{ 
		std::lock_guard<std::mutex> pullLock(m_pullLock);  // first pullLock
		return pullElements.empty (); 
	}
    /** get the current size of the queue*/
    size_t size () const 
	{ 
		std::lock_guard<std::mutex> pullLock(m_pullLock);  // first pullLock
		std::lock_guard<std::mutex> pushLock(m_pushLock);  // second pushLock
		return pullElements.size () + pushElements.size (); 
	}
    /** clear the queue*/
    void clear ()
    {
        std::lock_guard<std::mutex> pullLock (m_pullLock);  // first pullLock
        std::lock_guard<std::mutex> pushLock (m_pushLock);  // second pushLock
        pullElements.clear ();
        pushElements.clear ();
    }
    /** set the capacity of the queue
    actually double the requested the size will be reserved due to the use of two vectors internally
    @param[in] capacity  the capacity to reserve
    */
    void reserve (size_t capacity)
    {
        std::lock_guard<std::mutex> pullLock (m_pullLock);  // first pullLock
        std::lock_guard<std::mutex> pushLock (m_pushLock);  // second pushLock
        pullElements.reserve (capacity);
        pushElements.reserve (capacity);
    }

    /** push an element onto the queue
    val the value to push on the queue
    */
    template <class Z>
    void push (Z &&val)  // forwarding reference
    {
        if (!pullElements.empty ())
        {  // this is the most likely path
            std::lock_guard<std::mutex> pushLock (m_pushLock);  // only one lock on this branch
            pushElements.push_back (std::forward<Z> (val));
        }
        else
        {
            // acquire the lock for the pull stack
            std::lock_guard<std::mutex> pullLock (m_pullLock);  // first pullLock
            if (pullElements.empty ())
            {  // if it is still empty push the single element on that vector
                pullElements.push_back (std::forward<Z> (val));
            }
            else
            {  // this really shouldn't happen except in rare instances of high contention
               // we need to acquire the push lock and do the normal thing while holding the pull lock so the
               // last_element function will still behave properly
                std::lock_guard<std::mutex> pushLock (m_pushLock);  // second pushLock
                pushElements.push_back (std::forward<Z> (val));
            }
        }
    }
    /** push an element onto the queue
    val the value to push on the queue
    */
    template <class... Args>
    void emplace (Args &&... args)
    {
        if (!pullElements.empty ())
        {  // this is the most likely path
            std::lock_guard<std::mutex> pushLock (m_pushLock);  // only one lock on this branch
            pushElements.emplace_back (std::forward<Args> (args)...);
        }
        else
        {
            // acquire the lock for the pull stack
            std::lock_guard<std::mutex> pullLock (m_pullLock);  // first pullLock
            if (pullElements.empty ())
            {  // if it is still empty emplace the single element on the pull vector
                pullElements.emplace_back (std::forward<Args> (args)...);
            }
            else
            {  // this really shouldn't happen except in rare instances
               // now we need to acquire the push lock and do the normal thing while holding the pull lock so the
               // last_element function will still behave properly
                std::lock_guard<std::mutex> pushLock (m_pushLock);  // second pushLock
                pushElements.emplace_back (std::forward<Args> (args)...);
            }
        }
    }
    /*make sure there is no path to lock the push first then the pull second
    as that would be a race condition
    we either lock the pull first then the push,  or lock just one at a time
    otherwise we have a potential deadlock condition
    */
    /** extract the first element from the queue
    @return an empty optional if there is no element otherwise the optional will contain a value
    */
    stx::optional<X> pop ()
    {
        std::lock_guard<std::mutex> pullLock (m_pullLock);  // first pullLock
        if (pullElements.empty ())
        {
            std::unique_lock<std::mutex> pushLock (m_pushLock);  // second pushLock
            if (!pushElements.empty ())
            {  // on the off chance the queue got out of sync
                std::swap (pushElements, pullElements);
                pushLock.unlock ();  // we can free the push function to accept more elements after the swap call;
                std::reverse (pullElements.begin (), pullElements.end ());
                stx::optional<X> val (
                  std::move (pullElements.back ()));  // do it this way to allow moveable only types
                pullElements.pop_back ();
                return val;
            }
            return {};  // return the empty optional
        }
        else
        {
            stx::optional<X> val (
              std::move (pullElements.back ()));  // do it this way to allow moveable only types
            pullElements.pop_back ();
            if (pullElements.empty ())
            {
                std::unique_lock<std::mutex> pushLock (m_pushLock);  // second pushLock
                if (!pushElements.empty ())
                {  // this is the potential for slow operations
                    std::swap (pushElements, pullElements);
                    // we can free the push function to accept more elements after the swap call;
                    pushLock.unlock ();
                    std::reverse (pullElements.begin (), pullElements.end ());
                }
            }
            return val;
        }
    }
};

#endif
