/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_FILTERINFO_
#define _HELICS_FILTERINFO_

#include "helics/helics-config.h"
#include "helics-time.h"
#include "../common/blocking_queue.h"
#include "core.h"

#include <mutex> 
#include <thread> 
#include <vector> 
#include <map>

namespace helics {

/** data class defining the information about a filter*/
class FilterInfo
{
public:
	/** constructor from all fields*/
	FilterInfo(Core::Handle id_,Core::federate_id_t fed_id_,
		const std::string &key_,
		const std::string &type_,
		const std::string &target_,
		bool destFilter_)
		: id(id_),
		fed_id(fed_id_),
		key(key_),
		inputType(type_),
		dest_filter(destFilter_),
		filterTarget(target_)
	{
	}

	Core::Handle id=invalid_fed_id; //!< id handle of the filter
	Core::federate_id_t fed_id=invalid_Handle;	//!< id of the core that manages the filter
	std::string key;	//!< the identifier of the filter
	std::string inputType;	//!< the type of data for the filter
    std::string outputType; //!< the outputType of data of the filter
	bool has_update = false;	//!< indicator that the filter has updates
	bool dest_filter = false;	//! indicator that the filter is a destination filter
	// there is a 6 byte gap here
	std::shared_ptr<FilterOperator> filterOp;	//!< the callback operation of the filter
	std::string filterTarget;	//!< the target endpoint name of the filter
	std::pair<Core::federate_id_t, Core::Handle> target{ invalid_fed_id,invalid_Handle };	//!< the actual target information for the filter
};
} // namespace helics

#endif