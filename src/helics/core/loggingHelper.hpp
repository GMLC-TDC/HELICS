/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_LOGGING_HELPER_
#define _HELICS_LOGGING_HELPER_
#pragma once

#include "helics/helics-config.h"
/** @file
this file is meant to be included in the commonCore.cpp and coreBroker.cpp
and inherited class files
it assumes some knowledge of the internals of those programs via MACROS
using elsewhere is probably not going to work.  Someday this will be made more generic

*/

// just enumerating some print levels
enum log_level : int
{
    no_print = -1,  //!< never print
    error = 0,  //!< only print errors
    warning = 1,  //!< print/log warning and errors
    normal = 2,  //!< default print level
    debug = 3,  //!< debug level prints
    trace = 4,  //!< trace level printing
};

#define LOG_ERROR(id, ident, message) sendToLogger (id, log_level::error, ident, message);
#define LOG_WARNING(id, ident, message) sendToLogger (id, log_level::warning, ident, message);

#ifndef LOGGING_DISABLED
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
        sendToLogger (id, log_level::debug, ident, message);                                                      \
    }
#else
#define LOG_TRACE(id, ident, message)
#endif
#else
#define LOG_NORMAL(id, ident, message)
#define LOG_DEBUG(id, ident, message)
#define LOG_TRACE(id, ident, message)
#endif

#endif /* _HELICS_LOGGING_HELPER_ */