/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "JsonMapBuilder.hpp"
#include "../common/JsonProcessingFunctions.hpp"

namespace helics
{
Json::Value &JsonMapBuilder::getJValue ()
{
    if (!jMap)
    {
        jMap = std::make_unique<Json::Value> ();
    }
    return *jMap;
}

bool JsonMapBuilder::isCompleted () const { return (jMap) && (missing_components.empty ()); }

int JsonMapBuilder::generatePlaceHolder (const std::string &location)
{
    int index = static_cast<int> (missing_components.size ()) + 2;
    missing_components.emplace (index, location);
    return index;
}

bool JsonMapBuilder::addComponent (const std::string &info, int index)
{
    auto loc = missing_components.find (index);
    if (loc != missing_components.end ())
    {
        auto element = loadJsonStr (info);
        (*jMap)[loc->second].append (element);
        missing_components.erase (loc);

        return missing_components.empty ();
    }
    return false;
}

std::string JsonMapBuilder::generate ()
{
    if (jMap)
    {
        return generateJsonString (*jMap);
    }
    return "{}";
}

void JsonMapBuilder::reset ()
{
    jMap = nullptr;
    missing_components.clear ();
}

}  // namespace helics
