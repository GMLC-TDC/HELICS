/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef DEPENDENCY_INFO_H_
#define DEPENDENCY_INFO_H_
#pragma once

#include "core/helics-time.h"
#include "core/core.h"

namespace helics
{
/** data class containing information about interfederate dependencies*/
class DependencyInfo
{
public:
	Core::federate_id_t fedID;  //!< identifier for the dependent federate
	bool grant = false;  //!< whether time has been granted
	bool exec_iterating = false;  //!< whether the executing state has requested iteration
	bool exec_requested = false;  //!< whether execution state has been granted
	bool time_iterating = false;	//!< whether the current time point has requested iteration
	Time Tnext = timeZero;  //!<next possible message or value 
	Time Te = timeZero;  //!< the next currently scheduled event
	Time Tdemin = timeZero;  //!< min dependency event time
							 /** default constructor*/
	DependencyInfo() = default;
	/** construct from a federate id*/
	DependencyInfo(Core::federate_id_t id) : fedID(id) {};
};

}
#endif // DEPENDENCY_INFO_H_