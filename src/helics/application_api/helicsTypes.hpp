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
    subscription,
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

/** enumeration of a whether a publication is required or is optional*/
enum class interface_availability
{
    required,  //!< the matching interface is required to exist
    optional  //!< the matching interface is NOT required
};

constexpr interface_availability OPTIONAL = interface_availability::optional;
constexpr interface_availability REQUIRED = interface_availability::required;

/** class defining an  identifier type
@details  the intent of this class is to limit the operations available on a publication identifier
to those that are a actually required and make sense, and make it as low impact as possible.
it also acts to limit any mistakes of a publication_id for a subscription_id or other types of interfaces
*/
template <class BaseType, identifiers ID, BaseType invalidValue = (BaseType (-1))>
class identifier_id_t
{
  private:
    BaseType _value;  //!< the underlying index value

  public:
    static const identifiers identity{ID};  //!< the type of the identifier
    using underlyingType = BaseType;
    /** default constructor*/
    constexpr identifier_id_t () noexcept : _value (invalidValue){};
    /** value based constructor*/
    // cppcheck-suppress noExplicitConstructor
    /* implicit */ constexpr identifier_id_t (BaseType val) noexcept : _value (val){};
    /** copy constructor*/
    constexpr identifier_id_t (const identifier_id_t &id) noexcept : _value (id._value){};
    /** assignment from number*/
    identifier_id_t &operator= (BaseType val) noexcept
    {
        _value = val;
        return *this;
    };
    /** copy assignment*/
    identifier_id_t &operator= (const identifier_id_t &id) noexcept
    {
        _value = id._value;
        return *this;
    };

    /** get the underlying value*/
    BaseType value () const noexcept { return _value; };
    /** equality operator*/
    bool operator== (identifier_id_t id) const noexcept { return (_value == id._value); };
    /** inequality operator*/
    bool operator!= (identifier_id_t id) const noexcept { return (_value != id._value); };
    /** less than operator for sorting*/
    bool operator< (identifier_id_t id) const noexcept { return (_value < id._value); };
};

using publication_id_t = identifier_id_t<identifier_type, identifiers::publication, invalid_id_value>;
using subscription_id_t = identifier_id_t<identifier_type, identifiers::subscription, invalid_id_value>;
using endpoint_id_t = identifier_id_t<identifier_type, identifiers::endpoint, invalid_id_value>;
using filter_id_t = identifier_id_t<identifier_type, identifiers::filter, invalid_id_value>;
using query_id_t = identifier_id_t<identifier_type, identifiers::query, invalid_id_value>;

/** data class for pair of a string and double*/
class named_point
{
  public:
    std::string name;
    double value;
    named_point () = default;
    named_point (std::string valname, double valval) : name (std::move (valname)), value (valval) {}
    bool operator== (const named_point &opt) const
    {
        return ((std::isnan (value)) && (std::isnan (opt.value))) ? (name == opt.name) :
                                                                    ((value == opt.value) && (name == opt.name));
    }
    bool operator!= (const named_point &opt) const { return !operator== (opt); }
};

/** template class for generating a known name of a type*/
template <class X>
inline std::string typeNameString ()
{
    // this will probably not be the same on all platforms
    return std::string (typeid (X).name ());
}
template <>
inline std::string typeNameString<std::vector<std::string>> ()
{
    return "string_vector";
}
template <>
inline std::string typeNameString<std::vector<double>> ()
{
    return "double_vector";
}

template <>
inline std::string typeNameString<std::vector<std::complex<double>>> ()
{
    return "complex_vector";
}

/** for float*/
template <>
inline std::string typeNameString<double> ()
{
    return "double";
}

/** for float*/
template <>
inline std::string typeNameString<float> ()
{
    return "float";
}

/** for boolean*/
template <>
inline std::string typeNameString<bool> ()
{
    return "bool";
}

/** for character*/
template <>
inline std::string typeNameString<char> ()
{
    return "char";
}
/** for unsigned character*/
template <>
inline std::string typeNameString<unsigned char> ()
{
    return "uchar";
}
/** for integer*/
template <>
inline std::string typeNameString<std::int32_t> ()
{
    return "int32";
}
/** for unsigned integer*/
template <>
inline std::string typeNameString<std::uint32_t> ()
{
    return "uint32";
}
/** for 64 bit unsigned integer*/
template <>
inline std::string typeNameString<int64_t> ()
{
    return "int64";
}
/** for 64 bit unsigned integer*/
template <>
inline std::string typeNameString<std::uint64_t> ()
{
    return "uint64";
}
/** for complex double*/
template <>
inline std::string typeNameString<std::complex<float>> ()
{
    return "complex_f";
}
/** for complex double*/
template <>
inline std::string typeNameString<std::complex<double>> ()
{
    return "complex";
}
template <>
inline std::string typeNameString<std::string> ()
{
    return "string";
}

template <>
inline std::string typeNameString<named_point> ()
{
    return "named_point";
}
/** the base types for  helics*/
enum class helics_type_t : int
{
    helicsString = 0,
    helicsDouble = 1,
    helicsInt = 2,

    helicsComplex = 3,
    helicsVector = 4,
    helicsComplexVector = 5,
    helicsNamedPoint = 6,
    helicsBool = 7,
    helicsInvalid = 23425,
    helicsAny = 247652,
};

/** sometimes we just need a ref to a string for the basic types*/
const std::string &typeNameStringRef (helics_type_t type);

/** convert a string to a type*/
helics_type_t getTypeFromString (const std::string &typeName);

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
@details string will look like {<name>:val}
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
void helicsGetComplexVector (const std::string &val, std::vector<std::complex<double>> &data);

/** convert a string to a named point*/
named_point helicsGetNamedPoint (const std::string &val);
/** get a double from a string*/
double getDoubleFromString (const std::string &val);
std::complex<double> getComplexFromString (const std::string &val);

double vectorNorm (const std::vector<double> &vec);
double vectorNorm (const std::vector<std::complex<double>> &vec);
/** convert a value to a data block to be interpreted using the specified type
@param type the type used for the data conversion
@param val a double to convert
*/
data_block typeConvert (helics_type_t type, double val);
data_block typeConvert (helics_type_t type, int64_t val);
data_block typeConvert (helics_type_t type, const char *val);
data_block typeConvert (helics_type_t type, const std::string &val);
data_block typeConvert (helics_type_t type, const std::vector<double> &val);
data_block typeConvert (helics_type_t type, const double *vals, size_t size);
data_block typeConvert (helics_type_t type, const std::vector<std::complex<double>> &val);
data_block typeConvert (helics_type_t type, const std::complex<double> &val);
data_block typeConvert (helics_type_t type, const named_point &val);
data_block typeConvert (helics_type_t type, const char *str, double val);
data_block typeConvert (helics_type_t type, const std::string &str, double val);
data_block typeConvert (helics_type_t type, bool val);

/** template class for generating a known name of a type*/
template <class X>
constexpr helics_type_t helicsType ()
{
    return helics_type_t::helicsInvalid;
}

template <>
constexpr helics_type_t helicsType<int64_t> ()
{
    return helics_type_t::helicsInt;
}

template <>
constexpr helics_type_t helicsType<bool> ()
{
    return helics_type_t::helicsBool;
}

template <>
constexpr helics_type_t helicsType<std::string> ()
{
    return helics_type_t::helicsString;
}

template <>
constexpr helics_type_t helicsType<named_point> ()
{
    return helics_type_t::helicsNamedPoint;
}
template <>
constexpr helics_type_t helicsType<double> ()
{
    return helics_type_t::helicsDouble;
}

template <>
constexpr helics_type_t helicsType<std::complex<double>> ()
{
    return helics_type_t::helicsComplex;
}

template <>
constexpr helics_type_t helicsType<std::vector<double>> ()
{
    return helics_type_t::helicsVector;
}

template <>
constexpr helics_type_t helicsType<std::vector<std::complex<double>>> ()
{
    return helics_type_t::helicsComplexVector;
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
constexpr bool isConvertableType<int> ()
{
    return true;
}

template <>
constexpr bool isConvertableType<short> ()
{
    return true;
}

template <>
constexpr bool isConvertableType<unsigned short> ()
{
    return true;
}

template <>
constexpr bool isConvertableType<unsigned int> ()
{
    return true;
}

template <>
constexpr bool isConvertableType<char> ()
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
constexpr uint64_t invalidValue<uint64_t> ()
{
    return std::numeric_limits<uint64_t>::max ();
}

template <>
inline named_point invalidValue<named_point> ()
{
    return {std::string (), std::nan ("0")};
}

template <>
constexpr std::complex<double> invalidValue<std::complex<double>> ()
{
    return std::complex<double> (-1e48, 0.0);
}

}  // namespace helics
