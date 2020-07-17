/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include <string>
#include <array>
#include <cstddef>
#include <string_view>


namespace helics {
class SmallBuffer {
  public:
    SmallBuffer() noexcept: heap(buffer.data()){};

    SmallBuffer(const SmallBuffer& sb);
    SmallBuffer(SmallBuffer&& sb) noexcept;
    SmallBuffer(std::string_view val);
    SmallBuffer(const void* data, size_t size);
    ~SmallBuffer();
    SmallBuffer& operator=(const SmallBuffer& sb);
    SmallBuffer& operator=(SmallBuffer&& sb) noexcept;
    SmallBuffer& operator=(std::string_view val);
    /** return a pointer to the data location*/
    std::byte* data() const { return heap;}
    /** get the start of the data*/
    std::byte* begin() { return heap; }
    /** end iterator*/
    std::byte* end() { return heap + bufferSize; }
    /** get a const iterator*/
    const std::byte* begin() const { return heap; }
    /** get a const end iterator*/
    const std::byte* end() const { return heap + bufferSize; }
    /** get an element*/
    std::byte operator[](size_t index) const{ return heap[index]; }
    /** get an assignable reference to an element*/
    std::byte & operator[](size_t index) { return heap[index]; }
    /** interpret the data as a string*/
    std::string_view to_string() const
    {
        return std::string_view{reinterpret_cast<const char*>(heap), bufferSize};
    }
    void moveAssign(void* data, std::size_t size, std::size_t capacity) { heap = reinterpret_cast<std::byte*>(data);
        bufferCapacity = capacity;
        bufferSize = size;
    }

    void spanAssign(void* data, std::size_t size, std::size_t capacity)
    {
        heap = reinterpret_cast<std::byte*>(data);
        bufferCapacity = capacity;
        bufferSize = size;
        nonOwning = true;
    }
    void resize(size_t size);
    void resize(size_t size, std::byte val);
    void reserve(size_t reserve);
    /** check if the buffer is empty*/
    bool empty() const { return (bufferSize == 0); }
    /** get the current size of the buffer*/
    std::size_t size() const { return bufferSize; }
    /** get the current capacity of the buffer */
    std::size_t capacity() const { return bufferCapacity; }
    /** clear the buffer*/
    void clear() { bufferSize = 0; }

    private:
    std::array<std::byte, 64> buffer{};
    std::size_t bufferSize{0};
    std::size_t bufferCapacity{64};
    std::byte* heap;
    bool nonOwning{false};
};


/** operator to check if small buffers are equal to eachother*/
inline bool operator==(const SmallBuffer& sb1, const SmallBuffer& sb2)
{
    return (sb1.to_string() == sb2.to_string());
}

/** operator to check if small buffers are not equal to eachother*/
inline bool operator!=(const SmallBuffer& sb1, const SmallBuffer& sb2)
{
    return (sb1.to_string() != sb2.to_string());
}
}
