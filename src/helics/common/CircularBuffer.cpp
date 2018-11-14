/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "CircularBuffer.hpp"
#include <algorithm>


namespace helics
{
namespace common
{

CircularBufferRaw::CircularBufferRaw (unsigned char *dataBlock, int blockSize)
    : origin (dataBlock), next_write (origin), next_read (origin), capacity_ (blockSize)
{
}

bool CircularBufferRaw::isSpaceAvailable (int sz) const
{
    if (next_write >= next_read)
    {
        if ((capacity_ - (next_write - origin)) >= sz + 4)
        {
            return true;
        }
        else if ((next_read - origin) >= sz + 4)
        {
            return true;
        }
        return false;
    }
    else if ((next_read - next_write) >= sz + 4)
    {
        return true;
    }
    return false;
}

// Return number of bytes written.
bool CircularBufferRaw::push (const unsigned char *data, int blockSize)
{
    if (blockSize <= 0)
    {
        return false;
    }
    if (next_write >= next_read)
    {
        if ((capacity_ - (next_write - origin)) >= blockSize + 4)
        {
            *(reinterpret_cast<int *> (next_write)) = blockSize;
            memcpy (next_write + 4, data, blockSize);
            next_write += blockSize + 4;
            // loop around if there isn't really space for another block of at least 4 bytes and the
            // next_read>origin
            if (((capacity_ - (next_write - origin)) < 8) && (next_read > origin))
            {
                next_write = origin;
            }
            return true;
        }
        else if ((next_read - origin) >= blockSize + 4)
        {
            *(reinterpret_cast<int *> (next_write)) = -1;
            *(reinterpret_cast<int *> (origin)) = blockSize;
            memcpy (origin + 4, data, blockSize);
            next_write = origin + blockSize + 4;
            return true;
        }
    }
    else if ((next_read - next_write) >= blockSize + 4)
    {
        *(reinterpret_cast<int *> (next_write)) = blockSize;
        memcpy (next_write + 4, data, blockSize);
        next_write += blockSize + 4;
        return true;
    }
    return false;
}


int CircularBufferRaw::nextDataSize() const
{
    if (next_write == next_read)
    {
        return 0;
    }
    int size = *(reinterpret_cast<int *> (next_read));
    if (size < 0)
    {
        size = *(reinterpret_cast<int *> (origin));
    }
    return size;
}

// Return number of bytes read.
int CircularBufferRaw::pop (unsigned char *data, int maxLen)
{
    if (next_write == next_read)
    {
        return 0;
    }
    int size = *(reinterpret_cast<int *> (next_read));
    if (size < 0)
    {
        next_read = origin;
        size = *(reinterpret_cast<int *> (next_read));
    }
    if (size <= maxLen)
    {
        memcpy (data, next_read + 4, size);
        next_read += size + 4;
        if ((capacity_ - (next_read - origin)) < 8)
        {
            next_read = origin;
        }
        return size;
    }
    return 0;
}
/** check if the block is Empty or not*/
bool CircularBufferRaw::empty () const { return (next_write == next_read); }
void CircularBufferRaw::clear () { next_write = next_read = origin; }


CircularBuffer::CircularBuffer () noexcept : buffer (nullptr, 0){};
CircularBuffer::CircularBuffer (int size) : data (size), buffer (data.data (), size) {}

CircularBuffer::CircularBuffer (CircularBuffer &&cb) noexcept
    : data (std::move (cb.data)), buffer (std::move (cb.buffer))
{
    cb.buffer.capacity_ = 0;
    cb.buffer.origin = nullptr;
    cb.buffer.next_read = nullptr;
    cb.buffer.next_write = nullptr;
}

CircularBuffer::CircularBuffer (const CircularBuffer &cb) : data (cb.data), buffer(cb.buffer)
{
    auto read_offset = buffer.next_read - buffer.origin;
    auto write_offset = buffer.next_write - buffer.origin;
    buffer.origin = data.data ();
    buffer.next_read=buffer.origin+read_offset;
    buffer.next_write = buffer.origin+write_offset;
}

CircularBuffer &CircularBuffer::operator= (CircularBuffer &&cb) noexcept
{
    buffer = std::move (cb.buffer);
    data = std::move (cb.data);

    cb.buffer.capacity_ = 0;
    cb.buffer.origin = nullptr;
    cb.buffer.next_read = nullptr;
    cb.buffer.next_write = nullptr;
    return *this;
}

CircularBuffer &CircularBuffer::operator= (const CircularBuffer &cb)
{
    buffer = cb.buffer;
    data = cb.data;
    auto read_offset = buffer.next_read - buffer.origin;
    auto write_offset = buffer.next_write - buffer.origin;
    buffer.origin = data.data ();
    buffer.next_read = buffer.origin + read_offset;
    buffer.next_write = buffer.origin + write_offset;
    return *this;
}

void CircularBuffer::resize (int newsize)
{
    if (newsize == buffer.capacity_)
    {
        return;
    }
    if (buffer.empty())
    {
        data.resize (newsize);
        buffer = CircularBufferRaw (data.data (), newsize);
    }
    else if (newsize > data.size ())
    {
        data.resize (newsize);
        int read_offset = static_cast<int> (buffer.next_read - buffer.origin);
        int write_offset = static_cast<int> (buffer.next_write - buffer.origin);
		if (buffer.next_read < buffer.next_write)
		{
           
            buffer.capacity_ = newsize;
            buffer.origin = data.data ();
            buffer.next_read = buffer.origin + read_offset;
            buffer.next_write = buffer.origin + write_offset;
		}
		else
		{
            int readDiff = buffer.capacity_ - read_offset;
            memmove (data.data () + newsize - readDiff, data.data () + read_offset,
                     buffer.capacity_ - read_offset);
            buffer.origin = data.data ();
            buffer.next_write = buffer.origin + write_offset;
            buffer.next_read = buffer.origin + newsize - readDiff;
            buffer.capacity_ = newsize;
		}
        
    }
    else  // smaller size
    {
        int read_offset = static_cast<int>(buffer.next_read - buffer.origin);
		if (buffer.next_read < buffer.next_write)
		{
			if (read_offset <= newsize)
			{
                data.resize (newsize);
                buffer.capacity_ = newsize;
			}
		}
		else
		{
            int write_offset = static_cast<int> (buffer.next_write - buffer.origin);
            int readDiff = buffer.capacity_ - read_offset;
			if (readDiff + write_offset < newsize)
			{
                memmove (data.data () + newsize - readDiff, data.data () + read_offset,
                         buffer.capacity_ - read_offset);
                buffer.origin = data.data ();
                buffer.next_write = buffer.origin + write_offset;
                buffer.next_read = buffer.origin + newsize - readDiff;
                buffer.capacity_ = newsize;
			}
			else
			{
                throw (std::runtime_error (
                  "unable to resize, current data exceeds new size, please empty buffer before resizing"));
			}
		}
        
    }
}

}  // namespace common
}  // namespace helics
