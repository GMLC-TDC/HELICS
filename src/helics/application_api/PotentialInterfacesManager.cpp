/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "PotentialInterfacesManager.hpp"

#include "Federate.hpp"
#include "helics/common/JsonProcessingFunctions.hpp"
#include "helics/core/Core.hpp"

#include <set>

namespace helics {
PotentialInterfacesManager::PotentialInterfacesManager(Core* core, Federate* fed):
    corePtr(core), fedPtr(fed)
{
}

void PotentialInterfacesManager::loadPotentialInterfaces(Json::Value& json)
{
    static const std::set<std::string> interfaceTypes{
        "publications", "inputs", "endpoints", "filters", "translators", "datasinks"};
    if (!json.isMember("potential_interfaces")) {
        return;
    }
    auto interfaces = json["potential_interfaces"];
    for (auto& itype : interfaceTypes) {
        if (!interfaces.isMember(itype)) {
            continue;
        }
        auto tInterface = interfaces[itype];
        auto& pMap = potInterfaces[itype];
        for (auto& ispec : tInterface) {
            auto name = fileops::getName(ispec);
            pMap[name] = ispec;
        }
    }
}

void PotentialInterfacesManager::initialize()
{
    corePtr->setQueryCallback(
        fedPtr->getID(),
        [this](std::string_view query) { return generateQueryResponse(query); },
        2);
}

std::string PotentialInterfacesManager::generateQueryResponse(std::string_view query)
{
    if (query == "potential_interfaces") {
        if (respondedToCommand.load()) {
            // we have already generated interfaces so no need to respond to the query
            return std::string{};
        }
        Json::Value interfaces;
        for (const auto& iType : potInterfaces) {
            interfaces[iType.first] = Json::arrayValue;
            for (auto& ispec : iType.second) {
                interfaces[iType.first].append(ispec.first);
            }
        }
        return fileops::generateJsonString(interfaces);
    } else {
        return std::string{};
    }
}

void PotentialInterfacesManager::processCommand(std::pair<std::string, std::string> command)
{
    Json::Value json;
    try {
        json = fileops::loadJsonStr(command.first);
    }
    catch (const std::invalid_argument&) {
        extraCommands.push_back(std::move(command));
        return;
    }
    if (json.isMember("command")) {
        if (json["command"] == "register_interfaces") {
            Json::Value generator;
            for (auto& iType : potInterfaces) {
                if (json.isMember(iType.first)) {
                    if (iType.first == "endpoints") {
                        generator["targeted"] = true;
                    }
                    generator[iType.first] = Json::arrayValue;
                    iMap& pInterfaces = iType.second;
                    for (const auto& iface : json[iType.first]) {
                        const std::string name = iface.asString();
                        auto iLoc = pInterfaces.find(name);
                        if (iLoc != pInterfaces.end()) {
                            generator[iType.first].append(iLoc->second);
                        }
                    }
                }
            }
            std::string generatorList = fileops::generateJsonString(generator);
            fedPtr->registerInterfaces(generatorList);
            respondedToCommand.store(true);
            return;
        }
    }
    extraCommands.push_back(std::move(command));
}

bool PotentialInterfacesManager::hasExtraCommands() const
{
    return (!extraCommands.empty());
}

std::pair<std::string, std::string> PotentialInterfacesManager::getCommand()
{
    std::pair<std::string, std::string> cmd;
    if (!extraCommands.empty()) {
        cmd = std::move(extraCommands.front());
        extraCommands.pop_front();
    }
    return cmd;
}

}  // namespace helics
