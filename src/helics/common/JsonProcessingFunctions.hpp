/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

/** @file
@details functions related to loading and evaluating JSON files and helper functions for reading
them using the jsoncpp library
*/

#include "../core/helicsTime.hpp"

#include <functional>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>

namespace helics::fileops {

/** class to make it easier to contain certain things and remove need for including the actual json
 * library in the public headers*/
class JsonBuffer {
  public:
    JsonBuffer(const nlohmann::json& jref): reference(jref) {}

    const nlohmann::json& json() const { return reference; }

  private:
    const nlohmann::json& reference;
};

/** class to make it easier to contain certain things and remove need for including the actual json
 * library in the public headers*/
class JsonStorage {
  public:
    JsonStorage(const nlohmann::json& jref): jStorage(jref) {}
    nlohmann::json& json() { return jStorage; }
    const nlohmann::json& json() const { return jStorage; }

  private:
    nlohmann::json jStorage{};
};

/** check if the file has a valid JSON extension*/
bool hasJsonExtension(std::string_view jsonString);
/** check if the string looks like a possible config object in json format*/
bool looksLikeConfigJson(std::string_view jsonString);
/** load a JSON string or filename that points to a JSON file and return a
nlohmann::json to the root object
*/
nlohmann::json loadJson(const std::string& jsonString);

/** load a JSON object in a string
 */
nlohmann::json loadJsonStr(std::string_view jsonString);

/** read a time from a JSON value element*/
helics::Time loadJsonTime(const nlohmann::json& timeElement,
                          time_units defaultUnits = time_units::sec);

/** get a name or key from the element*/
std::string getName(const nlohmann::json& element);

/** generate a Json String*/
std::string generateJsonString(const nlohmann::json& block, bool hexConvert = true);

inline std::string JsonAsString(const nlohmann::json& element)
{
    return (element.is_string()) ? element.get<std::string>() : generateJsonString(element);
}

inline std::string
    getOrDefault(const nlohmann::json& element, const std::string& key, std::string_view defVal)
{
    return (element.contains(key)) ? JsonAsString(element[key]) : std::string(defVal);
}

inline double getOrDefault(const nlohmann::json& element, const std::string& key, double defVal)
{
    return (element.contains(key)) ? element[key].get<double>() : defVal;
}

inline bool getOrDefault(const nlohmann::json& element, const std::string& key, bool defVal)
{
    return (element.contains(key)) ? element[key].get<bool>() : defVal;
}

inline int64_t getOrDefault(const nlohmann::json& element, const std::string& key, int64_t defVal)
{
    return (element.contains(key)) ? element[key].get<int64_t>() : defVal;
}

inline bool callIfMember(const nlohmann::json& element,
                         const std::string& key,
                         const std::function<void(const std::string&, helics::Time)>& call)
{
    bool isMember = element.contains(key);
    if (isMember) {
        call(key, loadJsonTime(element[key]));
    }
    return isMember;
}

inline bool callIfMember(const nlohmann::json& element,
                         const std::string& key,
                         const std::function<void(const std::string&, bool)>& call)
{
    if (element.contains(key)) {
        call(key, element[key].get<bool>());
        return true;
    }
    return false;
}

inline bool callIfMember(const nlohmann::json& element,
                         const std::string& key,
                         const std::function<void(const std::string&, int)>& call)
{
    if (element.contains(key)) {
        call(key, element[key].get<int>());
        return true;
    }
    return false;
}

inline bool callIfMember(const nlohmann::json& element,
                         const std::string& key,
                         const std::function<void(const std::string&)>& call)
{
    if (element.contains(key)) {
        call(element[key].get<std::string>());
        return true;
    }
    return false;
}

inline void
    replaceIfMember(const nlohmann::json& element, const std::string& key, helics::Time& timeVal)
{
    if (element.contains(key)) {
        timeVal = loadJsonTime(element[key]);
    }
}

inline void
    replaceIfMember(const nlohmann::json& element, const std::string& key, std::string& sval)
{
    if (element.contains(key)) {
        sval = element[key].get<std::string>();
    }
}

inline void replaceIfMember(const nlohmann::json& element, const std::string& key, bool& bval)
{
    if (element.contains(key)) {
        bval = element[key].get<bool>();
    }
}

inline void replaceIfMember(const nlohmann::json& element, const std::string& key, int& sval)
{
    if (element.contains(key)) {
        sval = element[key].get<int>();
    }
}

inline void replaceIfMember(const nlohmann::json& element, const std::string& key, double& sval)
{
    if (element.contains(key)) {
        sval = element[key].get<double>();
    }
}

}  // namespace helics::fileops
