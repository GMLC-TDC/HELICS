/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include <cstring>

#include "SmallBuffer.hpp"

namespace helics {

    SmallBuffer::SmallBuffer(const SmallBuffer& sb): heap(buffer.data())
{
    resize(sb.size());
    std::memcpy(heap, sb.heap, sb.size());
    }

    SmallBuffer::SmallBuffer(SmallBuffer&& sb) noexcept {
        if (sb.bufferCapacity>64) {
            heap = sb.heap;
            bufferCapacity = sb.bufferCapacity;
        } else {
            std::memcpy(buffer.data(), sb.heap, sb.bufferSize);
            heap = buffer.data();
        }
        bufferSize = sb.bufferSize;
        sb.heap = sb.buffer.data();
        sb.bufferCapacity = 64;
        sb.bufferSize = 0;
    }
    SmallBuffer::SmallBuffer(std::string_view val): heap(buffer.data())
    {
        resize(val.size());
        std::memcpy(heap, val.data(), val.size());
    }

    SmallBuffer::~SmallBuffer() {
    if (bufferCapacity!=64) {
            if (!nonOwning) {
            delete[] heap;
        }
        
    }
    }


    SmallBuffer::SmallBuffer(const void* data, size_t size):
        heap(buffer.data()){
        resize(size);
        std::memcpy(heap, data, size);
    }

    SmallBuffer& SmallBuffer::operator=(const SmallBuffer& sb) {
        resize(sb.size());
        std::memcpy(heap, sb.heap, sb.size());
        return *this;
    }

    SmallBuffer& SmallBuffer::operator=(std::string_view val)
    {
        resize(val.size());
        std::memcpy(heap, val.data(), val.size());
        return *this;
    }

    SmallBuffer& SmallBuffer::operator=(SmallBuffer&& sb) noexcept {
        if (bufferCapacity>64) {
            if (!nonOwning) {
                delete[] heap;
            }
            
        }
        if (sb.bufferCapacity > 64) {
            heap = sb.heap;
            bufferCapacity = sb.bufferCapacity;
        } else {
            std::memcpy(buffer.data(), sb.heap, sb.bufferSize);
        }
        bufferSize = sb.bufferSize;
        sb.heap = sb.buffer.data();
        sb.bufferCapacity = 64;
        sb.bufferSize = 0;
        nonOwning = sb.nonOwning;
        return *this;
    }

void SmallBuffer::reserve(std::size_t size)
{
    if (size > bufferCapacity) {
        std::byte* ndata = new std::byte[size];
        
        if (bufferCapacity > 64) {
            std::memcpy(ndata, heap, bufferSize);
            if (!nonOwning) {
                delete[] heap;
            }
            
        } else {
            std::memcpy(ndata, buffer.data(), bufferSize);
        }
        
        heap = ndata;
        nonOwning = false;
        bufferCapacity = size;
    }
}

void SmallBuffer::resize(std::size_t size)
{
    reserve(size);
    bufferSize = size;
}

void SmallBuffer::resize(size_t size, std::byte val)
{
    reserve(size);
    if (size > bufferSize) {
        std::memset(heap + bufferSize, std::to_integer<int>(val), size - bufferSize);
    }
    bufferSize = size;
}
}  // namespace helics
