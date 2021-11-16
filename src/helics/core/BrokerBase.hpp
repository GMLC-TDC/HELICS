/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
/**
@file
virtual base class for object that function like a broker includes common parameters
and some common methods used cores and brokers
*/

#include "ActionMessage.hpp"
#include "FederateIdExtra.hpp"
#include "gmlc/containers/BlockingPriorityQueue.hpp"

#include <atomic>
#include <limits>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace spdlog {
class logger;
}
namespace helics {
class ForwardingTimeCoordinator;
class helicsCLI11App;
class ProfilerBuffer;
/** base class for broker like objects
 */
class BrokerBase {
  protected:
    static constexpr double mInvalidSimulationTime{-98763.2};
    /** the unique identifier for the broker(core or broker) */
    std::atomic<GlobalBrokerId> global_id{parent_broker_id};
    GlobalBrokerId global_broker_id_local{};  //!< meant to be the same as global_id but not
                                              //!< atomically protected
    GlobalBrokerId higher_broker_id{0};  //!< the id code of the broker 1 level about this broker
    /**the logging level to use,  levels >= this will be ignored*/
    std::atomic<int32_t> maxLogLevel{HELICS_LOG_LEVEL_WARNING};
    int32_t consoleLogLevel{HELICS_LOG_LEVEL_WARNING};  //!< the logging level for console display
    int32_t fileLogLevel{HELICS_LOG_LEVEL_WARNING};  //!< the logging level for logging to a file
    /** the minimum number of federates that must connect before entering init mode */
    int32_t minFederateCount{1};
    /** the minimum number of brokers that must connect before entering init mode */
    int32_t minBrokerCount{0};
    int32_t maxFederateCount{(std::numeric_limits<int32_t>::max)()};
    int32_t maxBrokerCount{(std::numeric_limits<int32_t>::max)()};
    /** the minimum number of children that must connect before entering init mode */
    int32_t minChildCount{0};
    int32_t maxIterationCount{10000};  //!< the maximum number of iterative loops that are allowed
    Time tickTimer{5.0};  //!< the length of each heartbeat tick
    Time timeout{30.0};  //!< timeout to wait to establish a broker connection before giving up
    Time networkTimeout{-1.0};  //!< timeout to establish a socket connection before giving up
    Time queryTimeout{15.0};  //!< timeout for queries, if the query isn't answered within this time
                              //!< period respond with timeout error
    Time errorDelay{10.0};  //!< time to delay before terminating after error state
    Time grantTimeout{-1.0}; //!< timeout for triggering diagnostic action waiting for a time grant
    std::string identifier;  //!< an identifier for the broker
    std::string brokerKey;  //!< a key that all joining federates must have to connect if empty no
                            //!< key is required
    // address is mutable since during initial phases it may not be fixed so to maintain a
    // consistent public interface for extracting it this variable may need to be updated in a
    // constant function
    mutable std::string address;  //!< network location of the broker
    std::shared_ptr<spdlog::logger>
        consoleLogger;  //!< default logging object to use if the logging callback is not specified
    std::shared_ptr<spdlog::logger>
        fileLogger;  //!< default logging object to use if the logging callback is not specified
    std::thread queueProcessingThread;  //!< thread for running the broker
    /** a logging function for logging or printing messages*/
    std::function<void(int, std::string_view, std::string_view)> loggerFunction;
    /// flag indicating that no further message should be processed
    std::atomic<bool> haltOperations{false};
    /// flag indicating the broker should use a conservative time policy
    bool restrictive_time_policy{false};
    /// flag indicating that the federation should halt on any error
    bool terminate_on_error{false};
    /// flag indicating operation in a user debugging mode
    bool debugging{false};
    /// flag indicating that the broker is an observer only
    bool observer{false};

  private:
    std::atomic<bool> mainLoopIsRunning{
        false};  //!< flag indicating that the main processing loop is running
    bool dumplog{false};  //!< flag indicating the broker should capture a dump log
    std::atomic<bool> forceLoggingFlush{false};  //!< force the log to flush after every message
    bool queueDisabled{
        false};  //!< flag indicating that the message queue should not be used and all functions
    //!< called directly instead of distinct thread
    bool disable_timer{false};  //!< turn off the timer/timeout subsystem completely
    /// counter for the total number of message processed
    std::atomic<std::size_t> messageCounter{0};

  protected:
    std::string logFile;  //!< the file to log message to
    std::unique_ptr<ForwardingTimeCoordinator> timeCoord;  //!< object managing the time control
    gmlc::containers::BlockingPriorityQueue<ActionMessage> actionQueue;  //!< primary routing queue
    // time coordinator for managing filters
    // std::unique_ptr<TimeCoordinator> filterTimeCoord;
    // global_federate_id filterFedID;
    /** enumeration of the possible core states*/
    enum class BrokerState : int16_t {
        created = -6,  //!< the broker has been created
        configuring = -5,  //!< the broker is in the processing of configuring
        configured = -4,  //!< the broker itself has been configured and is ready to connect
        connecting = -3,  //!< the connection process has started
        connected = -2,  //!< the connection process has completed
        initializing = -1,  //!< the enter initialization process has started
        operating = 0,  //!< normal operating conditions
        terminating = 1,  //!< the termination process has started
        terminated = 3,  //!< the termination process has started
        errored = 7,  //!< an error was encountered
    };

    enum class TickForwardingReasons : uint32_t {
        NONE = 0,
        NO_COMMS = 0x01,
        PING_RESPONSE = 0x02,
        QUERY_TIMEOUT = 0x04
    };
    bool noAutomaticID{false};  //!< the broker should not automatically generate an ID
    bool hasTimeDependency{false};  //!< set to true if the broker has Time dependencies
    bool enteredExecutionMode{
        false};  //!< flag indicating that the broker has entered execution mode
    bool waitingForBrokerPingReply{false};  //!< flag indicating we are waiting for a ping reply
    bool hasFilters{false};  //!< flag indicating filters come through the broker

    bool no_ping{false};  //!< indicator that the broker is not very responsive to ping requests
    bool uuid_like{false};  //!< will be set to true if the name looks like a uuid
    /** specify that outgoing connection should use json serialization */
    bool useJsonSerialization{false};
    bool enable_profiling{false};  //!< indicator that profiling is enabled
    decltype(std::chrono::steady_clock::now())
        errorTimeStart;  //!< time when the error condition started related to the errorDelay
    std::atomic<int> lastErrorCode{0};  //!< storage for last error code
    std::string lastErrorString;  //!< storage for last error string
  private:
    /// buffer for profiling messages
    std::shared_ptr<ProfilerBuffer> prBuff;

    /** indicator that ticks should be forwarded to the command processor regardless */
    bool forwardTick{false};
    /** reasons ticks might be forwarded*/
    uint32_t forwardingReasons{0U};
    /** storage for the current state of the system */
    std::atomic<BrokerState> brokerState{BrokerState::created};

  public:
    explicit BrokerBase(bool DisableQueue = false) noexcept;
    explicit BrokerBase(const std::string& broker_name, bool DisableQueue = false);

    virtual ~BrokerBase();
    /** parse configuration information from command line arguments
    @return 0 for OK, positive numbers for expected information calls and negative number for error
    */
    int parseArgs(int argc, char* argv[]);
    /** parse configuration information from a vector of command line like arguments
    @return 0 for OK, positive numbers for expected information calls and negative number for error
    */
    int parseArgs(std::vector<std::string> args);
    /** parse configuration information from a string of command line like arguments
    @return 0 for OK, positive numbers for expected information calls and negative number for error
    */
    int parseArgs(const std::string& initializationString);
    /** configure the base of all brokers and cores
     */
    virtual void configureBase();

    /** add an action Message to the process queue*/
    void addActionMessage(const ActionMessage& m);
    /** move a action Message into the commandQueue*/
    void addActionMessage(ActionMessage&& m);

    /** set the logging callback function
    @param logFunction a function with a signature of void(int level, std::string_view identifier,
    std::string_view message) the function takes a level indicating the logging level string with
    the source name and a string with the message
    */
    void setLoggerFunction(
        std::function<void(int level, std::string_view identifier, std::string_view message)>
            logFunction);
    /** flush the loggers*/
    void logFlush();
    /** check if the main processing loop of a broker is running*/
    bool isRunning() const { return mainLoopIsRunning.load(); }
    /** set the logging level */
    void setLogLevel(int32_t level);
    /** set the logging levels
    @param consoleLevel the logging level for the console display
    @param fileLevel the logging level for the log file
    */
    void setLogLevels(int32_t consoleLevel, int32_t fileLevel);
    /** get the internal global broker id*/
    GlobalBrokerId getGlobalId() const { return global_id.load(); }

  private:
    /** start main broker loop*/
    void queueProcessingLoop();
    /** helper function for doing some preprocessing on a command
    @return (CMD_IGNORE) if the command is a termination command*/
    action_message_def::action_t commandProcessor(ActionMessage& command);

    /** Generate the base CLI processor*/
    std::shared_ptr<helicsCLI11App> generateBaseCLI();
    /** generate the loggers for the broker*/
    void generateLoggers();
    /** handle some configuration options for the base*/
    void baseConfigure(ActionMessage& command);

  protected:
    /** check whether a code contains a specific reason*/
    static bool isReasonForTick(std::uint32_t code, TickForwardingReasons reason)
    {
        return ((static_cast<std::uint32_t>(reason) & code) != 0);
    }
    /** set tick forwarding for a specific reason*/
    void setTickForwarding(TickForwardingReasons reason, bool value = true);
    BrokerState getBrokerState() const { return brokerState.load(); }
    bool setBrokerState(BrokerState newState);
    bool transitionBrokerState(BrokerState expectedState, BrokerState newState);
    /** process a disconnect signal*/
    virtual void processDisconnect(bool skipUnregister = false) = 0;
    /** in the case of connection failure with a broker this function will try a reconnect procedure
     */
    virtual bool tryReconnect() = 0;
    /** process a single command action
    @details cmd may be modified by this function*/
    virtual void processCommand(ActionMessage&& cmd) = 0;
    /** function to process a priority command independent of the main queue
    @details called when processing a priority command.  The priority command has a response message
    which gets sent this mainly deals with some of the registration functions
    @param command the command to process
    */
    virtual void processPriorityCommand(ActionMessage&& command) = 0;

    /** send a Message to the logging system
    @return true if the message was actually logged
    */
    virtual bool sendToLogger(GlobalFederateId federateID,
                              int logLevel,
                              std::string_view name,
                              std::string_view message) const;
    /** save a profiling message*/
    void saveProfilingData(std::string_view message);
    /** write profiler data to file*/
    void writeProfilingData();
    /** generate a new random id*/
    void generateNewIdentifier();
    /** generate the local address information*/
    virtual std::string generateLocalAddressString() const = 0;
    /** generate a CLI11 Application for subprocesses for processing of command line arguments*/
    virtual std::shared_ptr<helicsCLI11App> generateCLI();
    /** set the broker error state and error string*/
    void setErrorState(int eCode, std::string_view estring);
    /** set the logging file if using the default logger*/
    void setLoggingFile(const std::string& lfile);
    /** get the value of a particular flag*/
    bool getFlagValue(int32_t flag) const;
    /** virtual function to return the current simulation time*/
    virtual double getSimulationTime() const { return mInvalidSimulationTime; }

  public:
    /** generate a callback function for the logging purposes*/
    std::function<void(int, const std::string&, const std::string&)> getLoggingCallback() const;
    /** close all the threads*/
    void joinAllThreads();
    /** get the number of messages that have been processed internally*/
    std::size_t currentMessageCounter() const
    {
        return messageCounter.load(std::memory_order_acquire);
    }
    friend class TimeoutMonitor;
    friend const std::string& brokerStateName(BrokerState state);
};

/** helper function to generate the name of a state as a string
 */
const std::string& brokerStateName(BrokerBase::BrokerState state);

}  // namespace helics
