/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "helicsTypes.hpp"
#include <boost/lexical_cast.hpp>
#include <regex>
#include <map>

namespace helics
{

static const std::string doubleString("double");
static const std::string intString("int64");
static const std::string stringString("string");
static const std::string complexString("complex");
static const std::string doubleVecString("double_vector");
static const std::string nullString("");


const std::string &typeNameStringRef(helicsType_t type)
{
	switch (type)
	{
	case helicsType_t::helicsDouble:
		return doubleString;
	case helicsType_t::helicsInt:
		return intString;
	case helicsType_t::helicsString:
		return stringString;
	case helicsType_t::helicsComplex:
		return complexString;
	case helicsType_t::helicsVector:
		return doubleVecString;
	default:
		return nullString;
	}
}


std::string helicsComplexString(double real, double imag)
{
	std::stringstream ss;
	ss << real;
	if (imag != 0.0)
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

static const std::map<std::string, helicsType_t> typeMap
{
	{"double",helicsType_t::helicsDouble},
	{"string",helicsType_t::helicsString},
	{"float",helicsType_t::helicsDouble},
	{"vector",helicsType_t::helicsVector},
	{"double_vector",helicsType_t::helicsVector},
	{"complex",helicsType_t::helicsComplex},
	{"int",helicsType_t::helicsInt},
	{"int64",helicsType_t::helicsInt},
};

helicsType_t getTypeFromString(const std::string &typeName)
{
	auto res = typeMap.find(typeName);
	if (res == typeMap.end())
	{
		return helicsType_t::helicsInvalid;
	}
	return res->second;
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
		re = boost::lexical_cast<double>(m[1]);
		im = boost::lexical_cast<double>(m[5]);

	}
	else
	{
		if ((val.back() == 'j') || (val.back() == 'i'))
		{
			im = boost::lexical_cast<double>(val.substr(0, val.size() - 1));

		}
		else
		{
			re = boost::lexical_cast<double>(val);
		}
	}
	return std::complex<double>(re, im);
}


std::string helicsVectorString(const std::vector<double> &val)
{
	std::string vString = "[";
	for (const auto &v : val)
	{
		vString.append(std::to_string(v));
		vString.push_back(';');
		vString.push_back(' ');
	}
	if (vString.size() > 2)
	{
		vString.pop_back();
		vString.pop_back();
	}
	vString.push_back(']');
	return vString;
}


std::vector<double> helicsGetVector(const std::string &val)
{
	return std::vector<double>();
}

void helicsGetVector(const std::string &val, std::vector<double> &data)
{

}

}