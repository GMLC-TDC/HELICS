/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_ZEROMQ_REQUEST_SETS_
#define _HELICS_ZEROMQ_REQUEST_SETS_
#pragma once

#include "helics/common/cppzmq/zmq.hpp"
#include "helics/core/ActionMessage.h"
#include "helics/common/zmqContextManager.h"
#include <map>
#include <deque>
#include <string>
#include "helics_includes/optional.h"

namespace helics
{

/**helper class to manage REQ sockets that are awaiting a response*/
class waitingResponse
{
public:
	int route;
	std::uint16_t loops = 0;
	bool waiting = false;
	ActionMessage txmsg;

};
/** class for dealing with the priority message paths from a ZMQ comm object
@details it manages a set of routes to different priority queues and handles the responses
*/
class ZmqRequestSets
{
public:
	/** constructor*/
	ZmqRequestSets();
	/** add a route to the request set*/
	void addRoutes(int routeNumber, const std::string &routeInfo);
	/** transmit a command to a specific route number*/
	bool transmit(int routeNumber, const ActionMessage &command);
	/** check if the request set is waiting on any on responses*/
	bool waiting() const;
	/** check for messages with a 0 second timeout*/
	bool checkForMessages();
	/** check for messages with an explicit timeout*/
	bool checkForMessages(std::chrono::milliseconds timeout);
	/** get any messages that have been received*/
	stx::optional<ActionMessage> getMessage();
private:
	/** check to send any messages that have been delayed waiting for a previous send*/
	void checkDelayedSend();
private:
	std::map<int, std::unique_ptr<zmq::socket_t>> routes; //!< map of all the routes
	std::map<int, bool> routes_waiting;	//!< routes that are waiting to be sent
	std::vector<zmq::pollitem_t> active_routes; //!< active routes for zmq poller
	std::vector<waitingResponse> active_messages; //!< more information about waiting messages
	std::vector<std::pair<int, ActionMessage>> waiting_messages; //!< messages that are queued up to send
	std::deque<ActionMessage> Responses;	//!< message that have been received and are waiting to be sent to the holder
	std::shared_ptr<zmqContextManager> ctx; //!< the zmq context manager

};
} //namespace helics

#endif
