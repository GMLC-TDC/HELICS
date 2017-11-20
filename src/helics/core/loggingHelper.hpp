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

#include "helics/config.h"
/** @file
this file is meant to be included in the commonCore.cpp and coreBroker.cpp
and inherited class files
it assumes some knowledge of the internals of those programs via MACROS
using elsewhere is probably not going to work.  Someday this will be made more generic

*/

// just enumerating some print levels
enum print_level : int
{
    no_print = -1,  //!< never print
    error = 0,  //!< only print errors
    warning = 1,  //!< print/log warning and errors
    normal = 2,  //!< default print level
    debug = 3,  //!< debug level prints
    trace = 4,  //!< trace level printing
};

#define LOG_ERROR(id, ident, message) sendToLogger (id, print_level::error, ident, message);
#define LOG_WARNING(id, ident, message) sendToLogger (id, print_level::warning, ident, message);

//#define LOGGING_ENABLED
//#define DEBUG_LOGGING_ENABLED
//#define TRACE_LOGGING_ENABLED

#ifdef LOGGING_ENABLED
#define LOG_NORMAL(id, ident, message)                                                                            \
    if (maxLogLevel >= print_level::normal)                                                                       \
    {                                                                                                             \
        sendToLogger (id, print_level::normal, ident, message);                                                   \
    }

#ifdef DEBUG_LOGGING_ENABLED
#define LOG_DEBUG(id, ident, message)                                                                             \
    if (maxLogLevel >= print_level::debug)                                                                        \
    {                                                                                                             \
        sendToLogger (id, print_level::debug, ident, message);                                                    \
    }
#else
#define LOG_DEBUG(id, ident, message)
#endif

#ifdef TRACE_LOGGING_ENABLED
#define LOG_TRACE(id, ident, message)                                                                             \
    if (maxLogLevel >= print_level::trace)                                                                        \
    {                                                                                                             \
        sendToLogger (id, print_level::debug, ident, message);                                                    \
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