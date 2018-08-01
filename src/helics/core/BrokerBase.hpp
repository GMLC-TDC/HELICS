/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once
/**
@file
virtual base class for object that function like a broker includes common parameters
and some common methods used cores and brokers
*/

#include "../common/BlockingPriorityQueue.hpp"
#include "ActionMessage.hpp"
#include "federate_id.hpp"
#include <atomic>
#include <memory>
#include <string>
#include <thread>
namespace helics
{
class Logger;
class ForwardingTimeCoordinator;
/** base class for broker like objects
 */
class BrokerBase
{
  protected:
      std::atomic<global_broker_id_t> global_broker_id{ parent_broker_id };  //!< the unique identifier for the broker(core or broker)
      global_broker_id_t global_broker_id_local;  //!< meant to be the same as global_broker_id but not atomically protected
    global_broker_id_t higher_broker_id{ 0 };  //!< the id code of the broker 1 level about this broker
    std::atomic<int32_t> maxLogLevel{1};  //!< the logging level to use levels >=this will be logged
    int32_t consoleLogLevel = 1;  //!< the logging level for console display
    int32_t fileLogLevel = 1;  //!< the logging level for logging to a file
    int32_t minFederateCount = 1;  //!< the minimum number of federates that must connect before entering init mode
    int32_t minBrokerCount = 0;  //!< the minimum number of brokers that must connect before entering init mode
    int32_t maxIterationCount = 10000;  //!< the maximum number of iterative loops that are allowed
    int32_t tickTimer = 4000;  //!< counter for the length of a keep alive tick in milliseconds
    int32_t timeout =
      30000;  //!< timeout to wait to establish a broker connection before giving up in milliseconds
    std::string identifier;  //!< an identifier for the broker

    std::unique_ptr<Logger>
      loggingObj;  //!< default logging object to use if the logging callback is not specified
    std::thread queueProcessingThread;  //!< thread for running the broker
    /** a logging function for logging or printing messages*/
    std::function<void(int, const std::string &, const std::string &)> loggerFunction;

    std::atomic<bool> haltOperations{false};  //!< flag indicating that no further message should be processed
  private:
    std::atomic<bool> mainLoopIsRunning{false};  //!< flag indicating that the main processing loop is running
    bool dumplog = false;  //!< flag indicating the broker should capture a dump log

  protected:
    std::string logFile;  //< the file to log message to
    std::unique_ptr<ForwardingTimeCoordinator> timeCoord;  //!< object managing the time control
    BlockingPriorityQueue<ActionMessage> actionQueue;  //!< primary routing queue
    /** enumeration of the possible core states*/
    enum broker_state_t : int16_t
    {
        created = -5,  //!< the broker has been created
        initialized = -4,  //!< the broker itself has been initialized and is ready to connect
        connecting = -3,  //!< the connection process has started
        connected = -2,  //!< the connection process has completed
        initializing = -1,  //!< the enter initialization process has started
        operating = 0,  //!< normal operating conditions
        terminating = 1,  //!< the termination process has started
        terminated = 3,  //!< the termination process has started
        errored = 7,  //!< an error was encountered
    };
    std::atomic<broker_state_t> brokerState{created};  //!< flag indicating that the structure is past the
                                                       //!< initialization stage indicating that no more changes
                                                       //!< can be made to the number of federates or handles
    bool noAutomaticID = false;
    bool hasTimeDependency = false;  //!< set to true if the broker has Time dependencies
    bool enteredExecutionMode = false;  //!< flag indicating that the broker has entered execution mode
    bool waitingForServerPingReply = false;  //!< flag indicating we are waiting for a ping reply
    bool hasFilters = false;  //!< flag indicating filters come through the broker
  public:
    /** display help messages for the broker*/
    static void displayHelp ();
    BrokerBase () noexcept;
    explicit BrokerBase (const std::string &broker_name);

    virtual ~BrokerBase ();

    /** initialize the core manager with command line arguments
    @param[in] argc the number of arguments
    @param[in] argv char pointers to the arguments
    */
    virtual void initializeFromCmdArgs (int argc, const char *const *argv);

    /** add an action Message to the process queue*/
    void addActionMessage (const ActionMessage &m);
    /** move a action Message into the commandQueue*/
    void addActionMessage (ActionMessage &&m);

    /** set the logging callback function
    @param logFunction a function with a signature of void(int level,  const std::string &source,  const
    std::string &message) the function takes a level indicating the logging level string with the source name and a
    string with the message
    */
    void setLoggerFunction (std::function<void(int, const std::string &, const std::string &)> logFunction);

    /** process a disconnect signal*/
    virtual void processDisconnect (bool skipUnregister = false) = 0;
    /** check if the main processing loop of a broker is running*/
    bool isRunning () const { return mainLoopIsRunning.load (); }
    /** set the logging level */
    void setLogLevel (int32_t level);
    /** set the logging levels
    @param consoleLevel the logging level for the console display
    @param fileLevel the logging level for the log file
    */
    void setLogLevels (int32_t consoleLevel, int32_t fileLevel);

  private:
    /** start main broker loop*/
    void queueProcessingLoop ();

  protected:
    /** in the case of connection failure with a broker this function will try a reconnect procedure
     */
    virtual bool tryReconnect () = 0;
    /** process a single command action
    @details cmd may be modified by this function*/
    virtual void processCommand (ActionMessage &&cmd) = 0;
    /** function to process a priority command independent of the main queue
    @detailed called when processing a priority command.  The priority command has a response message which gets
    sent this mainly deals with some of the registration functions
    @param[in] command the command to process
    @return a action message response to the priority command
    */
    virtual void processPriorityCommand (ActionMessage &&command) = 0;

    /** send a Message to the logging system
    @return true if the message was actually logged
    */
    virtual bool sendToLogger (global_federate_id_t federateID,
                               int logLevel,
                               const std::string &name,
                               const std::string &message) const;

    /** generate a new random id based on a uuid*/
    void generateNewIdentifier ();

  public:
    /** close all the threads*/
    void joinAllThreads ();
};

}  // namespace helics
