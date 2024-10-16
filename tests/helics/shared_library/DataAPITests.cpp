/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "ctestFixtures.hpp"
#include "helics/helics.h"

#include <complex>
#include <gtest/gtest.h>
#include <string>
#include <vector>

TEST(data, create)
{
    auto buff = helicsCreateDataBuffer(500);
    EXPECT_EQ(helicsDataBufferIsValid(buff), HELICS_TRUE);

    EXPECT_EQ(helicsDataBufferSize(buff), 0);

    EXPECT_GE(helicsDataBufferCapacity(buff), 500);

    helicsDataBufferReserve(buff, 23526);

    EXPECT_GE(helicsDataBufferCapacity(buff), 23526);

    EXPECT_EQ(helicsDataBufferSize(buff), 0);

    helicsDataBufferFree(buff);
}

TEST(data, wrap)
{
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

TEST(data, toFromInt)
{
    auto buff = helicsCreateDataBuffer(500);

    int64_t v1 = 35;
    auto cnt = helicsDataBufferFillFromInteger(buff, v1);
    EXPECT_EQ(helicsDataBufferType(buff), HELICS_DATA_TYPE_INT);
    EXPECT_GT(cnt, 0);
    int64_t v2 = helicsDataBufferToInteger(buff);
    EXPECT_EQ(v1, v2);
    helicsDataBufferFree(buff);
}

TEST(data, toFromDouble)
{
    auto buff = helicsCreateDataBuffer(500);

    double v1 = 35.7;
    auto cnt = helicsDataBufferFillFromDouble(buff, v1);
    EXPECT_EQ(helicsDataBufferType(buff), HELICS_DATA_TYPE_DOUBLE);
    EXPECT_GT(cnt, 0);
    double v2 = helicsDataBufferToDouble(buff);
    EXPECT_EQ(v1, v2);
    helicsDataBufferFree(buff);
}

TEST(data, toFromChar)
{
    auto buff = helicsCreateDataBuffer(500);

    char v1 = 'q';
    auto cnt = helicsDataBufferFillFromChar(buff, v1);
    EXPECT_GT(cnt, 0);
    double v2 = helicsDataBufferToChar(buff);
    EXPECT_EQ(v1, v2);
    helicsDataBufferFree(buff);
}

TEST(data, toFromTime)
{
    auto buff = helicsCreateDataBuffer(500);

    HelicsTime v1 = 12.77;
    auto cnt = helicsDataBufferFillFromTime(buff, v1);
    EXPECT_GT(cnt, 0);
    HelicsTime v2 = helicsDataBufferToTime(buff);
    EXPECT_EQ(v1, v2);
    helicsDataBufferFree(buff);
}

TEST(data, toFromString)
{
    auto buff = helicsCreateDataBuffer(500);

    std::string v1 = "this is an interesting string";
    auto cnt = helicsDataBufferFillFromString(buff, v1.c_str());
    EXPECT_GT(cnt, 0);
    std::string v2;
    v2.resize(100);
    int asize{0};
    EXPECT_EQ(helicsDataBufferType(buff), HELICS_DATA_TYPE_STRING);
    EXPECT_EQ(static_cast<int>(v1.size()), helicsDataBufferStringSize(buff) - 1);
    helicsDataBufferToString(buff, v2.data(), 100, &asize);
    EXPECT_EQ(asize, v1.size());
    v2.resize(asize);
    EXPECT_STREQ(v1.c_str(), v2.c_str());

    helicsDataBufferToString(buff, v2.data(), 11, &asize);
    EXPECT_EQ(asize, 10);
    v2.resize(asize);
    EXPECT_STREQ("this is an", v2.c_str());

    helicsDataBufferFree(buff);
}

TEST(data, toFromRawString)
{
    auto buff = helicsCreateDataBuffer(500);

    std::string v1 = "this is an interesting";
    v1.push_back('\0');
    v1.append(" string ");
    auto cnt = helicsDataBufferFillFromRawString(buff, v1.c_str(), static_cast<int>(v1.size()));
    EXPECT_GT(cnt, 0);
    std::string v2;
    v2.resize(100);
    int asize{0};
    EXPECT_EQ(helicsDataBufferType(buff), HELICS_DATA_TYPE_STRING);
    helicsDataBufferToRawString(buff, v2.data(), 100, &asize);
    EXPECT_EQ(asize, v1.size());
    v2.resize(asize);
    EXPECT_EQ(v1, v2);
    helicsDataBufferFree(buff);
}

TEST(data, toFromVector)
{
    auto buff = helicsCreateDataBuffer(500);

    std::vector<double> v1{34.7, -99.99999, 0, 43.7e231, std::nan("0")};
    auto cnt = helicsDataBufferFillFromVector(buff, v1.data(), static_cast<int>(v1.size()));
    EXPECT_GT(cnt, 0);
    std::vector<double> v2;
    v2.resize(5);
    int asize{0};
    EXPECT_EQ(helicsDataBufferType(buff), HELICS_DATA_TYPE_VECTOR);
    EXPECT_EQ(static_cast<int>(v1.size()), helicsDataBufferVectorSize(buff));
    helicsDataBufferToVector(buff, v2.data(), 5, &asize);
    EXPECT_EQ(asize, v1.size());
    EXPECT_EQ(v1[0], v2[0]);
    EXPECT_EQ(v1[1], v2[1]);
    EXPECT_EQ(v1[2], v2[2]);
    EXPECT_EQ(v1[3], v2[3]);
    EXPECT_TRUE(std::isnan(v2[4]));
    helicsDataBufferFree(buff);
}

#if defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wstrict-aliasing"
// std::complex is explicitly allowed to alias like this in the standard
#endif

TEST(data, toFromComplexVector)
{
    auto buff = helicsCreateDataBuffer(500);
    using cv = std::complex<double>;

    std::vector<cv> v1{cv{34.7, -19.7},
                       cv{-99.99999, 0.0000001},
                       cv{0, 0},
                       cv{43.7e231, -19.3e-88},
                       cv{std::nan("0"), 0}};
    auto cnt = helicsDataBufferFillFromComplexVector(buff,
                                                     reinterpret_cast<double*>(v1.data()),
                                                     static_cast<int>(v1.size()));
    EXPECT_GT(cnt, 0);
    std::vector<double> v2;
    v2.resize(10);
    int asize{0};
    EXPECT_EQ(helicsDataBufferType(buff), HELICS_DATA_TYPE_COMPLEX_VECTOR);
    EXPECT_EQ(static_cast<int>(v1.size()), helicsDataBufferVectorSize(buff));
    helicsDataBufferToComplexVector(buff, v2.data(), 5, &asize);
    EXPECT_EQ(asize, v1.size());
    EXPECT_EQ(v1[0].real(), v2[0]);
    EXPECT_EQ(v1[0].imag(), v2[1]);
    EXPECT_EQ(v1[1].real(), v2[2]);
    EXPECT_EQ(v1[1].imag(), v2[3]);
    EXPECT_EQ(v1[2].real(), v2[4]);
    EXPECT_EQ(v1[2].imag(), v2[5]);
    EXPECT_EQ(v1[3].real(), v2[6]);
    EXPECT_EQ(v1[3].imag(), v2[7]);
    EXPECT_TRUE(std::isnan(v2[8]));
    EXPECT_EQ(v1[4].imag(), v2[9]);
    helicsDataBufferFree(buff);
}

#if defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

TEST(data, toFromNamedPoint)
{
    auto buff = helicsCreateDataBuffer(500);

    auto cnt = helicsDataBufferFillFromNamedPoint(buff, "string_thing", 45.7);
    EXPECT_GT(cnt, 0);
    std::string v2name;
    double v2val;
    v2name.resize(15);
    int asize{0};
    EXPECT_EQ(helicsDataBufferType(buff), HELICS_DATA_TYPE_NAMED_POINT);

    helicsDataBufferToNamedPoint(buff, v2name.data(), 15, &asize, &v2val);
    EXPECT_STREQ(v2name.c_str(), "string_thing");
    EXPECT_EQ(v2val, 45.7);
    helicsDataBufferFree(buff);
}

TEST(data, converter)
{
    auto buff = helicsCreateDataBuffer(500);

    double v1 = 35.7;
    auto cnt = helicsDataBufferFillFromDouble(buff, v1);
    EXPECT_EQ(helicsDataBufferType(buff), HELICS_DATA_TYPE_DOUBLE);
    EXPECT_GT(cnt, 0);
    bool res = helicsDataBufferConvertToType(buff, HELICS_DATA_TYPE_INT) != HELICS_FALSE;
    EXPECT_TRUE(res);

    EXPECT_EQ(helicsDataBufferType(buff), HELICS_DATA_TYPE_INT);
    double v2 = helicsDataBufferToDouble(buff);
    EXPECT_EQ(35.0, v2);
    helicsDataBufferFree(buff);
}

TEST(data, clone)
{
    auto buff = helicsCreateDataBuffer(500);

    double v1 = 35.7;
    auto cnt = helicsDataBufferFillFromDouble(buff, v1);
    EXPECT_EQ(helicsDataBufferType(buff), HELICS_DATA_TYPE_DOUBLE);
    EXPECT_GT(cnt, 0);
    auto newbuff = helicsDataBufferClone(buff);

    bool res = helicsDataBufferConvertToType(buff, HELICS_DATA_TYPE_INT) != HELICS_FALSE;
    EXPECT_TRUE(res);

    EXPECT_EQ(helicsDataBufferType(buff), HELICS_DATA_TYPE_INT);
    EXPECT_EQ(helicsDataBufferType(newbuff), HELICS_DATA_TYPE_DOUBLE);
    double v2 = helicsDataBufferToDouble(buff);
    EXPECT_EQ(35.0, v2);
    helicsDataBufferFree(buff);

    v2 = helicsDataBufferToDouble(newbuff);
    EXPECT_EQ(v1, v2);
    helicsDataBufferFree(newbuff);
}
