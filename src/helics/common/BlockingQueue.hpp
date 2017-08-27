/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef HELICS_BLOCKING_QUEUE2_HPP_
#define HELICS_BLOCKING_QUEUE2_HPP_

#include <condition_variable>
#include <type_traits>
#include <mutex>
#include <string>
#include <vector>
#include <extra_includes/optional.h>

template<typename T>
class BlockingQueue2 {
private:
	mutable std::mutex m_pushLock;  //!< lock for operations on the pushElements vector
	mutable std::mutex m_pullLock;  //!< lock for elements on the pullLock vector
	std::vector<T> pushElements;  //!< vector of elements being added
	std::vector<T> pullElements;  //!< vector of elements waiting extraction

	std::condition_variable condition_;	//!< condition variable for notification of new data
public:
	/** default constructor*/
	BlockingQueue2() = default;

	BlockingQueue2(size_t capacity)
	{  // don't need to lock since we aren't out of the constructor yet
		pushElements.reserve(capacity);
		pullElements.reserve(capacity);
	}
	/** enable the move constructor not the copy constructor*/
	BlockingQueue2(BlockingQueue2 &&bq) noexcept
		: pushElements(std::move(sq.pushElements)), pullElements(std::move(sq.pullElements))
	{
	}

	/** enable the move assignment not the copy assignement*/
	BlockingQueue2 &operator= (BlockingQueue2 &&sq)
	{
		std::lock_guard<std::mutex> pullLock(m_pullLock);  // first pullLock
		std::lock_guard<std::mutex> pushLock(m_pushLock);  // second pushLock
		pushElements = std::move(sq.pushElements);
		pullElements = std::move(sq.pullElements);
		return *this;
	}
	/** DISABLE_COPY_AND_ASSIGN */
	BlockingQueue2(const BlockingQueue2&) = delete;
	BlockingQueue2& operator=(const BlockingQueue2&) = delete;


	/** set the capacity of the queue
	actually double the requested the size will be reserved due to the use of two vectors internally
	@param[in] capacity  the capacity to reserve
	*/
	void reserve(size_t capacity)
	{
		std::lock_guard<std::mutex> pullLock(m_pullLock);  // first pullLock
		std::lock_guard<std::mutex> pushLock(m_pushLock);  // second pushLock
		pullElements.reserve(capacity);
		pushElements.reserve(capacity);
	}

	/** push an element onto the queue
	val the value to push on the queue
	*/
	template <class Z>
	void push(Z &&val)  // forwarding reference
	{
		if (!pullElements.empty()) //this is read only and even if no entirely correct the next pop will resync it.  
		{  // this is the most likely path
			std::lock_guard<std::mutex> pushLock(m_pushLock);  // only one lock on this branch
			pushElements.push_back(std::forward<Z>(val));
		}
		else
		{
			// acquire the lock for the pull stack
			std::unique_lock<std::mutex> pullLock(m_pullLock);  // first pullLock
			if (pullElements.empty())
			{  // if it is still empty push the single element on that vector
				pullElements.push_back(std::forward<Z>(val));
			}
			else
			{  // this really shouldn't happen except in rare instances of high contention
			   // we need to acquire the push lock and do the normal thing while holding the pull lock so the
			   // last_element function will still behave properly
				std::lock_guard<std::mutex> pushLock(m_pushLock);  // second pushLock
				pushElements.push_back(std::forward<Z>(val));
			}
			pullLock.unlock();
			condition_.notify_one();
		}

	}

	/** construct on object in place on the queue*/
	template <class... Args>
	void emplace(Args &&... args)
	{
		if (!pullElements.empty())
		{  // this is the most likely path
			std::lock_guard<std::mutex> pushLock(m_pushLock);  // only one lock on this branch
			pushElements.emplace_back(std::forward<Args>(args)...);
		}
		else
		{
			// acquire the lock for the pull stack
			std::unique_lock<std::mutex> pullLock(m_pullLock);  // first pullLock
			if (pullElements.empty())
			{  // if it is still empty emplace the single element on the pull vector
				pullElements.emplace_back(std::forward<Args>(args)...);
			}
			else
			{  // this really shouldn't happen except in rare instances
			   // now we need to acquire the push lock and do the normal thing while holding the pull lock so the
			   // last_element function will still behave properly
				std::lock_guard<std::mutex> pushLock(m_pushLock);  // second pushLock
				pushElements.emplace_back(std::forward<Args>(args)...);
			}
			pullLock.unlock();
			condition_.notify_one();
		}
	}

	/** try to peek at an object without popping it from the stack
	@details only available for copy assignable objects
	@return an optional object with an object of type T if available
	*/
	template<typename =std::enable_if<std::is_copy_assignable<T>::value>>
	stx::optional<T> try_peek() const 
	{
		std::lock_guard<std::mutex> lock(m_pullLock);

		if (pullElements.empty()) 
		{
			return{};
		}

		auto t = pullElements.back();
		return t;
	}

	/** try to pop an object from the queue
	@return an optional containing the value if successful
	*/
	stx::optional<T> try_pop();

	/** blocking call to wait on an object from the stack*/
	T pop()
	{
		auto val = try_pop();
		while (!val)
		{
			std::unique_lock<std::mutex> pullLock(m_pullLock);  // get the lock then wait
			condition_.wait(pullLock);
			if (!pullElements.empty())
			{
				auto actval = std::move(pullElements.back());
				pullElements.pop_back();
				return actval;
			}
			else
			{
				pullLock.unlock();
				val = try_pop();
			}
		}
		return std::move(*val);
	}

	template< typename Functor>
	T pop(Functor callOnWaitFunction)
	{
		auto val = try_pop();
		while (!val)//may be spurious so make sure actually have a value
		{
			callOnWaitFunction();
			std::unique_lock<std::mutex> pullLock(m_pullLock);  // first pullLock
			if (!pullElements.empty()) //the callback may fill the queue or it may have been filled in the meantime
			{
				auto actval = std::move(pullElements.back());
				pullElements.pop_back();
				return actval;
			}
			condition_.wait(pullLock);
			if (!pullElements.empty())
			{
				auto actval = std::move(pullElements.back());
				pullElements.pop_back();
				return actval;
			}
			else
			{
				pullLock.unlock();
				val = try_pop();
			}
			
		}
		return std::move(*val);
	}


	
	/** check whether there are any elements in the queue
because this is meant for mutlti threaded applications this may or may not have any meaning
depending on the number of consumers
*/
	bool empty() const;
	/** get the current size of the queue
	@details this may or may not have much meaning depending on the number of consumers
	*/
	size_t size() const;




};



template<typename T>
stx::optional<T> BlockingQueue2<T>::try_pop() {
	std::lock_guard<std::mutex> pullLock(m_pullLock);  // first pullLock
	if (pullElements.empty())
	{
		std::unique_lock<std::mutex> pushLock(m_pushLock);  // second pushLock
		if (!pushElements.empty())
		{  // on the off chance the queue got out of sync
			std::swap(pushElements, pullElements);
			pushLock.unlock();  // we can free the push function to accept more elements after the swap call;
			std::reverse(pullElements.begin(), pullElements.end());
			stx::optional<T> val(
				std::move(pullElements.back()));  // do it this way to allow moveable only types
			pullElements.pop_back();
			return val;
		}
		return{};  // return the empty optional
	}
	else
	{
		stx::optional<T> val(
			std::move(pullElements.back()));  // do it this way to allow moveable only types
		pullElements.pop_back();
		if (pullElements.empty())
		{
			std::unique_lock<std::mutex> pushLock(m_pushLock);  // second pushLock
			if (!pushElements.empty())
			{  // this is the potential for slow operations
				std::swap(pushElements, pullElements);
				// we can free the push function to accept more elements after the swap call;
				pushLock.unlock();
				std::reverse(pullElements.begin(), pullElements.end());
			}
		}
		return val;
	}
}


template<typename T>
size_t BlockingQueue2<T>::size() const 
{
	std::lock_guard<std::mutex> pullLock(m_pullLock);  // first pullLock
	std::lock_guard<std::mutex> pushLock(m_pushLock);  // second pushLock
	return pullElements.size() + pushElements.size();
}

template<typename T>
bool BlockingQueue2<T>::empty() const
{
	std::lock_guard<std::mutex> pullLock(m_pullLock);  // first pullLock
	return pullElements.empty();
}
#endif

