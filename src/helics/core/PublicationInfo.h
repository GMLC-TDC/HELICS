/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_SUBSCRIPTION_
#define _HELICS_SUBSCRIPTION_

#include "helics/config.h"
#include "helics-time.h"
#include "helics/common/blocking_queue.h"
#include "helics/core/core.h"

#include <cstdint>
#include <mutex> 
#include <thread> 
#include <utility> 
#include <vector> 
#include <map>

namespace helics {

class PublicationInfo
{
public:
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

	~PublicationInfo() {}

	Core::Handle id;
	Core::federate_id_t fed_id;
	std::string key;
	std::string type;
	std::string units;
	bool has_update = false;
	std::string data;
	std::vector<std::pair<Core::federate_id_t, Core::Handle>> subscribers;
};
} // namespace helics

#endif