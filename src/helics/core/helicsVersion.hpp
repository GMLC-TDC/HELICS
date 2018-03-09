/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.
*/
#pragma once

#include "helics/helics-config.h"
#include <string>

/** @file
file linking with version info and containing some convenience functions
*/
namespace helics
{

/** @returns a string containing version information*/
extern const char * versionString;

/** get the Major version number*/
constexpr int versionMajor=HELICS_VERSION_MAJOR;
/** get the Minor version number*/
constexpr int versionMinor=HELICS_VERSION_MINOR;
/** get the patch number*/
constexpr int versionPatch=HELICS_VERSION_PATCH;

} //namespace helics

