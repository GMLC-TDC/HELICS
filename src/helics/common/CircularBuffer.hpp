/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include <chrono>
#include <vector>

namespace helics
{
namespace common
{

class CircularBufferRaw
{
  public:
    CircularBufferRaw (unsigned char *dataBlock, int capacity);

    int capacity () const { return capacity_; }
    bool isSpaceAvailable (int sz) const;
    // Return true if push was successful
    bool push (const unsigned char *data, int blockSize);
    int nextDataSize () const;
    // Return number of bytes read.
    int pop (unsigned char *data, int maxSize);
    /** check if the block is Empty or not*/
    bool empty () const;
    /** clear the buffer*/
    void clear ();

  private:
    unsigned char *origin;
    unsigned char *next_write;
    unsigned char *next_read;
    int capacity_ = 0;

  private:
    friend class CircularBuffer;
};

class CircularBuffer
{
  public:
    CircularBuffer () noexcept;
    explicit CircularBuffer (int size);
    ~CircularBuffer () = default;
    CircularBuffer (CircularBuffer &&cb) noexcept;
    CircularBuffer (const CircularBuffer &cb);

    CircularBuffer &operator= (CircularBuffer &&cb) noexcept;
    CircularBuffer &operator= (const CircularBuffer &cb);

    void resize (int newsize);
    int capacity () const { return buffer.capacity (); }
    bool isSpaceAvailable (int sz) const { return buffer.isSpaceAvailable (sz); }
    bool empty () const { return buffer.empty (); }

    bool push (const unsigned char *block, int blockSize) { return buffer.push (block, blockSize); }

    int next_data_size () const { return buffer.nextDataSize (); }

    int pop (unsigned char *block, int maxSize) { return buffer.pop (block, maxSize); }

    void clear () { buffer.clear (); }

  private:
    std::vector<unsigned char> data;
    CircularBufferRaw buffer;
};

}  // namespace common
}  // namespace helics
