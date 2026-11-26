/*
Copyright (c) 2017-2026,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Energy
Innovation LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "TomlProcessingFunctions.hpp"

#include "../core/core-exceptions.hpp"
#include "../utilities/timeStringOps.hpp"

#include <chrono>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace helics::fileops {

bool hasTomlExtension(std::string_view tomlString)
{
    auto ext = tomlString.substr(tomlString.length() - 4);
    return ((ext == "toml") || (ext == "TOML") || (ext == ".ini") || (ext == ".INI"));
}

bool looksLikeConfigToml(std::string_view tomlString)
{
    if (tomlString.empty()) {
        return false;
    }
    if (tomlString.find("[[") != std::string_view::npos) {
        return true;
    }

    return (tomlString.find_first_of('=') != std::string_view::npos);
}

static std::invalid_argument tomlParseError(const toml::parse_error& error)
{
    std::ostringstream message;
    message << error;
    return std::invalid_argument(message.str());
}

toml::table loadToml(const std::string& tomlString)
{
    std::ifstream file;
    if (tomlString.size() > 128) {
        try {
            return loadTomlStr(tomlString);
        }
        catch (const std::invalid_argument&) {
            file.open(tomlString, std::ios_base::binary);
            if (!file.is_open()) {
                throw;
            }
        }
    } else {
        file.open(tomlString, std::ios_base::binary);
        if (!file.is_open()) {
            return loadTomlStr(tomlString);
        }
    }

    try {
        return toml::parse(file, tomlString);
    }
    catch (const toml::parse_error& error) {
        throw tomlParseError(error);
    }
}

toml::table loadTomlStr(const std::string& tomlString)
{
    try {
        return toml::parse(tomlString);
    }
    catch (const toml::parse_error& error) {
        throw tomlParseError(error);
    }
}

helics::Time loadTomlTime(const toml::node& timeElement, time_units defaultUnits)
{
    if (const auto* table = timeElement.as_table()) {
        const auto unit = (*table)["unit"].value_or(std::string_view{});
        if (!unit.empty()) {
            defaultUnits = gmlc::utilities::timeUnitsFromString(unit);
        }
        const auto units = (*table)["units"].value_or(std::string_view{});
        if (!units.empty()) {
            defaultUnits = gmlc::utilities::timeUnitsFromString(units);
        }
        if (const auto* val = table->get("value")) {
            if (const auto* integer = val->as_integer()) {
                return {integer->get(), defaultUnits};
            }
            if (const auto* floating = val->as_floating_point()) {
                return {floating->get() * toSecondMultiplier(defaultUnits)};
            }
            return gmlc::utilities::loadTimeFromString<helics::Time>(tomlAsString(*val) + " " +
                                                                     std::string(units));
        }
        return helics::Time::minVal();
    }
    if (const auto* integer = timeElement.as_integer()) {
        return {integer->get(), defaultUnits};
    }
    if (const auto* floating = timeElement.as_floating_point()) {
        return {floating->get() * toSecondMultiplier(defaultUnits)};
    }
    if (const auto* time = timeElement.as_time()) {
        const auto& value = time->get();
        return {std::chrono::hours(value.hour) + std::chrono::minutes(value.minute) +
                std::chrono::seconds(value.second) + std::chrono::nanoseconds(value.nanosecond)};
    }
    return gmlc::utilities::loadTimeFromString<helics::Time>(tomlAsString(timeElement));
}

std::string getName(const toml::node& element)
{
    const toml::node_view view{element};
    const auto key = view["key"].value_or(std::string_view{});
    return key.empty() ? view["name"].value_or(std::string{}) : std::string(key);
}

std::string tomlAsString(const toml::node& element)
{
    if (const auto* str = element.as_string()) {
        return str->get();
    }
    if (const auto* floating = element.as_floating_point()) {
        return std::to_string(floating->get());
    }
    if (const auto* integer = element.as_integer()) {
        return std::to_string(integer->get());
    }

    std::ostringstream str;
    str << toml::toml_formatter{element};
    return str.str();
}

const toml::array* getTomlArray(const toml::table& doc, std::string_view key)
{
    if (const auto member = doc[key]) {
        if (const auto* array = member.as_array()) {
            return array;
        }
        throw InvalidParameter(std::string(key) + " section in toml file must be an array");
    }
    return nullptr;
}

const std::string& getTomlString(const toml::node& element)
{
    if (const auto* value = element.as_string()) {
        return value->get();
    }
    throw InvalidParameter("expected a TOML string");
}

std::pair<std::string_view, std::string_view> getTomlStringPair(const toml::array& array)
{
    const auto* first = array.get_as<std::string>(0);
    const auto* second = array.get_as<std::string>(1);
    if (first == nullptr || second == nullptr) {
        throw InvalidParameter("expected a pair of TOML strings");
    }
    return {first->get(), second->get()};
}

void loadTomlPairs(const toml::table& doc,
                   std::string_view key,
                   const std::function<void(std::string_view, std::string_view)>& pairAction)
{
    if (const auto member = doc[key]) {
        if (const auto* array = member.as_array()) {
            for (const auto& pair : *array) {
                const auto* entries = pair.as_array();
                if (entries == nullptr) {
                    throw InvalidParameter("expected an array of TOML string pairs");
                }
                const auto [first, second] = getTomlStringPair(*entries);
                pairAction(first, second);
            }
        } else if (const auto* table = member.as_table()) {
            for (const auto& [name, value] : *table) {
                pairAction(name.str(), getTomlString(value));
            }
        } else {
            throw InvalidParameter(std::string(key) +
                                   " section in toml file must be a table or an array");
        }
    }
}

void replaceIfMember(const toml::node& element, std::string_view key, bool& loc)
{
    const auto member = toml::node_view{element}[key];
    if (member) {
        const auto* value = member.as_boolean();
        if (value == nullptr) {
            throw InvalidParameter(std::string(key) + " must be a TOML boolean");
        }
        loc = value->get();
    }
}
}  // namespace helics::fileops
