/*
Copyright (c) 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "helics/external/optional.hpp"

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <type_traits>
#include <vector>

/** class implementing a blocking queue with a priority channel
@details this class uses locks one for push and pull it can exhibit longer blocking times if the internal
operations require a swap, however in high usage the two locks will reduce contention in most cases.
*/
template<typename T>
class BlockingPriorityQueue {
  private:
    mutable std::mutex m_pushLock; //!< lock for operations on the pushElements vector
    mutable std::mutex m_pullLock; //!< lock for elements on the pullLock vector
    std::vector<T> pushElements; //!< vector of elements being added
    std::vector<T> pullElements; //!< vector of elements waiting extraction
    std::atomic<bool> queueEmptyFlag{true}; //!< flag indicating the queue is empty
    std::queue<T> priorityQueue; //!< the priority channel
    // the condition variable should be keyed of the pullLock
    std::condition_variable condition; //!< condition variable for notification of new data
  public:
    /** default constructor*/
    BlockingPriorityQueue() = default;

    /** clear the queue*/
    void clear()
    {
        std::lock_guard<std::mutex> pullLock(m_pullLock); // first pullLock
        std::lock_guard<std::mutex> pushLock(m_pushLock); // second pushLock
        pullElements.clear();
        pushElements.clear();
        while (!priorityQueue.empty()) {
            priorityQueue.pop();
        }
        queueEmptyFlag = true;
    }

    ~BlockingPriorityQueue() { clear(); }
    /** constructor with the capacity numbers
    @details there are two internal vectors that alternate
    so the actual reserve is 2x the capacity numbers in two different vectors
    @param capacity the initial reserve capacity for the arrays
    */
    explicit BlockingPriorityQueue(size_t capacity)
    { // don't need to lock since we aren't out of the constructor yet
        pushElements.reserve(capacity);
        pullElements.reserve(capacity);
    }
    /** enable the move constructor not the copy constructor*/
    BlockingPriorityQueue(BlockingPriorityQueue&& bq) noexcept:
        pushElements(std::move(bq.pushElements)), pullElements(std::move(bq.pullElements)),
        priorityQueue(std::move(bq.priorityQueue))
    {
        queueEmptyFlag = (pullElements.empty() && priorityQueue.empty());
    }

    /** enable the move assignment not the copy assignment*/
    BlockingPriorityQueue& operator=(BlockingPriorityQueue&& sq) noexcept
    {
        std::lock_guard<std::mutex> pullLock(m_pullLock); // first pullLock
        std::lock_guard<std::mutex> pushLock(m_pushLock); // second pushLock
        pushElements = std::move(sq.pushElements);
        pullElements = std::move(sq.pullElements);
        priorityQueue = std::move(sq.priorityQueue);
        queueEmptyFlag = (pullElements.empty() && priorityQueue.empty());
        return *this;
    }
    /** DISABLE_COPY_AND_ASSIGN */
    BlockingPriorityQueue(const BlockingPriorityQueue&) = delete;
    BlockingPriorityQueue& operator=(const BlockingPriorityQueue&) = delete;

    /** set the capacity of the queue
    actually double the requested the size will be reserved due to the use of two vectors internally
    @param[in] capacity  the capacity to reserve
    */
    void reserve(size_t capacity)
    {
        std::lock_guard<std::mutex> pullLock(m_pullLock); // first pullLock
        std::lock_guard<std::mutex> pushLock(m_pushLock); // second pushLock
        pullElements.reserve(capacity);
        pushElements.reserve(capacity);
    }

    /** push an element onto the queue
    val the value to push on the queue
    */
    template<class Z>
    void push(Z&& val) // forwarding reference
    {
        std::unique_lock<std::mutex> pushLock(m_pushLock); // only one lock on this branch
        if (!pushElements.empty()) {
            pushElements.push_back(std::forward<Z>(val));
        } else {
            bool expEmpty = true;
            if (queueEmptyFlag.compare_exchange_strong(expEmpty, false)) {
                // release the push lock so we don't get a potential deadlock condition
                pushLock.unlock();
                std::unique_lock<std::mutex> pullLock(m_pullLock); // first pullLock
                queueEmptyFlag = false;
                if (pullElements.empty()) {
                    pullElements.push_back(std::forward<Z>(val));
                    // pullLock.unlock ();
                    condition.notify_all();
                } else {
                    pushLock.lock();
                    pushElements.push_back(std::forward<Z>(val));
                }
            } else {
                pushElements.push_back(std::forward<Z>(val));
                expEmpty = true;
                if (queueEmptyFlag.compare_exchange_strong(expEmpty, false)) {
                    condition.notify_all();
                }
            }
        }
    }

    /** push an element onto the queue
    val the value to push on the queue
    */
    template<class Z>
    void pushPriority(Z&& val) // forwarding reference
    {
        bool expEmpty = true;
        if (queueEmptyFlag.compare_exchange_strong(expEmpty, false)) {
            std::unique_lock<std::mutex> pullLock(m_pullLock); // first pullLock
            queueEmptyFlag = false; // need to set the flag again just in case after we get the lock
            priorityQueue.push(std::forward<Z>(val));
            // pullLock.unlock ();
            condition.notify_all();
        } else {
            std::unique_lock<std::mutex> pullLock(m_pullLock);
            priorityQueue.push(std::forward<Z>(val));
            expEmpty = true;
            if (queueEmptyFlag.compare_exchange_strong(expEmpty, false)) {
                condition.notify_all();
            }
        }
    }

    /** construct on object in place on the queue */
    template<class... Args>
    void emplace(Args&&... args)
    {
        std::unique_lock<std::mutex> pushLock(m_pushLock); // only one lock on this branch
        if (!pushElements.empty()) {
            pushElements.emplace_back(std::forward<Args>(args)...);
        } else {
            bool expEmpty = true;
            if (queueEmptyFlag.compare_exchange_strong(expEmpty, false)) {
                // release the push lock so we don't get a potential deadlock condition
                pushLock.unlock();
                std::unique_lock<std::mutex> pullLock(m_pullLock); // first pullLock
                queueEmptyFlag = false; // need to set the flag again after we get the lock
                if (pullElements.empty()) {
                    pullElements.emplace_back(std::forward<Args>(args)...);
                    //  pullLock.unlock ();
                    condition.notify_all();
                } else {
                    pushLock.lock();
                    pushElements.emplace_back(std::forward<Args>(args)...);
                }
            } else {
                pushElements.emplace_back(std::forward<Args>(args)...);
                expEmpty = true;
                if (queueEmptyFlag.compare_exchange_strong(expEmpty, false)) {
                    condition.notify_all();
                }
            }
        }
    }

    /** emplace an element onto the priority queue
    val the value to push on the queue
    */
    template<class... Args>
    void emplacePriority(Args&&... args)
    {
        bool expEmpty = true;
        if (queueEmptyFlag.compare_exchange_strong(expEmpty, false)) {
            std::unique_lock<std::mutex> pullLock(m_pullLock); // first pullLock
            queueEmptyFlag = false; // need to set the flag again just in case after we get the lock
            priorityQueue.emplace(std::forward<Args>(args)...);
            // pullLock.unlock ();
            condition.notify_all();
        } else {
            std::unique_lock<std::mutex> pullLock(m_pullLock);
            priorityQueue.emplace(std::forward<Args>(args)...);
            expEmpty = true;
            if (queueEmptyFlag.compare_exchange_strong(expEmpty, false)) {
                condition.notify_all();
            }
        }
    }
    /** try to peek at an object without popping it from the stack
    @details only available for copy assignable objects
    @return an optional object with an object of type T if available
    */
    template<typename = std::enable_if<std::is_copy_assignable<T>::value>>
    stx::optional<T> try_peek() const
    {
        std::lock_guard<std::mutex> lock(m_pullLock);
        if (!priorityQueue.empty()) {
            return priorityQueue.front();
        }
        if (pullElements.empty()) {
            return stx::nullopt;
        }

        auto t = pullElements.back();
        return t;
    }

    /** try to pop an object from the queue
    @return an optional containing the value if successful the optional will be empty if there is no
    element in the queue
    */
    stx::optional<T> try_pop();

    /** blocking call to wait on an object from the stack*/
    T pop()
    {
        T actval;
        auto val = try_pop();
        while (!val) {
            std::unique_lock<std::mutex> pullLock(m_pullLock); // get the lock then wait
            if (!priorityQueue.empty()) {
                actval = std::move(priorityQueue.front());
                priorityQueue.pop();
                return actval;
            }
            if (!pullElements.empty()) // make sure we are actually empty;
            {
                actval = std::move(pullElements.back());
                pullElements.pop_back();
                return actval;
            }
            condition.wait(pullLock); // now wait
            if (!priorityQueue.empty()) {
                actval = std::move(priorityQueue.front());
                priorityQueue.pop();
                return actval;
            }
            if (!pullElements.empty()) // check for spurious wake-ups
            {
                actval = std::move(pullElements.back());
                pullElements.pop_back();
                return actval;
            }
            pullLock.unlock();
            val = try_pop();
        }
        // move the value out of the optional
        actval = std::move(*val);
        return actval;
    }

    /** blocking call to wait on an object from the stack with timeout*/
    stx::optional<T> pop(std::chrono::milliseconds timeout)
    {
        auto val = try_pop();
        while (!val) {
            std::unique_lock<std::mutex> pullLock(m_pullLock); // get the lock then wait
            if (!priorityQueue.empty()) {
                val = std::move(priorityQueue.front());
                priorityQueue.pop();
                break;
            }
            if (!pullElements.empty()) // make sure we are actually empty;
            {
                val = std::move(pullElements.back());
                pullElements.pop_back();
                break;
            }
            auto res = condition.wait_for(pullLock, timeout); // now wait

            if (!priorityQueue.empty()) {
                val = std::move(priorityQueue.front());
                priorityQueue.pop();
                break;
            }
            if (!pullElements.empty()) // check for spurious wake-ups
            {
                val = std::move(pullElements.back());
                pullElements.pop_back();
                break;
            }
            pullLock.unlock();
            val = try_pop();
            if (res == std::cv_status::timeout) {
                break;
            }
        }
        // move the value out of the optional
        return val;
    }

    /** blocking call that will call the specified functor
    if the queue is empty
    @param callOnWaitFunction an nullary functor that will be called if the initial query does not return a value
    @details  after calling the function the call will check again and if still empty
    will block and wait.
    */
    template<typename Functor>
    T pop(Functor callOnWaitFunction)
    {
        auto val = try_pop();
        while (!val) // may be spurious so make sure actually have a value
        {
            callOnWaitFunction();
            std::unique_lock<std::mutex> pullLock(m_pullLock); // first pullLock
            if (!priorityQueue.empty()) {
                auto actval = std::move(priorityQueue.front());
                priorityQueue.pop();
                return actval;
            }
            if (!pullElements.empty())

            { // the callback may fill the queue or it may have been filled in the meantime
                auto actval = std::move(pullElements.back());
                pullElements.pop_back();
                return actval;
            }
            condition.wait(pullLock);
            // need to check again to handle spurious wake-up
            if (!priorityQueue.empty()) {
                auto actval = std::move(priorityQueue.front());
                priorityQueue.pop();
                return actval;
            }
            if (!pullElements.empty()) {
                auto actval = std::move(pullElements.back());
                pullElements.pop_back();
                return actval;
            }
            pullLock.unlock();
            val = try_pop();
        }
        return std::move(*val);
    }

    /** check whether there are any elements in the queue
because this is meant for multi-threaded applications this may or may not have any meaning
depending on the number of consumers
*/
    bool empty() const;
};

template<typename T>
stx::optional<T> BlockingPriorityQueue<T>::try_pop()
{
    std::lock_guard<std::mutex> pullLock(m_pullLock); // first pullLock
    if (!priorityQueue.empty()) {
        stx::optional<T> val(std::move(priorityQueue.front()));
        priorityQueue.pop();
        return val;
    }
    if (pullElements.empty()) {
        std::unique_lock<std::mutex> pushLock(m_pushLock); // second pushLock
        if (!pushElements.empty()) { // on the off chance the queue got out of sync
            std::swap(pushElements, pullElements);
            pushLock
                .unlock(); // we can free the push function to accept more elements after the swap call;
            std::reverse(pullElements.begin(), pullElements.end());
            stx::optional<T> val(
                std::move(pullElements.back())); // do it this way to allow movable only types
            pullElements.pop_back();
            if (pullElements.empty()) {
                pushLock.lock(); // second pushLock
                if (!pushElements.empty()) // more elements could have been added
                { // this is the potential for slow operations
                    std::swap(pushElements, pullElements);
                    // we can free the push function to accept more elements after the swap call;
                    pushLock.unlock();
                    std::reverse(pullElements.begin(), pullElements.end());
                } else {
                    queueEmptyFlag = true;
                }
            }
            return val;
        }
        queueEmptyFlag = true;
        return {}; // return the empty optional
    }
    stx::optional<T> val(
        std::move(pullElements.back())); // do it this way to allow movable only types
    pullElements.pop_back();
    if (pullElements.empty()) {
        std::unique_lock<std::mutex> pushLock(m_pushLock); // second pushLock
        if (!pushElements.empty()) { // this is the potential for slow operations
            std::swap(pushElements, pullElements);
            // we can free the push function to accept more elements after the swap call;
            pushLock.unlock();
            std::reverse(pullElements.begin(), pullElements.end());
        } else {
            queueEmptyFlag = true;
        }
    }
    return val;
}

template<typename T>
bool BlockingPriorityQueue<T>::empty() const
{
    return queueEmptyFlag;
}
