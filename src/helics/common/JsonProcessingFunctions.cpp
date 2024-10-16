/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "JsonProcessingFunctions.hpp"

#include "../core/helicsTime.hpp"
#include "../utilities/string_viewOps.h"
#include "../utilities/timeStringOps.hpp"

#include <fstream>
#include <memory>
#include <sstream>
#include <string>

namespace helics::fileops {

static std::string_view removeSpaceAndComment(std::string_view jsonString)
{
    gmlc::utilities::string_viewOps::trimString(jsonString);
    while (jsonString.size() > 2) {
        if (jsonString[0] == '/' && jsonString[1] == '/') {
            auto nextNewLine = jsonString.find_first_of('\n');
            if (nextNewLine == std::string_view::npos) {
                return {};
            }
            jsonString.remove_prefix(nextNewLine + 1);
            gmlc::utilities::string_viewOps::trimString(jsonString);
        } else {
            break;
        }
    }
    gmlc::utilities::string_viewOps::trimString(jsonString);
    return jsonString;
}

bool looksLikeConfigJson(std::string_view jsonString)
{
    if (jsonString.find("\n#") != std::string_view::npos) {
        return false;
    }
    jsonString = removeSpaceAndComment(jsonString);
    if (jsonString.size() < 7) {
        // minimum viable config file is 7 characters {"f":4}
        return false;
    }

    if (jsonString.front() != '{') {
        return false;
    }
    auto firstQuote = jsonString.find_first_of("\"'");
    if (firstQuote == std::string_view::npos) {
        return false;
    }
    auto firstColonLoc = jsonString.find_first_of(':');
    if (firstColonLoc == std::string_view::npos) {
        return false;
    }
    auto closeBracket = jsonString.find_last_of('}');
    if (closeBracket == std::string_view::npos) {
        return false;
    }

    auto afterBracket = removeSpaceAndComment(jsonString.substr(closeBracket + 1));
    return afterBracket.empty();
}

bool hasJsonExtension(std::string_view jsonString)
{
    auto ext = jsonString.substr(jsonString.length() - 4);
    return ((ext == "json") || (ext == "JSON") || (ext == ".jsn") || (ext == ".JSN"));
}

nlohmann::json loadJson(const std::string& jsonString)
{
    if (jsonString.size() > 128) {
        try {
            return loadJsonStr(jsonString);
        }
        catch (const std::invalid_argument&) {
            // this was a guess lets try a file now, the same error will be generated again later as
            // well
        }
    }
    std::ifstream file(jsonString);

    if (file.is_open()) {
        try {
            return nlohmann::json::parse(file, nullptr, true, true);
        }
        catch (const nlohmann::json::parse_error& errs) {
            throw(std::invalid_argument(errs.what()));
        }
    }
    return loadJsonStr(jsonString);
}

nlohmann::json loadJsonStr(std::string_view jsonString)
{
    try {
        return nlohmann::json::parse(jsonString.begin(), jsonString.end(), nullptr, true, true);
    }
    catch (const nlohmann::json::parse_error& errs) {
        throw(std::invalid_argument(errs.what()));
    }
}

/** read a time from a JSON value element*/
helics::Time loadJsonTime(const nlohmann::json& timeElement, time_units defaultUnits)
{
    if (timeElement.is_object()) {
        if (timeElement.contains("unit")) {
            defaultUnits =
                gmlc::utilities::timeUnitsFromString(timeElement["unit"].get<std::string>());
        }
        if (timeElement.contains("units")) {
            defaultUnits =
                gmlc::utilities::timeUnitsFromString(timeElement["units"].get<std::string>());
        }
        if (timeElement.contains("value")) {
            if (timeElement["value"].is_number_integer()) {
                return {timeElement["value"].get<int64_t>(), defaultUnits};
            }
            return {timeElement["value"].get<double>() * toSecondMultiplier(defaultUnits)};
        }
        return helics::Time::minVal();
    }
    if (timeElement.is_number_integer() || timeElement.is_number_unsigned()) {
        return {timeElement.get<std::int64_t>(), defaultUnits};
    }
    if (timeElement.is_number_float()) {
        return {timeElement.get<double>() * toSecondMultiplier(defaultUnits)};
    }
    return gmlc::utilities::loadTimeFromString<helics::Time>(timeElement.get<std::string>());
}

std::string getName(const nlohmann::json& element)
{
    return (element.contains("key")) ?
        element["key"].get<std::string>() :
        ((element.contains("name")) ? element["name"].get<std::string>() : std::string());
}

std::string generateJsonString(const nlohmann::json& block, bool hexConvert)
{
    return block.dump(3,
                      ' ',
                      true,
                      hexConvert ? nlohmann::json::error_handler_t::hex :
                                   nlohmann::json::error_handler_t::strict);
}

}  // namespace helics::fileops
