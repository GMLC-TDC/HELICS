/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <new>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wuninitialized"
#endif

namespace helics {
class SmallBuffer {
  public:
    SmallBuffer() noexcept: heap(buffer.data()) {}

    SmallBuffer(const SmallBuffer& sb): heap(buffer.data())
    {
        resize(sb.size());
        std::memcpy(heap, sb.heap, sb.size());
    }

    SmallBuffer(SmallBuffer&& sb) noexcept
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

    template<typename U,
             typename T = std::enable_if_t<std::is_constructible_v<std::string_view, U>>>
    /*implicit*/ SmallBuffer(U&& u): heap(buffer.data())
    {
        std::string_view val(std::forward<U>(u));
        resize(val.size());
        std::memcpy(heap, val.data(), val.size());
    }

    SmallBuffer(const void* data, size_t size): heap(buffer.data())
    {
        resize(size);
        std::memcpy(heap, data, size);
    }
    /** create a buffer with a specific size*/
    explicit SmallBuffer(std::size_t size): heap(buffer.data()) { resize(size); }

    /** create a buffer with a specific size and contents*/
    SmallBuffer(std::size_t size, std::byte val): heap(buffer.data()) { resize(size, val); }
    /** create a buffer with a specific size and contents*/
    SmallBuffer(std::size_t size, unsigned char val): heap(buffer.data())
    {
        resize(size, std::byte{val});
    }
    /** destructor*/
    ~SmallBuffer()
    {
        if (usingAllocatedBuffer && !nonOwning) {
            delete[] heap;
        }
    }
    SmallBuffer& operator=(const SmallBuffer& sb)
    {
        if (this == &sb) {
            return *this;
        }
        resize(sb.size());
        std::memcpy(heap, sb.heap, sb.size());
        return *this;
    }
    SmallBuffer& operator=(SmallBuffer&& sb) noexcept
    {
        if (locked) {
            // if locked then use the copy operation not move
            const SmallBuffer& buf = sb;
            try {
                return operator=(buf);
            }
            catch (std::bad_alloc&) {
                errorCondition = 2;
                return *this;
            }
        }
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
        locked = sb.locked;
        bufferSize = sb.bufferSize;
        sb.heap = sb.buffer.data();
        sb.bufferCapacity = 64;
        sb.bufferSize = 0;
        sb.usingAllocatedBuffer = false;
        sb.locked = false;

        return *this;
    }
    template<typename U,
             typename T = std::enable_if_t<std::is_constructible_v<std::string_view, U>>>
    SmallBuffer& operator=(U&& u)
    {
        std::string_view val(std::forward<U>(u));
        if (reinterpret_cast<const std::byte*>(val.data()) == heap) {
            bufferSize = val.size();
            return *this;
        }
        resize(val.size());
        if (val.size() > 0) {
            std::memcpy(heap, val.data(), val.size());
        }
        return *this;
    }
    /** return a pointer to the data location*/
    std::byte* data() const { return heap; }
    /** get the start of the data*/
    std::byte* begin() { return heap; }
    /** end iterator*/
    std::byte* end() { return heap + bufferSize; }
    /** get a const iterator*/
    const std::byte* begin() const { return heap; }
    /** get a const end iterator*/
    const std::byte* end() const { return heap + bufferSize; }
    /** get an element*/
    std::byte operator[](size_t index) const { return heap[index]; }
    /** get an assignable reference to an element*/
    std::byte& operator[](size_t index) { return heap[index]; }
    /** get the value at a particular index with bounds checking*/
    std::byte at(size_t index) const
    {
        if (index >= bufferSize) {
            throw(std::out_of_range("specified index is not valid"));
        }
        return heap[index];
    }
    /** get a value reference at a particular index with bounds checking*/
    std::byte& at(size_t index)
    {
        if (index >= bufferSize) {
            throw(std::out_of_range("specified index is not valid"));
        }
        return heap[index];
    }
    /** assign some data to the SmallBuffer*/
    void assign(const void* start, const void* end)
    {
        if (start > end) {
            throw(
                std::invalid_argument("invalid range specified, end pointer before start pointer"));
        }
        const auto* st1 = reinterpret_cast<const std::byte*>(start);
        const auto* end1 = reinterpret_cast<const std::byte*>(end);
        resize(end1 - st1);
        std::memcpy(heap, st1, end1 - st1);
    }
    void assign(const void* start, std::size_t size)
    {
        const auto* st1 = reinterpret_cast<const std::byte*>(start);
        resize(size);
        std::memcpy(heap, st1, size);
    }
    void append(const void* start, const void* end)
    {
        if (start > end) {
            throw(
                std::invalid_argument("invalid range specified, end pointer before start pointer"));
        }
        const auto* st1 = reinterpret_cast<const std::byte*>(start);
        const auto* end1 = reinterpret_cast<const std::byte*>(end);
        auto csize = bufferSize;
        resize(bufferSize + (end1 - st1));
        std::memcpy(heap + csize, st1, end1 - st1);
    }
    void append(const void* start, std::size_t size)
    {
        const auto* st1 = reinterpret_cast<const std::byte*>(start);
        auto csize = bufferSize;
        resize(bufferSize + size);
        std::memcpy(heap + csize, st1, size);
    }
    void append(std::string_view data)
    {
        const auto* st1 = reinterpret_cast<const std::byte*>(data.data());
        auto csize = bufferSize;
        resize(bufferSize + data.size());
        std::memcpy(heap + csize, st1, data.size());
    }

    void push_back(char c) { append(&c, 1); }

    void pop_back() { bufferSize > 0 ? --bufferSize : 0; }
    /** interpret the data as a string*/
    std::string_view to_string() const
    {
        return std::string_view{reinterpret_cast<const char*>(heap), bufferSize};
    }

    /** ensure there is a null terminator after the last buffer character*/
    void null_terminate()
    {
        if (bufferCapacity > bufferSize) {
            heap[bufferSize] = std::byte(0);
        } else {
            push_back('\0');
            pop_back();
        }
    }
    /** get a pointer to the data as a `char *`*/
    const char* char_data() const { return reinterpret_cast<const char*>(heap); }
    /** move raw memory into the buffer and give it a preallocated buffer*/
    void moveAssign(void* data, std::size_t size, std::size_t capacity)
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
        locked = false;
    }
    /** use other managed memory */
    void spanAssign(void* data, std::size_t size, std::size_t capacity)
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
        locked = false;
        heap = newHeap;
        bufferCapacity = capacity;
        bufferSize = size;
        nonOwning = true;
        usingAllocatedBuffer = true;
    }
    void resize(size_t size)
    {
        reserve(size);
        bufferSize = size;
    }
    void resize(size_t size, std::byte val)
    {
        reserve(size);
        if (size > bufferSize) {
            std::memset(heap + bufferSize, std::to_integer<int>(val), size - bufferSize);
        }
        bufferSize = size;
    }
    void reserve(size_t size)
    {
        static constexpr size_t bigSize{sizeof(size_t) == 8 ? 0x010'0000'0000U : 0xFFFF'0000U};
        if (size > bufferCapacity) {
            if (size > bigSize || locked) {
                throw(std::bad_alloc());
            }
            auto* ndata = new std::byte[size + 8];
            std::memcpy(ndata, heap, bufferSize);
            if (usingAllocatedBuffer && !nonOwning) {
                delete[] heap;
            }
            heap = ndata;
            nonOwning = false;
            usingAllocatedBuffer = true;
            bufferCapacity = size + 8;
        }
    }
    void lock(bool lockStatus = true) { locked = lockStatus; }

    bool isLocked() const { return locked; }
    /** get the error condition*/
    std::int8_t errorState() const { return errorCondition; }
    /** check if the buffer is empty*/
    bool empty() const { return (bufferSize == 0); }
    /** get the current size of the buffer*/
    std::size_t size() const { return bufferSize; }
    /** get the current capacity of the buffer */
    std::size_t capacity() const { return bufferCapacity; }
    /** clear the buffer*/
    void clear() { bufferSize = 0; }

    /** swap function */
    void swap(SmallBuffer& sb2) noexcept
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
    /** release the memory from ownership */
    std::byte* release()
    {
        if (!usingAllocatedBuffer) {
            return nullptr;
        }
        auto* released = heap;
        heap = buffer.data();
        usingAllocatedBuffer = false;
        nonOwning = false;
        locked = false;
        bufferCapacity = 64;
        bufferSize = 0;
        return released;
    }

  private:
    std::array<std::byte, 64> buffer{{std::byte{0}}};
    std::size_t bufferSize{0};
    std::size_t bufferCapacity{64};
    std::byte* heap;
    bool nonOwning{false};
    bool locked{false};
    bool usingAllocatedBuffer{false};
    std::int8_t errorCondition{0};

  public:
    std::uint32_t userKey{0};  // 32 bits of user data for whatever purpose is desired has no impact
                               // on state or operations
};

/** operator to check if small buffers are equal to each other*/
inline bool operator==(const SmallBuffer& sb1, const SmallBuffer& sb2)
{
    return (sb1.to_string() == sb2.to_string());
}

/** operator to check if small buffers are not equal to each other*/
inline bool operator!=(const SmallBuffer& sb1, const SmallBuffer& sb2)
{
    return (sb1.to_string() != sb2.to_string());
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#endif

}  // namespace helics
