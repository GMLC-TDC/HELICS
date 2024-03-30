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
    Json::Value value;
    EXPECT_NO_THROW(value = loadJsonStr(str));
    EXPECT_TRUE(value.isString());
    EXPECT_STREQ(value.asCString(), GetParam());
}

INSTANTIATE_TEST_SUITE_P(jsonString_tests, jsonStringGen_tests, ::testing::ValuesIn(test_strings));

TEST(error_generation, egen1)
{
    std::string message = "this is a message";
    int code{505};
    auto estring = generateJsonErrorResponse(static_cast<JsonErrorCodes>(code), message);
    Json::Value value;
    EXPECT_NO_THROW(value = loadJsonStr(estring));
    EXPECT_TRUE(value.isObject());

    EXPECT_TRUE(value["error"].isObject());
    EXPECT_TRUE(value["error"]["code"].isInt());
    EXPECT_EQ(value["error"]["code"].asInt(), code);
    EXPECT_STREQ(value["error"]["message"].asCString(), message.c_str());
}

TEST(error_generation, egen2)
{
    std::string message = "this is a \"quoted\" message";
    int code{-216};
    auto estring = generateJsonErrorResponse(static_cast<JsonErrorCodes>(code), message);
    Json::Value value;
    EXPECT_NO_THROW(value = loadJsonStr(estring));
    EXPECT_TRUE(value.isObject());

    EXPECT_TRUE(value["error"].isObject());
    EXPECT_TRUE(value["error"]["code"].isInt());
    EXPECT_EQ(value["error"]["code"].asInt(), code);
    EXPECT_EQ(value["error"]["message"].asString(), message);
}

TEST(looks_like_json, jsonConfig1)
{
    EXPECT_TRUE(looksLikeConfigJson(R"({"f":7})"));
    EXPECT_FALSE(looksLikeConfigJson(R"({"f"})"));
    EXPECT_FALSE(looksLikeConfigJson(R"(#{"f":7})"));
}

TEST(looks_like_json, jsonConfig2)
{
    std::ostringstream res;
    res << "// this a comment\n";
    res << "   {\n";
    res << " \"param1\" : \"value1\" \n";
    res << " \"param2\" : \"value3\" //with a comment##\n";
    res << "}";

    EXPECT_TRUE(looksLikeConfigJson(res.str()));
}

TEST(looks_like_json, jsonConfig3)
{
    std::ostringstream res;
    res << "// this a comment\n";
    res << "// this a second comment\n";
    res << "   {\n";
    res << " \"param1\" : \"value1\" \n";
    res << " \"param2\" : \"value3\" //with a comment##\n";
    res << " }  ";

    EXPECT_TRUE(looksLikeConfigJson(res.str()));
}

TEST(looks_like_json, jsonConfig4)
{
    std::ostringstream res;
    res << "// this a comment\n";
    res << "// this a second comment\n";
    res << "   {\n";
    res << " \"param1\" : \"value1\" \n";
    res << " \"param2\" : \"value3\" //with a comment##\n";
    res << " } //comment after close\n";

    EXPECT_TRUE(looksLikeConfigJson(res.str()));
}

TEST(looks_like_json, jsonConfig5)
{
    std::ostringstream res;
    res << "// this a comment\n";
    res << "// this a second comment\n";
    res << "   {\n";
    res << " \"param1\" : \"value1\" \n";
    res << " \"param2\" : \"value3\" //with a comment##\n";
    res << " } //comment after close\n";
    res << " } //second comment after close\n";

    EXPECT_TRUE(looksLikeConfigJson(res.str()));
}

TEST(looks_like_json, jsonConfig6)
{
    std::ostringstream res;
    res << "      // this a comment\n";
    res << "// this a second comment\n";
    res << "   {\n";
    res << " \"param1\" : \"value1\" \n";
    res << " \"param2\" : \"value3\" //with a comment##\n";
    res << " } //comment after close\n";
    res << " //second comment after close\n\n   \n   ";

    EXPECT_TRUE(looksLikeConfigJson(res.str()));
}
