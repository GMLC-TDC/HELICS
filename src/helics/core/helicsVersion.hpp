/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_VERSION_
#define _HELICS_VERSION_
#pragma once

#include "helics/helics-config.h"
#include <string>

/** @file
file linking with version info and containing some convenience functions
*/
namespace helics
{
/** @returns a string containing version information*/
std::string versionString ();

/** get the Major version number*/
constexpr int versionMajor=HELICS_VERSION_MAJOR;
/** get the Minor version number*/
constexpr int versionMinor=HELICS_VERSION_MINOR;
/** get the patch number*/
constexpr int versionPatch=HELICS_VERSION_PATCH;

} //namespace helics

#endif /*_HELICS_VERSION_*/
