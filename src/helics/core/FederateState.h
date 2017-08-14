/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef FEDERATE_STATE_H_
#define FEDERATE_STATE_H_
#pragma once


#include "CommonCore.h"
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

/** data class containing information about interfederate dependencies*/
class DependencyInfo
{
public:
	Core::federate_id_t fedID;  //!< identifier for the dependent federate
	bool grant=false;	//!< whether time has been granted
	bool converged=false;	//!< whether it is currently converged
	bool exec_requested = false;	//!< whether execution state has been granted
	Time Tnext=timeZero;  //!<next time computation
	Time Te=timeZero;		//!< executation time computation
	Time Tdemin=timeZero;	//!< min dependency event time
	/** default constructor*/
	DependencyInfo() = default;
	/** construct from a federate id*/
	DependencyInfo(Core::federate_id_t id) :fedID(id) {};
};

/** class managing the information about a single federate*/
class FederateState
{
public:
	/** constructor from name and information structure*/
	FederateState(const std::string &name_, const CoreFederateInfo &info_)
		: name(name_),info(info_)
	{
		state = HELICS_CREATED;
	}

	std::string name; //!< the name of the federate
	CoreFederateInfo info;	//!< basic federate info the core uses
	Core::federate_id_t local_id = invalid_fed_id; //!< id code, default to something invalid
	Core::federate_id_t global_id = invalid_fed_id; //!< global id code, default to invalid
	
private:
	std::atomic<helics_federate_state_type> state{ HELICS_NONE };	//!< the current state of the federate
	std::map<std::string, SubscriptionInfo *> subNames;	//!< translate names to subscriptions
	std::map<std::string, PublicationInfo *> pubNames;	//!< translate names to publications
	std::map<std::string, EndpointInfo *> epNames;	//!< translate names to endpoints
	std::map<std::string, FilterInfo *> filterNames;	//!< translate names to filterObjects
	std::vector<std::unique_ptr<SubscriptionInfo>> subscriptions;	//!< storage for all the subscriptions
	std::vector<std::unique_ptr<PublicationInfo>> publications;	//!< storage for all the publications
	std::vector<std::unique_ptr<EndpointInfo>> endpoints; //!< storage for all the endpoints
	std::vector<std::unique_ptr<FilterInfo>> filters; //!< storage for all the filters

	CommonCore *parent_=nullptr;  //!< pointer to the higher level;  
public:
	std::atomic<bool> init_requested{ false }; //!< this federate has requested entry to initialization
	bool processing = false;	//!< the federate is processing
	bool iterating = false;	//!< the federate is iterating at a timestep
	bool hasEndpoints = false;	//!< the federate has endpoints
	BlockingQueue<ActionMessage> queue; //!< processing queue for messages incoming to a federate

private:
	std::deque<ActionMessage> delayQueue;  //!< queue for delaying processing of messages for a time
public:
	
	Time time_granted = timeZero;	//!< the most recent time granted
	Time time_requested = timeZero;	//!< the most recent time requested
	Time time_next = timeZero;	//!< the next time to process
	Time time_minDe = timeZero;	//!< the minimum dependent event
	Time time_minTe = timeZero;	//!< the minimum event time
	Time time_event = timeZero;	//!< the time of the next processing event
	std::atomic<int> iteration{ 0 };  //!< iteration counter
	std::uint32_t max_iterations = 3;  //!< the maximum allowable number of iterations
	std::vector<Core::Handle> events;	//!< list of events to process
	std::map<Core::Handle, std::vector<std::unique_ptr<Message>>> message_queue; //structure of message queues
	mutable std::mutex _mutex; //!< the mutex protecting the fed state

	std::vector<DependencyInfo> dependencies;  //federates which this Federate is temporally dependent on
	std::vector<Core::federate_id_t> dependents;	//federates which temporally depend on this federate
	
private:
	/** DISABLE_COPY_AND_ASSIGN */
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
	std::unique_ptr<Message> receive(Core::Handle id);
	/** get any message ready for reception
	@param[out] id the the endpoint related to the message*/
	std::unique_ptr<Message> receiveAny(Core::Handle &id);
	/** get any message ready for processing by a filter
	@param[out] id the the filter related to the message*/
	std::unique_ptr<Message> receiveForFilter(Core::Handle &id);
	/** process the federate queue until returnable event
	@details processQueue will process messages until one of 3 things occur
	1.  the init state has been entered
	2.  the executation state has been granted (or init state reentered from a iterative request)
	3.  time has been granted
	4. a break event is encountered
	@return will return false if iteration is allowed --entering init state will always return true
	*/
	bool processQueue();

	void generateKnownDependencies();
	void addDependency(Core::federate_id_t fedToDependOn);
	void addDependent(Core::federate_id_t fedThatDependsOnThis);

	void setCoreObject(CommonCore *parent);

	iterationTime requestTime(Time nextTime, convergence_state converged);
};

}
#endif