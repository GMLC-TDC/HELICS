/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef HELICS_CPP98_HPP_
#define HELICS_CPP98_HPP_
#pragma once

#include "CombinationFederate.hpp"

#include <string>

/**
 * HELICS C++98 Interface
 */
namespace helicscpp {
/** get a string with the helics version info*/
inline std::string getHelicsVersionString()
{
    return std::string(helicsGetVersion());
}
/** get a string with the helics version info*/
inline std::string version()
{
    return std::string(helicsGetVersion());
}

/** generate an extended version and system info string in json format*/
inline std::string systemInfo()
{
    return std::string(helicsGetSystemInfo());
}

/** get a string with the helics version info*/
inline std::string buildFlags()
{
    return std::string(helicsGetBuildFlags());
}

/** get a string with the compiler used to compile the library*/
inline std::string compilerVersion()
{
    return std::string(helicsGetCompilerVersion());
}

/** do a cleanup of the brokers and cores currently in the library*/
inline void cleanupHelicsLibrary()
{
    helicsCleanupLibrary();
}

/** close the library and cleanup all open objects*/
inline void closeLibrary()
{
    helicsCloseLibrary();
}

inline void loadSignalHandler()
{
    helicsLoadSignalHandler();
}

inline void loadThreadedSignalHandler()
{
    helicsLoadThreadedSignalHandler();
}

inline void loadSignalHandler(HelicsBool (*handler)(int))
{
    helicsLoadSignalHandlerCallback(handler, HELICS_FALSE);
}

inline void loadSignalHandler(HelicsBool (*handler)(int), bool val)
{
    helicsLoadSignalHandlerCallback(handler, val ? HELICS_TRUE : HELICS_FALSE);
}

inline void clearSignalHandler()
{
    helicsClearSignalHandler();
}

}  // namespace helicscpp
#endif
