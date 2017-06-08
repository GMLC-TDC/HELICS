/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef CORE_BROKER_H_
#define CORE_BROKER_H_
#pragma once

#include <atomic>
#include <vector>
#include <memory>
#include <string>
#include <thread>
#include <mutex>
#include <map>
#include <unordered_map>

#include "BasicHandleInfo.h"
#include "ActionMessage.h"
#include "common/blocking_queue.h"

namespace helics
{
class BasicFedInfo
{
public:
	std::string name;
	Core::federate_id_t global_id=invalid_fed_id;
	int32_t route_id=invalid_fed_id;
	bool broker_ = false;
	BasicFedInfo(const std::string &fedname) :name(fedname) {};
};

class BasicBrokerInfo
{
public:
	std::string name;
	Core::federate_id_t global_id=invalid_fed_id;
	int32_t route_id=invalid_fed_id;
	std::string routeInfo;
	bool _initRequested = false;
	BasicBrokerInfo(const std::string &brokerName) :name(brokerName) {};
	
};

/** class implementing most of the functionality of a generic broker
Basically acts as a router for information,  deals with stuff internally if it can and sends higher up if it can't
or does something else if it is the root of the tree
*/
class CoreBroker
{
protected:
	std::atomic<bool> _operating{ false }; //!< flag indicating that the structure is past the initialization stage indicaing that no more changes can be made to the number of federates or handles
	bool _isRoot = false;  //!< set to true if this object is a root broker
	bool _gateway = false;  //!< set to true if this broker should act as a gateway.  
private:
	std::vector<std::pair<Core::federate_id_t, bool>> localBrokersInit; //!< indicator if the local brokers are ready to init
	std::vector<BasicFedInfo> _federates; //!< container for all federates
	std::vector<BasicHandleInfo> _handles; //!< container for the basic info for all handles
	std::vector<BasicBrokerInfo> _brokers;  //!< container for the basic broker info for all subbrokers
	int32_t global_broker_id;  //!< the identifier for the broker
	std::string local_broker_identifier;  //!< a randomly generated string  or assigned name for initial identification of the broker
	BlockingQueue<ActionMessage> _queue; //!< primary routing queue
	std::map<std::string, int32_t> fedNames;  //!< a map to lookup federates
	std::map<std::string, int32_t> brokerNames;  //!< a map to lookup brokers
	std::map<std::string, int32_t> publications; //!< map of publications;
	std::map<std::string, int32_t> endpoints;  //!< map of endpoints
	std::map<Core::federate_id_t, int32_t> global_id_translation; //!< map to translate global ids to local ones
	std::map<Core::federate_id_t, int32_t> routing_table;  //!< map for external routes  <global federate id, route id>
	std::map<Core::federate_id_t, int32_t> broker_table;  //!< map for brokers to map federates to brokers
	std::unordered_map<std::string, int32_t> knownExternalEndpoints; //!< external map for all known external endpoints with names and route
	std::thread _broker_thread;  //!< thread for running the broker
protected:
	std::atomic<bool> _initialized{ false }; //!< indicator if the system is initialized (mainly if the thread is running)
private:
	int32_t _min_federates=1;  //!< storage for the min number of federates
	int32_t _min_brokers=1;	//!< storage for the min number of brokers before starting
	mutable std::mutex mutex_;  //!< mutex lock for the federate information that could come in from multiple sources
	/** primary thread executable --the function that continually loops to process all the messages
	*/
	void broker();
	/** function that processes all the messages
	@param[in] command -- the message to process
	*/
	virtual void processCommand(ActionMessage &command);
	/** function to process a priority command independent of the main queue
	@detailed called from addMessage function which detects if the command is a priority command
	this mainly deals with some of the registration functions
	@param[in] command the command to process
	*/
	void processPriorityCommand(const ActionMessage &command);
protected:
	/** this function is the one that will change for various flavors of broker communication
	@details it takes a route info- a code of where to send the data and an action message
	and proceeds to transmit it to the appropriate location
	@param[in] route -the identifier for the routing information
	@param[in] command the actionMessage to transmit
	*/
	virtual void transmit(int32_t route, const ActionMessage &command) = 0;
	/** add a route to the type specific routing information and establish the connection
	@details add a route to a table, the connection information is contained in the string with the described identifier
	@param[in] route_id  the identifier for the route
	@param[in] routeInfo  a string describing the connection info
	*/
	virtual void addRoute(int route_id, const std::string &routeInfo) = 0;
public:
	CoreBroker(bool isRoot = false) noexcept;
	~CoreBroker();
	virtual void Initialize(const std::string &initializationString);
	/** add a message to the queue to process*/
	void addMessage(const ActionMessage &m);
	/** check if all the local federates are ready to be initialized
	@return true if everyone is ready, false otherwise
	*/
	bool allInitReady() const;
	void setIdentifier(const std::string &name);
	const std::string &getIdentifier() const
	{
		return local_broker_identifier;
	}
private:
	void checkPublications();
	void checkEndpoints();
	void checkFilters();
	/** locate the route to take to a particular federate*/
	int32_t getRoute(Core::federate_id_t fedid) const;
	int32_t getFedByName(const std::string &fedName) const;
	int32_t getBrokerByName(const std::string &brokerName) const;
};
} //namespace helics

#endif

