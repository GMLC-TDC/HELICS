/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/

#include "HelicsPrimaryTypes.hpp"
#include "ValueConverter.hpp"

namespace helics
{
bool changeDetected (const defV &prevValue, const std::string &val, double /*deltaV*/)
{
    if (prevValue.index () == stringLoc)
    {
        if (val == mpark::get<std::string> (prevValue))
        {
            return false;
        }
    }
    return true;
}

bool changeDetected (const defV &prevValue, const std::vector<double> &val, double deltaV)
{
    if (prevValue.index () == vectorLoc)
    {
        const auto &prevV = mpark::get<std::vector<double>> (prevValue);
        if (val.size () == prevV.size ())
        {
            for (size_t ii = 0; ii < val.size (); ++ii)
            {
                if (std::abs (prevV[ii] - val[ii]) > deltaV)
                {
                    return true;
                }
            }
            return false;
        }
    }
    return true;
}

bool changeDetected (const defV &prevValue, const std::vector<std::complex<double>> &val, double deltaV)
{
    if (prevValue.index () == complexVectorLoc)
    {
        const auto &prevV = mpark::get<std::vector<std::complex<double>>> (prevValue);
        if (val.size () == prevV.size ())
        {
            for (size_t ii = 0; ii < val.size (); ++ii)
            {
                if (std::abs (prevV[ii] - val[ii]) > deltaV)
                {
                    return true;
                }
            }
            return false;
        }
    }
    return true;
}

bool changeDetected (const defV &prevValue, const double *vals, size_t size, double deltaV)
{
    if (prevValue.index () == vectorLoc)
    {
        const auto &prevV = mpark::get<std::vector<double>> (prevValue);
        if (size == prevV.size ())
        {
            for (size_t ii = 0; ii < size; ++ii)
            {
                if (std::abs (prevV[ii] - vals[ii]) > deltaV)
                {
                    return true;
                }
            }
            return false;
        }
    }
    return true;
}

bool changeDetected (const defV &prevValue, const std::complex<double> &val, double deltaV)
{
    if (prevValue.index () == complexLoc)
    {
        const auto &prevV = mpark::get<std::complex<double>> (prevValue);
        if (std::abs (prevV.real () - val.real ()) > deltaV)
        {
            return true;
        }
        if (std::abs (prevV.imag () - val.imag ()) > deltaV)
        {
            return true;
        }
        return false;
    }
    return true;
}
bool changeDetected (const defV &prevValue, double val, double deltaV)
{
    if (prevValue.index () == doubleLoc)
    {
        if (std::abs (mpark::get<double> (prevValue) - val) <= deltaV)
        {
            return false;
        }
    }
    return true;
}
bool changeDetected (const defV &prevValue, int64_t val, double deltaV)
{
    if (prevValue.index () == intLoc)
    {
        if (std::abs (mpark::get<int64_t> (prevValue) - val) < static_cast<int64_t> (deltaV) + 1)
        {
            return false;
        }
    }
    return true;
}

bool changeDetected(const defV &prevValue, named_point val, double deltaV)
{
    if (prevValue.index() == doubleLoc)
    {
        if (std::abs(mpark::get<double>(prevValue) - val.second) <= deltaV)
        {
            return false;
        }
    }
    return true;
}

void valueExtract (const defV &dv, std::string &val)
{
    switch (dv.index ())
    {
    case doubleLoc:  // double
        val = std::to_string (mpark::get<double> (dv));
        break;
    case intLoc:  // int64_t
        val = std::to_string (mpark::get<int64_t> (dv));
        break;
    case stringLoc:  // string
    default:
        val = mpark::get<std::string> (dv);
        break;
    case complexLoc:  // complex
        val = helicsComplexString (mpark::get<std::complex<double>> (dv));
        break;
    case vectorLoc:  // vector
        val = helicsVectorString (mpark::get<std::vector<double>> (dv));
        break;
    case complexVectorLoc:  // vector
        val = helicsComplexVectorString (mpark::get<std::vector<std::complex<double>>> (dv));
        break;
    }
}

void valueExtract (const defV &dv, std::complex<double> &val)
{
    switch (dv.index ())
    {
    case doubleLoc:  // double
        val = std::complex<double> (mpark::get<double> (dv), 0.0);
        break;
    case intLoc:  // int64_t
        val = std::complex<double> (static_cast<double> (mpark::get<int64_t> (dv)), 0.0);
        break;
    case stringLoc:  // string
    default:
        val = helicsGetComplex (mpark::get<std::string> (dv));
        break;
    case complexLoc:  // complex
        val = mpark::get<std::complex<double>> (dv);
        break;
    case vectorLoc:  // vector
    {
        auto &vec = mpark::get<std::vector<double>> (dv);
        if (vec.size () == 1)
        {
            val = std::complex<double> (vec[0], 0.0);
        }
        else if (vec.size () > 2)
        {
            val = std::complex<double> (vec[0], vec[1]);
        }
        break;
    }
    case complexVectorLoc:
    {
        auto &vec = mpark::get<std::vector<std::complex<double>>> (dv);
        if (!vec.empty ())
        {
            val = vec.front ();
        }
        break;
    }
    }
}

void valueExtract (const defV &dv, std::vector<double> &val)
{
    val.resize (0);
    switch (dv.index ())
    {
    case doubleLoc:  // double
        val.push_back (mpark::get<double> (dv));
        break;
    case intLoc:  // int64_t
        val.push_back (static_cast<double> (mpark::get<int64_t> (dv)));
        break;
    case stringLoc:  // string
    default:
        helicsGetVector (mpark::get<std::string> (dv), val);
        break;
    case complexLoc:  // complex
    {
        auto cval = mpark::get<std::complex<double>> (dv);
        val.push_back (cval.real ());
        val.push_back (cval.imag ());
    }
    break;
    case vectorLoc:  // vector
        val = mpark::get<std::vector<double>> (dv);
        break;
    case complexVectorLoc:  // complex
    {
        auto cv = mpark::get<std::vector<std::complex<double>>> (dv);
        val.resize (2 * cv.size ());
        for (auto &cval : cv)
        {
            val.push_back (cval.real ());
            val.push_back (cval.imag ());
        }
    }
    }
}

void valueExtract (const defV &dv, std::vector<std::complex<double>> &val)
{
    val.resize (0);
    switch (dv.index ())
    {
    case doubleLoc:  // double
        val.emplace_back (mpark::get<double> (dv), 0.0);
        break;
    case intLoc:  // int64_t
        val.emplace_back (static_cast<double> (mpark::get<int64_t> (dv)), 0.0);
        break;
    case stringLoc:  // string
    default:
        helicsGetComplexVector (mpark::get<std::string> (dv), val);
        break;
    case complexLoc:  // complex
    {
        val.push_back (mpark::get<std::complex<double>> (dv));
    }
    break;
    case vectorLoc:  // vector
    {
        auto &v = mpark::get<std::vector<double>> (dv);
        val.resize (v.size () / 2);
        for (size_t ii = 0; ii < v.size () - 1; ii += 2)
        {
            val.emplace_back (v[ii], v[ii + 1]);
        }
        break;
    }
    case complexVectorLoc:  // complex
        val = mpark::get<std::vector<std::complex<double>>> (dv);
        break;
    }
}

void valueExtract(const defV &dv, named_point &val)
{
    
    switch (dv.index())
    {
    case doubleLoc:  // double
        val.first = "value";
        val.second = mpark::get<double>(dv);
        break;
    case intLoc:  // int64_t
        val.first = "value";
        val.second = static_cast<double> (mpark::get<int64_t>(dv));
        break;
    case stringLoc:  // string
    default:
        val = helicsGetNamedPoint(mpark::get<std::string>(dv));
        break;
    case complexLoc:  // complex
        val.first = "value";
        val.second = std::abs(mpark::get<std::complex<double>>(dv));
        break;
    case vectorLoc:  // vector
    {
        val.first = "value";
        auto &vec = mpark::get<std::vector<double>>(dv);
        if (vec.size() == 2)
        {
            val.second = std::hypot(vec[0], vec[1]);
        }
        else 
        {
            val.second = (vec.empty())?0.0:(vec.front());
        }
        break;
    }
    case complexVectorLoc:
    {
        val.first = "value";
        auto &vec = mpark::get<std::vector<std::complex<double>>>(dv);
        if (!vec.empty())
        {
            val.second = std::abs(vec.front());
        }
        else
        {
            val.second = 0.0;
        }
        break;
    }
    }
}

void valueExtract (const data_view &dv, helics_type_t baseType, std::string &val)
{
    switch (baseType)
    {
    case helics_type_t::helicsDouble:
    {
        auto V = ValueConverter<double>::interpret (dv);
        val = std::to_string (V);
        break;
    }
    case helics_type_t::helicsInt:
    {
        auto V = ValueConverter<int64_t>::interpret (dv);
        val = std::to_string (V);
        break;
    }
    case helics_type_t::helicsString:
    case helics_type_t::helicsAny:
    {
        val = dv.string ();
        break;
    }
    case helics_type_t::helicsVector:
    {
        val = helicsVectorString (ValueConverter<std::vector<double>>::interpret (dv));
        break;
    }
    case helics_type_t::helicsComplex:
    {
        val = helicsComplexString (ValueConverter<std::complex<double>>::interpret (dv));
        break;
    }
    case helics_type_t::helicsComplexVector:
    {
        val = helicsComplexVectorString (ValueConverter<std::vector<std::complex<double>>>::interpret (dv));
    }
    case helics_type_t::helicsInvalid:
    default:
        break;
    }
}

void valueExtract (const data_view &dv, helics_type_t baseType, std::vector<double> &val)
{
    val.resize (0);
    switch (baseType)
    {
    case helics_type_t::helicsDouble:
    {
        val.push_back (ValueConverter<double>::interpret (dv));
        break;
    }
    case helics_type_t::helicsInt:
    {
        val.push_back (static_cast<double> (ValueConverter<int64_t>::interpret (dv)));
        break;
    }
    case helics_type_t::helicsString:
    {
        helicsGetVector (dv.string (), val);
        break;
    }
    case helics_type_t::helicsVector:
    {
        ValueConverter<std::vector<double>>::interpret (dv, val);
        break;
    }
    case helics_type_t::helicsComplex:
    {
        auto cval = ValueConverter<std::complex<double>>::interpret (dv);
        val.push_back (cval.real ());
        val.push_back (cval.imag ());
        break;
    }
    case helics_type_t::helicsComplexVector:
    {
        auto cv = ValueConverter<std::vector<std::complex<double>>>::interpret (dv);
        val.reserve (2 * cv.size ());
        for (auto &cval : cv)
        {
            val.push_back (cval.real ());
            val.push_back (cval.imag ());
        }
        break;
    }
    case helics_type_t::helicsInvalid:
    default:
        break;
    }
}

void valueExtract (const data_view &dv, helics_type_t baseType, std::vector<std::complex<double>> &val)
{
    val.resize (0);
    switch (baseType)
    {
    case helics_type_t::helicsDouble:
    {
        val.emplace_back (ValueConverter<double>::interpret (dv), 0.0);
        break;
    }
    case helics_type_t::helicsInt:
    {
        val.emplace_back (static_cast<double> (ValueConverter<int64_t>::interpret (dv)), 0.0);
        break;
    }
    case helics_type_t::helicsString:
    {
        helicsGetComplexVector (dv.string (), val);
        break;
    }
    case helics_type_t::helicsVector:
    {
        auto V = ValueConverter<std::vector<double>>::interpret (dv);
        for (size_t ii = 0; ii < V.size () - 1; ii += 2)
        {
            val.emplace_back (V[ii], V[ii + 1]);
        }
        break;
    }
    case helics_type_t::helicsComplexVector:
    {
        ValueConverter<std::vector<std::complex<double>>>::interpret (dv, val);
        break;
    }
    case helics_type_t::helicsComplex:
    {
        auto cval = ValueConverter<std::complex<double>>::interpret (dv);
        val.push_back (cval);
        break;
    }
    case helics_type_t::helicsInvalid:
    default:
        break;
    }
}

void valueExtract (const data_view &dv, helics_type_t baseType, std::complex<double> &val)
{
    switch (baseType)
    {
    case helics_type_t::helicsDouble:
    {
        val = std::complex<double> (ValueConverter<double>::interpret (dv), 0.0);
        break;
    }
    case helics_type_t::helicsInt:
    {
        val = std::complex<double> (static_cast<double> (ValueConverter<int64_t>::interpret (dv)), 0.0);
        break;
    }
    case helics_type_t::helicsString:
    {
        val = helicsGetComplex (dv.string ());
        break;
    }
    case helics_type_t::helicsVector:
    {
        auto vec = ValueConverter<std::vector<double>>::interpret (dv);
        if (vec.size () == 1)
        {
            val = std::complex<double> (vec[0], 0.0);
        }
        else if (vec.size () > 2)
        {
            val = std::complex<double> (vec[0], vec[1]);
        }
        break;
    }
    case helics_type_t::helicsComplex:
    {
        val = ValueConverter<std::complex<double>>::interpret (dv);
        break;
    }
    case helics_type_t::helicsInvalid:
    default:
        break;
    }
}

}  // namespace helics