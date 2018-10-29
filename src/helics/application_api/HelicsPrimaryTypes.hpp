/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "ValueConverter.hpp"
#include "helics/helics-config.h"
#include "helicsTypes.hpp"
#include <cmath>
#include <complex>
#include <cstdint>
#include <helics_includes/variant.hpp>
#include <string>
#include <vector>
/** @file
@brief naming a set of types that are interchangeable and recognizable inside the HELICS application API and core
*/
namespace helics
{
/** define a variant with the different types*/

using defV = mpark::variant<double,
                            int64_t,
                            std::string,
                            std::complex<double>,
                            std::vector<double>,
                            std::vector<std::complex<double>>,
                            named_point>;

/**enumeration of the order inside the variant so the Which function returns match the enumeration*/
enum type_location
{
    doubleLoc = 0,
    intLoc = 1,
    stringLoc = 2,
    complexLoc = 3,
    vectorLoc = 4,
    complexVectorLoc = 5,
    namedPointLoc = 6
};
/** detect a change from the previous values*/
bool changeDetected (const defV &prevValue, const std::string &val, double deltaV);
bool changeDetected (const defV &prevValue, const char *val, double deltaV);
bool changeDetected (const defV &prevValue, const std::vector<double> &val, double deltaV);
bool changeDetected (const defV &prevValue, const std::vector<std::complex<double>> &val, double deltaV);
bool changeDetected (const defV &prevValue, const double *vals, size_t size, double deltaV);
bool changeDetected (const defV &prevValue, const std::complex<double> &val, double deltaV);
bool changeDetected (const defV &prevValue, double val, double deltaV);
bool changeDetected (const defV &prevValue, int64_t val, double deltaV);
bool changeDetected (const defV &prevValue, Time val, double deltaV);
bool changeDetected (const defV &prevValue, const named_point &val, double deltaV);
bool changeDetected (const defV &prevValue, bool val, double deltaV);

/** directly convert the boolean to integer*/
inline int64_t make_valid (bool obj) { return (obj) ? 1ll : 0ll; }

/** directly convert the boolean to integer*/
inline int64_t make_valid (uint64_t val) { return static_cast<int64_t> (val); }
inline int64_t make_valid (int16_t val) { return static_cast<int64_t> (val); }
inline int64_t make_valid (uint16_t val) { return static_cast<int64_t> (val); }
inline int64_t make_valid (char val) { return static_cast<int64_t> (val); }
inline int64_t make_valid (unsigned char val) { return static_cast<int64_t> (val); }
inline int64_t make_valid (int32_t val) { return static_cast<int64_t> (val); }
inline int64_t make_valid (uint32_t val) { return static_cast<int64_t> (val); }
inline int64_t make_valid (Time val) { return val.getBaseTimeCode(); }

inline double make_valid (float val) { return static_cast<double> (val); }

inline std::complex<double> make_valid (const std::complex<float> &val)
{
    return std::complex<double> (val.real (), val.imag ());
}

/** this template should do nothing for most classes the specific overloads are the important ones*/
template <class X>
decltype (auto) make_valid (X &&obj)
{
    return std::forward<X> (obj);
}
/** extract the value from a variant to a string*/
void valueExtract (const defV &dv, std::string &val);

/** extract the value from a variant to a complex number*/
void valueExtract (const defV &dv, std::complex<double> &val);

/** extract the value from a variant to a vector of doubles*/
void valueExtract (const defV &dv, std::vector<double> &val);

/** extract the value from a variant to a complex vector of doubles*/
void valueExtract (const defV &dv, std::vector<std::complex<double>> &val);

/** extract the value from a variant to a named point*/
void valueExtract (const defV &dv, named_point &val);

/** extract the value from a variant to a named point*/
void valueExtract (const defV &dv, Time &val);

/** extract the value from a variant to a character*/
void valueExtract (const defV &dv, char &val);

void valueExtract (const data_view &dv, helics_type_t baseType, std::string &val);

void valueExtract (const data_view &dv, helics_type_t baseType, std::vector<double> &val);

void valueExtract (const data_view &dv, helics_type_t baseType, std::complex<double> &val);

void valueExtract (const data_view &dv, helics_type_t baseType, std::vector<std::complex<double>> &val);

void valueExtract (const data_view &dv, helics_type_t baseType, named_point &val);

void valueExtract (const data_view &dv, helics_type_t baseType, Time &val);

void valueExtract (const data_view &dv, helics_type_t baseType, defV &val);

/** extract the value from a variant to a numerical type*/
template <class X>
std::enable_if_t<std::is_arithmetic<X>::value && (!std::is_same<X, char>::value)>
valueExtract (const defV &dv, X &val)
{
    switch (dv.index ())
    {
    case doubleLoc:  // double
        val = static_cast<X> (mpark::get<double> (dv));
        break;
    case intLoc:  // int64_t
        val = static_cast<X> (mpark::get<int64_t> (dv));
        break;
    case stringLoc:  // string
    default:
        if
            IF_CONSTEXPR (std::is_integral<X>::value)
            {
                val = static_cast<X> (std::stoll (mpark::get<std::string> (dv)));
            }
        else
        {
            val = static_cast<X> (std::stod (mpark::get<std::string> (dv)));
        }

        break;
    case complexLoc:  // complex
        val = static_cast<X> (std::abs (mpark::get<std::complex<double>> (dv)));
        break;
    case vectorLoc:  // vector
    {
        auto &vec = mpark::get<std::vector<double>> (dv);
        val = static_cast<X> (vectorNorm (vec));
        break;
    }
    case complexVectorLoc:  // complex vector
    {
        auto &vec = mpark::get<std::vector<std::complex<double>>> (dv);
        val = static_cast<X> (vectorNorm (vec));
        break;
    }
    case namedPointLoc:
    {
        auto &np = mpark::get<named_point> (dv);
        if (std::isnan (np.value))
        {
            val = static_cast<X> (getDoubleFromString (np.name));
        }
        else
        {
            val = static_cast<X> (np.value);
        }
        break;
    }
    }
}

/** assume it is some numeric type (int or double)*/
template <class X>
std::enable_if_t<std::is_arithmetic<X>::value> valueExtract (const data_view &dv, helics_type_t baseType, X &val)
{
    switch (baseType)
    {
    case helics_type_t::helicsAny:
    {
        if (dv.size () == 9)
        {
            auto V = ValueConverter<double>::interpret (dv);
            if (std::isnormal (V))
            {
                val = static_cast<X> (V);
            }
            else
            {
                auto Vint = ValueConverter<int64_t>::interpret (dv);
                val = static_cast<X> (Vint);
            }
        }
        else if (dv.size () == 17)
        {
            auto V = ValueConverter<std::complex<double>>::interpret (dv);
            val = static_cast<X> (std::abs (V));
        }
        else if (dv.size () == 1)
        {
            val = static_cast<X> ((dv[0] == '0') ? 0 : 1);
        }
        else
        {
            try
            {
                val = static_cast<X> (std::stod (dv.string ()));
            }
            catch (const std::invalid_argument &)
            {  // well lets try a vector conversion
                auto V = ValueConverter<std::vector<double>>::interpret (dv);
                val = static_cast<X> (vectorNorm (V));
            }
        }
        break;
    }
    case helics_type_t::helicsString:
        val = static_cast<X> (getDoubleFromString (dv.string ()));
        break;
    case helics_type_t::helicsBool:
        val = static_cast<X> ((dv.string () != "0"));
        break;
    case helics_type_t::helicsNamedPoint:
    {
        auto npval = ValueConverter<named_point>::interpret (dv);
        if (std::isnan (npval.value))
        {
            try
            {
                val = static_cast<X> (getDoubleFromString (npval.name));
            }
            catch (const std::invalid_argument &)
            {
                val = static_cast<X> (
                  invalidValue<std::conditional_t<std::is_integral<X>::value, int64_t, double>> ());
            }
        }
        else
        {
            val = static_cast<X> (npval.value);
        }

        break;
    }
    case helics_type_t::helicsDouble:
    {
        auto V = ValueConverter<double>::interpret (dv);
        val = static_cast<X> (V);
        break;
    }
    case helics_type_t::helicsInt:
    case helics_type_t::helicsTime:
    {
        auto V = ValueConverter<int64_t>::interpret (dv); val = static_cast<X> (V); 
		break;
    }

    case helics_type_t::helicsVector:
    {
        auto V = ValueConverter<std::vector<double>>::interpret (dv);
        val = static_cast<X> (vectorNorm (V));
        break;
    }
    case helics_type_t::helicsComplex:
    {
        auto V = ValueConverter<std::complex<double>>::interpret (dv);
        val = static_cast<X> (std::abs (V));
        break;
    }
    case helics_type_t::helicsComplexVector:
    {
        auto V = ValueConverter<std::vector<std::complex<double>>>::interpret (dv);
        val = static_cast<X> (vectorNorm (V));
        break;
    }
    case helics_type_t::helicsCustom:
        throw (std::invalid_argument ("unrecognized helics type"));
    }
}

void valueConvert (defV &val, helics_type_t newType);

}  // namespace helics
