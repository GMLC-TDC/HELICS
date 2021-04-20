/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helicsTypes.hpp"

#include "ValueConverter.hpp"
#include "gmlc/utilities/demangle.hpp"
#include "gmlc/utilities/stringConversion.h"

#include <algorithm>
#include <functional>
#include <numeric>
#include <regex>
#include <sstream>
#include <unordered_map>

using namespace gmlc::utilities;  // NOLINT

namespace helics {

const std::string& typeNameStringRef(data_type type)
{
    static const std::string doubleString("double");
    static const std::string intString("int64");
    static const std::string stringString("string");
    static const std::string complexString("complex");
    static const std::string boolString("bool");
    static const std::string doubleVecString("double_vector");
    static const std::string complexVecString("complex_vector");
    static const std::string namedPointString("named_point");
    static const std::string timeString("time");
    static const std::string nullString;
    switch (type) {
        case data_type::helics_double:
            return doubleString;
        case data_type::helics_int:
            return intString;
        case data_type::helics_bool:
            return boolString;
        case data_type::helics_string:
            return stringString;
        case data_type::helics_complex:
            return complexString;
        case data_type::helics_time:
            return timeString;
        case data_type::helics_vector:
            return doubleVecString;
        case data_type::helics_complex_vector:
            return complexVecString;
        case data_type::helics_named_point:
            return namedPointString;
        default:
            return nullString;
    }
}

double vectorNorm(const std::vector<double>& vec)
{
    return std::sqrt(std::inner_product(vec.begin(), vec.end(), vec.begin(), 0.0));
}

double vectorNorm(const std::vector<std::complex<double>>& vec)
{
    return std::sqrt(std::inner_product(
        vec.begin(), vec.end(), vec.begin(), 0.0, std::plus<>(), [](const auto& a, const auto& b) {
            return (a * std::conj(b)).real();
        }));
}

std::string helicsComplexString(double real, double imag)
{
    std::stringstream ss;
    ss << real;
    if (imag != 0.0) {
        ss << ((imag >= 0.0) ? '+' : ' ') << imag << 'j';
    }
    return ss.str();
}

std::string helicsComplexString(std::complex<double> val)
{
    return helicsComplexString(val.real(), val.imag());
}
/** map of an assortment of type string that can be converted to a known type*/
static const std::unordered_map<std::string, data_type> typeMap{
    {"double", data_type::helics_double},
    {"string", data_type::helics_string},
    {"binary", data_type::helics_bool},
    {"bool", data_type::helics_bool},
    {"boolean", data_type::helics_bool},
    {"flag", data_type::helics_bool},
    {"float", data_type::helics_double},
    {"vector", data_type::helics_vector},
    {"double_vector", data_type::helics_vector},
    {"double vector", data_type::helics_vector},
    {typeid(std::vector<double>).name(), data_type::helics_vector},
    {gmlc::demangle(typeid(std::vector<double>).name()), data_type::helics_vector},
    {typeid(double*).name(), data_type::helics_vector},
    {"complex", data_type::helics_complex},
    {"pair", data_type::helics_complex},
    {"int", data_type::helics_int},
    {"int64", data_type::helics_int},
    {typeid(double).name(), data_type::helics_double},
    {typeid(float).name(), data_type::helics_double},
    {typeid(char).name(), data_type::helics_string},
    {typeid(unsigned char).name(), data_type::helics_int},
    {typeid(short).name(), data_type::helics_int},  // NOLINT
    {typeid(unsigned short).name(), data_type::helics_int},  // NOLINT
    {typeid(int).name(), data_type::helics_int},
    {typeid(unsigned int).name(), data_type::helics_int},
    {typeid(long).name(), data_type::helics_int},  // NOLINT
    {typeid(unsigned long).name(), data_type::helics_int},  // NOLINT
    {typeid(long long).name(), data_type::helics_int},  // NOLINT
    {typeid(unsigned long long).name(), data_type::helics_int},  // NOLINT
    {typeid(int64_t).name(), data_type::helics_int},
    {typeid(uint64_t).name(), data_type::helics_int},
    {typeid(int32_t).name(), data_type::helics_int},
    {typeid(uint32_t).name(), data_type::helics_int},
    {typeid(int16_t).name(), data_type::helics_int},
    {typeid(uint16_t).name(), data_type::helics_int},
    {typeid(int8_t).name(), data_type::helics_int},
    {typeid(uint8_t).name(), data_type::helics_int},
    {typeid(bool).name(), data_type::helics_bool},
    {"long long", data_type::helics_int},
    {"integer", data_type::helics_int},
    {"int32", data_type::helics_int},
    {"uint32", data_type::helics_int},
    {"uint64", data_type::helics_int},
    {"int16", data_type::helics_int},
    {"uint16", data_type::helics_int},
    {"short", data_type::helics_int},
    {"unsigned short", data_type::helics_int},
    {"long", data_type::helics_int},
    {"unsigned long", data_type::helics_int},
    {"char", data_type::helics_string},
    {"uchar", data_type::helics_int},
    {"unsigned char", data_type::helics_int},
    {"byte", data_type::helics_int},
    {"int8", data_type::helics_int},
    {"uint8", data_type::helics_int},
    {"complex_vector", data_type::helics_complex_vector},
    {"complex vector", data_type::helics_complex_vector},
    {typeid(std::vector<std::complex<double>>).name(), data_type::helics_complex_vector},
    {gmlc::demangle(typeid(std::vector<std::complex<double>>).name()),
     data_type::helics_complex_vector},
    {"d", data_type::helics_double},
    {"s", data_type::helics_string},
    {"f", data_type::helics_double},
    {"v", data_type::helics_vector},
    {"c", data_type::helics_complex},
    {typeid(std::complex<double>).name(), data_type::helics_complex},
    {gmlc::demangle(typeid(std::complex<double>).name()), data_type::helics_complex},
    {"t", data_type::helics_time},
    {"i", data_type::helics_int},
    {"i64", data_type::helics_int},
    {"cv", data_type::helics_complex_vector},
    {"np", data_type::helics_named_point},
    {"point", data_type::helics_named_point},
    {"pt", data_type::helics_named_point},
    {"named_point", data_type::helics_named_point},
    {typeid(std::string).name(), data_type::helics_string},
    {gmlc::demangle(typeid(std::string).name()), data_type::helics_string},
    {typeid(char*).name(), data_type::helics_string},
    {typeid(const char*).name(), data_type::helics_string},
    {"default", data_type::helics_any},
    {"time", data_type::helics_time},
    {typeid(Time).name(), data_type::helics_time},
    {gmlc::demangle(typeid(Time).name()), data_type::helics_time},
    {"tm", data_type::helics_time},
    {"multi", data_type::helics_multi},
    {"many", data_type::helics_multi},
    {"def", data_type::helics_any},
    {"any", data_type::helics_any},
    {"", data_type::helics_any},
    {"all", data_type::helics_any}};

data_type getTypeFromString(const std::string& typeName)
{
    if (!typeName.empty() && typeName.front() == '[') {
        return data_type::helics_multi;
    }
    auto res = typeMap.find(typeName);
    if (res == typeMap.end()) {
        auto lcStr = convertToLowerCase(typeName);
        res = typeMap.find(lcStr);
        if (res == typeMap.end()) {
            return data_type::helics_custom;
        }
    }
    return res->second;
}

// regular expression to handle complex numbers of various formats
const std::regex creg(
    R"(([+-]?(\d+(\.\d+)?|\.\d+)([eE][+-]?\d+)?)\s*([+-]\s*(\d+(\.\d+)?|\.\d+)([eE][+-]?\d+)?)[ji]*)");

std::complex<double> helicsGetComplex(const std::string& val)
{
    if (val.empty()) {
        return invalidValue<std::complex<double>>();
    }
    std::smatch m;
    double re{invalidValue<double>()};
    double im{0.0};
    std::regex_search(val, m, creg);
    try {
        if (m.size() == 9) {
            re = numConv<double>(m[1]);

            im = numConv<double>(m[6]);

            if (*m[5].first == '-') {
                im = -im;
            }
        } else {
            if ((val.back() == 'j') || (val.back() == 'i')) {
                auto strval = val.substr(0, val.size() - 1);
                stringOps::trimString(strval);
                im = numConv<double>(strval);
                re = 0.0;
            } else {
                auto strval = val;
                stringOps::trimString(strval);
                re = numConv<double>(strval);
            }
        }
    }
    catch (const std::invalid_argument&) {
        re = invalidValue<double>();
    }
    return {re, im};
}

std::string helicsVectorString(const std::vector<double>& val)
{
    std::string vString("v");
    vString.append(std::to_string(val.size()));
    vString.push_back('[');
    for (const auto& v : val) {
        vString.append(std::to_string(v));
        vString.push_back(';');
        vString.push_back(' ');
    }
    if (vString.size() > 3)  // 3 for v0[ which would be for an empty vector
    {
        vString.pop_back();
        vString.pop_back();
    }
    vString.push_back(']');
    return vString;
}

std::string helicsVectorString(const double* vals, size_t size)
{
    std::string vString("v");
    vString.append(std::to_string(size));
    vString.push_back('[');
    for (size_t ii = 0; ii < size; ++ii) {
        vString.append(std::to_string(vals[ii]));
        vString.push_back(';');
        vString.push_back(' ');
    }
    if (vString.size() > 3)  // 3 for c0[ which would be for an empty vector
    {
        vString.pop_back();
        vString.pop_back();
    }
    vString.push_back(']');
    return vString;
}

std::string helicsComplexVectorString(const std::vector<std::complex<double>>& val)
{
    std::string vString("c");
    vString.append(std::to_string(val.size()));
    vString.push_back('[');
    for (const auto& v : val) {
        vString.append(helicsComplexString(v.real(), v.imag()));
        vString.push_back(';');
        vString.push_back(' ');
    }
    if (vString.size() > 3) {
        vString.pop_back();
        vString.pop_back();
    }
    vString.push_back(']');
    return vString;
}

std::string helicsNamedPointString(const NamedPoint& point)
{
    return helicsNamedPointString(point.name, point.value);
}
std::string helicsNamedPointString(const std::string& pointName, double val)
{
    std::string retStr = "{\"";
    if (!pointName.empty()) {
        retStr.append(pointName);
    } else {
        retStr.append("value");
    }
    retStr.push_back('"');
    retStr.push_back(':');
    retStr.append(std::to_string(val));
    retStr.push_back('}');
    return retStr;
}

std::string helicsNamedPointString(const char* pointName, double val)
{
    std::string retStr = "{\"";
    if (pointName != nullptr) {
        retStr.append(pointName);
    } else {
        retStr.append("value");
    }
    retStr.push_back('"');
    retStr.push_back(':');
    retStr.append(std::to_string(val));
    retStr.push_back('}');
    return retStr;
}

std::vector<double> helicsGetVector(const std::string& val)
{
    std::vector<double> V;
    helicsGetVector(val, V);
    return V;
}

std::vector<std::complex<double>> helicsGetComplexVector(const std::string& val)
{
    std::vector<std::complex<double>> V;
    helicsGetComplexVector(val, V);
    return V;
}

NamedPoint helicsGetNamedPoint(const std::string& val)
{
    auto loc = val.find_first_of('{');
    if (loc == std::string::npos) {
        auto fb = val.find_first_of('[');
        if (fb != std::string::npos) {
            return {val, getDoubleFromString(val)};
        }
        auto V = helicsGetComplex(val);
        if (V.real() <= invalidDouble) {
            return {val, std::nan("0")};
        }
        if (V.imag() == 0) {
            return {"value", V.real()};
        }
        return {val, V.real()};
    }
    auto locsep = val.find_last_of(':');
    auto locend = val.find_last_of('}');
    auto str1 = val.substr(loc + 1, locsep - loc);
    stringOps::trimString(str1);
    str1.pop_back();

    NamedPoint point;
    point.name = stringOps::removeQuotes(str1);
    auto vstr = val.substr(locsep + 1, locend - locsep - 1);
    stringOps::trimString(vstr);
    point.value = numConv<double>(vstr);
    return point;
}

static int readSize(const std::string& val)
{
    auto fb = val.find_first_of('[');
    if (fb > 1) {
        try {
            auto size = std::stoi(val.substr(1, fb - 1));
            return size;
        }
        catch (const std::invalid_argument&) {
            // go to the alternative path if this fails
        }
    }
    auto res = std::count_if(val.begin() + fb,
                             val.end(),
                             [](auto c) { return (c == ',') || (c == ';'); }) +
        1;
    return static_cast<int>(res);
}

std::complex<double> getComplexFromString(const std::string& val)
{
    if (val.empty()) {
        return invalidValue<std::complex<double>>();
    }
    if ((val.front() == 'v') || (val.front() == 'c') || val.front() == '[') {
        auto V = helicsGetVector(val);
        if (V.empty()) {
            return invalidValue<std::complex<double>>();
        }
        if (V.size() == 1) {
            return {V[0], 0.0};
        }
        return {V[0], V[1]};
    }
    return helicsGetComplex(val);
}

double getDoubleFromString(const std::string& val)
{
    if (val.empty()) {
        return invalidValue<double>();
    }
    if (val.front() == 'v' || val.front() == '[') {
        auto V = helicsGetVector(val);
        return (V.size() != 1) ? vectorNorm(V) : V[0];
    }
    if (val.front() == 'c') {
        auto cv = helicsGetComplexVector(val);
        return (cv.size() != 1) ? vectorNorm(cv) :
                                  ((cv[0].imag() == 0.0) ? cv[0].real() : std::abs(cv[0]));
    }
    auto cval = helicsGetComplex(val);
    return (cval.imag() == 0.0) ? cval.real() : std::abs(cval);
}

void helicsGetVector(const std::string& val, std::vector<double>& data)
{
    if (val.empty()) {
        data.resize(0);
        return;
    }
    if (val.front() == 'v' || val.front() == '[') {
        auto sz = readSize(val);
        if (sz > 0) {
            data.reserve(sz);
        }
        data.resize(0);
        auto fb = val.find_first_of('[');
        for (decltype(sz) ii = 0; ii < sz; ++ii) {
            auto nc = val.find_first_of(";,]", fb + 1);

            std::string vstr = val.substr(fb + 1, nc - fb - 1);
            stringOps::trimString(vstr);
            auto V = numeric_conversion<double>(vstr, invalidValue<double>());
            data.push_back(V);

            fb = nc;
        }
    } else if (val.front() == 'c') {
        auto sz = readSize(val);
        data.reserve(sz * 2);
        data.resize(0);
        auto fb = val.find_first_of('[');
        for (decltype(sz) ii = 0; ii < sz; ++ii) {
            auto nc = val.find_first_of(",;]", fb + 1);
            auto V = helicsGetComplex(val.substr(fb + 1, nc - fb - 1));
            data.push_back(V.real());
            data.push_back(V.imag());
            fb = nc;
        }
    } else {
        auto V = helicsGetComplex(val);
        if (V.imag() == 0) {
            data.resize(1);
            data[0] = V.real();
        } else {
            data.resize(2);
            data[0] = V.real();
            data[1] = V.imag();
        }
    }
}

void helicsGetComplexVector(const std::string& val, std::vector<std::complex<double>>& data)
{
    if (val.empty()) {
        data.resize(0);
        return;
    }
    if (val.front() == 'v') {
        auto sz = readSize(val);
        data.reserve(sz / 2);
        data.resize(0);
        auto fb = val.find_first_of('[');
        for (decltype(sz) ii = 0; ii < sz - 1; ii += 2) {
            auto nc = val.find_first_of(",;]", fb + 1);
            auto nc2 = val.find_first_of(",;]", nc + 1);
            try {
                std::string vstr1 = val.substr(fb + 1, nc - fb - 1);
                stringOps::trimString(vstr1);
                std::string vstr2 = val.substr(nc + 1, nc2 - nc - 1);
                stringOps::trimString(vstr2);
                auto V1 = numConv<double>(vstr1);
                auto V2 = numConv<double>(vstr2);
                data.emplace_back(V1, V2);
            }
            catch (const std::invalid_argument&) {
                data.push_back(invalidValue<std::complex<double>>());
            }
            fb = nc;
        }
    } else if (val.front() == 'c') {
        auto sz = readSize(val);
        data.reserve(sz);
        data.resize(0);
        auto fb = val.find_first_of('[');
        for (decltype(sz) ii = 0; ii < sz; ++ii) {
            auto nc = val.find_first_of(",;]", fb + 1);
            auto V = helicsGetComplex(val.substr(fb + 1, nc - fb - 1));
            data.push_back(V);
            fb = nc;
        }
    } else {
        auto V = helicsGetComplex(val);
        data.resize(0);
        data.push_back(V);
    }
}

bool helicsBoolValue(const std::string& val)
{
    static const std::unordered_map<std::string, bool> knownStrings{

        {"0", false},
        {"00", false},
        {"\0", false},
        {"0000", false},
        {std::string(8, '\0'), false},
        {"1", true},
        {"false", false},
        {"true", true},
        {"on", true},
        {"off", false},
        {"ON", true},
        {"OFF", false},
        {"False", false},
        {"True", true},
        {"FALSE", false},
        {"TRUE", true},
        {"f", false},
        {"t", true},
        {"F", false},
        {"T", true},
        {"n", false},
        {"y", true},
        {"N", false},
        {"Y", true},
        {"no", false},
        {"yes", true},
        {"No", false},
        {"Yes", true},
        {"NO", false},
        {"YES", true},
        {"disable", false},
        {"enable", true},
        {"disabled", false},
        {"enabled", true},
        {std::string{}, false},
    };
    // all known false strings are captured in known strings so if it isn't in there it evaluates to
    // true
    auto res = knownStrings.find(val);
    if (res != knownStrings.end()) {
        return res->second;
    }
    return true;
}

data_block emptyBlock(data_type outputType, data_type inputType = data_type::helics_any)
{
    switch (outputType) {
        case data_type::helics_double:
        default:
            return ValueConverter<double>::convert(0.0);
        case data_type::helics_int:
            return ValueConverter<int64_t>::convert(0);
        case data_type::helics_complex:
            return ValueConverter<std::complex<double>>::convert(std::complex<double>(0.0, 0.0));
        case data_type::helics_bool:
            return "0";
        case data_type::helics_named_point:
            return ValueConverter<NamedPoint>::convert(NamedPoint{"", std::nan("0")});
        case data_type::helics_string:
            switch (inputType) {
                default:
                    return std::string();
                case data_type::helics_vector:
                    return helicsVectorString(std::vector<double>());
                case data_type::helics_complex_vector:
                    return helicsComplexVectorString(std::vector<std::complex<double>>());
                case data_type::helics_named_point:
                    return "{\"\":0}";
            }
        case data_type::helics_complex_vector: {
            return ValueConverter<std::vector<std::complex<double>>>::convert(
                std::vector<std::complex<double>>());
        }
        case data_type::helics_vector:
            return ValueConverter<std::vector<double>>::convert(std::vector<double>());
    }
}
data_block typeConvert(data_type type, double val)
{
    switch (type) {
        case data_type::helics_double:
        default:
            return ValueConverter<double>::convert(val);
        case data_type::helics_int:
            return ValueConverter<int64_t>::convert(static_cast<int64_t>(val));
        case data_type::helics_complex:
            return ValueConverter<std::complex<double>>::convert(std::complex<double>(val, 0.0));
        case data_type::helics_bool:
            return (val != 0.0) ? "1" : "0";
        case data_type::helics_string:
            return std::to_string(val);
        case data_type::helics_named_point:
            return ValueConverter<NamedPoint>::convert(NamedPoint{"value", val});
        case data_type::helics_complex_vector: {
            std::complex<double> v2(val, 0.0);
            return ValueConverter<std::complex<double>>::convert(&v2, 1);
        }
        case data_type::helics_vector:
            return ValueConverter<double>::convert(&val, 1);
    }
}
data_block typeConvert(data_type type, int64_t val)
{
    switch (type) {
        case data_type::helics_double:
            return ValueConverter<double>::convert(static_cast<double>(val));
        case data_type::helics_int:
        default:
            return ValueConverter<int64_t>::convert(val);
        case data_type::helics_complex:
            return ValueConverter<std::complex<double>>::convert(
                std::complex<double>(static_cast<double>(val), 0.0));
        case data_type::helics_bool:
            return (val != 0) ? "1" : "0";
        case data_type::helics_string:
            return std::to_string(val);
        case data_type::helics_named_point:
            if (static_cast<uint64_t>(std::abs(val)) >
                (2ULL << 51U))  // this checks whether the actual value will fit in a double
            {
                return ValueConverter<NamedPoint>::convert(
                    NamedPoint{std::to_string(val), std::nan("0")});
            } else {
                return ValueConverter<NamedPoint>::convert(
                    NamedPoint{"value", static_cast<double>(val)});
            }

        case data_type::helics_complex_vector: {
            std::complex<double> v2(static_cast<double>(val), 0.0);
            return ValueConverter<std::complex<double>>::convert(&v2, 1);
        }
        case data_type::helics_vector: {
            auto v2 = static_cast<double>(val);
            return ValueConverter<double>::convert(&v2, 1);
        }
    }
}

data_block typeConvert(data_type type, const char* val)
{
    if (val == nullptr) {
        return emptyBlock(type);
    }
    switch (type) {
        case data_type::helics_double:
            return ValueConverter<double>::convert(getDoubleFromString(val));
        case data_type::helics_int:
            return ValueConverter<int64_t>::convert(static_cast<int64_t>(getDoubleFromString(val)));
        case data_type::helics_complex:
            return ValueConverter<std::complex<double>>::convert(helicsGetComplex(val));
        case data_type::helics_bool:
            return (helicsBoolValue(val)) ? "0" : "1";
        case data_type::helics_string:
        default:
            return data_block(val);
        case data_type::helics_named_point:
            return ValueConverter<NamedPoint>::convert(NamedPoint{val, std::nan("0")});
        case data_type::helics_complex_vector:
            return ValueConverter<std::vector<std::complex<double>>>::convert(
                helicsGetComplexVector(val));
        case data_type::helics_vector:
            return ValueConverter<std::vector<double>>::convert(helicsGetVector(val));
    }
}

data_block typeConvert(data_type type, const std::string& val)
{
    if (val.empty()) {
        return emptyBlock(type);
    }

    switch (type) {
        case data_type::helics_double:
            return ValueConverter<double>::convert(getDoubleFromString(val));
        case data_type::helics_int:
            return ValueConverter<int64_t>::convert(static_cast<int64_t>(getDoubleFromString(val)));
        case data_type::helics_complex:
            return ValueConverter<std::complex<double>>::convert(helicsGetComplex(val));
        case data_type::helics_bool:
            return (helicsBoolValue(val)) ? "1" : "0";
        case data_type::helics_string:
        default:
            return val;
        case data_type::helics_named_point:
            return ValueConverter<NamedPoint>::convert(NamedPoint{val, std::nan("0")});
        case data_type::helics_complex_vector:
            return ValueConverter<std::vector<std::complex<double>>>::convert(
                helicsGetComplexVector(val));
        case data_type::helics_vector:
            return ValueConverter<std::vector<double>>::convert(helicsGetVector(val));
    }
}

data_block typeConvert(data_type type, const std::vector<double>& val)
{
    if (val.empty()) {
        return emptyBlock(type, data_type::helics_vector);
    }
    switch (type) {
        case data_type::helics_double:
            return ValueConverter<double>::convert(val[0]);

        case data_type::helics_int:
            return ValueConverter<int64_t>::convert(static_cast<int64_t>(val[0]));
        case data_type::helics_complex: {
            std::complex<double> V(0.0, 0.0);
            if (val.size() >= 2) {
                V = std::complex<double>(val[0], val[1]);
            } else if (val.size() == 1) {
                V = std::complex<double>(val[0], 0.0);
            }
            return ValueConverter<std::complex<double>>::convert(V);
        }
        case data_type::helics_bool:
            return (vectorNorm(val) != 0.0) ? "1" : "0";
        case data_type::helics_string:
            return helicsVectorString(val);
        case data_type::helics_named_point:
            return ValueConverter<NamedPoint>::convert(
                NamedPoint{helicsVectorString(val), std::nan("0")});
        case data_type::helics_complex_vector: {
            std::vector<std::complex<double>> CD;
            CD.reserve(val.size() / 2);
            for (size_t ii = 0; ii < val.size() - 1; ii += 2) {
                CD.emplace_back(val[ii], val[ii + 1]);
            }
            return ValueConverter<std::vector<std::complex<double>>>::convert(CD);
        } break;
        case data_type::helics_vector:
        default:
            return ValueConverter<std::vector<double>>::convert(val);
    }
}

data_block typeConvert(data_type type, const double* vals, size_t size)
{
    if ((vals == nullptr) || (size == 0)) {
        return emptyBlock(type, data_type::helics_vector);
    }
    switch (type) {
        case data_type::helics_double:
            return ValueConverter<double>::convert(vals[0]);

        case data_type::helics_int:
            return ValueConverter<int64_t>::convert(static_cast<int64_t>(vals[0]));
        case data_type::helics_complex: {
            std::complex<double> V(0.0, 0.0);
            if (size >= 2) {
                V = std::complex<double>(vals[0], vals[1]);
            } else if (size == 1) {
                V = std::complex<double>(vals[0], 0.0);
            }
            return ValueConverter<std::complex<double>>::convert(V);
        }
        case data_type::helics_bool:
            for (size_t ii = 0; ii < size; ++ii) {
                if (vals[ii] != 0) {
                    return "1";
                }
            }
            return "0";
            break;
        case data_type::helics_string:
            return helicsVectorString(vals, size);
        case data_type::helics_named_point:
            return ValueConverter<NamedPoint>::convert(
                NamedPoint{helicsVectorString(vals, size), std::nan("0")});
        case data_type::helics_complex_vector: {
            std::vector<std::complex<double>> CD;
            CD.reserve(size / 2);
            for (size_t ii = 0; ii < size - 1; ii += 2) {
                CD.emplace_back(vals[ii], vals[ii + 1]);
            }
            return ValueConverter<std::vector<std::complex<double>>>::convert(CD);
        } break;
        case data_type::helics_vector:
        default:
            return ValueConverter<double>::convert(vals, size);
    }
}

data_block typeConvert(data_type type, const std::vector<std::complex<double>>& val)
{
    if (val.empty()) {
        return emptyBlock(type, data_type::helics_complex_vector);
    }
    switch (type) {
        case data_type::helics_double:
            return ValueConverter<double>::convert(std::abs(val[0]));
        case data_type::helics_int:
            return ValueConverter<int64_t>::convert(
                static_cast<int64_t>(std::abs(val[0])));  // NOLINT
        case data_type::helics_complex:
            return ValueConverter<std::complex<double>>::convert(val[0]);
        case data_type::helics_bool:
            return (vectorNorm(val) != 0.0) ? "1" : "0";
        case data_type::helics_string:
            return helicsComplexVectorString(val);
        case data_type::helics_named_point:
            return ValueConverter<NamedPoint>::convert(
                NamedPoint{helicsComplexVectorString(val), std::nan("0")});
        case data_type::helics_complex_vector:
        default:
            return ValueConverter<std::vector<std::complex<double>>>::convert(val);
        case data_type::helics_vector: {
            std::vector<double> DV;
            DV.reserve(val.size() * 2);
            for (const auto& vali : val) {
                DV.push_back(vali.real());
                DV.push_back(vali.imag());
            }
            return ValueConverter<std::vector<double>>::convert(DV);
        }
    }
}
data_block typeConvert(data_type type, const std::complex<double>& val)
{
    switch (type) {
        case data_type::helics_double:
            return ValueConverter<double>::convert(std::abs(val));
        case data_type::helics_int:
            return ValueConverter<int64_t>::convert(static_cast<int64_t>(std::abs(val)));
        case data_type::helics_complex:
        default:
            return ValueConverter<std::complex<double>>::convert(val);
        case data_type::helics_bool:
            return (std::abs(val) != 0.0) ? "1" : "0";
        case data_type::helics_string:
            return helicsComplexString(val);
        case data_type::helics_named_point:
            if (val.imag() == 0) {
                return ValueConverter<NamedPoint>::convert(NamedPoint{"value", val.real()});
            } else {
                return ValueConverter<NamedPoint>::convert(
                    NamedPoint{helicsComplexString(val), std::nan("0")});
            }
        case data_type::helics_complex_vector:
            return ValueConverter<std::complex<double>>::convert(&val, 1);
        case data_type::helics_vector: {
            std::vector<double> V{val.real(), val.imag()};
            return ValueConverter<std::vector<double>>::convert(V);
        }
    }
}

data_block typeConvert(data_type type, const NamedPoint& val)
{
    if (type == data_type::helics_named_point) {
        return ValueConverter<NamedPoint>::convert(val);
    }
    if (std::isnan(val.value)) {
        // just convert the string
        return typeConvert(type, val.name);
    }
    switch (type) {
        case data_type::helics_double:
            return ValueConverter<double>::convert(val.value);
        case data_type::helics_int:
            return ValueConverter<int64_t>::convert(static_cast<int64_t>(val.value));
        case data_type::helics_complex:
            return ValueConverter<std::complex<double>>::convert(
                std::complex<double>(val.value, 0.0));
        case data_type::helics_bool:
            return (val.value != 0) ? "1" : "0";
        case data_type::helics_named_point:
        default:
            return ValueConverter<NamedPoint>::convert(val);
        case data_type::helics_string:
            return (std::isnan(val.value)) ? val.name : helicsNamedPointString(val);
        case data_type::helics_complex_vector: {
            std::complex<double> v2(val.value, 0.0);
            return ValueConverter<std::complex<double>>::convert(&v2, 1);
        }
        case data_type::helics_vector:
            return ValueConverter<double>::convert(&(val.value), 1);
    }
}

data_block typeConvert(data_type type, const char* str, double val)
{
    if (type == data_type::helics_named_point) {
        return ValueConverter<NamedPoint>::convert(NamedPoint(str, val));
    }
    if (std::isnan(val)) {
        // just convert the string
        return typeConvert(type, str);
    }
    switch (type) {
        case data_type::helics_double:
            return ValueConverter<double>::convert(val);
        case data_type::helics_int:
            return ValueConverter<int64_t>::convert(static_cast<int64_t>(val));
        case data_type::helics_complex:
            return ValueConverter<std::complex<double>>::convert(std::complex<double>(val, 0.0));
        case data_type::helics_bool:
            return (val != 0) ? "1" : "0";
        case data_type::helics_named_point:
        default:
            return ValueConverter<NamedPoint>::convert(NamedPoint(str, val));
        case data_type::helics_string:
            return helicsNamedPointString(str, val);
        case data_type::helics_complex_vector: {
            std::complex<double> v2(val, 0.0);
            return ValueConverter<std::complex<double>>::convert(&v2, 1);
        }
        case data_type::helics_vector:
            return ValueConverter<double>::convert(&(val), 1);
    }
}

data_block typeConvert(data_type type, bool val)
{
    switch (type) {
        case data_type::helics_double:
            return ValueConverter<double>::convert(val ? 1.0 : 0.0);
        case data_type::helics_int:
            return ValueConverter<int64_t>::convert(val ? 1 : 0);
        case data_type::helics_complex:
            return ValueConverter<std::complex<double>>::convert(
                std::complex<double>(val ? 1.0 : 0.0, 0.0));
        case data_type::helics_string:
        case data_type::helics_bool:
        default:
            return val ? "1" : "0";
        case data_type::helics_named_point: {
            NamedPoint np{"value", val ? 1.0 : 0.0};
            return ValueConverter<NamedPoint>::convert(np);
        }

        case data_type::helics_complex_vector: {
            std::complex<double> v2(val ? 1.0 : 0.0, 0.0);
            return ValueConverter<std::complex<double>>::convert(&v2, 1);
        }
        case data_type::helics_vector: {
            auto v2 = val ? 1.0 : 0.0;
            return ValueConverter<double>::convert(&v2, 1);
        }
    }
}

data_block typeConvert(data_type type, const std::string& str, double val)
{
    return typeConvert(type, str.c_str(), val);
}

}  // namespace helics
