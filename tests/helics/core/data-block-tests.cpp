/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "gtest/gtest.h"

/** these test cases test data_block and data_view objects
 */

#include "helics/core/core-data.hpp"

using namespace helics;
TEST(data_block_tests, simple_data_block_tests)
{
    data_block db(7);
    EXPECT_EQ(db.size(), 7u);
    // test direct assignment
    db[0] = 'h';
    db[1] = 'a';
    db[2] = 'p';
    db[3] = 'p';
    db[4] = 'y';
    db[5] = '!';
    db[6] = '!';
    EXPECT_EQ(db[3], 'p');
    EXPECT_EQ(db.to_string(), "happy!!");

    data_block db2("test_string");
    EXPECT_EQ(db2.to_string(), "test_string");
}

TEST(data_block_tests, data_block_constructor_tests)
{
    data_block db(3, 't');
    EXPECT_EQ(db.size(), 3u);

    EXPECT_EQ(db.to_string(), "ttt");

    const char* str = "this is a test string";

    data_block db2(str);
    EXPECT_EQ(db2.size(), strlen(str));
    EXPECT_EQ(db2.to_string(), str);

    data_block db3(str, 7);
    EXPECT_EQ(db3.size(), 7u);
    EXPECT_EQ(db3.to_string(), "this is");

    data_block db4(400, 'r');
    // test move constructor
    data_block db5(std::move(db4));
    EXPECT_EQ(db5.size(), 400u);
    EXPECT_EQ(db4.size(), 0u);
    // test copy constructor
    data_block db6(db5);
    EXPECT_EQ(db6.size(), db5.size());
    EXPECT_EQ(db6[324], db5[324]);
    EXPECT_EQ(db6[324], 'r');

    // build from a vector
    std::vector<char> cvector(23, 'd');
    data_block db7(cvector);
    EXPECT_EQ(db7.size(), 23u);
    EXPECT_EQ(db7[17], 'd');

    std::vector<double> dvector(10, 0.07);
    data_block db8(dvector);
    EXPECT_EQ(db8.size(), sizeof(double) * 10);
}

TEST(data_block_tests, data_block_assignment_tests)
{
    data_block db(3, 't');

    EXPECT_EQ(db.to_string(), "ttt");

    const char* str = "this is a test string";

    db = str;
    EXPECT_EQ(db.size(), strlen(str));

    // assign the partial string
    data_block db3(str, 7);
    db3 = db;
    EXPECT_TRUE(db3 == db);

    data_block db4(400, 'r');

    data_block db5(10, 'e');
    // test move constructor
    db5 = std::move(db4);
    EXPECT_EQ(db5.size(), 400u);
    EXPECT_NE(db4.size(), 400u);
}

TEST(data_block_tests, data_block_range_for_ops)
{
    data_block test1(300, 23);

    for (auto& te : test1) {
        te += 2;
    }

    EXPECT_EQ(test1[221], 25);
    int sum = 0;
    for (const auto te : test1) {
        sum += te;
    }
    EXPECT_EQ(sum, 300 * 25);
}

/** test the raw data pointer*/
TEST(data_block_tests, data_block_data)
{
    data_block test1(300, 23);

    auto rawdata = test1.data();
    rawdata[45] = '\221';

    EXPECT_EQ(test1[45], '\221');
    EXPECT_EQ(rawdata[45], '\221');
}

/** test the swap function*/
TEST(data_block_tests, data_block_swap)
{
    data_block test1(300, 23);

    data_block test2(100, 45);

    std::swap(test1, test2);
    EXPECT_EQ(test1.size(), 100u);
    EXPECT_EQ(test2.size(), 300u);
}
