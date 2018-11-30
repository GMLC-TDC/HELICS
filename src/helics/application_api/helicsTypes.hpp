/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once
#include <complex>
#include <cstdint>
#include <limits>
#include <string>
#include <typeinfo>
#include <vector>

#include "../core/core-data.hpp"
/** @file
@details basic type information and control for HELICS
*/
namespace helics
{
using identifier_type = uint32_t;

constexpr identifier_type invalid_id_value = static_cast<identifier_type> (-1);  //!< defining an invalid id value

/** the known types of identifiers*/
enum class identifiers : char
{
    publication,
    input,
    filter,
    endpoint,
    query,
};

/** enumeration of locality namespaces*/
enum class interface_visibility
{
    local,
    global,
};

constexpr interface_visibility GLOBAL = interface_visibility::global;
constexpr interface_visibility LOCAL = interface_visibility::local;

/** class defining an  identifier type
@details  the intent of this class is to limit the operations available on an identifier
to those that are a actually required and make sense, and make it as low impact as possible.
it also acts to limit any mistakes of on type of identifier for another
*/
template <typename BaseType, identifiers ID, BaseType invalidValue>
class identifier_id_t
{
  private:
    BaseType ivalue = invalidValue;  //!< the underlying index value

  public:
    static const identifiers identity{ID};  //!< the type of the identifier
    using underlyingType = BaseType;
    /** default constructor*/
    constexpr identifier_id_t () noexcept : ivalue (invalidValue){};
    /** value based constructor*/
    constexpr explicit identifier_id_t (BaseType val) noexcept : ivalue (val){};
    /** copy constructor*/
    constexpr identifier_id_t (const identifier_id_t &id) noexcept : ivalue (id.ivalue){};
    /** assignment from number*/
    identifier_id_t &operator= (BaseType val) noexcept
    {
        ivalue = val;
        return *this;
    };
    /** copy assignment*/
    identifier_id_t &operator= (const identifier_id_t &id) noexcept
    {
        ivalue = id.ivalue;
        return *this;
    };

    /** get the underlying value*/
    BaseType value () const noexcept { return ivalue; };
    /** equality operator*/
    bool operator== (identifier_id_t id) const noexcept { return (ivalue == id.ivalue); };
    /** inequality operator*/
    bool operator!= (identifier_id_t id) const noexcept { return (ivalue != id.ivalue); };
    /** less than operator for sorting*/
    bool operator< (identifier_id_t id) const noexcept { return (ivalue < id.ivalue); };
    // check if the current value is not the invalidValue
    bool isValid () const noexcept { return (ivalue != invalidValue); };
};
}  // namespace helics

// specialize std::hash
namespace std
{
template <typename BaseType, helics::identifiers ID, BaseType invalidValue>
struct hash<helics::identifier_id_t<BaseType, ID, invalidValue>>
{
    using argument_type = helics::identifier_id_t<BaseType, ID, invalidValue>;
    using result_type = std::size_t;
    result_type operator() (argument_type const &key) const noexcept
    {
        return std::hash<BaseType>{}(key.value ());
    }
};
}  // namespace std

namespace helics
{
using publication_id_t = identifier_id_t<identifier_type, identifiers::publication, invalid_id_value>;
using input_id_t = identifier_id_t<identifier_type, identifiers::input, invalid_id_value>;
//using endpoint_id_t = identifier_id_t<identifier_type, identifiers::endpoint, invalid_id_value>;
//using filter_id_t = identifier_id_t<identifier_type, identifiers::filter, invalid_id_value>;
using query_id_t = identifier_id_t<identifier_type, identifiers::query, invalid_id_value>;

/** data class for pair of a string and double*/
class named_point
{
  public:
    std::string name;  //!< the text value for the named point
    double value;  //!< the data value for the named point
    named_point () = default;
    named_point (std::string valname, double valval) : name (std::move (valname)), value (valval) {}
    bool operator== (const named_point &opt) const
    {
        return ((std::isnan (value)) && (std::isnan (opt.value))) ? (name == opt.name) :
                                                                    ((value == opt.value) && (name == opt.name));
    }
    bool operator!= (const named_point &opt) const { return !operator== (opt); }
    bool operator< (const named_point &opt) const
    {
        return (name == opt.name) ? (name < opt.name) : (value < opt.value);
    }
};

/** template class for generating a known name of a type*/
template <class X>
inline constexpr const char *typeNameString ()
{
    // this will probably not be the same on all platforms
    return typeid (X).name ();
}
namespace typestrings
{
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
}

template <>
inline constexpr const char *typeNameString<std::vector<std::string>> ()
{
    return typestrings::svecstr;
}
template <>
inline constexpr const char *typeNameString<std::vector<double>> ()
{
    return typestrings::dvecstr;
}

template <>
inline constexpr const char *typeNameString<std::vector<std::complex<double>>> ()
{
    return typestrings::cvecstr;
}

/** for float*/
template <>
inline constexpr const char *typeNameString<double> ()
{
    return typestrings::doublestr;
}

/** for float*/
template <>
inline constexpr const char *typeNameString<float> ()
{
    return typestrings::floatstr;
}

/** for boolean*/
template <>
inline constexpr const char *typeNameString<bool> ()
{
    return typestrings::boolstr;
}

/** for character*/
template <>
inline constexpr const char *typeNameString<char> ()
{
    return typestrings::charstr;
}
/** for unsigned character*/
template <>
inline constexpr const char *typeNameString<unsigned char> ()
{
    return typestrings::ucharstr;
}
/** for integer*/
template <>
inline constexpr const char *typeNameString<std::int32_t> ()
{
    return typestrings::i32str;
}
/** for unsigned integer*/
template <>
inline constexpr const char *typeNameString<std::uint32_t> ()
{
    return typestrings::ui32str;
}
/** for 64 bit unsigned integer*/
template <>
inline constexpr const char *typeNameString<int64_t> ()
{
    return typestrings::i64str;
    ;
}
/** for 64 bit unsigned integer*/
template <>
inline constexpr const char *typeNameString<std::uint64_t> ()
{
    return typestrings::ui64str;
}
/** for complex double*/
template <>
inline constexpr const char *typeNameString<std::complex<float>> ()
{
    return typestrings::cfloatstr;
}
/** for complex double*/
template <>
inline constexpr const char *typeNameString<std::complex<double>> ()
{
    return typestrings::cdoublestr;
}
template <>
inline constexpr const char *typeNameString<std::string> ()
{
    return typestrings::strstr;
}

template <>
inline constexpr const char *typeNameString<named_point> ()
{
    return typestrings::npstr;
}
/** the base types for  helics*/
enum class data_type : int
{
    helicsString = helics_data_type_string,
    helicsDouble = helics_data_type_double,
    helicsInt = helics_data_type_int,

    helicsComplex = helics_data_type_complex,
    helicsVector = helics_data_type_vector,
    helicsComplexVector = helics_data_type_complex_vector,
    helicsNamedPoint = helics_data_type_named_point,
    helicsBool = helics_data_type_boolean,
	helicsTime = helics_data_type_time,
    helicsCustom = helics_data_type_raw,
    helicsAny = helics_data_type_any,
	helicsUnknown=262355,
};


inline constexpr bool isRawType (data_type type)
{
    return (type == data_type::helicsAny) || (type == data_type::helicsCustom);
}

/** sometimes we just need a ref to a string for the basic types*/
const std::string &typeNameStringRef (data_type type);

/** convert a string to a type*/
data_type getTypeFromString (const std::string &typeName);

/** generate a string representation of a complex number from separate real and imaginary parts*/
std::string helicsComplexString (double real, double imag);
/** generate a string representation of a complex number */
std::string helicsComplexString (std::complex<double> val);
/** generate a string representation of a vector
@details string will look like v[1.02,45]*/
std::string helicsVectorString (const std::vector<double> &val);
/** generate a string representation of a vector from pointer and size
@details string will look like v[1.02,45]*/
std::string helicsVectorString (const double *vals, size_t size);
/** generate a string representation of a complex vector
@details string will look like cv[1.02+2j,45]*/
std::string helicsComplexVectorString (const std::vector<std::complex<double>> &val);
/** generate a named point string
@details string will look like {"<name>":val}
*/
std::string helicsNamedPointString (const named_point &point);
std::string helicsNamedPointString (const std::string &pointName, double val);
std::string helicsNamedPointString (const char *pointName, double val);
/** convert a string to a complex number*/
std::complex<double> helicsGetComplex (const std::string &val);
/** convert a string to a vector*/
std::vector<double> helicsGetVector (const std::string &val);
void helicsGetVector (const std::string &val, std::vector<double> &data);

/** convert a string to a complex vector*/
std::vector<std::complex<double>> helicsGetComplexVector (const std::string &val);

/** convert a string to a complex vector using an existing vector*/
void helicsGetComplexVector (const std::string &val, std::vector<std::complex<double>> &data);

/** convert a string to a named point*/
named_point helicsGetNamedPoint (const std::string &val);
/** get a double from a string*/
double getDoubleFromString (const std::string &val);
/** get a complex number from a string*/
std::complex<double> getComplexFromString (const std::string &val);
/** compute the L2 norm of a vector*/
double vectorNorm (const std::vector<double> &vec);
/** compute the L2 norm of a magnitudes of a complex vector*/
double vectorNorm (const std::vector<std::complex<double>> &vec);
/** convert a value to a data block to be interpreted using the specified type
@param type the type used for the data conversion
@param val a double to convert
*/
data_block typeConvert (data_type type, double val);
data_block typeConvert (data_type type, int64_t val);
data_block typeConvert (data_type type, const char *val);
data_block typeConvert (data_type type, const std::string &val);
data_block typeConvert (data_type type, const std::vector<double> &val);
data_block typeConvert (data_type type, const double *vals, size_t size);
data_block typeConvert (data_type type, const std::vector<std::complex<double>> &val);
data_block typeConvert (data_type type, const std::complex<double> &val);
data_block typeConvert (data_type type, const named_point &val);
data_block typeConvert (data_type type, const char *str, double val);
data_block typeConvert (data_type type, const std::string &str, double val);
data_block typeConvert (data_type type, bool val);

/** template function for generating a known name of a type*/
template <class X>
constexpr data_type helicsType ()
{
    return data_type::helicsCustom;
}

template <>
constexpr data_type helicsType<int64_t> ()
{
    return data_type::helicsInt;
}

template <>
constexpr data_type helicsType<bool> ()
{
    return data_type::helicsBool;
}

template <>
constexpr data_type helicsType<std::string> ()
{
    return data_type::helicsString;
}

template <>
constexpr data_type helicsType<named_point> ()
{
    return data_type::helicsNamedPoint;
}
template <>
constexpr data_type helicsType<double> ()
{
    return data_type::helicsDouble;
}

template <>
constexpr data_type helicsType<Time> ()
{
    return data_type::helicsTime;
}

template <>
constexpr data_type helicsType<std::complex<double>> ()
{
    return data_type::helicsComplex;
}

template <>
constexpr data_type helicsType<std::vector<double>> ()
{
    return data_type::helicsVector;
}

template <>
constexpr data_type helicsType<std::vector<std::complex<double>>> ()
{
    return data_type::helicsComplexVector;
}

// check if the type is directly convertible to a base HelicsType
template <class X>
constexpr bool isConvertableType ()
{
    return false;
}

template <>
constexpr bool isConvertableType<float> ()
{
    return true;
}

template <>
constexpr bool isConvertableType<long double> ()
{
    return true;
}

template <>
constexpr bool isConvertableType<int32_t> ()
{
    return true;
}

template <>
constexpr bool isConvertableType<int16_t> ()
{
    return true;
}

template <>
constexpr bool isConvertableType<uint16_t> ()
{
    return true;
}

template <>
constexpr bool isConvertableType<char> ()
{
    return true;
}

template <>
constexpr bool isConvertableType<unsigned char> ()
{
    return true;
}

template <>
constexpr bool isConvertableType<uint64_t> ()
{
    return true;
}

/** generate an invalid value for the various types*/
template <class X>
inline X invalidValue ()
{
    return X ();
}

template <>
constexpr double invalidValue<double> ()
{
    return -1e48;
}

template <>
constexpr int64_t invalidValue<int64_t> ()
{
    return std::numeric_limits<int64_t>::min ();
}

template <>
constexpr uint64_t invalidValue<uint64_t> ()
{
    return std::numeric_limits<uint64_t>::max ();
}

template <>
constexpr Time invalidValue<Time> ()
{
    return Time::minVal();
}

template <>
inline named_point invalidValue<named_point> ()
{
    return {std::string (), std::nan ("0")};
}

template <>
constexpr std::complex<double> invalidValue<std::complex<double>> ()
{
    return {invalidValue<double> (), 0.0};
}

template <typename T>
using remove_cv_ref = std::remove_cv_t<std::remove_reference_t<T>>;

constexpr int primaryType = 0;
constexpr int convertibleType = 1;
constexpr int nonConvertibleType = 2;
/** template dividing types into 3 categories
0 is primary types
1 types convertible to primary types
2 type not convertible to primary types */
template <typename X>
using typeCategory = std::conditional_t < helicsType<remove_cv_ref<X>> () != data_type::helicsCustom,
                                                  std::integral_constant<int, primaryType>,
                                                  std::conditional_t<isConvertableType<remove_cv_ref<X>>(),
                                                                     std::integral_constant<int, convertibleType>,
                                                                     std::integral_constant<int, nonConvertibleType>>>;
}  // namespace helics
