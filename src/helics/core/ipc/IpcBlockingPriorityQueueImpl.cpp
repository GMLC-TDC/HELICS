/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "IpcBlockingPriorityQueueImpl.hpp"
#include <algorithm>
#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/interprocess/sync/scoped_lock.hpp>

namespace helics
{
namespace ipc
{
namespace detail
{


using namespace boost::interprocess;

static constexpr int sizeAlign8(int fullSize, double fraction)
{
    return (static_cast<int> (fullSize * fraction) >> 3) * 8;
}

  /** default constructor*/
IpcBlockingPriorityQueueImpl::IpcBlockingPriorityQueueImpl (unsigned char *dataBlock, int blockSize)
    : pushData (dataBlock, sizeAlign8(blockSize,0.4)),
      pullData (dataBlock + sizeAlign8 (blockSize, 0.4), sizeAlign8 (blockSize, 0.4)),
      priorityData (dataBlock + 2*sizeAlign8(blockSize,0.4), sizeAlign8(blockSize,0.2)),
      dataBlock_ (dataBlock), dataSize (blockSize)
{
}

/** clear the queue*/
void IpcBlockingPriorityQueueImpl::clear ()
{
    scoped_lock<interprocess_mutex> pullLock (m_pullLock);  // first pullLock
    scoped_lock<interprocess_mutex> pushLock (m_pushLock);  // second pushLock
    pullData.clear ();
    pushData.clear ();
    priorityData.clear ();
    queueEmptyFlag = true;
}

bool IpcBlockingPriorityQueueImpl::try_push(const unsigned char *data, int size)
{
    scoped_lock<interprocess_mutex> pushLock (m_pushLock);  // only one lock on this branch
    if (!pushData.empty ())
    {
        return pushData.push (data, size);
    }
    else
    {
        scoped_lock<interprocess_mutex> conditionLock (m_conditionLock);
        if (queueEmptyFlag)
        {
            queueEmptyFlag = false;
            conditionLock.unlock ();
            // release the push lock so we don't get a potential deadlock condition
            pushLock.unlock ();
            // all locks released
            // no lock the pullLock
            scoped_lock<interprocess_mutex> pullLock (m_pullLock);
            conditionLock.lock ();
            queueEmptyFlag = false;  // reset the queueEmptyflag
            conditionLock.unlock ();
            if (pullData.empty ())
            {
				if (pullData.push(data, size))
				{
                    // pullLock.unlock ();
                    condition_empty.notify_all ();
                    return true;
				}
                return false;
            }
            else
            {
                pushLock.lock ();
                return pushData.push (data, size);
            }
        }
        else
        {
            return pushData.push (data, size);
        }
    }
}

/** push an element onto the queue
val the value to push on the queue
*/
bool IpcBlockingPriorityQueueImpl::try_pushPriority(const unsigned char *data, int size)
{
    scoped_lock<interprocess_mutex> conditionLock (m_conditionLock);

    if (queueEmptyFlag)
    {
        conditionLock.unlock ();
        scoped_lock<interprocess_mutex> pullLock (m_pullLock);
        conditionLock.lock ();
        queueEmptyFlag = false;  // need to set the flag again just in case after we get the lock
        conditionLock.unlock ();
		if (priorityData.push(data, size))
		{
            // pullLock.unlock ();
            condition_empty.notify_all ();
            return true;
		}
       
    }
    else
    {
        conditionLock.unlock ();
        scoped_lock<interprocess_mutex> pullLock (m_pullLock);
		if (priorityData.push(data, size))
		{
            conditionLock.lock ();
            if (queueEmptyFlag)
            {
                queueEmptyFlag = false;
                conditionLock.unlock ();
                condition_empty.notify_all ();
            }
            return true;
		}
        
    }
    return false;
}

void IpcBlockingPriorityQueueImpl::push (const unsigned char *data, int size)  // forwarding reference
{
    scoped_lock<interprocess_mutex> pushLock (m_pushLock);  // only one lock on this branch
    if (!pushData.empty ())
    {
		if (!pushData.push(data, size))
		{
            scoped_lock<interprocess_mutex> conditionLock (m_conditionLock);
            if (queueFullFlag)
            {

			}
		}
    }
    else
    {
        scoped_lock<interprocess_mutex> conditionLock (m_conditionLock);
        if (queueEmptyFlag)
        {
            queueEmptyFlag = false;
            conditionLock.unlock ();
            // release the push lock so we don't get a potential deadlock condition
            pushLock.unlock ();
            // all locks released
            // no lock the pullLock
            scoped_lock<interprocess_mutex> pullLock (m_pullLock);
            conditionLock.lock ();
            queueEmptyFlag = false;  // reset the queueEmptyflag
            conditionLock.unlock ();
            if (pullData.empty ())
            {
                pullData.push (data, size);
                // pullLock.unlock ();
                condition_empty.notify_all ();
            }
            else
            {
                pushLock.lock ();
                pushData.push (data, size);
            }
        }
        else
        {
            pushData.push (data, size);
        }
    }
}

/** push an element onto the queue
val the value to push on the queue
*/
void IpcBlockingPriorityQueueImpl::pushPriority (const unsigned char *data, int size)  // forwarding reference
{
    scoped_lock<interprocess_mutex> conditionLock (m_conditionLock);

    if (queueEmptyFlag)
    {
        conditionLock.unlock ();
        scoped_lock<interprocess_mutex> pullLock (m_pullLock);
        conditionLock.lock ();
        queueEmptyFlag = false;  // need to set the flag again just in case after we get the lock
        conditionLock.unlock ();
        priorityData.push (data, size);
        // pullLock.unlock ();
        condition_empty.notify_all ();
    }
    else
    {
        conditionLock.unlock ();
        scoped_lock<interprocess_mutex> pullLock (m_pullLock);
        priorityData.push (data, size);
        conditionLock.lock ();
        if (queueEmptyFlag)
        {
            queueEmptyFlag = false;
            conditionLock.unlock ();
            condition_empty.notify_all ();
        }
    }
}

int IpcBlockingPriorityQueueImpl::try_pop (unsigned char *data, int maxSize)
{
    scoped_lock<interprocess_mutex> pullLock (m_pullLock);
    if (!priorityData.empty ())
    {
        return priorityData.pop (data, maxSize);
    }
    if (pullData.empty ())
    {
        scoped_lock<interprocess_mutex> pushLock (m_pushLock);
        if (!pushData.empty ())
        {  // on the off chance the queue got out of sync
            pushData.swap (pullData);
            pushLock.unlock ();  // we can free the push function to accept more elements after the swap call;
            pullData.reverse ();
            int ret = pullData.pop (data, maxSize);
            if (pullData.empty ())
            {
                pushLock.lock ();  // second pushLock
                if (!pushData.empty ())  // more elements could have been added
                {  // this is the potential for slow operations
                    pushData.swap (pullData);
                    // we can free the push function to accept more elements after the swap call;
                    pushLock.unlock ();
                    pullData.reverse ();
                }
                else
                {
                    scoped_lock<interprocess_mutex> conditionLock (m_conditionLock);
                    queueEmptyFlag = true;
                }
            }
            return ret;
        }
        scoped_lock<interprocess_mutex> conditionLock (m_conditionLock);
        queueEmptyFlag = true;
        return 0;  // return the empty optional
    }
    int ret = pullData.pop (data, maxSize);
    if (pullData.empty ())
    {
        scoped_lock<interprocess_mutex> pushLock (m_pushLock);  // second PushLock
        if (!pushData.empty ())
        {  // this is the potential for slow operations
            pushData.swap (pullData);
            // we can free the push function to accept more elements after the swap call;
            pushLock.unlock ();
            pullData.reverse ();
        }
        else
        {
            scoped_lock<interprocess_mutex> conditionLock (m_conditionLock);
            queueEmptyFlag = true;
        }
    }
    return ret;
}

/** blocking call to wait on an object from the stack*/
int IpcBlockingPriorityQueueImpl::pop (unsigned char *data, int maxSize)
{
    auto val = try_pop (data, maxSize);
    if (val < 0)
    {
        return val;
    }
    while (val == 0)
    {
        scoped_lock<interprocess_mutex> pullLock (m_pullLock);
        if (!priorityData.empty ())
        {
            return priorityData.pop (data, maxSize);
        }
        if (!pullData.empty ())  // make sure we are actually empty;
        {
            return pullData.pop (data, maxSize);
        }
        condition_empty.wait (pullLock);  // now wait
        if (!priorityData.empty ())
        {
            return priorityData.pop (data, maxSize);
        }
        if (!pullData.empty ())  // make sure we are actually empty;
        {
            return pullData.pop (data, maxSize);
        }
        pullLock.unlock ();
        val = try_pop (data, maxSize);
    }
    // move the value out of the optional
    return val;
}

/** blocking call to wait on an object from the stack with timeout*/
int IpcBlockingPriorityQueueImpl::pop (std::chrono::milliseconds timeout, unsigned char *data, int maxSize)
{
    auto val = try_pop (data, maxSize);
    if (val < 0)
    {
        return val;
    }
    while (val == 0)
    {
        scoped_lock<interprocess_mutex> pullLock (m_pullLock);
        if (!priorityData.empty ())
        {
            return priorityData.pop (data, maxSize);
        }
        if (!pullData.empty ())  // make sure we are actually empty;
        {
            return pullData.pop (data, maxSize);
        }
        bool timedOut =
          condition_empty.timed_wait (pullLock, boost::posix_time::microsec_clock::universal_time () +
                                                  boost::posix_time::milliseconds (timeout.count ()));  // now wait
        if (!priorityData.empty ())
        {
            return priorityData.pop (data, maxSize);
        }
        if (!pullData.empty ())  // make sure we are actually empty;
        {
            return pullData.pop (data, maxSize);
        }

        pullLock.unlock ();
        val = try_pop (data, maxSize);
        if (!timedOut)
        {
            return val;
        }
    }
    // move the value out of the optional
    return val;
}

bool IpcBlockingPriorityQueueImpl::empty () const
{
    scoped_lock<interprocess_mutex> conditionLock (m_conditionLock);
    return queueEmptyFlag;
}

}  // namespace detail
}  // namespace ipc
}  // namespace helics
