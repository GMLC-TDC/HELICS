/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helicsTypes.hpp"

#include "../common/JsonGeneration.hpp"
#include "../common/JsonProcessingFunctions.hpp"
#include "../common/frozen_map.h"
#include "ValueConverter.hpp"
#include "gmlc/utilities/demangle.hpp"
#include "gmlc/utilities/stringConversion.h"
#include "gmlc/utilities/stringOps.h"
#include "gmlc/utilities/string_viewConversion.h"

#include <algorithm>
#include <fmt/format.h>
#if FMT_VERSION >= 110000
#    include <fmt/ranges.h>
#endif
#include <functional>
#include <numeric>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace gmlc::utilities;  // NOLINT

template<>
struct fmt::formatter<std::complex<double>> {
    // Formats std::complex

    static constexpr auto parse(format_parse_context& ctx) { return ctx.end(); }

    template<typename FormatContext>
    auto format(const std::complex<double>& value, FormatContext& ctx) const
    {
        // ctx.out() is an output iterator to write to.
        // return format(ctx.out(), "[{},{}]",p.real(), p.imag());
        return fmt::format_to(ctx.out(), "[{},{}]", value.real(), value.imag());
    }
};

namespace frozen {
template<>
struct elsa<std::string_view> {
    constexpr std::size_t operator()(std::string_view value) const { return hash_string(value); }
    constexpr std::size_t operator()(std::string_view value, std::size_t seed) const
    {
        return hash_string(value, seed);
    }
};
}  // namespace frozen

namespace helics {

const std::string& typeNameStringRef(DataType type)
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
    static const std::string jsonString("json");
    static const std::string charString("char");
    static const std::string anyString("any");
    static const std::string nullString;
    switch (type) {
        case DataType::HELICS_DOUBLE:
            return doubleString;
        case DataType::HELICS_INT:
            return intString;
        case DataType::HELICS_BOOL:
            return boolString;
        case DataType::HELICS_STRING:
            return stringString;
        case DataType::HELICS_COMPLEX:
            return complexString;
        case DataType::HELICS_TIME:
            return timeString;
        case DataType::HELICS_VECTOR:
            return doubleVecString;
        case DataType::HELICS_COMPLEX_VECTOR:
            return complexVecString;
        case DataType::HELICS_NAMED_POINT:
            return namedPointString;
        case DataType::HELICS_JSON:
            return jsonString;
        case DataType::HELICS_CHAR:
            return charString;
        case DataType::HELICS_ANY:
            return anyString;
        default:
            return nullString;
    }
}

double vectorNorm(const std::vector<double>& vec)
{
    return std::sqrt(std::inner_product(vec.begin(), vec.end(), vec.begin(), 0.0));
}

double vectorNorm(const double* vec, std::size_t size)
{
    return std::sqrt(std::inner_product(vec, vec + size, vec, 0.0));
}

double vectorNorm(const std::vector<std::complex<double>>& vec)
{
    return std::sqrt(std::inner_product(vec.begin(),
                                        vec.end(),
                                        vec.begin(),
                                        0.0,
                                        std::plus<>(),
                                        [](const auto& avec, const auto& bvec) {
                                            return (avec * std::conj(bvec)).real();
                                        }));
}

std::string helicsComplexString(double real, double imag)
{
    return (imag != 0.0) ? fmt::format(FMT_STRING("[{},{}]"), real, imag) :
                           fmt::format(FMT_STRING("{}"), real);
}

std::string helicsComplexString(std::complex<double> val)
{
    return helicsComplexString(val.real(), val.imag());
}
/** map of an assortment of type string that can be converted to a known type*/
static constexpr frozen::unordered_map<std::string_view, DataType, 64> typeMap{
    {"double", DataType::HELICS_DOUBLE},
    {"string", DataType::HELICS_STRING},
    {"binary", DataType::HELICS_BOOL},
    {"bool", DataType::HELICS_BOOL},
    {"boolean", DataType::HELICS_BOOL},
    {"flag", DataType::HELICS_BOOL},
    {"float", DataType::HELICS_DOUBLE},
    {"vector", DataType::HELICS_VECTOR},
    {"double_vector", DataType::HELICS_VECTOR},
    {"double vector", DataType::HELICS_VECTOR},
    {"doublevector", DataType::HELICS_VECTOR},
    {"complex", DataType::HELICS_COMPLEX},
    {"pair", DataType::HELICS_COMPLEX},
    {"int", DataType::HELICS_INT},
    {"int64", DataType::HELICS_INT},
    {"long long", DataType::HELICS_INT},
    {"longlong", DataType::HELICS_INT},
    {"integer", DataType::HELICS_INT},
    {"int32", DataType::HELICS_INT},
    {"uint32", DataType::HELICS_INT},
    {"uint64", DataType::HELICS_INT},
    {"int16", DataType::HELICS_INT},
    {"uint16", DataType::HELICS_INT},
    {"short", DataType::HELICS_INT},
    {"unsigned short", DataType::HELICS_INT},
    {"unsignedshort", DataType::HELICS_INT},
    {"long", DataType::HELICS_INT},
    {"unsigned long", DataType::HELICS_INT},
    {"unsignedlong", DataType::HELICS_INT},
    {"char", DataType::HELICS_CHAR},
    {"uchar", DataType::HELICS_INT},
    {"unsigned char", DataType::HELICS_INT},
    {"unsignedchar", DataType::HELICS_INT},
    {"byte", DataType::HELICS_INT},
    {"int8", DataType::HELICS_INT},
    {"uint8", DataType::HELICS_INT},
    {"char8_t", DataType::HELICS_CHAR},
    {"complex_vector", DataType::HELICS_COMPLEX_VECTOR},
    {"complex vector", DataType::HELICS_COMPLEX_VECTOR},
    {"complexvector", DataType::HELICS_COMPLEX_VECTOR},
    {"d", DataType::HELICS_DOUBLE},
    {"s", DataType::HELICS_STRING},
    {"f", DataType::HELICS_DOUBLE},
    {"v", DataType::HELICS_VECTOR},
    // typeid(char).name sometimes is produced from char
    {"c", DataType::HELICS_CHAR},
    {"t", DataType::HELICS_TIME},
    {"i", DataType::HELICS_INT},
    {"i64", DataType::HELICS_INT},
    {"cv", DataType::HELICS_COMPLEX_VECTOR},
    {"np", DataType::HELICS_NAMED_POINT},
    {"point", DataType::HELICS_NAMED_POINT},
    {"pt", DataType::HELICS_NAMED_POINT},
    {"named_point", DataType::HELICS_NAMED_POINT},
    {"namedpoint", DataType::HELICS_NAMED_POINT},
    {"default", DataType::HELICS_ANY},
    {"time", DataType::HELICS_TIME},
    {"tm", DataType::HELICS_TIME},
    {"multi", DataType::HELICS_MULTI},
    {"many", DataType::HELICS_MULTI},
    {"json", DataType::HELICS_JSON},
    {"def", DataType::HELICS_ANY},
    {"any", DataType::HELICS_ANY},
    {"", DataType::HELICS_ANY},
    {"all", DataType::HELICS_ANY}};

static const std::unordered_map<std::string, DataType> demangle_names{
    {gmlc::demangle(typeid(Time).name()), DataType::HELICS_TIME},
    {gmlc::demangle(typeid(std::string).name()), DataType::HELICS_STRING},
    {gmlc::demangle(typeid(std::string_view).name()), DataType::HELICS_STRING},
    {gmlc::demangle(typeid(std::complex<double>).name()), DataType::HELICS_COMPLEX},
    {gmlc::demangle(typeid(std::vector<double>).name()), DataType::HELICS_VECTOR},
    {gmlc::demangle(typeid(std::vector<std::complex<double>>).name()),
     DataType::HELICS_COMPLEX_VECTOR},
    {typeid(std::vector<double>).name(), DataType::HELICS_VECTOR},
    {typeid(double*).name(), DataType::HELICS_VECTOR},
    {typeid(double).name(), DataType::HELICS_DOUBLE},
    {typeid(float).name(), DataType::HELICS_DOUBLE},
    {typeid(char).name(), DataType::HELICS_CHAR},
    {typeid(unsigned char).name(), DataType::HELICS_INT},
    {typeid(short).name(), DataType::HELICS_INT},  // NOLINT
    {typeid(unsigned short).name(), DataType::HELICS_INT},  // NOLINT
    {typeid(int).name(), DataType::HELICS_INT},
    {typeid(unsigned int).name(), DataType::HELICS_INT},
    {typeid(long).name(), DataType::HELICS_INT},  // NOLINT
    {typeid(unsigned long).name(), DataType::HELICS_INT},  // NOLINT
    {typeid(long long).name(), DataType::HELICS_INT},  // NOLINT
    {typeid(unsigned long long).name(), DataType::HELICS_INT},  // NOLINT
    {typeid(int64_t).name(), DataType::HELICS_INT},
    {typeid(uint64_t).name(), DataType::HELICS_INT},
    {typeid(int32_t).name(), DataType::HELICS_INT},
    {typeid(uint32_t).name(), DataType::HELICS_INT},
    {typeid(int16_t).name(), DataType::HELICS_INT},
    {typeid(uint16_t).name(), DataType::HELICS_INT},
    {typeid(int8_t).name(), DataType::HELICS_INT},
    {typeid(uint8_t).name(), DataType::HELICS_INT},
    {typeid(bool).name(), DataType::HELICS_BOOL},
    {typeid(std::vector<std::complex<double>>).name(), DataType::HELICS_COMPLEX_VECTOR},
    {typeid(std::complex<double>).name(), DataType::HELICS_COMPLEX},
    {typeid(std::string).name(), DataType::HELICS_STRING},
    {typeid(char*).name(), DataType::HELICS_STRING},
    {typeid(const char*).name(), DataType::HELICS_STRING},
    {typeid(NamedPoint).name(), DataType::HELICS_STRING},
    {typeid(Time).name(), DataType::HELICS_TIME}};

DataType getTypeFromString(std::string_view typeName)
{
    if (!typeName.empty() && typeName.front() == '[') {
        return DataType::HELICS_MULTI;
    }
    const auto* res = typeMap.find(typeName);
    if (res != typeMap.end()) {
        return res->second;
    }
    std::string strName(typeName);
    auto dres = demangle_names.find(strName);
    if (dres != demangle_names.end()) {
        return dres->second;
    }
    makeLowerCase(strName);
    res = typeMap.find(strName);
    if (res != typeMap.end()) {
        return res->second;
    }
    dres = demangle_names.find(strName);
    if (dres != demangle_names.end()) {
        return dres->second;
    }
    return DataType::HELICS_CUSTOM;
}

std::string_view getCleanedTypeName(std::string_view typeName)
{
    if (!typeName.empty() && typeName.front() == '[') {
        return typeName;
    }
    const auto* res = typeMap.find(typeName);
    if (res != typeMap.end()) {
        return typeName;
    }
    std::string strName(typeName);
    auto dres = demangle_names.find(strName);
    if (dres != demangle_names.end()) {
        return typeNameStringRef(dres->second);
    }
    makeLowerCase(strName);
    res = typeMap.find(strName);
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
    double real{invalidValue<double>()};
    double imag{0.0};
    if (val.front() == '[') {
        auto sep = val.find_first_of(',');
        if (sep == std::string_view::npos) {
            val.remove_prefix(1);
            val.remove_suffix(1);
            real = numConv<double>(val);
            return {real, imag};
        }
        if (val.find_first_of(',', sep + 1) != std::string_view::npos) {
            auto vectorVal = helicsGetVector(val);
            if (vectorVal.size() >= 2) {
                return {vectorVal[0], vectorVal[1]};
            }
            return invalidValue<std::complex<double>>();
        }
        real = numConv<double>(val.substr(1, sep));
        val.remove_suffix(1);
        imag = numConv<double>(val.substr(sep + 1));
        return {real, imag};
    }
    std::smatch match;

    auto temp = std::string(val);
    std::regex_search(temp, match, creg);
    try {
        if (match.size() == 9) {
            real = numConv<double>(match[1]);

            imag = numConv<double>(match[6]);

            if (*match[5].first == '-') {
                imag = -imag;
            }
        } else {
            if ((val.back() == 'j') || (val.back() == 'i')) {
                auto strval = val.substr(0, val.size() - 1);
                string_viewOps::trimString(strval);
                imag = numConv<double>(strval);
                real = 0.0;
            } else {
                auto strval = val;
                string_viewOps::trimString(strval);
                real = numConv<double>(strval);
            }
        }
    }
    catch (const std::invalid_argument&) {
        real = invalidValue<double>();
    }
    return {real, imag};
}

std::string helicsIntString(std::int64_t val)
{
    return fmt::format("{}", val);
}

std::string helicsDoubleString(double val)
{
    return fmt::format("{}", val);
}

std::string helicsVectorString(const std::vector<double>& val)
{
    return fmt::format("[{}]", fmt::join(val, ","));
}

std::string helicsVectorString(const double* vals, size_t size)
{
    return fmt::format("[{}]", fmt::join(vals, vals + size, ","));
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
    nlohmann::json namePoint;
    namePoint["value"] = val;
    if (pointName.empty()) {
    } else {
        namePoint["name"] = pointName;
    }
    return fileops::generateJsonString(namePoint);
}

std::vector<double> helicsGetVector(std::string_view val)
{
    std::vector<double> vectorVal;
    helicsGetVector(val, vectorVal);
    return vectorVal;
}

std::vector<std::complex<double>> helicsGetComplexVector(std::string_view val)
{
    std::vector<std::complex<double>> vectorVal;
    helicsGetComplexVector(val, vectorVal);
    return vectorVal;
}

NamedPoint helicsGetNamedPoint(std::string_view val)
{
    NamedPoint namePoint;
    try {
        auto json = fileops::loadJsonStr(val);
        switch (json.type()) {
            case nlohmann::json::value_t::number_float:
                namePoint.value = json.get<double>();
                namePoint.name = "value";
                break;
            case nlohmann::json::value_t::string:
                namePoint.name = json.get<std::string>();
                break;
            case nlohmann::json::value_t::array:
                break;
            case nlohmann::json::value_t::number_integer:
            case nlohmann::json::value_t::number_unsigned:
                namePoint.value = static_cast<double>(json.get<int>());
                namePoint.name = "value";
                break;
            case nlohmann::json::value_t::object:
                fileops::replaceIfMember(json, "value", namePoint.value);
                fileops::replaceIfMember(json, "name", namePoint.name);
                break;
            default:
                break;
        }
    }
    catch (...) {
        namePoint.name = val;
    }
    return namePoint;
}

static int readSize(std::string_view val)
{
    auto firstBracket = val.find_first_of('[');
    if (firstBracket > 1) {
        try {
            auto size = numConv<int>(val.substr(1, firstBracket - 1));
            return size;
        }
        // NOLINTNEXTLINE
        catch (const std::invalid_argument&) {
            // go to the alternative path if this fails
        }
    }
    if (val.find_first_not_of(" ]", firstBracket + 1) == std::string_view::npos) {
        return 0;
    }
    auto res = std::count_if(val.begin() + firstBracket,
                             val.end(),
                             [](auto nextChar) { return (nextChar == ',') || (nextChar == ';'); }) +
        1;
    return static_cast<int>(res);
}

std::complex<double> getComplexFromString(std::string_view val)
{
    if (val.empty()) {
        return invalidValue<std::complex<double>>();
    }
    if ((val.front() == 'v') || (val.front() == 'c') || val.front() == '[') {
        auto vectorVal = helicsGetVector(val);
        if (vectorVal.empty()) {
            return invalidValue<std::complex<double>>();
        }
        if (vectorVal.size() == 1) {
            return {vectorVal[0], 0.0};
        }
        return {vectorVal[0], vectorVal[1]};
    }
    return helicsGetComplex(val);
}

std::int64_t getIntFromString(std::string_view val)
{
    static constexpr std::int64_t conversionFailValue{invalidValue<std::int64_t>() + 3};

    auto ival = numeric_conversionComplete<std::int64_t>(val, conversionFailValue);
    if (ival == conversionFailValue) {
        return static_cast<int64_t>(getDoubleFromString(val));
    }
    return ival;
}

double getDoubleFromString(std::string_view val)
{
    if (val.empty()) {
        return invalidValue<double>();
    }
    if (val.front() == 'v' || val.front() == '[') {
        auto vectorVal = helicsGetVector(val);
        return (vectorVal.size() != 1) ? vectorNorm(vectorVal) : vectorVal[0];
    }
    if (val.front() == 'c') {
        auto complexVal = helicsGetComplexVector(val);
        return (complexVal.size() != 1) ?
            vectorNorm(complexVal) :
            ((complexVal[0].imag() == 0.0) ? complexVal[0].real() : std::abs(complexVal[0]));
    }
    auto cval = helicsGetComplex(val);
    return (cval.imag() == 0.0) ? cval.real() : std::abs(cval);
}

void helicsGetVector(std::string_view val, std::vector<double>& data)
{
    if (val.empty()) {
        data.resize(0);
        return;
    }
    if (val.front() == 'v' || val.front() == '[') {
        auto size = readSize(val);
        if (size > 0) {
            data.reserve(size);
        }
        data.resize(0);
        auto firstBracket = val.find_first_of('[');
        for (decltype(size) ii = 0; ii < size; ++ii) {
            auto nextChar = val.find_first_of(";,]", firstBracket + 1);

            auto vstr = val.substr(firstBracket + 1, nextChar - firstBracket - 1);
            string_viewOps::trimString(vstr);
            auto vectorVal = numeric_conversion<double>(vstr, invalidValue<double>());
            data.push_back(vectorVal);

            firstBracket = nextChar;
        }
    } else if (val.front() == 'c') {
        auto size = readSize(val);
        data.reserve(static_cast<std::size_t>(size) * 2);
        data.resize(0);
        auto firstBracket = val.find_first_of('[');
        for (decltype(size) ii = 0; ii < size; ++ii) {
            auto nextChar = val.find_first_of(",;]", firstBracket + 1);
            auto vectorVal =
                helicsGetComplex(val.substr(firstBracket + 1, nextChar - firstBracket - 1));
            data.push_back(vectorVal.real());
            data.push_back(vectorVal.imag());
            firstBracket = nextChar;
        }
    } else {
        auto cval = helicsGetComplex(val);
        if (cval.imag() == 0) {
            data.resize(1);
            data[0] = cval.real();
        } else {
            data.resize(2);
            data[0] = cval.real();
            data[1] = cval.imag();
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
        auto size = readSize(val);
        data.reserve(size / 2);
        data.resize(0);
        auto firstBracket = val.find_first_of('[');
        for (decltype(size) ii = 0; ii < size - 1; ii += 2) {
            auto nextChar = val.find_first_of(",;]", firstBracket + 1);
            auto secondChar = val.find_first_of(",;]", nextChar + 1);
            try {
                auto vstr1 = val.substr(firstBracket + 1, nextChar - firstBracket - 1);
                string_viewOps::trimString(vstr1);
                auto vstr2 = val.substr(nextChar + 1, secondChar - nextChar - 1);
                string_viewOps::trimString(vstr2);
                auto val1 = numConv<double>(vstr1);
                auto val2 = numConv<double>(vstr2);
                data.emplace_back(val1, val2);
            }
            catch (const std::invalid_argument&) {
                data.push_back(invalidValue<std::complex<double>>());
            }
            firstBracket = nextChar;
        }
    } else if (val.front() == 'c') {
        auto size = readSize(val);
        data.reserve(size);
        data.resize(0);
        auto firstBracket = val.find_first_of('[');
        for (decltype(size) ii = 0; ii < size; ++ii) {
            auto nextChar = val.find_first_of(",;]", firstBracket + 1);
            auto cval = helicsGetComplex(val.substr(firstBracket + 1, nextChar - firstBracket - 1));
            data.push_back(cval);
            firstBracket = nextChar;
        }
    } else {
        if (val.find_first_of("ji") != std::string_view::npos) {
            auto cval = helicsGetComplex(val);
            data.resize(0);
            data.push_back(cval);
        } else {
            auto json = fileops::loadJsonStr(val);
            int cnt{0};
            switch (json.type()) {
                case nlohmann::json::value_t::number_float:
                case nlohmann::json::value_t::number_integer:
                case nlohmann::json::value_t::number_unsigned:
                    data.resize(0);
                    data.emplace_back(json.get<double>(), 0.0);
                    break;
                case nlohmann::json::value_t::array:
                    for (auto& aval : json) {
                        if (aval.is_number()) {
                            if (cnt == 0) {
                                data.emplace_back(aval.get<double>(), 0.0);
                                cnt = 1;
                            } else {
                                data.back() += std::complex<double>{0.0, aval.get<double>()};
                                cnt = 0;
                            }
                        } else if (aval.is_array()) {
                            cnt = 0;
                            if (aval.size() >= 2) {
                                if (aval[0].is_number()) {
                                    data.emplace_back(aval[0].get<double>(), aval[1].get<double>());
                                }
                            } else if (aval.size() == 1) {
                                if (aval[0].is_number()) {
                                    data.emplace_back(aval[0].get<double>(), 0.0);
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
    static constexpr const frozen::unordered_map<std::string_view, bool, 37> knownStrings{

        {"0", false},
        {"00", false},
        {std::string_view("\0", 1), false},
        {"0000", false},
        {std::string_view("\0\0\0\0\0\0\0\0", 8), false},
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
        {"-", false},
        {"t", true},
        {"+", true},
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
    const auto* res = knownStrings.find(val);
    if (res != knownStrings.end()) {
        return res->second;
    }
    return true;
}

SmallBuffer emptyBlock(DataType outputType, DataType inputType = DataType::HELICS_ANY)
{
    switch (outputType) {
        case DataType::HELICS_DOUBLE:
        default:
            return ValueConverter<double>::convert(0.0);
        case DataType::HELICS_INT:
        case DataType::HELICS_TIME:
            return ValueConverter<int64_t>::convert(0);
        case DataType::HELICS_COMPLEX:
            return ValueConverter<std::complex<double>>::convert(std::complex<double>(0.0, 0.0));
        case DataType::HELICS_BOOL:
            return ValueConverter<std::string_view>::convert("0");
        case DataType::HELICS_NAMED_POINT:
            return ValueConverter<NamedPoint>::convert(NamedPoint{"", std::nan("0")});
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
            switch (inputType) {
                default:
                    return ValueConverter<std::string_view>::convert("");
                case DataType::HELICS_VECTOR:
                    return ValueConverter<std::string_view>::convert(
                        helicsVectorString(std::vector<double>()));
                case DataType::HELICS_COMPLEX_VECTOR:
                    return ValueConverter<std::string_view>::convert(
                        helicsComplexVectorString(std::vector<std::complex<double>>()));
                case DataType::HELICS_NAMED_POINT:
                    return ValueConverter<std::string_view>::convert("0");
            }
        case DataType::HELICS_COMPLEX_VECTOR: {
            return ValueConverter<std::vector<std::complex<double>>>::convert(
                std::vector<std::complex<double>>());
        }
        case DataType::HELICS_VECTOR:
            return ValueConverter<std::vector<double>>::convert(std::vector<double>());
    }
}
SmallBuffer typeConvert(DataType type, double val)
{
    switch (type) {
        case DataType::HELICS_DOUBLE:
        default:
            return ValueConverter<double>::convert(val);
        case DataType::HELICS_INT:
            return ValueConverter<int64_t>::convert(static_cast<int64_t>(val));
        case DataType::HELICS_TIME:
            return ValueConverter<int64_t>::convert(Time(val).getBaseTimeCode());
        case DataType::HELICS_COMPLEX:
            return ValueConverter<std::complex<double>>::convert(std::complex<double>(val, 0.0));
        case DataType::HELICS_BOOL:
            return ValueConverter<std::string_view>::convert((val != 0.0) ? "1" : "0");
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
            return ValueConverter<std::string_view>::convert(helicsDoubleString(val));
        case DataType::HELICS_NAMED_POINT:
            return ValueConverter<NamedPoint>::convert(NamedPoint{"value", val});
        case DataType::HELICS_COMPLEX_VECTOR: {
            const std::complex<double> cvec(val, 0.0);
            return ValueConverter<std::complex<double>>::convert(&cvec, 1);
        }
        case DataType::HELICS_VECTOR:
            return ValueConverter<double>::convert(&val, 1);
        case DataType::HELICS_JSON: {
            nlohmann::json json;
            json["type"] = typeNameStringRef(DataType::HELICS_DOUBLE);
            json["value"] = val;
            return fileops::generateJsonString(json);
        }
    }
}
SmallBuffer typeConvert(DataType type, int64_t val)
{
    switch (type) {
        case DataType::HELICS_DOUBLE:
            return ValueConverter<double>::convert(static_cast<double>(val));
        case DataType::HELICS_INT:
        case DataType::HELICS_TIME:
        default:
            return ValueConverter<int64_t>::convert(val);
        case DataType::HELICS_COMPLEX:
            return ValueConverter<std::complex<double>>::convert(
                std::complex<double>(static_cast<double>(val), 0.0));
        case DataType::HELICS_BOOL:
            return ValueConverter<std::string_view>::convert((val != 0) ? "1" : "0");
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
            return ValueConverter<std::string_view>::convert(helicsIntString(val));
        case DataType::HELICS_NAMED_POINT:
            if (static_cast<uint64_t>(std::abs(val)) >
                (2ULL << 51U))  // this checks whether the actual value will fit in a double
            {
                return ValueConverter<NamedPoint>::convert(
                    NamedPoint{helicsIntString(val), std::nan("0")});
            } else {
                return ValueConverter<NamedPoint>::convert(
                    NamedPoint{"value", static_cast<double>(val)});
            }

        case DataType::HELICS_COMPLEX_VECTOR: {
            const std::complex<double> cvec(static_cast<double>(val), 0.0);
            return ValueConverter<std::complex<double>>::convert(&cvec, 1);
        }
        case DataType::HELICS_VECTOR: {
            const auto doubleVal = static_cast<double>(val);
            return ValueConverter<double>::convert(&doubleVal, 1);
        }
        case DataType::HELICS_JSON: {
            nlohmann::json json;
            json["type"] = typeNameStringRef(DataType::HELICS_INT);
            json["value"] = val;
            return fileops::generateJsonString(json);
        }
    }
}

SmallBuffer typeConvert(DataType type, std::string_view val)
{
    if (val.empty()) {
        return emptyBlock(type);
    }

    switch (type) {
        case DataType::HELICS_DOUBLE:
            return ValueConverter<double>::convert(getDoubleFromString(val));
        case DataType::HELICS_INT:
            return ValueConverter<int64_t>::convert(static_cast<int64_t>(getDoubleFromString(val)));
        case DataType::HELICS_TIME:
            return ValueConverter<int64_t>::convert(
                Time(getDoubleFromString(val)).getBaseTimeCode());
        case DataType::HELICS_COMPLEX:
            return ValueConverter<std::complex<double>>::convert(helicsGetComplex(val));
        case DataType::HELICS_BOOL:
            return ValueConverter<std::string_view>::convert((helicsBoolValue(val)) ? "1" : "0");
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
        default:
            return ValueConverter<std::string_view>::convert(val);
        case DataType::HELICS_NAMED_POINT:
            return ValueConverter<NamedPoint>::convert(NamedPoint{val, std::nan("0")});
        case DataType::HELICS_COMPLEX_VECTOR:
            return ValueConverter<std::vector<std::complex<double>>>::convert(
                helicsGetComplexVector(val));
        case DataType::HELICS_VECTOR:
            return ValueConverter<std::vector<double>>::convert(helicsGetVector(val));
        case DataType::HELICS_JSON: {
            nlohmann::json json;
            json["type"] = typeNameStringRef(DataType::HELICS_STRING);
            json["value"] = std::string(val);
            return fileops::generateJsonString(json);
        }
    }
}

SmallBuffer typeConvert(DataType type, const std::vector<double>& val)
{
    return typeConvert(type, val.data(), val.size());
}

SmallBuffer typeConvert(DataType type, const double* vals, size_t size)
{
    if ((vals == nullptr) || (size == 0)) {
        return emptyBlock(type, DataType::HELICS_VECTOR);
    }
    if (size == 1) {
        // treat like a single double
        return typeConvert(type, vals[0]);
    }
    switch (type) {
        case DataType::HELICS_DOUBLE:
            return ValueConverter<double>::convert(vectorNorm(vals, size));
        case DataType::HELICS_TIME:
            return ValueConverter<int64_t>::convert(Time(vectorNorm(vals, size)).getBaseTimeCode());
        case DataType::HELICS_INT:
            return ValueConverter<int64_t>::convert(static_cast<int64_t>(vectorNorm(vals, size)));
        case DataType::HELICS_COMPLEX: {
            const std::complex<double> cval(vals[0], vals[1]);
            return ValueConverter<std::complex<double>>::convert(cval);
        }
        case DataType::HELICS_BOOL:
            return ValueConverter<std::string_view>::convert((vectorNorm(vals, size) != 0.0) ? "1" :
                                                                                               "0");
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
            return ValueConverter<std::string_view>::convert(helicsVectorString(vals, size));
        case DataType::HELICS_NAMED_POINT:
            return ValueConverter<NamedPoint>::convert(
                NamedPoint{helicsVectorString(vals, size), std::nan("0")});
        case DataType::HELICS_COMPLEX_VECTOR: {
            std::vector<std::complex<double>> cvec;
            cvec.reserve(size);
            for (size_t ii = 0; ii < size; ++ii) {
                cvec.emplace_back(vals[ii], 0.0);
            }
            return ValueConverter<std::vector<std::complex<double>>>::convert(cvec);
        } break;
        case DataType::HELICS_VECTOR:
        default:
            return ValueConverter<double>::convert(vals, size);
        case DataType::HELICS_JSON: {
            nlohmann::json json;
            json["type"] = typeNameStringRef(DataType::HELICS_VECTOR);
            nlohmann::json jsonArray = nlohmann::json::array();
            for (size_t ii = 0; ii < size; ++ii) {
                jsonArray.push_back(vals[ii]);
            }
            json["value"] = std::move(jsonArray);
            return fileops::generateJsonString(json);
        }
    }
}

SmallBuffer typeConvertComplex(DataType type, const double* vals, size_t size)
{
    if ((vals == nullptr) || (size == 0)) {
        return emptyBlock(type, DataType::HELICS_COMPLEX_VECTOR);
    }
    if (size == 1) {
        // treat like a single complex
        return typeConvert(type, std::complex<double>{vals[0], vals[1]});
    }
    switch (type) {
        case DataType::HELICS_DOUBLE:
            return ValueConverter<double>::convert(vectorNorm(vals, size));
        case DataType::HELICS_TIME:
            return ValueConverter<int64_t>::convert(Time(vectorNorm(vals, size)).getBaseTimeCode());
        case DataType::HELICS_INT:
            return ValueConverter<int64_t>::convert(static_cast<int64_t>(vectorNorm(vals, size)));
        case DataType::HELICS_COMPLEX: {
            const std::complex<double> cval(vals[0], vals[1]);
            return ValueConverter<std::complex<double>>::convert(cval);
        }
        case DataType::HELICS_BOOL:
            return ValueConverter<std::string_view>::convert((vectorNorm(vals, size) != 0.0) ? "1" :
                                                                                               "0");
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR: {
            std::vector<std::complex<double>> cvec;
            cvec.reserve(size);
            for (size_t ii = 0; ii < size; ++ii) {
                cvec.emplace_back(vals[2 * ii], vals[2 * ii + 1]);
            }
            return ValueConverter<std::string_view>::convert(helicsComplexVectorString(cvec));
        }
        case DataType::HELICS_NAMED_POINT:
            return ValueConverter<NamedPoint>::convert(
                NamedPoint{helicsVectorString(vals, size), std::nan("0")});
        case DataType::HELICS_COMPLEX_VECTOR: {
            std::vector<std::complex<double>> cvec;
            cvec.reserve(size);
            for (size_t ii = 0; ii < size; ++ii) {
                cvec.emplace_back(vals[2 * ii], vals[2 * ii + 1]);
            }
            return ValueConverter<std::vector<std::complex<double>>>::convert(cvec);
        } break;
        case DataType::HELICS_VECTOR:
        default:
            return ValueConverter<double>::convert(vals, size);
        case DataType::HELICS_JSON: {
            nlohmann::json json;
            json["type"] = typeNameStringRef(DataType::HELICS_VECTOR);
            nlohmann::json jsonArray = nlohmann::json::array();
            for (size_t ii = 0; ii < size; ++ii) {
                jsonArray.push_back(vals[ii]);
            }
            json["value"] = std::move(jsonArray);
            return fileops::generateJsonString(json);
        }
    }
}

SmallBuffer typeConvert(DataType type, const std::vector<std::complex<double>>& val)
{
    if (val.empty()) {
        return emptyBlock(type, DataType::HELICS_COMPLEX_VECTOR);
    }
    if (val.size() == 1) {
        return typeConvert(type, val[0]);
    }
    switch (type) {
        case DataType::HELICS_DOUBLE:
            return ValueConverter<double>::convert(vectorNorm(val));
        case DataType::HELICS_TIME:
            return ValueConverter<int64_t>::convert(Time(vectorNorm(val)).getBaseTimeCode());
        case DataType::HELICS_INT:
            return ValueConverter<int64_t>::convert(
                static_cast<int64_t>(vectorNorm(val)));  // NOLINT
        case DataType::HELICS_COMPLEX:
            return ValueConverter<std::complex<double>>::convert(val[0]);
        case DataType::HELICS_BOOL:
            return ValueConverter<std::string_view>::convert((vectorNorm(val) != 0.0) ? "1" : "0");
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
            return ValueConverter<std::string_view>::convert(helicsComplexVectorString(val));
        case DataType::HELICS_NAMED_POINT:
            return ValueConverter<NamedPoint>::convert(
                NamedPoint{helicsComplexVectorString(val), std::nan("0")});
        case DataType::HELICS_COMPLEX_VECTOR:
        default:
            return ValueConverter<std::vector<std::complex<double>>>::convert(val);
        case DataType::HELICS_VECTOR: {
            std::vector<double> vectorVal;
            vectorVal.reserve(val.size() * 2);
            for (const auto& vali : val) {
                vectorVal.push_back(vali.real());
                vectorVal.push_back(vali.imag());
            }
            return ValueConverter<std::vector<double>>::convert(vectorVal);
        }
        case DataType::HELICS_JSON: {
            nlohmann::json json;
            json["type"] = typeNameStringRef(DataType::HELICS_COMPLEX_VECTOR);
            nlohmann::json jsonArray = nlohmann::json::array();
            for (const auto& element : val) {
                jsonArray.push_back(element.real());
                jsonArray.push_back(element.imag());
            }
            json["value"] = std::move(jsonArray);
            return fileops::generateJsonString(json);
        }
    }
}

SmallBuffer typeConvert(DataType type, const std::complex<double>& val)
{
    if (val.imag() == 0.0) {
        return typeConvert(type, val.real());
    }
    switch (type) {
        case DataType::HELICS_DOUBLE:
            return ValueConverter<double>::convert(std::abs(val));
        case DataType::HELICS_INT:
            return ValueConverter<int64_t>::convert(static_cast<int64_t>(std::abs(val)));
        case DataType::HELICS_TIME:
            return ValueConverter<int64_t>::convert(Time(std::abs(val)).getBaseTimeCode());
        case DataType::HELICS_COMPLEX:
        default:
            return ValueConverter<std::complex<double>>::convert(val);
        case DataType::HELICS_BOOL:
            return ValueConverter<std::string_view>::convert((std::abs(val) != 0.0) ? "1" : "0");
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
            return ValueConverter<std::string_view>::convert(helicsComplexString(val));
        case DataType::HELICS_NAMED_POINT:
            return ValueConverter<NamedPoint>::convert(
                NamedPoint{helicsComplexString(val), std::nan("0")});
        case DataType::HELICS_COMPLEX_VECTOR:
            return ValueConverter<std::complex<double>>::convert(&val, 1);
        case DataType::HELICS_VECTOR: {
            const std::vector<double> vectorVal{val.real(), val.imag()};
            return ValueConverter<std::vector<double>>::convert(vectorVal);
        }
        case DataType::HELICS_JSON: {
            nlohmann::json json;
            json["type"] = typeNameStringRef(DataType::HELICS_COMPLEX);
            nlohmann::json jsonArray = nlohmann::json::array();
            jsonArray.push_back(val.real());
            jsonArray.push_back(val.imag());
            json["value"] = std::move(jsonArray);
            return fileops::generateJsonString(json);
        }
    }
}

SmallBuffer typeConvert(DataType type, const NamedPoint& val)
{
    if (type == DataType::HELICS_NAMED_POINT) {
        return ValueConverter<NamedPoint>::convert(val);
    }
    if (std::isnan(val.value)) {
        // just convert the string
        return typeConvert(type, val.name);
    }
    switch (type) {
        case DataType::HELICS_DOUBLE:
            return ValueConverter<double>::convert(val.value);
        case DataType::HELICS_INT:
            return ValueConverter<int64_t>::convert(static_cast<int64_t>(val.value));
        case DataType::HELICS_TIME:
            return ValueConverter<int64_t>::convert(Time(val.value).getBaseTimeCode());
        case DataType::HELICS_COMPLEX:
            return ValueConverter<std::complex<double>>::convert(
                std::complex<double>(val.value, 0.0));
        case DataType::HELICS_BOOL:
            return ValueConverter<std::string_view>::convert((val.value != 0) ? "1" : "0");
        case DataType::HELICS_NAMED_POINT:
        default:
            return ValueConverter<NamedPoint>::convert(val);
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
            return ValueConverter<std::string_view>::convert(
                (std::isnan(val.value)) ? val.name : helicsNamedPointString(val));
        case DataType::HELICS_COMPLEX_VECTOR: {
            const std::complex<double> cval(val.value, 0.0);
            return ValueConverter<std::complex<double>>::convert(&cval, 1);
        }
        case DataType::HELICS_VECTOR:
            return ValueConverter<double>::convert(&(val.value), 1);
        case DataType::HELICS_JSON: {
            nlohmann::json json;
            json["type"] = typeNameStringRef(DataType::HELICS_NAMED_POINT);
            json["name"] = val.name;
            json["value"] = val.value;
            return fileops::generateJsonString(json);
        }
    }
}

SmallBuffer typeConvert(DataType type, std::string_view str, double val)
{
    if (type == DataType::HELICS_NAMED_POINT) {
        return ValueConverter<NamedPoint>::convert(NamedPoint(str, val));
    }
    if (std::isnan(val)) {
        // just convert the string
        return typeConvert(type, str);
    }
    switch (type) {
        case DataType::HELICS_DOUBLE:
            return ValueConverter<double>::convert(val);
        case DataType::HELICS_INT:
        case DataType::HELICS_TIME:
            return ValueConverter<int64_t>::convert(static_cast<int64_t>(val));
        case DataType::HELICS_COMPLEX:
            return ValueConverter<std::complex<double>>::convert(std::complex<double>(val, 0.0));
        case DataType::HELICS_BOOL:
            return ValueConverter<std::string_view>::convert((val != 0.0) ? "1" : "0");
        case DataType::HELICS_NAMED_POINT:
        default:
            return ValueConverter<NamedPoint>::convert(NamedPoint(str, val));
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
            return ValueConverter<std::string_view>::convert(helicsNamedPointString(str, val));
        case DataType::HELICS_COMPLEX_VECTOR: {
            const std::complex<double> cval(val, 0.0);
            return ValueConverter<std::complex<double>>::convert(&cval, 1);
        }
        case DataType::HELICS_VECTOR:
            return ValueConverter<double>::convert(&(val), 1);
        case DataType::HELICS_JSON: {
            nlohmann::json json;
            json["type"] = typeNameStringRef(DataType::HELICS_NAMED_POINT);
            json["name"] = std::string(str);
            json["value"] = val;
            return fileops::generateJsonString(json);
        }
    }
}

SmallBuffer typeConvert(DataType type, bool val)
{
    switch (type) {
        case DataType::HELICS_DOUBLE:
            return ValueConverter<double>::convert(val ? 1.0 : 0.0);
        case DataType::HELICS_INT:
        case DataType::HELICS_TIME:
            return ValueConverter<int64_t>::convert(val ? 1 : 0);
        case DataType::HELICS_COMPLEX:
            return ValueConverter<std::complex<double>>::convert(
                std::complex<double>(val ? 1.0 : 0.0, 0.0));
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
        case DataType::HELICS_BOOL:
        default:
            return ValueConverter<std::string_view>::convert(val ? "1" : "0");
        case DataType::HELICS_NAMED_POINT: {
            const NamedPoint namePoint{"value", val ? 1.0 : 0.0};
            return ValueConverter<NamedPoint>::convert(namePoint);
        }
        case DataType::HELICS_COMPLEX_VECTOR: {
            const std::complex<double> cval(val ? 1.0 : 0.0, 0.0);
            return ValueConverter<std::complex<double>>::convert(&cval, 1);
        }
        case DataType::HELICS_VECTOR: {
            auto vec = val ? 1.0 : 0.0;
            return ValueConverter<double>::convert(&vec, 1);
        }
        case DataType::HELICS_JSON: {
            nlohmann::json json;
            json["type"] = typeNameStringRef(DataType::HELICS_BOOL);
            json["value"] = val;
            return fileops::generateJsonString(json);
        }
    }
}

SmallBuffer typeConvert(DataType type, char val)
{
    switch (type) {
        case DataType::HELICS_DOUBLE:
            return ValueConverter<double>::convert(static_cast<double>(val));
        case DataType::HELICS_INT:
        case DataType::HELICS_TIME:
            return ValueConverter<int64_t>::convert(static_cast<std::int64_t>(val));
        case DataType::HELICS_COMPLEX:
            return ValueConverter<std::complex<double>>::convert(
                std::complex<double>(static_cast<double>(val), 0.0));
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
        case DataType::HELICS_BOOL:
        default:
            return ValueConverter<std::string_view>::convert(std::string_view(&val, 1));
        case DataType::HELICS_NAMED_POINT: {
            const NamedPoint namePoint{"value", static_cast<double>(val)};
            return ValueConverter<NamedPoint>::convert(namePoint);
        }
        case DataType::HELICS_COMPLEX_VECTOR: {
            const std::complex<double> cvec(static_cast<double>(val), 0.0);
            return ValueConverter<std::complex<double>>::convert(&cvec, 1);
        }
        case DataType::HELICS_VECTOR: {
            const auto doubleVal = static_cast<double>(val);
            return ValueConverter<double>::convert(&doubleVal, 1);
        }
        case DataType::HELICS_JSON: {
            nlohmann::json json;
            json["type"] = typeNameStringRef(DataType::HELICS_INT);
            json["value"] = val;
            return fileops::generateJsonString(json);
        }
    }
}

SmallBuffer typeConvert(DataType type, Time val)
{
    switch (type) {
        case DataType::HELICS_DOUBLE:
            return ValueConverter<double>::convert(static_cast<double>(val));
        case DataType::HELICS_INT:
        case DataType::HELICS_TIME:
            return ValueConverter<int64_t>::convert(val.getBaseTimeCode());
        case DataType::HELICS_COMPLEX:
        default:
            return ValueConverter<std::complex<double>>::convert(
                std::complex<double>(static_cast<double>(val), 0.0));
        case DataType::HELICS_BOOL:
            return ValueConverter<std::string_view>::convert((val != timeZero) ? "1" : "0");
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
            return ValueConverter<std::string_view>::convert(
                helicsIntString(val.getBaseTimeCode()));
        case DataType::HELICS_NAMED_POINT:
            if (static_cast<uint64_t>(val.getBaseTimeCode()) >
                (2ULL << 51U))  // this checks whether the actual value will fit in a double
            {
                return ValueConverter<NamedPoint>::convert(
                    NamedPoint{helicsIntString(val.getBaseTimeCode()), std::nan("0")});
            } else {
                return ValueConverter<NamedPoint>::convert(
                    NamedPoint{"value", static_cast<double>(val)});
            }
        case DataType::HELICS_COMPLEX_VECTOR: {
            std::vector<std::complex<double>> cval;
            cval.emplace_back(static_cast<double>(val), 0.0);
            return ValueConverter<std::vector<std::complex<double>>>::convert(cval);
        }
        case DataType::HELICS_VECTOR: {
            const std::vector<double> vec{static_cast<double>(val)};
            return ValueConverter<std::vector<double>>::convert(vec);
        }
        case DataType::HELICS_JSON: {
            nlohmann::json json;
            json["type"] = typeNameStringRef(DataType::HELICS_TIME);
            json["value"] = val.getBaseTimeCode();
            return fileops::generateJsonString(json);
        }
    }
}

}  // namespace helics
