/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../helics_enums.h"

#include <string>
#include <unordered_map>

namespace helics {
/** @file
adding some additional enumerations and helper functions concerning logging
*/

/** enumeration of defined print levels*/
enum LogLevels : int {
    DUMPLOG = HELICS_LOG_LEVEL_DUMPLOG,  //!< only for dumplog
    NO_PRINT = HELICS_LOG_LEVEL_NO_PRINT,  //!< never print
    ERROR_LEVEL = HELICS_LOG_LEVEL_ERROR,  //!< only print errors
    PROFILING = HELICS_LOG_LEVEL_PROFILING,  //!< profiling log level
    WARNING = HELICS_LOG_LEVEL_WARNING,  //!< print/log warning and errors
    SUMMARY = HELICS_LOG_LEVEL_SUMMARY,  //!< print/log summary information
    CONNECTIONS =
        HELICS_LOG_LEVEL_CONNECTIONS,  //!< print summary+federate level connection information
    INTERFACES =
        HELICS_LOG_LEVEL_INTERFACES,  //!< print connections+interface level connection information
    TIMING = HELICS_LOG_LEVEL_TIMING,  //!< print interfaces+timing(exec/grant/disconnect)
    DATA = HELICS_LOG_LEVEL_DATA,  //!< print timing+data transmissions
    DEBUG = HELICS_LOG_LEVEL_DEBUG,  //!< print data+additional debug info
    TRACE = HELICS_LOG_LEVEL_TRACE,  //!< trace level printing (all processed messages)
    FED = 99999  //!< special logging command for message coming from a fed
};

extern const std::unordered_map<std::string, int> gLogLevelMap;

LogLevels logLevelFromString(std::string_view level);

std::string logLevelToString(LogLevels level);

}  // namespace helics
