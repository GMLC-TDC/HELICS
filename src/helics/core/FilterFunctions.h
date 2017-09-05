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
/** helper data class to manage filtering
*/
class filterData
{
public:
	Core::federate_id_t fed_id=invalid_fed_id; //!< the federate ID of the filter
	Core::Handle handle=invalid_Handle; //!< the handle of the filter related to FederateID
	bool hasOperator_flag=false;  //!< a flag indicating that the filter has an operator
	std::string input_type;	//!< the input_type of the filter
	std::string output_type;  //!< the output type of the filter
	filterData() = default;
	filterData(Core::federate_id_t fid, Core::Handle handle_, const std::string &inputType, const std::string &outputType) : fed_id(fid), handle(handle_), input_type(inputType), output_type(outputType)
	{};
};

/** class to manage the ordering of filter operations for an endpoint
*/
class FilterCoordinator
{
public:
	std::vector<filterData> sourceOperators; //!< ordered set of source operators
	filterData finalSourceFilter;	//!< the final source filter
	filterData destOperator;	//!< the destination operator handle

	std::vector<filterData> allSourceFilters; //!< storage for all the source filters before sorting
	bool hasSourceOperators=false; //!< indicator that the endpoint has source filter operators	
	bool hasSourceFilter=false;	//!< indicator that an endpoint has source filters
	bool hasDestOperator=false;	//!< indicator that an endpoint has a destination filter
};
}

#endif