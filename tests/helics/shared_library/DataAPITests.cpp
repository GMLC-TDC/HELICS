/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/helics.h"
#include "ctestFixtures.hpp"

#include <gtest/gtest.h>

TEST(data, create) {
    auto buff = helicsCreateDataBuffer(500);
    EXPECT_EQ(helicsDataBufferIsValid(buff), HELICS_TRUE);

    EXPECT_EQ(helicsDataBufferSize(buff), 0);

    EXPECT_GE(helicsDataBufferCapacity(buff), 500);

    helicsDataBufferReserve(buff,23526);

    EXPECT_GE(helicsDataBufferCapacity(buff), 23526);

     EXPECT_EQ(helicsDataBufferSize(buff), 0);

    helicsDataBufferFree(buff);
}

TEST(data, wrap) {
    std::vector<char> dblock(594, 'a');
    auto wrap = helicsWrapDataInBuffer(dblock.data(), 594, 594);
    EXPECT_EQ(helicsDataBufferIsValid(wrap), HELICS_TRUE);
    EXPECT_EQ(helicsDataBufferSize(wrap), 594);

    EXPECT_EQ(helicsDataBufferCapacity(wrap), 594);

    const auto* c = reinterpret_cast<char*>(helicsDataBufferData(wrap));
    EXPECT_EQ(c, dblock.data());
    helicsDataBufferFree(wrap);
    EXPECT_EQ(dblock[479], 'a');
}


TEST(data, toFromInt) {
    auto buff = helicsCreateDataBuffer(500);

    int64_t v1 = 35;
    auto cnt=helicsIntToBytes(v1, buff);
    EXPECT_EQ(helicsDataBufferType(buff), HELICS_DATA_TYPE_INT);
    EXPECT_GT(cnt, 0);
    int64_t v2 = helicsDataBufferToInt(buff);
    EXPECT_EQ(v1, v2);
}


TEST(data, toFromDouble)
{
    auto buff = helicsCreateDataBuffer(500);

    double v1 = 35.7;
    auto cnt = helicsDoubleToBytes(v1, buff);
    EXPECT_EQ(helicsDataBufferType(buff), HELICS_DATA_TYPE_DOUBLE);
    EXPECT_GT(cnt, 0);
    double v2 = helicsDataBufferToDouble(buff);
    EXPECT_EQ(v1, v2);
}

TEST(data, toFromChar)
{
    auto buff = helicsCreateDataBuffer(500);

    char v1 = 'q';
    auto cnt = helicsCharToBytes(v1, buff);
    EXPECT_GT(cnt, 0);
    double v2 = helicsDataBufferToChar(buff);
    EXPECT_EQ(v1, v2);
}

TEST(data, toFromTime)
{
    auto buff = helicsCreateDataBuffer(500);

    HelicsTime v1 = 12.77;
    auto cnt = helicsTimeToBytes(v1, buff);
    EXPECT_GT(cnt, 0);
    HelicsTime v2 = helicsDataBufferToTime(buff);
    EXPECT_EQ(v1, v2);
}


TEST(data, toFromString)
{
    auto buff = helicsCreateDataBuffer(500);

    std::string v1 = "this is an interesting string";
    auto cnt = helicsStringToBytes(v1.c_str(), buff);
    EXPECT_GT(cnt, 0);
    std::string v2;
    v2.resize(100);
    int asize{0};
    EXPECT_EQ(helicsDataBufferType(buff), HELICS_DATA_TYPE_STRING);
    EXPECT_EQ(static_cast<int>(v1.size()), helicsDataBufferStringSize(buff));
    helicsDataBufferToString(buff, v2.data(), 100,&asize);
    EXPECT_EQ(asize, v1.size());
    v2.resize(asize);
    EXPECT_STREQ(v1.c_str(), v2.c_str());
}


TEST(data, toFromRawString)
{
    auto buff = helicsCreateDataBuffer(500);

    std::string v1 = "this is an interesting string";
    auto cnt = helicsRawStringToBytes(v1.c_str(),v1.size(), buff);
    EXPECT_GT(cnt, 0);
    std::string v2;
    v2.resize(100);
    int asize{0};
    EXPECT_EQ(helicsDataBufferType(buff), HELICS_DATA_TYPE_STRING);
    EXPECT_EQ(static_cast<int>(v1.size()), helicsDataBufferStringSize(buff));
    helicsDataBufferToString(buff, v2.data(), 100, &asize);
    EXPECT_EQ(asize, v1.size());
    v2.resize(asize);
    EXPECT_STREQ(v1.c_str(), v2.c_str());
}
