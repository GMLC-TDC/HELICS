/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

/** @file
@details functions related to loading and evaluating JSON files and helper functions for reading them
using the jsoncpp library
*/

#include "json/jsoncpp.h"
#include <functional>

#include "../core/helics-time.hpp"
/** load a JSON string or filename that points to a JSON file and return a
json::Value to the root object
*/
Json_helics::Value loadJson (const std::string &jsonString);

/** load a JSON object in a string
 */
Json_helics::Value loadJsonStr (const std::string &jsonString);

/** read a time from a JSON value element*/
helics::Time loadJsonTime (const Json_helics::Value &timeElement, timeUnits defaultUnits = timeUnits::sec);

/** get a name or key from the element*/
std::string getKey (const Json_helics::Value &element);

inline std::string
jsonGetOrDefault (const Json_helics::Value &element, const std::string &key, const std::string &defVal)
{
    return (element.isMember (key)) ? element[key].asString () : defVal;
}

inline double jsonGetOrDefault (const Json_helics::Value &element, const std::string &key, double defVal)
{
    return (element.isMember (key)) ? element[key].asDouble () : defVal;
}

inline int64_t jsonGetOrDefault (const Json_helics::Value &element, const std::string &key, int64_t defVal)
{
    return (element.isMember (key)) ? element[key].asInt64 () : defVal;
}

inline void jsonCallIfMember (const Json_helics::Value &element,
                              const std::string &key,
                              const std::function<void(const std::string &, helics::Time)> &call)
{
    if (element.isMember (key))
    {
        call(key,loadJsonTime (element[key]));
    }
}

inline void jsonCallIfMember (const Json_helics::Value &element,
                              const std::string &key,
                              const std::function<void(const std::string &, bool)> &call)
{
    if (element.isMember (key))
    {
        call (key, element[key].asBool ());
    }
}

inline void jsonCallIfMember (const Json_helics::Value &element,
                              const std::string &key,
                              const std::function<void(const std::string &, int)> &call)
{
    if (element.isMember (key))
    {
        call (key, element[key].asInt ());
    }
}

inline void jsonReplaceIfMember (const Json_helics::Value &element, const std::string &key, helics::Time &timeVal)
{
    if (element.isMember (key))
    {
        timeVal = loadJsonTime (element[key]);
    }
}

inline void jsonReplaceIfMember (const Json_helics::Value &element, const std::string &key, std::string &sval)
{
    if (element.isMember (key))
    {
        sval = element[key].asString ();
    }
}

inline void jsonReplaceIfMember (const Json_helics::Value &element, const std::string &key, bool &bval)
{
    if (element.isMember (key))
    {
        bval = element[key].asBool ();
    }
}

/** generate a Json String*/
std::string generateJsonString (const Json_helics::Value &block);