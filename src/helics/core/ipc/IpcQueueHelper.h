/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_IPC_QUEUE_HELPER_
#define _HELICS_IPC_QUEUE_HELPER_
#pragma once

#include "helics/core/ActionMessage.h"
#include <memory>
#include <algorithm>
#include <cctype>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>



using ipc_queue = boost::interprocess::message_queue;
using ipc_state = boost::interprocess::shared_memory_object;

namespace helics {
/** translate a string to a C++ qualified name for variable naming purposes
*/
inline std::string stringTranslateToCppName(std::string in)
{
	std::replace_if(in.begin(), in.end(), [](auto c) {return !(std::isalnum(c) || (c == '_')); }, '_');
	return in;
}

enum class queue_state_t :int
{
	startup = 0,
	connected = 1,
	operating = 2,
	closing = 3,
};

class shared_queue_state
{
private:
	mutable boost::interprocess::interprocess_mutex data_lock;
	queue_state_t state = queue_state_t::startup;
public:
	queue_state_t getState() const
	{
		boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(data_lock);
		return state;
	}
	void setState(queue_state_t newState)
	{
		boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(data_lock);
		state = newState;
	}
};


class ownedQueue
{
private:
	std::unique_ptr<ipc_queue> rqueue;
	std::unique_ptr<ipc_state> queue_state;
	std::string connectionNameOrig;
	std::string connectionName;
	std::string stateName;
	std::string errorString;
	std::vector<char> buffer;
	int mxSize = 0;
	bool connected = false;
public:
	ownedQueue() = default;
	~ownedQueue();
	bool connect(const std::string &connection, int maxMessages, int maxSize);

	void changeState(queue_state_t newState);

	ActionMessage getMessage(int timeout);

	const std::string &getError() const
	{
		return errorString;
	}

};


class sendToQueue
{
private:
	std::unique_ptr<ipc_queue> txqueue;
	std::string connectionNameOrig;
	std::string connectionName;
	std::string errorString;
	std::vector<char> buffer;
	bool connected = false;
public:
	sendToQueue() = default;

	bool connect(const std::string &connection, bool initOnly, int retries);

	void sendMessage(const ActionMessage &cmd, int priority);

	const std::string &getError() const
	{
		return errorString;
	}

};


} // namespace helics

#endif /* _HELICS_IPC_QUEUE_HELPER_ */

