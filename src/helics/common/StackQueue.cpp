/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "StackQueue.hpp"
#include <algorithm>

namespace helics
{
namespace common
{

StackQueueRaw::StackQueueRaw (unsigned char *newBlock, int blockSize)
    : origin (newBlock), next (newBlock), dataSize (blockSize)
{
    nextIndex = reinterpret_cast<dataIndex *> (origin + dataSize - sizeof (dataIndex));
}

void StackQueueRaw::swap (StackQueueRaw &other) noexcept
{
    std::swap (origin, other.origin);
    std::swap (next, other.next);
    std::swap (dataSize, other.dataSize);
    std::swap (nextIndex, other.nextIndex);
    std::swap (dataCount, other.dataCount);
}

bool StackQueueRaw::isSpaceAvailable (int sz) const
{
    return (dataSize - (next - origin) - (dataCount + 1) * sizeof (dataIndex)) >= sz;
}

bool StackQueueRaw::push (const unsigned char *block, int blockSize)
{
    if (blockSize <= 0)
    {
        return false;
    }
    if (!isSpaceAvailable (blockSize))
    {
        return false;
    }
    memcpy (next, block, blockSize);
    nextIndex->offset = static_cast<int> (next - origin);
    nextIndex->dataSize = blockSize;
    next += blockSize;
    --nextIndex;
    ++dataCount;
    return true;
}

int StackQueueRaw::nextDataSize () const
{
    if (dataCount > 0)
    {
        return nextIndex[1].dataSize;
    }
    return 0;
}

int StackQueueRaw::pop (unsigned char *block, int maxSize)
{
    if (dataCount > 0)
    {
        int blkSize = nextIndex[1].dataSize;
        if (maxSize >= blkSize)
        {
            memcpy (block, origin + nextIndex[1].offset, blkSize);
            if (nextIndex[1].offset + blkSize == static_cast<int> (next - origin))
            {
                next -= blkSize;
            }
            ++nextIndex;
            --dataCount;
            if (dataCount == 0)
            {
                next = origin;
                nextIndex = reinterpret_cast<dataIndex *> (origin + dataSize - sizeof (dataIndex));
            }
            return blkSize;
        }
    }
    return 0;
}

/** reverse the order in which the data will be extracted*/
void StackQueueRaw::reverse ()
{
    if (dataCount <= 1)
    {
        return;
    }
    std::reverse (nextIndex + 1, nextIndex + dataCount + 1);
}

void StackQueueRaw::clear ()
{
    next = origin;
    dataCount = 0;
    nextIndex = reinterpret_cast<dataIndex *> (origin + dataSize - sizeof (dataIndex));
}


    StackQueue::StackQueue () noexcept : stack (nullptr, 0){};
	StackQueue::StackQueue(int size) : data(size), stack(data.data(), size)
	{

	}
	
	StackQueue::StackQueue (StackQueue &&sq) noexcept : data(std::move(sq.data)),stack (std::move(sq.stack))
	{
        sq.stack.dataSize = 0;
        sq.stack.origin = nullptr;
        sq.stack.next = nullptr;
        sq.stack.dataCount = 0;
        sq.stack.nextIndex = nullptr;
	}

	StackQueue::StackQueue (const StackQueue &sq) : data (sq.data), stack (sq.stack)
    {
        auto offset = stack.next - stack.origin;
        stack.origin = data.data ();
        stack.next = stack.origin + offset;
        stack.nextIndex = reinterpret_cast<dataIndex *> (stack.origin + stack.dataSize - sizeof (dataIndex));
        stack.nextIndex -= stack.dataCount;
	}

	StackQueue &StackQueue::operator= (StackQueue &&sq) noexcept { 
		stack = std::move (sq.stack);
        data = std::move (sq.data);

		 sq.stack.dataSize = 0;
        sq.stack.origin = nullptr;
        sq.stack.next = nullptr;
        sq.stack.dataCount = 0;
        sq.stack.nextIndex = nullptr;
        return *this;
	}

	StackQueue &StackQueue::operator= (const StackQueue &sq)
	{
        stack = sq.stack;
        data = sq.data;
        auto offset = stack.next - stack.origin;
        stack.origin = data.data ();
        stack.next = stack.origin + offset;
        stack.nextIndex = reinterpret_cast<dataIndex *> (stack.origin + stack.dataSize - sizeof (dataIndex));
        stack.nextIndex -= stack.dataCount;
        return *this;
	}

	void StackQueue::resize(int newsize)
	{
		if (newsize == stack.dataSize)
		{
            return;
		}
		if (stack.dataCount == 0)
		{
            data.resize (newsize);
            stack = StackQueueRaw (data.data (), newsize);
		}
        else if (newsize>data.size())
		{
            data.resize (newsize);
            int indexOffset = stack.dataSize - sizeof (dataIndex) * stack.dataCount;
            int newOffset = newsize - sizeof (dataIndex) * stack.dataCount;
            memmove(data.data () + newOffset, data.data () + indexOffset, sizeof (dataIndex) * stack.dataCount);
            stack.dataSize = newsize;
			stack.origin = data.data ();
            stack.next = stack.origin + newsize;
            stack.nextIndex = reinterpret_cast<dataIndex *> (stack.origin + stack.dataSize - sizeof (dataIndex));
            stack.nextIndex -= stack.dataCount;
		}
		else  //smaller size
		{
            int indexOffset = stack.dataSize - sizeof (dataIndex) * stack.dataCount;
            int newOffset = newsize - sizeof (dataIndex) * stack.dataCount;
            int dataOffset = static_cast<int>(stack.next - stack.origin);
			if (newsize < dataOffset + sizeof(dataIndex) * stack.dataCount)
			{
                throw (std::runtime_error (
                  "unable to resize, current data exceeds new size, please empty stack before resizing"));
			}
            memmove (data.data () + newOffset, data.data () + indexOffset, sizeof (dataIndex) * stack.dataCount);
            stack.dataSize = newsize;
            stack.origin = data.data ();
            stack.next = stack.origin + newsize;
            stack.nextIndex = reinterpret_cast<dataIndex *> (stack.origin + stack.dataSize - sizeof (dataIndex));
            stack.nextIndex -= stack.dataCount;
            data.resize (newsize);
		}
	}

}  // namespace common
}  // namespace helics
