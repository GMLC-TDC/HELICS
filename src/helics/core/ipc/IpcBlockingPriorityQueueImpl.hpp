/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "helics_includes/optional.hpp"
#include <chrono>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include "helics/common/CircularBuffer.hpp"
#include "helics/common/StackQueue.hpp"

namespace helics
{
namespace ipc
{
namespace detail
{

/** class implementing a blocking queue with a priority channel
@details this class uses locks one for push and pull it can exhibit longer blocking times if the internal
operations require a swap, however in high usage the two locks will reduce contention in most cases.
*/
    class IpcBlockingPriorityQueueImpl
    {
      private:
        boost::interprocess::interprocess_mutex m_pushLock;  //!< lock for operations on the pushElements vector
        common::StackQueueRaw pushData;
        boost::interprocess::interprocess_mutex
          m_pullLock;  //!< lock for elements on the pullData and priority structure
        common::StackQueueRaw pullData;
        mutable boost::interprocess::interprocess_mutex m_conditionLock;  //!< lock for the empty and full Flag
        bool queueEmptyFlag{true};  //!< flag indicating the queue is empty
        bool queueFullFlag{false};
        // the condition variable should be keyed of the conditionLock
        boost::interprocess::interprocess_condition
          condition_empty;  //!< condition variable for notification of new data
        boost::interprocess::interprocess_condition
          condition_full;  //!< condition variable for notification of available space
        unsigned char *dataBlock_;
        size_t dataSize;
        common::CircularBufferRaw priorityData;

      public:
        /** default constructor*/
        IpcBlockingPriorityQueueImpl (unsigned char *dataBlock, int blockSize);

        /** clear the queue*/
        void clear ();

        /** DISABLE_COPY_AND_ASSIGN */
        IpcBlockingPriorityQueueImpl (const IpcBlockingPriorityQueueImpl &) = delete;
        IpcBlockingPriorityQueueImpl &operator= (const IpcBlockingPriorityQueueImpl &) = delete;

        /** push a data block
        val the value to push on the queue
        */
        void push (const unsigned char *data, int size);
        /** push an element onto the queue
        val the value to push on the queue
        */
        void pushPriority (const unsigned char *data, int size);
        /** push a data block
        val the value to push on the queue
        */
        bool try_push (const unsigned char *data, int size);

        /** push an element onto the queue
        val the value to push on the queue
        */
        bool try_pushPriority (const unsigned char *data, int size);

        /** try to pop an object from the queue
        @return an optional containing the value if successful the optional will be empty if there is no
        element in the queue
        */
        int try_pop (unsigned char *data, int maxSize);

        int pop (unsigned char *data, int maxSize);

        /** blocking call to wait on an object from the stack with timeout*/
        int pop (std::chrono::milliseconds timeout, unsigned char *data, int maxSize);

        /** check whether there are any elements in the queue
    because this is meant for multi-process applications this may or may not have any meaning
    depending on the number of consumers
    */
        bool empty () const;
    };

}  // namespace detail
}  // namespace ipc
}  // namespace helics
