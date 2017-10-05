/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "helicsTypes.hpp"
#include <map>
#include <regex>
#include <boost/lexical_cast.hpp>

namespace helics
{
static const std::string doubleString("double");
static const std::string intString("int64");
static const std::string stringString("string");
static const std::string complexString("complex");
static const std::string doubleVecString("double_vector");
static const std::string complexVecString("complex_vector");
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
	case helicsType_t::helicsComplexVector:
		return complexVecString;
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

static const std::map<std::string, helicsType_t> typeMap{
  {"double", helicsType_t::helicsDouble},
  {"string", helicsType_t::helicsString},
  {"float", helicsType_t::helicsDouble},
  {"vector", helicsType_t::helicsVector},
  {"double_vector", helicsType_t::helicsVector},
  {"complex", helicsType_t::helicsComplex},
  {"int", helicsType_t::helicsInt},
  {"int64", helicsType_t::helicsInt},
  {"complex_vector", helicsType_t::helicsComplexVector},
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

// regular expression to handle complex numbers of various formats
const std::regex creg(
	"([+-]?(\\d+(\\.\\d+)?|\\.\\d+)([eE][+-]?\\d+)?)\\s*([+-]\\s*(\\d+(\\.\\d+)?|\\.\\d+)([eE][+-]?\\d+)?)[ji]*");

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
	try
	{
		if (m.size() == 9)
		{
			re = boost::lexical_cast<double> (m[1]);

			im = boost::lexical_cast<double> (m[6]);

			if (*m[5].first == '-')
			{
				im = -im;
			}
		}
		else
		{
			if ((val.back() == 'j') || (val.back() == 'i'))
			{
				im = boost::lexical_cast<double> (val.substr(0, val.size() - 1));
			}
			else
			{
				re = boost::lexical_cast<double> (val);
			}
		}
	}
	catch (const boost::bad_lexical_cast &)
	{
		im = 0.0;
	}
	return std::complex<double>(re, im);
}

std::string helicsVectorString(const std::vector<double> &val)
{
	std::string vString("v");
	vString.append(std::to_string(val.size()));
	vString.push_back('[');
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

std::string helicsComplexVectorString(const std::vector<std::complex<double>> &val)
{
	std::string vString("c");
	vString.append(std::to_string(val.size()));
	vString.push_back('[');
	for (const auto &v : val)
	{
		vString.append(helicsComplexString(v.real(), v.imag()));
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
	std::vector<double> V;
	helicsGetVector(val, V);
	return V;
}

std::vector<std::complex<double>> helicsGetComplexVector(const std::string &val)
{
	std::vector<std::complex<double>> V;
	helicsGetComplexVector(val, V);
	return V;
}

static auto readSize(const std::string &val)
{
	auto fb = val.find_first_of('[');
	auto size = std::stoull(val.substr(1, fb - 1));
	return size;
}

void helicsGetVector(const std::string &val, std::vector<double> &data)
{
	if (val.empty())
	{
		data.resize(0);
	}
	if (val.front() == 'v')
	{
		auto sz = readSize(val);
		data.reserve(sz);
		data.resize(0);
		auto fb = val.find_first_of('[');
		for (decltype(sz) ii = 0; ii < sz; ++ii)
		{
			auto nc = val.find_first_of(",]", fb + 1);
			try
			{
				auto V = boost::lexical_cast<double>(val.data() + fb + 1, nc - fb - 1);
				data.push_back(V);
			}
			catch (const boost::bad_lexical_cast &)
			{
				data.push_back(-1e49);
			}
			fb = nc;
		}
	}
	else if (val.front() == 'c')
	{
		auto sz = readSize(val);
		data.reserve(sz*2);
		data.resize(0);
		auto fb = val.find_first_of('[');
		for (decltype(sz) ii = 0; ii < sz; ++ii)
		{
			auto nc = val.find_first_of(",]", fb + 1);
			auto V = helicsGetComplex(val.substr(fb+1,nc-fb-1));
			data.push_back(V.real());
			data.push_back(V.imag());
			fb = nc;
		}
	}
	else
	{
			auto V = helicsGetComplex(val);
			if (V.imag() == 0)
			{
				data.resize(1);
				data[0] = V.real();
			}
			else
			{
				data.resize(2);
				data[0] = V.real();
				data[1] = V.imag();
			}
		
	}
	return;
}

void helicsGetComplexVector(const std::string &val, std::vector<std::complex<double>> &data)
{
if (val.empty())
{
	data.resize(0);
}
if (val.front() == 'v')
{
	auto sz = readSize(val);
	data.reserve(sz/2);
	data.resize(0);
	auto fb = val.find_first_of('[');
	for (decltype(sz) ii = 0; ii < sz-1; ii+=2)
	{
		auto nc = val.find_first_of(",]", fb + 1);
		auto nc2 = val.find_first_of(",]", nc + 1);
		try
		{
			auto V1 = boost::lexical_cast<double>(val.data() + fb + 1, nc - fb - 1);
			auto V2 = boost::lexical_cast<double>(val.data() + nc + 1, nc2 - nc - 1);
			data.emplace_back(V1,V2);
		}
		catch (const boost::bad_lexical_cast &)
		{
			data.push_back(-1e49);
		}
		fb = nc;
	}
}
else if (val.front() == 'c')
{
	auto sz = readSize(val);
	data.reserve(sz);
	data.resize(0);
	auto fb = val.find_first_of('[');
	for (decltype(sz) ii = 0; ii < sz; ++ii)
	{
		auto nc = val.find_first_of(",]", fb + 1);
		auto V = helicsGetComplex(val.substr(fb + 1, nc - fb - 1));
		data.push_back(V);
		fb = nc;
	}
}
else
{
	auto V = helicsGetComplex(val);
	if (V.imag() == 0)
	{
		data.resize(1);
		data[0] = V.real();
	}
	else
	{
		data.resize(2);
		data[0] = V.real();
		data[1] = V.imag();
	}
}
return;
}
}  // namespace helics