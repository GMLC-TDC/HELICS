/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "helics/helics-config.h"
/** @file
this file is meant to be included in the commonCore.cpp and coreBroker.cpp
and inherited class files
it assumes some knowledge of the internals of those programs via MACROS
using elsewhere is probably not going to work.  Someday this will be made more generic
*/

/** enumeration of defined print levels*/
enum log_level : int
{
    no_print = -1,  //!< never print
    error = 0,  //!< only print errors
    warning = 1,  //!< print/log warning and errors
	summary = 2, //!< print/log summary information
    normal = 3,  //!< default print level
    debug = 4,  //!< debug level prints
    trace = 5,  //!< trace level printing
};

#define LOG_ERROR(id, ident, message) sendToLogger (id, log_level::error, ident, message);
#define LOG_WARNING(id, ident, message) sendToLogger (id, log_level::warning, ident, message);

#ifndef LOGGING_DISABLED
#define LOG_SUMMARY(id, ident, message)                                                                            \
    if (maxLogLevel >= log_level::summary)                                                                         \
    {                                                                                                             \
        sendToLogger (id, log_level::summary, ident, message);                                                     \
    }

#define LOG_NORMAL(id, ident, message)                                                                            \
    if (maxLogLevel >= log_level::normal)                                                                         \
    {                                                                                                             \
        sendToLogger (id, log_level::normal, ident, message);                                                     \
    }

#ifndef DEBUG_LOGGING_DISABLED
#define LOG_DEBUG(id, ident, message)                                                                             \
    if (maxLogLevel >= log_level::debug)                                                                          \
    {                                                                                                             \
        sendToLogger (id, log_level::debug, ident, message);                                                      \
    }
#else
#define LOG_DEBUG(id, ident, message)
#endif

#ifndef TRACE_LOGGING_DISABLED
#define LOG_TRACE(id, ident, message)                                                                             \
    if (maxLogLevel >= log_level::trace)                                                                          \
    {                                                                                                             \
        sendToLogger (id, log_level::trace, ident, message);                                                      \
    }
#else
#define LOG_TRACE(id, ident, message)
#endif
#else
#define LOG_SUMMARY(id, ident, message)
#define LOG_NORMAL(id, ident, message)
#define LOG_DEBUG(id, ident, message)
#define LOG_TRACE(id, ident, message)
#endif
