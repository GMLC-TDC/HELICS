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
#include <string>
#include <utility>

namespace helics {
PotentialInterfacesManager::PotentialInterfacesManager(Core* core, Federate* fed):
    corePtr(core), fedPtr(fed)
{
}

void PotentialInterfacesManager::loadPotentialInterfaces(const nlohmann::json& json)
{
    static const std::set<std::string> interfaceTypes{
        "publications", "inputs", "endpoints", "filters", "translators", "datasinks"};
    if (json.contains("potential_interfaces")) {
        const auto& interfaces = json["potential_interfaces"];
        for (const auto& itype : interfaceTypes) {
            if (interfaces.contains(itype)) {
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
            if (interfaces.contains(tempString)) {
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
    if (json.contains("potential_interface_templates")) {
        const auto& interfaces = json["potential_interface_templates"];
        for (const auto& itype : interfaceTypes) {
            std::string tempString = itype;
            tempString.pop_back();
            tempString += "_templates";
            if (interfaces.contains(tempString)) {
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
        nlohmann::json interfaces;
        for (const auto& iType : potInterfaces) {
            interfaces[iType.first] = nlohmann::json::array();
            for (const auto& ispec : iType.second) {
                interfaces[iType.first].push_back(ispec.first);
            }
        }
        for (const auto& iType : potInterfaceTemplates) {
            std::string templateKey = iType.first;
            templateKey.pop_back();
            templateKey += "_templates";
            interfaces[templateKey] = nlohmann::json::array();
            for (const auto& ispec : iType.second) {
                interfaces[templateKey].push_back(ispec.second);
            }
        }
        return fileops::generateJsonString(interfaces);
    }
    return std::string{};
}

void PotentialInterfacesManager::processCommand(std::pair<std::string, std::string> command)
{
    nlohmann::json json;
    try {
        json = fileops::loadJsonStr(command.first);
    }
    catch (const std::invalid_argument&) {
        extraCommands.push_back(std::move(command));
        return;
    }
    if (json.contains("command")) {
        if (json["command"] == "register_interfaces") {
            nlohmann::json generator;
            for (auto& iType : potInterfaces) {
                if (json.contains(iType.first)) {
                    if (iType.first == "endpoints") {
                        generator["targeted"] = true;
                    }
                    generator[iType.first] = nlohmann::json::array();
                    iMap& pInterfaces = iType.second;
                    for (const auto& iface : json[iType.first]) {
                        const std::string name = iface.get<std::string>();
                        auto iLoc = pInterfaces.find(name);
                        if (iLoc != pInterfaces.end()) {
                            generator[iType.first].push_back(iLoc->second);
                        }
                    }
                }
            }
            for (auto& iType : potInterfaceTemplates) {
                std::string templateKey{"templated_"};
                templateKey.append(iType.first);
                if (json.contains(templateKey)) {
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
                            nlohmann::json interfaceSpec = templateGenerator["template"];
                            if (interfaceName.is_array()) {
                                interfaceSpec["name"] = interfaceName[0];
                                std::string str = interfaceName[1].get<std::string>();
                                if (!str.empty()) {
                                    interfaceSpec["type"] = interfaceName[1];
                                }
                                if (!noUnits) {
                                    str = interfaceName[2].get<std::string>();
                                    if (!str.empty()) {
                                        interfaceSpec["units"] = interfaceName[2];
                                    }
                                }
                            } else {
                                interfaceSpec["name"] = interfaceName.get<std::string>();
                            }
                            generator[iType.first].push_back(interfaceSpec);
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
