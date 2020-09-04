/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helicsTypes.hpp"

#include "../common/JsonGeneration.hpp"
#include "../common/JsonProcessingFunctions.hpp"
#include "../common/frozen_map.h"
#include "ValueConverter.hpp"
#include "fmt/format.h"
#include "gmlc/utilities/demangle.hpp"
#include "gmlc/utilities/stringConversion.h"
#include "gmlc/utilities/stringOps.h"
#include "gmlc/utilities/string_viewConversion.h"

#include <algorithm>
#include <functional>
#include <numeric>
#include <regex>
#include <sstream>
#include <string_view>
#include <unordered_map>

using namespace gmlc::utilities;  // NOLINT

template<>
struct fmt::formatter<std::complex<double>> {
    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    static constexpr auto parse(format_parse_context& ctx) { return ctx.end(); }

    template<typename FormatContext>
    auto format(const std::complex<double>& p, FormatContext& ctx)
    {
        // ctx.out() is an output iterator to write to.
        return format_to(ctx.out(), "[{:.9g},{:.9g}]", p.real(), p.imag());
    }
};

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
    return (imag != 0.0) ? fmt::format(FMT_STRING("[{:.9g},{:.9g}]"), real, imag) :
                           fmt::format(FMT_STRING("{:.9g}"), real);
}

std::string helicsComplexString(std::complex<double> val)
{
    return helicsComplexString(val.real(), val.imag());
}
/** map of an assortment of type string that can be converted to a known type*/
static constexpr frozen::unordered_map<frozen::string, data_type, 56> typeMap{
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
    {"complex", data_type::helics_complex},
    {"pair", data_type::helics_complex},
    {"int", data_type::helics_int},
    {"int64", data_type::helics_int},
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
    {"char8_t", data_type::helics_string},
    {"complex_vector", data_type::helics_complex_vector},
    {"complex vector", data_type::helics_complex_vector},
    {"d", data_type::helics_double},
    {"s", data_type::helics_string},
    {"f", data_type::helics_double},
    {"v", data_type::helics_vector},
    // typeid(char).name sometimes is produced from char
    {"c", data_type::helics_string},
    {"t", data_type::helics_time},
    {"i", data_type::helics_int},
    {"i64", data_type::helics_int},
    {"cv", data_type::helics_complex_vector},
    {"np", data_type::helics_named_point},
    {"point", data_type::helics_named_point},
    {"pt", data_type::helics_named_point},
    {"named_point", data_type::helics_named_point},
    {"default", data_type::helics_any},
    {"time", data_type::helics_time},
    {"tm", data_type::helics_time},
    {"multi", data_type::helics_multi},
    {"many", data_type::helics_multi},
    {"def", data_type::helics_any},
    {"any", data_type::helics_any},
    {"", data_type::helics_any},
    {"all", data_type::helics_any}};

static const std::unordered_map<std::string, data_type> demangle_names{
    {gmlc::demangle(typeid(Time).name()), data_type::helics_time},
    {gmlc::demangle(typeid(std::string).name()), data_type::helics_string},
    {gmlc::demangle(typeid(std::complex<double>).name()), data_type::helics_complex},
    {gmlc::demangle(typeid(std::vector<double>).name()), data_type::helics_vector},
    {gmlc::demangle(typeid(std::vector<std::complex<double>>).name()),
     data_type::helics_complex_vector},
    {typeid(std::vector<double>).name(), data_type::helics_vector},
    {typeid(double*).name(), data_type::helics_vector},
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
    {typeid(std::vector<std::complex<double>>).name(), data_type::helics_complex_vector},
    {typeid(std::complex<double>).name(), data_type::helics_complex},
    {typeid(std::string).name(), data_type::helics_string},
    {typeid(char*).name(), data_type::helics_string},
    {typeid(const char*).name(), data_type::helics_string},
    {typeid(Time).name(), data_type::helics_time}};

data_type getTypeFromString(std::string_view typeName)
{
    if (!typeName.empty() && typeName.front() == '[') {
        return data_type::helics_multi;
    }
    const auto* res = typeMap.find(frozen::string(typeName.data(), typeName.size()));
    if (res != typeMap.end()) {
        return res->second;
    }
    std::string strName(typeName);
    auto dres = demangle_names.find(strName);
    if (dres != demangle_names.end()) {
        return dres->second;
    }
    makeLowerCase(strName);
    res = typeMap.find(frozen::string(strName.data(), strName.size()));
    if (res != typeMap.end()) {
        return res->second;
    }
    dres = demangle_names.find(strName);
    if (dres != demangle_names.end()) {
        return dres->second;
    }
    return data_type::helics_custom;
}

std::string_view getCleanedTypeName(std::string_view typeName)
{
    if (!typeName.empty() && typeName.front() == '[') {
        return typeName;
    }
    const auto* res = typeMap.find(frozen::string(typeName.data(), typeName.size()));
    if (res != typeMap.end()) {
        return typeName;
    }
    std::string strName(typeName);
    auto dres = demangle_names.find(strName);
    if (dres != demangle_names.end()) {
        return typeNameStringRef(dres->second);
    }
    makeLowerCase(strName);
    res = typeMap.find(frozen::string(strName.data(), strName.size()));
    if (res != typeMap.end()) {
        return typeName;
    }
    dres = demangle_names.find(strName);
    if (dres != demangle_names.end()) {
        return typeNameStringRef(dres->second);
    }
    return typeName;
}

// regular expression to handle complex numbers of various formats
const std::regex creg(
    R"(([+-]?(\d+(\.\d+)?|\.\d+)([eE][+-]?\d+)?)\s*([+-]\s*(\d+(\.\d+)?|\.\d+)([eE][+-]?\d+)?)[ji]*)");

std::complex<double> helicsGetComplex(std::string_view val)
{
    if (val.empty()) {
        return invalidValue<std::complex<double>>();
    }
    double re{invalidValue<double>()};
    double im{0.0};
    if (val.front() == '[') {
        auto sep = val.find_first_of(',');
        if (sep == std::string_view::npos) {
            val.remove_prefix(1);
            val.remove_suffix(1);
            re = numConv<double>(val);
            return {re, im};
        }
        if (val.find_first_of(',', sep + 1) != std::string_view::npos) {
            auto V = helicsGetVector(val);
            if (V.size() >= 2) {
                return {V[0], V[1]};
            }
            return invalidValue<std::complex<double>>();
        }
        re = numConv<double>(val.substr(1, sep));
        val.remove_suffix(1);
        im = numConv<double>(val.substr(sep + 1));
        return {re, im};
    }
    std::smatch m;

    auto temp = std::string(val);
    std::regex_search(temp, m, creg);
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
                string_viewOps::trimString(strval);
                im = numConv<double>(strval);
                re = 0.0;
            } else {
                auto strval = val;
                string_viewOps::trimString(strval);
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
    return fmt::format("[{:g}]", fmt::join(val, ","));
}

std::string helicsVectorString(const double* vals, size_t size)
{
    return fmt::format("[{:g}]", fmt::join(vals, vals + size, ","));
}

std::string helicsComplexVectorString(const std::vector<std::complex<double>>& val)
{
    return fmt::format("[{}]", fmt::join(val, ","));
}

std::string helicsNamedPointString(const NamedPoint& point)
{
    return helicsNamedPointString(point.name, point.value);
}
std::string helicsNamedPointString(std::string_view pointName, double val)
{
    Json::Value NP;
    NP["value"] = val;
    if (pointName.empty()) {
    } else {
        NP["name"] = Json::Value(pointName.data(), pointName.data() + pointName.size());
    }
    return generateJsonString(NP);
}

std::vector<double> helicsGetVector(std::string_view val)
{
    std::vector<double> V;
    helicsGetVector(val, V);
    return V;
}

std::vector<std::complex<double>> helicsGetComplexVector(std::string_view val)
{
    std::vector<std::complex<double>> V;
    helicsGetComplexVector(val, V);
    return V;
}

NamedPoint helicsGetNamedPoint(std::string_view val)
{
    NamedPoint p;
    try {
        auto jv = loadJsonStr(val);
        switch (jv.type()) {
            case Json::ValueType::realValue:
                p.value = jv.asDouble();
                p.name = "value";
                break;
            case Json::ValueType::stringValue:
                p.name = jv.asString();
                break;
            case Json::ValueType::arrayValue:
                break;
            case Json::ValueType::intValue:
            case Json::ValueType::uintValue:
                p.value = static_cast<double>(jv.asInt());
                p.name = "value";
                break;
            case Json::ValueType::objectValue:
                replaceIfMember(jv, "value", p.value);
                replaceIfMember(jv, "name", p.name);
                break;
            default:
                break;
        }
    }
    catch (...) {
        p.name = val;
    }
    return p;
}

static int readSize(std::string_view val)
{
    auto fb = val.find_first_of('[');
    if (fb > 1) {
        try {
            auto size = numConv<int>(val.substr(1, fb - 1));
            return size;
        }
        catch (const std::invalid_argument&) {
            // go to the alternative path if this fails
        }
    }
    if (val.find_first_not_of(" ]", fb + 1) == std::string_view::npos) {
        return 0;
    }
    auto res = std::count_if(val.begin() + fb,
                             val.end(),
                             [](auto c) { return (c == ',') || (c == ';'); }) +
        1;
    return static_cast<int>(res);
}

std::complex<double> getComplexFromString(std::string_view val)
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

double getDoubleFromString(std::string_view val)
{
    if (val.empty()) {
        return invalidValue<double>();
    }
    if (val.front() == 'v' || val.front() == '[') {
        auto V = helicsGetVector(val);
        return vectorNorm(V);
    }
    if (val.front() == 'c') {
        auto cv = helicsGetComplexVector(val);
        return vectorNorm(cv);
    }
    return std::abs(helicsGetComplex(val));
}

void helicsGetVector(std::string_view val, std::vector<double>& data)
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

            auto vstr = val.substr(fb + 1, nc - fb - 1);
            string_viewOps::trimString(vstr);
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

void helicsGetComplexVector(std::string_view val, std::vector<std::complex<double>>& data)
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
                auto vstr1 = val.substr(fb + 1, nc - fb - 1);
                string_viewOps::trimString(vstr1);
                auto vstr2 = val.substr(nc + 1, nc2 - nc - 1);
                string_viewOps::trimString(vstr2);
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
        if (val.find_first_of("ji") != std::string_view::npos) {
            auto V = helicsGetComplex(val);
            data.resize(0);
            data.push_back(V);
        } else {
            auto JV = loadJsonStr(val);
            int cnt = 0;
            switch (JV.type()) {
                case Json::ValueType::realValue:
                case Json::ValueType::intValue:
                case Json::ValueType::uintValue:
                    data.resize(0);
                    data.emplace_back(JV.asDouble(), 0.0);
                    break;
                case Json::ValueType::arrayValue:
                    for (auto& av : JV) {
                        if (av.isNumeric()) {
                            if (cnt == 0) {
                                data.emplace_back(av.asDouble(), 0.0);
                                cnt = 1;
                            } else {
                                data.back() += std::complex<double>{0.0, av.asDouble()};
                                cnt = 0;
                            }
                        } else if (av.isArray()) {
                            cnt = 0;
                            if (av.size() >= 2) {
                                if (av[0].isNumeric()) {
                                    data.emplace_back(av[0].asDouble(), av[1].asDouble());
                                }
                            } else if (av.size() == 1) {
                                if (av[0].isNumeric()) {
                                    data.emplace_back(av[0].asDouble(), 0.0);
                                }
                            } else {
                                data.push_back(invalidValue<std::complex<double>>());
                            }
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

bool helicsBoolValue(std::string_view val)
{
    static constexpr const frozen::unordered_map<frozen::string, bool, 35> knownStrings{

        {"0", false},
        {"00", false},
        {frozen::string("\0", 1), false},
        {"0000", false},
        {frozen::string("\0\0\0\0\0\0\0\0", 8), false},
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
        {"", false}};
    // all known false strings are captured in known strings so if it isn't in there it evaluates to
    // true
    const auto* res = knownStrings.find(frozen::string(val.data(), val.size()));
    if (res != knownStrings.end()) {
        return res->second;
    }
    return true;
}

SmallBuffer emptyBlock(data_type outputType, data_type inputType = data_type::helics_any)
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
            return ValueConverter<std::string_view>::convert("0");
        case data_type::helics_named_point:
            return ValueConverter<NamedPoint>::convert(NamedPoint{"", std::nan("0")});
        case data_type::helics_string:
            switch (inputType) {
                default:
                    return ValueConverter<std::string_view>::convert("");
                case data_type::helics_vector:
                    return ValueConverter<std::string_view>::convert(
                        helicsVectorString(std::vector<double>()));
                case data_type::helics_complex_vector:
                    return ValueConverter<std::string_view>::convert(
                        helicsComplexVectorString(std::vector<std::complex<double>>()));
                case data_type::helics_named_point:
                    return ValueConverter<std::string_view>::convert("0");
            }
        case data_type::helics_complex_vector: {
            return ValueConverter<std::vector<std::complex<double>>>::convert(
                std::vector<std::complex<double>>());
        }
        case data_type::helics_vector:
            return ValueConverter<std::vector<double>>::convert(std::vector<double>());
    }
}
SmallBuffer typeConvert(data_type type, double val)
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
            return ValueConverter<std::string_view>::convert((val != 0.0) ? "1" : "0");
        case data_type::helics_string:
            return ValueConverter<std::string_view>::convert(std::to_string(val));
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
SmallBuffer typeConvert(data_type type, int64_t val)
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
            return ValueConverter<std::string_view>::convert((val != 0) ? "1" : "0");
        case data_type::helics_string:
            return ValueConverter<std::string_view>::convert(std::to_string(val));
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

SmallBuffer typeConvert(data_type type, std::string_view val)
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
            return ValueConverter<std::string_view>::convert((helicsBoolValue(val)) ? "1" : "0");
        case data_type::helics_string:
        default:
            return ValueConverter<std::string_view>::convert(val);
        case data_type::helics_named_point:
            return ValueConverter<NamedPoint>::convert(NamedPoint{val, std::nan("0")});
        case data_type::helics_complex_vector:
            return ValueConverter<std::vector<std::complex<double>>>::convert(
                helicsGetComplexVector(val));
        case data_type::helics_vector:
            return ValueConverter<std::vector<double>>::convert(helicsGetVector(val));
    }
}

SmallBuffer typeConvert(data_type type, const std::vector<double>& val)
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
            return ValueConverter<std::string_view>::convert((vectorNorm(val) != 0.0) ? "1" : "0");
        case data_type::helics_string:
            return ValueConverter<std::string_view>::convert(helicsVectorString(val));
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

SmallBuffer typeConvert(data_type type, const double* vals, size_t size)
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
                    return ValueConverter<std::string_view>::convert("1");
                }
            }
            return ValueConverter<std::string_view>::convert("0");
            break;
        case data_type::helics_string:
            return ValueConverter<std::string_view>::convert(helicsVectorString(vals, size));
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

SmallBuffer typeConvert(data_type type, const std::vector<std::complex<double>>& val)
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
            return ValueConverter<std::string_view>::convert((vectorNorm(val) != 0.0) ? "1" : "0");
        case data_type::helics_string:
            return ValueConverter<std::string_view>::convert(helicsComplexVectorString(val));
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
SmallBuffer typeConvert(data_type type, const std::complex<double>& val)
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
            return ValueConverter<std::string_view>::convert((std::abs(val) != 0.0) ? "1" : "0");
        case data_type::helics_string:
            return ValueConverter<std::string_view>::convert(helicsComplexString(val));
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

SmallBuffer typeConvert(data_type type, const NamedPoint& val)
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
            return ValueConverter<std::string_view>::convert((val.value != 0) ? "1" : "0");
        case data_type::helics_named_point:
        default:
            return ValueConverter<NamedPoint>::convert(val);
        case data_type::helics_string:
            return ValueConverter<std::string_view>::convert(
                (std::isnan(val.value)) ? val.name : helicsNamedPointString(val));
        case data_type::helics_complex_vector: {
            std::complex<double> v2(val.value, 0.0);
            return ValueConverter<std::complex<double>>::convert(&v2, 1);
        }
        case data_type::helics_vector:
            return ValueConverter<double>::convert(&(val.value), 1);
    }
}

SmallBuffer typeConvert(data_type type, std::string_view str, double val)
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
            return ValueConverter<std::string_view>::convert((val != 0.0) ? "1" : "0");
        case data_type::helics_named_point:
        default:
            return ValueConverter<NamedPoint>::convert(NamedPoint(str, val));
        case data_type::helics_string:
            return ValueConverter<std::string_view>::convert(helicsNamedPointString(str, val));
        case data_type::helics_complex_vector: {
            std::complex<double> v2(val, 0.0);
            return ValueConverter<std::complex<double>>::convert(&v2, 1);
        }
        case data_type::helics_vector:
            return ValueConverter<double>::convert(&(val), 1);
    }
}

SmallBuffer typeConvert(data_type type, bool val)
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
            return ValueConverter<std::string_view>::convert(val ? "1" : "0");
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

}  // namespace helics
