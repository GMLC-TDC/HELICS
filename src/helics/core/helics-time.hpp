/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../utilities/timeRepresentation.hpp"
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
constexpr Time operator"" _t (long double val) { return Time (static_cast<double> (val)); }  // NOLINT

/** simple structure with the time and completion marker for iterations or dense time steps*/
struct iteration_time
{
    Time grantedTime;  //!< the time of the granted step
    iteration_result state;  //!< the convergence state
};

}  // namespace helics
