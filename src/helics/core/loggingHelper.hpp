/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "helics/helics-config.h"
#include "helics/helics_enums.h"

namespace helics {
/** @file
this file is meant to be included in the commonCore.cpp and coreBroker.cpp
and inherited class files
it assumes some knowledge of the internals of those programs via MACROS
using elsewhere is probably not going to work.
*/

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
}  // namespace helics
