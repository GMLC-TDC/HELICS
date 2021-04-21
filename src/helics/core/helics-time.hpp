/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#ifndef _HELICS_TIME_HEADER_
#    define _HELICS_TIME_HEADER_
#    include "../utilities/timeRepresentation.hpp"
#    include "core-types.hpp"
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

/** common definition of currentTime in initialization mode*/
constexpr Time initializationTime = negEpsilon;

/** user defined literal for a time variable*/
constexpr Time operator"" _t(long double val)
{
    return {static_cast<double>(val)};
}  // NOLINT

/** simple structure with the time and completion marker for iterations or dense time steps*/
struct iteration_time {
    Time grantedTime;  //!< the time of the granted step
    iteration_result state;  //!< the convergence state
};

}  // namespace helics

// #TOBEDEPRECTATED The use of the the core-types header for the functions contained in
// ../application_api/timeOperations.hpp is deprectaced and will be removed in HELICS 3.0
// please use ../application_api/timeOperations.hpp directory for those functions.
// This next section should be removed in HELICS 3.0 but is needed to prevent breaking changes
#    if defined HELICS_SHARED_LIBRARY || !defined HELICS_STATIC_CORE_LIBRARY
#        include "../application_api/timeOperations.hpp"
#    endif
#endif  //_HELICS_TIME_HEADER_
