/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include <gtest/gtest.h>

/** these test cases test SmallBuffer
 */

#include "helics/core/SmallBuffer.hpp"

#include <string>
#include <utility>
#include <vector>

using namespace helics;

TEST(small_buffer_tests, empty)
{
    SmallBuffer buffer;
    EXPECT_EQ(buffer.size(), 0U);
    EXPECT_TRUE(buffer.empty());
    EXPECT_EQ(buffer.begin(), buffer.end());
}

TEST(small_buffer_tests, resize)
{
    SmallBuffer buffer;
    buffer.resize(35);
    EXPECT_EQ(buffer.size(), 35U);
    buffer.resize(135121);
    EXPECT_EQ(buffer.size(), 135121U);
    buffer.resize(1541);
    EXPECT_EQ(buffer.size(), 1541U);
}

TEST(small_buffer_tests, resize_assign)
{
    SmallBuffer buffer;
    buffer.resize(37, std::byte{34});
    EXPECT_EQ(buffer.size(), 37U);
    for (auto bufferElement : buffer) {
        EXPECT_EQ(bufferElement, std::byte{34});
    }

    buffer.resize(182, std::byte{36});
    EXPECT_EQ(buffer.size(), 182U);
    for (std::size_t ii = 0; ii < buffer.size(); ++ii) {
        if (ii < 37) {
            EXPECT_EQ(buffer[ii], std::byte{34});
        } else {
            EXPECT_EQ(buffer[ii], std::byte{36});
        }
    }
}

TEST(small_buffer_tests, index_assign)
{
    SmallBuffer buffer;
    buffer.resize(45);
    buffer[23] = std::byte{5};
    EXPECT_EQ(buffer[23], std::byte{5});

    buffer[23] = std::byte{17};
    EXPECT_EQ(buffer[23], std::byte{17});
    EXPECT_NE(buffer[24], std::byte{17});
}

TEST(small_buffer_tests, at)
{
    SmallBuffer buffer;
    buffer.resize(45);
    buffer.at(23) = std::byte{5};
    EXPECT_EQ(buffer[23], std::byte{5});
    EXPECT_EQ(buffer.at(23), std::byte{5});

    EXPECT_THROW(buffer.at(56), std::out_of_range);

    buffer.at(14) = std::byte{7};
    const SmallBuffer& buffer2 = buffer;

    EXPECT_EQ(buffer2.at(14), std::byte{7});
    EXPECT_EQ(buffer2[14], std::byte{7});
}

TEST(small_buffer_tests, string_construct)
{
    SmallBuffer buffer("test string 1");
    EXPECT_EQ(buffer.to_string(), "test string 1");
}

TEST(small_buffer_tests, string_construct2)
{
    SmallBuffer buffer("test111111", 5);
    EXPECT_EQ(buffer.to_string(), "test1");
}

TEST(small_buffer_tests, string_assign)
{
    SmallBuffer buffer;
    buffer = "this is a test string";
    auto ts = buffer.to_string();
    EXPECT_EQ(ts, "this is a test string");
}

TEST(small_buffer_tests, string_assign2)
{
    SmallBuffer buffer;
    std::string testStr = "this is a test 2";
    buffer = testStr;
    auto ts = buffer.to_string();
    EXPECT_EQ(ts, testStr);
}

TEST(small_buffer_tests, capacity)
{
    SmallBuffer buffer;
    buffer.reserve(450);
    EXPECT_GE(buffer.capacity(), 450U);
    buffer.reserve(12514);
    EXPECT_GE(buffer.capacity(), 12514U);
    buffer.reserve(400);
    EXPECT_GE(buffer.capacity(), 12514U);
}

TEST(small_buffer_tests, equality)
{
    SmallBuffer buffer1("string test 1");
    SmallBuffer buffer2("string test 2");
    EXPECT_FALSE(buffer1 == buffer2);
    EXPECT_TRUE(buffer1 != buffer2);
    SmallBuffer buffer3("string test 1");
    EXPECT_EQ(buffer1, buffer3);
    EXPECT_NE(buffer2, buffer3);
}

TEST(small_buffer_tests, iterators)
{
    const char* tstring = "string test 1";
    SmallBuffer buffer1(tstring, 9);

    std::string string2(reinterpret_cast<const char*>(buffer1.begin()),
                        reinterpret_cast<const char*>(buffer1.end()));
    EXPECT_EQ(string2.size(), 9U);

    const SmallBuffer& buffer2 = buffer1;
    std::string string3(reinterpret_cast<const char*>(buffer2.begin()),
                        reinterpret_cast<const char*>(buffer2.end()));
    EXPECT_EQ(string3.size(), 9U);
    // first 9 elements of the string
    EXPECT_EQ(string3, "string te");
}

TEST(small_buffer_tests, copy_constructor)
{
    SmallBuffer buffer1(std::string(400, 'a'));
    EXPECT_EQ(buffer1[234], std::byte{'a'});
    SmallBuffer buffer2(buffer1);

    EXPECT_EQ(buffer1, buffer2);
    EXPECT_EQ(buffer2[219], std::byte{'a'});
}

TEST(small_buffer_tests, copy_constructor2)
{
    SmallBuffer buffer1(std::string(16, 'a'));
    EXPECT_EQ(buffer1[13], std::byte{'a'});
    SmallBuffer buffer2(buffer1);

    EXPECT_EQ(buffer1, buffer2);
    EXPECT_EQ(buffer2[11], std::byte{'a'});
}

TEST(small_buffer_tests, copy_assign)
{
    SmallBuffer buffer1(std::string(400, 'a'));
    EXPECT_EQ(buffer1[234], std::byte{'a'});
    SmallBuffer buffer2;
    EXPECT_TRUE(buffer2.empty());
    buffer2 = buffer1;
    EXPECT_EQ(buffer1, buffer2);
    EXPECT_EQ(buffer2[219], std::byte{'a'});
}

TEST(small_buffer_tests, copy_assign2)
{
    SmallBuffer buffer1(std::string(16, 'a'));
    EXPECT_EQ(buffer1[13], std::byte{'a'});
    SmallBuffer buffer2;
    EXPECT_TRUE(buffer2.empty());
    buffer2 = buffer1;
    EXPECT_EQ(buffer1, buffer2);
    EXPECT_EQ(buffer2[11], std::byte{'a'});
}

TEST(small_buffer_tests, copy_assign_full)
{
    SmallBuffer buffer1(std::string(400, 'a'));
    EXPECT_EQ(buffer1[234], std::byte{'a'});
    SmallBuffer buffer2(std::string(36223, 'b'));
    EXPECT_EQ(buffer2.size(), 36223U);
    buffer2 = buffer1;
    EXPECT_EQ(buffer1, buffer2);
    EXPECT_EQ(buffer2[219], std::byte{'a'});
}

TEST(small_buffer_tests, copy_assign_full2)
{
    SmallBuffer buffer1(std::string(16, 'a'));
    EXPECT_EQ(buffer1[13], std::byte{'a'});
    SmallBuffer buffer2(std::string(36223, 'b'));
    EXPECT_EQ(buffer2.size(), 36223U);
    buffer2 = buffer1;
    EXPECT_EQ(buffer1, buffer2);
    EXPECT_EQ(buffer2[11], std::byte{'a'});
}

TEST(small_buffer_tests, move_constructor)
{
    SmallBuffer buffer1(std::string(400, 'a'));
    EXPECT_EQ(buffer1[234], std::byte{'a'});
    SmallBuffer buffer2(std::move(buffer1));

    EXPECT_EQ(buffer2[219], std::byte{'a'});
}

TEST(small_buffer_tests, move_assign)
{
    SmallBuffer buffer1(std::string(400, 'a'));
    EXPECT_EQ(buffer1[234], std::byte{'a'});
    SmallBuffer buffer2;
    EXPECT_TRUE(buffer2.empty());
    buffer2 = std::move(buffer1);
    EXPECT_EQ(buffer2[219], std::byte{'a'});
    EXPECT_EQ(buffer2.size(), 400U);
}

TEST(small_buffer_tests, move_assign2)
{
    SmallBuffer buffer1(std::string(16, 'a'));
    EXPECT_EQ(buffer1[13], std::byte{'a'});
    SmallBuffer buffer2;
    EXPECT_TRUE(buffer2.empty());
    buffer2 = std::move(buffer1);
    EXPECT_EQ(buffer2[11], std::byte{'a'});
}

TEST(small_buffer_tests, move_assign_full)
{
    SmallBuffer buffer1(std::string(400, 'a'));
    EXPECT_EQ(buffer1[234], std::byte{'a'});
    SmallBuffer buffer2(std::string(36223, 'b'));
    EXPECT_EQ(buffer2.size(), 36223U);
    buffer2 = std::move(buffer1);
    EXPECT_EQ(buffer2[219], std::byte{'a'});
}

TEST(small_buffer_tests, move_assign_full2)
{
    SmallBuffer buffer1(std::string(16, 'a'));
    EXPECT_EQ(buffer1[13], std::byte{'a'});
    SmallBuffer buffer2(std::string(36223, 'b'));
    EXPECT_EQ(buffer2.size(), 36223U);
    buffer2 = std::move(buffer1);
    EXPECT_EQ(buffer2[11], std::byte{'a'});
}

TEST(small_buffer_tests, move_assign_self)
{
    SmallBuffer buffer1(std::string(36214, 'e'));
    EXPECT_EQ(buffer1.size(), 36214U);
    // this is testing self move so ignore the warning about
#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wself-move"
#endif

#if defined(__GNUC__) && (__GNUC__ >= 13)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wself-move"
#endif
    buffer1 = std::move(buffer1);

#if defined(__GNUC__) && (__GNUC__ >= 13)
#    pragma GCC diagnostic pop
#endif

#if defined(__clang__)
#    pragma clang diagnostic pop
#endif

    EXPECT_EQ(buffer1[11], std::byte{'e'});  // NOLINT
}

TEST(small_buffer_tests, buffer_transfer)
{
    SmallBuffer buffer1;
    auto* buffer = new std::byte[5000];

    buffer1.moveAssign(buffer, 4567, 5000);
    EXPECT_EQ(buffer1.size(), 4567U);
    EXPECT_EQ(buffer1.capacity(), 5000U);
    buffer1[27] = std::byte{15};

    EXPECT_EQ(buffer[27], std::byte{15});
    // the SMallbuffer should take care of deletion
}

TEST(small_buffer_tests, buffer_transfer_full)
{
    SmallBuffer buffer1(std::string(2354, 'b'));
    auto* buffer = new std::byte[5000];

    buffer1.moveAssign(buffer, 4567, 5000);
    EXPECT_EQ(buffer1.size(), 4567U);
    EXPECT_EQ(buffer1.capacity(), 5000U);
    buffer1[27] = std::byte{15};

    EXPECT_EQ(buffer[27], std::byte{15});
    // the SMallbuffer should take care of deletion
}

TEST(small_buffer_tests, buffer_transfer_self_assign)
{
    SmallBuffer buffer1(std::string(2354, 'b'));

    buffer1.moveAssign(buffer1.data(), 2314, 2354);
    EXPECT_EQ(buffer1.size(), 2314U);
    EXPECT_GE(buffer1.capacity(), 2354U);
    buffer1[27] = std::byte{15};

    EXPECT_EQ(buffer1[27], std::byte{15});
    EXPECT_EQ(buffer1[20], std::byte{'b'});
    // the SMallbuffer should take care of deletion
}

TEST(small_buffer_tests, buffer_borrow)
{
    SmallBuffer buffer1;
    std::vector<std::byte> buffer;
    buffer.resize(5000);

    buffer1.spanAssign(buffer.data(), 4567, 5000);
    EXPECT_EQ(buffer1.size(), 4567U);
    EXPECT_EQ(buffer1.capacity(), 5000U);
    buffer1[27] = std::byte{15};

    EXPECT_EQ(buffer[27], std::byte{15});
    // the Smallbuffer should not delete the object
}

TEST(small_buffer_tests, buffer_borrow_full)
{
    SmallBuffer buffer1(std::string(2354, 'b'));
    std::vector<std::byte> buffer;
    buffer.resize(5000);

    buffer1.spanAssign(buffer.data(), 4567, 5000);
    EXPECT_EQ(buffer1.size(), 4567U);
    EXPECT_EQ(buffer1.capacity(), 5000U);
    buffer1[27] = std::byte{15};

    EXPECT_EQ(buffer[27], std::byte{15});
    // the Smallbuffer should not delete the object
}

TEST(small_buffer_tests, buffer_borrow_locked)
{
    SmallBuffer buffer1(std::string(2354, 'b'));
    std::vector<std::byte> buffer;
    buffer.resize(5000);

    buffer1.spanAssign(buffer.data(), 4567, 5000);
    buffer1.lock();
    EXPECT_TRUE(buffer1.isLocked());
    EXPECT_EQ(buffer1.size(), 4567U);
    EXPECT_EQ(buffer1.capacity(), 5000U);
    EXPECT_THROW(buffer1.resize(5025), std::bad_alloc);
    buffer1.lock(false);
    EXPECT_FALSE(buffer1.isLocked());
    EXPECT_NO_THROW(buffer1.resize(5025));
}

TEST(small_buffer_tests, buffer_borrow_self_assign)
{
    SmallBuffer buffer1(std::string(2354, 'b'));

    buffer1.spanAssign(buffer1.data(), 1986, 2000);
    EXPECT_EQ(buffer1.size(), 1986U);
    EXPECT_GE(buffer1.capacity(), 2000U);
    buffer1[27] = std::byte{15};

    EXPECT_EQ(buffer1[27], std::byte{15});
    EXPECT_EQ(buffer1[20], std::byte{'b'});
    // this should trigger self assignment detection
}

TEST(small_buffer_tests, assign)
{
    SmallBuffer buffer1;
    std::string string1(3634, 'g');
    buffer1.assign(string1.data(), string1.data() + string1.size());
    EXPECT_EQ(string1.size(), buffer1.size());
    EXPECT_EQ(buffer1[3333], std::byte{'g'});

    buffer1.assign(string1.data(), string1.data());
    EXPECT_EQ(buffer1.size(), 0U);
}

TEST(small_buffer_tests, assign_size)
{
    SmallBuffer buffer1;
    std::string string1(3615, 'q');
    buffer1.assign(string1.data(), string1.size());
    EXPECT_EQ(string1.size(), buffer1.size());
    EXPECT_EQ(buffer1[3333], std::byte{'q'});
}

TEST(small_buffer_tests, assign_invalid)
{
    SmallBuffer buffer1;
    std::string string1;
    buffer1.assign(string1.data(), string1.data() + string1.size());
    EXPECT_EQ(string1.size(), buffer1.size());

    EXPECT_THROW(buffer1.assign(string1.data(), string1.data() - 45), std::invalid_argument);
}

TEST(small_buffer_tests, append)
{
    SmallBuffer buffer1;
    std::string string1(3634, 'g');
    std::string string2(1516, 'k');
    buffer1.append(string1.data(), string1.data() + string1.size());
    buffer1.append(string2.data(), string2.data() + string2.size());
    EXPECT_EQ(string1.size() + string2.size(), buffer1.size());
    EXPECT_EQ(buffer1[3333], std::byte{'g'});
    EXPECT_EQ(buffer1[3634], std::byte{'k'});
}

TEST(small_buffer_tests, append_size)
{
    SmallBuffer buffer1;
    std::string string1(3634, 'r');
    std::string string2(1516, 't');
    buffer1.append(string1.data(), string1.size());
    buffer1.append(string2.data(), string2.size());
    EXPECT_EQ(string1.size() + string2.size(), buffer1.size());
    EXPECT_EQ(buffer1[3333], std::byte{'r'});
    EXPECT_EQ(buffer1[3634], std::byte{'t'});
}

TEST(small_buffer_tests, append_invalid)
{
    SmallBuffer buffer1;
    std::string string1;
    buffer1.append(string1.data(), string1.data() + string1.size());
    EXPECT_EQ(string1.size(), buffer1.size());

    EXPECT_THROW(buffer1.append(string1.data(), string1.data() - 45), std::invalid_argument);
}

TEST(small_buffer_tests, swap1)
{
    constexpr std::string_view testString("this is a test");
    SmallBuffer buffer1;
    SmallBuffer buffer2(testString);
    buffer1.swap(buffer2);
    EXPECT_TRUE(buffer2.empty());
    EXPECT_EQ(buffer1.to_string(), testString);
}

TEST(small_buffer_tests, swap2)
{
    const std::string testString(21514, 'c');
    SmallBuffer buffer1;
    SmallBuffer buffer2(testString);
    buffer1.swap(buffer2);
    EXPECT_TRUE(buffer2.empty());
    EXPECT_EQ(buffer1.to_string(), testString);
}

TEST(small_buffer_tests, swap3)
{
    const std::string testString1(21514, 'c');
    const std::string testString2(3453, 'e');
    SmallBuffer buffer1(testString1);
    SmallBuffer buffer2(testString2);
    buffer1.swap(buffer2);
    EXPECT_EQ(buffer2.to_string(), testString1);
    EXPECT_EQ(buffer1.to_string(), testString2);
}

TEST(small_buffer_tests, swap4)
{
    const std::string testString(21514, 'c');
    SmallBuffer buffer1;
    SmallBuffer buffer2(testString);
    buffer2.swap(buffer1);
    EXPECT_TRUE(buffer2.empty());
    EXPECT_EQ(buffer1.to_string(), testString);
}

TEST(small_buffer_tests, swap5)
{
    const std::string testString(21514, 'c');
    SmallBuffer buffer1;
    std::vector<std::byte> buffer;
    buffer.resize(5000);

    buffer1.spanAssign(buffer.data(), 4567, 5000);

    SmallBuffer buffer2(testString);
    buffer2.swap(buffer1);
    EXPECT_EQ(buffer2.size(), 4567U);
    EXPECT_EQ(buffer1.to_string(), testString);
}

TEST(small_buffer_tests, release)
{
    auto* buffer1 = new SmallBuffer(std::string(1562268, 'f'));

    EXPECT_EQ(buffer1->size(), 1562268U);
    (*buffer1)[57515] = std::byte{'q'};

    auto* buffer = buffer1->release();
    ASSERT_FALSE(buffer == nullptr);
    buffer[13] = std::byte{'r'};
    EXPECT_EQ(buffer[57515], std::byte{'q'});
    EXPECT_EQ(buffer[13], std::byte{'r'});
    delete buffer1;
    EXPECT_EQ(buffer[57515], std::byte{'q'});
    EXPECT_EQ(buffer[13], std::byte{'r'});
    delete[] buffer;
}
