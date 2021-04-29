/*
Copyright (c) 2017-2021,
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
enum log_level : int {
    no_print = helics_log_level_no_print,  //!< never print
    error = helics_log_level_error,  //!< only print errors
    warning = helics_log_level_warning,  //!< print/log warning and errors
    summary = helics_log_level_summary,  //!< print/log summary information
    connections =
        helics_log_level_connections,  //!< print summary+ federate level connection information
    interfaces =
        helics_log_level_interfaces,  //!< print connections +interface level connection information
    timing = helics_log_level_timing,  //!< print interfaces+ timing(exec/grant/disconnect)
    data = helics_log_level_data,  //!< print timing+data transmissions
    trace = helics_log_level_trace,  //!< trace level printing (all processed messages)
    fed = 99999  //!< special logging command for message coming from a fed
};

#define LOG_ERROR(id, ident, message) sendToLogger(id, log_level::error, ident, message)
#define LOG_ERROR_SIMPLE(message)                                                                  \
    sendToLogger(global_broker_id_local, log_level::error, getIdentifier(), message)
#define LOG_WARNING(id, ident, message) sendToLogger(id, log_level::warning, ident, message)

#define LOG_WARNING_SIMPLE(message)                                                                \
    sendToLogger(global_broker_id_local, log_level::warning, getIdentifier(), message)

#ifdef HELICS_ENABLE_LOGGING
#    define LOG_SUMMARY(id, ident, message)                                                        \
        if (maxLogLevel >= log_level::summary) {                                                   \
            sendToLogger(id, log_level::summary, ident, message);                                  \
        }

#    define LOG_CONNECTIONS(id, ident, message)                                                    \
        if (maxLogLevel >= log_level::connections) {                                               \
            sendToLogger(id, log_level::connections, ident, message);                              \
        }

#    define LOG_INTERFACES(id, ident, message)                                                     \
        if (maxLogLevel >= log_level::interfaces) {                                                \
            sendToLogger(id, log_level::interfaces, ident, message);                               \
        }

#    ifdef HELICS_ENABLE_DEBUG_LOGGING
#        define LOG_TIMING(id, ident, message)                                                     \
            if (maxLogLevel >= log_level::timing) {                                                \
                sendToLogger(id, log_level::timing, ident, message);                               \
            }
#        define LOG_DATA_MESSAGES(id, ident, message)                                              \
            if (maxLogLevel >= log_level::data) {                                                  \
                sendToLogger(id, log_level::data, ident, message);                                 \
            }
#    else
#        define LOG_TIMING(id, ident, message)
#        define LOG_DATA_MESSAGES(id, ident, message)
#    endif

#    ifdef HELICS_ENABLE_TRACE_LOGGING
#        define LOG_TRACE(id, ident, message)                                                      \
            if (maxLogLevel >= log_level::trace) {                                                 \
                sendToLogger(id, log_level::trace, ident, message);                                \
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
#    define LOG_TRACE(id, ident, message)
#endif
