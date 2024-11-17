/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "../core/core-data.hpp"
#include "helics_cxx_export.h"

#include <complex>
#include <limits>
#include <string>
#include <string_view>
#include <typeinfo>
#include <utility>
#include <vector>
/** @file
@details basic type information and control for HELICS
*/
namespace helics {
using IdentifierType = std::uint32_t;  //!< specify the underlying type used in the identifiers
using std::int32_t;
using std::int64_t;

constexpr IdentifierType invalid_id_value =
    static_cast<IdentifierType>(-1);  //!< defining an invalid id value

/** the known types of identifiers*/
enum class Identifiers : char {
    PUBLICATION = 'p',
    INPUT = 'i',
    FILTER = 'f',
    ENDPOINT = 'e',
    QUERY = 'q',
    TRANSLATORS = 't'
};

/** enumeration of locality namespaces*/
enum class InterfaceVisibility {
    LOCAL,
    GLOBAL,
};

/** class defining an  identifier type
@details  the intent of this class is to limit the operations available on an identifier
to those that are a actually required and make sense, and make it as low impact as possible.
it also acts to limit any mistakes of on type of identifier for another
*/

template<typename BaseType, Identifiers ID, BaseType invalidValue>
class IdentifierId {
  private:
    BaseType ivalue = invalidValue;  //!< the underlying index value

  public:
    static const Identifiers identity{ID};  //!< the type of the identifier
    using UnderlyingType = BaseType;
    /** default constructor*/
    constexpr IdentifierId() noexcept: ivalue(invalidValue) {}
    /** value based constructor*/
    constexpr explicit IdentifierId(BaseType val) noexcept: ivalue(val) {}
    /** copy constructor*/
    constexpr IdentifierId(const IdentifierId& id) noexcept: ivalue(id.ivalue) {}
    /** assignment from number*/
    IdentifierId& operator=(BaseType val) noexcept
    {
        ivalue = val;
        return *this;
    }
    /** copy assignment*/
    IdentifierId& operator=(const IdentifierId& id) = default;

    /** get the underlying value*/
    BaseType value() const noexcept { return ivalue; };
    /** equality operator*/
    bool operator==(IdentifierId id) const noexcept { return (ivalue == id.ivalue); }
    /** inequality operator*/
    bool operator!=(IdentifierId id) const noexcept { return (ivalue != id.ivalue); }
    /** less than operator for sorting*/
    bool operator<(IdentifierId id) const noexcept { return (ivalue < id.ivalue); }
    // check if the current value is not the invalidValue
    bool isValid() const noexcept { return (ivalue != invalidValue); }
};
}  // namespace helics

// specialize std::hash
namespace std {
/** custom hashing function for identifier types so they can be used in hash maps*/
template<typename BaseType, helics::Identifiers ID, BaseType invalidValue>
struct hash<helics::IdentifierId<BaseType, ID, invalidValue>> {
    using argument_type =
        helics::IdentifierId<BaseType, ID, invalidValue>;  //!< the type of object to hash
    using result_type = std::size_t;  //!< the result type of the hash code
    /** the actual hash operator*/
    result_type operator()(argument_type const& key) const noexcept
    {
        return std::hash<BaseType>{}(key.value());
    }
};
}  // namespace std

namespace helics {
using PublicationId = IdentifierId<IdentifierType, Identifiers::PUBLICATION, invalid_id_value>;
using InputId = IdentifierId<IdentifierType, Identifiers::INPUT, invalid_id_value>;

using QueryId = IdentifierId<IdentifierType, Identifiers::QUERY, invalid_id_value>;

/** data class for pair of a string and double*/
class NamedPoint {
  public:
    std::string name;  //!< the text value for the named point
    double value =
        std::numeric_limits<double>::quiet_NaN();  //!< the data value for the named point
    /** default constructor*/
    NamedPoint() = default;
    /** construct directly from name value*/
    NamedPoint(std::string_view valname, double valval): name(valname), value(valval) {}
    /** equality operator
    @details if either value is nan it check only the string
    otherwise it checks the name and value
    @return true if the objects are equivalent*/
    bool operator==(const NamedPoint& np) const
    {
        return ((std::isnan(value)) && (std::isnan(np.value))) ?
            (name == np.name) :
            ((value == np.value) && (name == np.name));
    }
    bool operator!=(const NamedPoint& np) const { return !operator==(np); }
    /** less than operator
    @details checks by name order, then value order
    */
    bool operator<(const NamedPoint& np) const
    {
        return (name == np.name) ? (name < np.name) : (value < np.value);
    }
    /** greater than operator
    @details checks by name order, then value order
    */
    bool operator>(const NamedPoint& np) const
    {
        return (name == np.name) ? (name > np.name) : (value > np.value);
    }
};

/** template class for generating a known name of a type*/
template<class X>
inline constexpr const char* typeNameString()
{
    // this will probably not be the same on all platforms
    return typeid(X).name();
}
namespace typestrings {
    constexpr auto svecstr = "string_vector";
    constexpr auto dvecstr = "double_vector";
    constexpr auto cvecstr = "complex_vector";

    constexpr auto doublestr = "double";
    constexpr auto floatstr = "float";
    constexpr auto boolstr = "bool";

    constexpr auto charstr = "char";
    constexpr auto ucharstr = "uchar";
    constexpr auto i32str = "int32";
    constexpr auto ui32str = "uint32";
    constexpr auto i64str = "int64";
    constexpr auto ui64str = "uint64";

    constexpr auto cfloatstr = "complex_f";
    constexpr auto cdoublestr = "complex";
    constexpr auto npstr = "named_point";
    constexpr auto strstr = "string";
}  // namespace typestrings

template<>
inline constexpr const char* typeNameString<std::vector<std::string>>()
{
    return typestrings::svecstr;
}
template<>
inline constexpr const char* typeNameString<std::vector<double>>()
{
    return typestrings::dvecstr;
}

template<>
inline constexpr const char* typeNameString<std::vector<std::complex<double>>>()
{
    return typestrings::cvecstr;
}

/** for float*/
template<>
inline constexpr const char* typeNameString<double>()
{
    return typestrings::doublestr;
}

/** for float*/
template<>
inline constexpr const char* typeNameString<float>()
{
    return typestrings::floatstr;
}

/** for boolean*/
template<>
inline constexpr const char* typeNameString<bool>()
{
    return typestrings::boolstr;
}

/** for character*/
template<>
inline constexpr const char* typeNameString<char>()
{
    return typestrings::charstr;
}
/** for unsigned character*/
template<>
inline constexpr const char* typeNameString<unsigned char>()
{
    return typestrings::ucharstr;
}
/** for integer*/
template<>
inline constexpr const char* typeNameString<std::int32_t>()
{
    return typestrings::i32str;
}
/** for unsigned integer*/
template<>
inline constexpr const char* typeNameString<std::uint32_t>()
{
    return typestrings::ui32str;
}
/** for 64 bit unsigned integer*/
template<>
inline constexpr const char* typeNameString<int64_t>()
{
    return typestrings::i64str;
}
/** for 64 bit unsigned integer*/
template<>
inline constexpr const char* typeNameString<std::uint64_t>()
{
    return typestrings::ui64str;
}
/** for complex double*/
template<>
inline constexpr const char* typeNameString<std::complex<float>>()
{
    return typestrings::cfloatstr;
}
/** for complex double*/
template<>
inline constexpr const char* typeNameString<std::complex<double>>()
{
    return typestrings::cdoublestr;
}
template<>
inline constexpr const char* typeNameString<std::string>()
{
    return typestrings::strstr;
}

template<>
inline constexpr const char* typeNameString<NamedPoint>()
{
    return typestrings::npstr;
}
/** the base types for  helics*/
enum class DataType : int {
    HELICS_UNKNOWN = HELICS_DATA_TYPE_UNKNOWN,
    HELICS_STRING = HELICS_DATA_TYPE_STRING,
    HELICS_DOUBLE = HELICS_DATA_TYPE_DOUBLE,
    HELICS_INT = HELICS_DATA_TYPE_INT,

    HELICS_COMPLEX = HELICS_DATA_TYPE_COMPLEX,
    HELICS_VECTOR = HELICS_DATA_TYPE_VECTOR,
    HELICS_COMPLEX_VECTOR = HELICS_DATA_TYPE_COMPLEX_VECTOR,
    HELICS_NAMED_POINT = HELICS_DATA_TYPE_NAMED_POINT,
    HELICS_BOOL = HELICS_DATA_TYPE_BOOLEAN,
    HELICS_TIME = HELICS_DATA_TYPE_TIME,
    HELICS_CHAR = HELICS_DATA_TYPE_CHAR,
    HELICS_CUSTOM = HELICS_DATA_TYPE_RAW,
    HELICS_ANY = HELICS_DATA_TYPE_ANY,
    HELICS_JSON = HELICS_DATA_TYPE_JSON,
    HELICS_MULTI = HELICS_DATA_TYPE_MULTI,
};

inline constexpr bool isBytesType(DataType type)
{
    return (type == DataType::HELICS_ANY) || (type == DataType::HELICS_CUSTOM);
}

/** sometimes we just need a ref to a string for the basic types*/
HELICS_CXX_EXPORT const std::string& typeNameStringRef(DataType type);

/** convert a string to a type*/
HELICS_CXX_EXPORT DataType getTypeFromString(std::string_view typeName);

/** convert to a string and also get a cleaned view of the string if it were a specific typeid
@details returns the input if not changed*/
HELICS_CXX_EXPORT std::string_view getCleanedTypeName(std::string_view typeName);

/** generate a string representation of a complex number from separate real and imaginary parts*/
HELICS_CXX_EXPORT std::string helicsComplexString(double real, double imag);
/** generate a string representation of a complex number */
HELICS_CXX_EXPORT std::string helicsComplexString(std::complex<double> val);
/** generate a string representation of a vector
@details string will look like v[1.02,45]*/
HELICS_CXX_EXPORT std::string helicsVectorString(const std::vector<double>& val);
/** generate a string representation of an int */
HELICS_CXX_EXPORT std::string helicsIntString(std::int64_t val);
/** generate a string representation of a double */
HELICS_CXX_EXPORT std::string helicsDoubleString(double val);
/** generate a string representation of a vector from pointer and size
@details string will look like v[1.02,45]*/
HELICS_CXX_EXPORT std::string helicsVectorString(const double* vals, size_t size);
/** generate a string representation of a complex vector
@details string will look like cv[1.02+2j,45]*/
HELICS_CXX_EXPORT std::string
    helicsComplexVectorString(const std::vector<std::complex<double>>& val);
/** generate a named point string
@details string will look like {"<name>":val}
*/
HELICS_CXX_EXPORT std::string helicsNamedPointString(const NamedPoint& point);
HELICS_CXX_EXPORT std::string helicsNamedPointString(std::string_view pointName, double val);
/** convert a string to a complex number*/
HELICS_CXX_EXPORT std::complex<double> helicsGetComplex(std::string_view val);
/** convert a string to a vector*/
HELICS_CXX_EXPORT std::vector<double> helicsGetVector(std::string_view val);
HELICS_CXX_EXPORT void helicsGetVector(std::string_view val, std::vector<double>& data);

/** convert a string to a complex vector*/
HELICS_CXX_EXPORT std::vector<std::complex<double>> helicsGetComplexVector(std::string_view val);

/** convert a string to a complex vector using an existing vector*/
HELICS_CXX_EXPORT void helicsGetComplexVector(std::string_view val,
                                              std::vector<std::complex<double>>& data);

/** convert a string to a named point*/
HELICS_CXX_EXPORT NamedPoint helicsGetNamedPoint(std::string_view val);
/** get an int from a string*/
HELICS_CXX_EXPORT std::int64_t getIntFromString(std::string_view val);
/** get a double from a string*/
HELICS_CXX_EXPORT double getDoubleFromString(std::string_view val);
/** get a complex number from a string*/
HELICS_CXX_EXPORT std::complex<double> getComplexFromString(std::string_view val);
/** get the boolean value of a string*/
HELICS_CXX_EXPORT bool helicsBoolValue(std::string_view val);
/** compute the L2 norm of a vector*/
HELICS_CXX_EXPORT double vectorNorm(const std::vector<double>& vec);
/** compute the L2 norm of a magnitudes of a complex vector*/
HELICS_CXX_EXPORT double vectorNorm(const std::vector<std::complex<double>>& vec);
/** convert a value to a data block to be interpreted using the specified type
@param type the type used for the data conversion
@param val a double to convert
*/
HELICS_CXX_EXPORT SmallBuffer typeConvert(DataType type, double val);
HELICS_CXX_EXPORT SmallBuffer typeConvert(DataType type, int64_t val);
HELICS_CXX_EXPORT SmallBuffer typeConvert(DataType type, std::string_view val);
HELICS_CXX_EXPORT SmallBuffer typeConvert(DataType type, const std::vector<double>& val);
HELICS_CXX_EXPORT SmallBuffer typeConvert(DataType type, const double* vals, size_t size);
HELICS_CXX_EXPORT SmallBuffer typeConvertComplex(DataType type, const double* vals, size_t size);
HELICS_CXX_EXPORT SmallBuffer typeConvert(DataType type,
                                          const std::vector<std::complex<double>>& val);
HELICS_CXX_EXPORT SmallBuffer typeConvert(DataType type, const std::complex<double>& val);
HELICS_CXX_EXPORT SmallBuffer typeConvert(DataType type, const NamedPoint& val);
HELICS_CXX_EXPORT SmallBuffer typeConvert(DataType type, std::string_view str, double val);
HELICS_CXX_EXPORT SmallBuffer typeConvert(DataType type, bool val);
HELICS_CXX_EXPORT SmallBuffer typeConvert(DataType type, char val);
HELICS_CXX_EXPORT SmallBuffer typeConvert(DataType type, Time val);

/** template function for generating a known name of a type*/
template<class X>
constexpr DataType helicsType()
{
    return DataType::HELICS_CUSTOM;
}

template<>
constexpr DataType helicsType<int64_t>()
{
    return DataType::HELICS_INT;
}

template<>
constexpr DataType helicsType<bool>()
{
    return DataType::HELICS_BOOL;
}

template<>
constexpr DataType helicsType<std::string>()
{
    return DataType::HELICS_STRING;
}

template<>
constexpr DataType helicsType<NamedPoint>()
{
    return DataType::HELICS_NAMED_POINT;
}
template<>
constexpr DataType helicsType<double>()
{
    return DataType::HELICS_DOUBLE;
}

template<>
constexpr DataType helicsType<Time>()
{
    return DataType::HELICS_TIME;
}

template<>
constexpr DataType helicsType<std::complex<double>>()
{
    return DataType::HELICS_COMPLEX;
}

template<>
constexpr DataType helicsType<std::vector<double>>()
{
    return DataType::HELICS_VECTOR;
}

template<>
constexpr DataType helicsType<std::vector<std::complex<double>>>()
{
    return DataType::HELICS_COMPLEX_VECTOR;
}

// check if the type is directly convertible to a base HelicsType
template<class X>
constexpr bool isConvertableType()
{
    return false;
}

template<>
constexpr bool isConvertableType<float>()
{
    return true;
}

template<>
constexpr bool isConvertableType<long double>()
{
    return true;
}

template<>
constexpr bool isConvertableType<int32_t>()
{
    return true;
}

template<>
constexpr bool isConvertableType<int16_t>()
{
    return true;
}

template<>
constexpr bool isConvertableType<uint16_t>()
{
    return true;
}

template<>
constexpr bool isConvertableType<char>()
{
    return true;
}

template<>
constexpr bool isConvertableType<unsigned char>()
{
    return true;
}

template<>
constexpr bool isConvertableType<uint64_t>()
{
    return true;
}

/** generate an invalid value for the various types*/
template<class X>
inline X invalidValue()
{
    return X();
}

/// defined constant for an invalid value as a double
constexpr double invalidDouble = HELICS_INVALID_DOUBLE;

template<>
constexpr double invalidValue<double>()
{
    return invalidDouble;
}

template<>
constexpr int64_t invalidValue<int64_t>()
{
    return (std::numeric_limits<int64_t>::min)();
}

template<>
constexpr uint64_t invalidValue<uint64_t>()
{
    return (std::numeric_limits<uint64_t>::max)();
}

template<>
constexpr Time invalidValue<Time>()
{
    return Time::minVal();
}

template<>
inline NamedPoint invalidValue<NamedPoint>()
{
    return {std::string(), std::nan("0")};
}

template<>
constexpr std::complex<double> invalidValue<std::complex<double>>()
{
    return {invalidValue<double>(), 0.0};
}
/// Helper template to remove const volatile references
template<typename T>
using remove_cv_ref = std::remove_cv_t<std::remove_reference_t<T>>;

constexpr int primaryType = 0;
constexpr int convertibleType = 1;
constexpr int nonConvertibleType = 2;
/** template dividing types into 3 categories
0 is primary types
1 types convertible to primary types
2 type not convertible to primary types */
template<typename X>
using typeCategory =
    std::conditional_t<helicsType<remove_cv_ref<X>>() != DataType::HELICS_CUSTOM,
                       std::integral_constant<int, primaryType>,
                       std::conditional_t<isConvertableType<remove_cv_ref<X>>(),
                                          std::integral_constant<int, convertibleType>,
                                          std::integral_constant<int, nonConvertibleType>>>;
}  // namespace helics
