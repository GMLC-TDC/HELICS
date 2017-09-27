/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "Subscriptions.hpp"

namespace helics
{


	void Subscription::handleCallback(Time time)
	{
		auto dv = fed->getValueRaw(id);
		switch (value_callback.which())
		{
		case stringLoc:
		default:
		{
			std::string val;
			valueExtract(dv,type, val);
			boost::get < std::function<void(const std::string &, Time)>>(value_callback)(val, time);
		}
		break;
		case doubleLoc:
		{
			double val;
			valueExtract(dv, type, val);
			boost::get < std::function<void(const double &, Time)>>(value_callback)(val, time);
		}
		break;
		case intLoc:
		{
			int64_t val;
			valueExtract(dv, type, val);
			boost::get < std::function<void(const int64_t &, Time)>>(value_callback)(val, time);
		}
		break;
		case complexLoc:
		{
			std::complex<double> val;
			valueExtract(dv, type, val);
			boost::get < std::function<void(const std::complex<double> &, Time)>>(value_callback)(val, time);
		}
		break;
		case vectorLoc:
		{
			std::vector<double> val;
			valueExtract(dv, type, val);
			boost::get < std::function<void(const std::vector<double> &, Time)>>(value_callback)(val, time);
		}
		break;

			
		}
		
	}

void valueExtract(const defV &dv, std::string &val)
{
	switch (dv.which())
	{
	case 0: //string
	default:
		val = boost::get<std::string>(dv);
		break;
	case 1: //double
		val = std::to_string(boost::get<double>(dv));
		break;
	case 2: //int64_t
		val = std::to_string(boost::get<int64_t>(dv));
		break;
	case 3: //complex
		val = helicsComplexString(boost::get<std::complex<double>>(dv));
		break;
	case 4: //vector
		val = helicsVectorString(boost::get<std::vector<double>>(dv));
		break;
	}
}

void valueExtract(const defV &dv, std::complex<double> &val)
{
	switch (dv.which())
	{
	case 0: //string
	default:
		val = helicsGetComplex(boost::get < std::string>(dv));
		break;
	case 1: //double
		val = std::complex<double>(boost::get<double>(dv), 0.0);
		break;
	case 2: //int64_t
		val = std::complex<double>(static_cast<double>(boost::get<int64_t>(dv)), 0.0);
		break;
	case 3: //complex
		val = boost::get<std::complex<double>>(dv);
		break;
	case 4: //vector
	{
		auto &vec = boost::get<std::vector<double>>(dv);
		if (vec.size() == 1)
		{
			val = std::complex<double>(vec[0], 0.0);
		}
		else if (vec.size() > 2)
		{
			val = std::complex<double>(vec[0], vec[1]);
		}
		break;
	}
	}
}

void valueExtract(const defV &dv, std::vector<double> &val)
{
	val.resize(0);
	switch (dv.which())
	{
	case stringLoc: //string
	default:
		helicsGetVector(boost::get<std::string>(dv), val);
		break;
	case 1: //double
		val.push_back(boost::get<double>(dv));
		break;
	case 2: //int64_t
		val.push_back(static_cast<double>(boost::get<int64_t>(dv)));
		break;
	case 3: //complex
	{
		auto cval = boost::get<std::complex<double>>(dv);
		val.push_back(cval.real());
		val.push_back(cval.imag());
	}
	break;
	case 4: //vector
		val = boost::get<std::vector<double>>(dv);
		break;
	}
}

void valueExtract(const data_view &dv, helicsType_t baseType, std::string &val)
{
	switch (baseType)
	{
	case helicsType_t::helicsDouble:
	{
		auto V = ValueConverter<double>::interpret(dv);
		val = std::to_string(V);
		break;
	}
	case helicsType_t::helicsInt:
	{
		auto V = ValueConverter<int64_t>::interpret(dv);
		val = std::to_string(V);
		break;
	}
	case helicsType_t::helicsString:
	{
		val = dv.string();
		break;
	}
	case helicsType_t::helicsVector:
	{
		val = helicsVectorString(ValueConverter<std::vector<double>>::interpret(dv));
		break;
	}
	case helicsType_t::helicsComplex:
	{
		val = helicsComplexString(ValueConverter<std::complex<double>>::interpret(dv));
		break;
	}
	case helicsType_t::helicsInvalid:
	default:
		break;
	}
}

void valueExtract(const data_view &dv, helicsType_t baseType, std::vector<double> &val)
{
	val.resize(0);
	switch (baseType)
	{
	case helicsType_t::helicsDouble:
	{
		val.push_back(ValueConverter<double>::interpret(dv));
		break;
	}
	case helicsType_t::helicsInt:
	{
		val.push_back(static_cast<double>(ValueConverter<int64_t>::interpret(dv)));
		break;
	}
	case helicsType_t::helicsString:
	{
		helicsGetVector(dv.string(),val);
		break;
	}
	case helicsType_t::helicsVector:
	{
		ValueConverter<std::vector<double>>::interpret(dv,val);
		break;
	}
	case helicsType_t::helicsComplex:
	{
		auto cval=ValueConverter<std::complex<double>>::interpret(dv);
		val.push_back(cval.real());
		val.push_back(cval.imag());
		break;
	}
	case helicsType_t::helicsInvalid:
	default:
		break;
	}
}

void valueExtract(const data_view &dv, helicsType_t baseType, std::complex<double> &val)
{
	switch (baseType)
	{
	case helicsType_t::helicsDouble:
	{
		val = std::complex<double>(ValueConverter<double>::interpret(dv), 0.0);
		break;
	}
	case helicsType_t::helicsInt:
	{
		val = std::complex<double>(static_cast<double>(ValueConverter<int64_t>::interpret(dv)), 0.0);
		break;
	}
	case helicsType_t::helicsString:
	{
		val = helicsGetComplex(dv.string());
		break;
	}
	case helicsType_t::helicsVector:
	{
		auto vec = ValueConverter<std::vector<double>>::interpret(dv);
		if (vec.size() == 1)
		{
			val = std::complex<double>(vec[0], 0.0);
		}
		else if (vec.size() > 2)
		{
			val = std::complex<double>(vec[0], vec[1]);
		}
		break;
	}
	case helicsType_t::helicsComplex:
	{
		val = ValueConverter<std::complex<double>>::interpret(dv);
		break;
	}
	case helicsType_t::helicsInvalid:
	default:
		break;
	}
}

}