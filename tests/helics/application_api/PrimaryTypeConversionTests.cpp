/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/common/JsonProcessingFunctions.hpp"

#include <complex>
#include <gtest/gtest.h>
#include <list>
#include <set>

/** these test cases test out the value converters
 */
#include "helics/application_api/HelicsPrimaryTypes.hpp"

using namespace std::string_literals;
using namespace helics;

template<class T1, class T2>
bool checkTypeConversion1(const T1& val1, const T2& exp)
{
    defV val = val1;
    T2 v2{};
    valueExtract(val, v2);
    return (v2 == exp);
}

TEST(type_conversion, vectorNorm)
{
    using c = std::complex<double>;
    using cv = std::vector<c>;
    EXPECT_EQ(vectorNorm(std::vector<double>()), 0.0);
    EXPECT_EQ(vectorNorm(std::vector<double>{4.0}), 4.0);
    EXPECT_EQ(vectorNorm(std::vector<double>{3.0, 4.0}), 5.0);
    EXPECT_EQ(vectorNorm(std::vector<double>{-3.0, -4.0}), 5.0);
    EXPECT_EQ(vectorNorm(std::vector<double>{-3.0, -3.0, -3.0, -3.0, -3.0}), std::sqrt(9.0 * 5.0));

    EXPECT_EQ(vectorNorm(cv()), 0.0);
    EXPECT_EQ(vectorNorm(cv{c(4.0, 0)}), 4.0);
    EXPECT_EQ(vectorNorm(cv{c(3.0, 4.0)}), 5.0);
    EXPECT_EQ(vectorNorm(cv{c(-3.0, -4.0)}), 5.0);
    EXPECT_EQ(vectorNorm(
                  cv{c(-3.0, -4.0), c(-3.0, -4.0), c(-3.0, -4.0), c(-3.0, -4.0), c(-3.0, -4.0)}),
              std::sqrt(25.0 * 5.0));
}

TEST(type_conversion, string_type_tests)
{
    EXPECT_TRUE(helicsType<std::string>() == DataType::HELICS_STRING);
    // EXPECT_TRUE(helicsType<char *>() == DataType::HELICS_STRING);
}

TEST(type_conversion, string_converstion)
{
    std::string vstr("45.786");
    double val = 45.786;
    EXPECT_TRUE(checkTypeConversion1(vstr, vstr));
    EXPECT_TRUE(checkTypeConversion1(vstr, val));
    EXPECT_TRUE(checkTypeConversion1(vstr, static_cast<int64_t>(val)));
    EXPECT_TRUE(checkTypeConversion1(vstr, static_cast<float>(val)));
    EXPECT_TRUE(checkTypeConversion1(vstr, std::complex<double>(val, 0)));
    EXPECT_TRUE(checkTypeConversion1(vstr, std::vector<double>{val}));
    EXPECT_TRUE(
        checkTypeConversion1(vstr,
                             std::vector<std::complex<double>>{std::complex<double>(val, 0.0)}));
    EXPECT_TRUE(checkTypeConversion1(vstr, true));
    EXPECT_TRUE(checkTypeConversion1(vstr, NamedPoint{"value", val}));
    std::string test1("test1");
    EXPECT_TRUE(checkTypeConversion1(test1, NamedPoint{test1, std::nan("0")}));
}

TEST(type_conversion, string_converstion_negative)
{
    std::string vstr("-15.212");
    double val = -15.212;
    EXPECT_TRUE(checkTypeConversion1(vstr, vstr));
    EXPECT_TRUE(checkTypeConversion1(vstr, val));
    EXPECT_TRUE(checkTypeConversion1(vstr, static_cast<int64_t>(val)));
    EXPECT_TRUE(checkTypeConversion1(vstr, static_cast<float>(val)));
    EXPECT_TRUE(checkTypeConversion1(vstr, std::complex<double>(val, 0)));
    EXPECT_TRUE(checkTypeConversion1(vstr, std::vector<double>{val}));
    EXPECT_TRUE(
        checkTypeConversion1(vstr,
                             std::vector<std::complex<double>>{std::complex<double>(val, 0.0)}));
    EXPECT_TRUE(checkTypeConversion1(vstr, true));
    EXPECT_TRUE(checkTypeConversion1(vstr, NamedPoint{"value", val}));
    std::string test1("test1");
    EXPECT_TRUE(checkTypeConversion1(test1, NamedPoint{test1, std::nan("0")}));
}

TEST(type_conversion, double_type)
{
    EXPECT_TRUE(helicsType<double>() == DataType::HELICS_DOUBLE);
    EXPECT_TRUE(helicsType<float>() == DataType::HELICS_CUSTOM);
    EXPECT_TRUE(isConvertableType<float>() == true);
    EXPECT_TRUE(isConvertableType<double>() == false);
}

TEST(type_conversion, double_conversion)
{
    double val = 45.786;
    EXPECT_TRUE(checkTypeConversion1(val, val));
    EXPECT_TRUE(checkTypeConversion1(val, std::to_string(val)));
    EXPECT_TRUE(checkTypeConversion1(val, static_cast<int64_t>(val)));
    EXPECT_TRUE(checkTypeConversion1(val, static_cast<float>(val)));
    EXPECT_TRUE(checkTypeConversion1(val, std::complex<double>(val, 0)));
    EXPECT_TRUE(checkTypeConversion1(val, std::vector<double>{val}));
    EXPECT_TRUE(
        checkTypeConversion1(val,
                             std::vector<std::complex<double>>{std::complex<double>(val, 0.0)}));
    EXPECT_TRUE(checkTypeConversion1(val, true));
    EXPECT_TRUE(checkTypeConversion1(val, NamedPoint{"value", val}));
}

TEST(type_conversion, integer_type)
{
    EXPECT_TRUE(helicsType<int64_t>() == DataType::HELICS_INT);
    EXPECT_TRUE(helicsType<int>() == DataType::HELICS_CUSTOM);
    EXPECT_TRUE(isConvertableType<int>() == true);
    EXPECT_TRUE(isConvertableType<int64_t>() == false);

    EXPECT_TRUE(isConvertableType<short>() == true);
    EXPECT_TRUE(isConvertableType<uint64_t>() == true);

    EXPECT_TRUE(isConvertableType<char>() == true);
    EXPECT_TRUE(isConvertableType<unsigned char>() == true);

    EXPECT_TRUE(isConvertableType<unsigned char>() == true);
}

TEST(type_conversion, namedType)
{
    EXPECT_TRUE(getTypeFromString("int") == DataType::HELICS_INT);
    EXPECT_TRUE(getTypeFromString("INT") == DataType::HELICS_INT);
    EXPECT_TRUE(getTypeFromString("char") == DataType::HELICS_STRING);
    EXPECT_TRUE(getTypeFromString(typeid(int).name()) == DataType::HELICS_INT);
    EXPECT_TRUE(getTypeFromString(typeid(float).name()) == DataType::HELICS_DOUBLE);
    EXPECT_TRUE(getTypeFromString(typeid(std::string).name()) == DataType::HELICS_STRING);
    EXPECT_TRUE(getTypeFromString(typeid(char*).name()) == DataType::HELICS_STRING);
    EXPECT_TRUE(getTypeFromString(typeid(const char*).name()) == DataType::HELICS_STRING);
    EXPECT_TRUE(getTypeFromString(typeid(double).name()) == DataType::HELICS_DOUBLE);
    EXPECT_TRUE(getTypeFromString(typeid(bool).name()) == DataType::HELICS_BOOL);
    EXPECT_TRUE(getTypeFromString(typeid(int64_t).name()) == DataType::HELICS_INT);
    EXPECT_TRUE(getTypeFromString(typeid(char).name()) == DataType::HELICS_STRING);
    EXPECT_TRUE(getTypeFromString(typeid(std::complex<double>).name()) == DataType::HELICS_COMPLEX);
    EXPECT_TRUE(getTypeFromString("COMPLEX") == DataType::HELICS_COMPLEX);
    EXPECT_TRUE(getTypeFromString("map") == DataType::HELICS_CUSTOM);
    EXPECT_TRUE(getTypeFromString("any") == DataType::HELICS_ANY);
    EXPECT_TRUE(getTypeFromString("json") == DataType::HELICS_JSON);
    EXPECT_TRUE(getTypeFromString("JSON") == DataType::HELICS_JSON);
    EXPECT_TRUE(getTypeFromString("") == DataType::HELICS_ANY);
    EXPECT_TRUE(getTypeFromString(typeid(std::vector<std::complex<double>>).name()) ==
                DataType::HELICS_COMPLEX_VECTOR);
    EXPECT_TRUE(getTypeFromString(typeid(Time).name()) == DataType::HELICS_TIME);
}

TEST(type_conversion, integer_conversion)
{
    int64_t val = -10;
    EXPECT_TRUE(checkTypeConversion1(val, val));
    EXPECT_TRUE(checkTypeConversion1(val, std::to_string(val)));
    EXPECT_TRUE(checkTypeConversion1(val, static_cast<double>(val)));
    EXPECT_TRUE(checkTypeConversion1(val, static_cast<float>(val)));
    EXPECT_TRUE(checkTypeConversion1(val, static_cast<short>(val)));
    EXPECT_TRUE(checkTypeConversion1(val, static_cast<int>(val)));
    EXPECT_TRUE(checkTypeConversion1(val, std::complex<double>(static_cast<double>(val), 0)));
    EXPECT_TRUE(checkTypeConversion1(val, std::vector<double>{static_cast<double>(val)}));
    EXPECT_TRUE(checkTypeConversion1(val,
                                     std::vector<std::complex<double>>{
                                         std::complex<double>(static_cast<double>(val), 0.0)}));
    EXPECT_TRUE(checkTypeConversion1(val, true));
    EXPECT_TRUE(checkTypeConversion1(static_cast<int64_t>(0), false));
    EXPECT_TRUE(checkTypeConversion1(val, NamedPoint{"value", static_cast<double>(val)}));
}

TEST(type_conversion, namedpoint_type)
{
    EXPECT_TRUE(helicsType<NamedPoint>() == DataType::HELICS_NAMED_POINT);
    // EXPECT_TRUE(helicsType<char *>() == DataType::HELICS_STRING);
}

TEST(type_conversion, namedpoint_conversion)
{
    double val = 45.786;
    NamedPoint vp{"point", val};
    EXPECT_TRUE(checkTypeConversion1(vp, vp));
    EXPECT_TRUE(checkTypeConversion1(vp, val));
    EXPECT_TRUE(checkTypeConversion1(vp, static_cast<int64_t>(val)));
    EXPECT_TRUE(checkTypeConversion1(vp, static_cast<float>(val)));
    EXPECT_TRUE(checkTypeConversion1(vp, std::complex<double>(val, 0)));
    EXPECT_TRUE(checkTypeConversion1(vp, std::vector<double>{val}));
    EXPECT_TRUE(
        checkTypeConversion1(vp,
                             std::vector<std::complex<double>>{std::complex<double>(val, 0.0)}));
    EXPECT_TRUE(checkTypeConversion1(vp, true));
    Json::Value v1;
    v1["name"] = "point";
    v1["value"] = val;
    EXPECT_TRUE(checkTypeConversion1(vp, helics::fileops::generateJsonString(v1)));

    NamedPoint vp2{"v2[3.0,-4.0]", std::nan("0")};
    double v2 = 5.0;
    EXPECT_TRUE(checkTypeConversion1(vp2, vp2));
    EXPECT_TRUE(checkTypeConversion1(vp2, v2));
    EXPECT_TRUE(checkTypeConversion1(vp2, static_cast<int64_t>(v2)));
    EXPECT_TRUE(checkTypeConversion1(vp2, static_cast<float>(v2)));
    EXPECT_TRUE(checkTypeConversion1(vp2, std::complex<double>(3.0, -4.0)));
    EXPECT_TRUE(checkTypeConversion1(vp2, std::vector<double>{3.0, -4.0}));
    EXPECT_TRUE(
        checkTypeConversion1(vp2,
                             std::vector<std::complex<double>>{std::complex<double>(3.0, -4.0)}));
    EXPECT_TRUE(checkTypeConversion1(vp2, true));
    EXPECT_TRUE(checkTypeConversion1(vp2, vp2.name));

    NamedPoint t1("this is a longer string for testing purposes", 234.252622334);
    auto s = helicsNamedPointString(t1);
    auto t2 = helicsGetNamedPoint(s);
    EXPECT_EQ(t1.name, t2.name);
    EXPECT_NEAR(t1.value, t2.value, 0.00001);
}

TEST(type_conversion, bool_conversion)
{
    bool val{false};
    EXPECT_TRUE(checkTypeConversion1(std::string{"false"}, val));
    EXPECT_TRUE(checkTypeConversion1(std::string{"0"}, val));
    EXPECT_TRUE(checkTypeConversion1(std::string{"F"}, val));
    EXPECT_TRUE(checkTypeConversion1(0.0F, val));
    EXPECT_TRUE(checkTypeConversion1(0.0, val));
    EXPECT_TRUE(checkTypeConversion1(NamedPoint{"value", 0.0}, val));
    EXPECT_TRUE(checkTypeConversion1(std::vector<double>{0.0}, val));
    EXPECT_TRUE(checkTypeConversion1(std::vector<double>{}, val));
    EXPECT_TRUE(checkTypeConversion1(std::complex<double>{0.0, 0.0}, val));
    EXPECT_TRUE(checkTypeConversion1(std::vector<std::complex<double>>{{0.0, 0.0}}, val));
    EXPECT_TRUE(checkTypeConversion1(std::vector<std::complex<double>>{}, val));
    val = true;
    EXPECT_TRUE(checkTypeConversion1(std::string{"on"}, val));
    EXPECT_TRUE(checkTypeConversion1(std::string{"1"}, val));
    EXPECT_TRUE(checkTypeConversion1(std::string{"Y"}, val));

    EXPECT_TRUE(checkTypeConversion1(1.0F, val));
    EXPECT_TRUE(checkTypeConversion1(1.0, val));
    EXPECT_TRUE(checkTypeConversion1(NamedPoint{"value", 1.0}, val));
    EXPECT_TRUE(checkTypeConversion1(std::vector<double>{1.0}, val));
    EXPECT_TRUE(checkTypeConversion1(std::vector<double>{0.0, 1.0}, val));
    EXPECT_TRUE(checkTypeConversion1(std::complex<double>{0.0, 1.0}, val));
    EXPECT_TRUE(checkTypeConversion1(std::complex<double>{0.0, -0.5}, val));
}
