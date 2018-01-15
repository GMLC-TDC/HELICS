/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_TIME_
#define _HELICS_TIME_

#include "core-types.h"
#include "../common/timeRepresentation.hpp"
#include <cstdint>

namespace helics
{
/**
 * Simulation virtual time.
 *
 * Class represents time in the core.
 */

using Time = TimeRepresentation<count_time<9>>;


constexpr Time timeZero = Time::zeroVal();
constexpr Time timeEpsilon = Time::epsilon();
constexpr Time negEpsilon = -Time::epsilon();

constexpr Time operator "" _t(long double val)
{
	return Time(val);
}

/** simple struct with the time and completion marker for iterations or dense time steps*/
struct iteration_time
{
public:
	Time grantedTime; //!< the time of the granted step
	iteration_result state;	//!< the convergence state
	/** default constructor*/
    iteration_time()=default;
	/** construct from properties*/
	constexpr iteration_time(Time t, iteration_result iterate) noexcept:grantedTime(t), state(iterate)
	{};
};
}
#endif
