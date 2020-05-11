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
#include <chrono>
#include <utility>

namespace helics {
namespace ipc {
    namespace detail {
        struct dataIndex {
            int32_t offset;
            int32_t dataSize;
        };
        /** class containing the raw data block implementation*/
        class dataBlock {
          private:
            unsigned char* origin = nullptr;
            unsigned char* next = nullptr;
            size_t totalSize = 0;
            dataIndex* next = nullptr;
            int dataCount = 0;

          public:
            dataBlock(unsigned char* newBlock, size_t blockSize);

            void swap(dataBlock& other) noexcept;

            int getCurrentCount() const { return dataCount; }
            bool isSpaceAvaialble(int sz) const;
            bool empty() const { return (dataCount == 0); }

            bool push(const unsigned char* block, int blockSize);

            int next_data_size() const;

            int pop(unsigned char* block);

            /** reverse the order in which the data will be extracted*/
            void reverse();
            /** clear all data from the dataBlock*/
            void clear();
        };
        /** class implementing a blocking queue with a priority channel
@details this class uses locks one for push and pull it can exhibit longer blocking times if the
internal operations require a swap, however in high usage the two locks will reduce contention in
most cases.
*/
        class IpcBlockingPriorityQueueImpl {
          private:
            mutable boost::interprocess::interprocess_mutex
                m_pushLock;  //!< lock for operations on the pushElements vector
            dataBlock pushData;
            mutable boost::interprocess::interprocess_mutex
                m_pullLock;  //!< lock for elements on the pullLock vector
            dataBlock pullData;
            mutable boost::interprocess::interprocess_mutex
                m_conditionLock;  //!< lock for the empty and full Flag
            bool queueEmptyFlag{true};  //!< flag indicating the queue is empty
            bool queueFullFlag{false};
            // the condition variable should be keyed of the conditionLock
            boost::interprocess::interprocess_condition
                condition_empty;  //!< condition variable for notification of new data
            boost::interprocess::interprocess_condition
                condition_full;  //!< condition variable for notification of available space
            unsigned char* data1 = nullptr;
            unsigned char* data2 = nullptr;
            size_t data1Size = 0;
            size_t data2Size = 0;
            unsigned char* dataPriority = nullptr;
            size_t dataPrioritySize = 0;

          public:
            /** default constructor*/
            IpcBlockingPriorityQueueImpl(void* dataBlock, size_t blockSize);

            /** clear the queue*/
            void clear();

            /** DISABLE_COPY_AND_ASSIGN */
            IpcBlockingPriorityQueueImpl(const IpcBlockingPriorityQueueImpl&) = delete;
            IpcBlockingPriorityQueueImpl& operator=(const IpcBlockingPriorityQueueImpl&) = delete;

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

    }  // namespace detail
}  // namespace ipc
}  // namespace helics
