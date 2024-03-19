/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "PotentialInterfacesManager.hpp"

#include "../core/core-exceptions.hpp"
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
    const auto& interfaces = json["potential_interfaces"];
    for (const auto& itype : interfaceTypes) {
        if (interfaces.isMember(itype)) {
            auto tInterface = interfaces[itype];
            auto& pMap = potInterfaces[itype];
            for (auto& ispec : tInterface) {
                auto name = fileops::getName(ispec);
                pMap[name] = ispec;
            }
        }
        std::string tempString = itype;
        tempString.pop_back();
        tempString += "_templates";
        if (interfaces.isMember(tempString)) {
            auto templateInterfaces = interfaces[tempString];
            auto& tMap = potInterfaceTemplates[itype];
            for (auto& tspec : templateInterfaces) {
                auto name = fileops::getName(tspec);
                if (name.find("}${") != std::string::npos) {
                    throw(helics::InvalidParameter(
                        std::string(
                            "template key definitions must not be adjacent, they must have separator characters [") +
                        name + ']'));
                }
                tMap[name] = tspec;
            }
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
            for (const auto& ispec : iType.second) {
                interfaces[iType.first].append(ispec.first);
            }
        }
        for (const auto& iType : potInterfaceTemplates) {
            std::string templateKey = iType.first;
            templateKey.pop_back();
            templateKey += "_templates";
            interfaces[templateKey] = Json::arrayValue;
            for (const auto& ispec : iType.second) {
                interfaces[templateKey].append(ispec.second);
            }
        }
        return fileops::generateJsonString(interfaces);
    }
    return std::string{};
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
            for (auto& iType : potInterfaceTemplates) {
                std::string templateKey{"templated_"};
                templateKey.append(iType.first);
                if (json.isMember(templateKey)) {
                    bool noUnits{false};
                    if (iType.first == "endpoints") {
                        generator["targeted"] = true;
                        noUnits = true;
                    }
                    for (auto& templateInterfaces : json[templateKey]) {
                        auto templateName = fileops::getName(templateInterfaces);

                        auto templateLoc = iType.second.find(templateName);
                        if (templateLoc == iType.second.end()) {
                            continue;
                        }
                        auto& templateGenerator = templateLoc->second;
                        for (auto& interfaceName : templateInterfaces["interfaces"]) {
                            Json::Value interfaceSpec = Json::nullValue;
                            interfaceSpec.copy(templateGenerator["template"]);
                            if (interfaceName.isArray()) {
                                interfaceSpec["name"] = interfaceName[0];
                                std::string str = interfaceName[1].asString();
                                if (!str.empty()) {
                                    interfaceSpec["type"] = interfaceName[1];
                                }
                                if (!noUnits) {
                                    str = interfaceName[2].asString();
                                    if (!str.empty()) {
                                        interfaceSpec["units"] = interfaceName[2];
                                    }
                                }
                            } else {
                                interfaceSpec["name"] = interfaceName.asString();
                            }
                            generator[iType.first].append(interfaceSpec);
                        }
                    }
                }
            }
            const std::string generatorList = fileops::generateJsonString(generator);
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
