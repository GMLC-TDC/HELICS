/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "JsonBuilder.hpp"

#include "JsonProcessingFunctions.hpp"
#include "gmlc/utilities/stringOps.h"

#include <utility>

namespace helics {
using stringVector = gmlc::utilities::stringVector;

JsonMapBuilder::JsonMapBuilder() noexcept {}

JsonMapBuilder::~JsonMapBuilder() = default;

Json::Value& JsonMapBuilder::getJValue()
{
    if (!jMap) {
        jMap = std::make_unique<Json::Value>();
    }
    return *jMap;
}

bool JsonMapBuilder::isCompleted() const
{
    return (jMap) && (missing_components.empty());
}

int JsonMapBuilder::generatePlaceHolder(const std::string& location)
{
    int index = static_cast<int>(missing_components.size()) + 2;
    missing_components.emplace(index, location);
    return index;
}

bool JsonMapBuilder::addComponent(const std::string& info, int index)
{
    auto loc = missing_components.find(index);
    if (loc != missing_components.end()) {
        auto element = loadJsonStr(info);
        (*jMap)[loc->second].append(element);
        missing_components.erase(loc);

        return missing_components.empty();
    }
    return false;
}

std::string JsonMapBuilder::generate()
{
    if (jMap) {
        return generateJsonString(*jMap);
    }
    return "{}";
}

void JsonMapBuilder::reset()
{
    jMap = nullptr;
    missing_components.clear();
}

JsonBuilder::JsonBuilder() noexcept {}

JsonBuilder::~JsonBuilder() = default;
/** add an element on a specific path*/
void JsonBuilder::addElement(const std::string& path, const std::string& value)
{
    stringVector res = gmlc::utilities::stringOps::splitline(
        path, "\\/:.", gmlc::utilities::stringOps::delimiter_compression::on);
    auto jv = &getJValue();
    size_t ii = 0;
    for (ii = 0; ii < res.size() - 1; ++ii) {
        auto& sub = (*jv)[res[ii]];
        if (sub.isNull()) {
            (*jv)[res[ii]] = Json::Value();
        }
        jv = &(*jv)[res[ii]];
    }
    (*jv)[res.back()] = value;
}

void JsonBuilder::addElement(const std::string& path, double value)
{
    stringVector res = gmlc::utilities::stringOps::splitline(
        path, "\\/:.", gmlc::utilities::stringOps::delimiter_compression::on);
    auto jv = &getJValue();
    size_t ii = 0;
    for (ii = 0; ii < res.size() - 1; ++ii) {
        auto& sub = (*jv)[res[ii]];
        if (sub.isNull()) {
            (*jv)[res[ii]] = Json::Value();
        }
        jv = &(*jv)[res[ii]];
    }
    (*jv)[res.back()] = value;
}

Json::Value& JsonBuilder::getJValue()
{
    if (!jMap) {
        jMap = std::make_unique<Json::Value>();
    }
    return *jMap;
}

std::string JsonBuilder::generate()
{
    if (jMap) {
        return generateJsonString(*jMap);
    }
    return "{}";
}

void JsonBuilder::reset()
{
    jMap = nullptr;
}
} // namespace helics
