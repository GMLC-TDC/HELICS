/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "helics/helics-config.h"

/** @file
file linking with version info and containing some convenience functions
*/
namespace helics {
/** a string representation of the HELICS version*/
constexpr auto versionString = HELICS_VERSION_STRING;

/** get the Major version number*/
constexpr int versionMajor = HELICS_VERSION_MAJOR;
/** get the Minor version number*/
constexpr int versionMinor = HELICS_VERSION_MINOR;
/** get the patch number*/
constexpr int versionPatch = HELICS_VERSION_PATCH;
/** the build string if any*/
constexpr auto versionBuild = HELICS_VERSION_BUILD;

} // namespace helics
