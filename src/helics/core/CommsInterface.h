/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_COMMS_INTERFACE_
#define _HELICS_COMMS_INTERFACE_
#pragma once

#include "ActionMessage.h"
#include "common/BlockingQueue3.hpp"
#include <functional>
#include <thread>
#include <atomic>

namespace helics {

/** implementation of a generic communications interface
*/
class CommsInterface {

public:
	/** default constructor*/
	CommsInterface() = default;
	CommsInterface(const std::string &localTarget, const std::string &brokerTarget);
	/** destructor*/
	virtual ~CommsInterface();
	/** transmit a message along a particular route
	*/
	void transmit(int route_id, const ActionMessage &cmd);
	/** add a new route assigned to the appropriate id
	*/
	void addRoute(int route_id, const std::string &routeInfo);
	/** connect the commsInterface
	@return true if the connection was successful false otherwise
	*/
	bool connect();
	/** disconnected the comms interface
	*/
	void disconnect();
	/** set the name of the communicator*/
	void setName(const std::string &name);
	/** set the callback for processing the messages
	*/
	void setCallback(std::function<void(ActionMessage &&)> callback);
	/** set the max message size and max Queue size 
	*/
	void setMessageSize(int maxMessageSize, int maxQueueSize);
	/** check if the commInterface is connected
	*/
	bool isConnected() const;
protected:
	//enumeration of the connection status flags for more immediate feedback from the processing threads
	enum class connection_status :int
	{
		error = -1,	//!< some error occurred on the connection
		startup = 0, //!< the connection is in startup mode
		connected = 1,	//!< we are connected
		terminated=2,	//!< the connection has been terminated

	};
	std::atomic<connection_status> rx_status{ connection_status::startup }; //!< the status of the receiver thread
	std::string name;  //!< the name of the object
	std::string localTarget_; //!< the identifier for the receive address
	std::string brokerTarget_;	//!< the identifier for the broker address
	std::atomic<connection_status> tx_status{ connection_status::startup }; //!< the status of the transmitter thread
	int maxMessageSize_ = 16 * 1024; //!< the maximum message size for the queues (if needed)
	int maxMessageCount_ = 512;  //!< the maximum number of message to buffer (if needed)
	std::function<void(ActionMessage &&)> ActionCallback; //!< the callback for what to do with a received message
	BlockingQueue3<std::pair<int, ActionMessage>> txQueue; //!< set of messages waiting to be transmitted
	// closing the files or connection can take some time so there is a need for interthread communication to not spit out warning messages if it is in the process of disconnecting
	std::atomic<bool> disconnecting{ false }; //!<flag indicating that the comm system is in the process of disconnecting
private:
	std::thread queue_transmitter; //!< single thread for sending data
	std::thread queue_watcher; //!< thread monitoring the receive queue
	std::mutex threadSyncLock; //!< lock to handle thread operations
	virtual void queue_rx_function()=0;	//!< the functional loop for the receive queue
	virtual void queue_tx_function()=0;  //!< the loop for transmitting data
	virtual void closeTransmitter() = 0; //!< function to instruct the transmitter loop to close
	virtual void closeReceiver() = 0;  //!< function to instruct the receiver loop to close
};


} // namespace helics

#endif /* _HELICS_COMMS_INTERFACE_ */
