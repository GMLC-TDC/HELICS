/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_SUBSCRIPTION_
#define _HELICS_SUBSCRIPTION_

#include "helics/helics-config.h"
#include "helics-time.h"
#include "../common/blocking_queue.h"
#include "core.h"

#include <cstdint>
#include <mutex> 
#include <thread> 
#include <utility> 
#include <vector> 
#include <map>

namespace helics {

/** data class containing the information about a publication*/
class PublicationInfo
{
public:
	/** constructor from the basic information*/
	PublicationInfo(Core::Handle id_, Core::federate_id_t fed_id_,
		const std::string &key_,
		const std::string &type_,
		const std::string &units_)
		: id(id_),
		fed_id(fed_id_),
		key(key_),
		type(type_),
		units(units_)
	{
	}

	const Core::Handle id;  //!< the handle id 
	const Core::federate_id_t fed_id;	//!< the identifier for the containing federate
	std::vector<std::pair<Core::federate_id_t, Core::Handle>> subscribers; //!< container for all the subscribers of a publication
	const std::string key;	//!< the key identifier for the publication
	const std::string type;	//!< the type of the publication data
	const std::string units;	//!< the units of the publication data
	std::string data;	//!< the most recent publication data
	bool has_update = false;	//!< indicator that the publication has updates
	
    /** check the value if it is the same as the most recent data and if changed store it*/
    bool CheckSetValue(const char *checkData, uint64_t len);
	
	
};
} // namespace helics

#endif