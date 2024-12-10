/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../common/GuardedTypes.hpp"
#include "ActionMessage.hpp"
#include "BasicHandleInfo.hpp"
#include "CoreTypes.hpp"
#include "InterfaceInfo.hpp"
#include "core-data.hpp"
#include "gmlc/containers/BlockingQueue.hpp"
#include "helicsTime.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <deque>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <tuple>
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
class LogManager;

constexpr Time startupTime = Time::minVal();
constexpr Time initialTime{-1000000.0};

/// @brief enumeration of possible time coordination methods
enum class TimeSynchronizationMethod : uint8_t { DISTRIBUTED = 0, GLOBAL = 1, ASYNC = 2 };

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
    /// object that manages the time to determine granting
    std::unique_ptr<TimeCoordinator> timeCoord;

  public:
    LocalFederateId local_id;  //!< id code for the local federate descriptor
    std::atomic<GlobalFederateId> global_id;  //!< global id code, default to invalid
  private:
    //!< the current state of the federate
    std::atomic<FederateStates> state{FederateStates::CREATED};
    bool only_transmit_on_change{false};  //!< flag indicating that values should only be
                                          //!< transmitted if different than previous values
    bool realtime{false};  //!< flag indicating that the federate runs in real time
    bool observer{false};  //!< flag indicating the federate is an observer only
    bool reentrant{false};  //!< flag indicating the federate can be reentrant
    bool mSourceOnly{false};  //!< flag indicating the federate is a source_only
    bool mCallbackBased{false};  //!< flag indicating the federate is a callback federate
    /// flag indicating that inputs should have strict type checking
    bool strict_input_type_checking{false};
    bool ignore_unit_mismatch{false};  //!< flag to ignore mismatching units
    /// flag indicating that a federate is likely to be slow in responding
    bool mSlowResponding{false};
    /// @brief flag indicating that the federate is open to remote control
    bool mAllowRemoteControl{true};
    InterfaceInfo interfaceInformation;  //!< the container for the interface information objects
    std::unique_ptr<LogManager> mLogManager;
    int maxLogLevel{HELICS_LOG_LEVEL_NO_PRINT};

  public:
    std::atomic<bool> init_transmitted{false};  //!< the initialization request has been transmitted
    /// storage for index group location (this only matters on construction so can be public)
    int indexGroup{0};

  private:
    /// flag indicating that the federate should delay for the current time
    bool wait_for_current_time{false};
    /// flag indicating that time mismatches should be ignored
    bool ignore_time_mismatch_warnings{false};
    /// flag indicating that the profiler code should be activated
    bool mProfilerActive{false};
    /// flag indicating that the profiling should be captured in the federate log instead of
    /// forwarded
    bool mLocalProfileCapture{false};
    int errorCode{0};  //!< storage for an error code
    CommonCore* mParent{nullptr};  //!< pointer to the higher level;
    std::string errorString;  //!< storage for an error string populated on an error
    /** time the initialization mode started for real time capture */
    decltype(std::chrono::steady_clock::now()) start_clock_time;
    Time rt_lag{timeZero};  //!< max lag for the rt control
    Time rt_lead{timeZero};  //!< min lag for the realtime control
    Time grantTimeOutPeriod{timeZero};  //!< period to raise an inquiry about lack of grant
    std::int32_t realTimeTimerIndex{-1};  //!< the timer index for the real time timer;
    std::int32_t grantTimeoutTimeIndex{-1};  //!< time index for the grant time out timer;
  public:
    /** atomic flag indicating this federate has requested entry to initialization */
    std::atomic<bool> initRequested{false};
    // temporary
    std::atomic<bool> requestingMode{false};

    std::atomic<bool> initIterating{false};

  private:
    bool iterating{false};  //!< the federate is iterating at a time step
    bool timeGranted_mode{false};  //!< indicator if the federate is in a granted state or a
                                   //!< requesting state waiting to grant
    bool terminate_on_error{false};  //!< indicator that if the federate encounters a configuration
                                     //!< error it should cause a co-simulation abort
    IterationRequest lastIterationRequest{IterationRequest::NO_ITERATIONS};
    /// the time keeping method in use
    TimeSynchronizationMethod timeMethod{TimeSynchronizationMethod::DISTRIBUTED};
    /** counter for the number of times time or execution mode has been granted */
    std::uint32_t mGrantCount{0};  // this is intended to allow wrapping
    /** message timer object for real time operations and timeouts */
    std::shared_ptr<MessageTimer> mTimer;
    /** processing queue for messages incoming to a federate */
    gmlc::containers::BlockingQueue<ActionMessage> queue;
    /** processing queue for commands incoming to a federate */
    gmlc::containers::BlockingQueue<std::pair<std::string, std::string>> commandQueue;
    /** current defaults for operational flags of interfaces for this federate */
    std::atomic<uint16_t> interfaceFlags{0};
    /** queue for delaying processing of messages for a time */
    std::map<GlobalFederateId, std::deque<ActionMessage>> delayQueues;
    std::vector<InterfaceHandle> events;  //!< list of value events to process
    std::vector<InterfaceHandle> eventMessages;  //!< list of endpoints with messages to process
    std::vector<GlobalFederateId> delayedFederates;  //!< list of federates to delay messages from
    Time time_granted{startupTime};  //!< the most recent granted time;
    Time allowed_send_time{startupTime};  //!< the next time a message can be sent;
    Time minimumReceiveTime{startupTime};  //!< minimum receive time for messages

#if __cplusplus >= 201703L
    mutable std::atomic_flag processing{};  //!< the federate is processing
#else
    mutable std::atomic_flag processing = ATOMIC_FLAG_INIT;  //!< the federate is processing
#endif

    /** a callback for additional queries */
    std::vector<std::function<std::string(std::string_view)>> queryCallbacks;
    std::shared_ptr<FederateOperator> fedCallbacks;  //!< storage for a callback federate
    std::vector<std::pair<std::string, std::string>> tags;  //!< storage for user defined tags
    std::atomic<bool> queueProcessing{false};
    /** find the next Value Event*/
    Time nextValueTime() const;
    /** find the next Message Event*/
    Time nextMessageTime() const;

    /** update the federate state */
    void setState(FederateStates newState);

    /** check if a message should be delayed*/
    bool messageShouldBeDelayed(const ActionMessage& cmd) const noexcept;
    /** add a federate to the delayed list*/
    void addFederateToDelay(GlobalFederateId gid);
    /** generate a component of json config string*/
    void generateConfig(nlohmann::json& base) const;

  public:
    /** reset the federate to created state*/
    void reset(const CoreFederateInfo& fedInfo);
    /** get the name of the federate*/
    const std::string& getIdentifier() const { return name; }
    /** get the current state of the federate*/
    FederateStates getState() const;
    /** get the information that comes from the interface including timing information*/
    InterfaceInfo& interfaces() { return interfaceInformation; }
    /** const version of the interface info retrieval function*/
    const InterfaceInfo& interfaces() const { return interfaceInformation; }

    /** get the size of a message queue for a specific endpoint or filter handle*/
    uint64_t getQueueSize(InterfaceHandle hid) const;
    /** get the sum of all message queue sizes i.e. the total number of messages available in all
     * endpoints*/
    uint64_t getQueueSize() const;
    /** get the current iteration counter for an iterative call
    @details this will work properly even when a federate is processing
    */
    int32_t getCurrentIteration() const;
    /** get the next available message for an endpoint
    @param hid the handle of an endpoint or filter
    @return a pointer to a message -the ownership of the message is transferred to the caller*/
    std::unique_ptr<Message> receive(InterfaceHandle hid);
    /** get any message ready for reception
    @param[out] hid the endpoint related to the message*/
    std::unique_ptr<Message> receiveAny(InterfaceHandle& hid);
    /**
     * Return the data for the specified handle or the latest input
     */
    const std::shared_ptr<const SmallBuffer>& getValue(InterfaceHandle handle,
                                                       uint32_t* inputIndex);

    /**
     * Return all the available data for the specified handle or the latest input
     *
     */
    const std::vector<std::shared_ptr<const SmallBuffer>>& getAllValues(InterfaceHandle handle);

    /** getPublishedValue */
    std::pair<SmallBuffer, Time> getPublishedValue(InterfaceHandle handle);
    /** set the CommonCore object that is managing this Federate*/
    void setParent(CommonCore* coreObject) { mParent = coreObject; }
    /** update the info structure
   @details public call so it also calls the federate lock before calling private update function
   the action Message should be CMD_FED_CONFIGURE
   */
    void setProperties(const ActionMessage& cmd);
    /** set a property on a specific interface*/
    void setInterfaceProperty(const ActionMessage& cmd);
    /** set a timeProperty on the federate*/
    void setProperty(int timeProperty, Time propertyVal);
    /** set an integral property on the federate*/
    void setProperty(int intProperty, int propertyVal);
    /** set an option Flag on the federate*/
    void setOptionFlag(int optionFlag, bool value);
    /** get a time Property*/
    Time getTimeProperty(int timeProperty) const;
    /** get an option flag value*/
    bool getOptionFlag(int optionFlag) const;
    /** get the currently active option for a handle*/
    int32_t getHandleOption(InterfaceHandle handle, char iType, int32_t option) const;
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
    int loggingLevel() const;

    /** set a tag (key-value pair)*/
    void setTag(std::string_view tag, std::string_view value);
    /** search for a tag by name*/
    const std::string& getTag(std::string_view tag) const;
    /** get a tag (key-value pair) by index*/
    const std::pair<std::string, std::string>& getTagByIndex(size_t index) const
    {
        return tags[index];
    }
    /** get the number of tags associated with an interface*/
    auto tagCount() const { return tags.size(); }
    /** return true if the federate is callback based*/
    bool isCallbackFederate() const { return mCallbackBased; }

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
    MessageProcessingResult processQueue() noexcept;

    /** process the federate delayed Message queue until a returnable event or it is empty
    @details processQueue will process messages until one of 3 things occur
    1.  the initialization state has been entered
    2.  the execution state has been granted (or initialization state reentered from a iterative
    request)
    3.  time has been granted
    4. a break event is encountered
    @return a convergence state value with an indicator of return reason and state of convergence
    */
    MessageProcessingResult processDelayQueue() noexcept;

    std::optional<MessageProcessingResult>
        checkProcResult(std::tuple<FederateStates, MessageProcessingResult, bool>& proc_result,
                        ActionMessage& cmd);
    /** process a single message
    @return a convergence state value with an indicator of return reason and state of convergence
    */
    MessageProcessingResult processActionMessage(ActionMessage& cmd);

    /** process a data connection management message
     */
    void processDataConnectionMessage(ActionMessage& cmd);

    /** process a message containing data
     */
    void processDataMessage(ActionMessage& cmd);
    /** run a timeout check*/
    void timeoutCheck(ActionMessage& cmd);
    /** process a logging message*/
    void processLoggingMessage(ActionMessage& cmd);
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
    void addDependency(GlobalFederateId fedToDependOn);
    /** add a dependent federate*/
    void addDependent(GlobalFederateId fedThatDependsOnThis);
    /** for dynamic reentrant federates reset the connection*/
    void resetDependency(GlobalFederateId gid);

    /** check the interfaces for any issues*/
    int checkInterfaces();
    /** generate results from a query*/
    std::string processQueryActual(std::string_view query) const;
    /** generate a federate profiling message
    @param enterHelicsCode set to true when entering HELICS code section,
    false when exiting*/
    void generateProfilingMessage(bool enterHelicsCode);
    /** generate a timing marker message system time + steady time*/
    void generateProfilingMarker();
    /** go through and update the max log level*/
    void updateMaxLogLevel();

    /** run the processing but don't block assuming a callback based federate*/
    void callbackProcessing() noexcept;
    void callbackReturnResult(FederateStates lastState,
                              MessageProcessingResult result,
                              FederateStates newState) noexcept;
    void initCallbackProcessing();
    void execCallbackProcessing(IterationResult result);
    /** update the data and time after being granted Exec entry*/
    void updateDataForExecEntry(MessageProcessingResult result, IterationRequest iterate);
    /** update the data and time after being granted time request*/
    void updateDataForTimeReturn(MessageProcessingResult result,
                                 Time nextTime,
                                 IterationRequest iterate);

  public:
    /** get the granted time of a federate*/
    Time grantedTime() const { return time_granted; }
    /** get allowable message time*/
    Time nextAllowedSendTime() const { return allowed_send_time; }
    /**get a reference to the handles of subscriptions with value updates
     */
    const std::vector<InterfaceHandle>& getEvents() const;
    /** get a vector of the federates this one depends on
     */
    std::vector<GlobalFederateId> getDependencies() const;
    /** get a vector to the global ids of dependent federates
     */
    std::vector<GlobalFederateId> getDependents() const;
    /** get the last error string */
    const std::string& lastErrorString() const { return errorString; }
    /** get the last error code*/
    int lastErrorCode() const noexcept { return errorCode; }
    /** set the managing core object */
    void setCoreObject(CommonCore* parent);
    // the next 5 functions are the processing functions that actually process the queue
    /** process until the federate has verified its membership and assigned a global id number*/
    IterationResult waitSetup();
    /** process until the initialization state has been entered, an iteration request granted or
    there is a failure
    @param request the desired iteration condition
    @return the result of the iteration request*/
    IterationResult enterInitializingMode(IterationRequest request);
    /** function to call when entering execution state
    @param iterate indicator of whether the fed should iterate if need be or not
    @param sendRequest generates the local actionMessage inside the function leaving to false
    assumes the caller generated the message returns either converged or nonconverged depending on
    whether an iteration is needed
    @return an iteration time with two elements the granted time and the iteration result. The time
    will usually be 0 unless the federate is joining dynamically
    */
    iteration_time enterExecutingMode(IterationRequest iterate, bool sendRequest = false);
    /** request a time advancement
    @param nextTime the time of the requested advancement
    @param iterate the type of iteration requested
    @param sendRequest generates the local actionMessage inside the function leaving to false
    assumes the caller generated the message
    @return an iteration time with two elements the granted time and the iteration result
    */
    iteration_time requestTime(Time nextTime, IterationRequest iterate, bool sendRequest = false);
    /** get a list of current subscribers to a publication
    @param handle the publication handle to use
    */
    std::vector<GlobalHandle> getSubscribers(InterfaceHandle handle);

    /** get a list of the endpoints a message should be sent to
    @param handle the endpoint handle to use
    */
    std::vector<std::pair<GlobalHandle, std::string_view>>
        getMessageDestinations(InterfaceHandle handle);

    /** function to process the queue in a generic fashion used to just process messages
    with no specific end in mind
    @param busyReturn if set to true will return if the federate is already processing
    */
    MessageProcessingResult genericUnspecifiedQueueProcess(bool busyReturn);
    /** function to process the queue until a disconnect_fed_ack is received*/
    void finalize();
    /** process incoming messages for a certain amount of time*/
    void processCommunications(std::chrono::milliseconds period);
    /** add an action message to the queue*/
    void addAction(const ActionMessage& action);
    /** move a message to the queue*/
    void addAction(ActionMessage&& action);
    /** sometime a message comes in after a federate has terminated and may require a response*/
    std::optional<ActionMessage> processPostTerminationAction(const ActionMessage& action);

    /** force processing of a specific message out of order*/
    void forceProcessMessage(ActionMessage& action);

    /** log a message to the federate Logger
    @param level the logging level of the message
    @param logMessageSource the name of the object that sent the message
    @param message the message to log
    @param fromRemote indicator that the message is from a remote source and should be treated
    accordingly
    */
    void logMessage(int level,
                    std::string_view logMessageSource,
                    std::string_view message,
                    bool fromRemote = false) const;

    /** set the logging function
    @details function must have signature void(int level, const std::string &sourceName, const
    std::string &message)
    */
    void setLogger(std::function<void(int, std::string_view, std::string_view)> logFunction);

    /** set the federate callback operator
     */
    void setCallbackOperator(std::shared_ptr<FederateOperator> fed)
    {
        fedCallbacks = std::move(fed);
    }

    /** set the query callback function
    @details function must have signature std::string(const std::string &query)
    */
    void setQueryCallback(std::function<std::string(std::string_view)> queryCallbackFunction,
                          int order)
    {
        order = std::clamp(order, 1, 10);

        if (static_cast<int>(queryCallbacks.size()) < order) {
            queryCallbacks.resize(order);
        }
        queryCallbacks[order - 1] = std::move(queryCallbackFunction);
    }
    /** generate the result of a query string
    @param query a query string
    @param force_ordering true if the query should be processed in a force_ordering way
    @return the resulting string from the query or "#wait" if the federate is not available to
    answer immediately*/
    std::string processQuery(std::string_view query, bool force_ordering = false) const;
    /** check if a value should be published or not and if needed archive it as a changed value for
    future change detection
    @param pub_id the handle of the publication
    @param data the raw data to check
    @param len the length of the data
    @return true if it should be published, false if not
    */
    bool checkAndSetValue(InterfaceHandle pub_id, const char* data, uint64_t len);

    /** route a message either forward to parent or add to queue*/
    void routeMessage(const ActionMessage& msg);

    /** move a message either to parent or add to queue*/
    void routeMessage(ActionMessage&& msg);
    /** create an interface*/
    void createInterface(InterfaceType htype,
                         InterfaceHandle handle,
                         std::string_view key,
                         std::string_view type,
                         std::string_view units,
                         uint16_t flags);
    /** close an interface*/
    void closeInterface(InterfaceHandle handle, InterfaceType type);
    /** send a command to a federate*/
    void sendCommand(ActionMessage& command);

    /** get a command for a federate from its queue*/
    std::pair<std::string, std::string> getCommand();
    /** wait for a command to a federate*/
    std::pair<std::string, std::string> waitCommand();
};

}  // namespace helics
