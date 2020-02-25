/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
/**
@file
virtual base class for object that function like a broker includes common parameters
and some common methods used cores and brokers
*/

#include "ActionMessage.hpp"
#include "federate_id_extra.hpp"
#include "gmlc/containers/BlockingPriorityQueue.hpp"

#include <atomic>
#include <memory>
#include <string>
#include <thread>

namespace helics {
class Logger;
class ForwardingTimeCoordinator;
class helicsCLI11App;
/** base class for broker like objects
 */
class BrokerBase {
  protected:
    std::atomic<global_broker_id> global_id{
        parent_broker_id}; //!< the unique identifier for the broker(core or broker)
    global_broker_id
        global_broker_id_local{}; //!< meant to be the same as global_id but not atomically protected
    global_broker_id higher_broker_id{0}; //!< the id code of the broker 1 level about this broker
    std::atomic<int32_t> maxLogLevel{1}; //!< the logging level to use levels >=this will be ignored
    int32_t consoleLogLevel{1}; //!< the logging level for console display
    int32_t fileLogLevel{1}; //!< the logging level for logging to a file
    int32_t minFederateCount{
        1}; //!< the minimum number of federates that must connect before entering init mode
    int32_t minBrokerCount{
        0}; //!< the minimum number of brokers that must connect before entering init mode
    int32_t maxIterationCount{10000}; //!< the maximum number of iterative loops that are allowed
    Time tickTimer{5.0}; //!< the length of each heartbeat tick
    Time timeout{30.0}; //!< timeout to wait to establish a broker connection before giving up
    Time networkTimeout{-1.0}; //!< timeout to establish a socket connection before giving up
    Time errorDelay{ 10.0 };  //!< time to delay before terminating after error state
    std::string identifier; //!< an identifier for the broker
    std::string
        brokerKey; //!< a key that all joining federates must have to connect if empty no key is required
    // address is mutable since during initial phases it may not be fixed so to maintain a consistent public
    // interface for extracting it this variable may need to be updated in a constant function
    mutable std::string address; //!< network location of the broker
    std::unique_ptr<Logger>
        loggingObj; //!< default logging object to use if the logging callback is not specified
    std::thread queueProcessingThread; //!< thread for running the broker
    /** a logging function for logging or printing messages*/
    std::function<void(int, const std::string&, const std::string&)> loggerFunction;

    std::atomic<bool> haltOperations{
        false}; //!< flag indicating that no further message should be processed
    bool restrictive_time_policy{
        false}; //!< flag indicating the broker should use a conservative time policy
  private:
    std::atomic<bool> mainLoopIsRunning{
        false}; //!< flag indicating that the main processing loop is running
    bool dumplog{false}; //!< flag indicating the broker should capture a dump log
    bool forceLoggingFlush{false}; //!< force the log to flush after every message
    bool queueDisabled{
        false}; //!< flag indicating that the message queue should not be used and all functions
    //!< called directly instead of distinct thread
    bool disable_timer{false}; //!< turn off the timer/timeout subsystem completely
    std::atomic<std::size_t> messageCounter{
        0}; //!< counter for the total number of message processed
  protected:
    std::string logFile; //!< the file to log message to
    std::unique_ptr<ForwardingTimeCoordinator> timeCoord; //!< object managing the time control
    gmlc::containers::BlockingPriorityQueue<ActionMessage> actionQueue; //!< primary routing queue
    /** enumeration of the possible core states*/
    enum class broker_state_t : int16_t {
        created = -6, //!< the broker has been created
        configuring = -5, //!< the broker is in the processing of configuring
        configured = -4, //!< the broker itself has been configured and is ready to connect
        connecting = -3, //!< the connection process has started
        connected = -2, //!< the connection process has completed
        initializing = -1, //!< the enter initialization process has started
        operating = 0, //!< normal operating conditions
        terminating = 1, //!< the termination process has started
        terminated = 3, //!< the termination process has started
        errored = 7, //!< an error was encountered
    };
    std::atomic<broker_state_t> brokerState{
        broker_state_t::created}; //!< flag indicating that the structure is past the
    //!< initialization stage indicating that no more changes
    //!< can be made to the number of federates or handles
    bool noAutomaticID{false}; //!< the broker should not automatically generate an ID
    bool hasTimeDependency{false}; //!< set to true if the broker has Time dependencies
    bool enteredExecutionMode{
        false}; //!< flag indicating that the broker has entered execution mode
    bool waitingForBrokerPingReply{false}; //!< flag indicating we are waiting for a ping reply
    bool hasFilters{false}; //!< flag indicating filters come through the broker
    bool forwardTick{
        false}; //!< indicator that ticks should be forwarded to the command processor regardless
    bool no_ping{false}; //!< indicator that the broker is not very responsive to ping requests

    std::atomic<int> errorCode{0}; //!< storage for last error code
    std::string lastErrorString; //!< storage for last error string

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
    @param logFunction a function with a signature of void(int level,  const std::string &source,  const
    std::string &message) the function takes a level indicating the logging level string with the source name and a
    string with the message
    */
    void setLoggerFunction(
        std::function<void(int, const std::string&, const std::string&)> logFunction);

    /** check if the main processing loop of a broker is running*/
    bool isRunning() const { return mainLoopIsRunning.load(); }
    /** set the logging level */
    void setLogLevel(int32_t level);
    /** set the logging levels
    @param consoleLevel the logging level for the console display
    @param fileLevel the logging level for the log file
    */
    void setLogLevels(int32_t consoleLevel, int32_t fileLevel);

  private:
    /** start main broker loop*/
    void queueProcessingLoop();
    /** helper function for doing some preprocessing on a command
    @return (CMD_IGNORE) if the command is a termination command*/
    action_message_def::action_t commandProcessor(ActionMessage& command);

    /** Generate the base CLI processor*/
    std::shared_ptr<helicsCLI11App> generateBaseCLI();

  protected:
    /** process a disconnect signal*/
    virtual void processDisconnect(bool skipUnregister = false) = 0;
    /** in the case of connection failure with a broker this function will try a reconnect procedure
     */
    virtual bool tryReconnect() = 0;
    /** process a single command action
    @details cmd may be modified by this function*/
    virtual void processCommand(ActionMessage&& cmd) = 0;
    /** function to process a priority command independent of the main queue
    @details called when processing a priority command.  The priority command has a response message which gets
    sent this mainly deals with some of the registration functions
    @param command the command to process
    */
    virtual void processPriorityCommand(ActionMessage&& command) = 0;

    /** send a Message to the logging system
    @return true if the message was actually logged
    */
    virtual bool sendToLogger(
        global_federate_id federateID,
        int logLevel,
        const std::string& name,
        const std::string& message) const;

    /** generate a new random id*/
    void generateNewIdentifier();
    /** generate the local address information*/
    virtual std::string generateLocalAddressString() const = 0;
    /** generate a CLI11 Application for subprocesses for processing of command line arguments*/
    virtual std::shared_ptr<helicsCLI11App> generateCLI();
    /** set the broker error state and error string*/
    void setErrorState(int eCode, const std::string& estring);
    /** set the logging file if using the default logger*/
    void setLoggingFile(const std::string& lfile);

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
    friend const std::string& brokerStateName(broker_state_t state);
};

/** helper function to generate the name of a state as a string
*/
const std::string& brokerStateName(BrokerBase::broker_state_t state);
} // namespace helics
