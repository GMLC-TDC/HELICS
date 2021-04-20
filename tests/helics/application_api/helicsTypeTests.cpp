/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include <complex>
#include <gtest/gtest.h>

/** these test cases test out the value converters
 */
#include "helics/application_api/HelicsPrimaryTypes.hpp"

using namespace std::string_literals;
using namespace helics;

TEST(helics_types, complex_empty_string)
{
    auto v = getComplexFromString(std::string{});
    EXPECT_GT(std::abs(v.real()), 1e40);
    auto v2 = getComplexFromString(std::string{});
    EXPECT_GT(std::abs(v.real()), 1e40);
    EXPECT_EQ(v, v2);
}

TEST(helics_types, complex_vector_string)
{
    auto v = getComplexFromString("[1,2]");
    EXPECT_EQ(v.real(), 1);
    EXPECT_EQ(v.imag(), 2);
    v = getComplexFromString("v[1,2]");
    EXPECT_EQ(v.real(), 1);
    EXPECT_EQ(v.imag(), 2);
    v = getComplexFromString("vector[1,2]");
    EXPECT_EQ(v.real(), 1);
    EXPECT_EQ(v.imag(), 2);
    v = getComplexFromString("v2[1,2]");
    EXPECT_EQ(v.real(), 1);
    EXPECT_EQ(v.imag(), 2);
    v = getComplexFromString("v4[1,2,3,5]");
    EXPECT_EQ(v.real(), 1);
    EXPECT_EQ(v.imag(), 2);
    v = getComplexFromString("v1[1]");
    EXPECT_EQ(v.real(), 1);
    EXPECT_EQ(v.imag(), 0);
    v = getComplexFromString("v0[]");
    EXPECT_GT(std::abs(v.real()), 1e40);
    EXPECT_EQ(v.imag(), 0);
}

TEST(helics_types, complex_cvector_string)
{
    auto v = getComplexFromString("c[1+2j]");
    EXPECT_EQ(v.real(), 1);
    EXPECT_EQ(v.imag(), 2);
    v = getComplexFromString("c1[1+2i]");
    EXPECT_EQ(v.real(), 1);
    EXPECT_EQ(v.imag(), 2);
    v = getComplexFromString("c2[1+2j,3-5j]");
    EXPECT_EQ(v.real(), 1);
    EXPECT_EQ(v.imag(), 2);
    v = getComplexFromString("complex[1+2j]");
    EXPECT_EQ(v.real(), 1);
    EXPECT_EQ(v.imag(), 2);

    v = getComplexFromString("c1[1]");
    EXPECT_EQ(v.real(), 1);
    EXPECT_EQ(v.imag(), 0);
}

TEST(helics_types, double_string)
{
    auto v = getDoubleFromString(std::string{});
    EXPECT_GT(std::abs(v), 1e40);
}

TEST(helics_types, vector_string)
{
    auto v = helicsGetVector(std::string{});
    EXPECT_TRUE(v.empty());
}

TEST(helics_types, cvector_string)
{
    auto v = helicsGetComplexVector(std::string{});
    EXPECT_TRUE(v.empty());
    v = helicsGetComplexVector("c[1+4j,2-3j,invalid]");
    ASSERT_EQ(v.size(), 3U);
    EXPECT_EQ(v[2], invalidValue<std::complex<double>>());
}
