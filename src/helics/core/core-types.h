/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _CORE_TYPES_H_
#define _CORE_TYPES_H_
#pragma once

#include <string>



enum helics_federate_state_type
{
	HELICS_CREATED,
	HELICS_INITIALIZING,
	HELICS_EXECUTING,
	HELICS_ERROR,
	HELICS_FINISHED,
	HELICS_NONE,
};

enum helics_time_unit
{
	Y,
	D,
	H,
	MIN,
	S,
	MS,
	US,
	NS,
	PS
};

namespace helics
{

	enum class core_type:int
	{
		DEFAULT=0,
		ZMQ=1,
		MPI=2,
		TEST=3,
		INTERPROCESS=4,
		IPC=5,
		TCP=6,
		UDP=7,
		UNRECOGNIZED=8,
		
	};

/** enumeration of the possible states of convergence*/
enum class iteration_state :signed char
{
	error = -5,		//!< indicator that an error has occurred
	continue_processing = -1, //!< the current loop should continue
	next_step = 0,  //!< indicator that the iterations have completed
	iterating = 2,	//!< indicator that the iterations need to continue
	halted = 3,	//!< indicator that the simulation has been halted

};

/** enumeration of the possible states of convergence*/
enum class iteration_result :signed char
{
	error = -5,		//!< indicator that an error has occurred
	next_step = 0,  //!< indicator that the iterations have completed and the federate has moved to the next step
	iterating = 2,	//!< indicator that the iterations need to continue
	halted = 3,	//!< indicator that the simulation has been halted
};

/** enumeration of the possible states of convergence*/
enum class iteration_request :signed char
{
	no_iterations = 0,  //!< indicator that the iterations have completed
	force_iteration = 1, //!< force an iteration whether it is needed or not
	iterate_if_needed = 2,	//!< indicator that the iterations need to continue
};

#define ITERATION_COMPLETE helics::iteration_request::no_iterations
#define NO_ITERATION helics::iteration_request::no_iterations
#define FORCE_ITERATION helics::iteration_request::force_iteration
#define ITERATE_IF_NEEDED helics::iteration_request::iterate_if_needed

/** defining some check modes for dealing with required or optional components*/
enum class handle_check_mode : char
{
	required = 0, //!< indicator that the publication or endpoint is required to be there
	optional = 1,	//!< indicator that the publication or endpoint is optional
};

/**generate a string based on the core type*/
std::string helicsTypeString(core_type type);

/** generate a core type value from a std::string
@param a string describing the desired core type
@return a value of the helics_core_type enumeration
@throws invalid_argument if the string is not recognized
*/
core_type coreTypeFromString(std::string type);

/**
* Returns true if core/broker type specified is available in current compilation.
*/
bool isAvailable(core_type type);

}

#define PUBLICATION_REQUIRED helics::handle_check_mode::required
#define PUBLICATION_OPTIONAL helics::handle_check_mode::optional

#endif

