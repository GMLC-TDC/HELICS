/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef FEDERATE_STATE_H_
#define FEDERATE_STATE_H_
#pragma once


#include "core-common.h"
#include "helics-time.h"
#include "helics/config.h"
#include "helics/common/blocking_queue.h"
#include "core.h"
#include "core-data.h"
#include "ActionMessage.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include <atomic>

namespace helics
{
class SubscriptionInfo;
class PublicationInfo;
class EndpointInfo;
class FilterInfo;
class CommonCore;

class DependencyInfo
{
public:
	Core::federate_id_t fedID;
	bool grant=false;
	bool converged=false;
	bool exec_requested = false;
	Time Tnext=timeZero;  //!<next time computation
	Time Te=timeZero;		//!< executation time computation
	Time Tdemin=timeZero;	//!< min dependency event time
	
	DependencyInfo() = default;
	DependencyInfo(Core::federate_id_t id) :fedID(id) {};
};

class FederateState
{
public:
	FederateState(const std::string &name_, const FederateInfo &info_)
		: name(name_),info(info_)
	{
		state = HELICS_CREATED;
	}

	std::string name;
	FederateInfo info;
	Core::federate_id_t local_id = invalid_fed_id; //!< id code, default to something invalid
	Core::federate_id_t global_id = invalid_fed_id; //!< global id code, default to invalid
	
private:
	std::atomic<helics_federate_state_type> state{ HELICS_NONE };
	std::map<std::string, SubscriptionInfo *> subNames;
	std::map<std::string, PublicationInfo *> pubNames;
	std::map<std::string, EndpointInfo *> epNames;
	std::map<std::string, FilterInfo *> filterNames;
	std::vector<std::unique_ptr<SubscriptionInfo>> subscriptions;
	std::vector<std::unique_ptr<PublicationInfo>> publications;
	std::vector<std::unique_ptr<EndpointInfo>> endpoints;
	std::vector<std::unique_ptr<FilterInfo>> filters;

	CommonCore *parent_=nullptr;  //!< pointer to the higher level;  
public:
	BlockingQueue<ActionMessage> queue; //!< processing queue for messages incoming to a federate

private:
	std::deque<ActionMessage> delayQueue;  //!< queue for delaying processing of messages for a time
public:
	bool init_requested = true;
	bool processing = false;
	bool iterating = false;
	bool hasEndpoints = false;
	std::atomic<int> iteration{ 0 };
	Time time_granted = timeZero;
	Time time_requested = timeZero;
	Time time_next = timeZero;
	Time time_minDe = timeZero;
	Time time_minTe = timeZero;
	Time time_event = timeZero;
	std::uint64_t max_iterations = 3;
	std::vector<Core::Handle> events;
	std::map<Core::Handle, std::deque<message_t *>> message_queue;
	std::mutex _mutex;

	std::vector<DependencyInfo> dependencies;  //federates which this Federate is temporally dependent on
	std::vector<Core::federate_id_t> dependents;	//federates which temporally depend on this federate
	/** DISABLE_COPY_AND_ASSIGN */
private:
	FederateState(const FederateState &) = delete;
	FederateState &operator= (const FederateState &) = delete;

	int processExternalTimeMessage(ActionMessage &cmd);
	int updateTimeFactors();

	int processExecRequest(ActionMessage &cmd);

	int checkExecEntry();
	//take a global id and get a reference to the dependencyInfo for the other fed
	DependencyInfo &getDependencyInfo(Core::federate_id_t ofed);
public:

	void setState(helics_federate_state_type newState);
	helics_federate_state_type getState() const;

	SubscriptionInfo *getSubscription(const std::string &subName) const;
	SubscriptionInfo *getSubscription(Core::Handle handle_) const;
	PublicationInfo *getPublication(const std::string &subName) const;
	PublicationInfo *getPublication(Core::Handle handle_) const;
	EndpointInfo *getEndpoint(const std::string &subName) const;
	EndpointInfo *getEndpoint(Core::Handle handle_) const;
	FilterInfo *getFilter(const std::string &subName)const ;
	FilterInfo *getFilter(Core::Handle handle_) const;

	void createSubscription(Core::Handle id_, const std::string &key,
		const std::string &type,
		const std::string &units,
		bool required);
	void createPublication(Core::Handle id_, const std::string &key,
		const std::string &type,
		const std::string &units);
	void createEndpoint(Core::Handle id_, const std::string &key,
		const std::string &type);
	void createFilter(Core::Handle id_, bool destFilter, const std::string &key, const std::string &target,
		const std::string &type);

	uint64_t getQueueSize(Core::Handle id) const;
	uint64_t getQueueSize() const;
	uint64_t getFilterQueueSize() const;
	message_t *receive(Core::Handle id);
	std::pair<Core::Handle, message_t*> receive();
	std::pair<Core::Handle, message_t*> receiveForFilter();
	/** process the federate queue until returnable event
	@details processQueue will process messages until one of 3 things occur
	1.  the init state has been entered
	2.  the executation state has been granted (or init state reentered from a iterative request)
	3.  time has been granted
	@return will return false if iteration is allowed --entering init state will always return true
	*/
	bool processQueue();

	void generateKnownDependencies();
	void addDependency(Core::federate_id_t);
	void addDependent(Core::federate_id_t);

	void setCoreObject(CommonCore *parent);

	std::pair<Time, bool> requestTime(Time nextTime, bool iterationRequested);
};

}
#endif