/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "PrecHelper.h"


#ifdef __GNUC__
#pragma GCC diagnostic push
//#pragma GCC diagnostic warning "-w"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#pragma GCC diagnostic pop
#else
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#endif

#include <algorithm>
#include <sstream>
#include <regex>
#include <complex>

valueTypes_t getType(const std::string &typeString)
{
	auto tstr = typeString;
	//trim the string
	tstr.erase(tstr.find_last_not_of(" \t\n\0") + 1);
	tstr.erase(0, tstr.find_first_not_of(" \t\n\0"));
	if (tstr.empty())
	{
		return valueTypes_t::unknownValue;
	}
	if (tstr.size() == 1)
	{
		switch (tstr[0])
		{
		case 's':case 'S':
			return valueTypes_t::stringValue;
		case 'd':case 'D':case 'f':case 'F':
			return valueTypes_t::doubleValue;
		case 'i':case 'I':
			return valueTypes_t::int64Value;
		case 'c': case 'C':
			return valueTypes_t::complexValue;
		case 'v': case 'V':
			return valueTypes_t::vectorValue;
		default:
			return valueTypes_t::unknownValue;
		}
	}
	
	std::transform(tstr.begin(), tstr.end(), tstr.begin(), ::tolower);

	if (tstr == "string")
	{
		return valueTypes_t::stringValue;
	}
	if ((tstr == "double") || (tstr == "float"))
	{
		return valueTypes_t::doubleValue;
	}
	if ((tstr == "int") || (tstr == "int64") || (tstr == "integer"))
	{
		return valueTypes_t::int64Value;
	}
	if (tstr == "vector")
	{
		return valueTypes_t::vectorValue;
	}
	if ((tstr == "complex") || (tstr == "pair"))
	{
		return valueTypes_t::complexValue;
	}
	return valueTypes_t::unknownValue;
}

std::string typeString(valueTypes_t type)
{
	switch (type)
	{
	case valueTypes_t::stringValue:
		return "string";
	case valueTypes_t::doubleValue:
		return "double";
	case valueTypes_t::int64Value:
		return "integer";
	case valueTypes_t::complexValue:
		return "complex";
	case valueTypes_t::vectorValue:
		return "vector_double";
	case valueTypes_t::unknownValue:
	default:
		return "unknown";
	}
}


char typeCharacter(valueTypes_t type)
{
	switch (type)
	{
	case valueTypes_t::stringValue:
		return 's';
	case valueTypes_t::doubleValue:
		return 'd';
	case valueTypes_t::int64Value:
		return 'i';
	case valueTypes_t::complexValue:
		return 'c';
	case valueTypes_t::vectorValue:
		return 'v';
	case valueTypes_t::unknownValue:
	default:
		return 'u';
	}
}
std::string helicsComplexString(double real, double imag)
{
	std::stringstream ss;
	ss << real;
	if (imag != 0)
	{
		if (imag >= 0.0)
		{
			ss << '+' << imag;
		}
		else
		{
			ss << imag;
		}
		ss << 'j';
	}
	
	return ss.str();
}

std::string helicsComplexString(std::complex<double> val)
{
	return helicsComplexString(val.real(), val.imag());
}

const std::regex creg("([+-]?(\\d+(\\.\\d+)?|\\.\\d+)([eE][+-]?\\d+)?)\\s*([+-]\\s*(\\d+(\\.\\d+)?|\\.\\d+)([eE][+-]?\\d+)?)[ji]*");

std::complex<double> helicsGetComplex(const std::string &val)
{

	if (val.empty())
	{
		return std::complex<double>(-1e49, -1e49);
	}
	std::smatch m;
	double re = 0.0;
	double im = 0.0;
	std::regex_search(val, m, creg);
	if (m.size() == 9)
	{
		try
		{
			re = std::stod(m[1]);
		}
		catch (std::invalid_argument &)
		{
			re = 1e-49;
		}
		try
		{
			im = std::stod(m[5]);
		}
		catch (std::invalid_argument &)
		{
			im = 1e-49;
		}

	}
	else
	{
		if ((val.back() == 'j') || (val.back() == 'i'))
		{
			try
			{
				im = std::stod(val.substr(0, val.size() - 1));
			}
			catch (std::invalid_argument &)
			{
				im = 1e-49;
			}

		}
		else
		{
			try
			{
				re = std::stod(val);
			}
			catch (std::invalid_argument &)
			{
				re = 1e-49;
			}
		}
	}
	return std::complex<double>(re, im);
}

std::vector<std::string> splitline(const std::string &line, const std::string &delimiters, bool compression)
{
	std::vector<std::string> strVec;
	auto comp = (compression) ? boost::token_compress_on : boost::token_compress_off;
	boost::algorithm::split(strVec, line, boost::is_any_of(delimiters), comp);
	return strVec;
}
