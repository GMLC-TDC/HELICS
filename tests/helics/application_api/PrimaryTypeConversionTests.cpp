/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/common/JsonProcessingFunctions.hpp"

#include <complex>
#include <gtest/gtest.h>
#include <list>
#include <set>
#include <string>
#include <vector>

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
    EXPECT_TRUE(getTypeFromString("char") == DataType::HELICS_CHAR);
    EXPECT_TRUE(getTypeFromString(typeid(int).name()) == DataType::HELICS_INT);
    EXPECT_TRUE(getTypeFromString(typeid(float).name()) == DataType::HELICS_DOUBLE);
    EXPECT_TRUE(getTypeFromString(typeid(std::string).name()) == DataType::HELICS_STRING);
    EXPECT_TRUE(getTypeFromString(typeid(char*).name()) == DataType::HELICS_STRING);
    EXPECT_TRUE(getTypeFromString(typeid(const char*).name()) == DataType::HELICS_STRING);
    EXPECT_TRUE(getTypeFromString(typeid(double).name()) == DataType::HELICS_DOUBLE);
    EXPECT_TRUE(getTypeFromString(typeid(bool).name()) == DataType::HELICS_BOOL);
    EXPECT_TRUE(getTypeFromString(typeid(int64_t).name()) == DataType::HELICS_INT);
    EXPECT_TRUE(getTypeFromString(typeid(char).name()) == DataType::HELICS_CHAR);
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
    nlohmann::json v1;
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

template<class T1>
bool roundTripTest1(const T1& val1,
                    DataType type1,
                    const std::function<bool(const T1& v1, const T1& v2)>& comp)
{
    auto buffer = typeConvert(type1, val1);
    T1 out;
    valueExtract(buffer, type1, out);
    return comp(val1, out);
}

// test to make sure the type conversion round trip works as expected
template<class T1>
bool roundTripTest2(const T1& val1,
                    DataType type1,
                    const std::function<bool(const T1& v1, const T1& v2)>& comp)
{
    auto buffer = typeConvert(type1, val1);
    T1 out;
    defV dv;
    valueExtract(buffer, type1, dv);
    valueExtract(dv, out);
    return comp(val1, out);
}

TEST(roundTripConversions, integer)
{
    std::vector<std::int64_t> vals{10, 256161341561, -3637, 0};
    std::vector<DataType> ctypes{DataType::HELICS_STRING,
                                 DataType::HELICS_DOUBLE,
                                 DataType::HELICS_INT,

                                 DataType::HELICS_COMPLEX,
                                 DataType::HELICS_VECTOR,
                                 DataType::HELICS_COMPLEX_VECTOR,
                                 DataType::HELICS_NAMED_POINT,
                                 DataType::HELICS_TIME,
                                 DataType::HELICS_JSON};

    std::function<bool(const std::int64_t& v1, const std::int64_t& v2)> comp =
        [](const std::int64_t& v1, const std::int64_t& v2) { return (v1 == v2); };
    for (auto& cval : vals) {
        for (auto& ttype : ctypes) {
            bool rt1 = roundTripTest1(cval, ttype, comp);
            EXPECT_TRUE(rt1) << typeNameStringRef(ttype) << ": error1 val " << cval;
            if (!rt1) {
                roundTripTest1(cval, ttype, comp);
            }
            bool rt2 = roundTripTest2(cval, ttype, comp);
            EXPECT_TRUE(rt2) << typeNameStringRef(ttype) << ": error2 val " << cval;
            if (!rt2) {
                roundTripTest2(cval, ttype, comp);
            }
        }
    }
}

TEST(roundTripConversions, BigInteger)
{
    std::vector<std::int64_t> vals{0x7FE4'7FE4'7FE4'7FE4, -265262626261771716};
    std::vector<DataType> ctypes{DataType::HELICS_STRING,
                                 DataType::HELICS_INT,
                                 DataType::HELICS_NAMED_POINT,
                                 DataType::HELICS_TIME,
                                 DataType::HELICS_JSON};

    std::function<bool(const std::int64_t& v1, const std::int64_t& v2)> comp =
        [](const std::int64_t& v1, const std::int64_t& v2) { return (v1 == v2); };
    for (auto& cval : vals) {
        for (auto& ttype : ctypes) {
            bool rt1 = roundTripTest1(cval, ttype, comp);
            EXPECT_TRUE(rt1) << typeNameStringRef(ttype) << ": error1 val " << cval;
            if (!rt1) {
                roundTripTest1(cval, ttype, comp);
            }
            bool rt2 = roundTripTest2(cval, ttype, comp);
            EXPECT_TRUE(rt2) << typeNameStringRef(ttype) << ": error2 val " << cval;
            if (!rt2) {
                roundTripTest2(cval, ttype, comp);
            }
        }
    }
}

TEST(roundTripConversions, double)
{
    std::vector<double> vals{10.0, 2.56161341561, -3637.2365, 0.0, 5.1e7, -4.5e-7};
    std::vector<DataType> ctypes{DataType::HELICS_STRING,
                                 DataType::HELICS_DOUBLE,

                                 DataType::HELICS_COMPLEX,
                                 DataType::HELICS_VECTOR,
                                 DataType::HELICS_COMPLEX_VECTOR,
                                 DataType::HELICS_NAMED_POINT,
                                 DataType::HELICS_JSON};
    std::function<bool(const double& a, const double& b)> comp = [](const double& a,
                                                                    const double& b) {
        return fabs(a - b) <= ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * 0.0000000001);
    };
    for (auto& cval : vals) {
        for (auto& ttype : ctypes) {
            bool rt1 = roundTripTest1(cval, ttype, comp);
            EXPECT_TRUE(rt1) << typeNameStringRef(ttype) << ": error1 val " << cval;
            if (!rt1) {
                roundTripTest1(cval, ttype, comp);
            }
            bool rt2 = roundTripTest2(cval, ttype, comp);
            EXPECT_TRUE(rt2) << typeNameStringRef(ttype) << ": error2 val " << cval;
            if (!rt2) {
                roundTripTest2(cval, ttype, comp);
            }
        }
    }
}

TEST(roundTripConversions, complex)
{
    using cp = std::complex<double>;
    std::vector<cp> vals{cp{10.0, 5.0},
                         cp{2.56161341561, -5.256261541414},
                         cp{-3637.2365, 0.0},
                         cp{0.0, 0.0},
                         cp{0.0, 5.62e34},
                         cp{-4.5e-7, -4.52525e-9}};
    std::vector<DataType> ctypes{DataType::HELICS_STRING,
                                 DataType::HELICS_COMPLEX,
                                 DataType::HELICS_VECTOR,
                                 DataType::HELICS_COMPLEX_VECTOR,
                                 DataType::HELICS_NAMED_POINT,
                                 DataType::HELICS_JSON};
    std::function<bool(const cp& a, const cp& b)> comp = [](const cp& v1, const cp& v2) {
        double a = v1.real();
        double b = v2.real();
        bool rcomp = fabs(a - b) <= ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * 0.0000000001);
        a = v1.imag();
        b = v2.imag();
        bool icomp = fabs(a - b) <= ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * 0.0000000001);
        return rcomp && icomp;
    };

    for (auto& cval : vals) {
        for (auto& ttype : ctypes) {
            bool rt1 = roundTripTest1(cval, ttype, comp);
            EXPECT_TRUE(rt1) << typeNameStringRef(ttype) << ": error val " << cval;
            if (!rt1) {
                roundTripTest1(cval, ttype, comp);
            }
            bool rt2 = roundTripTest2(cval, ttype, comp);
            EXPECT_TRUE(rt2) << typeNameStringRef(ttype) << ": error val " << cval;
            if (!rt2) {
                roundTripTest2(cval, ttype, comp);
            }
        }
    }
}

TEST(roundTripConversions, vector)
{
    using vd = std::vector<double>;
    std::vector<vd> vals{vd{-10.0},
                         vd{10.0},
                         vd{10.0, 5.0},
                         vd{2.56161341561, -5.256261541414, 5.62248785566332},
                         vd{-3637.2365, 0.0, 6.7},
                         vd{-3637.2365, 0.0, 6.7, 9.7},
                         std::vector<double>(151, 4.5),
                         std::vector<double>(35, 27.35)};
    std::vector<DataType> ctypes{DataType::HELICS_STRING,
                                 DataType::HELICS_VECTOR,
                                 DataType::HELICS_COMPLEX_VECTOR,
                                 DataType::HELICS_NAMED_POINT,
                                 DataType::HELICS_JSON};
    std::function<bool(const vd& a, const vd& b)> comp = [](const vd& v1, const vd& v2) {
        if (v1.size() != v2.size()) {
            return false;
        }
        for (size_t ii = 0; ii < v1.size(); ++ii) {
            auto a = v1[ii];
            auto b = v2[ii];
            bool rcomp = fabs(a - b) <= ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * 0.0000000001);
            if (!rcomp) {
                return false;
            }
        }
        return true;
    };

    for (auto& cval : vals) {
        for (auto& ttype : ctypes) {
            bool rt1 = roundTripTest1(cval, ttype, comp);
            EXPECT_TRUE(rt1) << typeNameStringRef(ttype) << ": error val "
                             << helicsVectorString(cval);
            if (!rt1) {
                roundTripTest1(cval, ttype, comp);
            }
            bool rt2 = roundTripTest2(cval, ttype, comp);
            EXPECT_TRUE(rt2) << typeNameStringRef(ttype) << ": error val "
                             << helicsVectorString(cval);
            if (!rt2) {
                roundTripTest2(cval, ttype, comp);
            }
        }
    }
}

TEST(roundTripConversions, complex_vector)
{
    using cd = std::complex<double>;
    using cvd = std::vector<cd>;
    std::vector<cvd> vals{cvd{cd{-10.0, 0.0}},
                          cvd{cd{10.0, 10.0}},
                          cvd{cd{10.0, 0.0}, cd{5.0, -1.3}},
                          cvd{cd{2.56161341561, -235235.2525151},
                              cd{-5.256261541414, 4e-7},
                              cd{5.62248785566332, -252.23523523}},
                          cvd{cd{-3637.2365, 5.6}, cd{0.0, 0.0}, cd{6.7, 4.6}},
                          cvd{cd{-3637.2365, 23.665665363636},
                              cd{0.0, 0.0},
                              cd{6.7, 9.7},
                              cd{-3e-45, 3.455213414e16}},
                          cvd(151, cd{4.5, 18.7}),
                          cvd(35, cd{27.35, -2.3})};
    std::vector<DataType> ctypes{DataType::HELICS_STRING,
                                 DataType::HELICS_VECTOR,
                                 DataType::HELICS_COMPLEX_VECTOR,
                                 DataType::HELICS_NAMED_POINT,
                                 DataType::HELICS_JSON};
    std::function<bool(const cvd& a, const cvd& b)> comp = [](const cvd& v1, const cvd& v2) {
        if (v1.size() != v2.size()) {
            return false;
        }
        for (size_t ii = 0; ii < v1.size(); ++ii) {
            double a = v1[ii].real();
            double b = v2[ii].real();
            bool rcomp = fabs(a - b) <= ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * 0.0000000001);
            a = v1[ii].imag();
            b = v2[ii].imag();
            bool icomp = fabs(a - b) <= ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * 0.0000000001);
            if (!(rcomp && icomp)) {
                return false;
            }
        }
        return true;
    };

    for (auto& cval : vals) {
        for (auto& ttype : ctypes) {
            bool rt1 = roundTripTest1(cval, ttype, comp);
            EXPECT_TRUE(rt1) << typeNameStringRef(ttype) << ": error val "
                             << helicsComplexVectorString(cval);
            if (!rt1) {
                roundTripTest1(cval, ttype, comp);
            }
            bool rt2 = roundTripTest2(cval, ttype, comp);
            EXPECT_TRUE(rt2) << typeNameStringRef(ttype) << ": error val "
                             << helicsComplexVectorString(cval);
            if (!rt2) {
                roundTripTest2(cval, ttype, comp);
            }
        }
    }
}

TEST(roundTripConversions, boolean)
{
    std::vector<int> vals{1, 0};
    std::vector<DataType> ctypes{DataType::HELICS_STRING,
                                 DataType::HELICS_DOUBLE,
                                 DataType::HELICS_INT,
                                 DataType::HELICS_BOOL,
                                 DataType::HELICS_COMPLEX,
                                 DataType::HELICS_VECTOR,
                                 DataType::HELICS_COMPLEX_VECTOR,
                                 DataType::HELICS_NAMED_POINT,
                                 DataType::HELICS_TIME,
                                 DataType::HELICS_JSON};

    std::function<bool(const bool& v1, const bool& v2)> comp = [](const bool& v1, const bool& v2) {
        return (v1 == v2);
    };
    for (auto cval : vals) {
        for (auto& ttype : ctypes) {
            EXPECT_TRUE(roundTripTest1(static_cast<bool>(cval), ttype, comp))
                << typeNameStringRef(ttype) << ": error val " << cval;
            EXPECT_TRUE(roundTripTest2(static_cast<bool>(cval), ttype, comp))
                << typeNameStringRef(ttype) << ": error val " << cval;
        }
    }
}

TEST(roundTripConversions, char)
{
    std::vector<char> vals{0, 1, 5, 7, static_cast<char>(-9), 14, 'a', 'X', 127};
    std::vector<DataType> ctypes{DataType::HELICS_STRING,
                                 DataType::HELICS_DOUBLE,
                                 DataType::HELICS_INT,
                                 DataType::HELICS_COMPLEX,
                                 DataType::HELICS_VECTOR,
                                 DataType::HELICS_COMPLEX_VECTOR,
                                 DataType::HELICS_NAMED_POINT,
                                 DataType::HELICS_TIME,
                                 DataType::HELICS_JSON};

    std::function<bool(const char& v1, const char& v2)> comp = [](const char& v1, const char& v2) {
        return (v1 == v2);
    };
    for (auto cval : vals) {
        for (auto& ttype : ctypes) {
            bool rt1 = roundTripTest1(cval, ttype, comp);
            EXPECT_TRUE(rt1) << typeNameStringRef(ttype) << ": error1 val " << cval;
            if (!rt1) {
                roundTripTest1(cval, ttype, comp);
            }
            bool rt2 = roundTripTest2(cval, ttype, comp);
            EXPECT_TRUE(rt2) << typeNameStringRef(ttype) << ": error2 val " << cval;
            if (!rt2) {
                roundTripTest2(cval, ttype, comp);
            }
        }
    }
}

TEST(roundTripConversions, string)
{
    std::vector<std::string> vals{"string1", " this is a test string", std::string(200, 'a')};
    std::vector<DataType> ctypes{DataType::HELICS_STRING,
                                 DataType::HELICS_NAMED_POINT,
                                 DataType::HELICS_JSON};

    std::function<bool(const std::string& v1, const std::string& v2)> comp =
        [](const std::string& v1, const std::string& v2) { return (v1 == v2); };
    for (const auto& cval : vals) {
        for (auto& ttype : ctypes) {
            EXPECT_TRUE(roundTripTest1(cval, ttype, comp))
                << typeNameStringRef(ttype) << ": error val " << cval;
            EXPECT_TRUE(roundTripTest2(cval, ttype, comp))
                << typeNameStringRef(ttype) << ": error val " << cval;
        }
    }
}

TEST(roundTripConversions, named_point)
{
    std::vector<NamedPoint> vals{NamedPoint{"test1", 45.662},
                                 NamedPoint{"", -2352.235265622},
                                 NamedPoint{"this is a longer tag", 45.662},
                                 NamedPoint{std::string(564, 'b'), 5e-7},
                                 NamedPoint{"test b", -99.99e99}};
    std::vector<DataType> ctypes{DataType::HELICS_STRING,
                                 DataType::HELICS_NAMED_POINT,
                                 DataType::HELICS_JSON};
    std::function<bool(const NamedPoint& a, const NamedPoint& b)> comp = [](const NamedPoint& v1,
                                                                            const NamedPoint& v2) {
        double a = v1.value;
        double b = v2.value;
        bool rcomp = fabs(a - b) <= ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * 0.0000000001);

        bool icomp = v1.name == v2.name;
        return rcomp && icomp;
    };

    for (auto& cval : vals) {
        for (auto& ttype : ctypes) {
            bool rt1 = roundTripTest1(cval, ttype, comp);
            EXPECT_TRUE(rt1) << typeNameStringRef(ttype) << ": error val "
                             << helicsNamedPointString(cval);
            if (!rt1) {
                roundTripTest1(cval, ttype, comp);
            }
            bool rt2 = roundTripTest2(cval, ttype, comp);
            EXPECT_TRUE(rt2) << typeNameStringRef(ttype) << ": error val "
                             << helicsNamedPointString(cval);
            if (!rt2) {
                roundTripTest2(cval, ttype, comp);
            }
        }
    }
}

TEST(roundTripConversions, time)
{
    std::vector<Time> vals{45.7, timeZero, Time::maxVal(), Time::epsilon(), -10.5, 23526262};
    std::vector<DataType> ctypes{DataType::HELICS_STRING,
                                 DataType::HELICS_DOUBLE,
                                 DataType::HELICS_INT,

                                 DataType::HELICS_COMPLEX,
                                 DataType::HELICS_VECTOR,
                                 DataType::HELICS_COMPLEX_VECTOR,
                                 DataType::HELICS_NAMED_POINT,
                                 DataType::HELICS_TIME,
                                 DataType::HELICS_JSON};

    std::function<bool(const Time& v1, const Time& v2)> comp = [](const Time& v1, const Time& v2) {
        return (v1 == v2);
    };
    for (auto& cval : vals) {
        for (auto& ttype : ctypes) {
            bool rt1 = roundTripTest1(cval, ttype, comp);
            EXPECT_TRUE(rt1) << typeNameStringRef(ttype) << ": error val "
                             << static_cast<double>(cval);
            if (!rt1) {
                roundTripTest1(cval, ttype, comp);
            }
            bool rt2 = roundTripTest2(cval, ttype, comp);
            EXPECT_TRUE(rt2) << typeNameStringRef(ttype) << ": error val "
                             << static_cast<double>(cval);
            if (!rt2) {
                roundTripTest2(cval, ttype, comp);
            }
        }
    }
}
