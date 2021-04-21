/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "../core/helics-time.hpp"
#include "helics_cxx_export.h"

#include <string>
/** @file
@details defining the time representation to use throughout helics
*/
namespace helics {

/** generate a time from a string
@details the string can be a double or with units
for example "1.234",  or "1032ms" or "10423425 ns"
@return a helics time generated from the string
@throw invalid_argument if the string is not a valid time
*/
HELICS_CXX_EXPORT Time loadTimeFromString(const std::string& timeString);

/** generate a time from a string
@details the string can be a double or with units
for example "1.234"  or "1032ms"
@param timeString the string containing the time
@param defUnit the units to apply to a string with no other units specified
@return a helics time generated from the string
@throws invalid_argument if the string is not a valid time
*/
HELICS_CXX_EXPORT Time loadTimeFromString(std::string timeString, time_units defUnit);

/** generate a time related unit,
@return a time_units enumeration value
@throw invalid_argument if the string is not a valid unit
*/
HELICS_CXX_EXPORT time_units timeUnitsFromString(const std::string& unitString);

}  // namespace helics
