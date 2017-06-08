/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_ENDPOINTINFO_
#define _HELICS_ENDPOINTINFO_

#include "helics/config.h"
#include "helics-time.h"
#include "helics/common/blocking_queue.h"
#include "helics/core/core.h"

#include <cstdint>
#include <mutex> 
#include <thread> 
#include <utility> 
#include <vector> 
#include <deque>
#include <map>

namespace helics {

class EndpointInfo
{
public:
	EndpointInfo(Core::Handle id_,Core::federate_id_t fed_id_,
		const std::string &key_,
		const std::string &type_)
		: id(id_),
		fed_id(fed_id_),
		key(key_),
		type(type_)
	{
	}

	~EndpointInfo() {}

	Core::Handle id;
	Core::federate_id_t fed_id; //!< local federate id
	std::string key; //!< name of the endpoint
	std::string type;	//!< type of the endpoint
private:
	std::deque<message_t *>message_queue;
public:
	bool hasFilter;
	message_t* getMessage(Time maxTime);
	int32_t queueSize(Time maxTime);
	void addMessage(message_t *);
	Time firstMessageTime();



};
} // namespace helics

#endif