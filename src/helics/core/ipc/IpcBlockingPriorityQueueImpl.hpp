/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "gmlc/containers/CircularBuffer.hpp"
#include "gmlc/containers/StackBuffer.hpp"
#include <chrono>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>

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
    const int queueSize;
    const int prioritySize;
    gmlc::containers::StackBufferRaw pushData;
    boost::interprocess::interprocess_mutex
      m_pullLock;  //!< lock for elements on the pullData and priority structure
    gmlc::containers::StackBufferRaw pullData;
    mutable boost::interprocess::interprocess_mutex m_conditionLock;  //!< lock for the empty and full Flag
    bool queueEmptyFlag{true};  //!< flag indicating the queue is empty
    bool queueFullFlag{false};

    // the condition variable should be keyed of the conditionLock
    boost::interprocess::interprocess_condition
      condition_empty;  //!< condition variable for notification of new data
    boost::interprocess::interprocess_condition
      condition_full;  //!< condition variable for notification of available space
    unsigned char *dataBlock_;
    const size_t dataSize;
    gmlc::containers::CircularBufferRaw priorityData;

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

    /** push a data block
    val the value to push on the queue
    */
    int push (std::chrono::milliseconds timeout, const unsigned char *data, int size);

    /** push an element onto the queue
    val the value to push on the queue
    */
    void pushPriority (const unsigned char *data, int size);
    /** push an element onto the queue
    @param timeout the time to wait
    @param data the data to put into the queue
    @param size the number of bytes of data to store
    @return size if successful, 0 if not
    @throws invalid_argument if the size is greater than the capacity
    */
    int pushPriority (std::chrono::milliseconds timeout, const unsigned char *data, int size);
    /** push a data block
    val the value to push on the queue
    */
    bool try_push (const unsigned char *data, int size);

    /** push an element onto the queue
    val the value to push on the queue
    */
    bool try_pushPriority (const unsigned char *data, int size);

    /** try to pop an object from the queue
    @details this function does not block,  will return 0 if no data could be popped
    @return an integer with the size of the popped value,
    */
    int try_pop (unsigned char *data, int maxSize);

    /** pop an object from the queue
    @details this function will block,  will return 0 if the data does not fit into the max size
    @return an integer with the size of the popped value,
    */
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
