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

Json::Value loadJson(const std::string& jsonString)
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
        Json::Value doc;
        const Json::CharReaderBuilder rbuilder;
        std::string errs;
        const bool success = Json::parseFromStream(rbuilder, file, &doc, &errs);
        if (!success) {
            throw(std::invalid_argument(errs.c_str()));
        }
        return doc;
    }
    return loadJsonStr(jsonString);
}

Json::Value loadJsonStr(std::string_view jsonString)
{
    Json::Value doc;
    const Json::CharReaderBuilder rbuilder;
    std::string errs;
    auto reader = std::unique_ptr<Json::CharReader>(rbuilder.newCharReader());
    const bool parseOk =
        reader->parse(jsonString.data(), jsonString.data() + jsonString.size(), &doc, &errs);
    if (!parseOk) {
        throw(std::invalid_argument(errs.c_str()));
    }
    return doc;
}

/** read a time from a JSON value element*/
helics::Time loadJsonTime(const Json::Value& timeElement, time_units defaultUnits)
{
    if (timeElement.isObject()) {
        if (timeElement.isMember("unit")) {
            defaultUnits = gmlc::utilities::timeUnitsFromString(timeElement["unit"].asString());
        }
        if (timeElement.isMember("units")) {
            defaultUnits = gmlc::utilities::timeUnitsFromString(timeElement["units"].asString());
        }
        if (timeElement.isMember("value")) {
            if (timeElement["value"].isInt64()) {
                return {timeElement["value"].asInt64(), defaultUnits};
            }
            return {timeElement["value"].asDouble() * toSecondMultiplier(defaultUnits)};
        }
        return helics::Time::minVal();
    }
    if (timeElement.isInt64()) {
        return {timeElement.asInt64(), defaultUnits};
    }
    if (timeElement.isDouble()) {
        return {timeElement.asDouble() * toSecondMultiplier(defaultUnits)};
    }
    return gmlc::utilities::loadTimeFromString<helics::Time>(timeElement.asString());
}

std::string getName(const Json::Value& element)
{
    return (element.isMember("key")) ?
        element["key"].asString() :
        ((element.isMember("name")) ? element["name"].asString() : std::string());
}

std::string generateJsonString(const Json::Value& block)
{
    Json::StreamWriterBuilder builder;
    builder["emitUTF8"] = true;
    builder["commentStyle"] = "None";
    builder["indentation"] = "   ";  // or whatever you like
    builder["precision"] = 17;
    auto writer = std::unique_ptr<Json::StreamWriter>(builder.newStreamWriter());
    std::stringstream sstr;
    writer->write(block, &sstr);
    auto ret = sstr.str();
    return ret;
}

}  // namespace helics::fileops
