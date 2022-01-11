/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "helics/helics-config.h"
#include "helics/helics_enums.h"

/** @file
this file is meant to be included in the commonCore.cpp and coreBroker.cpp
and inherited class files
it assumes some knowledge of the internals of those programs via MACROS
using elsewhere is probably not going to work.
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
        HELICS_LOG_LEVEL_CONNECTIONS,  //!< print summary+ federate level connection information
    INTERFACES =
        HELICS_LOG_LEVEL_INTERFACES,  //!< print connections +interface level connection information
    TIMING = HELICS_LOG_LEVEL_TIMING,  //!< print interfaces+ timing(exec/grant/disconnect)
    DATA = HELICS_LOG_LEVEL_DATA,  //!< print timing+data transmissions
    DEBUG = HELICS_LOG_LEVEL_DEBUG,  //!< print data+additional debug info
    TRACE = HELICS_LOG_LEVEL_TRACE,  //!< trace level printing (all processed messages)
    FED = 99999  //!< special logging command for message coming from a fed
};

#define LOG_ERROR(id, ident, message) sendToLogger(id, LogLevels::ERROR_LEVEL, ident, message)
#define LOG_ERROR_SIMPLE(message)                                                                  \
    sendToLogger(global_broker_id_local, LogLevels::ERROR_LEVEL, getIdentifier(), message)
#define LOG_WARNING(id, ident, message) sendToLogger(id, LogLevels::WARNING, ident, message)

#define LOG_WARNING_SIMPLE(message)                                                                \
    sendToLogger(global_broker_id_local, LogLevels::WARNING, getIdentifier(), message)

#ifdef HELICS_ENABLE_LOGGING
#    define LOG_SUMMARY(id, ident, message)                                                        \
        if (maxLogLevel >= LogLevels::SUMMARY) {                                                   \
            sendToLogger(id, LogLevels::SUMMARY, ident, message);                                  \
        }

#    define LOG_CONNECTIONS(id, ident, message)                                                    \
        if (maxLogLevel >= LogLevels::CONNECTIONS) {                                               \
            sendToLogger(id, LogLevels::CONNECTIONS, ident, message);                              \
        }

#    define LOG_INTERFACES(id, ident, message)                                                     \
        if (maxLogLevel >= LogLevels::INTERFACES) {                                                \
            sendToLogger(id, LogLevels::INTERFACES, ident, message);                               \
        }

#    ifdef HELICS_ENABLE_DEBUG_LOGGING
#        define LOG_TIMING(id, ident, message)                                                     \
            if (maxLogLevel >= LogLevels::TIMING) {                                                \
                sendToLogger(id, LogLevels::TIMING, ident, message);                               \
            }
#        define LOG_DATA_MESSAGES(id, ident, message)                                              \
            if (maxLogLevel >= LogLevels::DATA) {                                                  \
                sendToLogger(id, LogLevels::DATA, ident, message);                                 \
            }
#        define LOG_DEBUG_MESSAGES(id, ident, message)                                             \
            if (maxLogLevel >= LogLevels::DEBUG) {                                                 \
                sendToLogger(id, LogLevels::DEBUG, ident, message);                                \
            }
#    else
#        define LOG_TIMING(id, ident, message)
#        define LOG_DATA_MESSAGES(id, ident, message)
#        define LOG_DEBUG_MESSAGES(id, ident, message)
#    endif

#    ifdef HELICS_ENABLE_TRACE_LOGGING
#        define LOG_TRACE(id, ident, message)                                                      \
            if (maxLogLevel >= LogLevels::TRACE) {                                                 \
                sendToLogger(id, LogLevels::TRACE, ident, message);                                \
            }
#    else
#        define LOG_TRACE(id, ident, message)
#    endif
#else
#    define LOG_SUMMARY(id, ident, message)
#    define LOG_CONNECTIONS(id, ident, message)
#    define LOG_INTERFACES(id, ident, message)
#    define LOG_TIMING(id, ident, message)
#    define LOG_DATA_MESSAGES(id, ident, message)
#    define LOG_DEBUG_MESSAGES(id, ident, message)
#    define LOG_TRACE(id, ident, message)
#endif
