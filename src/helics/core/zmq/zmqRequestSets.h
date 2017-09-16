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
	void addRoutes(int routeNumber, const std::string &routeInfo);
	bool transmit(int routeNumber, const ActionMessage &command);

	bool waiting() const;
	bool checkForMessages();
	bool checkForMessages(std::chrono::milliseconds timeout);
	stx::optional<ActionMessage> getMessage();
private:
	void checkDelayedSend();
private:
	std::map<int, std::unique_ptr<zmq::socket_t>> routes;
	std::map<int, bool> routes_waiting;
	std::vector<zmq::pollitem_t> active_routes;
	std::vector<waitingResponse> active_messages;
	std::vector<std::pair<int, ActionMessage>> waiting_messages;
	std::deque<ActionMessage> Responses;
	std::shared_ptr<zmqContextManager> ctx;

};
} //namespace helics

#endif
