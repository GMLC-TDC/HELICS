/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include <gtest/gtest.h>

/** these test cases test SmallBuffer
 */

#include "helics/core/SmallBuffer.hpp"

using namespace helics;

TEST(small_buffer_tests, empty)
{
    SmallBuffer sb;
    EXPECT_EQ(sb.size(), 0U);
    EXPECT_TRUE(sb.empty());
    EXPECT_EQ(sb.begin(), sb.end());
}

TEST(small_buffer_tests, resize)
{
    SmallBuffer sb;
    sb.resize(35);
    EXPECT_EQ(sb.size(), 35U);
    sb.resize(135121);
    EXPECT_EQ(sb.size(), 135121);
    sb.resize(1541);
    EXPECT_EQ(sb.size(), 1541);
}

TEST(small_buffer_tests, resize_assign)
{
    SmallBuffer sb;
    sb.resize(37, std::byte{34});
    EXPECT_EQ(sb.size(), 37U);
    for (auto be : sb) {
        EXPECT_EQ(be, std::byte{34});
    }

    sb.resize(182, std::byte{36});
    EXPECT_EQ(sb.size(), 182U);
    for (std::size_t ii = 0; ii < sb.size(); ++ii) {
        if (ii < 37) {
            EXPECT_EQ(sb[ii], std::byte{34});
        } else {
            EXPECT_EQ(sb[ii], std::byte{36});
        }
    }
}

TEST(small_buffer_tests, index_assign)
{
    SmallBuffer sb;
    sb.resize(45);
    sb[23] = std::byte{5};
    EXPECT_EQ(sb[23], std::byte{5});

    sb[23] = std::byte{17};
    EXPECT_EQ(sb[23], std::byte{17});
    EXPECT_NE(sb[24], std::byte{17});
}

TEST(small_buffer_tests, at)
{
    SmallBuffer sb;
    sb.resize(45);
    sb.at(23) = std::byte{5};
    EXPECT_EQ(sb[23], std::byte{5});
    EXPECT_EQ(sb.at(23), std::byte{5});

    EXPECT_THROW(sb.at(56), std::out_of_range);

    sb.at(14) = std::byte{7};
    const SmallBuffer& sb2 = sb;

    EXPECT_EQ(sb2.at(14), std::byte{7});
    EXPECT_EQ(sb2[14], std::byte{7});
}

TEST(small_buffer_tests, string_construct)
{
    SmallBuffer sb("test string 1");
    EXPECT_EQ(sb.to_string(), "test string 1");
}

TEST(small_buffer_tests, string_construct2)
{
    SmallBuffer sb("test111111", 5);
    EXPECT_EQ(sb.to_string(), "test1");
}

TEST(small_buffer_tests, string_assign)
{
    SmallBuffer sb;
    sb = "this is a test string";
    auto ts = sb.to_string();
    EXPECT_EQ(ts, "this is a test string");
}

TEST(small_buffer_tests, string_assign2)
{
    SmallBuffer sb;
    std::string testStr = "this is a test 2";
    sb = testStr;
    auto ts = sb.to_string();
    EXPECT_EQ(ts, testStr);
}

TEST(small_buffer_tests, capacity)
{
    SmallBuffer sb;
    sb.reserve(450);
    EXPECT_GE(sb.capacity(), 450);
    sb.reserve(12514);
    EXPECT_GE(sb.capacity(), 12514);
    sb.reserve(400);
    EXPECT_GE(sb.capacity(), 12514);
}

TEST(small_buffer_tests, equality)
{
    SmallBuffer sb1("string test 1");
    SmallBuffer sb2("string test 2");
    EXPECT_FALSE(sb1 == sb2);
    EXPECT_TRUE(sb1 != sb2);
    SmallBuffer sb3("string test 1");
    EXPECT_EQ(sb1, sb3);
    EXPECT_NE(sb2, sb3);
}

TEST(small_buffer_tests, iterators)
{
    const char* tstring = "string test 1";
    SmallBuffer sb1(tstring, 9);

    std::string b2(reinterpret_cast<const char*>(sb1.begin()),
                   reinterpret_cast<const char*>(sb1.end()));
    EXPECT_EQ(b2.size(), 9U);

    const SmallBuffer& sb2 = sb1;
    std::string b3(reinterpret_cast<const char*>(sb2.begin()),
                   reinterpret_cast<const char*>(sb2.end()));
    EXPECT_EQ(b3.size(), 9U);
    // first 9 elements of the string
    EXPECT_EQ(b3, "string te");
}

TEST(small_buffer_tests, copy_constructor)
{
    SmallBuffer sb1(std::string(400, 'a'));
    EXPECT_EQ(sb1[234], std::byte{'a'});
    SmallBuffer sb2(sb1);

    EXPECT_EQ(sb1, sb2);
    EXPECT_EQ(sb2[219], std::byte{'a'});
}

TEST(small_buffer_tests, copy_constructor2)
{
    SmallBuffer sb1(std::string(16, 'a'));
    EXPECT_EQ(sb1[13], std::byte{'a'});
    SmallBuffer sb2(sb1);

    EXPECT_EQ(sb1, sb2);
    EXPECT_EQ(sb2[11], std::byte{'a'});
}

TEST(small_buffer_tests, copy_assign)
{
    SmallBuffer sb1(std::string(400, 'a'));
    EXPECT_EQ(sb1[234], std::byte{'a'});
    SmallBuffer sb2;
    EXPECT_TRUE(sb2.empty());
    sb2 = sb1;
    EXPECT_EQ(sb1, sb2);
    EXPECT_EQ(sb2[219], std::byte{'a'});
}

TEST(small_buffer_tests, copy_assign2)
{
    SmallBuffer sb1(std::string(16, 'a'));
    EXPECT_EQ(sb1[13], std::byte{'a'});
    SmallBuffer sb2;
    EXPECT_TRUE(sb2.empty());
    sb2 = sb1;
    EXPECT_EQ(sb1, sb2);
    EXPECT_EQ(sb2[11], std::byte{'a'});
}

TEST(small_buffer_tests, copy_assign_full)
{
    SmallBuffer sb1(std::string(400, 'a'));
    EXPECT_EQ(sb1[234], std::byte{'a'});
    SmallBuffer sb2(std::string(36223, 'b'));
    EXPECT_EQ(sb2.size(), 36223);
    sb2 = sb1;
    EXPECT_EQ(sb1, sb2);
    EXPECT_EQ(sb2[219], std::byte{'a'});
}

TEST(small_buffer_tests, copy_assign_full2)
{
    SmallBuffer sb1(std::string(16, 'a'));
    EXPECT_EQ(sb1[13], std::byte{'a'});
    SmallBuffer sb2(std::string(36223, 'b'));
    EXPECT_EQ(sb2.size(), 36223);
    sb2 = sb1;
    EXPECT_EQ(sb1, sb2);
    EXPECT_EQ(sb2[11], std::byte{'a'});
}

TEST(small_buffer_tests, move_constructor)
{
    SmallBuffer sb1(std::string(400, 'a'));
    EXPECT_EQ(sb1[234], std::byte{'a'});
    SmallBuffer sb2(std::move(sb1));

    EXPECT_EQ(sb2[219], std::byte{'a'});
}

TEST(small_buffer_tests, move_assign)
{
    SmallBuffer sb1(std::string(400, 'a'));
    EXPECT_EQ(sb1[234], std::byte{'a'});
    SmallBuffer sb2;
    EXPECT_TRUE(sb2.empty());
    sb2 = std::move(sb1);
    EXPECT_EQ(sb2[219], std::byte{'a'});
    EXPECT_EQ(sb2.size(), 400U);
}

TEST(small_buffer_tests, move_assign2)
{
    SmallBuffer sb1(std::string(16, 'a'));
    EXPECT_EQ(sb1[13], std::byte{'a'});
    SmallBuffer sb2;
    EXPECT_TRUE(sb2.empty());
    sb2 = std::move(sb1);
    EXPECT_EQ(sb2[11], std::byte{'a'});
}

TEST(small_buffer_tests, move_assign_full)
{
    SmallBuffer sb1(std::string(400, 'a'));
    EXPECT_EQ(sb1[234], std::byte{'a'});
    SmallBuffer sb2(std::string(36223, 'b'));
    EXPECT_EQ(sb2.size(), 36223);
    sb2 = std::move(sb1);
    EXPECT_EQ(sb2[219], std::byte{'a'});
}

TEST(small_buffer_tests, move_assign_full2)
{
    SmallBuffer sb1(std::string(16, 'a'));
    EXPECT_EQ(sb1[13], std::byte{'a'});
    SmallBuffer sb2(std::string(36223, 'b'));
    EXPECT_EQ(sb2.size(), 36223);
    sb2 = std::move(sb1);
    EXPECT_EQ(sb2[11], std::byte{'a'});
}

TEST(small_buffer_tests, move_assign_self)
{
    SmallBuffer sb1(std::string(36214, 'e'));
    EXPECT_EQ(sb1.size(), 36214);
    sb1 = std::move(sb1);
    EXPECT_EQ(sb1[11], std::byte{'e'});  // NOLINT
}

TEST(small_buffer_tests, buffer_transfer)
{
    SmallBuffer sb1;
    auto* buffer = new std::byte[5000];

    sb1.moveAssign(buffer, 4567, 5000);
    EXPECT_EQ(sb1.size(), 4567U);
    EXPECT_EQ(sb1.capacity(), 5000);
    sb1[27] = std::byte{15};

    EXPECT_EQ(buffer[27], std::byte{15});
    // the SMallbuffer should take care of deletion
}

TEST(small_buffer_tests, buffer_transfer_full)
{
    SmallBuffer sb1(std::string(2354, 'b'));
    auto* buffer = new std::byte[5000];

    sb1.moveAssign(buffer, 4567, 5000);
    EXPECT_EQ(sb1.size(), 4567U);
    EXPECT_EQ(sb1.capacity(), 5000);
    sb1[27] = std::byte{15};

    EXPECT_EQ(buffer[27], std::byte{15});
    // the SMallbuffer should take care of deletion
}

TEST(small_buffer_tests, buffer_transfer_self_assign)
{
    SmallBuffer sb1(std::string(2354, 'b'));

    sb1.moveAssign(sb1.data(), 2314, 2354);
    EXPECT_EQ(sb1.size(), 2314U);
    EXPECT_GE(sb1.capacity(), 2354);
    sb1[27] = std::byte{15};

    EXPECT_EQ(sb1[27], std::byte{15});
    EXPECT_EQ(sb1[20], std::byte{'b'});
    // the SMallbuffer should take care of deletion
}

TEST(small_buffer_tests, buffer_borrow)
{
    SmallBuffer sb1;
    std::vector<std::byte> buffer;
    buffer.resize(5000);

    sb1.spanAssign(buffer.data(), 4567, 5000);
    EXPECT_EQ(sb1.size(), 4567U);
    EXPECT_EQ(sb1.capacity(), 5000);
    sb1[27] = std::byte{15};

    EXPECT_EQ(buffer[27], std::byte{15});
    // the Smallbuffer should not delete the object
}

TEST(small_buffer_tests, buffer_borrow_full)
{
    SmallBuffer sb1(std::string(2354, 'b'));
    std::vector<std::byte> buffer;
    buffer.resize(5000);

    sb1.spanAssign(buffer.data(), 4567, 5000);
    EXPECT_EQ(sb1.size(), 4567U);
    EXPECT_EQ(sb1.capacity(), 5000);
    sb1[27] = std::byte{15};

    EXPECT_EQ(buffer[27], std::byte{15});
    // the Smallbuffer should not delete the object
}

TEST(small_buffer_tests, buffer_borrow_self_assign)
{
    SmallBuffer sb1(std::string(2354, 'b'));

    sb1.spanAssign(sb1.data(), 1986, 2000);
    EXPECT_EQ(sb1.size(), 1986U);
    EXPECT_GE(sb1.capacity(), 2000);
    sb1[27] = std::byte{15};

    EXPECT_EQ(sb1[27], std::byte{15});
    EXPECT_EQ(sb1[20], std::byte{'b'});
    // this should trigger self assignment detection
}

TEST(small_buffer_tests, assign)
{
    SmallBuffer sb1;
    std::string t1(3634, 'g');
    sb1.assign(t1.data(), t1.data() + t1.size());
    EXPECT_EQ(t1.size(), sb1.size());
    EXPECT_EQ(sb1[3333], std::byte{'g'});

    sb1.assign(t1.data(), t1.data());
    EXPECT_EQ(sb1.size(), 0);
}

TEST(small_buffer_tests, assign_size)
{
    SmallBuffer sb1;
    std::string t1(3615, 'q');
    sb1.assign(t1.data(), t1.size());
    EXPECT_EQ(t1.size(), sb1.size());
    EXPECT_EQ(sb1[3333], std::byte{'q'});
}

TEST(small_buffer_tests, assign_invalid)
{
    SmallBuffer sb1;
    std::string t1;
    sb1.assign(t1.data(), t1.data() + t1.size());
    EXPECT_EQ(t1.size(), sb1.size());

    EXPECT_THROW(sb1.assign(t1.data(), t1.data() - 45), std::invalid_argument);
}

TEST(small_buffer_tests, append)
{
    SmallBuffer sb1;
    std::string t1(3634, 'g');
    std::string t2(1516, 'k');
    sb1.append(t1.data(), t1.data() + t1.size());
    sb1.append(t2.data(), t2.data() + t2.size());
    EXPECT_EQ(t1.size() + t2.size(), sb1.size());
    EXPECT_EQ(sb1[3333], std::byte{'g'});
    EXPECT_EQ(sb1[3634], std::byte{'k'});
}

TEST(small_buffer_tests, append_size)
{
    SmallBuffer sb1;
    std::string t1(3634, 'r');
    std::string t2(1516, 't');
    sb1.append(t1.data(), t1.size());
    sb1.append(t2.data(), t2.size());
    EXPECT_EQ(t1.size() + t2.size(), sb1.size());
    EXPECT_EQ(sb1[3333], std::byte{'r'});
    EXPECT_EQ(sb1[3634], std::byte{'t'});
}

TEST(small_buffer_tests, append_invalid)
{
    SmallBuffer sb1;
    std::string t1;
    sb1.append(t1.data(), t1.data() + t1.size());
    EXPECT_EQ(t1.size(), sb1.size());

    EXPECT_THROW(sb1.append(t1.data(), t1.data() - 45), std::invalid_argument);
}

TEST(small_buffer_tests, swap1)
{
    constexpr std::string_view testString("this is a test");
    SmallBuffer sb1;
    SmallBuffer sb2(testString);
    sb1.swap(sb2);
    EXPECT_TRUE(sb2.empty());
    EXPECT_EQ(sb1.to_string(), testString);
}

TEST(small_buffer_tests, swap2)
{
    const std::string testString(21514, 'c');
    SmallBuffer sb1;
    SmallBuffer sb2(testString);
    sb1.swap(sb2);
    EXPECT_TRUE(sb2.empty());
    EXPECT_EQ(sb1.to_string(), testString);
}

TEST(small_buffer_tests, swap3)
{
    const std::string testString1(21514, 'c');
    const std::string testString2(3453, 'e');
    SmallBuffer sb1(testString1);
    SmallBuffer sb2(testString2);
    sb1.swap(sb2);
    EXPECT_EQ(sb2.to_string(), testString1);
    EXPECT_EQ(sb1.to_string(), testString2);
}

TEST(small_buffer_tests, swap4)
{
    const std::string testString(21514, 'c');
    SmallBuffer sb1;
    SmallBuffer sb2(testString);
    sb2.swap(sb1);
    EXPECT_TRUE(sb2.empty());
    EXPECT_EQ(sb1.to_string(), testString);
}

TEST(small_buffer_tests, swap5)
{
    const std::string testString(21514, 'c');
    SmallBuffer sb1;
    std::vector<std::byte> buffer;
    buffer.resize(5000);

    sb1.spanAssign(buffer.data(), 4567, 5000);

    SmallBuffer sb2(testString);
    sb2.swap(sb1);
    EXPECT_EQ(sb2.size(), 4567U);
    EXPECT_EQ(sb1.to_string(), testString);
}

TEST(small_buffer_tests, release)
{
    auto* sb1 = new SmallBuffer(std::string(1562268, 'f'));

    EXPECT_EQ(sb1->size(), 1562268U);
    (*sb1)[57515] = std::byte{'q'};

    auto* buffer = sb1->release();

    buffer[13] = std::byte{'r'};
    EXPECT_EQ(buffer[57515], std::byte{'q'});
    EXPECT_EQ(buffer[13], std::byte{'r'});
    delete sb1;
    EXPECT_EQ(buffer[57515], std::byte{'q'});
    EXPECT_EQ(buffer[13], std::byte{'r'});
    delete[] buffer;
}
