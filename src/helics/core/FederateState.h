/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef FEDERATE_STATE_H_
#define FEDERATE_STATE_H_
#pragma once


#include "ActionMessage.h"
#include "CommonCore.h"
#include "core-data.h"
#include "core.h"
#include "helics-time.h"
#include "../common/BlockingQueue3.hpp"
#include "helics/helics-config.h"
#include "core-types.h"
#include "coreFederateInfo.h"
#include "TimeDependencies.h"

#include <atomic>
#include <map>
#include <mutex>
#include <vector>

namespace helics
{
class SubscriptionInfo;
class PublicationInfo;
class EndpointInfo;
class FilterInfo;
class CommonCore;

class TimeCoordinator;

constexpr Time startupTime = Time::minVal();
constexpr Time initialTime{ -1000000.0 };
/** class managing the information about a single federate*/
class FederateState
{
  public:
    /** constructor from name and information structure*/
	FederateState(const std::string &name_, const CoreFederateInfo &info_);
	/** destructor*/
	~FederateState();
  private:
	  std::string name;  //!< the name of the federate
	  std::unique_ptr<TimeCoordinator> timeCoord;  //!< object that manages the time to determine granting
  public:
    Core::federate_id_t local_id = invalid_fed_id;  //!< id code, default to something invalid
    Core::federate_id_t global_id = invalid_fed_id;  //!< global id code, default to invalid

  private:
    std::atomic<helics_federate_state_type> state{HELICS_NONE};  //!< the current state of the federate
    bool only_update_on_change{ false };  //!< flag indicating that values should only be updated on change
    bool only_transmit_on_change{ false };  //!< flag indicating that values should only be transmitted if different than previous values
    std::map<std::string, SubscriptionInfo *> subNames;  //!< translate names to subscriptions
    std::map<std::string, PublicationInfo *> pubNames;  //!< translate names to publications
    std::map<std::string, EndpointInfo *> epNames;  //!< translate names to endpoints
public:
	std::atomic<bool> init_transmitted{ false }; //!< the initialization request has been transmitted
private:
   
	std::vector<std::unique_ptr<SubscriptionInfo>> subscriptions;  //!< storage for all the subscriptions
    std::vector<std::unique_ptr<PublicationInfo>> publications;  //!< storage for all the publications
    std::vector<std::unique_ptr<EndpointInfo>> endpoints;  //!< storage for all the endpoints
    
	
    CommonCore *parent_ = nullptr;  //!< pointer to the higher level;
  public:
    std::atomic<bool> init_requested{false};  //!< this federate has requested entry to initialization
	
    bool iterating = false;  //!< the federate is iterating at a time step
    bool hasEndpoints = false;  //!< the federate has endpoints
    //1 byte free
	int logLevel=1;
  private:
    BlockingQueue3<ActionMessage> queue;  //!< processing queue for messages incoming to a federate


    std::deque<ActionMessage> delayQueue;  //!< queue for delaying processing of messages for a time

   
    std::vector<Core::Handle> events;  //!< list of value events to process
    std::map<Core::Handle, std::vector<std::unique_ptr<Message>>> message_queue;  // structure of message queues
	Time time_granted = startupTime;  //!< the most recent granted time;
    mutable std::mutex _mutex;  //!< the mutex protecting the fed state
   
    std::atomic<bool> processing{false};  //!< the federate is processing
  private:
    /** DISABLE_COPY_AND_ASSIGN */
    FederateState (const FederateState &) = delete;
    FederateState &operator= (const FederateState &) = delete;

	
	
    /** a logging function for logging or printing messages*/
    std::function<void(int, const std::string &, const std::string &)> loggerFunction;
	std::function<std::string(const std::string &)> queryCallback; //!< a callback for additional queries
	/** find the next Value Event*/
	Time nextValueTime() const;
	/** find the next Message Event*/
	Time nextMessageTime() const;
	
	/** update the federate state */
	void setState(helics_federate_state_type newState);
  public:
   /** reset the federate to created state*/
	  void reset();
	  /** reset the federate to the initializing state*/
	  void reInit();
	  /** get the name of the federate*/
	  const std::string &getIdentifier() const
	  {
		  return name;
	  }
    helics_federate_state_type getState () const;

    SubscriptionInfo *getSubscription (const std::string &subName) const;
    SubscriptionInfo *getSubscription (Core::Handle handle_) const;
    PublicationInfo *getPublication (const std::string &subName) const;
    PublicationInfo *getPublication (Core::Handle handle_) const;
    EndpointInfo *getEndpoint (const std::string &subName) const;
    EndpointInfo *getEndpoint (Core::Handle handle_) const;


    void createSubscription (Core::Handle id_,
                             const std::string &key,
                             const std::string &type,
                             const std::string &units,
                             handle_check_mode check_mode);
    void createPublication (Core::Handle id_,
                            const std::string &key,
                            const std::string &type,
                            const std::string &units);
    void createEndpoint (Core::Handle id_, const std::string &key, const std::string &type);

	/** get the size of a message queue for a specific endpoint or filter handle*/
    uint64_t getQueueSize (Core::Handle id) const;
	/** get the sum of all message queue sizes i.e. the total number of messages available in all endpoints*/
    uint64_t getQueueSize () const;
	/** get the current iteration counter for an iterative call
	@details this will work properly even when a federate is processing
	*/
	int32_t getCurrentIteration() const;
    /** get the next available message for an endpoint
	@param id the handle of an endpoint or filter
	@return a pointer to a message -the ownership of the message is transfered to the caller*/
    std::unique_ptr<Message> receive (Core::Handle id);
    /** get any message ready for reception
    @param[out] id the endpoint related to the message*/
    std::unique_ptr<Message> receiveAny (Core::Handle &id);
	/** set the CommonCore object that is managing this Federate*/
	void setParent(CommonCore *coreObject);
	
  private:
    /** process the federate queue until returnable event
    @details processQueue will process messages until one of 3 things occur
    1.  the init state has been entered
    2.  the execution state has been granted (or init state reentered from a iterative request)
    3.  time has been granted
    4. a break event is encountered
    @return a convergence state value with an indicator of return reason and state of convergence
    */
	iteration_state processQueue ();
	/** process a single message
	@return a convergence state value with an indicator of return reason and state of convergence
	*/
	iteration_state processActionMessage(ActionMessage &m);
    /** process a message that updates the configuration of the federate for timing*/
    void processConfigUpdate(const ActionMessage &m);
	/** fill event list
	@param the time of the update
	*/
	void fillEventVector(Time currentTime);
    /** add a dependency to the timing coordination*/
    void addDependency(Core::federate_id_t fedToDependOn);
    /** add a dependent federate*/
    void addDependent(Core::federate_id_t fedThatDependsOnThis);
    /** specify the core object that manages this federate*/
  public:
	  /** get the info structure for the federate
	  */
	  CoreFederateInfo getInfo() const;
	/** update the info structure 
    @details public call so it also calls the federate lock before calling private update function
    the action Message should be CMD_FED_CONFIGURE
	*/
    void UpdateFederateInfo(const ActionMessage &m);
	/** get the granted time of a federate*/
    Time grantedTime () const { return time_granted; }
	/**get a reference to the handles of subscriptions with value updates
	*/
	const std::vector<Core::Handle> &getEvents() const;
	/** get a reference to the global ids of dependent federates
	*/
	const std::vector<Core::federate_id_t> &getDependents() const;
	
    
    void setCoreObject (CommonCore *parent);
	//the next 5 functions are the processing functions that actually process the queue
	/** process until the federate has verified its membership and assigned a global id number*/
	iteration_result waitSetup();
	/** process until the init state has been entered or there is a failure*/
	iteration_result enterInitState();
    /** function to call when entering execution state
	@param converged indicator of whether the fed should iterate if need be or not
    returns either converged or nonconverged depending on whether an iteration is needed
    */
    iteration_result enterExecutingState (iteration_request converged);
	/** request a time advancement
	@param nextTime the time of the requested advancement
	@param converged set to complete to end dense time step iteration, nonconverged to continue iterating if need be
	@return an iteration time with two elements the granted time and the convergence state
	*/
    iterationTime requestTime (Time nextTime, iteration_request converged);
	/** function to process the queue in a generic fashion used to just process messages
	with no specific end in mind
	*/
	iteration_result genericUnspecifiedQueueProcess();
	/** add an action message to the queue*/
    void addAction (const ActionMessage &action);
	/** move a message to the queue*/
	void addAction(ActionMessage &&action);

	/** log a message to the federate logger
	@param level the logging level of the message
	@param logMessageSource- the name of the object that sent the message
	@param message the message to log
	*/
	void logMessage(int level, const std::string &logMessageSource, const std::string &message) const;

	/** set the logging function 
	@details function must have signature void(int level, const std::string &sourceName, const std::string &message)
	*/
    void setLogger (std::function<void(int, const std::string &, const std::string &)> logFunction)
    {
        loggerFunction = std::move (logFunction);
    }
	/** set the query callback function
	@details function must have signature std::string(const std::string &query)
	*/
	void setQueryCallback(std::function<std::string(const std::string &)> queryCallbackFunction)
	{
		queryCallback = std::move(queryCallbackFunction);
	}
	/** generate the result of a query string
	@param query a query string
	@return the resulting string from the query*/
	std::string processQuery(const std::string &query) const;
    /** check if a value should be published or not
    @param pub_id the handle of the publication
    @param data the raw data to check
    @param len the length of the data
    @return true if it should be published, false if not
    */
    bool checkSetValue(Core::Handle pub_id, const char *data, uint64_t len) const;
};
}
#endif