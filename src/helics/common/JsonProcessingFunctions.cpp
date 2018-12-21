/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "JsonProcessingFunctions.hpp"
#include <fstream>

bool hasJsonExtension (const std::string &jsonString)
{
    auto ext = jsonString.substr (jsonString.length () - 4);
    return ((ext == "json") || (ext == "JSON") || (ext == ".jsn") || (ext == ".JSN"));
}

Json_helics::Value loadJson (const std::string &jsonString)
{
    if (jsonString.size() > 128)
    {
        try
        {
            return loadJsonStr(jsonString);
        }
        catch (const std::invalid_argument &)
        {
            //this was a guess lets try a file now, the same error will be generated again later as well
        }
    }
    std::ifstream file (jsonString);
    
    if (file.is_open ())
    {
        Json_helics::Value doc;
        Json_helics::CharReaderBuilder rbuilder;
        std::string errs;
        bool ok = Json_helics::parseFromStream (rbuilder, file, &doc, &errs);
        if (!ok)
        {
            throw (std::invalid_argument (errs.c_str ()));
        }
        return doc;
    }
    return loadJsonStr(jsonString);
}

Json_helics::Value loadJsonStr(const std::string &jsonString)
{
    Json_helics::Value doc;
    Json_helics::CharReaderBuilder rbuilder;
    std::string errs;
    std::istringstream jstring(jsonString);
    bool ok = Json_helics::parseFromStream(rbuilder, jstring, &doc, &errs);
    if (!ok)
    {
        throw (std::invalid_argument(errs.c_str()));
    }
    return doc;
}

/** read a time from a JSON value element*/
helics::Time loadJsonTime (const Json_helics::Value &timeElement, timeUnits defaultUnits)
{
    if (timeElement.isObject ())
    {
        if (timeElement.isMember ("units"))
        {
            defaultUnits = helics::timeUnitsFromString (timeElement["units"].asString ());
        }
        if (timeElement.isMember ("value"))
        {
            if (timeElement["value"].isInt64 ())
            {
                return helics::Time (timeElement["value"].asInt64 (), defaultUnits);
            }
            return helics::Time (timeElement["value"].asDouble () * toSecondMultiplier (defaultUnits));
        }
    }
    else if (timeElement.isInt64 ())
    {
        return helics::Time (timeElement.asInt64 (), defaultUnits);
    }
    else if (timeElement.isDouble ())
    {
        return helics::Time (timeElement.asDouble () * toSecondMultiplier (defaultUnits));
    }
    else
    {
        return helics::loadTimeFromString (timeElement.asString ());
    }
    return helics::Time::minVal ();
}

std::string getKey (const Json_helics::Value &element)
{
    return (element.isMember ("key")) ?
             element["key"].asString () :
             ((element.isMember ("name")) ? element["name"].asString () : std::string ());
}

std::string generateJsonString(const Json_helics::Value &block)
{
    Json_helics::StreamWriterBuilder builder;
    builder["commentStyle"] = "None";
    builder["indentation"] = "   ";  // or whatever you like
    auto writer(builder.newStreamWriter());
    std::stringstream sstr;
    writer->write(block, &sstr);
    return sstr.str();
}
