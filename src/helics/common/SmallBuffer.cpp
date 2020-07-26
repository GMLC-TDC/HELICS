/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "SmallBuffer.hpp"

#include <cstring>

namespace helics {

SmallBuffer::SmallBuffer(const SmallBuffer& sb): heap(buffer.data())
{
    resize(sb.size());
    std::memcpy(heap, sb.heap, sb.size());
}

SmallBuffer::SmallBuffer(SmallBuffer&& sb) noexcept
{
    if (sb.usingAllocatedBuffer) {
        heap = sb.heap;
        bufferCapacity = sb.bufferCapacity;
        usingAllocatedBuffer = sb.usingAllocatedBuffer;
        nonOwning = sb.nonOwning;
        sb.usingAllocatedBuffer = false;
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

SmallBuffer::~SmallBuffer()
{
    if (usingAllocatedBuffer && !nonOwning) {
        delete[] heap;
    }
}

SmallBuffer::SmallBuffer(const void* data, size_t size): heap(buffer.data())
{
    resize(size);
    std::memcpy(heap, data, size);
}

SmallBuffer& SmallBuffer::operator=(const SmallBuffer& sb)
{
    if (this == &sb) {
        return *this;
    }
    resize(sb.size());
    std::memcpy(heap, sb.heap, sb.size());
    return *this;
}

SmallBuffer& SmallBuffer::operator=(std::string_view val)
{
    if (reinterpret_cast<const std::byte*>(val.data()) == heap) {
        bufferSize = val.size();
        return *this;
    }
    resize(val.size());
    std::memcpy(heap, val.data(), val.size());
    return *this;
}

SmallBuffer& SmallBuffer::operator=(SmallBuffer&& sb) noexcept
{
    if (usingAllocatedBuffer) {
        if (nonOwning) {
            if (sb.heap == heap) {
                bufferSize = sb.bufferSize;
                bufferCapacity = sb.bufferCapacity;
                return *this;
            }
        } else {
            if (sb.heap == heap) {
                bufferSize = sb.bufferSize;
                return *this;
            }
            delete[] heap;
        }
    }
    if (sb.usingAllocatedBuffer) {
        heap = sb.heap;
        bufferCapacity = sb.bufferCapacity;
        usingAllocatedBuffer = true;
        nonOwning = sb.nonOwning;
    } else {
        std::memcpy(buffer.data(), sb.heap, sb.bufferSize);
        usingAllocatedBuffer = false;
        nonOwning = false;
        heap = buffer.data();
        bufferCapacity = 64;
    }
    bufferSize = sb.bufferSize;
    sb.heap = sb.buffer.data();
    sb.bufferCapacity = 64;
    sb.bufferSize = 0;
    sb.usingAllocatedBuffer = false;
    return *this;
}

std::byte* SmallBuffer::release()
{
    if (!usingAllocatedBuffer) {
        return nullptr;
    }
    auto* released = heap;
    heap = buffer.data();
    usingAllocatedBuffer = false;
    nonOwning = false;
    bufferCapacity = 64;
    bufferSize = 0;
    return released;
}

void SmallBuffer::moveAssign(void* data, std::size_t size, std::size_t capacity)
{
    auto* newHeap = reinterpret_cast<std::byte*>(data);
    if (usingAllocatedBuffer && !nonOwning) {
        if (newHeap != heap) {
            delete[] heap;
        }
    }
    heap = newHeap;
    bufferCapacity = capacity;
    bufferSize = size;
    nonOwning = false;
    usingAllocatedBuffer = true;
}

void SmallBuffer::spanAssign(void* data, std::size_t size, std::size_t capacity)
{
    auto* newHeap = reinterpret_cast<std::byte*>(data);
    if (usingAllocatedBuffer && !nonOwning) {
        if (newHeap == heap) {
            // if the heaps are the same the only thing to change is the size and capacity
            // don't change the other characteristics
            bufferSize = size;
            bufferCapacity = capacity;
            return;
        }
        delete[] heap;
    }
    heap = newHeap;
    bufferCapacity = capacity;
    bufferSize = size;
    nonOwning = true;
    usingAllocatedBuffer = true;
}

void SmallBuffer::assign(const void* start, const void* end)
{
    if (start > end) {
        throw(std::invalid_argument("invalid range specified, end pointer before start pointer"));
    }
    const auto* st1 = reinterpret_cast<const std::byte*>(start);
    const auto* end1 = reinterpret_cast<const std::byte*>(end);
    resize(end1 - st1);
    std::memcpy(heap, st1, end1 - st1);
}

void SmallBuffer::assign(const void* start, std::size_t size)
{
    const auto* st1 = reinterpret_cast<const std::byte*>(start);
    resize(size);
    std::memcpy(heap, st1, size);
}

void SmallBuffer::append(const void* start, const void* end)
{
    if (start > end) {
        throw(std::invalid_argument("invalid range specified, end pointer before start pointer"));
    }
    const auto* st1 = reinterpret_cast<const std::byte*>(start);
    const auto* end1 = reinterpret_cast<const std::byte*>(end);
    auto csize = bufferSize;
    resize(bufferSize + (end1 - st1));
    std::memcpy(heap + csize, st1, end1 - st1);
}

void SmallBuffer::append(const void* start, std::size_t size)
{
    const auto* st1 = reinterpret_cast<const std::byte*>(start);
    auto csize = bufferSize;
    resize(bufferSize + size);
    std::memcpy(heap + csize, st1, size);
}

void SmallBuffer::swap(SmallBuffer& sb2) noexcept
{
    if (sb2.usingAllocatedBuffer && usingAllocatedBuffer) {
        std::swap(heap, sb2.heap);
        std::swap(nonOwning, sb2.nonOwning);
        std::swap(bufferCapacity, sb2.bufferCapacity);
        std::swap(bufferSize, sb2.bufferSize);
    } else if (usingAllocatedBuffer) {
        sb2.heap = heap;
        sb2.bufferCapacity = bufferCapacity;
        sb2.usingAllocatedBuffer = true;
        sb2.nonOwning = nonOwning;
        usingAllocatedBuffer = false;
        nonOwning = false;
        heap = buffer.data();
        bufferCapacity = 64;

        std::memcpy(heap, sb2.buffer.data(), sb2.size());
        std::swap(sb2.bufferSize, bufferSize);
    } else if (sb2.usingAllocatedBuffer) {
        heap = sb2.heap;
        bufferCapacity = sb2.bufferCapacity;
        usingAllocatedBuffer = true;
        nonOwning = sb2.nonOwning;
        sb2.usingAllocatedBuffer = false;
        sb2.nonOwning = false;
        sb2.heap = buffer.data();
        sb2.bufferCapacity = 64;

        std::memcpy(sb2.heap, buffer.data(), bufferSize);
        std::swap(sb2.bufferSize, bufferSize);
    } else {
        std::swap(sb2.buffer, buffer);
        std::swap(sb2.bufferSize, bufferSize);
    }
}
void SmallBuffer::reserve(std::size_t size)
{
    if (size > bufferCapacity) {
        auto* ndata = new std::byte[size];
        std::memcpy(ndata, heap, bufferSize);
        if (usingAllocatedBuffer && !nonOwning) {
            delete[] heap;
        }
        heap = ndata;
        nonOwning = false;
        usingAllocatedBuffer = true;
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
