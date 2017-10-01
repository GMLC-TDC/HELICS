/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "Subscriptions.hpp"

namespace helics
{
void Subscription::handleCallback (Time time)
{
    auto dv = fed->getValueRaw (id);
	if (type == helicsType_t::helicsInvalid)
	{
		type = getTypeFromString(fed->getPublicationType(id));
	}
    switch (value_callback.which ())
    {
 
    case doubleLoc:
    {
        double val;
        valueExtract (dv, type, val);
        boost::get<std::function<void(const double &, Time)>> (value_callback) (val, time);
    }
    break;
    case intLoc:
    {
        int64_t val;
        valueExtract (dv, type, val);
        boost::get<std::function<void(const int64_t &, Time)>> (value_callback) (val, time);
    }
    break;
	case stringLoc:
	default:
	{
		std::string val;
		valueExtract(dv, type, val);
		boost::get<std::function<void(const std::string &, Time)>>(value_callback) (val, time);
	}
	break;
    case complexLoc:
    {
        std::complex<double> val;
        valueExtract (dv, type, val);
        boost::get<std::function<void(const std::complex<double> &, Time)>> (value_callback) (val, time);
    }
    break;
    case vectorLoc:
    {
        std::vector<double> val;
        valueExtract (dv, type, val);
        boost::get<std::function<void(const std::vector<double> &, Time)>> (value_callback) (val, time);
    }
    break;
    }
}

void valueExtract (const defV &dv, std::string &val)
{
    switch (dv.which ())
    {
   
    case doubleLoc:  // double
        val = std::to_string (boost::get<double> (dv));
        break;
    case intLoc:  // int64_t
        val = std::to_string (boost::get<int64_t> (dv));
        break;
	case stringLoc:  // string
	default:
		val = boost::get<std::string>(dv);
		break;
    case complexLoc:  // complex
        val = helicsComplexString (boost::get<std::complex<double>> (dv));
        break;
    case vectorLoc:  // vector
        val = helicsVectorString (boost::get<std::vector<double>> (dv));
        break;
	case complexVectorLoc:  // vector
		val = helicsComplexVectorString(boost::get<std::vector<std::complex<double>>>(dv));
		break;
    }
}

void valueExtract (const defV &dv, std::complex<double> &val)
{
    switch (dv.which ())
    {
   
    case doubleLoc:  // double
        val = std::complex<double> (boost::get<double> (dv), 0.0);
        break;
    case intLoc:  // int64_t
        val = std::complex<double> (static_cast<double> (boost::get<int64_t> (dv)), 0.0);
        break;
	case stringLoc:  // string
	default:
		val = helicsGetComplex(boost::get<std::string>(dv));
		break;
    case complexLoc:  // complex
        val = boost::get<std::complex<double>> (dv);
        break;
    case vectorLoc:  // vector
    {
        auto &vec = boost::get<std::vector<double>> (dv);
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
		auto &vec = boost::get<std::vector<std::complex<double>>>(dv);
		if (!vec.empty())
		{
			val = vec.front();
		}
		break;
	}
    }
}

void valueExtract (const defV &dv, std::vector<double> &val)
{
    val.resize (0);
    switch (dv.which ())
    {
   
    case doubleLoc:  // double
        val.push_back (boost::get<double> (dv));
        break;
    case intLoc:  // int64_t
        val.push_back (static_cast<double> (boost::get<int64_t> (dv)));
        break;
	case stringLoc:  // string
	default:
		helicsGetVector(boost::get<std::string>(dv), val);
		break;
    case complexLoc:  // complex
    {
        auto cval = boost::get<std::complex<double>> (dv);
        val.push_back (cval.real ());
        val.push_back (cval.imag ());
    }
    break;
    case vectorLoc:  // vector
        val = boost::get<std::vector<double>> (dv);
        break;
	case complexVectorLoc:  // complex
	{
		auto cv = boost::get<std::vector<std::complex<double>>>(dv);
		val.resize(2 * cv.size());
		for (auto &cval : cv)
		{
			val.push_back(cval.real());
			val.push_back(cval.imag());
		}
	}
    }
}

void valueExtract (const data_view &dv, helicsType_t baseType, std::string &val)
{
    switch (baseType)
    {
    case helicsType_t::helicsDouble:
    {
        auto V = ValueConverter<double>::interpret (dv);
        val = std::to_string (V);
        break;
    }
    case helicsType_t::helicsInt:
    {
        auto V = ValueConverter<int64_t>::interpret (dv);
        val = std::to_string (V);
        break;
    }
    case helicsType_t::helicsString:
    {
        val = dv.string ();
        break;
    }
    case helicsType_t::helicsVector:
    {
        val = helicsVectorString (ValueConverter<std::vector<double>>::interpret (dv));
        break;
    }
    case helicsType_t::helicsComplex:
    {
        val = helicsComplexString (ValueConverter<std::complex<double>>::interpret (dv));
        break;
    }
	case helicsType_t::helicsComplexVector:
	{
		val = helicsComplexVectorString(ValueConverter<std::vector<std::complex<double>>>::interpret(dv));
	}
    case helicsType_t::helicsInvalid:
    default:
        break;
    }
}

void valueExtract (const data_view &dv, helicsType_t baseType, std::vector<double> &val)
{
    val.resize (0);
    switch (baseType)
    {
    case helicsType_t::helicsDouble:
    {
        val.push_back (ValueConverter<double>::interpret (dv));
        break;
    }
    case helicsType_t::helicsInt:
    {
        val.push_back (static_cast<double> (ValueConverter<int64_t>::interpret (dv)));
        break;
    }
    case helicsType_t::helicsString:
    {
        helicsGetVector (dv.string (), val);
        break;
    }
    case helicsType_t::helicsVector:
    {
        ValueConverter<std::vector<double>>::interpret (dv, val);
        break;
    }
    case helicsType_t::helicsComplex:
    {
        auto cval = ValueConverter<std::complex<double>>::interpret (dv);
        val.push_back (cval.real ());
        val.push_back (cval.imag ());
        break;
    }
	case helicsType_t::helicsComplexVector:
	{
		auto cv=ValueConverter<std::vector<std::complex<double>>>::interpret(dv);
		val.reserve(2 * cv.size());
		for (auto &cval : cv)
		{
			val.push_back(cval.real());
			val.push_back(cval.imag());
		}
		break;
	}
    case helicsType_t::helicsInvalid:
    default:
        break;
    }
}

void valueExtract(const data_view &dv, helicsType_t baseType, std::vector<std::complex<double>> &val)
{
	val.resize(0);
	switch (baseType)
	{
	case helicsType_t::helicsDouble:
	{
		val.emplace_back(ValueConverter<double>::interpret(dv),0.0);
		break;
	}
	case helicsType_t::helicsInt:
	{
		val.emplace_back(static_cast<double> (ValueConverter<int64_t>::interpret(dv)),0.0);
		break;
	}
	case helicsType_t::helicsString:
	{
		helicsGetComplexVector(dv.string(), val);
		break;
	}
	case helicsType_t::helicsVector:
	{
		auto V=ValueConverter<std::vector<double>>::interpret(dv);
		for (size_t ii = 0; ii < V.size()-1; ii += 2)
		{
			val.emplace_back(V[ii], V[ii + 1]);
		}
		break;
	}
	case helicsType_t::helicsComplexVector:
	{
		ValueConverter<std::vector<std::complex<double>>>::interpret(dv, val);
		break;
	}
	case helicsType_t::helicsComplex:
	{
		auto cval = ValueConverter<std::complex<double>>::interpret(dv);
		val.push_back(cval);
		break;
	}
	case helicsType_t::helicsInvalid:
	default:
		break;
	}
}

void valueExtract (const data_view &dv, helicsType_t baseType, std::complex<double> &val)
{
    switch (baseType)
    {
    case helicsType_t::helicsDouble:
    {
        val = std::complex<double> (ValueConverter<double>::interpret (dv), 0.0);
        break;
    }
    case helicsType_t::helicsInt:
    {
        val = std::complex<double> (static_cast<double> (ValueConverter<int64_t>::interpret (dv)), 0.0);
        break;
    }
    case helicsType_t::helicsString:
    {
        val = helicsGetComplex (dv.string ());
        break;
    }
    case helicsType_t::helicsVector:
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
    case helicsType_t::helicsComplex:
    {
        val = ValueConverter<std::complex<double>>::interpret (dv);
        break;
    }
    case helicsType_t::helicsInvalid:
    default:
        break;
    }
}
}