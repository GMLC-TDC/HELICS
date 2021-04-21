/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include <complex>
#include <gtest/gtest.h>
#include <list>
#include <set>

/** these test cases test out the value converters
 */
#include "helics/application_api/ValueConverter.hpp"
#include "helics/application_api/ValueConverter_impl.hpp"
#include "helics/application_api/data_view.hpp"
#include "helics/core/core-data.hpp"

using namespace std::string_literals;

template<class X>
void converterTests(const X& testValue1,
                    const X& testValue2,
                    size_t sz1 = 0,
                    size_t sz2 = 0,
                    const std::string& type = "")
{
    auto converter = helics::ValueConverter<X>();

    // check the type
    if (!type.empty()) {
        auto typeString = converter.type();
        EXPECT_EQ(type, typeString);
    }

    // convert to a data view
    auto dv = converter.convert(testValue1);
    if (sz1 > 0) {
        EXPECT_EQ(dv.size(), sz1);
    }

    // convert back to a value
    auto res = converter.interpret(dv);
    EXPECT_TRUE(res == testValue1);
    // convert back to a value in a different way
    X res2;
    converter.interpret(dv, res2);
    EXPECT_TRUE(res2 == testValue1);

    helics::data_block db;

    converter.convert(testValue2, db);
    if (sz2 > 0) {
        EXPECT_EQ(db.size(), sz2);
    }

    auto res3 = converter.interpret(db);
    EXPECT_TRUE(res3 == testValue2);
}

TEST(valueConverter_tests, basic)
{
    converterTests(45.54, 23.7e-7, sizeof(double) + 1, sizeof(double) + 1, "double");
    converterTests<int>(45, -234252, sizeof(int) + 1, sizeof(int) + 1, "int32");
    converterTests<uint64_t>(
        352, 0x2323427FA, sizeof(uint64_t) + 1, sizeof(uint64_t) + 1, "uint64");

    converterTests('r', 't', 2, 2, "char");

    converterTests(static_cast<unsigned char>(223), static_cast<unsigned char>(46), 2, 2, "uchar");

    using compd = std::complex<double>;
    compd v1{1.7, 0.9};
    compd v2{-34e5, 0.345};
    converterTests(v1, v2, sizeof(compd) + 1, sizeof(compd) + 1, "complex");
    std::string testValue1 = "this is a long string test";
    std::string test2;
    converterTests(testValue1, test2, testValue1.size(), test2.size(), "string");
    // test a vector
    using vecd = std::vector<double>;
    vecd vec1 = {45.4, 23.4, -45.2, 34.2234234};
    vecd testv2(234, 0.45);
    converterTests<vecd>(vec1, testv2, 0, 0, "double_vector");
}
TEST(valueConverter_tests, named_point)
{
    helics::NamedPoint A{"tests", 47.676};
    helics::NamedPoint B{"this is a long string test", 99.345345};
    converterTests<helics::NamedPoint>(A, B, A.name.size() + 17, B.name.size() + 17, "named_point");

    helics::NamedPoint C{"", std::nan("0")};
    helics::NamedPoint D{"0", 99.345345};
    converterTests<helics::NamedPoint>(C, D, C.name.size() + 17, D.name.size() + 17, "named_point");
}

TEST(valueConverter_tests, traits)
{
    EXPECT_TRUE(helics::is_vector<std::vector<double>>::value == true);
    EXPECT_TRUE(helics::is_vector<std::vector<std::complex<double>>>::value == true);
    EXPECT_TRUE(helics::is_vector<std::string>::value == false);
    EXPECT_TRUE(helics::is_vector<double>::value == false);

    EXPECT_TRUE(helics::is_iterable<std::vector<std::complex<double>>>::value == true);
    EXPECT_TRUE(helics::is_iterable<std::string>::value == true);
    EXPECT_TRUE(helics::is_iterable<double>::value == false);

    EXPECT_TRUE(helics::is_iterable<std::vector<std::string>>::value == true);
    EXPECT_TRUE(helics::is_iterable<std::list<std::string>>::value == true);
    EXPECT_TRUE(helics::is_iterable<std::list<double>>::value == true);
    EXPECT_TRUE(helics::is_iterable<std::set<std::string>>::value == true);
    EXPECT_TRUE(helics::is_iterable<std::set<double>>::value == true);
    EXPECT_TRUE(helics::is_iterable<int>::value == false);
}

TEST(valueConverter_tests, minSize)
{
    EXPECT_EQ(helics::getMinSize<std::vector<double>>(), 9u);
    EXPECT_EQ(helics::getMinSize<double>(), sizeof(double) + 1);
    EXPECT_EQ(helics::getMinSize<int>(), sizeof(int) + 1);
    EXPECT_EQ(helics::getMinSize<std::complex<double>>(), sizeof(std::complex<double>) + 1);
    EXPECT_EQ(helics::getMinSize<std::string>(), 0u);
    EXPECT_EQ(helics::getMinSize<const char*>(), 0u);
    EXPECT_EQ(helics::getMinSize<std::set<double>>(), 9u);
    EXPECT_EQ(helics::getMinSize<helics::NamedPoint>(), 10u);
}

/** this one is a bit annoying to use the template so it gets its own case
we are testing vectors of strings
*/
TEST(valueConverter_tests, vector_string)
{
    using vecstr = std::vector<std::string>;
    using converter = helics::ValueConverter<vecstr>;

    vecstr testValue1 = {"test1", "test45", "this is a longer string to test", ""};
    // check the type
    auto type = converter::type();
    EXPECT_EQ(type, "string_vector");

    // convert to a data view
    auto dv = converter::convert(testValue1);
    // convert back to a vector
    auto val = converter::interpret(dv);
    EXPECT_TRUE(val == testValue1);
    // convert back to a string in a different way
    vecstr val2;
    converter::interpret(dv, val2);
    EXPECT_TRUE(val2 == testValue1);

    vecstr test2{
        "test string 1",
        "*SDFSDF*JJ\nSSFSDsdkjflsdjflsdkfjlskdbnowhfoihfoai\0shfoaishfoasifhaofsihaoifhaosifhaosfihaosfihaosfihaohoaihsfiohoh"s};
    helics::data_block db;

    converter::convert(test2, db);

    auto val3 = converter::interpret(db);
    EXPECT_TRUE(val3 == test2);
}

TEST(valueConverter_tests, block_vectors)
{
    using vecblock = std::vector<helics::data_block>;
    using converter = helics::ValueConverter<vecblock>;

    vecblock vb(4);
    vb[0] = helics::data_block(437, '<');
    vb[1] =
        "*SDFSDF*JJ\nSSFSDsdkjflsdjflsdkfjlskdbnowhfoihfoaishfoai\0shfoasifhaofsihaoifhaosifhaosfihaosfihaosfihaohoaihsfiohoh"s;
    vb[2] = helics::ValueConverter<double>::convert(3.1415);
    vb[3] = helics::ValueConverter<int>::convert(9999);

    auto rb = converter::convert(vb);

    auto res = helics::ValueConverter<std::vector<helics::data_view>>::interpret(rb);

    ASSERT_EQ(res.size(), vb.size());
    EXPECT_EQ(res[0].size(), vb[0].size());
    EXPECT_EQ(res[0][5], vb[0][5]);

    EXPECT_TRUE(res[1].string() == vb[1].to_string());

    EXPECT_EQ(3.1415, helics::ValueConverter<double>::interpret(res[2]));
    EXPECT_EQ(9999, helics::ValueConverter<int>::interpret(res[3]));

    auto res2 = helics::ValueConverter<std::vector<helics::data_block>>::interpret(rb);

    ASSERT_EQ(res2.size(), vb.size());
    EXPECT_EQ(res2[0].size(), vb[0].size());
    EXPECT_EQ(res2[0][5], vb[0][5]);

    EXPECT_TRUE(res2[1].to_string() == vb[1].to_string());

    EXPECT_EQ(3.1415, helics::ValueConverter<double>::interpret(res2[2]));
    EXPECT_EQ(9999, helics::ValueConverter<int>::interpret(res2[3]));
}

/** check that the converters do actually throw on invalid sizes*/
TEST(valueConverter_tests, errors)
{
    auto vb1 = helics::ValueConverter<double>::convert(3.1415);
    auto vb2 = helics::ValueConverter<int>::convert(10);

    EXPECT_THROW(helics::ValueConverter<double>::interpret(vb2), std::invalid_argument);
    EXPECT_LT(vb2.size(), 8u);
    EXPECT_GT(vb2.size(), 4u);
    EXPECT_THROW(helics::ValueConverter<std::complex<double>>::interpret(vb1),
                 std::invalid_argument);
    EXPECT_LT(vb1.size(), 12u);
    EXPECT_GT(vb1.size(), 8u);
}
