/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "Subscriptions.hpp"

namespace helics
{

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
		val = helicsComplexString(boost::get<std::complex<double>>(dv)));
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
		val = boost::lexical_cast<X>(boost::get<std::string>(v));
		break;
	case 1: //double
		val = static_cast<X>(boost::get<double>(v));
		break;
	case 2: //int64_t
		val = static_cast<X>(boost::get<int64_t>(v));
		break;
	case 3: //complex
		val = static_cast<X>(std::abs(boost::get<std::complex<double>>(v)));
		break;
	case 4: //vector
	{
		auto &vec = boost::get<std::vector<double>>(dv);
		if (!vec.empty())
		{
			val = static_cast<X>(vec.front());
		}
		else
		{
			val = static_cast<X>(-1e49);
		}
		break;
	}
}

void valueExtract(const defV &dv, std::vector<double> &val)
{
	switch (dv.which())
	{
	case 0: //string
	default:
		val = boost::lexical_cast<X>(boost::get<std::string>(v));
		break;
	case 1: //double
		val = static_cast<X>(boost::get<double>(v));
		break;
	case 2: //int64_t
		val = static_cast<X>(boost::get<int64_t>(v));
		break;
	case 3: //complex
		val = static_cast<X>(std::abs(boost::get<std::complex<double>>(v)));
		break;
	case 4: //vector
	{
		auto &vec = boost::get<std::vector<double>>(dv);
		if (!vec.empty())
		{
			val = static_cast<X>(vec.front());
		}
		else
		{
			val = static_cast<X>(-1e49);
		}
		break;
	}
}

void valueExtract(const data_view &dv, helicsType_t baseType, std::string &val)
{

}

void valueExtract(const data_view &dv, helicsType_t baseType, std::vector<double> &val)
{

}

void valueExtract(const data_view &dv, helicsType_t baseType, std::complex<double> &val)
{

}

}