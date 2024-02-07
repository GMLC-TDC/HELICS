/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#ifndef HELICS_TIME_HEADER_
#    define HELICS_TIME_HEADER_
#    include "../utilities/timeRepresentation.hpp"
#    include "CoreTypes.hpp"
#    include "helics/helics-config.h"

#    include <cstdint>
/** @file
@details defining the time representation to use throughout helics
*/
namespace helics {
/**
 * Simulation virtual time.
 *
 * Class represents time in the core.
 */
#    ifdef HELICS_USE_PICOSECOND_TIME
using Time = TimeRepresentation<count_time<12>>;
#    else
using Time = TimeRepresentation<count_time<9>>;
#    endif

/** constexpr definition for starting time*/
constexpr Time timeZero = Time::zeroVal();
/** definition of the minimum time resolution*/
constexpr Time timeEpsilon = Time::epsilon();
/** definition of the smallest negative increment of time*/
constexpr Time negEpsilon = -Time::epsilon();
/** definition of large time representing simulation end*/
constexpr Time cBigTime = Time{static_cast<int64_t>(HELICS_BIG_NUMBER * 1000000)};

/** common definition of currentTime in initialization mode*/
constexpr Time initializationTime = negEpsilon;

/** simple structure with the time and completion marker for iterations or dense time steps*/
struct iteration_time {
    Time grantedTime;  //!< the time of the granted step
    IterationResult state;  //!< the convergence state
};

}  // namespace helics

#endif  // HELICS_TIME_HEADER_
