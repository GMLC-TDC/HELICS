/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "ValueConverter.hpp"
#include "helics/helics-config.h"
#include "helicsTypes.hpp"
#include "helics_cxx_export.h"

#include <cmath>
#include <complex>
#include <cstdint>
#include <string>
#include <utility>
#include <variant>
#include <vector>
/** @file
@brief naming a set of types that are interchangeable and recognizable inside the HELICS application
API and core
*/
namespace helics {

/** detect if a string is not a string that represents false*/
HELICS_CXX_EXPORT bool isTrueString(std::string_view str);

/** define a variant with the different types*/
using defV = std::variant<double,
                          int64_t,
                          std::string,
                          std::complex<double>,
                          std::vector<double>,
                          std::vector<std::complex<double>>,
                          NamedPoint>;

/**enumeration of the index value of the types inside the defV variant*/
enum TypeLocation : decltype(std::declval<defV>().index()) {
    double_loc = 0U,
    int_loc = 1U,
    string_loc = 2U,
    complex_loc = 3U,
    vector_loc = 4U,
    complex_vector_loc = 5U,
    named_point_loc = 6U
};
/** detect a change from the previous values*/
HELICS_CXX_EXPORT bool changeDetected(const defV& prevValue, const std::string& val, double deltaV);
HELICS_CXX_EXPORT bool changeDetected(const defV& prevValue, std::string_view val, double deltaV);
HELICS_CXX_EXPORT bool
    changeDetected(const defV& prevValue, const std::vector<double>& val, double deltaV);
HELICS_CXX_EXPORT bool changeDetected(const defV& prevValue,
                                      const std::vector<std::complex<double>>& val,
                                      double deltaV);
HELICS_CXX_EXPORT bool
    changeDetected(const defV& prevValue, const double* vals, size_t size, double deltaV);
HELICS_CXX_EXPORT bool
    changeDetected(const defV& prevValue, const std::complex<double>& val, double deltaV);
HELICS_CXX_EXPORT bool changeDetected(const defV& prevValue, double val, double deltaV);
HELICS_CXX_EXPORT bool changeDetected(const defV& prevValue, int64_t val, double deltaV);
HELICS_CXX_EXPORT bool changeDetected(const defV& prevValue, Time val, double deltaV);
HELICS_CXX_EXPORT bool changeDetected(const defV& prevValue, const NamedPoint& val, double deltaV);
HELICS_CXX_EXPORT bool changeDetected(const defV& prevValue, bool val, double deltaV);

/** directly convert the boolean to integer*/
inline int64_t make_valid(bool obj)
{
    return (obj) ? 1LL : 0LL;
}

/** directly convert the boolean to integer*/
inline int64_t make_valid(uint64_t val)
{
    return static_cast<int64_t>(val);
}
inline int64_t make_valid(int16_t val)
{
    return static_cast<int64_t>(val);
}
inline int64_t make_valid(uint16_t val)
{
    return static_cast<int64_t>(val);
}
inline int64_t make_valid(char val)
{
    return static_cast<int64_t>(val);
}
inline int64_t make_valid(unsigned char val)
{
    return static_cast<int64_t>(val);
}
inline int64_t make_valid(int32_t val)
{
    return static_cast<int64_t>(val);
}
inline int64_t make_valid(uint32_t val)
{
    return static_cast<int64_t>(val);
}
inline int64_t make_valid(Time val)
{
    return val.getBaseTimeCode();
}

inline double make_valid(float val)
{
    return static_cast<double>(val);
}

inline std::complex<double> make_valid(const std::complex<float>& val)
{
    return {val.real(), val.imag()};
}

/** this template should do nothing for most classes the specific overloads are the important ones*/
template<class X>
decltype(auto) make_valid(X&& obj)
{
    return std::forward<X>(obj);
}
/** extract the value from a variant to a string*/
HELICS_CXX_EXPORT void valueExtract(const defV& data, std::string& val);

/** extract the value from a variant to a complex number*/
HELICS_CXX_EXPORT void valueExtract(const defV& data, std::complex<double>& val);

/** extract the value from a variant to a vector of doubles*/
HELICS_CXX_EXPORT void valueExtract(const defV& data, std::vector<double>& val);

/** extract the value from a variant to a complex vector of doubles*/
HELICS_CXX_EXPORT void valueExtract(const defV& data, std::vector<std::complex<double>>& val);

/** extract the value from a variant to a named point*/
HELICS_CXX_EXPORT void valueExtract(const defV& data, NamedPoint& val);

/** extract the value from a variant to a named point*/
HELICS_CXX_EXPORT void valueExtract(const defV& data, Time& val);

/** extract the value from a variant to a character*/
HELICS_CXX_EXPORT void valueExtract(const defV& data, char& val);

/** extract the value from a variant to a bool*/
HELICS_CXX_EXPORT void valueExtract(const defV& data, bool& val);

HELICS_CXX_EXPORT defV readJsonValue(const data_view& data);

HELICS_CXX_EXPORT void valueExtract(const data_view& data, DataType baseType, std::string& val);

HELICS_CXX_EXPORT void
    valueExtract(const data_view& data, DataType baseType, std::vector<double>& val);

HELICS_CXX_EXPORT void
    valueExtract(const data_view& data, DataType baseType, std::complex<double>& val);

HELICS_CXX_EXPORT void
    valueExtract(const data_view& data, DataType baseType, std::vector<std::complex<double>>& val);

HELICS_CXX_EXPORT void valueExtract(const data_view& data, DataType baseType, NamedPoint& val);

HELICS_CXX_EXPORT void valueExtract(const data_view& data, DataType baseType, Time& val);

HELICS_CXX_EXPORT void valueExtract(const data_view& data, DataType baseType, bool& val);

HELICS_CXX_EXPORT void valueExtract(const data_view& data, DataType baseType, defV& val);

HELICS_CXX_EXPORT void valueExtract(const data_view& data, DataType baseType, char& val);

/** extract the value from a variant to a numerical type*/
template<class X>
std::enable_if_t<std::is_arithmetic<X>::value && (!std::is_same<X, char>::value)>
    valueExtract(const defV& data, X& val)
{
    switch (data.index()) {
        case double_loc:  // double
            val = static_cast<X>(std::get<double>(data));
            break;
        case int_loc:  // int64_t
            val = static_cast<X>(std::get<int64_t>(data));
            break;
        case string_loc:  // string
        default: {
            const auto& v = std::get<std::string>(data);
            if (v.find_first_of(".eE[]") == std::string::npos) {
                val = static_cast<X>(getIntFromString(v));
            } else {
                val = static_cast<X>(getDoubleFromString(v));
            }
        }

        break;
        case complex_loc:  // complex
        {
            auto& cv = std::get<std::complex<double>>(data);
            val = static_cast<X>((cv.imag() != 0.0) ? std::abs(cv) : cv.real());
        }

        break;
        case vector_loc:  // vector
        {
            const auto& vec = std::get<std::vector<double>>(data);
            val = static_cast<X>((vec.size() == 1) ? vec[0] : vectorNorm(vec));
            break;
        }
        case complex_vector_loc:  // complex vector
        {
            const auto& vec = std::get<std::vector<std::complex<double>>>(data);
            double vald{0.0};
            if (vec.size() == 1) {
                if (vec[0].imag() == 0.0) {
                    vald = vec[0].real();
                } else {
                    vald = std::abs(vec[0]);
                }
            } else {
                vald = vectorNorm(vec);
            }
            val = static_cast<X>(vald);
            break;
        }
        case named_point_loc: {
            const auto& np = std::get<NamedPoint>(data);
            if (std::isnan(np.value)) {
                if (np.name.find_first_of(".eE[]") == std::string::npos) {
                    val = static_cast<X>(getIntFromString(np.name));
                } else {
                    val = static_cast<X>(getDoubleFromString(np.name));
                }
            } else {
                val = static_cast<X>(np.value);
            }
            break;
        }
    }
}

/** assume it is some numeric type (int or double)*/
template<class X>
std::enable_if_t<std::is_arithmetic<X>::value && (!std::is_same<X, char>::value)>
    valueExtract(const data_view& data, DataType baseType, X& val)
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
            const auto v = ValueConverter<std::string_view>::interpret(data);
            if (v.find_first_of(".eE[]") == std::string_view::npos) {
                val = static_cast<X>(getIntFromString(v));
            } else {
                val = static_cast<X>(getDoubleFromString(v));
            }
        }

        break;
        case DataType::HELICS_BOOL:
            val = static_cast<X>((ValueConverter<std::string_view>::interpret(data) != "0"));
            break;
        case DataType::HELICS_NAMED_POINT: {
            auto npval = ValueConverter<NamedPoint>::interpret(data);
            if (std::isnan(npval.value)) {
                try {
                    if (npval.name.find_first_of(".eE[]") == std::string::npos) {
                        val = static_cast<X>(getIntFromString(npval.name));
                    } else {
                        val = static_cast<X>(getDoubleFromString(npval.name));
                    }
                }
                catch (const std::invalid_argument&) {
                    val = static_cast<X>(
                        invalidValue<
                            std::conditional_t<std::is_integral<X>::value, int64_t, double>>());
                }
            } else {
                val = static_cast<X>(npval.value);
            }

            break;
        }
        case DataType::HELICS_DOUBLE: {
            auto V = ValueConverter<double>::interpret(data);
            val = static_cast<X>(V);
            break;
        }
        case DataType::HELICS_INT: {
            auto V = ValueConverter<int64_t>::interpret(data);
            val = static_cast<X>(V);
            break;
        }
        case DataType::HELICS_TIME: {
            Time vtime;
            vtime.setBaseTimeCode(ValueConverter<int64_t>::interpret(data));
            val = std::is_integral<X>::value ? static_cast<X>(vtime.getBaseTimeCode()) :
                                               static_cast<X>(static_cast<double>(vtime));
            break;
        }
        case DataType::HELICS_VECTOR: {
            auto V = ValueConverter<std::vector<double>>::interpret(data);
            if (V.size() == 1) {
                val = static_cast<X>(V[0]);
            } else {
                val = static_cast<X>(vectorNorm(V));
            }
            break;
        }
        case DataType::HELICS_COMPLEX: {
            auto V = ValueConverter<std::complex<double>>::interpret(data);

            val = static_cast<X>((V.imag() != 0) ? std::abs(V) : V.real());
            break;
        }
        case DataType::HELICS_COMPLEX_VECTOR: {
            auto V = ValueConverter<std::vector<std::complex<double>>>::interpret(data);
            if (V.size() == 1) {
                if (V[0].imag() == 0) {
                    val = static_cast<X>(V[0].real());
                } else {
                    val = static_cast<X>(std::abs(V[0]));
                }
            } else {
                val = static_cast<X>(vectorNorm(V));
            }

            break;
        }
        case DataType::HELICS_JSON:
            valueExtract(readJsonValue(data), val);
            break;
        case DataType::HELICS_CUSTOM:
            throw(std::invalid_argument("unrecognized helics type"));
    }
}

HELICS_CXX_EXPORT void valueConvert(defV& val, DataType newType);

HELICS_CXX_EXPORT SmallBuffer typeConvertDefV(DataType type, const defV& val);
HELICS_CXX_EXPORT SmallBuffer typeConvertDefV(const defV& val);
}  // namespace helics
