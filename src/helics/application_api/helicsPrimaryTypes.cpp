/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "HelicsPrimaryTypes.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "../utilities/timeStringOps.hpp"
#include "ValueConverter.hpp"

#include <set>
#include <string>
#include <utility>
#include <vector>

namespace helics {
bool changeDetected(const defV& prevValue, const std::string& val, double /*deltaV*/)
{
    if (prevValue.index() == string_loc) {
        return (val != std::get<std::string>(prevValue));
    }
    return true;
}

bool changeDetected(const defV& prevValue, std::string_view val, double /*deltaV*/)
{
    if (prevValue.index() == string_loc) {
        return (val != std::get<std::string>(prevValue));
    }
    return true;
}

static constexpr int nullstringRep{0};
static const std::set<std::string_view> falseString{
    "0",        "",         "false",
    "False",    "FALSE",    "off",
    "Off",      "OFF",      "disabled",
    "Disabled", "DISABLED", "disable",
    "Disable",  "DISABLE",  "f",
    "F",        "0",        std::string_view(reinterpret_cast<const char*>(&nullstringRep), 1),
    " ",        "no",       "NO",
    "No",       "-"};

bool isTrueString(const std::string_view str)
{
    if (str.size() == 1 && str[0] == '1') {
        return true;
    }
    if (str.size() == 1 && str[0] == '0') {
        return false;
    }
    return (falseString.find(str) == falseString.end());
}

bool changeDetected(const defV& prevValue, bool val, double /*deltaV*/)
{
    if (prevValue.index() == string_loc) {
        return (isTrueString(std::get<std::string>(prevValue)) != val);
    }
    if (prevValue.index() == int_loc) {
        return ((std::get<int64_t>(prevValue) != 0) != val);
    }
    return true;
}

bool changeDetected(const defV& prevValue, const std::vector<double>& val, double deltaV)
{
    if (prevValue.index() == vector_loc) {
        const auto& prevV = std::get<std::vector<double>>(prevValue);
        if (val.size() == prevV.size()) {
            for (size_t ii = 0; ii < val.size(); ++ii) {
                if (std::abs(prevV[ii] - val[ii]) > deltaV) {
                    return true;
                }
            }
            return false;
        }
    }
    return true;
}

bool changeDetected(const defV& prevValue,
                    const std::vector<std::complex<double>>& val,
                    double deltaV)
{
    if (prevValue.index() == complex_vector_loc) {
        const auto& prevV = std::get<std::vector<std::complex<double>>>(prevValue);
        if (val.size() == prevV.size()) {
            for (size_t ii = 0; ii < val.size(); ++ii) {
                if (std::abs(prevV[ii] - val[ii]) > deltaV) {
                    return true;
                }
            }
            return false;
        }
    }
    return true;
}

bool changeDetected(const defV& prevValue, const double* vals, size_t size, double deltaV)
{
    if (prevValue.index() == vector_loc) {
        const auto& prevV = std::get<std::vector<double>>(prevValue);
        if (size == prevV.size()) {
            for (size_t ii = 0; ii < size; ++ii) {
                if (std::abs(prevV[ii] - vals[ii]) > deltaV) {
                    return true;
                }
            }
            return false;
        }
    }
    return true;
}

bool changeDetected(const defV& prevValue, const std::complex<double>& val, double deltaV)
{
    if (prevValue.index() == complex_loc) {
        const auto& prevV = std::get<std::complex<double>>(prevValue);
        if (std::abs(prevV.real() - val.real()) > deltaV) {
            return true;
        }
        if (std::abs(prevV.imag() - val.imag()) > deltaV) {
            return true;
        }
        return false;
    }
    return true;
}

bool changeDetected(const defV& prevValue, double val, double deltaV)
{
    if (prevValue.index() == double_loc) {
        return (std::abs(std::get<double>(prevValue) - val) > deltaV);
    }
    return true;
}

bool changeDetected(const defV& prevValue, Time val, double deltaV)
{
    if (prevValue.index() == double_loc) {
        return (std::abs(std::get<double>(prevValue) - static_cast<double>(val)) > deltaV);
    }
    if (prevValue.index() == int_loc) {
        return (std::abs(Time(std::get<int64_t>(prevValue), time_units::ns) - val) > deltaV);
    }
    return true;
}

bool changeDetected(const defV& prevValue, int64_t val, double deltaV)
{
    if (prevValue.index() == int_loc) {
        return (std::abs(std::get<int64_t>(prevValue) - val) > static_cast<int64_t>(deltaV));
    }
    return true;
}

bool changeDetected(const defV& prevValue, const NamedPoint& val, double deltaV)
{
    if ((prevValue.index() == double_loc) && (!std::isnan(val.value))) {
        return (std::abs(std::get<double>(prevValue) - val.value) > deltaV);
    }
    if (prevValue.index() == named_point_loc) {
        if ((std::get<NamedPoint>(prevValue).name == val.name) && (!std::isnan(val.value))) {
            return (std::abs(std::get<NamedPoint>(prevValue).value - val.value) > deltaV);
        }
    }
    return true;
}

void valueExtract(const defV& data, std::string& val)
{
    switch (data.index()) {
        case double_loc:  // double
            val = std::to_string(std::get<double>(data));
            break;
        case int_loc:  // int64_t
            val = std::to_string(std::get<int64_t>(data));
            break;
        case string_loc:  // string
        default:
            val = std::get<std::string>(data);
            break;
        case complex_loc:  // complex
            val = helicsComplexString(std::get<std::complex<double>>(data));
            break;
        case vector_loc:  // vector
            val = helicsVectorString(std::get<std::vector<double>>(data));
            break;
        case complex_vector_loc:  // vector
            val = helicsComplexVectorString(std::get<std::vector<std::complex<double>>>(data));
            break;
        case named_point_loc: {
            const auto& point = std::get<NamedPoint>(data);
            val = (std::isnan(point.value)) ? point.name : helicsNamedPointString(point);
            break;
        }
    }
}

void valueExtract(const defV& data, std::complex<double>& val)
{
    switch (data.index()) {
        case double_loc:  // double
            val = std::complex<double>(std::get<double>(data), 0.0);
            break;
        case int_loc:  // int64_t
            val = std::complex<double>(static_cast<double>(std::get<int64_t>(data)), 0.0);
            break;
        case string_loc:  // string
        default:
            val = getComplexFromString(std::get<std::string>(data));
            break;
        case complex_loc:  // complex
            val = std::get<std::complex<double>>(data);
            break;
        case vector_loc:  // vector
        {
            const auto& vec = std::get<std::vector<double>>(data);
            if (vec.size() == 1) {
                val = std::complex<double>(vec[0], 0.0);
            } else if (vec.size() >= 2) {
                val = std::complex<double>(vec[0], vec[1]);
            }
            break;
        }
        case complex_vector_loc: {
            const auto& vec = std::get<std::vector<std::complex<double>>>(data);
            if (!vec.empty()) {
                val = vec.front();
            }
            break;
        }
        case named_point_loc: {
            const auto& point = std::get<NamedPoint>(data);
            if (std::isnan(point.value)) {
                val = getComplexFromString(point.name);
            } else {
                val = std::complex<double>(point.value, 0.0);
            }
        } break;
    }
}

void valueExtract(const defV& data, std::vector<double>& val)
{
    val.resize(0);
    switch (data.index()) {
        case double_loc:  // double
            val.push_back(std::get<double>(data));
            break;
        case int_loc:  // int64_t
            val.push_back(static_cast<double>(std::get<int64_t>(data)));
            break;
        case string_loc:  // string
        default:
            helicsGetVector(std::get<std::string>(data), val);
            break;
        case complex_loc:  // complex
        {
            const std::complex<double> cval = std::get<std::complex<double>>(data);
            val.push_back(cval.real());
            if (cval.imag() != 0.0) {
                val.push_back(cval.imag());
            }
        } break;
        case vector_loc:  // vector
            val = std::get<std::vector<double>>(data);
            break;
        case complex_vector_loc:  // complex
        {
            const auto& cvec = std::get<std::vector<std::complex<double>>>(data);
            val.reserve(cvec.size());
            val.clear();
            for (const auto& cval : cvec) {
                if (cval.imag() == 0.0) {
                    val.push_back(cval.real());
                } else {
                    val.push_back(std::abs(cval));
                }
            }
        } break;
        case named_point_loc:  // named point
        {
            const auto& point = std::get<NamedPoint>(data);
            if (std::isnan(point.value)) {
                val = helicsGetVector(point.name);
            } else {
                val.resize(1);
                val[0] = point.value;
            }
            break;
        }
    }
}

void valueExtract(const defV& data, std::vector<std::complex<double>>& val)
{
    val.resize(0);
    switch (data.index()) {
        case double_loc:  // double
            val.emplace_back(std::get<double>(data), 0.0);
            break;
        case int_loc:  // int64_t
            val.emplace_back(static_cast<double>(std::get<int64_t>(data)), 0.0);
            break;
        case string_loc:  // string
        default:
            helicsGetComplexVector(std::get<std::string>(data), val);
            break;
        case complex_loc:  // complex
        {
            val.push_back(std::get<std::complex<double>>(data));
        } break;
        case vector_loc:  // vector
        {
            const auto& vec = std::get<std::vector<double>>(data);
            val.reserve(vec.size() + 1 / 2);
            val.clear();
            for (size_t ii = 0; ii < vec.size() - 1; ii += 2) {
                val.emplace_back(vec[ii], vec[ii + 1]);
            }
            if (vec.size() % 2 == 1) {
                val.emplace_back(vec.back(), 0.0);
            }
            break;
        }
        case complex_vector_loc:  // complex
            val = std::get<std::vector<std::complex<double>>>(data);
            break;
        case named_point_loc:  // named point
        {
            const auto& point = std::get<NamedPoint>(data);
            if (std::isnan(point.value)) {
                val = helicsGetComplexVector(point.name);
            } else {
                val.resize(1);
                val[0] = std::complex<double>(point.value, 0.0);
            }
            break;
        }
    }
}

void valueExtract(const defV& data, NamedPoint& val)
{
    switch (data.index()) {
        case double_loc:  // double
            val.name = "value";
            val.value = std::get<double>(data);
            break;
        case int_loc:  // int64_t
            val.name = "value";
            val.value = static_cast<double>(std::get<int64_t>(data));
            break;
        case string_loc:  // string
        default:
            val = helicsGetNamedPoint(std::get<std::string>(data));
            break;
        case complex_loc:  // complex
            val.name = helicsComplexString(std::get<std::complex<double>>(data));
            val.value = std::nan("0");
            break;
        case vector_loc:  // vector
        {
            const auto& vec = std::get<std::vector<double>>(data);
            if (vec.size() == 1) {
                val.name = "value";
                val.value = vec[0];
            } else {
                val.name = helicsVectorString(vec);
                val.value = std::nan("0");
            }

            break;
        }
        case complex_vector_loc: {
            val.value = std::nan("0");
            const auto& vec = std::get<std::vector<std::complex<double>>>(data);
            if (vec.size() == 1) {
                val.name = helicsComplexString(vec[0]);
            } else {
                val.name = helicsComplexVectorString(vec);
            }
            break;
        }
        case named_point_loc:
            val = std::get<NamedPoint>(data);
            break;
    }
}

void valueExtract(const defV& data, Time& val)
{
    switch (data.index()) {
        case double_loc:  // double
            val = std::get<double>(data);
            break;
        case int_loc:  // int64_t
        default:
            val.setBaseTimeCode(std::get<int64_t>(data));
            break;
        case string_loc:  // string
        {
            size_t index;
            const auto& str = std::get<std::string>(data);
            try {
                auto timeCodeValue = std::stoll(str, &index);
                if ((index == std::string::npos) || (index == str.size())) {
                    val.setBaseTimeCode(timeCodeValue);
                } else {
                    val = gmlc::utilities::loadTimeFromString<helics::Time>(
                        std::get<std::string>(data));
                }
            }
            catch (...) {
                val = timeZero;
            }
            break;
        }
        case complex_loc:  // complex
            val = std::get<std::complex<double>>(data).real();
            break;
        case vector_loc:  // vector
        {
            const auto& vec = std::get<std::vector<double>>(data);
            val = (!vec.empty()) ? Time(vec[0]) : timeZero;
            break;
        }
        case complex_vector_loc: {
            const auto& vec = std::get<std::vector<std::complex<double>>>(data);
            val = (!vec.empty()) ? Time(vec[0].real()) : timeZero;
            break;
        }
        case named_point_loc:
            const auto& point = std::get<NamedPoint>(data);
            if (std::isnan(point.value)) {
                if (point.name.find(".[eE") == std::string::npos) {
                    const std::int64_t intVal = getIntFromString(point.name);
                    val.setBaseTimeCode(intVal);
                } else {
                    val = getDoubleFromString(point.name);
                }
            } else {
                val = point.value;
            }
            break;
    }
}

void valueExtract(const defV& data, char& val)
{
    switch (data.index()) {
        case double_loc:  // double
            val = static_cast<char>(std::get<double>(data));
            break;
        case int_loc:  // int64_t
        default:
            val = static_cast<char>(std::get<int64_t>(data));
            break;
        case string_loc:  // string
        {
            const auto& str = std::get<std::string>(data);
            val = (str.empty()) ? '\0' : str[0];
            break;
        }
        case complex_loc:  // complex
            val = static_cast<char>(std::get<std::complex<double>>(data).real());
            break;
        case vector_loc:  // vector
        {
            const auto& vec = std::get<std::vector<double>>(data);
            val = (!vec.empty()) ? static_cast<char>(vec[0]) : '\0';
            break;
        }
        case complex_vector_loc: {
            const auto& vec = std::get<std::vector<std::complex<double>>>(data);
            val = (!vec.empty()) ? static_cast<char>(vec[0].real()) : '\0';
            break;
        }
        case named_point_loc: {
            const auto& point = std::get<NamedPoint>(data);
            if (std::isnan(point.value)) {
                const double vald = getDoubleFromString(point.name);
                if (vald != invalidDouble) {
                    val = static_cast<char>(vald);
                } else {
                    val = !point.name.empty() ? point.name[0] : 0;
                }
            } else {
                val = static_cast<char>(point.value);
            }
        } break;
    }
}

void valueExtract(const defV& data, bool& val)
{
    switch (data.index()) {
        case double_loc:  // double
            val = std::abs(std::get<double>(data)) > 0.0;
            break;
        case int_loc:  // int64_t
        default:
            val = (std::get<int64_t>(data) != 0);
            break;
        case string_loc:  // string
        {
            const auto& str = std::get<std::string>(data);
            val = helicsBoolValue(str);
            break;
        }
        case complex_loc:  // complex
            val = std::abs(std::get<std::complex<double>>(data)) > 0.0;
            break;
        case vector_loc:  // vector
        {
            const auto& vec = std::get<std::vector<double>>(data);
            val = vectorNorm(vec) != 0.0;
            break;
        }
        case complex_vector_loc: {
            const auto& vec = std::get<std::vector<std::complex<double>>>(data);
            val = vectorNorm(vec) != 0.0;
            break;
        }
        case named_point_loc: {
            const auto& point = std::get<NamedPoint>(data);
            const auto& str = point.name;
            val = str.empty() || helicsBoolValue(str);
            if (val) {
                if ((str == "value" || str.empty()) && point.value == 0.0) {
                    val = false;
                }
            }
        } break;
    }
}

defV readJsonValue(const data_view& data)
{
    defV result;
    try {
        auto json = fileops::loadJsonStr(data.string_view());
        switch (getTypeFromString(json["type"].get<std::string>())) {
            case DataType::HELICS_DOUBLE:
                result = json["value"].get<double>();
                break;
            case DataType::HELICS_COMPLEX:
                result = std::complex<double>(json["value"][0].get<double>(),
                                              json["value"][1].get<double>());
                break;
            case DataType::HELICS_BOOL:
                result = static_cast<std::int64_t>(json["value"].get<bool>());
                break;
            case DataType::HELICS_VECTOR: {
                std::vector<double> res;
                for (const auto& value : json["value"]) {
                    res.push_back(value.get<double>());
                }
                result = std::move(res);
            } break;
            case DataType::HELICS_COMPLEX_VECTOR: {
                std::vector<std::complex<double>> res;
                auto& array = json["value"];
                for (std::size_t ii = 0; ii < array.size() - 1; ii += 2) {
                    res.emplace_back(array[ii].get<double>(), array[ii + 1].get<double>());
                }
                result = std::move(res);
            } break;
            case DataType::HELICS_INT:
            case DataType::HELICS_TIME:
                result = json["value"].get<int64_t>();
                break;
            case DataType::HELICS_STRING:
            case DataType::HELICS_CHAR:
                result = json["value"].get<std::string>();
                break;
            case DataType::HELICS_NAMED_POINT:
                result = NamedPoint(json["name"].get<std::string>(), json["value"].get<double>());
                break;
            case DataType::HELICS_MULTI:
            default:
                result = data.string();
        }
    }
    catch (...) {
        result = data.string();
    }
    return result;
}

void valueExtract(const data_view& data, DataType baseType, std::string& val)
{
    switch (baseType) {
        case DataType::HELICS_DOUBLE: {
            auto value = ValueConverter<double>::interpret(data);
            val = helicsDoubleString(value);
            break;
        }
        case DataType::HELICS_INT:
        case DataType::HELICS_TIME: {
            auto value = ValueConverter<int64_t>::interpret(data);
            val = helicsIntString(value);
            break;
        }
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
        case DataType::HELICS_BOOL:
        case DataType::HELICS_ANY:
            val = ValueConverter<std::string_view>::interpret(data);
            break;
        case DataType::HELICS_UNKNOWN:
        case DataType::HELICS_CUSTOM:
        case DataType::HELICS_MULTI:
            val = data.string();
            break;
        case DataType::HELICS_NAMED_POINT: {
            auto npv = ValueConverter<NamedPoint>::interpret(data);
            val = std::isnan(npv.value) ? npv.name : helicsNamedPointString(npv);
            break;
        }
        case DataType::HELICS_VECTOR:
            val = helicsVectorString(ValueConverter<std::vector<double>>::interpret(data));
            break;
        case DataType::HELICS_COMPLEX:
            val = helicsComplexString(ValueConverter<std::complex<double>>::interpret(data));
            break;
        case DataType::HELICS_COMPLEX_VECTOR:
            val = helicsComplexVectorString(
                ValueConverter<std::vector<std::complex<double>>>::interpret(data));
            break;
        case DataType::HELICS_JSON:
            valueExtract(readJsonValue(data), val);
            break;
    }
}

void valueExtract(const data_view& data, DataType baseType, std::vector<double>& val)
{
    val.resize(0);
    switch (baseType) {
        case DataType::HELICS_DOUBLE: {
            val.push_back(ValueConverter<double>::interpret(data));
            break;
        }
        case DataType::HELICS_INT: {
            val.push_back(static_cast<double>(ValueConverter<int64_t>::interpret(data)));
            break;
        }
        case DataType::HELICS_TIME: {
            const Time time(ValueConverter<int64_t>::interpret(data), time_units::ns);
            val.push_back(static_cast<double>(time));
        } break;
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
        default: {
            helicsGetVector(ValueConverter<std::string_view>::interpret(data), val);
            break;
        }
        case DataType::HELICS_NAMED_POINT: {
            auto npval = ValueConverter<NamedPoint>::interpret(data);
            if (std::isnan(npval.value)) {
                val = helicsGetVector(npval.name);
            } else {
                val.push_back(npval.value);
            }
            break;
        }
        case DataType::HELICS_VECTOR: {
            ValueConverter<std::vector<double>>::interpret(data, val);
            break;
        }
        case DataType::HELICS_COMPLEX: {
            auto cval = ValueConverter<std::complex<double>>::interpret(data);
            val.push_back(cval.real());
            if (cval.imag() != 0.0) {
                val.push_back(cval.imag());
            }
            break;
        }
        case DataType::HELICS_COMPLEX_VECTOR: {
            auto cvec = ValueConverter<std::vector<std::complex<double>>>::interpret(data);
            val.reserve(cvec.size());
            for (auto& cval : cvec) {
                if (cval.imag() == 0.0) {
                    val.push_back(cval.real());
                } else {
                    val.push_back(std::abs(cval));
                }
            }
            break;
        }
        case DataType::HELICS_JSON:
            valueExtract(readJsonValue(data), val);
            break;
    }
}

void valueExtract(const data_view& data, DataType baseType, std::vector<std::complex<double>>& val)
{
    val.resize(0);
    switch (baseType) {
        case DataType::HELICS_DOUBLE: {
            val.emplace_back(ValueConverter<double>::interpret(data), 0.0);
            break;
        }
        case DataType::HELICS_INT: {
            val.emplace_back(static_cast<double>(ValueConverter<int64_t>::interpret(data)), 0.0);
            break;
        }
        case DataType::HELICS_TIME: {
            const Time time(ValueConverter<int64_t>::interpret(data), time_units::ns);
            val.emplace_back(static_cast<double>(time), 0.0);
        } break;
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
        default: {
            helicsGetComplexVector(ValueConverter<std::string_view>::interpret(data), val);
            break;
        }
        case DataType::HELICS_VECTOR: {
            auto vec = ValueConverter<std::vector<double>>::interpret(data);
            for (size_t ii = 0; ii < vec.size() - 1; ii += 2) {
                val.emplace_back(vec[ii], vec[ii + 1]);
            }
            if (vec.size() % 2 == 1) {
                val.emplace_back(vec.back(), 0.0);
            }

            break;
        }
        case DataType::HELICS_COMPLEX_VECTOR: {
            ValueConverter<std::vector<std::complex<double>>>::interpret(data, val);
            break;
        }
        case DataType::HELICS_NAMED_POINT: {
            auto npval = ValueConverter<NamedPoint>::interpret(data);
            if (std::isnan(npval.value)) {
                val = helicsGetComplexVector(npval.name);
            } else {
                val.emplace_back(npval.value, 0.0);
            }
            break;
        }
        case DataType::HELICS_COMPLEX: {
            auto cval = ValueConverter<std::complex<double>>::interpret(data);
            val.push_back(cval);
            break;
        }
        case DataType::HELICS_JSON:
            valueExtract(readJsonValue(data), val);
            break;
    }
}

void valueExtract(const data_view& data, DataType baseType, std::complex<double>& val)
{
    switch (baseType) {
        case DataType::HELICS_DOUBLE: {
            val = std::complex<double>(ValueConverter<double>::interpret(data), 0.0);
            break;
        }
        case DataType::HELICS_INT: {
            val =
                std::complex<double>(static_cast<double>(ValueConverter<int64_t>::interpret(data)),
                                     0.0);
            break;
        }
        case DataType::HELICS_TIME: {
            const Time time(ValueConverter<int64_t>::interpret(data), time_units::ns);
            val = std::complex<double>(static_cast<double>(time), 0.0);
        } break;
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
        default: {
            val = helicsGetComplex(ValueConverter<std::string_view>::interpret(data));
            break;
        }
        case DataType::HELICS_NAMED_POINT: {
            auto npval = ValueConverter<NamedPoint>::interpret(data);
            if (std::isnan(npval.value)) {
                val = helicsGetComplex(npval.name);
            } else {
                val = std::complex<double>(npval.value, 0.0);
            }
            break;
        }
        case DataType::HELICS_VECTOR: {
            auto vec = ValueConverter<std::vector<double>>::interpret(data);
            if (vec.size() == 1) {
                val = std::complex<double>(vec[0], 0.0);
            } else if (vec.size() >= 2) {
                val = std::complex<double>(vec[0], vec[1]);
            }
            break;
        }
        case DataType::HELICS_COMPLEX:
            val = ValueConverter<std::complex<double>>::interpret(data);
            break;
        case DataType::HELICS_COMPLEX_VECTOR: {
            auto cvec = ValueConverter<std::vector<std::complex<double>>>::interpret(data);
            if (!cvec.empty()) {
                val = cvec[0];
            }
            break;
        }
        case DataType::HELICS_JSON:
            valueExtract(readJsonValue(data), val);
            break;
    }
}

void valueExtract(const data_view& data, DataType baseType, NamedPoint& val)
{
    switch (baseType) {
        case DataType::HELICS_DOUBLE: {
            auto value = ValueConverter<double>::interpret(data);
            val.name = "value";
            val.value = value;
            break;
        }
        case DataType::HELICS_INT: {
            auto value = ValueConverter<int64_t>::interpret(data);
            val.name = "value";
            val.value = static_cast<double>(value);
            break;
        }
        case DataType::HELICS_TIME: {
            const Time time(ValueConverter<int64_t>::interpret(data), time_units::ns);
            val.name = "time";
            val.value = static_cast<double>(time);
        } break;
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
        default: {
            val = helicsGetNamedPoint(ValueConverter<std::string_view>::interpret(data));
            break;
        }
        case DataType::HELICS_VECTOR: {
            auto vec = ValueConverter<std::vector<double>>::interpret(data);
            if (vec.size() == 1) {
                val.name = "value";

                val.value = vec[0];
            } else {
                val.name = helicsVectorString(vec);
                val.value = std::nan("0");
            }
            break;
        }
        case DataType::HELICS_COMPLEX: {
            auto cval = ValueConverter<std::complex<double>>::interpret(data);
            if (cval.imag() == 0) {
                val.name = "value";
                val.value = cval.real();
            } else {
                val.name = helicsComplexString(cval);
                val.value = std::nan("0");
            }

            break;
        }
        case DataType::HELICS_COMPLEX_VECTOR: {
            auto cvec = ValueConverter<std::vector<std::complex<double>>>::interpret(data);
            if (cvec.size() == 1) {
                val.name = helicsComplexString(cvec[0]);
                val.value = std::nan("0");
            } else {
                val.name = helicsComplexVectorString(cvec);
                val.value = std::nan("0");
            }
            break;
        }
        case DataType::HELICS_NAMED_POINT:
            val = ValueConverter<NamedPoint>::interpret(data);
            break;
        case DataType::HELICS_JSON:
            valueExtract(readJsonValue(data), val);
            break;
    }
}

void valueExtract(const data_view& data, DataType baseType, Time& val)
{
    switch (baseType) {
        case DataType::HELICS_DOUBLE: {
            val = ValueConverter<double>::interpret(data);
            break;
        }
        case DataType::HELICS_INT:
        case DataType::HELICS_TIME: {
            val.setBaseTimeCode(ValueConverter<int64_t>::interpret(data));
            break;
        }
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
        default: {
            size_t index;
            try {
                auto stringData = ValueConverter<std::string_view>::interpret(data);
                auto timeCode = std::stoll(std::string(stringData), &index);
                if ((index == std::string::npos) || (index == stringData.size())) {
                    val.setBaseTimeCode(timeCode);
                } else {
                    val = gmlc::utilities::loadTimeFromString<helics::Time>(stringData);
                }
            }
            catch (...) {
                val = timeZero;
            }

            break;
        }
        case DataType::HELICS_VECTOR: {
            auto vec = ValueConverter<std::vector<double>>::interpret(data);
            val = (!vec.empty()) ? Time(vec[0]) : timeZero;
            break;
        }
        case DataType::HELICS_COMPLEX: {
            auto cval = ValueConverter<std::complex<double>>::interpret(data);
            val = cval.real();
            break;
        }
        case DataType::HELICS_COMPLEX_VECTOR: {
            auto cvec = ValueConverter<std::vector<std::complex<double>>>::interpret(data);
            val = (!cvec.empty()) ? Time(cvec[0].real()) : timeZero;
            break;
        }
        case DataType::HELICS_NAMED_POINT: {
            auto point = ValueConverter<NamedPoint>::interpret(data);
            if (std::isnan(point.value)) {
                if (point.name.find(".[eE") == std::string::npos) {
                    const std::int64_t intVal = getIntFromString(point.name);
                    val.setBaseTimeCode(intVal);
                } else {
                    val = getDoubleFromString(point.name);
                }
            } else {
                val = point.value;
            }
            break;
        }
        case DataType::HELICS_JSON:
            valueExtract(readJsonValue(data), val);
            break;
    }
}

void valueExtract(const data_view& data, DataType baseType, bool& val)
{
    switch (baseType) {
        case DataType::HELICS_ANY: {
            defV val_dv;
            valueExtract(data, baseType, val_dv);
            valueExtract(val_dv, val);
            break;
        }
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
        default:
            val = helicsBoolValue(ValueConverter<std::string_view>::interpret(data));
            break;
        case DataType::HELICS_BOOL:
            val = (ValueConverter<std::string_view>::interpret(data) != "0");
            break;
        case DataType::HELICS_NAMED_POINT: {
            auto npval = ValueConverter<NamedPoint>::interpret(data);
            auto& str = npval.name;
            val = str.empty() || helicsBoolValue(str);
            if (val) {
                if ((str == "value" || str.empty()) && npval.value == 0.0) {
                    val = false;
                }
            }

            break;
        }
        case DataType::HELICS_DOUBLE: {
            auto value = ValueConverter<double>::interpret(data);
            val = std::abs(value) != 0;
            break;
        }
        case DataType::HELICS_INT:
        case DataType::HELICS_TIME: {
            auto value = ValueConverter<int64_t>::interpret(data);
            val = (value != 0);
            break;
        }

        case DataType::HELICS_VECTOR: {
            auto value = ValueConverter<std::vector<double>>::interpret(data);
            val = (vectorNorm(value) != 0.0);
            break;
        }
        case DataType::HELICS_COMPLEX: {
            auto value = ValueConverter<std::complex<double>>::interpret(data);
            val = (std::abs(value) != 0.0);
            break;
        }
        case DataType::HELICS_COMPLEX_VECTOR: {
            auto value = ValueConverter<std::vector<std::complex<double>>>::interpret(data);
            val = (vectorNorm(value) != 0.0);
            break;
        }
        case DataType::HELICS_JSON:
            valueExtract(readJsonValue(data), val);
            break;
        case DataType::HELICS_CUSTOM:
            throw(std::invalid_argument("unrecognized helics type"));
    }
}

void valueExtract(const data_view& data, DataType baseType, char& val)
{
    switch (baseType) {
        case DataType::HELICS_ANY: {
            defV val_dv;
            valueExtract(data, baseType, val_dv);
            valueExtract(val_dv, val);
            break;
        }
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
        default: {
            auto value = ValueConverter<std::string_view>::interpret(data);
            if (value.size() == 1) {
                val = value[0];
            } else {
                const double dval = getDoubleFromString(value);
                if (dval != invalidDouble) {
                    val = static_cast<char>(dval);
                } else {
                    val = value[0];
                }
            }
        }

        break;
        case DataType::HELICS_BOOL:
            val = ValueConverter<std::string_view>::interpret(data)[0];
            break;
        case DataType::HELICS_NAMED_POINT: {
            auto npval = ValueConverter<NamedPoint>::interpret(data);
            if (std::isnan(npval.value)) {
                if (npval.name.size() == 1) {
                    val = npval.name[0];
                } else {
                    const double value = getDoubleFromString(npval.name);
                    if (value != invalidDouble) {
                        val = static_cast<char>(value);
                    } else {
                        val = npval.name[0];
                    }
                }
            } else {
                val = static_cast<char>(npval.value);
            }
            break;
        }
        case DataType::HELICS_DOUBLE: {
            val = static_cast<char>(ValueConverter<double>::interpret(data));
            break;
        }
        case DataType::HELICS_INT:
        case DataType::HELICS_TIME: {
            auto intVal = ValueConverter<int64_t>::interpret(data);
            val = static_cast<char>(intVal);
            break;
        }

        case DataType::HELICS_VECTOR: {
            auto vec = ValueConverter<std::vector<double>>::interpret(data);
            val = static_cast<char>((vec.size() == 1) ? vec[0] : vectorNorm(vec));
            break;
        }
        case DataType::HELICS_COMPLEX: {
            auto cval = ValueConverter<std::complex<double>>::interpret(data);
            val = static_cast<char>((cval.imag() == 0.0) ? cval.real() : std::abs(cval));
            break;
        }
        case DataType::HELICS_COMPLEX_VECTOR: {
            auto cvec = ValueConverter<std::vector<std::complex<double>>>::interpret(data);
            val = static_cast<char>(
                (cvec.size() == 1) ?
                    ((cvec[0].imag() == 0.0) ? cvec[0].real() : std::abs(cvec[0])) :
                    vectorNorm(cvec));
            break;
        }
        case DataType::HELICS_JSON:
            valueExtract(readJsonValue(data), val);
            break;
        case DataType::HELICS_CUSTOM:
        case DataType::HELICS_MULTI:
            throw(std::invalid_argument("unrecognized helics type"));
    }
}

void valueExtract(const data_view& data, DataType baseType, defV& val)
{
    if (baseType == DataType::HELICS_ANY || baseType == DataType::HELICS_UNKNOWN) {
        baseType = detail::detectType(data.bytes());
    }
    switch (baseType) {
        case DataType::HELICS_DOUBLE:
            val = ValueConverter<double>::interpret(data);
            break;
        case DataType::HELICS_INT:
        case DataType::HELICS_TIME:
            val = ValueConverter<int64_t>::interpret(data);
            break;
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
        case DataType::HELICS_BOOL:
        case DataType::HELICS_ANY:
            val = std::string(ValueConverter<std::string_view>::interpret(data));
            break;
        case DataType::HELICS_UNKNOWN:
        case DataType::HELICS_CUSTOM:
        case DataType::HELICS_MULTI:
            val = data.string();
            break;
        case DataType::HELICS_VECTOR:
            val = ValueConverter<std::vector<double>>::interpret(data);
            break;
        case DataType::HELICS_COMPLEX:
            val = ValueConverter<std::complex<double>>::interpret(data);
            break;
        case DataType::HELICS_COMPLEX_VECTOR:
            val = ValueConverter<std::vector<std::complex<double>>>::interpret(data);
            break;
        case DataType::HELICS_NAMED_POINT:
            val = ValueConverter<NamedPoint>::interpret(data);
            break;
        case DataType::HELICS_JSON:
            val = readJsonValue(data);
            break;
    }
}

void valueConvert(defV& val, DataType newType)
{
    auto index = val.index();
    switch (newType) {
        case DataType::HELICS_DOUBLE: {
            if (index == double_loc) {
                return;
            }
            double value{0};
            valueExtract(val, value);
            val = value;
            break;
        }
        case DataType::HELICS_INT: {
            if (index == int_loc) {
                return;
            }
            int64_t value{0};
            valueExtract(val, value);
            val = value;
            break;
        }
        case DataType::HELICS_TIME: {
            if (index == int_loc) {
                return;
            }
            Time value{timeZero};
            valueExtract(val, value);
            val = value.getBaseTimeCode();
            break;
        }
        case DataType::HELICS_JSON:
            break;
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
        default: {
            if (index == string_loc) {
                return;
            }
            std::string value;
            valueExtract(val, value);
            val = std::move(value);
            break;
        }
        case DataType::HELICS_VECTOR: {
            if (index == vector_loc) {
                return;
            }
            std::vector<double> vec;
            valueExtract(val, vec);
            val = std::move(vec);
            break;
        }
        case DataType::HELICS_COMPLEX: {
            if (index == complex_loc) {
                return;
            }
            std::complex<double> cval;
            valueExtract(val, cval);
            val = cval;
            break;
        }
        case DataType::HELICS_COMPLEX_VECTOR: {
            if (index == complex_vector_loc) {
                return;
            }
            std::vector<std::complex<double>> cvec;
            valueExtract(val, cvec);
            val = std::move(cvec);
            break;
        }
        case DataType::HELICS_NAMED_POINT: {
            if (index == named_point_loc) {
                return;
            }
            NamedPoint point;
            valueExtract(val, point);
            val = std::move(point);
            break;
        }
    }
}

SmallBuffer typeConvertDefV(DataType type, const defV& val)
{
    switch (val.index()) {
        case double_loc:  // double
            return typeConvert(type, std::get<double>(val));
        case int_loc:  // int64_t
            return typeConvert(type, std::get<int64_t>(val));
        case string_loc:  // string
        default:
            return typeConvert(type, std::string_view{std::get<std::string>(val)});
        case complex_loc:  // complex
            return typeConvert(type, std::get<std::complex<double>>(val));
        case vector_loc:  // vector
            return typeConvert(type, std::get<std::vector<double>>(val));
        case complex_vector_loc:  // complex
            return typeConvert(type, std::get<std::vector<std::complex<double>>>(val));
        case named_point_loc:
            return typeConvert(type, std::get<NamedPoint>(val));
    }
}

SmallBuffer typeConvertDefV(const defV& val)
{
    switch (val.index()) {
        case double_loc:  // double
            return typeConvert(DataType::HELICS_DOUBLE, std::get<double>(val));
        case int_loc:  // int64_t
            return typeConvert(DataType::HELICS_INT, std::get<int64_t>(val));
        case string_loc:  // string
        default:
            return typeConvert(DataType::HELICS_STRING,
                               std::string_view{std::get<std::string>(val)});
        case complex_loc:  // complex
            return typeConvert(DataType::HELICS_COMPLEX, std::get<std::complex<double>>(val));
        case vector_loc:  // vector
            return typeConvert(DataType::HELICS_VECTOR, std::get<std::vector<double>>(val));
        case complex_vector_loc:  // complex
            return typeConvert(DataType::HELICS_COMPLEX_VECTOR,
                               std::get<std::vector<std::complex<double>>>(val));
        case named_point_loc:
            return typeConvert(DataType::HELICS_NAMED_POINT, std::get<NamedPoint>(val));
    }
}

}  // namespace helics
