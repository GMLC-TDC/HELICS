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
#include "helics/common/blocking_queue.h"
#include "helics/config.h"
#include "core/core-types.h"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <map>
#include <mutex>
#include <sstream>
#include <thread>
#include <utility>
#include <vector>

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
    bool grant = false;  //!< whether time has been granted
    bool converged = false;  //!< whether it is currently converged
    bool exec_requested = false;  //!< whether execution state has been granted
    Time Tnext = timeZero;  //!<next time computation
    Time Te = timeZero;  //!< executation time computation
    Time Tdemin = timeZero;  //!< min dependency event time
    /** default constructor*/
    DependencyInfo () = default;
    /** construct from a federate id*/
    DependencyInfo (Core::federate_id_t id) : fedID (id){};
};

/** class managing the information about a single federate*/
class FederateState
{
  public:
    /** constructor from name and information structure*/
    FederateState (const std::string &name_, const CoreFederateInfo &info_) : name (name_), info (info_)
    {
        state = HELICS_CREATED;
    }

    std::string name;  //!< the name of the federate
  private:
    CoreFederateInfo info;  //!< basic federate info the core uses
  public:
    Core::federate_id_t local_id = invalid_fed_id;  //!< id code, default to something invalid
    Core::federate_id_t global_id = invalid_fed_id;  //!< global id code, default to invalid

  private:
    std::atomic<helics_federate_state_type> state{HELICS_NONE};  //!< the current state of the federate
    std::map<std::string, SubscriptionInfo *> subNames;  //!< translate names to subscriptions
    std::map<std::string, PublicationInfo *> pubNames;  //!< translate names to publications
    std::map<std::string, EndpointInfo *> epNames;  //!< translate names to endpoints
    std::map<std::string, FilterInfo *> filterNames;  //!< translate names to filterObjects
    std::vector<std::unique_ptr<SubscriptionInfo>> subscriptions;  //!< storage for all the subscriptions
    std::vector<std::unique_ptr<PublicationInfo>> publications;  //!< storage for all the publications
    std::vector<std::unique_ptr<EndpointInfo>> endpoints;  //!< storage for all the endpoints
    std::vector<std::unique_ptr<FilterInfo>> filters;  //!< storage for all the filters

    CommonCore *parent_ = nullptr;  //!< pointer to the higher level;
  public:
    std::atomic<bool> init_requested{false};  //!< this federate has requested entry to initialization

    bool iterating = false;  //!< the federate is iterating at a timestep
    bool hasEndpoints = false;  //!< the federate has endpoints
  private:
    BlockingQueue<ActionMessage> queue;  //!< processing queue for messages incoming to a federate


    std::deque<ActionMessage> delayQueue;  //!< queue for delaying processing of messages for a time

    // the variables for time coordination
    Time time_granted = timeZero;  //!< the most recent time granted
    Time time_requested = timeZero;  //!< the most recent time requested
    Time time_next = Time::maxVal();  //!< the next time to process
    Time time_minDe = Time::maxVal();  //!< the minimum dependent event
    Time time_minTe = Time::maxVal();  //!< the minimum event time
    Time time_event = Time::maxVal();  //!< the time of the next processing event
	Time time_message = Time::maxVal();	//!< the time of the earliest message
	Time time_value = Time::maxVal();	//!< the time of the earliest value 

    std::atomic<int32_t> iteration{0};  //!< iteration counter
    std::vector<Core::Handle> events;  //!< list of value events to process
    std::map<Core::Handle, std::vector<std::unique_ptr<Message>>> message_queue;  // structure of message queues

    mutable std::mutex _mutex;  //!< the mutex protecting the fed state
    std::vector<DependencyInfo> dependencies;  // federates which this Federate is temporally dependent on
    std::vector<Core::federate_id_t> dependents;  // federates which temporally depend on this federate
    std::atomic<bool> processing{false};  //!< the federate is processing
  private:
    /** DISABLE_COPY_AND_ASSIGN */
    FederateState (const FederateState &) = delete;
    FederateState &operator= (const FederateState &) = delete;

	/** process a message related to exec request
	@return true if it did anything
	*/
	bool processExecRequest(ActionMessage &cmd);
	/** check if entry to the executing state can be granted*/
	convergence_state checkExecEntry();
	/** process a message related to time
	@return true if it did anything
	*/
	bool processExternalTimeMessage (ActionMessage &cmd);
	/** compute updates to time and determine if time could be granted*/
	convergence_state updateTimeFactors ();

	
    // take a global id and get a reference to the dependencyInfo for the other fed
    DependencyInfo &getDependencyInfo (Core::federate_id_t ofed);
    /** a logging function for logging or printing messages*/
    std::function<void(int, const std::string &, const std::string &)> loggerFunction;

	/** helper function for computing various time values*/
	void computeNextEventTime(Time requested);
	/** update the federate state */
	void setState(helics_federate_state_type newState);
  public:
   /** reset the federate to created state*/
	  void reset();
	  /** reset the federate to the initializing state*/
	  void reInit();
    helics_federate_state_type getState () const;

    SubscriptionInfo *getSubscription (const std::string &subName) const;
    SubscriptionInfo *getSubscription (Core::Handle handle_) const;
    PublicationInfo *getPublication (const std::string &subName) const;
    PublicationInfo *getPublication (Core::Handle handle_) const;
    EndpointInfo *getEndpoint (const std::string &subName) const;
    EndpointInfo *getEndpoint (Core::Handle handle_) const;
    FilterInfo *getFilter (const std::string &subName) const;
    FilterInfo *getFilter (Core::Handle handle_) const;

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
    void createSourceFilter (Core::Handle id_,
                             const std::string &key,
                             const std::string &target,
                             const std::string &type);
    void createDestFilter (Core::Handle id_,
                           const std::string &key,
                           const std::string &target,
                           const std::string &type);

	/** get the size of a message queue for a specific endpoint or filter handle*/
    uint64_t getQueueSize (Core::Handle id) const;
	/** get the sum of all message queue sizes ie the total number of messages available in all endpoints*/
    uint64_t getQueueSize () const;
	/** get the sum of all messages in filter queues*/
	uint64_t getFilterQueueSize() const;
	/** get the current iteration counter for an iterative call
	@details this will work properly even when a federate is processing
	*/
    int32_t getCurrentIteration () const { return iteration; }
    /** get the next available message for an endpoint
	@param id the handle of an endpoint or filter
	@return a pointer to a message -the ownership of the message is transfered to the caller*/
    std::unique_ptr<Message> receive (Core::Handle id);
    /** get any message ready for reception
    @param[out] id the the endpoint related to the message*/
    std::unique_ptr<Message> receiveAny (Core::Handle &id);
    /** get any message ready for processing by a filter
    @param[out] id the the filter related to the message*/
    std::unique_ptr<Message> receiveForFilter (Core::Handle &id);

  private:
    /** process the federate queue until returnable event
    @details processQueue will process messages until one of 3 things occur
    1.  the init state has been entered
    2.  the executation state has been granted (or init state reentered from a iterative request)
    3.  time has been granted
    4. a break event is encountered
    @return a convergence state value with an indicator of return reason and state of convergence
    */
	convergence_state processQueue ();


  public:
	  /** get the info structure for the federate
	  */
	  CoreFederateInfo getInfo() const;
	/** update the info structure as an atomic operation
	*/
    void UpdateFederateInfo (CoreFederateInfo &newInfo);

	/** get the granted time of a federate*/
    Time grantedTime () const { return time_granted; }
	/**get a reference to the handles of subscriptions with value updates
	*/
    const std::vector<Core::Handle> &getEvents () const { return events; }
	/** get a reference to the global ids of dependent federates
	*/
    const std::vector<Core::federate_id_t> &getDependents () const { return dependents; }
	/** compute all the known dependencies
	*/
    void generateKnownDependencies ();
    void addDependency (Core::federate_id_t fedToDependOn);
    void addDependent (Core::federate_id_t fedThatDependsOnThis);
	/** specify the core object that manages this federate*/
    void setCoreObject (CommonCore *parent);
	//the next 5 functions are the processing functions that actually process the queue
	/** process until the federate has verified its membership and assigned a global id number*/
	convergence_state waitSetup();
	/** process until the init state has been entered or there is a failure*/
	convergence_state enterInitState();
    /** function to call when enterering execution state
    returns either converged or nonconverged depening on whether an iteration is needed
    */
    convergence_state enterExecutingState ();

    iterationTime requestTime (Time nextTime, convergence_state converged);
	/** function to process the queue in a generic fashion used to just process messages
	with no specific end in mind
	*/
	convergence_state genericUnspecifiedQueueProcess();
    void addAction (const ActionMessage &action);

    void setLogger (std::function<void(int, const std::string &, const std::string &)> logFunction)
    {
        loggerFunction = std::move (logFunction);
    }
};
}
#endif