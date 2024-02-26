/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "PotentialInterfacesManager.hpp"

#include "helics/common/JsonProcessingFunctions.hpp"
#include "helics/core/Core.hpp"

#include <set>

namespace helics {
PotentialInterfacesManager::PotentialInterfacesManager(Core* core): corePtr(core) {}

void PotentialInterfacesManager::loadPotentialInterfaces(Json::Value& json)
{
    static const std::set<std::string> interfaceTypes{
        "publications", "inputs", "endpoints", "filters", "translators", "datasink"};
}

std::string PotentialInterfacesManager::generateQueryResponse(std::string_view query)
{
    return std::string{};
}
}  // namespace helics
