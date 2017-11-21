/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef FILTER_FUNCTIONS_H_
#define FILTER_FUNCTIONS_H_
#pragma once

#include "core.h"

#include <vector>
namespace helics
{

    class FilterInfo;
/** class to manage the ordering of filter operations for an endpoint
*/
class FilterCoordinator
{
public:
	std::vector<FilterInfo *> sourceFilters; //!< ordered set of source operators
    FilterInfo *finalSourceFilter=nullptr; //!< the last sourceFilter to trigger
	FilterInfo *destOperator=nullptr;	//!< the destination operator handle

	std::vector<FilterInfo *> allSourceFilters; //!< storage for all the source filters before sorting
	bool hasSourceFilter=false;	//!< indicator that an endpoint has source filters
	bool hasDestOperator=false;	//!< indicator that an endpoint has a destination filter
};
}

#endif