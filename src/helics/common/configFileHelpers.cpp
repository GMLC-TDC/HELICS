/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "configFileHelpers.hpp"

#include <string>

namespace helics::fileops {
bool looksLikeCommandLine(std::string_view testString)
{
    if (testString.empty()) {
        return false;
    }
    if (testString.front() == '-') {
        return true;
    }
    if (testString.front() == '\\' || testString.front() == '#' || testString.front() == '{') {
        return false;
    }
    return testString.find(" -") != std::string_view::npos;
}

bool looksLikeFile(std::string_view configString)
{
    if (helics::fileops::hasTomlExtension(configString)) {
        return true;
    }
    if ((helics::fileops::hasJsonExtension(configString)) ||
        (configString.find_first_of('{') != std::string_view::npos)) {
        return true;
    }
    return false;
}

ConfigType getConfigType(std::string_view configString)
{
    if (configString.empty()) {
        return ConfigType::NONE;
    }
    if (configString.front() == '-') {
        return ConfigType::CMD_LINE;
    }
    if (fileops::hasJsonExtension(configString)) {
        return ConfigType::JSON_FILE;
    }
    if (fileops::hasTomlExtension(configString)) {
        return ConfigType::TOML_FILE;
    }
    if (fileops::looksLikeConfigJson(configString)) {
        return ConfigType::JSON_STRING;
    }
    if (fileops::looksLikeCommandLine(configString)) {
        return ConfigType::CMD_LINE;
    }
    if (fileops::looksLikeConfigToml(configString)) {
        return ConfigType::TOML_STRING;
    }
    return ConfigType::NONE;
}

}  // namespace helics::fileops
