/*
Copyright (c) 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "helics/external/optional.hpp"

#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <chrono>
#include <utility>

namespace helics {
namespace ipc {

    /** class implementing a blocking queue with a priority channel
@details this class uses locks one for push and pull it can exhibit longer blocking times if the
internal operations require a swap, however in high usage the two locks will reduce contention in
most cases.
*/
    class IpcBlockingPriorityQueue {
      private:
      public:
        /** default constructor*/
        IpcBlockingPriorityQueue(void* dataBlock, size_t blockSize);

        /** clear the queue*/
        void clear();

        /** DISABLE_COPY_AND_ASSIGN */
        IpcBlockingPriorityQueue(const IpcBlockingPriorityQueue&) = delete;
        IpcBlockingPriorityQueue& operator=(const IpcBlockingPriorityQueue&) = delete;

        /** push a data block
    val the value to push on the queue
    */
        void push(const unsigned char* data, size_t size);
        /** push an element onto the queue
    val the value to push on the queue
    */
        void pushPriority(const unsigned char* data, size_t size);
        /** push a data block
    val the value to push on the queue
    */
        bool try_push(const unsigned char* data, size_t size);

        /** push an element onto the queue
    val the value to push on the queue
    */
        bool try_pushPriority(const unsigned char* data, size_t size);

        /** try to peek at an object without popping it from the stack
    @details only available for copy assignable objects
    @return an optional object with an object of type T if available
    */
        stx::optional<std::pair<unsigned char*, int>> try_peek() const;

        /** try to pop an object from the queue
    @return an optional containing the value if successful the optional will be empty if there is no
    element in the queue
    */
        bool try_pop(unsigned char* data, int maxSize);

        int pop(unsigned char* data, int maxSize);

        /** blocking call to wait on an object from the stack with timeout*/
        void pop(std::chrono::milliseconds timeout, unsigned char* data, int maxSize);

        /** check whether there are any elements in the queue
because this is meant for multi-process applications this may or may not have any meaning
depending on the number of consumers
*/
        bool empty() const;
    };

}  // namespace ipc
}  // namespace helics
