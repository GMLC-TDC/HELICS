/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "helics/helics-config.h"

/** @file
file linking with version info and containing some convenience functions
*/
namespace helics
{
/** @returns a string containing version information*/
constexpr auto versionString = HELICS_VERSION_STRING;

/** get the Major version number*/
constexpr int versionMajor = HELICS_VERSION_MAJOR;
/** get the Minor version number*/
constexpr int versionMinor = HELICS_VERSION_MINOR;
/** get the patch number*/
constexpr int versionPatch = HELICS_VERSION_PATCH;

}  // namespace helics
