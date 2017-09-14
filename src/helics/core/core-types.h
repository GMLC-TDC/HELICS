/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _CORE_TYPES_H_
#define _CORE_TYPES_H_
#pragma once

#include <string>

enum helics_core_type
{
	HELICS_ZMQ,
	HELICS_MPI,
	HELICS_TEST,
	HELICS_INTERPROCESS,
	HELICS_DEFAULT,
};

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
/** enumeration of the possible states of convergence*/
enum class convergence_state :signed char
{
	error = -5,		//!< indicator that an error has occured
	continue_processing = -1, //!< the current loop should continue
	complete = 0,  //!< indicator that the iterations have completed
	nonconverged = 1,	//!< indicator that the iterations need to continue
	halted = 3,	//!< indicator that the simulation has been halted

};

/** defining some check modes for dealing with required or optional components*/
enum class handle_check_mode : char
{
	required = 0, //!< indicator that the publication or endpoint is required to be there
	optional = 1,	//!< indicator that the publication or endpoint is optional
};

/** generate a core type value from a std::string
@param a string describing the desired core type
@return a value of the helics_core_type enumeration
@throws invalid_argument if the string is not recognized
*/
helics_core_type coreTypeFromString(const std::string &type);

}

#define PUBLICATION_REQUIRED helics::handle_check_mode::required
#define PUBLICATION_OPTIONAL helics::handle_check_mode::optional

#endif

