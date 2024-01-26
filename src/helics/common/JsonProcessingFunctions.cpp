/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "JsonProcessingFunctions.hpp"

#include "../core/helicsTime.hpp"
#include "../utilities/timeStringOps.hpp"

#include <fstream>
#include <memory>
#include <sstream>

namespace helics::fileops {

bool looksLikeCommandLine(std::string_view testString)
{
    if (testString.empty()) {
        return false;
    }
    if (testString.front() == '-') {
        return true;
    }
    return testString.find(" -") != std::string::npos;
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
        const bool ok = Json::parseFromStream(rbuilder, file, &doc, &errs);
        if (!ok) {
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
    const bool ok =
        reader->parse(jsonString.data(), jsonString.data() + jsonString.size(), &doc, &errs);
    if (!ok) {
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
