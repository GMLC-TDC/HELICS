/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "../common/timeRepresentation.hpp"
#include "core-types.hpp"
#include "helics/helics-config.h"
#include <cstdint>
/** @file
@details defining the time representation to use throughout helics
*/
namespace helics
{
/**
 * Simulation virtual time.
 *
 * Class represents time in the core.
 */
#ifdef HELICS_USE_PICOSECOND_TIME
using Time = TimeRepresentation<count_time<12>>;
#else
using Time = TimeRepresentation<count_time<9>>;
#endif

/** constexpr definition for starting time*/
constexpr Time timeZero = Time::zeroVal ();
/** definition of the minimum time resolution*/
constexpr Time timeEpsilon = Time::epsilon ();
/** definition of the smallest negative increment of time*/
constexpr Time negEpsilon = -Time::epsilon ();
/** user defined literal for a time variable*/
constexpr Time operator"" _t (long double val) { return Time (val); }  // NOLINT

/** simple structure with the time and completion marker for iterations or dense time steps*/
struct iteration_time
{
  public:
    Time grantedTime;  //!< the time of the granted step
    iteration_result state;  //!< the convergence state
    /** default constructor*/
    iteration_time () = default;
    /** construct from properties*/
    constexpr iteration_time (Time t, iteration_result iterate) noexcept : grantedTime (t), state (iterate){};
};

/** generate a time from a string
@details the string can be a double or with units
for example "1.234",  or "1032ms" or "10423425 ns"
@return a helics time generated from the string
@throw, invalid_argument if the string is not a valid time
*/
Time loadTimeFromString (const std::string &timeString);

/** generate a time from a string
@details the string can be a double or with units
for example "1.234"  or "1032ms"
@return a helics time generated from the string
@throws invalid_argument if the string is not a valid time
*/
Time loadTimeFromString (const std::string &timeString, time_units defUnit);

/** generate a time related unit,
@return a time_units enumeration value
@throw, invalid_argument if the string is not a valid unit
*/
time_units timeUnitsFromString (const std::string &unitString);

}  // namespace helics
