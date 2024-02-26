/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include <string_view>
#include <string>
#include <vector>
#include <map>

namespace Json
{
    class Value;
}

namespace helics
{
    class Core;

    class PotentialInterfacesManager
    {
    public:
        PotentialInterfacesManager(Core *core);
        void loadPotentialInterfaces(Json::Value &json);
        std::string generateQueryResponse(std::string_view query);
    private:
        Core *corePtr{nullptr};
    };
}
