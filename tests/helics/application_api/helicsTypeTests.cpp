/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/common/JsonProcessingFunctions.hpp"

#include <complex>
#include <gtest/gtest.h>
#include <string>
#include <vector>

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

TEST(helics_types, vector_to_string)
{
    std::vector<double> V1{10.0, 3.5, 5.6};
    auto v = helicsVectorString(V1);
    EXPECT_FALSE(v.empty());
    EXPECT_EQ(v, "[10,3.5,5.6]");
}

TEST(helics_types, cvector_string)
{
    auto v = helicsGetComplexVector(std::string{});
    EXPECT_TRUE(v.empty());
    v = helicsGetComplexVector("c[1+4j,2-3j,invalid]");
    ASSERT_EQ(v.size(), 3U);
    EXPECT_EQ(v[2], invalidValue<std::complex<double>>());
}

TEST(helics_types, cvector_to_string)
{
    std::vector<std::complex<double>> V1{{1, 4}, {2, -3}};
    auto v = helicsComplexVectorString(V1);
    EXPECT_FALSE(v.empty());
    EXPECT_EQ(v, "[[1,4],[2,-3]]");

    auto V2 = helicsGetComplexVector(v);
    EXPECT_EQ(V1, V2);
}

TEST(json_type_conversion, to_json)
{
    auto res = typeConvert(DataType::HELICS_JSON, 49.7);
    nlohmann::json jv;
    defV result;
    EXPECT_NO_THROW(jv = fileops::loadJsonStr(res.to_string()));
    EXPECT_TRUE(jv.contains("value"));
    EXPECT_TRUE(jv.contains("type"));
    result = readJsonValue(res);
    EXPECT_EQ(result.index(), double_loc);
    EXPECT_DOUBLE_EQ(std::get<double>(result), 49.7);

    res = typeConvert(DataType::HELICS_JSON, std::int64_t(1956258));
    EXPECT_NO_THROW(jv = fileops::loadJsonStr(res.to_string()));
    EXPECT_TRUE(jv.contains("value"));
    EXPECT_TRUE(jv.contains("type"));
    result = readJsonValue(res);
    EXPECT_EQ(result.index(), int_loc);
    EXPECT_EQ(std::get<std::int64_t>(result), 1956258LL);

    std::string_view testString("this is a test");
    res = typeConvert(DataType::HELICS_JSON, testString);
    EXPECT_NO_THROW(jv = fileops::loadJsonStr(res.to_string()));
    EXPECT_TRUE(jv.contains("value"));
    EXPECT_TRUE(jv.contains("type"));

    result = readJsonValue(res);
    EXPECT_EQ(result.index(), string_loc);
    EXPECT_EQ(std::get<std::string>(result), testString);

    std::vector<double> testV{456.6, 19.5};
    res = typeConvert(DataType::HELICS_JSON, testV);
    EXPECT_NO_THROW(jv = fileops::loadJsonStr(res.to_string()));
    EXPECT_TRUE(jv.contains("value"));
    EXPECT_TRUE(jv.contains("type"));
    result = readJsonValue(res);
    EXPECT_EQ(result.index(), vector_loc);
    EXPECT_EQ(std::get<std::vector<double>>(result), testV);
    res = typeConvert(DataType::HELICS_JSON, testV.data(), testV.size());
    EXPECT_NO_THROW(jv = fileops::loadJsonStr(res.to_string()));
    EXPECT_TRUE(jv.contains("value"));
    EXPECT_TRUE(jv.contains("type"));
    result = readJsonValue(res);
    EXPECT_EQ(result.index(), vector_loc);
    EXPECT_EQ(std::get<std::vector<double>>(result), testV);

    std::vector<std::complex<double>> testcv;
    testcv.emplace_back(15.7, -5363.55);
    testcv.emplace_back(-543623.44, 151.133);
    res = typeConvert(DataType::HELICS_JSON, testcv);
    EXPECT_NO_THROW(jv = fileops::loadJsonStr(res.to_string()));
    EXPECT_TRUE(jv.contains("value"));
    EXPECT_TRUE(jv.contains("type"));
    result = readJsonValue(res);
    EXPECT_EQ(result.index(), complex_vector_loc);
    EXPECT_EQ(std::get<std::vector<std::complex<double>>>(result), testcv);

    res = typeConvert(DataType::HELICS_JSON, testcv.front());
    EXPECT_NO_THROW(jv = fileops::loadJsonStr(res.to_string()));
    EXPECT_TRUE(jv.contains("value"));
    EXPECT_TRUE(jv.contains("type"));
    result = readJsonValue(res);
    EXPECT_EQ(result.index(), complex_loc);
    EXPECT_EQ(std::get<std::complex<double>>(result), testcv.front());

    NamedPoint t1("vvvv", 1851.44);
    res = typeConvert(DataType::HELICS_JSON, t1);
    EXPECT_NO_THROW(jv = fileops::loadJsonStr(res.to_string()));
    EXPECT_TRUE(jv.contains("value"));
    EXPECT_TRUE(jv.contains("type"));
    result = readJsonValue(res);
    EXPECT_EQ(result.index(), named_point_loc);
    EXPECT_EQ(std::get<NamedPoint>(result), t1);

    res = typeConvert(DataType::HELICS_JSON, t1.name, t1.value);
    EXPECT_NO_THROW(jv = fileops::loadJsonStr(res.to_string()));
    EXPECT_TRUE(jv.contains("value"));
    EXPECT_TRUE(jv.contains("type"));
    result = readJsonValue(res);
    EXPECT_EQ(result.index(), named_point_loc);
    EXPECT_EQ(std::get<NamedPoint>(result), t1);

    res = typeConvert(DataType::HELICS_JSON, true);
    EXPECT_NO_THROW(jv = fileops::loadJsonStr(res.to_string()));
    EXPECT_TRUE(jv.contains("value"));
    EXPECT_TRUE(jv.contains("type"));
    result = readJsonValue(res);
    EXPECT_EQ(result.index(), int_loc);
    EXPECT_EQ(std::get<std::int64_t>(result), 1);
}
