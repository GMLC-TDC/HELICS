/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "CommsInterface.h"



namespace helics
{

CommsInterface::CommsInterface(const std::string &localTarget, const std::string &brokerTarget) :localTarget_(localTarget), brokerTarget_(brokerTarget)
{

}
/** destructor*/
CommsInterface::~CommsInterface()
{
	disconnect();
}

void CommsInterface::transmit(int route_id, const ActionMessage &cmd)
{
	txQueue.push(std::pair<int, ActionMessage>(route_id, cmd));
}

void CommsInterface::addRoute(int route_id, const std::string &routeInfo)
{
	ActionMessage rt(CMD_PROTOCOL);
	rt.payload = routeInfo;
	rt.index = NEW_ROUTE;
	rt.dest_id = route_id;
	transmit(-1, rt);
}

bool CommsInterface::connect()
{
	if (!ActionCallback)
	{
		std::cerr << "no callback specified, the receiver cannot start\n";
		return false;
	}
	queue_watcher = std::thread([this] {queue_rx_function(); });
	queue_transmitter = std::thread([this] {queue_tx_function(); });

	while (rx_status == connection_status::startup)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	while (tx_status == connection_status::startup)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	if (rx_status != connection_status::connected)
	{
		if (tx_status == connection_status::connected)
		{
			if (queue_transmitter.joinable())
			{
				closeTransmitter();
				queue_transmitter.join();
			}
		}
		return false;
	}

	if (tx_status != connection_status::connected)
	{
		if (rx_status == connection_status::connected)
		{
			if (queue_watcher.joinable())
			{
				closeReceiver();
				queue_watcher.join();
			}
		}
		return false;
	}
	return true;
}


void CommsInterface::disconnect()
{
	if (queue_watcher.joinable())
	{
		closeReceiver();
		queue_watcher.join();
	}
	if (queue_transmitter.joinable())
	{
		closeTransmitter();
		queue_transmitter.join();
	}
}

void CommsInterface::setCallback(std::function<void(ActionMessage &&)> callback)
{
	ActionCallback = std::move(callback);
}

void CommsInterface::setMessageSize(int maxMessageSize, int maxMessageCount)
{
	if (maxMessageSize > 0)
	{
		maxMessageSize_ = maxMessageSize;
	}
	if (maxMessageCount > 0)
	{
		maxMessageCount_ = maxMessageCount;
	}
	
}

}