/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "JsonProcessingFunctions.hpp"
#include "TomlProcessingFunctions.hpp"

#include <string_view>

namespace helics::fileops {
enum class ConfigType {
    JSON_STRING,
    JSON_FILE,
    TOML_STRING,
    TOML_FILE,
    CMD_LINE,
    NONE,
};
/** check if a string looks like a recognized file or a command line of some kind
Does not check actual file existence or do any other verification other than do a preliminary check
if the string could be a json|toml|ini file or looks like a json string, and if it does return true
otherwise return false
*/
bool looksLikeFile(std::string_view configString);

/** check if a string looks like a command line string*/
bool looksLikeCommandLine(std::string_view testString);

/** check if a string looks like a recognized file or a command line of some kind
Does not check actual file existence or do any other verification other than do a preliminary check
if the string could be a json|toml|ini file or looks like a json string, and if it does return true
otherwise return false
*/
ConfigType getConfigType(std::string_view configString);
}  // namespace helics::fileops
