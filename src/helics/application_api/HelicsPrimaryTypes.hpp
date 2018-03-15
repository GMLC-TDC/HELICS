/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "ValueConverter.hpp"
#include "helicsTypes.hpp"
#include <cmath>
#include <complex>
#include <cstdint>
#include <string>
#include <vector>
#include <helics_includes/variant.hpp>
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
                            std::vector<std::complex<double>>>;

/**enumeration of the order inside the variant so the Which function returns match the enumeration*/
enum type_location
{

    doubleLoc = 0,
    intLoc = 1,
    stringLoc = 2,
    complexLoc = 3,
    vectorLoc = 4,
    complexVectorLoc = 5
};
/** detect a change from the previous values*/
bool changeDetected (const defV &prevValue, const std::string &val, double deltaV);
bool changeDetected (const defV &prevValue, const std::vector<double> &val, double deltaV);
bool changeDetected (const defV &prevValue, const std::vector<std::complex<double>> &val, double deltaV);
bool changeDetected (const defV &prevValue, const double *vals, size_t size, double deltaV);
bool changeDetected (const defV &prevValue, const std::complex<double> &val, double deltaV);
bool changeDetected (const defV &prevValue, double val, double deltaV);
bool changeDetected (const defV &prevValue, int64_t val, double deltaV);
bool changeDetected (const defV &prevValue, named_point val, double deltaV);

void valueExtract (const defV &dv, std::string &val);

void valueExtract (const defV &dv, std::complex<double> &val);

void valueExtract (const defV &dv, std::vector<double> &val);

void valueExtract (const defV &dv, std::vector<std::complex<double>> &val);

void valueExtract(const defV &dv, named_point &val);

void valueExtract (const data_view &dv, helics_type_t baseType, std::string &val);

void valueExtract (const data_view &dv, helics_type_t baseType, std::vector<double> &val);

void valueExtract (const data_view &dv, helics_type_t baseType, std::complex<double> &val);

void valueExtract (const data_view &dv, helics_type_t baseType, std::vector<std::complex<double>> &val);

/** for numeric types*/
template <class X>
std::enable_if_t<std::is_arithmetic<X>::value> valueExtract (const defV &dv, X &val)
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
        if IF_CONSTEXPR(std::is_integral<X>::value)
        {
            val = static_cast<X>(std::stoll(mpark::get<std::string>(dv)));
        }
        else
        {
            val = static_cast<X>(std::stod (mpark::get<std::string>(dv)));
        }
        
        break;
    case complexLoc:  // complex
        val = static_cast<X> (std::abs (mpark::get<std::complex<double>> (dv)));
        break;
    case vectorLoc:  // vector
    {
        auto &vec = mpark::get<std::vector<double>> (dv);
        if (!vec.empty ())
        {
            if (vec.size() == 2)
            {
                val = static_cast<X> (std::hypot(vec[0],vec[1]));
            }
            else
            {
                val = static_cast<X> (vec.front());
            }

        }
        else
        {
            val = X(0);
        }
        break;
    }
    case complexVectorLoc:  // complex vector
    {
        auto &vec = mpark::get<std::vector<std::complex<double>>> (dv);
        if (!vec.empty ())
        {
            val = static_cast<X> (std::abs (vec.front ()));
        }
        else
        {
            val =X(0);
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
        if (dv.size() == 9)
        {
            auto V = ValueConverter<double>::interpret(dv);
            if (std::isnormal(V))
            {
                val = static_cast<X> (V);
            }
            else
            {
                auto Vint = ValueConverter<int64_t>::interpret(dv);
                val = static_cast<X> (Vint);
            }
        }
        else if (dv.size() == 17)
        {
            auto V = ValueConverter<std::complex<double>>::interpret(dv);
            val = static_cast<X> (std::abs(V));
        }
        try
        {
            val = static_cast<X> (std::stod (dv.string ()));
        }
        catch (const std::invalid_argument &ble)
        {  // well lets try a vector conversion
            auto V = ValueConverter<std::vector<double>>::interpret(dv);
            if (V.size() == 2)
            {
                val = static_cast<X> (std::hypot(V[0], V[1]));
            }
            else
            {
                val = (V.empty()) ? X(0) : static_cast<X> (V.front());
            }
        }
        break;
    }
    case helics_type_t::helicsString:
    {
        if IF_CONSTEXPR(std::is_integral<X>::value)
        {
            val = static_cast<X>(std::stoll(ValueConverter<std::string>::interpret(dv)));
        }
        else
        {
            val = static_cast<X>(std::stod(ValueConverter<std::string>::interpret(dv)));
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
    {
        auto V = ValueConverter<int64_t>::interpret (dv);
        val = static_cast<X> (V);
        break;
    }

    case helics_type_t::helicsVector:
    {
        auto V = ValueConverter<std::vector<double>>::interpret (dv);
        if (V.size() == 2)
        {
            val = static_cast<X> (std::hypot(V[0], V[1]));
        }
        else
        {
            val = (V.empty()) ? X(0) : static_cast<X> (V.front());
        }
        break;
    }
    case helics_type_t::helicsComplex:
    {
        auto V = ValueConverter<std::complex<double>>::interpret (dv);
        val = static_cast<X> (std::abs (V));
        break;
    }
    case helics_type_t::helicsNamedPoint:
    {
        auto V = ValueConverter<named_point>::interpret (dv);
        val = static_cast<X> (V.second);
        break;
    }
    case helics_type_t::helicsComplexVector:
    {
        auto V = ValueConverter<std::vector<std::complex<double>>>::interpret (dv);
        val = (!V.empty ()) ? static_cast<X> (std::abs (V.front ())) : 0.0;
        break;
    }
    case helics_type_t::helicsInvalid:
        throw (std::invalid_argument ("unrecognized helics type"));
    }
}
}  // namespace helics

