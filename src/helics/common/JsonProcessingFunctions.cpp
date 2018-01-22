/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "JsonProcessingFunctions.hpp"
#include "stringOps.h"
#include <fstream>

Json_helics::Value loadJsonString(const std::string &jsonString)
{
	std::ifstream file(jsonString);
	Json_helics::Value doc;

	if (file.is_open())
	{
		Json_helics::CharReaderBuilder rbuilder;
		std::string errs;
		bool ok = Json_helics::parseFromStream(rbuilder, file, &doc, &errs);
		if (!ok)
		{
			throw(std::invalid_argument(errs.c_str()));
		}
	}
	else
	{
		Json_helics::CharReaderBuilder rbuilder;
		std::string errs;
		std::istringstream jstring(jsonString);
		bool ok = Json_helics::parseFromStream(rbuilder, jstring, &doc, &errs);
		if (!ok)
		{
			throw(std::invalid_argument(errs.c_str()));
		}
	}
	return doc;
}


constexpr double unitMult[] = { 1e-12,1e-9,1e-6,1e-3,1.0,1.0,60.0,3600.0,86400.0 };

const std::map<std::string, timeUnits> timeUnitStrings
{
	{"ps",timeUnits::ps},
	{"ns",timeUnits::ns},
	{"us",timeUnits::us},
	{"ms",timeUnits::ms},
	{"s",timeUnits::s},
	{"sec",timeUnits::sec},
	{"seconds",timeUnits::sec},
	{"second",timeUnits::sec},
	{"min",timeUnits::minutes},
	{"minute",timeUnits::minutes},
	{"minute",timeUnits::minutes},
	{"hr",timeUnits::hr},
	{"hour",timeUnits::hr},
	{"hours",timeUnits::hr},
	{"day",timeUnits::day}
};

timeUnits timeUnitsFromString(const std::string &unitString)
{
	auto fnd = timeUnitStrings.find(unitString);
	if (fnd != timeUnitStrings.end())
	{
		return fnd->second;
	}
	auto lcUstring = convertToLowerCase(stringOps::trim(unitString));
	fnd = timeUnitStrings.find(lcUstring);
	if (fnd != timeUnitStrings.end())
	{
		return fnd->second;
	}
	throw(std::invalid_argument(std::string("unit ") + unitString + " not recognized"));
}
/** read a time from a JSON value element*/
helics::Time loadJsonTime(const Json_helics::Value &timeElement)
{
	if (timeElement.isObject())
	{
		std::string units = "s";
		if (timeElement.isMember("units"))
		{
			units = timeElement["units"].asString();
		}
		if (timeElement.isMember("value"))
		{
			if (timeElement["value"].isInt64())
			{
				return helics::Time(timeElement["value"].asInt64(), timeUnitsFromString(units));
			}
			else
			{
				return helics::Time(timeElement["value"].asDouble()*unitMult[static_cast<int>(timeUnitsFromString(units))]);
			}
		}
	}
	else if (timeElement.isDouble())
	{
		return helics::Time(timeElement.asDouble());
	}
	else
	{
		return loadTimeFromString(timeElement.asString());
		
	}
	return 0;
}


/** read a time from a JSON value element*/
helics::Time loadTimeFromString(const std::string &timeString)
{
	size_t pos;
	double val = std::stod(timeString, &pos);
	while ((pos < timeString.size()) && (timeString[pos] != ' '))
	{
		++pos;
	}
	std::string units = timeString.substr(pos);
	return helics::Time(val*unitMult[static_cast<int>(timeUnitsFromString(units))]);
}
