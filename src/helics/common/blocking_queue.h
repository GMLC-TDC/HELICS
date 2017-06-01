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

namespace helics {

template<typename T>
class BlockingQueue {
  public:
    explicit BlockingQueue();

	/** DISABLE_COPY_AND_ASSIGN */
	BlockingQueue(const BlockingQueue&) = delete;
	BlockingQueue& operator=(const BlockingQueue&) = delete;

    void push(const T& t);

    bool try_pop(T* t);

    // This logs a message if the threads needs to be blocked
    // useful for detecting e.g. when data feeding is too slow
    T pop(const std::string& log_on_wait = "");

    bool try_peek(T* t);

    // Return element without removing it
    T peek();

    size_t size() const;

  protected:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;


};

template<typename T>
BlockingQueue<T>::BlockingQueue()
{
}

template<typename T>
void BlockingQueue<T>::push(const T& t) {
  std::unique_lock<std::mutex> lock(mutex_);
  queue_.push(t);
  //lock.unlock();
  condition_.notify_one();
  // unlock occurs when we go out of scope, and only in extreme cases
  // should we not hold the lock before signalling
}

template<typename T>
bool BlockingQueue<T>::try_pop(T* t) {
  std::unique_lock<std::mutex> lock(mutex_);

  if (queue_.empty()) {
    return false;
  }

  *t = queue_.front();
  queue_.pop();
  return true;
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

  T t = queue_.front();
  queue_.pop();
  return t;
}

template<typename T>
bool BlockingQueue<T>::try_peek(T* t) {
  std::unique_lock<std::mutex> lock(mutex_);

  if (queue_.empty()) {
    return false;
  }

  *t = queue_.front();
  return true;
}

template<typename T>
T BlockingQueue<T>::peek() {
  std::unique_lock<std::mutex> lock(mutex_);

  while (queue_.empty()) {
    condition_.wait(lock);
  }

  return queue_.front();
}

template<typename T>
size_t BlockingQueue<T>::size() const {
  std::unique_lock<std::mutex> lock(mutex_);
  return queue_.size();
}

}  // namespace helics

#endif
