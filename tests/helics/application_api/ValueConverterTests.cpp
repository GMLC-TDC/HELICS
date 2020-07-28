/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include <algorithm>
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

using compd = std::complex<double>;

TEST(valueConverter_tests, basic)
{
    converterTests(45.54, 23.7e-7, sizeof(double) + 1, sizeof(double) + 1, "double");
    converterTests<int>(45, -234252, sizeof(int) + 1, sizeof(int) + 1, "int32");
    converterTests<uint64_t>(
        352, 0x2323427FA, sizeof(uint64_t) + 1, sizeof(uint64_t) + 1, "uint64");

    converterTests('r', 't', 2, 2, "char");

    converterTests(static_cast<unsigned char>(223), static_cast<unsigned char>(46), 2, 2, "uchar");

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
    EXPECT_EQ(helics::getMinSize<std::vector<double>>(), 9U);
    EXPECT_EQ(helics::getMinSize<double>(), sizeof(double) + 1);
    EXPECT_EQ(helics::getMinSize<int>(), sizeof(int) + 1);
    EXPECT_EQ(helics::getMinSize<std::complex<double>>(), sizeof(std::complex<double>) + 1);
    EXPECT_EQ(helics::getMinSize<std::string>(), 0U);
    EXPECT_EQ(helics::getMinSize<const char*>(), 0U);
    EXPECT_EQ(helics::getMinSize<std::set<double>>(), 9U);
    EXPECT_EQ(helics::getMinSize<helics::NamedPoint>(), 10U);
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
    EXPECT_LT(vb2.size(), 8U);
    EXPECT_GT(vb2.size(), 4U);
    EXPECT_THROW(helics::ValueConverter<std::complex<double>>::interpret(vb1),
                 std::invalid_argument);
    EXPECT_LT(vb1.size(), 12U);
    EXPECT_GT(vb1.size(), 8U);
}

class new_converter_tests_double: public ::testing::TestWithParam<double> {
};

TEST_P(new_converter_tests_double, double_tests)
{
    double v2 = GetParam();
    std::string convString;
    convString.resize(40);
    auto sz = helics::detail::convertToBinary(reinterpret_cast<std::byte*>(convString.data()), v2);
    EXPECT_EQ(helics::detail::detectType(reinterpret_cast<std::byte*>(convString.data())),
              helics::data_type::helics_double);
    EXPECT_EQ(sz, 16);
    double v3;
    helics::detail::convertFromBinary(reinterpret_cast<std::byte*>(convString.data()), v3);
    if (std::isnan(v2)) {
        EXPECT_TRUE(std::isnan(v3));
    } else {
        EXPECT_DOUBLE_EQ(v2, v3);
    }
}

INSTANTIATE_TEST_SUITE_P(double_testing,
                         new_converter_tests_double,
                         ::testing::Values(49.6,
                                           std::numeric_limits<double>::infinity(),
                                           -99.6,
                                           78.895e-45,
                                           -std::numeric_limits<double>::infinity(),
                                           std::numeric_limits<double>::quiet_NaN()));

class new_converter_tests_int: public ::testing::TestWithParam<std::int64_t> {
};

TEST_P(new_converter_tests_int, int_tests)
{
    std::int64_t v2 = GetParam();
    std::string convString;
    convString.resize(40);
    auto sz = helics::detail::convertToBinary(reinterpret_cast<std::byte*>(convString.data()), v2);
    EXPECT_EQ(sz, 16);
    EXPECT_EQ(helics::detail::detectType(reinterpret_cast<std::byte*>(convString.data())),
              helics::data_type::helics_int);
    std::int64_t v3;
    helics::detail::convertFromBinary(reinterpret_cast<std::byte*>(convString.data()), v3);
    EXPECT_EQ(v2, v3);
}

INSTANTIATE_TEST_SUITE_P(int_testing,
                         new_converter_tests_int,
                         ::testing::Values(5735,
                                           std::numeric_limits<std::int64_t>::max(),
                                           -246234,
                                           -std::numeric_limits<std::int64_t>::min(),
                                           0));

class new_converter_tests_complex: public ::testing::TestWithParam<std::complex<double>> {
};

TEST_P(new_converter_tests_complex, complex_tests)
{
    auto v2 = GetParam();
    std::string convString;
    convString.resize(40);
    auto sz = helics::detail::convertToBinary(reinterpret_cast<std::byte*>(convString.data()), v2);
    EXPECT_EQ(sz, 24);

    EXPECT_EQ(helics::detail::detectType(reinterpret_cast<std::byte*>(convString.data())),
              helics::data_type::helics_complex);

    decltype(v2) v3;
    helics::detail::convertFromBinary(reinterpret_cast<std::byte*>(convString.data()), v3);
    if (std::isnan(v2.real())) {
        EXPECT_TRUE(std::isnan(v3.real()));
    } else {
        EXPECT_DOUBLE_EQ(v2.real(), v3.real());
    }

    if (std::isnan(v2.imag())) {
        EXPECT_TRUE(std::isnan(v3.imag()));
    } else {
        EXPECT_DOUBLE_EQ(v2.imag(), v3.imag());
    }
}

INSTANTIATE_TEST_SUITE_P(complex_testing,
                         new_converter_tests_complex,
                         ::testing::Values(compd(1.0, 2.0),
                                           compd(4.5, std::numeric_limits<double>::max()),
                                           compd(-246234, -22354),
                                           compd(-std::numeric_limits<double>::infinity(),
                                                 53.26e-25),
                                           compd(std::numeric_limits<double>::quiet_NaN(),
                                                 std::numeric_limits<double>::quiet_NaN())));

class new_converter_tests_string: public ::testing::TestWithParam<std::string> {
};

TEST_P(new_converter_tests_string, string_tests)
{
    auto v2 = GetParam();
    std::string convString;
    convString.resize(v2.size() + 20);
    auto sz = helics::detail::convertToBinary(reinterpret_cast<std::byte*>(convString.data()), v2);
    EXPECT_EQ(sz, v2.size() + 8);
    EXPECT_EQ(helics::detail::detectType(reinterpret_cast<std::byte*>(convString.data())),
              helics::data_type::helics_string);
    EXPECT_EQ(v2.size(),
              helics::detail::getDataSize(reinterpret_cast<std::byte*>(convString.data())));
    decltype(v2) v3;
    helics::detail::convertFromBinary(reinterpret_cast<std::byte*>(convString.data()), v3);
    EXPECT_EQ(v3, v2);
}

TEST_P(new_converter_tests_string, char_star_tests)
{
    auto v2 = GetParam();
    std::string convString;
    convString.resize(v2.size() + 20);
    auto sz = helics::detail::convertToBinary(reinterpret_cast<std::byte*>(convString.data()), v2);
    EXPECT_EQ(sz, v2.size() + 8);
    auto sz_result = helics::detail::getDataSize(reinterpret_cast<std::byte*>(convString.data()));
    EXPECT_EQ(v2.size(), sz_result);
    decltype(v2) v3;
    v3.resize(sz_result);

    helics::detail::convertFromBinary(reinterpret_cast<std::byte*>(convString.data()), v3.data());
    EXPECT_EQ(v3, v2);
}

INSTANTIATE_TEST_SUITE_P(string_testing,
                         new_converter_tests_string,
                         ::testing::Values("test1",
                                           "this is a longer string so it doesn't fit in SSO",
                                           std::string("\0\0\0\0\0\0\0\0", 8),
                                           std::string(621365, 'a'),
                                           std::string{}));

class new_converter_tests_np: public ::testing::TestWithParam<helics::NamedPoint> {
};

TEST_P(new_converter_tests_np, np_tests)
{
    auto v2 = GetParam();
    std::string convString;
    convString.resize(v2.name.size() + 20);
    auto sz = helics::detail::convertToBinary(reinterpret_cast<std::byte*>(convString.data()), v2);
    EXPECT_EQ(sz, v2.name.size() + 16);
    EXPECT_EQ(helics::detail::detectType(reinterpret_cast<std::byte*>(convString.data())),
              helics::data_type::helics_named_point);
    EXPECT_EQ(v2.name.size(),
              helics::detail::getDataSize(reinterpret_cast<std::byte*>(convString.data())));
    decltype(v2) v3;
    helics::detail::convertFromBinary(reinterpret_cast<const std::byte*>(convString.data()), v3);
    EXPECT_EQ(v3.name, v2.name);

    if (std::isnan(v2.value)) {
        EXPECT_TRUE(std::isnan(v3.value));
    } else {
        EXPECT_EQ(v2.value, v3.value);
    }
}

INSTANTIATE_TEST_SUITE_P(
    np_testing,
    new_converter_tests_np,
    ::testing::Values(helics::NamedPoint{"test", 3.7},
                      helics::NamedPoint{"this is a longer string so it doesn't fit in SSO",
                                         -456.6234},
                      helics::NamedPoint{std::string("\0\0\0\0\0\0\0\0", 8),
                                         std::numeric_limits<double>::quiet_NaN()},
                      helics::NamedPoint{std::string(621365, 'a'), 55.21},
                      helics::NamedPoint{std::string{}, 0.0}));

class new_converter_tests_vector: public ::testing::TestWithParam<std::vector<double>> {
};

TEST_P(new_converter_tests_vector, vector_tests)
{
    auto v2 = GetParam();
    std::string convString;
    convString.resize(v2.size() * 8 + 20);
    auto sz = helics::detail::convertToBinary(reinterpret_cast<std::byte*>(convString.data()), v2);
    EXPECT_EQ(sz, v2.size() * 8 + 8);
    EXPECT_EQ(v2.size(),
              helics::detail::getDataSize(reinterpret_cast<std::byte*>(convString.data())));
    EXPECT_EQ(helics::detail::detectType(reinterpret_cast<std::byte*>(convString.data())),
              helics::data_type::helics_vector);
    decltype(v2) v3;
    helics::detail::convertFromBinary(reinterpret_cast<const std::byte*>(convString.data()), v3);
    if (std::any_of(v2.begin(), v2.end(), [](double val) { return std::isnan(val); })) {
        for (size_t ii = 0; ii < v2.size(); ++ii) {
            if (std::isnan(v2[ii])) {
                EXPECT_TRUE(std::isnan(v3[ii]));
            } else {
                EXPECT_EQ(v2[ii], v3[ii]);
            }
        }
    } else {
        EXPECT_EQ(v3, v2);
    }
}

TEST_P(new_converter_tests_vector, double_star_tests)
{
    auto v2 = GetParam();
    std::string convString;
    convString.resize(v2.size() * 8 + 20);
    auto sz = helics::detail::convertToBinary(reinterpret_cast<std::byte*>(convString.data()), v2);
    EXPECT_EQ(sz, v2.size() * 8 + 8);
    auto sz_result = helics::detail::getDataSize(reinterpret_cast<std::byte*>(convString.data()));
    EXPECT_EQ(v2.size(), sz_result);
    decltype(v2) v3;
    v3.resize(sz_result);

    helics::detail::convertFromBinary(reinterpret_cast<const std::byte*>(convString.data()),
                                      v3.data());
    if (std::any_of(v2.begin(), v2.end(), [](double val) { return std::isnan(val); })) {
        for (size_t ii = 0; ii < v2.size(); ++ii) {
            if (std::isnan(v2[ii])) {
                EXPECT_TRUE(std::isnan(v3[ii]));
            } else {
                EXPECT_EQ(v2[ii], v3[ii]);
            }
        }
    } else {
        EXPECT_EQ(v3, v2);
    }
}

INSTANTIATE_TEST_SUITE_P(
    vector_testing,
    new_converter_tests_vector,
    ::testing::Values(std::vector<double>{3.5, 2.7, -9.3},
                      std::vector<double>{},
                      std::vector<double>(25, 97.7),
                      std::vector<double>(151341.1514),
                      std::vector<double>(0.0),
                      std::vector<double>{0.0, -7.2},
                      std::vector<double>{std::numeric_limits<double>::infinity(),
                                          -std::numeric_limits<double>::infinity(),
                                          std::numeric_limits<double>::signaling_NaN(),
                                          std::numeric_limits<double>::quiet_NaN(),
                                          std::numeric_limits<double>::min(),
                                          std::numeric_limits<double>::max(),
                                          -std::numeric_limits<double>::max()}));

class new_converter_tests_cvector:
    public ::testing::TestWithParam<std::vector<std::complex<double>>> {
};

TEST_P(new_converter_tests_cvector, cvector_tests)
{
    auto v2 = GetParam();
    std::string convString;
    convString.resize(v2.size() * 16 + 20);
    auto sz = helics::detail::convertToBinary(reinterpret_cast<std::byte*>(convString.data()), v2);
    EXPECT_EQ(sz, v2.size() * 16 + 8);
    EXPECT_EQ(v2.size(),
              helics::detail::getDataSize(reinterpret_cast<std::byte*>(convString.data())));
    EXPECT_EQ(helics::detail::detectType(reinterpret_cast<std::byte*>(convString.data())),
              helics::data_type::helics_complex_vector);
    decltype(v2) v3;
    helics::detail::convertFromBinary(reinterpret_cast<const std::byte*>(convString.data()), v3);
    if (std::any_of(v2.begin(), v2.end(), [](auto& val) {
            return std::isnan(val.real()) || std::isnan(val.imag());
        })) {
        for (size_t ii = 0; ii < v2.size(); ++ii) {
            if (std::isnan(v2[ii].real())) {
                EXPECT_TRUE(std::isnan(v3[ii].real()));
            } else {
                EXPECT_DOUBLE_EQ(v2[ii].real(), v3[ii].real());
            }
            if (std::isnan(v2[ii].imag())) {
                EXPECT_TRUE(std::isnan(v3[ii].imag()));
            } else {
                EXPECT_DOUBLE_EQ(v2[ii].imag(), v3[ii].imag());
            }
        }
    } else {
        EXPECT_EQ(v3, v2);
    }
}

using cv = std::vector<std::complex<double>>;
using cplx = std::complex<double>;

INSTANTIATE_TEST_SUITE_P(
    cvector_testing,
    new_converter_tests_cvector,
    ::testing::Values(
        cv{cplx{3.5, 2.7}, cplx{-9.3, 5.6}},
        cv{},
        cv(55, cplx{97.7, 1e-212}),
        cv{cplx{0.0, 0.0}},
        cv{cplx{std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()},
           cplx{std::numeric_limits<double>::signaling_NaN(),
                std::numeric_limits<double>::quiet_NaN()},
           cplx{std::numeric_limits<double>::min(), std::numeric_limits<double>::max()},
           cplx{-std::numeric_limits<double>::max(), std::numeric_limits<double>::quiet_NaN()}}));
