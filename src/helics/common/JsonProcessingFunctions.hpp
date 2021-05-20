/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

/** @file
@details functions related to loading and evaluating JSON files and helper functions for reading
them using the jsoncpp library
*/

#include "../core/helics-time.hpp"

#include "json/json.h"
#include <functional>
#include <string>
/** check if the file has a valid JSON extension*/
bool hasJsonExtension(const std::string& jsonString);

/** load a JSON string or filename that points to a JSON file and return a
JSON::Value to the root object
*/
Json::Value loadJson(const std::string& jsonString);

/** load a JSON object in a string
 */
Json::Value loadJsonStr(const std::string& jsonString);

/** read a time from a JSON value element*/
helics::Time loadJsonTime(const Json::Value& timeElement,
                          time_units defaultUnits = time_units::sec);

/** get a name or key from the element*/
std::string getKey(const Json::Value& element);

/** generate a Json String*/
std::string generateJsonString(const Json::Value& block);

inline std::string JsonAsString(const Json::Value& element)
{
    return (element.isString()) ? element.asString() : generateJsonString(element);
}

inline std::string
    getOrDefault(const Json::Value& element, const std::string& key, const std::string& defVal)
{
    return (element.isMember(key)) ? JsonAsString(element[key]) : defVal;
}

inline double getOrDefault(const Json::Value& element, const std::string& key, double defVal)
{
    return (element.isMember(key)) ? element[key].asDouble() : defVal;
}

inline bool getOrDefault(const Json::Value& element, const std::string& key, bool defVal)
{
    return (element.isMember(key)) ? element[key].asBool() : defVal;
}

inline int64_t getOrDefault(const Json::Value& element, const std::string& key, int64_t defVal)
{
    return (element.isMember(key)) ? element[key].asInt64() : defVal;
}

inline bool callIfMember(const Json::Value& element,
                         const std::string& key,
                         const std::function<void(const std::string&, helics::Time)>& call)
{
    if (element.isMember(key)) {
        call(key, loadJsonTime(element[key]));
        return true;
    }
    return false;
}

inline bool callIfMember(const Json::Value& element,
                         const std::string& key,
                         const std::function<void(const std::string&, bool)>& call)
{
    if (element.isMember(key)) {
        call(key, element[key].asBool());
        return true;
    }
    return false;
}

inline bool callIfMember(const Json::Value& element,
                         const std::string& key,
                         const std::function<void(const std::string&, int)>& call)
{
    if (element.isMember(key)) {
        call(key, element[key].asInt());
        return true;
    }
    return false;
}

inline bool callIfMember(const Json::Value& element,
                         const std::string& key,
                         const std::function<void(const std::string&)>& call)
{
    if (element.isMember(key)) {
        call(element[key].asString());
        return true;
    }
    return false;
}

inline void
    replaceIfMember(const Json::Value& element, const std::string& key, helics::Time& timeVal)
{
    if (element.isMember(key)) {
        timeVal = loadJsonTime(element[key]);
    }
}

inline void replaceIfMember(const Json::Value& element, const std::string& key, std::string& sval)
{
    if (element.isMember(key)) {
        sval = element[key].asString();
    }
}

inline void replaceIfMember(const Json::Value& element, const std::string& key, bool& bval)
{
    if (element.isMember(key)) {
        bval = element[key].asBool();
    }
}

inline void replaceIfMember(const Json::Value& element, const std::string& key, int& sval)
{
    if (element.isMember(key)) {
        sval = element[key].asInt();
    }
}
