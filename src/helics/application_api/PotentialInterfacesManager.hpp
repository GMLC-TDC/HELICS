/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace Json {
class Value;
}

namespace helics {
class Core;

class PotentialInterfacesManager {
  public:
    explicit PotentialInterfacesManager(Core* core);
    void loadPotentialInterfaces(Json::Value& json);
    std::string generateQueryResponse(std::string_view query);

  private:
    Core* corePtr{nullptr};
};
}  // namespace helics
