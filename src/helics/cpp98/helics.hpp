/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef _HELICS_CPP98_API_
#define _HELICS_CPP98_API_
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

}  // namespace helicscpp
#endif
