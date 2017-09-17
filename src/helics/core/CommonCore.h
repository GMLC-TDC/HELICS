/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_COMMON_CORE_
#define _HELICS_COMMON_CORE_

#include "helics/config.h"
#include "helics-time.h"
#include "helics/common/BlockingQueue.hpp"
#include "helics/common/simpleQueue.hpp"
#include "helics/core/core.h"
#include "core/ActionMessage.h"

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
class CommonCore : public Core {

public:

	/** default constructor*/
CommonCore() noexcept;
/** construct from a core name*/
CommonCore(const std::string &core_name);
/** virtual destructor*/
  virtual ~CommonCore();
  virtual void initialize (const std::string &initializationString) override final;
  /** initialize the core manager with command line arguments
  @param[in] argc the number of arguments
  @param[in] argv char pointers to the arguments
  */
  virtual void initializeFromArgs(int argc, char *argv[]);
  virtual bool isInitialized () const override final;
  virtual bool isJoinable() const override final; 
  virtual void error (federate_id_t federateID, int errorCode=-1) override final;
  virtual void finalize (federate_id_t federateID) override final;
  virtual void enterInitializingState (federate_id_t federateID) override final;
  virtual convergence_state enterExecutingState(federate_id_t federateID, convergence_state converged = convergence_state::complete) override final;
  virtual federate_id_t registerFederate (const std::string &name, const CoreFederateInfo &info) override final;
  virtual const std::string &getFederateName (federate_id_t federateId) const override final;
  virtual federate_id_t getFederateId (const std::string &name) override final;
  virtual int32_t getFederationSize () override final;
  virtual Time timeRequest (federate_id_t federateId, Time next) override final;
  virtual iterationTime requestTimeIterative (federate_id_t federateId, Time next, convergence_state converged) override final;
  virtual uint64_t getCurrentReiteration (federate_id_t federateId) override final;
  virtual void setMaximumIterations (federate_id_t federateId, uint64_t iterations) override final;
  virtual void setTimeDelta (federate_id_t federateId, Time time) override final;
  virtual void setLookAhead(federate_id_t federateId, Time timeLookAhead) override final;
  virtual void setImpactWindow(federate_id_t federateId, Time timeImpact) override final;
  virtual Handle registerSubscription (federate_id_t federateId, const std::string &key, const std::string &type, const std::string &units, handle_check_mode check_mode) override final;
  virtual Handle getSubscription (federate_id_t federateId, const std::string &key) override final;
  virtual Handle registerPublication (federate_id_t federateId, const std::string &key, const std::string &type, const std::string &units) override final;
  virtual Handle getPublication (federate_id_t federateId, const std::string &key) override final;
  virtual const std::string &getUnits (Handle handle) const override final;
  virtual const std::string &getType (Handle handle) const override final;
  virtual void setValue (Handle handle, const char *data, uint64_t len) override final;
  virtual std::shared_ptr<const data_block> getValue (Handle handle) override final;

  virtual const std::vector<Handle> &getValueUpdates (federate_id_t federateId) override final;
  virtual Handle registerEndpoint (federate_id_t federateId, const std::string &name, const std::string &type) override final;
  virtual Handle registerSourceFilter (federate_id_t federateId, const std::string &filterName, const std::string &source, const std::string &type_in,const std::string &type_out) override final;
  virtual Handle registerDestinationFilter (federate_id_t federateId, const std::string &filterName, const std::string &dest, const std::string &type_in,const std::string &type_out) override final;
  virtual void addDependency(federate_id_t federateId, const std::string &federateName) override final;
  virtual void registerFrequentCommunicationsPair (const std::string &source, const std::string &dest) override final;
  virtual void send (Handle sourceHandle, const std::string &destination, const char *data, uint64_t length) override final;
  virtual void sendEvent (Time time, Handle sourceHandle, const std::string &destination, const char *data, uint64_t length) override final;
  virtual void sendMessage (Handle sourceHandle, std::unique_ptr<Message> message) override final;
  virtual uint64_t receiveCount (Handle destination) override final;
  virtual std::unique_ptr<Message> receive (Handle destination) override final;
  virtual std::unique_ptr<Message> receiveAny (federate_id_t federateId, Handle &endpoint_id) override final;
  virtual uint64_t receiveCountAny (federate_id_t federateId) override final;
  virtual void logMessage(federate_id_t federateId, int logLevel, const std::string &logMessage) override final;
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
  /** add a command to the process queue*/
 void addCommand(const ActionMessage &m);
 /** move a command into the commandQueue*/
 void addCommand(ActionMessage &&m);
 virtual bool connect() override final;
 virtual bool isConnected() const override final;
 virtual void disconnect() override final;
private:
	/** implementation details of the connection process
	*/
	virtual bool brokerConnect()=0;
	/** implementation details of the disconnection process
	*/
	virtual void brokerDisconnect() = 0;
protected:
	/** start main broker loop*/
  void queueProcessingLoop();
  /** process a single command action
  @details cmd may be modified by this function*/
  virtual void processCommand(ActionMessage &cmd);
  /** function to process a priority command independent of the main queue
  @detailed called when processing a priority command.  The priority command has a response message which gets sent
  this mainly deals with some of the registration functions
  @param[in] command the command to process
  @return a action message response to the priority command 
  */
  void processPriorityCommand(const ActionMessage &command);
  /** transit an ActionMessage to another core or broker
  @param route_id the identifier for the route information to send the message to
  @param[in] cmd the actionMessage to send*/
  virtual void transmit(int route_id, const ActionMessage &cmd) = 0;
  /** add a route to whatever internal structure manages the routes
  @param route_id the identification of the route
  @param routeInfo a string containing the information necessary to connect*/
  virtual void addRoute(int route_id, const std::string &routeInfo) = 0;
  /** get the federate Information from the federateId*/
  FederateState *getFederate(federate_id_t federateId) const;
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
  /** sendaMessage to the logging system
  */
  void sendToLogger(federate_id_t federateID, int logLevel, const std::string &name, const std::string &message) const;
private:
	std::atomic<int32_t> global_broker_id{ 0 };  //!< global identifier for the broker
	int maxLogLevel=1;  //!< the logging level to use levels >=this will be logged
	std::string identifier;  //!< an identifier for the broker
	std::string prevIdentifier;  //!< storage for the case of requiring a renaming
	BlockingQueue2<ActionMessage> _queue; //!< primary routing queue
	std::map<Core::federate_id_t, Core::federate_id_t> global_id_translation; //!< map to translate global ids to local ones
	std::map<Core::federate_id_t, int32_t> routing_table;  //!< map for external routes  <global federate id, route id>
	simpleQueue<ActionMessage> delayTransmitQueue; //!< FIFO queue for transmissions to the root that need to be delays for a certain time
	std::unordered_map<std::string, int32_t> knownExternalEndpoints; //!< external map for all known external endpoints with names and route
	std::unique_ptr<logger> loggingObj;  //!< default logging object to use if the logging callback is not specified

	/** actually transmit messages that were delayed until the core was actually registered*/
	void transmitDelayedMessages();
	/**function for doing the actual routing either to a local fed or up the broker chain*/
	void routeMessage(ActionMessage &cmd, federate_id_t dest);
	/** function for routing a message from based on the destination specified in the ActionMessage*/
	void routeMessage(const ActionMessage &cmd);
	
protected:
	/** enumeration of the possible core states*/
	enum core_state_t :int
	{
		created = -5,
		initialized = -4,
		connecting = -3,
		connected = -2,
		initializing=-1,
		operating=0,
		terminated=3,
		errored=7,
	};
	std::atomic<core_state_t> coreState{created}; //!< flag indicating that the structure is past the initialization stage indicaing that no more changes can be made to the number of federates or handles
	int32_t _global_federation_size = 0;  //!< total size of the federation
	std::vector<std::unique_ptr<FederateState>> _federates; //!< local federate information
														  //using pointers to minimize time in a critical section- though this should be timed more in the future
  std::vector<std::unique_ptr<BasicHandleInfo>> handles;  //!< local handle information
  int32_t _min_federates;  //!< the minimum number of federates that must connect before entering init mode
  int32_t _max_iterations; //!< the maximum allowable number of iterations
  std::thread _queue_processing_thread;	//!< thread for processing the queue
  std::string logFile;  //!< file to log to
  std::atomic<Core::Handle> handleCounter{ 1 };	//!< counter for the handle index
  
  std::unordered_map<std::string, Handle> publications;	//!< map of all local publications
  std::unordered_map<std::string, Handle> endpoints;	//!< map of all local endpoints
  std::unordered_map<std::string, federate_id_t> federateNames;  //!< map of federate names to id
  std::map<Handle, std::unique_ptr<FilterCoordinator>> filters; //!< map of all filters
 private:
  mutable std::mutex _mutex; //!< mutex protecting the federate creation and modification
  mutable std::mutex _handlemutex; //!< mutex protecting the publications and subscription structures
/** a logging function for logging or printing messages*/
  std::function<void(int, const std::string &, const std::string &)> loggerFunction;
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
  variation targetted at filters
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
};

//just enumerating some print levels 
enum print_level : int
{
	no_print = 0,  //!< never print
	error = 1,  //!< only print errors
	warning = 2,  //!< print/log warning and errors
	normal = 3,  //!< defualt print level
	debug = 4,  //!< debug level prints
	trace = 5,  //!< trace level printing
};

#define LOG_ERROR(id,ident, message) sendToLogger (id,print_level::error,ident,message);
#define LOG_WARNING(id, ident, message) sendToLogger (id,print_level::warning,ident,message);

#define LOG_ENABLE
#define DEBUG_LOG_ENABLE
#define TRACE_LOG_ENABLE

#ifdef LOG_ENABLE
#define LOG_NORMAL(id, ident, message) if (maxLogLevel>=print_level::normal){sendToLogger (id,print_level::normal,ident,message);}

#ifdef DEBUG_LOG_ENABLE
#define LOG_DEBUG(id, ident, message) if (maxLogLevel>=print_level::debug){sendToLogger (id,print_level::debug,ident,message);}
#else
#define LOG_DEBUG(id, ident, message)
#endif

#ifdef TRACE_LOG_ENABLE
#define LOG_TRACE(id, ident, message) if (maxLogLevel>=print_level::trace){sendToLogger (id,print_level::debug,ident,message);}
#else
#define LOG_TRACE(id, ident, message)
#endif
#else
#define LOG_NORMAL(id, ident, message)
#define LOG_DEBUG(id, ident, message)
#define LOG_TRACE(id,ident,message)
#endif

} // namespace helics
 
#endif /* _HELICS_TEST_CORE_ */
