/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include <atomic>
#include <deque>
#include <map>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace helics {
class Core;
class Federate;

class PotentialInterfacesManager {
  public:
    PotentialInterfacesManager(Core* core, Federate* fed);
    /** get a listing of potential interfaces from a json object*/
    void loadPotentialInterfaces(const nlohmann::json& json);
    /** generate a query response for potential interfaces*/
    std::string generateQueryResponse(std::string_view query);
    /** process a command to generate the interfaces*/
    void processCommand(std::pair<std::string, std::string> command);

    /** do some initialization work*/
    void initialize();

    bool hasExtraCommands() const;

    std::pair<std::string, std::string> getCommand();

  private:
    Core* corePtr{nullptr};
    Federate* fedPtr{nullptr};
    std::atomic<bool> respondedToCommand{false};
    using iMap = std::unordered_map<std::string, nlohmann::json>;
    std::map<std::string, iMap> potInterfaces;
    std::map<std::string, iMap> potInterfaceTemplates;
    /// @brief  storage for unrelated commands that come through
    std::deque<std::pair<std::string, std::string>> extraCommands;
};
}  // namespace helics
