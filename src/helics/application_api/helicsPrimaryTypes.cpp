/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "HelicsPrimaryTypes.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "../utilities/timeStringOps.hpp"
#include "ValueConverter.hpp"

#include <set>
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

static const std::set<std::string> falseString{"0",
                                               "",
                                               "false",
                                               "False",
                                               "FALSE",
                                               "f",
                                               "F",
                                               "0",
                                               std::string(1, '\0'),
                                               " ",
                                               "no",
                                               "NO",
                                               "No",
                                               "-"};

static bool isTrueString(const std::string& str)
{
    if (str == "1") {
        return true;
    }
    if (str == "0") {
        return false;
    }
    return (falseString.find(str) != falseString.end());
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

void valueExtract(const defV& dv, std::string& val)
{
    switch (dv.index()) {
        case double_loc:  // double
            val = std::to_string(std::get<double>(dv));
            break;
        case int_loc:  // int64_t
            val = std::to_string(std::get<int64_t>(dv));
            break;
        case string_loc:  // string
        default:
            val = std::get<std::string>(dv);
            break;
        case complex_loc:  // complex
            val = helicsComplexString(std::get<std::complex<double>>(dv));
            break;
        case vector_loc:  // vector
            val = helicsVectorString(std::get<std::vector<double>>(dv));
            break;
        case complex_vector_loc:  // vector
            val = helicsComplexVectorString(std::get<std::vector<std::complex<double>>>(dv));
            break;
        case named_point_loc: {
            const auto& np = std::get<NamedPoint>(dv);
            val = (std::isnan(np.value)) ? np.name : helicsNamedPointString(np);
            break;
        }
    }
}

void valueExtract(const defV& dv, std::complex<double>& val)
{
    switch (dv.index()) {
        case double_loc:  // double
            val = std::complex<double>(std::get<double>(dv), 0.0);
            break;
        case int_loc:  // int64_t
            val = std::complex<double>(static_cast<double>(std::get<int64_t>(dv)), 0.0);
            break;
        case string_loc:  // string
        default:
            val = getComplexFromString(std::get<std::string>(dv));
            break;
        case complex_loc:  // complex
            val = std::get<std::complex<double>>(dv);
            break;
        case vector_loc:  // vector
        {
            const auto& vec = std::get<std::vector<double>>(dv);
            if (vec.size() == 1) {
                val = std::complex<double>(vec[0], 0.0);
            } else if (vec.size() >= 2) {
                val = std::complex<double>(vec[0], vec[1]);
            }
            break;
        }
        case complex_vector_loc: {
            const auto& vec = std::get<std::vector<std::complex<double>>>(dv);
            if (!vec.empty()) {
                val = vec.front();
            }
            break;
        }
        case named_point_loc: {
            const auto& np = std::get<NamedPoint>(dv);
            if (std::isnan(np.value)) {
                val = getComplexFromString(np.name);
            } else {
                val = std::complex<double>(np.value, 0.0);
            }
        } break;
    }
}

void valueExtract(const defV& dv, std::vector<double>& val)
{
    val.resize(0);
    switch (dv.index()) {
        case double_loc:  // double
            val.push_back(std::get<double>(dv));
            break;
        case int_loc:  // int64_t
            val.push_back(static_cast<double>(std::get<int64_t>(dv)));
            break;
        case string_loc:  // string
        default:
            helicsGetVector(std::get<std::string>(dv), val);
            break;
        case complex_loc:  // complex
        {
            auto cval = std::get<std::complex<double>>(dv);
            val.push_back(cval.real());
            if (cval.imag() != 0.0) {
                val.push_back(cval.imag());
            }
        } break;
        case vector_loc:  // vector
            val = std::get<std::vector<double>>(dv);
            break;
        case complex_vector_loc:  // complex
        {
            const auto& cv = std::get<std::vector<std::complex<double>>>(dv);
            val.reserve(cv.size());
            val.clear();
            for (const auto& cval : cv) {
                if (cval.imag() == 0.0) {
                    val.push_back(cval.real());
                } else {
                    val.push_back(std::abs(cval));
                }
            }
        } break;
        case named_point_loc:  // named point
        {
            const auto& np = std::get<NamedPoint>(dv);
            if (std::isnan(np.value)) {
                val = helicsGetVector(np.name);
            } else {
                val.resize(1);
                val[0] = np.value;
            }
            break;
        }
    }
}

void valueExtract(const defV& dv, std::vector<std::complex<double>>& val)
{
    val.resize(0);
    switch (dv.index()) {
        case double_loc:  // double
            val.emplace_back(std::get<double>(dv), 0.0);
            break;
        case int_loc:  // int64_t
            val.emplace_back(static_cast<double>(std::get<int64_t>(dv)), 0.0);
            break;
        case string_loc:  // string
        default:
            helicsGetComplexVector(std::get<std::string>(dv), val);
            break;
        case complex_loc:  // complex
        {
            val.push_back(std::get<std::complex<double>>(dv));
        } break;
        case vector_loc:  // vector
        {
            const auto& v = std::get<std::vector<double>>(dv);
            val.reserve(v.size() + 1 / 2);
            val.clear();
            for (size_t ii = 0; ii < v.size() - 1; ii += 2) {
                val.emplace_back(v[ii], v[ii + 1]);
            }
            if (v.size() % 2 == 1) {
                val.emplace_back(v.back(), 0.0);
            }
            break;
        }
        case complex_vector_loc:  // complex
            val = std::get<std::vector<std::complex<double>>>(dv);
            break;
        case named_point_loc:  // named point
        {
            const auto& np = std::get<NamedPoint>(dv);
            if (std::isnan(np.value)) {
                val = helicsGetComplexVector(np.name);
            } else {
                val.resize(1);
                val[0] = std::complex<double>(np.value, 0.0);
            }
            break;
        }
    }
}

void valueExtract(const defV& dv, NamedPoint& val)
{
    switch (dv.index()) {
        case double_loc:  // double
            val.name = "value";
            val.value = std::get<double>(dv);
            break;
        case int_loc:  // int64_t
            val.name = "value";
            val.value = static_cast<double>(std::get<int64_t>(dv));
            break;
        case string_loc:  // string
        default:
            val = helicsGetNamedPoint(std::get<std::string>(dv));
            break;
        case complex_loc:  // complex
            val.name = helicsComplexString(std::get<std::complex<double>>(dv));
            val.value = std::nan("0");
            break;
        case vector_loc:  // vector
        {
            const auto& vec = std::get<std::vector<double>>(dv);
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
            const auto& vec = std::get<std::vector<std::complex<double>>>(dv);
            if (vec.size() == 1) {
                val.name = helicsComplexString(vec[0]);
            } else {
                val.name = helicsComplexVectorString(vec);
            }
            break;
        }
        case named_point_loc:
            val = std::get<NamedPoint>(dv);
            break;
    }
}

void valueExtract(const defV& dv, Time& val)
{
    switch (dv.index()) {
        case double_loc:  // double
            val = std::get<double>(dv);
            break;
        case int_loc:  // int64_t
        default:
            val.setBaseTimeCode(std::get<int64_t>(dv));
            break;
        case string_loc:  // string
        {
            size_t index;
            const auto& str = std::get<std::string>(dv);
            try {
                auto ul = std::stoll(str, &index);
                if ((index == std::string::npos) || (index == str.size())) {
                    val.setBaseTimeCode(ul);
                } else {
                    val = gmlc::utilities::loadTimeFromString<helics::Time>(
                        std::get<std::string>(dv));
                }
            }
            catch (...) {
                val = timeZero;
            }
            break;
        }
        case complex_loc:  // complex
            val = std::get<std::complex<double>>(dv).real();
            break;
        case vector_loc:  // vector
        {
            const auto& vec = std::get<std::vector<double>>(dv);
            val = (!vec.empty()) ? Time(vec[0]) : timeZero;
            break;
        }
        case complex_vector_loc: {
            const auto& vec = std::get<std::vector<std::complex<double>>>(dv);
            val = (!vec.empty()) ? Time(vec[0].real()) : timeZero;
            break;
        }
        case named_point_loc:
            auto np = std::get<NamedPoint>(dv);
            if (std::isnan(np.value)) {
                if (np.name.find(".[eE") == std::string::npos) {
                    std::int64_t v = getIntFromString(np.name);
                    val.setBaseTimeCode(v);
                } else {
                    val = getDoubleFromString(np.name);
                }
            } else {
                val = np.value;
            }
            break;
    }
}

void valueExtract(const defV& dv, char& val)
{
    switch (dv.index()) {
        case double_loc:  // double
            val = static_cast<char>(std::get<double>(dv));
            break;
        case int_loc:  // int64_t
        default:
            val = static_cast<char>(std::get<int64_t>(dv));
            break;
        case string_loc:  // string
        {
            const auto& str = std::get<std::string>(dv);
            val = (str.empty()) ? '\0' : str[0];
            break;
        }
        case complex_loc:  // complex
            val = static_cast<char>(std::get<std::complex<double>>(dv).real());
            break;
        case vector_loc:  // vector
        {
            const auto& vec = std::get<std::vector<double>>(dv);
            val = (!vec.empty()) ? static_cast<char>(vec[0]) : '\0';
            break;
        }
        case complex_vector_loc: {
            const auto& vec = std::get<std::vector<std::complex<double>>>(dv);
            val = (!vec.empty()) ? static_cast<char>(vec[0].real()) : '\0';
            break;
        }
        case named_point_loc: {
            const auto& np = std::get<NamedPoint>(dv);
            if (std::isnan(np.value)) {
                double vald = getDoubleFromString(np.name);
                if (vald != invalidDouble) {
                    val = static_cast<char>(vald);
                } else {
                    val = !np.name.empty() ? np.name[0] : 0;
                }
            } else {
                val = static_cast<char>(np.value);
            }
        } break;
    }
}

void valueExtract(const defV& dv, bool& val)
{
    switch (dv.index()) {
        case double_loc:  // double
            val = std::abs(std::get<double>(dv)) > 0.0;
            break;
        case int_loc:  // int64_t
        default:
            val = (std::get<int64_t>(dv) != 0);
            break;
        case string_loc:  // string
        {
            const auto& str = std::get<std::string>(dv);
            val = helicsBoolValue(str);
            break;
        }
        case complex_loc:  // complex
            val = std::abs(std::get<std::complex<double>>(dv)) > 0.0;
            break;
        case vector_loc:  // vector
        {
            const auto& vec = std::get<std::vector<double>>(dv);
            val = vectorNorm(vec) != 0.0;
            break;
        }
        case complex_vector_loc: {
            const auto& vec = std::get<std::vector<std::complex<double>>>(dv);
            val = vectorNorm(vec) != 0.0;
            break;
        }
        case named_point_loc: {
            const auto& np = std::get<NamedPoint>(dv);
            const auto& str = np.name;
            val = str.empty() || helicsBoolValue(str);
            if (val) {
                if ((str == "value" || str.empty()) && np.value == 0.0) {
                    val = false;
                }
            }
        } break;
    }
}

defV readJsonValue(const data_view& dv)
{
    defV result;
    try {
        auto jv = fileops::loadJsonStr(dv.string_view());
        switch (getTypeFromString(jv["type"].asCString())) {
            case DataType::HELICS_DOUBLE:
                result = jv["value"].asDouble();
                break;
            case DataType::HELICS_COMPLEX:
                result = std::complex<double>(jv["value"][0].asDouble(), jv["value"][1].asDouble());
                break;
            case DataType::HELICS_BOOL:
                result = static_cast<std::int64_t>(jv["value"].asBool());
                break;
            case DataType::HELICS_VECTOR: {
                std::vector<double> res;
                for (const auto& v : jv["value"]) {
                    res.push_back(v.asDouble());
                }
                result = std::move(res);
            } break;
            case DataType::HELICS_COMPLEX_VECTOR: {
                std::vector<std::complex<double>> res;
                auto ca = jv["value"];
                for (Json::ArrayIndex ii = 0; ii < ca.size() - 1; ii += 2) {
                    res.emplace_back(ca[ii].asDouble(), ca[ii + 1].asDouble());
                }
                result = std::move(res);
            } break;
            case DataType::HELICS_INT:
            case DataType::HELICS_TIME:
                result = jv["value"].asInt64();
                break;
            case DataType::HELICS_STRING:
            case DataType::HELICS_CHAR:
                result = jv["value"].asString();
                break;
            case DataType::HELICS_NAMED_POINT:
                result = NamedPoint(jv["name"].asCString(), jv["value"].asDouble());
                break;
            default:
                result = dv.string();
        }
    }
    catch (...) {
        result = dv.string();
    }
    return result;
}

void valueExtract(const data_view& dv, DataType baseType, std::string& val)
{
    switch (baseType) {
        case DataType::HELICS_DOUBLE: {
            auto V = ValueConverter<double>::interpret(dv);
            val = helicsDoubleString(V);
            break;
        }
        case DataType::HELICS_INT:
        case DataType::HELICS_TIME: {
            auto V = ValueConverter<int64_t>::interpret(dv);
            val = helicsIntString(V);
            break;
        }
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
        default:
            val = ValueConverter<std::string_view>::interpret(dv);
            break;
        case DataType::HELICS_NAMED_POINT: {
            auto npv = ValueConverter<NamedPoint>::interpret(dv);
            val = std::isnan(npv.value) ? npv.name : helicsNamedPointString(npv);
            break;
        }
        case DataType::HELICS_VECTOR:
            val = helicsVectorString(ValueConverter<std::vector<double>>::interpret(dv));
            break;
        case DataType::HELICS_COMPLEX:
            val = helicsComplexString(ValueConverter<std::complex<double>>::interpret(dv));
            break;
        case DataType::HELICS_COMPLEX_VECTOR:
            val = helicsComplexVectorString(
                ValueConverter<std::vector<std::complex<double>>>::interpret(dv));
            break;
        case DataType::HELICS_JSON:
            valueExtract(readJsonValue(dv), val);
            break;
    }
}

void valueExtract(const data_view& dv, DataType baseType, std::vector<double>& val)
{
    val.resize(0);
    switch (baseType) {
        case DataType::HELICS_DOUBLE: {
            val.push_back(ValueConverter<double>::interpret(dv));
            break;
        }
        case DataType::HELICS_INT: {
            val.push_back(static_cast<double>(ValueConverter<int64_t>::interpret(dv)));
            break;
        }
        case DataType::HELICS_TIME: {
            Time tm(ValueConverter<int64_t>::interpret(dv), time_units::ns);
            val.push_back(static_cast<double>(tm));
        } break;
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
        default: {
            helicsGetVector(ValueConverter<std::string_view>::interpret(dv), val);
            break;
        }
        case DataType::HELICS_NAMED_POINT: {
            auto npval = ValueConverter<NamedPoint>::interpret(dv);
            if (std::isnan(npval.value)) {
                val = helicsGetVector(npval.name);
            } else {
                val.push_back(npval.value);
            }
            break;
        }
        case DataType::HELICS_VECTOR: {
            ValueConverter<std::vector<double>>::interpret(dv, val);
            break;
        }
        case DataType::HELICS_COMPLEX: {
            auto cval = ValueConverter<std::complex<double>>::interpret(dv);
            val.push_back(cval.real());
            if (cval.imag() != 0.0) {
                val.push_back(cval.imag());
            }
            break;
        }
        case DataType::HELICS_COMPLEX_VECTOR: {
            auto cv = ValueConverter<std::vector<std::complex<double>>>::interpret(dv);
            val.reserve(cv.size());
            for (auto& cval : cv) {
                if (cval.imag() == 0.0) {
                    val.push_back(cval.real());
                } else {
                    val.push_back(std::abs(cval));
                }
            }
            break;
        }
        case DataType::HELICS_JSON:
            valueExtract(readJsonValue(dv), val);
            break;
    }
}

void valueExtract(const data_view& dv, DataType baseType, std::vector<std::complex<double>>& val)
{
    val.resize(0);
    switch (baseType) {
        case DataType::HELICS_DOUBLE: {
            val.emplace_back(ValueConverter<double>::interpret(dv), 0.0);
            break;
        }
        case DataType::HELICS_INT: {
            val.emplace_back(static_cast<double>(ValueConverter<int64_t>::interpret(dv)), 0.0);
            break;
        }
        case DataType::HELICS_TIME: {
            Time tm(ValueConverter<int64_t>::interpret(dv), time_units::ns);
            val.emplace_back(static_cast<double>(tm), 0.0);
        } break;
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
        default: {
            helicsGetComplexVector(ValueConverter<std::string_view>::interpret(dv), val);
            break;
        }
        case DataType::HELICS_VECTOR: {
            auto V = ValueConverter<std::vector<double>>::interpret(dv);
            for (size_t ii = 0; ii < V.size() - 1; ii += 2) {
                val.emplace_back(V[ii], V[ii + 1]);
            }
            if (V.size() % 2 == 1) {
                val.emplace_back(V.back(), 0.0);
            }

            break;
        }
        case DataType::HELICS_COMPLEX_VECTOR: {
            ValueConverter<std::vector<std::complex<double>>>::interpret(dv, val);
            break;
        }
        case DataType::HELICS_NAMED_POINT: {
            auto npval = ValueConverter<NamedPoint>::interpret(dv);
            if (std::isnan(npval.value)) {
                val = helicsGetComplexVector(npval.name);
            } else {
                val.emplace_back(npval.value, 0.0);
            }
            break;
        }
        case DataType::HELICS_COMPLEX: {
            auto cval = ValueConverter<std::complex<double>>::interpret(dv);
            val.push_back(cval);
            break;
        }
        case DataType::HELICS_JSON:
            valueExtract(readJsonValue(dv), val);
            break;
    }
}

void valueExtract(const data_view& dv, DataType baseType, std::complex<double>& val)
{
    switch (baseType) {
        case DataType::HELICS_DOUBLE: {
            val = std::complex<double>(ValueConverter<double>::interpret(dv), 0.0);
            break;
        }
        case DataType::HELICS_INT: {
            val = std::complex<double>(static_cast<double>(ValueConverter<int64_t>::interpret(dv)),
                                       0.0);
            break;
        }
        case DataType::HELICS_TIME: {
            Time tm(ValueConverter<int64_t>::interpret(dv), time_units::ns);
            val = std::complex<double>(static_cast<double>(tm), 0.0);
        } break;
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
        default: {
            val = helicsGetComplex(ValueConverter<std::string_view>::interpret(dv));
            break;
        }
        case DataType::HELICS_NAMED_POINT: {
            auto npval = ValueConverter<NamedPoint>::interpret(dv);
            if (std::isnan(npval.value)) {
                val = helicsGetComplex(npval.name);
            } else {
                val = std::complex<double>(npval.value, 0.0);
            }
            break;
        }
        case DataType::HELICS_VECTOR: {
            auto vec = ValueConverter<std::vector<double>>::interpret(dv);
            if (vec.size() == 1) {
                val = std::complex<double>(vec[0], 0.0);
            } else if (vec.size() >= 2) {
                val = std::complex<double>(vec[0], vec[1]);
            }
            break;
        }
        case DataType::HELICS_COMPLEX:
            val = ValueConverter<std::complex<double>>::interpret(dv);
            break;
        case DataType::HELICS_COMPLEX_VECTOR: {
            auto cvec = ValueConverter<std::vector<std::complex<double>>>::interpret(dv);
            if (!cvec.empty()) {
                val = cvec[0];
            }
            break;
        }
        case DataType::HELICS_JSON:
            valueExtract(readJsonValue(dv), val);
            break;
    }
}

void valueExtract(const data_view& dv, DataType baseType, NamedPoint& val)
{
    switch (baseType) {
        case DataType::HELICS_DOUBLE: {
            auto V = ValueConverter<double>::interpret(dv);
            val.name = "value";
            val.value = V;
            break;
        }
        case DataType::HELICS_INT: {
            auto V = ValueConverter<int64_t>::interpret(dv);
            val.name = "value";
            val.value = static_cast<double>(V);
            break;
        }
        case DataType::HELICS_TIME: {
            Time tm(ValueConverter<int64_t>::interpret(dv), time_units::ns);
            val.name = "time";
            val.value = static_cast<double>(tm);
        } break;
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
        default: {
            val = helicsGetNamedPoint(ValueConverter<std::string_view>::interpret(dv));
            break;
        }
        case DataType::HELICS_VECTOR: {
            auto vec = ValueConverter<std::vector<double>>::interpret(dv);
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
            auto cval = ValueConverter<std::complex<double>>::interpret(dv);
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
            auto cvec = ValueConverter<std::vector<std::complex<double>>>::interpret(dv);
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
            val = ValueConverter<NamedPoint>::interpret(dv);
            break;
        case DataType::HELICS_JSON:
            valueExtract(readJsonValue(dv), val);
            break;
    }
}

void valueExtract(const data_view& dv, DataType baseType, Time& val)
{
    switch (baseType) {
        case DataType::HELICS_DOUBLE: {
            val = ValueConverter<double>::interpret(dv);
            break;
        }
        case DataType::HELICS_INT:
        case DataType::HELICS_TIME: {
            val.setBaseTimeCode(ValueConverter<int64_t>::interpret(dv));
            break;
        }
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
        default: {
            size_t index;
            try {
                auto data = ValueConverter<std::string_view>::interpret(dv);
                auto ul = std::stoll(std::string(data), &index);
                if ((index == std::string::npos) || (index == data.size())) {
                    val.setBaseTimeCode(ul);
                } else {
                    val = gmlc::utilities::loadTimeFromString<helics::Time>(data);
                }
            }
            catch (...) {
                val = timeZero;
            }

            break;
        }
        case DataType::HELICS_VECTOR: {
            auto vec = ValueConverter<std::vector<double>>::interpret(dv);
            val = (!vec.empty()) ? Time(vec[0]) : timeZero;
            break;
        }
        case DataType::HELICS_COMPLEX: {
            auto cval = ValueConverter<std::complex<double>>::interpret(dv);
            val = cval.real();
            break;
        }
        case DataType::HELICS_COMPLEX_VECTOR: {
            auto cvec = ValueConverter<std::vector<std::complex<double>>>::interpret(dv);
            val = (!cvec.empty()) ? Time(cvec[0].real()) : timeZero;
            break;
        }
        case DataType::HELICS_NAMED_POINT: {
            auto np = ValueConverter<NamedPoint>::interpret(dv);
            if (std::isnan(np.value)) {
                if (np.name.find(".[eE") == std::string::npos) {
                    std::int64_t v = getIntFromString(np.name);
                    val.setBaseTimeCode(v);
                } else {
                    val = getDoubleFromString(np.name);
                }
            } else {
                val = np.value;
            }
            break;
        }
        case DataType::HELICS_JSON:
            valueExtract(readJsonValue(dv), val);
            break;
    }
}

void valueExtract(const data_view& dv, DataType baseType, bool& val)
{
    switch (baseType) {
        case DataType::HELICS_ANY: {
            defV val_dv;
            valueExtract(dv, baseType, val_dv);
            valueExtract(val_dv, val);
            break;
        }
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
        default:
            val = helicsBoolValue(ValueConverter<std::string_view>::interpret(dv));
            break;
        case DataType::HELICS_BOOL:
            val = (ValueConverter<std::string_view>::interpret(dv) != "0");
            break;
        case DataType::HELICS_NAMED_POINT: {
            auto npval = ValueConverter<NamedPoint>::interpret(dv);
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
            auto V = ValueConverter<double>::interpret(dv);
            val = std::abs(V) != 0;
            break;
        }
        case DataType::HELICS_INT:
        case DataType::HELICS_TIME: {
            auto V = ValueConverter<int64_t>::interpret(dv);
            val = (V != 0);
            break;
        }

        case DataType::HELICS_VECTOR: {
            auto V = ValueConverter<std::vector<double>>::interpret(dv);
            val = (vectorNorm(V) != 0.0);
            break;
        }
        case DataType::HELICS_COMPLEX: {
            auto V = ValueConverter<std::complex<double>>::interpret(dv);
            val = (std::abs(V) != 0.0);
            break;
        }
        case DataType::HELICS_COMPLEX_VECTOR: {
            auto V = ValueConverter<std::vector<std::complex<double>>>::interpret(dv);
            val = (vectorNorm(V) != 0.0);
            break;
        }
        case DataType::HELICS_JSON:
            valueExtract(readJsonValue(dv), val);
            break;
        case DataType::HELICS_CUSTOM:
            throw(std::invalid_argument("unrecognized helics type"));
    }
}

void valueExtract(const data_view& dv, DataType baseType, char& val)
{
    switch (baseType) {
        case DataType::HELICS_ANY: {
            defV val_dv;
            valueExtract(dv, baseType, val_dv);
            valueExtract(val_dv, val);
            break;
        }
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
        default: {
            auto sv = ValueConverter<std::string_view>::interpret(dv);
            if (sv.size() == 1) {
                val = sv[0];
            } else {
                double v = getDoubleFromString(sv);
                if (v != invalidDouble) {
                    val = static_cast<char>(v);
                } else {
                    val = sv[0];
                }
            }
        }

        break;
        case DataType::HELICS_BOOL:
            val = ValueConverter<std::string_view>::interpret(dv)[0];
            break;
        case DataType::HELICS_NAMED_POINT: {
            auto npval = ValueConverter<NamedPoint>::interpret(dv);
            if (std::isnan(npval.value)) {
                if (npval.name.size() == 1) {
                    val = npval.name[0];
                } else {
                    double v = getDoubleFromString(npval.name);
                    if (v != invalidDouble) {
                        val = static_cast<char>(v);
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
            val = static_cast<char>(ValueConverter<double>::interpret(dv));
            break;
        }
        case DataType::HELICS_INT:
        case DataType::HELICS_TIME: {
            auto V = ValueConverter<int64_t>::interpret(dv);
            val = static_cast<char>(V);
            break;
        }

        case DataType::HELICS_VECTOR: {
            auto V = ValueConverter<std::vector<double>>::interpret(dv);
            val = static_cast<char>((V.size() == 1) ? V[0] : vectorNorm(V));
            break;
        }
        case DataType::HELICS_COMPLEX: {
            auto V = ValueConverter<std::complex<double>>::interpret(dv);
            val = static_cast<char>((V.imag() == 0.0) ? V.real() : std::abs(V));
            break;
        }
        case DataType::HELICS_COMPLEX_VECTOR: {
            auto V = ValueConverter<std::vector<std::complex<double>>>::interpret(dv);
            val = static_cast<char>((V.size() == 1) ?
                                        ((V[0].imag() == 0.0) ? V[0].real() : std::abs(V[0])) :
                                        vectorNorm(V));
            break;
        }
        case DataType::HELICS_JSON:
            valueExtract(readJsonValue(dv), val);
            break;
        case DataType::HELICS_CUSTOM:
            throw(std::invalid_argument("unrecognized helics type"));
    }
}

void valueExtract(const data_view& dv, DataType baseType, defV& val)
{
    if (baseType == DataType::HELICS_ANY || baseType == DataType::HELICS_UNKNOWN) {
        baseType = detail::detectType(dv.bytes());
    }
    switch (baseType) {
        case DataType::HELICS_DOUBLE:
            val = ValueConverter<double>::interpret(dv);
            break;
        case DataType::HELICS_INT:
        case DataType::HELICS_TIME:
            val = ValueConverter<int64_t>::interpret(dv);
            break;
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
        default:
            val = std::string(ValueConverter<std::string_view>::interpret(dv));
            break;
        case DataType::HELICS_VECTOR:
            val = ValueConverter<std::vector<double>>::interpret(dv);
            break;
        case DataType::HELICS_COMPLEX:
            val = ValueConverter<std::complex<double>>::interpret(dv);
            break;
        case DataType::HELICS_COMPLEX_VECTOR:
            val = ValueConverter<std::vector<std::complex<double>>>::interpret(dv);
            break;
        case DataType::HELICS_NAMED_POINT:
            val = ValueConverter<NamedPoint>::interpret(dv);
            break;
        case DataType::HELICS_JSON:
            val = readJsonValue(dv);
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
            double V{0};
            valueExtract(val, V);
            val = V;
            break;
        }
        case DataType::HELICS_INT: {
            if (index == int_loc) {
                return;
            }
            int64_t V{0};
            valueExtract(val, V);
            val = V;
            break;
        }
        case DataType::HELICS_TIME: {
            if (index == int_loc) {
                return;
            }
            Time V{timeZero};
            valueExtract(val, V);
            val = V.getBaseTimeCode();
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
            std::string V;
            valueExtract(val, V);
            val = std::move(V);
            break;
        }
        case DataType::HELICS_VECTOR: {
            if (index == vector_loc) {
                return;
            }
            std::vector<double> V;
            valueExtract(val, V);
            val = std::move(V);
            break;
        }
        case DataType::HELICS_COMPLEX: {
            if (index == complex_loc) {
                return;
            }
            std::complex<double> V;
            valueExtract(val, V);
            val = V;
            break;
        }
        case DataType::HELICS_COMPLEX_VECTOR: {
            if (index == complex_vector_loc) {
                return;
            }
            std::vector<std::complex<double>> V;
            valueExtract(val, V);
            val = std::move(V);
            break;
        }
        case DataType::HELICS_NAMED_POINT: {
            if (index == named_point_loc) {
                return;
            }
            NamedPoint V;
            valueExtract(val, V);
            val = std::move(V);
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
            return typeConvert(type, std::string_view(std::get<std::string>(val)));
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
                               std::string_view(std::get<std::string>(val)));
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
