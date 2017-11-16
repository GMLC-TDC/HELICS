/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_COMMON_CORE_
#define _HELICS_COMMON_CORE_
#pragma once

#include "helics/helics-config.h"
#include "helics-time.h"
#include "../common/simpleQueue.hpp"
#include "core.h"
#include "ActionMessage.h"
#include "BrokerBase.h"

#include <cstdint> 
#include <thread> 
#include <utility> 
#include <atomic>
#include <map>
#include <unordered_map>

namespace helics {

class TestHandle;
class FederateState;

class BasicHandleInfo;
class FilterCoordinator;
class logger;

enum BasicHandleType:char;

/** base class implementing a standard interaction strategy between federates
@details the CommonCore is virtual class that manages local federates and handles most of the
interaction between federate it is meant to be instantiated for specific interfederate communication
strategies*/
class CommonCore : public Core, public BrokerBase {

public:

	/** default constructor*/
CommonCore() noexcept;
/**function mainly to match some other object constructors does the same thing as the default constructor*/
CommonCore(bool arg) noexcept;
/** construct from a core name*/
CommonCore(const std::string &core_name);
/** virtual destructor*/
  virtual ~CommonCore();
  virtual void initialize (const std::string &initializationString) override final;
  /** initialize the core manager with command line arguments
  @param[in] argc the number of arguments
  @param[in] argv char pointers to the arguments
  */
  virtual void InitializeFromArgs(int argc, const char * const *argv) override;
  virtual bool isInitialized () const override final;
  virtual bool isJoinable() const override final; 
  virtual void error (federate_id_t federateID, int errorCode=-1) override final;
  virtual void finalize (federate_id_t federateID) override final;
  virtual void enterInitializingState (federate_id_t federateID) override final;
  virtual iteration_result enterExecutingState(federate_id_t federateID, iteration_request iterate = NO_ITERATION) override final;
  virtual federate_id_t registerFederate (const std::string &name, const CoreFederateInfo &info) override final;
  virtual const std::string &getFederateName (federate_id_t federateID) const override final;
  virtual federate_id_t getFederateId (const std::string &name) override final;
  virtual int32_t getFederationSize () override final;
  virtual Time timeRequest (federate_id_t federateID, Time next) override final;
  virtual iterationTime requestTimeIterative (federate_id_t federateID, Time next, iteration_request iterate) override final;
  virtual Time getCurrentTime(federate_id_t federateID) const override final;
  virtual uint64_t getCurrentReiteration (federate_id_t federateID) const override final;
  virtual void setMaximumIterations (federate_id_t federateID, uint64_t iterations) override final;
  virtual void setTimeDelta (federate_id_t federateID, Time time) override final;
  virtual void setLookAhead(federate_id_t federateID, Time timeLookAhead) override final;
  virtual void setPeriod(federate_id_t federateID, Time timePeriod) override final;
  virtual void setTimeOffset(federate_id_t federateID, Time timeOffset) override final;
  virtual void setImpactWindow(federate_id_t federateID, Time timeImpact) override final;
  virtual void setLoggingLevel(federate_id_t federateID, int loggingLevel) override final;
  virtual Handle registerSubscription (federate_id_t federateID, const std::string &key, const std::string &type, const std::string &units, handle_check_mode check_mode) override final;
  virtual Handle getSubscription (federate_id_t federateID, const std::string &key) override final;
  virtual Handle registerPublication (federate_id_t federateID, const std::string &key, const std::string &type, const std::string &units) override final;
  virtual Handle getPublication (federate_id_t federateID, const std::string &key) override final;
  virtual const std::string &getUnits (Handle handle) const override final;
  virtual const std::string &getType (Handle handle) const override final;
  virtual void setValue (Handle handle, const char *data, uint64_t len) override final;
  virtual std::shared_ptr<const data_block> getValue (Handle handle) override final;

  virtual const std::vector<Handle> &getValueUpdates (federate_id_t federateID) override final;
  virtual Handle registerEndpoint (federate_id_t federateID, const std::string &name, const std::string &type) override final;
  virtual Handle registerSourceFilter (federate_id_t federateID, const std::string &filterName, const std::string &source, const std::string &type_in,const std::string &type_out) override final;
  virtual Handle registerDestinationFilter (federate_id_t federateID, const std::string &filterName, const std::string &dest, const std::string &type_in,const std::string &type_out) override final;
  virtual void addDependency(federate_id_t federateID, const std::string &federateName) override final;
  virtual void registerFrequentCommunicationsPair (const std::string &source, const std::string &dest) override final;
  virtual void send (Handle sourceHandle, const std::string &destination, const char *data, uint64_t length) override final;
  virtual void sendEvent (Time time, Handle sourceHandle, const std::string &destination, const char *data, uint64_t length) override final;
  virtual void sendMessage (Handle sourceHandle, std::unique_ptr<Message> message) override final;
  virtual uint64_t receiveCount (Handle destination) override final;
  virtual std::unique_ptr<Message> receive (Handle destination) override final;
  virtual std::unique_ptr<Message> receiveAny (federate_id_t federateID, Handle &endpoint_id) override final;
  virtual uint64_t receiveCountAny (federate_id_t federateID) override final;
  virtual void logMessage(federate_id_t federateID, int logLevel, const std::string &logMessage) override final;
  virtual void setFilterOperator(Handle filter, std::shared_ptr<FilterOperator> callback) override final;

  virtual uint64_t receiveFilterCount(federate_id_t federateID) override final;

  virtual std::unique_ptr<Message> receiveAnyFilter(federate_id_t federateID, Handle &filter_id) override final;
  /** set the local identification for the core*/
  void setIdentifier(const std::string &name);
  /** get the local identifier for the core*/
  const std::string &getIdentifier() const override final
  {
	  return identifier;
  }

  virtual void setLoggingCallback(federate_id_t federateID, std::function<void(int, const std::string &, const std::string &)> logFunction) override final;
  
  virtual std::string query(const std::string &target, const std::string &queryStr) override;

  /** get a string representing the connection info to send data to this object*/
  virtual std::string getAddress() const=0;

 virtual bool connect() override final;
 virtual bool isConnected() const override final;
 virtual void disconnect() override final;
 void unregister();
 virtual void processDisconnect(bool skipUnregister = false) override final;
private:
	/** implementation details of the connection process
	*/
	virtual bool brokerConnect()=0;
	/** implementation details of the disconnection process
	*/
	virtual void brokerDisconnect() = 0;
    
protected:

  virtual void processCommand(ActionMessage &&cmd) override final;
  
  virtual void processPriorityCommand(ActionMessage &&command) override final;

  
  /** transit an ActionMessage to another core or broker
  @param route_id the identifier for the route information to send the message to
  @param[in] cmd the actionMessage to send*/
  virtual void transmit(int route_id, const ActionMessage &cmd) = 0;
  /** add a route to whatever internal structure manages the routes
  @param route_id the identification of the route
  @param routeInfo a string containing the information necessary to connect*/
  virtual void addRoute(int route_id, const std::string &routeInfo) = 0;
  /** get the federate Information from the federateID*/
  FederateState *getFederate(federate_id_t federateID) const;
  /** get the federate Information from the federateID*/
  FederateState *getFederate(const std::string &fedName) const;
  /** get the federate Information from a handle
  @param id a handle identifier as generated by the one of the functions*/
  FederateState *getHandleFederate(Handle id_);
  /** generate a new Handle*/
  Core::Handle getNewHandle();
  /** get the basic handle information*/
  BasicHandleInfo *getHandleInfo(Handle id_) const;
  /** get a localEndpoint from the name*/
  BasicHandleInfo *getLocalEndpoint(const std::string &name);
  /** get a filtering function object*/
  FilterCoordinator *getFilterCoordinator(Handle id_);
  /** check if all federates managed by the core are ready to enter initialization state*/
  bool allInitReady() const;
  /** check if all federates have said good-bye*/
  bool allDisconnected() const;


  virtual bool sendToLogger(federate_id_t federateID, int logLevel, const std::string &name, const std::string &message) const override;
private:
	std::string prevIdentifier;  //!< storage for the case of requiring a renaming
	
	std::map<Core::federate_id_t, Core::federate_id_t> global_id_translation; //!< map to translate global ids to local ones
	std::map<Core::federate_id_t, int32_t> routing_table;  //!< map for external routes  <global federate id, route id>
	simpleQueue<ActionMessage> delayTransmitQueue; //!< FIFO queue for transmissions to the root that need to be delays for a certain time
	std::unordered_map<std::string, int32_t> knownExternalEndpoints; //!< external map for all known external endpoints with names and route

	/** actually transmit messages that were delayed until the core was actually registered*/
	void transmitDelayedMessages();
	/**function for doing the actual routing either to a local fed or up the broker chain*/
	void routeMessage(ActionMessage &cmd, federate_id_t dest);
	/** function for routing a message from based on the destination specified in the ActionMessage*/
	void routeMessage(const ActionMessage &cmd);
	/** process any filter or route the message*/
	void processMessageFilter(ActionMessage &cmd);
	
protected:
	
	int32_t _global_federation_size = 0;  //!< total size of the federation
	std::vector<std::unique_ptr<FederateState>> _federates; //!< local federate information
														  //using pointers to minimize time in a critical section- though this should be timed more in the future
  std::vector<std::unique_ptr<BasicHandleInfo>> handles;  //!< local handle information
  std::atomic<Core::Handle> handleCounter{ 1 };	//!< counter for the handle index
  
  std::unordered_map<std::string, Handle> publications;	//!< map of all local publications
  std::unordered_map<std::string, Handle> endpoints;	//!< map of all local endpoints
  std::unordered_map<std::string, federate_id_t> federateNames;  //!< map of federate names to id
  std::atomic<int> queryCounter{ 0 };
  std::map<Handle, std::unique_ptr<FilterCoordinator>> filters; //!< map of all filters
 private:
  mutable std::mutex _mutex; //!< mutex protecting the federate creation and modification
  mutable std::mutex _handlemutex; //!< mutex protecting the publications and subscription structures
/** a logging function for logging or printing messages*/
  
protected:
	/** add a message to the queue*/
	void queueMessage(ActionMessage &m);
  /** function to deal with a source filters*/
  ActionMessage &processMessage(BasicHandleInfo *hndl, ActionMessage &m);
  /** add a new handle to the generic structure
  and return a ptr to it, the ptr is non-owning
  */
  BasicHandleInfo* createBasicHandle(Handle id_, federate_id_t global_federateId, federate_id_t local_federateId, BasicHandleType HandleType, const std::string &key, const std::string &type, const std::string &units, bool required);
  /** add a new handle to the generic structure
  and return a ptr to it the ptr is non owning
  variation targeted at filters
  */
  BasicHandleInfo *createBasicHandle(Handle id_,
	  federate_id_t global_federateId,
	  federate_id_t local_federateId,
	  BasicHandleType HandleType,
	  const std::string &key,
	  const std::string &target,
	  const std::string &type_in,
	  const std::string &type_out);

  /** check if a global id represents a local federate
  @param[in] global_id the federate global id
  @return true if it is a local federate*/
  bool isLocal(Core::federate_id_t global_id) const;
  /** get a route id for a non-local federate
  @param[in] global_id the federate global id
  @return 0 if unknown, otherwise returns the route_id*/
  int32_t getRoute(Core::federate_id_t global_id) const;

  /** process a message for potential additions to the filter ordering
  @param command the message to process
  */
  void processFilterInfo(ActionMessage &command);

  /** organize filters
  @detsils organize the filter and report and potential warnings and errors
  */
  void organizeFilterOperations();

  std::string federateQuery(Core::federate_id_t id, const std::string &queryStr) const;
};


} // namespace helics
 
#endif /* _HELICS_TEST_CORE_ */
