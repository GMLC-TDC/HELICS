/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../common/GuardedTypes.hpp"
#include "ActionMessage.hpp"
#include "BasicHandleInfo.hpp"
#include "InterfaceInfo.hpp"
#include "core-data.hpp"
#include "core-types.hpp"
#include "gmlc/containers/BlockingQueue.hpp"
#include "helics-time.hpp"

#include <atomic>
#include <chrono>
#include <deque>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace helics {
class SubscriptionInfo;
class PublicationInfo;
class EndpointInfo;
class FilterInfo;
class CommonCore;
class CoreFederateInfo;

class TimeCoordinator;
class MessageTimer;

constexpr Time startupTime = Time::minVal();
constexpr Time initialTime{-1000000.0};
/** class managing the information about a single federate*/
class FederateState {
  public:
    /** constructor from name and information structure*/
    FederateState(const std::string& fedName, const CoreFederateInfo& fedInfo);
    // the destructor is defined so some classes linked with unique ptrs don't have to be defined in
    // the header
    /** DISABLE_COPY_AND_ASSIGN */
    FederateState(const FederateState&) = delete;
    FederateState& operator=(const FederateState&) = delete;
    /** destructor*/
    ~FederateState();

  private:
    const std::string name;  //!< the name of the federate
    std::unique_ptr<TimeCoordinator>
        timeCoord;  //!< object that manages the time to determine granting
  public:
    local_federate_id local_id;  //!< id code for the local federate descriptor
    std::atomic<global_federate_id> global_id;  //!< global id code, default to invalid

  private:
    std::atomic<federate_state> state{HELICS_CREATED};  //!< the current state of the federate
    bool only_transmit_on_change{false};  //!< flag indicating that values should only be
                                          //!< transmitted if different than previous values
    bool realtime{false};  //!< flag indicating that the federate runs in real time
    bool observer{false};  //!< flag indicating the federate is an observer only
    bool source_only{false};  //!< flag indicating the federate is a source_only
    bool ignore_time_mismatch_warnings{
        false};  //!< flag indicating that time mismatches should be ignored
    bool strict_input_type_checking{
        false};  //!< flag indicating that inputs should have strict type checking
    bool ignore_unit_mismatch{false};  //!< flag to ignore mismatching units
    bool slow_responding{
        false};  //!< flag indicating that a federate is likely to be slow in responding
    InterfaceInfo interfaceInformation;  //!< the container for the interface information objects

  public:
    std::atomic<bool> init_transmitted{false};  //!< the initialization request has been transmitted
  private:
    bool wait_for_current_time{
        false};  //!< flag indicating that the federate should delay for the current time
    int errorCode{0};  //!< storage for an error code
    CommonCore* parent_{nullptr};  //!< pointer to the higher level;
    std::string errorString;  //!< storage for an error string populated on an error
    decltype(std::chrono::steady_clock::now())
        start_clock_time;  //!< time the initialization mode started for real time capture
    Time rt_lag{timeZero};  //!< max lag for the rt control
    Time rt_lead{timeZero};  //!< min lag for the realtime control
    int32_t realTimeTimerIndex{-1};  //!< the timer index for the real time timer;
  public:
    std::atomic<bool> init_requested{
        false};  //!< this federate has requested entry to initialization
    // temporary
    std::atomic<bool> requestingMode{false};

  private:
    bool iterating{false};  //!< the federate is iterating at a time step
    bool timeGranted_mode{false};  //!< indicator if the federate is in a granted state or a
                                   //!< requesting state waiting to grant
    bool terminate_on_error{false};  //!< indicator that if the federate encounters a configuration
                                     //!< error it should cause a co-simulation abort
    int logLevel{1};  //!< the level of logging used in the federate

    //   std::vector<ActionMessage> messLog;
  private:
    std::shared_ptr<MessageTimer>
        mTimer;  //!< message timer object for real time operations and timeouts
    gmlc::containers::BlockingQueue<ActionMessage>
        queue;  //!< processing queue for messages incoming to a federate
    std::atomic<uint16_t> interfaceFlags{
        0};  //!< current defaults for operational flags of interfaces for this federate
    std::map<global_federate_id, std::deque<ActionMessage>>
        delayQueues;  //!< queue for delaying processing of messages for a time
    std::vector<interface_handle> events;  //!< list of value events to process
    std::vector<interface_handle> eventMessages;  //!< list of endpoints with messages to process
    std::vector<global_federate_id> delayedFederates;  //!< list of federates to delay messages from
    Time time_granted{startupTime};  //!< the most recent granted time;
    Time allowed_send_time{startupTime};  //!< the next time a message can be sent;
    mutable std::atomic_flag processing = ATOMIC_FLAG_INIT;  //!< the federate is processing
  private:
    /** a logging function for logging or printing messages*/
    std::function<void(int, const std::string&, const std::string&)>
        loggerFunction;  //!< callback for logging functions
    std::function<std::string(const std::string&)>
        queryCallback;  //!< a callback for additional queries
    /** find the next Value Event*/
    Time nextValueTime() const;
    /** find the next Message Event*/
    Time nextMessageTime() const;

    /** update the federate state */
    void setState(federate_state newState);

    /** check if a message should be delayed*/
    bool messageShouldBeDelayed(const ActionMessage& cmd) const;
    /** add a federate to the delayed list*/
    void addFederateToDelay(global_federate_id id);
    /** generate a component of json config string*/
    void generateConfig(Json::Value& base) const;

  public:
    /** reset the federate to created state*/
    void reset();
    /** reset the federate to the initializing state*/
    void reInit();
    /** get the name of the federate*/
    const std::string& getIdentifier() const { return name; }
    /** get the current state of the federate*/
    federate_state getState() const;
    /** get the information that comes from the interface including timing information*/
    InterfaceInfo& interfaces() { return interfaceInformation; }
    /** const version of the interface info retrieval function*/
    const InterfaceInfo& interfaces() const { return interfaceInformation; }

    /** get the size of a message queue for a specific endpoint or filter handle*/
    uint64_t getQueueSize(interface_handle id) const;
    /** get the sum of all message queue sizes i.e. the total number of messages available in all
     * endpoints*/
    uint64_t getQueueSize() const;
    /** get the current iteration counter for an iterative call
    @details this will work properly even when a federate is processing
    */
    int32_t getCurrentIteration() const;
    /** get the next available message for an endpoint
    @param id the handle of an endpoint or filter
    @return a pointer to a message -the ownership of the message is transferred to the caller*/
    std::unique_ptr<Message> receive(interface_handle id);
    /** get any message ready for reception
    @param[out] id the endpoint related to the message*/
    std::unique_ptr<Message> receiveAny(interface_handle& id);
    /**
     * Return the data for the specified handle or the latest input
     */
    const std::shared_ptr<const data_block>& getValue(interface_handle handle,
                                                      uint32_t* inputIndex);

    /**
     * Return all the available data for the specified handle or the latest input
     *
     */
    const std::vector<std::shared_ptr<const data_block>>& getAllValues(interface_handle handle);

    /** set the CommonCore object that is managing this Federate*/
    void setParent(CommonCore* coreObject) { parent_ = coreObject; }
    /** update the info structure
   @details public call so it also calls the federate lock before calling private update function
   the action Message should be CMD_FED_CONFIGURE
   */
    void setProperties(const ActionMessage& cmd);
    /** set a property on a specific interface*/
    void setInterfaceProperty(const ActionMessage& cmd);
    /** set a timeProperty for a the coordinator*/
    void setProperty(int timeProperty, Time propertyVal);
    /** set a timeProperty for a the coordinator*/
    void setProperty(int intProperty, int propertyVal);
    /** set an option Flag for a the coordinator*/
    void setOptionFlag(int optionFlag, bool value);
    /** get a time Property*/
    Time getTimeProperty(int timeProperty) const;
    /** get an option flag value*/
    bool getOptionFlag(int optionFlag) const;
    /** get the currently active option for a handle*/
    int32_t getHandleOption(interface_handle handle, char iType, int32_t option) const;
    /** get the currently active interface flags*/
    uint16_t getInterfaceFlags() const { return interfaceFlags.load(); }
    /** get an option flag value*/
    int getIntegerProperty(int intProperty) const;
    /** get the number of publications*/
    int publicationCount() const;
    /** get the number of endpoints*/
    int endpointCount() const;
    /** get the number of inputs*/
    int inputCount() const;
    /** locks the processing with a busy loop*/
    void spinlock() const
    {
        while (processing.test_and_set()) {
            ;  // spin
        }
    }
    /** locks the processing with a sleep loop*/
    void sleeplock() const
    {
        if (!processing.test_and_set()) {
            return;
        }
        // spin for 10000 tries
        for (int ii = 0; ii < 10000; ++ii) {
            if (!processing.test_and_set()) {
                return;
            }
        }
        while (processing.test_and_set()) {
            std::this_thread::yield();
        }
    }
    /** locks the processing so FederateState can be used with lock_guard*/
    void lock() { sleeplock(); }

    /** tries to lock the processing return true if successful and false if not*/
    bool try_lock() const { return !processing.test_and_set(); }
    /** unlocks the processing*/
    void unlock() const { processing.clear(); }
    /** get the current logging level*/
    int loggingLevel() const { return logLevel; }

  private:
    /** process the federate queue until returnable event
    @details processQueue will process messages until one of 3 things occur
    1.  the initialization state has been entered
    2.  the execution state has been granted (or initialization state reentered from a iterative
    request)
    3.  time has been granted
    4. a break event is encountered
    @return a convergence state value with an indicator of return reason and state of convergence
    */
    message_processing_result processQueue() noexcept;

    /** process the federate delayed Message queue until a returnable event or it is empty
    @details processQueue will process messages until one of 3 things occur
    1.  the initialization state has been entered
    2.  the execution state has been granted (or initialization state reentered from a iterative
    request)
    3.  time has been granted
    4. a break event is encountered
    @return a convergence state value with an indicator of return reason and state of convergence
    */
    message_processing_result processDelayQueue() noexcept;
    /** process a single message
    @return a convergence state value with an indicator of return reason and state of convergence
    */
    message_processing_result processActionMessage(ActionMessage& cmd);
    /** fill event list
    @param currentTime the time of the update
    */
    void fillEventVectorUpTo(Time currentTime);
    /** fill event list
    @param currentTime the time of the update
    */
    void fillEventVectorInclusive(Time currentTime);
    /** fill event list
    @param currentTime the time of the update
    */
    void fillEventVectorNextIteration(Time currentTime);
    /** add a dependency to the timing coordination*/
    void addDependency(global_federate_id fedToDependOn);
    /** add a dependent federate*/
    void addDependent(global_federate_id fedThatDependsOnThis);
    /** check the interfaces for any issues*/
    int checkInterfaces();
    /** generate results from a query*/
    std::string processQueryActual(const std::string& query) const;

  public:
    /** get the granted time of a federate*/
    Time grantedTime() const { return time_granted; }
    /** get allowable message time*/
    Time nextAllowedSendTime() const { return allowed_send_time; }
    /**get a reference to the handles of subscriptions with value updates
     */
    const std::vector<interface_handle>& getEvents() const;
    /** get a vector of the federates this one depends on
     */
    std::vector<global_federate_id> getDependencies() const;
    /** get a vector to the global ids of dependent federates
     */
    std::vector<global_federate_id> getDependents() const;
    /** get the last error string */
    const std::string& lastErrorString() const { return errorString; }
    /** get the last error code*/
    int lastErrorCode() const noexcept { return errorCode; }
    /** set the managing core object */
    void setCoreObject(CommonCore* parent);
    // the next 5 functions are the processing functions that actually process the queue
    /** process until the federate has verified its membership and assigned a global id number*/
    iteration_result waitSetup();
    /** process until the initialization state has been entered or there is a failure*/
    iteration_result enterInitializingMode();
    /** function to call when entering execution state
    @param iterate indicator of whether the fed should iterate if need be or not
    @param sendRequest generates the local actionMessage inside the function leaving to false
    assumes the caller generated the message returns either converged or nonconverged depending on
    whether an iteration is needed
    */
    iteration_result enterExecutingMode(iteration_request iterate, bool sendRequest = false);
    /** request a time advancement
    @param nextTime the time of the requested advancement
    @param iterate the type of iteration requested
    @param sendRequest generates the local actionMessage inside the function leaving to false
    assumes the caller generated the message
    @return an iteration time with two elements the granted time and the convergence state
    */
    iteration_time requestTime(Time nextTime, iteration_request iterate, bool sendRequest = false);
    /** get a list of current subscribers to a publication
    @param handle the publication handle to use
    */
    std::vector<global_handle> getSubscribers(interface_handle handle);

    /** function to process the queue in a generic fashion used to just process messages
    with no specific end in mind
    */
    iteration_result genericUnspecifiedQueueProcess();
    /** function to process the queue until a disconnect_fed_ack is received*/
    void finalize();

    /** add an action message to the queue*/
    void addAction(const ActionMessage& action);
    /** move a message to the queue*/
    void addAction(ActionMessage&& action);
    /** sometime a message comes in after a federate has terminated and may require a response*/
    opt<ActionMessage> processPostTerminationAction(const ActionMessage& action);
    /** log a message to the federate Logger
    @param level the logging level of the message
    @param logMessageSource the name of the object that sent the message
    @param message the message to log
    */
    void logMessage(int level,
                    const std::string& logMessageSource,
                    const std::string& message) const;

    /** set the logging function
    @details function must have signature void(int level, const std::string &sourceName, const
    std::string &message)
    */
    void setLogger(std::function<void(int, const std::string&, const std::string&)> logFunction)
    {
        loggerFunction = std::move(logFunction);
    }
    /** set the query callback function
    @details function must have signature std::string(const std::string &query)
    */
    void setQueryCallback(std::function<std::string(const std::string&)> queryCallbackFunction)
    {
        queryCallback = std::move(queryCallbackFunction);
    }
    /** generate the result of a query string
    @param query a query string
    @param force_ordering true if the query should be processed in a force_ordering way
    @return the resulting string from the query or "#wait" if the federate is not available to
    answer immediately*/
    std::string processQuery(const std::string& query, bool force_ordering = false) const;
    /** check if a value should be published or not and if needed archive it as a changed value for
    future change detection
    @param pub_id the handle of the publication
    @param data the raw data to check
    @param len the length of the data
    @return true if it should be published, false if not
    */
    bool checkAndSetValue(interface_handle pub_id, const char* data, uint64_t len);

    /** route a message either forward to parent or add to queue*/
    void routeMessage(const ActionMessage& msg);
    /** create an interface*/
    void createInterface(handle_type htype,
                         interface_handle handle,
                         const std::string& key,
                         const std::string& type,
                         const std::string& units);
    /** close an interface*/
    void closeInterface(interface_handle handle, handle_type type);
};

}  // namespace helics
