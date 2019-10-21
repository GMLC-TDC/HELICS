/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "helics-time.hpp"
/** @file
@details defining the time representation to use throughout helics
*/
namespace helics
{
namespace core
{
/** generate a time from a string
@details the string can be a double or with units
for example "1.234",  or "1032ms" or "10423425 ns"
@return a helics time generated from the string
@throw invalid_argument if the string is not a valid time
*/
Time loadTimeFromString (const std::string &timeString);

/** generate a time from a string
@details the string can be a double or with units
for example "1.234"  or "1032ms"
@param timeString the string containing the time
@param defUnit the units to apply to a string with no other units specified
@return a helics time generated from the string
@throws invalid_argument if the string is not a valid time
*/
Time loadTimeFromString (std::string timeString, time_units defUnit);

/** generate a time related unit,
@return a time_units enumeration value
@throw invalid_argument if the string is not a valid unit
*/
time_units timeUnitsFromString (const std::string &unitString);

}  // namespace core
}  // namespace helics
