/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_FILTERINFO_
#define _HELICS_FILTERINFO_

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

class FilterInfo
{
public:
	FilterInfo(Core::Handle id_,Core::federate_id_t fed_id_,
		const std::string &key_,
		const std::string &type_,
		const std::string &target_,
		bool destFilter_)
		: id(id_),
		fed_id(fed_id_),
		key(key_),
		type(type_),
		dest_filter(destFilter_),
		filterTarget(target_)
	{
	}

	~FilterInfo() {}

	Core::Handle id;
	Core::federate_id_t fed_id;
	std::string key;
	std::string type;
	bool has_update = false;
	bool dest_filter = false;
	FilterOperator *filterOp = nullptr;
	std::string filterTarget;
	std::pair<Core::federate_id_t, Core::Handle> target;
private:
	std::deque<message_t *>message_queue;
public:
	message_t* getMessage(Time maxTime);
	int32_t queueSize(Time maxTime);
	void addMessage(message_t *);
	Time firstMessageTime();
};
} // namespace helics

#endif