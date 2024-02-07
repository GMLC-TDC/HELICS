/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/common/JsonGeneration.hpp"
#include "helics/common/JsonProcessingFunctions.hpp"

#include <gtest/gtest.h>

using namespace helics;
using namespace helics::fileops;
class jsonStringGen_tests: public ::testing::TestWithParam<const char*> {};

static constexpr const char* test_strings[] = {"test1",
                                               "test 2",
                                               "This is a longer test",
                                               "hippity::hop",
                                               "\"string with quotes\"",
                                               "string with \t\ttab",
                                               "string with \nnewline",
                                               "string with \\'' \b\f"};

TEST_P(jsonStringGen_tests, convert_tests)
{
    auto str = generateJsonQuotedString(GetParam());
    Json::Value V;
    EXPECT_NO_THROW(V = loadJsonStr(str));
    EXPECT_TRUE(V.isString());
    EXPECT_STREQ(V.asCString(), GetParam());
}

INSTANTIATE_TEST_SUITE_P(jsonString_tests, jsonStringGen_tests, ::testing::ValuesIn(test_strings));

TEST(error_generation, egen1)
{
    std::string message = "this is a message";
    int code{505};
    auto estring = generateJsonErrorResponse(static_cast<JsonErrorCodes>(code), message);
    Json::Value V;
    EXPECT_NO_THROW(V = loadJsonStr(estring));
    EXPECT_TRUE(V.isObject());

    EXPECT_TRUE(V["error"].isObject());
    EXPECT_TRUE(V["error"]["code"].isInt());
    EXPECT_EQ(V["error"]["code"].asInt(), code);
    EXPECT_STREQ(V["error"]["message"].asCString(), message.c_str());
}

TEST(error_generation, egen2)
{
    std::string message = "this is a \"quoted\" message";
    int code{-216};
    auto estring = generateJsonErrorResponse(static_cast<JsonErrorCodes>(code), message);
    Json::Value V;
    EXPECT_NO_THROW(V = loadJsonStr(estring));
    EXPECT_TRUE(V.isObject());

    EXPECT_TRUE(V["error"].isObject());
    EXPECT_TRUE(V["error"]["code"].isInt());
    EXPECT_EQ(V["error"]["code"].asInt(), code);
    EXPECT_EQ(V["error"]["message"].asString(), message);
}
