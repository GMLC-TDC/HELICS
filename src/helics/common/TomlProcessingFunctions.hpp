/*
Copyright (c) 2017-2026,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Energy
Innovation LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

/** @file
@details functions related to loading and evaluating TOML files and helper functions for reading
them using the toml library
*/

#include "../core/helicsTime.hpp"

#include <functional>
#include <string>
#include <string_view>
#include <toml++/toml.hpp>
#include <utility>

namespace helics::fileops {
/** load a TOML string or filename and return the root table */
toml::table loadToml(const std::string& tomlString);
/** return true if the string has a toml extension */
bool hasTomlExtension(std::string_view tomlString);
/** check if the string looks like a possible config object in toml format */
bool looksLikeConfigToml(std::string_view tomlString);
/** load a TOML document from a string
 * @throws std::invalid_argument if TOML parsing fails
 */
toml::table loadTomlStr(const std::string& tomlString);

/** get a toml value as a string */
std::string tomlAsString(const toml::node& element);

/** read a time from a TOML value element */
helics::Time loadTomlTime(const toml::node& timeElement, time_units defaultUnits = time_units::sec);

/** get a name or key from the element */
std::string getName(const toml::node& element);

/** return the array at key, or nullptr if it is absent */
const toml::array* getTomlArray(const toml::table& doc, std::string_view key);

/** require a string value */
const std::string& getTomlString(const toml::node& element);

/** read the first two entries of an array as strings */
std::pair<std::string_view, std::string_view> getTomlStringPair(const toml::array& array);

/** read a section that can be either a table or an array of string pairs */
void loadTomlPairs(const toml::table& doc,
                   std::string_view key,
                   const std::function<void(std::string_view, std::string_view)>& pairAction);

/** get a string value or use the default if it is not a member */
inline std::string
    getOrDefault(const toml::node& element, std::string_view key, std::string_view defVal)
{
    const auto member = toml::node_view{element}[key];
    return member ? tomlAsString(*member.node()) : std::string(defVal);
}

/** get a double value or use the default if it is absent or has a different type */
inline double getOrDefault(const toml::node& element, std::string_view key, double defVal)
{
    return toml::node_view{element}[key].value_exact<double>().value_or(defVal);
}

/** get a boolean value or use the default if it is absent or has a different type */
inline bool getOrDefault(const toml::node& element, std::string_view key, bool defVal)
{
    return toml::node_view{element}[key].value_exact<bool>().value_or(defVal);
}

/** call a function if a nonempty string member exists */
inline bool callIfMember(const toml::node& element,
                         std::string_view key,
                         const std::function<void(const std::string&)>& call)
{
    const auto* val = toml::node_view{element}[key].as_string();
    if (val != nullptr && !val->get().empty()) {
        call(val->get());
        return true;
    }
    return false;
}

/** call a function if a member exists and pass a time to the invoked object */
inline bool callIfMember(const toml::node& element,
                         const std::string& key,
                         const std::function<void(const std::string&, helics::Time)>& call)
{
    if (const auto member = toml::node_view{element}[key]) {
        call(key, loadTomlTime(*member.node()));
        return true;
    }
    return false;
}

void replaceIfMember(const toml::node& element, std::string_view key, bool& loc);

inline void replaceIfMember(const toml::node& element, std::string_view key, std::string& loc)
{
    if (const auto member = toml::node_view{element}[key]) {
        loc = tomlAsString(*member.node());
    }
}

}  // namespace helics::fileops
