/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _CORE_TYPES_H_
#define _CORE_TYPES_H_
#pragma once

enum helics_core_type
{
	HELICS_ZMQ,
	HELICS_MPI,
	HELICS_TEST,
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

#endif

