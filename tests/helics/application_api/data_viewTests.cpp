/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include <gtest/gtest.h>

/** these test cases test data_block and data_view objects
 */

#include "helics/application_api/data_view.hpp"

using namespace helics;

TEST(data_view_tests, simple_tests)
{
    data_view dv("string");

    EXPECT_EQ(dv.string(), "string");

    std::string hip = "hippo";
    data_view dv2(hip);
    EXPECT_EQ(dv2.string(), "hippo");
    EXPECT_EQ(dv2.size(), hip.length());
}

TEST(data_view_tests, constructor_tests)
{
    const char* str = "this is a test string";

    data_view dv2(str);
    EXPECT_EQ(dv2.size(), strlen(str));
    EXPECT_EQ(dv2.string(), str);

    data_view dv3(str, 7);
    EXPECT_EQ(dv3.size(), 7U);
    EXPECT_EQ(dv3.string(), "this is");

    stx::string_view stv(str, 10);
    // test copy constructor
    data_view db6(stv);
    EXPECT_EQ(db6.size(), stv.size());
    EXPECT_EQ(db6[8], stv[8]);

    // build from a vector
    std::vector<char> cvector(23, 'd');
    data_view db7(cvector);
    EXPECT_EQ(db7.size(), 23U);
    EXPECT_EQ(db7[17], 'd');

    std::vector<double> dvector(10, 0.07);
    data_view db8(dvector);
    EXPECT_EQ(db8.size(), sizeof(double) * 10);
}

TEST(data_view_tests, assignment_tests)
{
    data_block db(3, 't');

    data_view dv1(db);
    const char* str = "this is a test string";
    EXPECT_EQ(dv1.size(), 3U);
    dv1 = str;
    EXPECT_EQ(dv1.size(), strlen(str));

    // assign the partial string
    data_block db3(str, 7);

    data_block db4(400, 'r');
    data_view dv4(db4);
    data_view dv5;
    // test move constructor
    dv5 = std::move(dv4);
    EXPECT_EQ(dv5.size(), 400U);
}

TEST(data_view_tests, range_for_ops)
{
    data_block test1(300, 25);
    data_view testv1(test1);

    int sum = 0;
    for (const auto te : testv1) {
        sum += te;
    }
    EXPECT_EQ(sum, 300 * 25);
}

/** test the swap function*/
TEST(data_view_tests, swap)
{
    data_block test1(300, 23);

    data_view v1(test1);
    data_block test2(100, 45);
    data_view v2(test2);
    EXPECT_EQ(v1.size(), 300U);
    EXPECT_EQ(v2.size(), 100U);
    std::swap(v1, v2);
    EXPECT_EQ(v1.size(), 100U);
    EXPECT_EQ(v2.size(), 300U);
}

/** test the swap function*/
TEST(data_view_tests, shared_ptr)
{
    auto db = std::make_shared<data_block>(400, 'r');
    data_view dv1(db);

    auto sz1 = db->size();
    auto checkel = (*db)[67];
    EXPECT_EQ(dv1.size(), sz1);
    EXPECT_EQ(dv1[67], checkel);

    EXPECT_EQ(db.use_count(), 2);
    db = nullptr;
    // should keep a valid memory
    EXPECT_EQ(dv1.size(), sz1);
    EXPECT_EQ(dv1[67], checkel);
}
