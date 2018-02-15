/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/
#pragma once

#include "../common/timeRepresentation.hpp"
#include "core-types.hpp"
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
using Time = TimeRepresentation<count_time<9>>;

constexpr Time timeZero = Time::zeroVal ();
constexpr Time timeEpsilon = Time::epsilon ();
constexpr Time negEpsilon = -Time::epsilon ();

constexpr Time operator"" _t (long double val) { return Time (val); }

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


/** generate a time from a string,
@details the string can be a double or with units
@example "1.234",  or "1032ms"
@return a helics time generated from the string
@throw, invalid_argument if the string is not a valid time
*/
Time loadTimeFromString(const std::string &timeString);

/** generate a time from a string,
@details the string can be a double or with units
@example "1.234",  or "1032ms"
@return a helics time generated from the string
@throw, invalid_argument if the string is not a valid time
*/
Time loadTimeFromString(const std::string &timeString, timeUnits defUnit);

/** generate a time related unit,
@return a timeUnits enumeration value
@throw, invalid_argument if the string is not a valid unit
*/
timeUnits timeUnitsFromString(const std::string &unitString);

}
