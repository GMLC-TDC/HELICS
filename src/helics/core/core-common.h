/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_COMMON_CORE_
#define _HELICS_COMMON_CORE_

#include "helics/config.h"
#include "helics-time.h"
#include "helics/common/blocking_queue.h"
#include "helics/core/core.h"
#include "core/ActionMessage.h"

#include <cstdint>
#include <mutex> 
#include <thread> 
#include <utility> 
#include <vector> 
#include <atomic>
#include <map>
#include <unordered_map>

namespace helics {

class TestHandle;
class FederateState;

class BasicHandleInfo;
class FilterFunctions;

enum BasicHandleType:char;

class CommonCore : public Core {

public:

	CommonCore();
  virtual ~CommonCore();
  virtual void initialize (const std::string &initializationString) override;
          void terminate();
  virtual bool isInitialized () const override;
  virtual void error (federate_id_t federateID, int errorCode=-1) override;
  virtual void finalize (federate_id_t federateID) override;
  virtual void enterInitializingState (federate_id_t federateID) override;
  virtual bool enterExecutingState(federate_id_t federateID, bool iterationCompleted = true) override;
  virtual federate_id_t registerFederate (const std::string &name, const FederateInfo &info) override;
  virtual const std::string &getFederateName (federate_id_t federateId) const override;
  virtual federate_id_t getFederateId (const std::string &name) override;
  virtual void setFederationSize (unsigned int size);
  virtual int32_t getFederationSize () override;
  virtual Time timeRequest (federate_id_t federateId, Time next) override;
  virtual std::pair<Time, bool> requestTimeIterative (federate_id_t federateId, Time next, bool localConverged) override;
  virtual uint64_t getCurrentReiteration (federate_id_t federateId) override;
  virtual void setMaximumIterations (federate_id_t federateId, uint64_t iterations) override;
  virtual void setTimeDelta (federate_id_t federateId, Time time) override;
  virtual void setLookAhead(federate_id_t federateId, Time timeLookAhead) override;
  virtual void setImpactWindow(federate_id_t federateId, Time timeImpact) override;
  virtual Handle registerSubscription (federate_id_t federateId, const std::string &key, const std::string &type, const std::string &units, bool required) override;
  virtual Handle getSubscription (federate_id_t federateId, const std::string &key) override;
  virtual Handle registerPublication (federate_id_t federateId, const std::string &key, const std::string &type, const std::string &units) override;
  virtual Handle getPublication (federate_id_t federateId, const std::string &key) override;
  virtual const std::string &getUnits (Handle handle) const override;
  virtual const std::string &getType (Handle handle) const override;
  virtual void setValue (Handle handle, const char *data, uint64_t len) override;
  virtual data_t* getValue (Handle handle) override;
  virtual void dereference(data_t *data) override;
  virtual void dereference(message_t *data) override;
  virtual const Handle* getValueUpdates (federate_id_t federateId, uint64_t *size) override;
  virtual Handle registerEndpoint (federate_id_t federateId, const std::string &name, const std::string &type) override;
  virtual Handle registerSourceFilter (federate_id_t federateId, const std::string &filterName, const std::string &source, const std::string &type_in) override;
  virtual Handle registerDestinationFilter (federate_id_t federateId, const std::string &filterName, const std::string &dest, const std::string &type_in) override;
  virtual void addDependency(federate_id_t federateId, const std::string &federateName) override;
  virtual void registerFrequentCommunicationsPair (const std::string &source, const std::string &dest) override;
  virtual void send (Handle sourceHandle, const std::string &destination, const char *data, uint64_t length) override;
  virtual void sendEvent (Time time, Handle sourceHandle, const std::string &destination, const char *data, uint64_t length) override;
  virtual void sendMessage (Handle sourceHandle, message_t *message) override;
  virtual uint64_t receiveCount (Handle destination) override;
  virtual message_t* receive (Handle destination) override;
  virtual std::pair<const Handle, message_t*> receiveAny (federate_id_t federateId) override;
  virtual uint64_t receiveCountAny (federate_id_t federateId) override;
  virtual void logMessage(federate_id_t federateId, int logCode, const std::string &logMessage) override;
  virtual void setFilterOperator(Handle filter, FilterOperator* callback) override;

  virtual uint64_t receiveFilterCount(federate_id_t federateID) override;

  virtual std::pair<const Handle, message_t*> receiveAnyFilter(federate_id_t federateID) override;

  void setIdentifier(const std::string &name);
  const std::string &getIdentifier() const
  {
	  return identifier;
  }

  virtual void addCommand(const ActionMessage &m)
  {
	  _queue.push(m);
  }
protected:
  void broker();

  virtual void processCommand(ActionMessage &cmd);

  virtual void transmit(int route_id, ActionMessage &cmd) = 0;
  virtual void addRoute(int route_id, const std::string &routeInfo) = 0;
  /** get the federate Information from the federateId*/
  FederateState *getFederate(federate_id_t federateId) const;
  /** get the federate Information from a handle*/
  FederateState *getHandleFederate(Handle id_);
  /** generate a new Handle*/
  Core::Handle getNewHandle();
  /** get the basic handle information*/
  BasicHandleInfo *getHandleInfo(Handle id_) const;
  BasicHandleInfo *getLocalEndpoint(const std::string &name);
  FilterFunctions *getFilterFunctions(Handle id_);
  
  bool allInitReady() const;
private:
	int32_t global_broker_id;  //!< the identifier for the broker
	std::string identifier;  //!< a randomly generated string for initial identification of the broker
	BlockingQueue<ActionMessage> _queue; //!< primary routing queue
	std::map<Core::federate_id_t, Core::federate_id_t> global_id_translation; //!< map to translate global ids to local ones
	std::map<Core::federate_id_t, int32_t> routing_table;  //!< map for external routes  <global federate id, route id>
	std::unordered_map<std::string, int32_t> knownExternalEndpoints; //!< external map for all known external endpoints with names and route
protected:
  std::atomic<bool> _operating; //!< flag indicating that the structure is past the initialization stage indicaing that no more changes can be made to the number of federates or handles
  std::vector<std::unique_ptr<FederateState>> _federates; //!< local federate information
  std::vector<std::unique_ptr<BasicHandleInfo>> handles;  //!< local handle information
  int32_t _min_federates;  //!< the minimum number of federates that must connect before entering init mode
  int32_t _max_iterations; //!< the maximum allowable number of iterations

  std::thread _broker_thread;	//!< thread for the broker loop
  int32_t _global_federation_size;  //!< total size of the federation
  std::atomic<Core::Handle> handleCounter{ 1 };	//!< counter for the handle index
  
  std::unordered_map<std::string, Handle> publications;	//!< map of all local publications
  std::unordered_map<std::string, Handle> endpoints;	//!< map of all local endpoints
  std::atomic<bool> _initialized;  //!< indicator that the structure has been initialized
  std::map<Handle, std::unique_ptr<FilterFunctions>> filters; //!< map of all filters
 private:
  mutable std::mutex _mutex;

protected:
	void queueMessage(ActionMessage &m);
  /** function to deal with an source filters*/
  ActionMessage &processMessage(BasicHandleInfo *hndl, ActionMessage &m);
  void createBasicHandle(Handle id_, federate_id_t federateId, BasicHandleType HandleType, const std::string &key, const std::string &type, const std::string &units, bool required);

  /** check if a global id represents a local federate
  @param[in] global_id the federate global id
  @return true if it is a local federate*/
  bool isLocal(Core::federate_id_t global_id) const;
  /** get a route id for a non-local federate
  @param[in] global_id the federate global id
  @return 0 if unknown, otherwise returns the route_id*/
  int32_t getRoute(Core::federate_id_t global_id) const;
};



} // namespace helics
 
#endif /* _HELICS_TEST_CORE_ */
