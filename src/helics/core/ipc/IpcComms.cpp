/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "IpcComms.h"
#include "helics/core/ActionMessage.h"
#include <memory>
#include <boost/interprocess/ipc/message_queue.hpp>


using ipc_queue = boost::interprocess::message_queue;

namespace helics
{



IpcComms::IpcComms(const std::string &brokerTarget, const std::string &localTarget):CommsInterface(brokerTarget,localTarget)
{

}
/** destructor*/
IpcComms::~IpcComms()
{

}

	void IpcComms::queue_rx_function()
	{
		std::unique_ptr<boost::interprocess::message_queue> rxQueue; //!< the receive queue
		try
		{
			rxQueue = std::make_unique<ipc_queue>(boost::interprocess::create_only, localTarget_.c_str(), maxMessageCount_, maxMessageSize_);
		}
		catch (boost::interprocess::interprocess_exception const& )
		{
			boost::interprocess::message_queue::remove(localTarget_.c_str());
			try
			{
				rxQueue = std::make_unique<ipc_queue>(boost::interprocess::create_only, localTarget_.c_str(), maxMessageCount_, maxMessageSize_);
			}
			catch (boost::interprocess::interprocess_exception const& )
			{
				ActionMessage err(CMD_ERROR);
				err.payload = "Unable to open local connection";
				ActionCallback(std::move(err));
				rx_status = connection_status::error; //the connection has failed
				return;
			}
		}
		rx_status = connection_status::connected; //this is a atomic indicator that the rx queue is ready
		unsigned int priority;
		size_t rx_size;
		std::vector<char> buffer(maxMessageSize_);

		while (1)
		{
			rxQueue->receive(buffer.data(), maxMessageSize_, rx_size, priority);
			if (rx_size < 8)
			{
				continue;
			}
			ActionMessage cmd(buffer.data(), rx_size);
			if ((cmd.action() == CMD_PROTOCOL) || (cmd.action() == CMD_PROTOCOL_BIG))
			{
				if (cmd.index == CLOSE_RECEIVER)
				{
					break;
				}
				continue;
			}

			ActionCallback(std::move(cmd));
		}
		rxQueue.reset();
		boost::interprocess::message_queue::remove(localTarget_.c_str());
		rx_status = connection_status::terminated;
	}

	void IpcComms::queue_tx_function()
	{
		std::unique_ptr<boost::interprocess::message_queue> brokerQueue;	//!< the queue of the broker
		std::unique_ptr<boost::interprocess::message_queue> rxQueue;
		std::map<int, std::unique_ptr<boost::interprocess::message_queue>> routes; //!< table of the routes to other brokers
		bool hasBroker = false;
		
		int sleep_counter = 50;
		if (!brokerTarget_.empty())
		{
			while (sleep_counter < 1700)
			{
				try
				{
					int test = 1;
					brokerQueue = std::make_unique<ipc_queue>(boost::interprocess::open_only, brokerTarget_.c_str());
					brokerQueue->send(&test, sizeof(int), 1);
					//brokerQueue->send(&test, sizeof(int), 2);
					//brokerQueue->send(&test, sizeof(int), 3);
					break;
				}
				catch (boost::interprocess::interprocess_exception const&)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(sleep_counter));
					sleep_counter *= 2;
					if (sleep_counter > 1700)
					{
						ActionMessage err(CMD_ERROR);
						err.payload = "Unable to open broker connection";
						ActionCallback(std::move(err));
						tx_status = connection_status::error;
						return;
					}
				}
			}
			hasBroker = true;
		}
		sleep_counter = 50;
		while (rx_status == connection_status::startup)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(sleep_counter));
			sleep_counter *= 2;
			if (sleep_counter > 1700)
			{
				ActionMessage err(CMD_ERROR);
				err.payload = "Unable to link with receiver";
				ActionCallback(std::move(err));
				tx_status = connection_status::error;
				return;
			}

		}
		if (rx_status == connection_status::error)
		{
			tx_status = connection_status::error;
			return;
		}
			try
			{
				int test = 1;
				rxQueue = std::make_unique<ipc_queue>(boost::interprocess::open_only, localTarget_.c_str());
				rxQueue->send(&test, sizeof(int), 1);
			}
			catch (boost::interprocess::interprocess_exception const&)
			{
					ActionMessage err(CMD_ERROR);
					err.payload = "Unable to open receiver connection";
					ActionCallback(std::move(err));
					tx_status = connection_status::error;
					return;
			}
			tx_status = connection_status::connected;
		while (1)
		{
			int route_id;
			ActionMessage cmd;
			std::tie(route_id, cmd) = txQueue.pop();
			if ((cmd.action() == CMD_PROTOCOL)||(cmd.action()==CMD_PROTOCOL_PRIORITY))
			{
				if (route_id == -1)
				{
					switch (cmd.index)
					{
					case NEW_ROUTE:
					{
						int tries = 0;
						while (tries < 3)
						{
							try
							{
								auto newQueue = std::make_unique<ipc_queue>(boost::interprocess::open_only, cmd.payload.c_str());
								routes.emplace(cmd.dest_id, std::move(newQueue));
								break;
							}
							catch (boost::interprocess::interprocess_exception const& ipe)
							{
								std::this_thread::sleep_for(std::chrono::milliseconds(100));
								++tries;
							}
						}
						continue;
					}
					case DISCONNECT:
						tx_status = connection_status::terminated;
						return;
					}
				}
			}
			std::string buffer = cmd.to_string();
			int priority = isPriorityCommand(cmd) ? 3 : 1;
			if (route_id == 0)
			{
				//brokerQueue->send(&priority, sizeof(int), 3);
				if (brokerQueue)
				{
					brokerQueue->send(buffer.data(), buffer.size(), priority);
				}
				
			}
			else if (route_id == -1)
			{
				rxQueue->send(buffer.data(), buffer.size(), priority);
			}
			else
			{
				auto routeFnd = routes.find(route_id);
				if (routeFnd != routes.end())
				{
					routeFnd->second->send(buffer.data(), buffer.size(), priority);
				}
				else
				{
					if (brokerQueue)
					{
						brokerQueue->send(buffer.data(), buffer.size(), priority);
					}
					
				}
			}

		}
		tx_status = connection_status::terminated;
	}
	


void IpcComms::closeTransmitter()
{
	ActionMessage rt(CMD_PROTOCOL);
	rt.index = DISCONNECT;
	transmit(-1, rt);
}

void IpcComms::closeReceiver()
{
	if (tx_status == connection_status::connected)
	{
		ActionMessage cmd(CMD_PROTOCOL);
		cmd.index = CLOSE_RECEIVER;
		transmit(-1, cmd);
	}
	else
	{
		//TODO:: what to do here
	}
}

}