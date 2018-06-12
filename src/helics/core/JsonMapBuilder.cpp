/*
Copyright � 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#pragma once
#include "../common/JsonProcessingFunctions.hpp"
#include "JsonMapBuilder.hpp"

namespace helics
{

    Json_helics::Value &JsonMapBuilder::getJValue()
    {
        if (!jMap)
        {
            jMap = std::make_unique<Json_helics::Value>();
        }
        return *jMap;
    }

    bool JsonMapBuilder::isCompleted() const
    {
        return (jMap) && (missing_components.empty());
    }

    int JsonMapBuilder::generatePlaceHolder(const std::string &location)
    {
        int index = static_cast<int>(missing_components.size())+2;
        missing_components.emplace(index, location);
        return index;
    }

    bool JsonMapBuilder::addComponent(const std::string &info, int index)
    {
        auto loc = missing_components.find(index);
        if (loc != missing_components.end())
        {
            auto element = loadJsonStr(info);
            (*jMap)[loc->second].append(element);
            missing_components.erase(loc);

            return missing_components.empty();
        }
        return false;
    }

    std::string JsonMapBuilder::generate()
    {
        if (jMap)
        {
            return generateJsonString(*jMap);
        }
        return "{}";
    }

    void JsonMapBuilder::reset()
    {
        jMap = nullptr;
        missing_components.clear();
    }

} //namespace helics