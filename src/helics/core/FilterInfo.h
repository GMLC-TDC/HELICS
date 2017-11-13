/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_FILTERINFO_
#define _HELICS_FILTERINFO_

#include "helics/config.h"
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
		type(type_),
		dest_filter(destFilter_),
		filterTarget(target_)
	{
	}

	Core::Handle id=invalid_fed_id; //!< id handle of the filter
	Core::federate_id_t fed_id=invalid_Handle;	//!< id of the federate that manages the filter
	std::string key;	//!< the identifier of the filter
	std::string type;	//!< the type of data for the filter
	bool has_update = false;	//!< indicator that the filter has updates
	bool dest_filter = false;	//! indicator that the filter is a destination filter
	// there is a 6 byte gap here
	std::shared_ptr<FilterOperator> filterOp;	//!< the callback operation of the filter
	std::string filterTarget;	//!< the target endpoint name of the filter
	std::pair<Core::federate_id_t, Core::Handle> target{ invalid_fed_id,invalid_Handle };	//!< the actual target information for the filter
private:
	std::deque<std::unique_ptr<Message>>message_queue; //!< data structure containing the queued messages
	mutable std::mutex queueLock;	//!< the lock for multi-thread access to the queue
public:
	/** get the next message in the queue that comes at or before the given time
	*/
	std::unique_ptr<Message> getMessage(Time maxTime);
	/** get the current size of the queue with message up to maxTime
	*/
	int32_t queueSize(Time maxTime) const;
	/** add a message to the queue*/
	void addMessage(std::unique_ptr<Message> message);
	/** get the time of the first message in the queue*/
	Time firstMessageTime() const;
};
} // namespace helics

#endif