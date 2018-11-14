/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "helics_includes/optional.hpp"
#include <chrono>
#include <vector>

namespace helics
{
namespace common
{
struct dataIndex
{
    int32_t offset;
    int32_t dataSize;
};
/** class containing the raw stackQueue implementation
@details the stackQueueRaw class operates on raw memory
it is given a memory location and uses that for the life of the queue, it does not own the memory so care must be 
taken for memory management  It operates on blocks of raw data
*/
class StackQueueRaw
{
  private:
    unsigned char *origin = nullptr;
    unsigned char *next = nullptr;
    dataIndex *nextIndex = nullptr;
    int dataSize = 0;
    int dataCount = 0;

  public:
    StackQueueRaw (unsigned char *newBlock, int blockSize);

    void swap (StackQueueRaw &other) noexcept;

    int capacity () const { return dataSize; };
    int getCurrentCount () const { return dataCount; }
    bool isSpaceAvailable (int sz) const;
    bool empty () const { return (dataCount == 0); }

    bool push (const unsigned char *block, int blockSize);

    int nextDataSize () const;

    int pop (unsigned char *block, int maxSize);

    /** reverse the order in which the data will be extracted*/
    void reverse ();
    /** clear all data from the StackQueueRaw*/
    void clear ();

  private:
    friend class StackQueue;
};

/** StackQueue manages memory for a StackQueueRaw and adds some convenience functions */
class StackQueue
{
  public:
    StackQueue () noexcept;
    explicit StackQueue (int size);
    ~StackQueue () = default;
    StackQueue (StackQueue &&sq) noexcept;
    StackQueue (const StackQueue &sq);

    StackQueue &operator= (StackQueue &&sq) noexcept;
    StackQueue &operator= (const StackQueue &sq);

    void resize (int newsize);
    int getCurrentCount () const { return stack.getCurrentCount (); }
    int capacity () const { return stack.capacity (); }
    bool isSpaceAvailable (int sz) const { return stack.isSpaceAvailable (sz); }
    bool empty () const { return stack.empty (); }

    bool push (const unsigned char *block, int blockSize) { return stack.push (block, blockSize); }

    int nextDataSize () const { return stack.nextDataSize (); }

    int pop (unsigned char *block, int maxSize) { return stack.pop (block, maxSize); }

    void reverse () { stack.reverse (); }
    void clear () { stack.clear (); }

  private:
    std::vector<unsigned char> data;
    StackQueueRaw stack;
};

}  // namespace common
}  // namespace helics
