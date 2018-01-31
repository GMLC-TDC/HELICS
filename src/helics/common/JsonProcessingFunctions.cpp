/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "JsonProcessingFunctions.hpp"
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

/** read a time from a JSON value element*/
helics::Time loadJsonTime(const Json_helics::Value &timeElement, timeUnits defaultUnits)
{
    if (timeElement.isObject())
    {
        if (timeElement.isMember("units"))
        {
            defaultUnits = helics::timeUnitsFromString(timeElement["units"].asString());
        }
        if (timeElement.isMember("value"))
        {
            if (timeElement["value"].isInt64())
            {
                return helics::Time(timeElement["value"].asInt64(), defaultUnits);
            }
            else
            {
                return helics::Time(timeElement["value"].asDouble()*toSecondMultiplier(defaultUnits));
            }
        }
    }
    else if (timeElement.isInt64())
    {
        return helics::Time(timeElement.asInt64(), defaultUnits);
    }
    else if (timeElement.isDouble())
    {
        return helics::Time(timeElement.asDouble()*toSecondMultiplier(defaultUnits));
    }
    else
    {
        return helics::loadTimeFromString(timeElement.asString());

    }
    return 0;
}


