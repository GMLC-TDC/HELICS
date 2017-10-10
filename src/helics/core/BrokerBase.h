/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

/** 
@file
virtual base class for object that function like a broker includes common parameters
and some common methods used cores and brokers
*/

#ifndef BROKER_BASE_H_
#define BROKER_BASE_H_
#pragma once
#include <string>
#include <memory>
#include <thread>
#include <atomic>

#include "core.h"
#include "ActionMessage.h"
#include "helics/common/BlockingQueue3.hpp"
namespace helics
{

class logger;
class TimeCoordinator;
/** base class for broker like objects
*/
class BrokerBase
{
protected:
	std::atomic<Core::federate_id_t> global_broker_id{ 0 };
	int32_t maxLogLevel = 1;  //!< the logging level to use levels >=this will be logged
	int32_t consoleLogLevel = 1; //!< the logging level for console display
	int32_t fileLogLevel = 1; //!< the logging level for logging to a file
	int32_t _min_federates=1;  //!< the minimum number of federates that must connect before entering init mode
	int32_t _min_brokers=0;  //!< the minimum number of brokers that must connect before entering init mode
	int32_t _max_iterations=10000; //!< the maximum number of iterative loops that are allowed
	std::string identifier;  //!< an identifier for the broker

	std::unique_ptr<logger> loggingObj;  //!< default logging object to use if the logging callback is not specified
	std::thread _queue_processing_thread;  //!< thread for running the broker
										   /** a logging function for logging or printing messages*/
	std::function<void(int, const std::string &, const std::string &)> loggerFunction;
	std::atomic<bool> haltOperations{ false };  //!< flag indicating that no further message should be processed
	std::string logFile; //< the file to log message to
	std::unique_ptr<TimeCoordinator> timeCoord; //!< object managing the time control
	BlockingQueue3<ActionMessage> _queue; //!< primary routing queue
										  /** enumeration of the possible core states*/
	enum broker_state_t :int
	{
		created = -5,
		initialized = -4,
		connecting = -3,
		connected = -2,
		initializing = -1,
		operating = 0,
		terminating = 1,
		terminated = 3,
		errored = 7,
	};
	std::atomic<broker_state_t> brokerState{ created }; //!< flag indicating that the structure is past the initialization stage indicating that no more changes can be made to the number of federates or handles
public:
	BrokerBase() noexcept;
	BrokerBase(const std::string &broker_name);

	virtual ~BrokerBase();

	/** initialize the core manager with command line arguments
	@param[in] argc the number of arguments
	@param[in] argv char pointers to the arguments
	*/
	virtual void InitializeFromArgs(int argc, char *argv[]);

	/** add an action Message to the process queue*/
	void addActionMessage(const ActionMessage &m);
	/** move a action Message into the commandQueue*/
	void addActionMessage(ActionMessage &&m);

	void setLoggerFunction(std::function<void(int, const std::string &, const std::string &)> logFunction);
private:
	/** start main broker loop*/
	void queueProcessingLoop();
protected:
	/** process a single command action
	@details cmd may be modified by this function*/
	virtual void processCommand(ActionMessage &&cmd) = 0;
	/** function to process a priority command independent of the main queue
	@detailed called when processing a priority command.  The priority command has a response message which gets sent
	this mainly deals with some of the registration functions
	@param[in] command the command to process
	@return a action message response to the priority command
	*/
	virtual void processPriorityCommand(const ActionMessage &command) = 0;
	/* process a disconnect signal*/
	virtual void processDisconnect() = 0;

	/** send a Message to the logging system
	@return true if the message was actually logged
	*/
	virtual bool sendToLogger(Core::federate_id_t federateID, int logLevel, const std::string &name, const std::string &message) const;

	/** generate a new random id based on a uuid*/
	void generateNewIdentifier();
	/** close all the threads*/
	void joinAllThreads();
private:



};


}  //namespace helics
#endif
