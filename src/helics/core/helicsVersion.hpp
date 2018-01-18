/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_VERSION_
#define _HELICS_VERSION_
#pragma once

#include "helics/helics-config.h"
#include <string>

namespace helics
{
std::string helicsVersionString();

inline int helicsVersionMajor() { return HELICS_VERSION_MAJOR; }

inline int helicsVersionMinor() { return HELICS_VERSION_MINOR; }
inline int helicsVersionPatch() { return HELICS_VERSION_PATCH; }
}


#endif _HELICS_VERSION_
