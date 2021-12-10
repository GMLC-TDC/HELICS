/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helicsVersion.hpp"
#include "../common/JsonProcessingFunctions.hpp"
#include "CoreFactory.hpp"

namespace helics {

    std::string extendedVersionInfo() {
        Json::Value base;
        base["version"]["string"] = helics::versionString;
        base["version"]["major"] = helics::versionMajor;
        base["version"]["minor"] = helics::versionMinor;
        base["version"]["patch"] = helics::versionPatch;
        base["version"]["build"] = helics::versionBuild;
        base["buildflags"] = helics::buildFlags;
        base["compiler"] = helics::compiler;
        base["cores"] = Json::arrayValue;

        auto ctypesA = CoreFactory::getAvailableCoreTypes();
        for (const auto &ctype:ctypesA) {
            base["cores"].append(ctype);
        }
        return fileops::generateJsonString(base);
    }

}
