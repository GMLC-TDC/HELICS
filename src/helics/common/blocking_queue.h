/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef HELICS_BLOCKING_QUEUE_HPP_
#define HELICS_BLOCKING_QUEUE_HPP_

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <extra_includes/optional.h>

namespace helics {

template<typename T>
class BlockingQueue {
  public:
	  /** default constructor*/
	  BlockingQueue() = default;

	/** DISABLE_COPY_AND_ASSIGN */
	BlockingQueue(const BlockingQueue&) = delete;
	BlockingQueue& operator=(const BlockingQueue&) = delete;

	/** push an object onto the queue*/
    void push(const T& t);
	/** construct on object in place on the queue*/
	template <class... Args>
	void emplace(Args &&... args)
	{
		std::unique_lock<std::mutex> lock(mutex_);
		queue_.emplace(std::forward(args)...);
		lock.unlock();
		condition_.notify_one();
		// unlock occurs when we go out of scope, and only in extreme cases
	}
	/** try to pop an object from the queue
	@param[out] t the location in which to place the object
	@return true if the operation was successful
	*/
    stx::optional<T> try_pop();

    // This logs a message if the threads needs to be blocked
    // useful for detecting e.g. when data feeding is too slow
    T pop(const std::string& log_on_wait = "");

	/** try to peek at an object without popping it from the stack
	@return an optional object with an objec of type T if available
	*/
	stx::optional<T> try_peek() const;

    /** Return element without removing it */
    T peek();

	/** get the current size of the queue*/
    size_t size() const;

  protected:
    std::queue<T> queue_;  //!< the actual storage for the data
    mutable std::mutex mutex_;	//!< mutex protecting the queue
    std::condition_variable condition_;	//!< condition variable for notification of new data


};

template<typename T>
void BlockingQueue<T>::push(const T& t) {
  std::unique_lock<std::mutex> lock(mutex_);
  queue_.push(t);
  lock.unlock();
  condition_.notify_one();
  // unlock occurs when we go out of scope, and only in extreme cases
}

template<typename T>
stx::optional<T> BlockingQueue<T>::try_pop() {
  std::lock_guard<std::mutex> lock(mutex_);

  if (queue_.empty()) {
    return{};
  }

  auto t = std::move(queue_.front());
  queue_.pop();
  return t;
}

template<typename T>
T BlockingQueue<T>::pop(const std::string& log_on_wait) {
  std::unique_lock<std::mutex> lock(mutex_);

  while (queue_.empty()) {
    if (!log_on_wait.empty()) {
      std::cerr << log_on_wait << std::endl;
    }
    condition_.wait(lock);
  }

  T t = std::move(queue_.front());
  queue_.pop();
  return t;
}

template<typename T>
stx::optional<T> BlockingQueue<T>::try_peek() const {
  std::lock_guard<std::mutex> lock(mutex_);

  if (queue_.empty()) {
    return {};
  }

  auto t = queue_.front();
  return t;
}

template<typename T>
size_t BlockingQueue<T>::size() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return queue_.size();
}

}  // namespace helics

#endif
