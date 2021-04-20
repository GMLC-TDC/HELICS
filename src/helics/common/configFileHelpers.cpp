/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "configFileHelpers.hpp"

#include <string>

bool looksLikeFile(const std::string& configString)
{
    if (hasTomlExtension(configString)) {
        return true;
    }
    if ((hasJsonExtension(configString)) ||
        (configString.find_first_of('{') != std::string::npos)) {
        return true;
    }
    return false;
}
