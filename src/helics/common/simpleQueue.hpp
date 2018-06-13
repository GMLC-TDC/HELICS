/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include <algorithm>
#include <atomic>
#include <mutex>
#include <type_traits>
#include <vector>

#include <helics_includes/optional.hpp>

/** class for very simple thread safe queue
@details  uses two vectors for the operations,  once the pull vector is empty it swaps the vectors
and reverses it so it can pop from the back as well as an atomic flag indicating the queue is empty
@tparam X the base class of the queue*/
template <class X>
class SimpleQueue
{
  private:
    mutable std::mutex m_pushLock;  //!< lock for operations on the pushElements vector
    mutable std::mutex m_pullLock;  //!< lock for elements on the pullLock vector
    std::vector<X> pushElements;  //!< vector of elements being added
    std::vector<X> pullElements;  //!< vector of elements waiting extraction
    std::atomic<bool> queueEmptyFlag{true};  //!< flag indicating the queue is Empty
  public:
    /** default constructor */
    SimpleQueue () = default;
    /** destructor*/
    ~SimpleQueue ()
    {
        // these locks are primarily for memory synchronization multiple access in the destructor would be a bad
        // thing
        std::lock_guard<std::mutex> pullLock (m_pullLock);  // first pullLock
        std::lock_guard<std::mutex> pushLock (m_pushLock);  // second pushLock
        /** clear the elements as part of the destruction while the locks are engaged*/
        pushElements.clear ();
        pullElements.clear ();
    }
    /** constructor with a reservation size
    @param[in] capacity  the initial storage capacity of the queue*/
    explicit SimpleQueue (size_t capacity)
    {  // don't need to lock since we aren't out of the constructor yet
        pushElements.reserve (capacity);
        pullElements.reserve (capacity);
    }
    /** enable the move constructor not the copy constructor*/
    SimpleQueue (SimpleQueue &&sq) noexcept
        : pushElements (std::move (sq.pushElements)), pullElements (std::move (sq.pullElements))
    {
        queueEmptyFlag = pullElements.empty ();
    }

    /** enable the move assignment not the copy assignment*/
    SimpleQueue &operator= (SimpleQueue &&sq) noexcept
    {
        std::lock_guard<std::mutex> pullLock (m_pullLock);  // first pullLock
        std::lock_guard<std::mutex> pushLock (m_pushLock);  // second pushLock
        pushElements = std::move (sq.pushElements);
        pullElements = std::move (sq.pullElements);
        queueEmptyFlag = pullElements.empty ();
        return *this;
    }
    /** DISABLE_COPY_AND_ASSIGN */
    SimpleQueue (const SimpleQueue &) = delete;
    SimpleQueue &operator= (const SimpleQueue &) = delete;

    /** check whether there are any elements in the queue
    because this is meant for multi threaded applications this may or may not have any meaning
    depending on the number of consumers
    */
    bool empty () const
    {
        std::lock_guard<std::mutex> pullLock (m_pullLock);  // first pullLock
        return pullElements.empty ();
    }
    /** get the current size of the queue*/
    size_t size () const
    {
        std::lock_guard<std::mutex> pullLock (m_pullLock);  // first pullLock
        std::lock_guard<std::mutex> pushLock (m_pushLock);  // second pushLock
        return pullElements.size () + pushElements.size ();
    }
    /** clear the queue*/
    void clear ()
    {
        std::lock_guard<std::mutex> pullLock (m_pullLock);  // first pullLock
        std::lock_guard<std::mutex> pushLock (m_pushLock);  // second pushLock
        pullElements.clear ();
        pushElements.clear ();
        queueEmptyFlag = true;
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
        std::unique_lock<std::mutex> pushLock (m_pushLock);  // only one lock on this branch
        if (!pushElements.empty ())
        {
            pushElements.push_back (std::forward<Z> (val));
        }
        else
        {
            bool expEmpty = true;
            if (queueEmptyFlag.compare_exchange_strong (expEmpty, false))
            {
                // release the push lock
                pushLock.unlock ();
                std::unique_lock<std::mutex> pullLock (m_pullLock);  // first pullLock
                queueEmptyFlag = false;
                if (pullElements.empty ())
                {
                    pullElements.push_back (std::forward<Z> (val));
                    pullLock.unlock ();
                }
                else
                {
                    pushLock.lock ();
                    pushElements.push_back (std::forward<Z> (val));
                }
            }
            else
            {
                pushElements.push_back (std::forward<Z> (val));
            }
        }
    }

/** push a vector onto the queue
    val the vector of values to push on the queue
    */
    void pushVector (const std::vector<X> &val)  // universal reference
    {
        std::unique_lock<std::mutex> pushLock(m_pushLock);  // only one lock on this branch
        if (!pushElements.empty())
        {
            pushElements.insert(pushElements.end(), val.begin(), val.end());
        }
        else
        {
            bool expEmpty = true;
            if (queueEmptyFlag.compare_exchange_strong(expEmpty, false))
            {
                // release the push lock
                pushLock.unlock();
                std::unique_lock<std::mutex> pullLock(m_pullLock);  // first pullLock
                queueEmptyFlag = false;
                if (pullElements.empty())
                {
                    pullElements.insert(pullElements.end(), val.rbegin(), val.rend());
                    pullLock.unlock();
                }
                else
                {
                    pushLock.lock();
                    pushElements.insert(pushElements.end(), val.begin(), val.end());
                }
            }
            else
            {
                pushElements.insert(pushElements.end(), val.begin(), val.end());
            }
        }
    }

    /** emplace an element onto the queue
    val the value to emplace on the queue
    */
    template <class... Args>
    void emplace (Args &&... args)
    {
        std::unique_lock<std::mutex> pushLock (m_pushLock);  // only one lock on this branch
        if (!pushElements.empty ())
        {
            pushElements.emplace_back (std::forward<Args> (args)...);
        }
        else
        {
            bool expEmpty = true;
            if (queueEmptyFlag.compare_exchange_strong (expEmpty, false))
            {
                // release the push lock
                pushLock.unlock ();
                std::unique_lock<std::mutex> pullLock (m_pullLock);  // first pullLock
                queueEmptyFlag = false;
                if (pullElements.empty ())
                {
                    pullElements.emplace_back (std::forward<Args> (args)...);
                    pullLock.unlock ();
                }
                else
                {
                    pushLock.lock ();
                    pushElements.emplace_back (std::forward<Args> (args)...);
                }
            }
            else
            {
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
                if (pullElements.empty ())
                {
                    pushLock.lock ();  // second pushLock
                    if (!pushElements.empty ())  // more elements could have been added
                    {  // this is the potential for slow operations
                        std::swap (pushElements, pullElements);
                        // we can free the push function to accept more elements after the swap call;
                        pushLock.unlock ();
                        std::reverse (pullElements.begin (), pullElements.end ());
                    }
                    else
                    {
                        queueEmptyFlag = true;
                    }
                }
                return val;
            }
            queueEmptyFlag = true;
            return {};  // return the empty optional
        }
        stx::optional<X> val (std::move (pullElements.back ()));  // do it this way to allow moveable only types
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
            else
            {
                queueEmptyFlag = true;
            }
        }
        return val;
    }

    /** try to peek at an object without popping it from the stack
    @details only available for copy assignable objects
    @return an optional object with an object of type T if available
    */
    template <typename = std::enable_if<std::is_copy_assignable<X>::value>>
    stx::optional<X> peek () const
    {
        std::lock_guard<std::mutex> lock (m_pullLock);

        if (pullElements.empty ())
        {
            return {};
        }

        auto t = pullElements.back ();
        return t;
    }
};
