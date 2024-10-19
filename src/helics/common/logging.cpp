/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "logging.hpp"

#include "gmlc/utilities/string_viewConversion.h"
#include "gmlc/utilities/string_viewOps.h"

#include <map>
#include <string>
#include <unordered_map>

namespace helics {
extern const std::unordered_map<std::string, int> gLogLevelMap{
    {"none", HELICS_LOG_LEVEL_NO_PRINT},
    {"no_print", HELICS_LOG_LEVEL_NO_PRINT},
    {"noprint", HELICS_LOG_LEVEL_NO_PRINT},
    {"error", HELICS_LOG_LEVEL_ERROR},
    {"profiling", HELICS_LOG_LEVEL_PROFILING},
    {"warning", HELICS_LOG_LEVEL_WARNING},
    {"summary", HELICS_LOG_LEVEL_SUMMARY},
    {"connections", HELICS_LOG_LEVEL_CONNECTIONS},
    {"interfaces", HELICS_LOG_LEVEL_INTERFACES},
    {"timing", HELICS_LOG_LEVEL_TIMING},
    {"data", HELICS_LOG_LEVEL_DATA},
    {"debug", HELICS_LOG_LEVEL_DEBUG},
    {"trace", HELICS_LOG_LEVEL_TRACE},

    {"NONE", HELICS_LOG_LEVEL_NO_PRINT},
    {"NO_PRINT", HELICS_LOG_LEVEL_NO_PRINT},
    {"NOPRINT", HELICS_LOG_LEVEL_NO_PRINT},
    {"ERROR", HELICS_LOG_LEVEL_ERROR},
    {"PROFILING", HELICS_LOG_LEVEL_PROFILING},
    {"WARNING", HELICS_LOG_LEVEL_WARNING},
    {"SUMMARY", HELICS_LOG_LEVEL_SUMMARY},
    {"CONNECTIONS", HELICS_LOG_LEVEL_CONNECTIONS},
    {"INTERFACES", HELICS_LOG_LEVEL_INTERFACES},
    {"TIMING", HELICS_LOG_LEVEL_TIMING},
    {"DATA", HELICS_LOG_LEVEL_DATA},
    {"DEBUG", HELICS_LOG_LEVEL_DEBUG},
    {"TRACE", HELICS_LOG_LEVEL_TRACE},
    {"None", HELICS_LOG_LEVEL_NO_PRINT},
    {"No_print", HELICS_LOG_LEVEL_NO_PRINT},
    {"No_Print", HELICS_LOG_LEVEL_NO_PRINT},
    {"Noprint", HELICS_LOG_LEVEL_NO_PRINT},
    {"NoPrint", HELICS_LOG_LEVEL_NO_PRINT},
    {"Error", HELICS_LOG_LEVEL_ERROR},
    {"Profiling", HELICS_LOG_LEVEL_PROFILING},
    {"Warning", HELICS_LOG_LEVEL_WARNING},
    {"Summary", HELICS_LOG_LEVEL_SUMMARY},
    {"Connections", HELICS_LOG_LEVEL_CONNECTIONS},
    {"Interfaces", HELICS_LOG_LEVEL_INTERFACES},
    {"Timing", HELICS_LOG_LEVEL_TIMING},
    {"Data", HELICS_LOG_LEVEL_DATA},
    {"Debug", HELICS_LOG_LEVEL_DEBUG},
    {"Trace", HELICS_LOG_LEVEL_TRACE}};

LogLevels logLevelFromString(std::string_view level)
{
    auto res = gLogLevelMap.find(std::string(level));
    if (res != gLogLevelMap.end()) {
        return static_cast<LogLevels>(res->second);
    }
    if (level.compare(0, 9, "loglevel_") == 0) {
        return static_cast<LogLevels>(
            gmlc::utilities::numeric_conversion<int>(level.substr(9), -999999));
    } else {
        return static_cast<LogLevels>(-999999);
    }
}

const std::map<LogLevels, std::string_view> levelMaps{{static_cast<LogLevels>(-999999), "no_print"},
                                                      {LogLevels::NO_PRINT, "no_print"},
                                                      {LogLevels::ERROR_LEVEL, "error"},
                                                      {LogLevels::PROFILING, "profiling"},
                                                      {LogLevels::WARNING, "warning"},
                                                      {LogLevels::SUMMARY, "summary"},
                                                      {LogLevels::CONNECTIONS, "connections"},
                                                      {LogLevels::INTERFACES, "interfaces"},
                                                      {LogLevels::TIMING, "timing"},
                                                      {LogLevels::DATA, "data"},
                                                      {LogLevels::DEBUG, "debug"},
                                                      {LogLevels::TRACE, "trace"}};

std::string logLevelToString(LogLevels level)
{
    auto res = levelMaps.find(level);
    if (res != levelMaps.end()) {
        return std::string(res->second);
    }
    std::string ret = "loglevel_" + std::to_string(static_cast<int>(level));
    return ret;
}

}  // namespace helics
