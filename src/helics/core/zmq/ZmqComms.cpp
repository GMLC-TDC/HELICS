/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "ZmqComms.h"
#include "helics/common/zmqContextManager.h"
#include "helics/common/zmqHelper.h"
#include "helics/common/zmqSocketDescriptor.h"

#include "helics/core/ActionMessage.h"
#include <memory>



namespace helics
{



ZmqComms::ZmqComms(const std::string &brokerTarget, const std::string &localTarget):CommsInterface(brokerTarget,localTarget)
{

}
/** destructor*/
ZmqComms::~ZmqComms()
{

}

void ZmqComms::setPortNumbers(int repPort, int pullPort)
{
	if (rx_status == connection_status::startup)
	{
		repPortNumber = repPort;
		pullPortNumber = pullPort;
	}
	
}

void ZmqComms::setReplyCallback(std::function<ActionMessage(ActionMessage &&)> callback)
{
	replyCallback = std::move(callback);
}

	void ZmqComms::queue_rx_function()
	{
		auto ctx = zmqContextManager::getContextPointer();
		zmq::socket_t pullSocket(ctx->getContext(), ZMQ_PULL);

		pullSocket.bind(localTarget_.c_str());
		zmq::socket_t controlSocket(ctx->getContext(), ZMQ_PAIR);
		std::string controlsockString = std::string("inproc://") + localTarget_ + "_control";
		controlSocket.connect(controlsockString.c_str());

		zmq::socket_t repSocket(ctx->getContext(), ZMQ_REP);
		//repSocket.bind(brokerRepAddress.c_str());
		std::vector<zmq::pollitem_t> poller(3);
		poller[0].socket = static_cast<void *>(controlSocket);
		poller[0].events = ZMQ_POLLIN;
		poller[1].socket = static_cast<void *>(pullSocket);
		poller[1].events = ZMQ_POLLIN;
		poller[2].socket = static_cast<void *>(repSocket);
		poller[2].events = ZMQ_POLLIN;
		rx_status = connection_status::connected; //this is a atomic indicator that the rx queue is ready
		while (1)
		{
			auto rc = zmq::poll(poller);
			if (rc > 0)
			{
				zmq::message_t msg;
				if ((poller[0].revents&ZMQ_POLLIN) != 0)
				{
					controlSocket.recv(&msg);
					if (msg.size() == 5)
					{
						std::string str(static_cast<char *>(msg.data()), msg.size());
						if (str == "close")
						{
							break;
						}
					}

				}
				if ((poller[1].revents&ZMQ_POLLIN) != 0)
				{
					pullSocket.recv(&msg);
					
				}
				if ((poller[2].revents&ZMQ_POLLIN) != 0)
				{
					repSocket.recv(&msg);
					ActionMessage M(static_cast<char *>(msg.data()), msg.size());
					if (replyCallback)
					{
						auto Mresp = replyCallback(std::move(M));
						auto str = Mresp.to_string();

						repSocket.send(str.data(), str.size());
					}
					else
					{
						ActionCallback(std::move(M));
						ActionMessage resp(CMD_PRIORITY_ACK);
						auto str = resp.to_string();

						repSocket.send(str.data(), str.size());
					}
					
					continue;
				}
				ActionMessage M(static_cast<char *>(msg.data()), msg.size());
				ActionCallback(std::move(M));
			}
		}
		rx_status = connection_status::terminated;
	}

	void ZmqComms::queue_tx_function()
	{
		bool hasBroker = false;
		auto ctx = zmqContextManager::getContextPointer();
		zmq::socket_t reqSocket(ctx->getContext(), ZMQ_REQ);
		if (!brokerTarget_.empty())
		{
			reqSocket.connect(brokerTarget_.c_str());
			hasBroker = true;
		}
		
		zmq::socket_t brokerPushSocket(ctx->getContext(), ZMQ_PUSH);
		std::map<int, zmq::socket_t> routes;  // for all the other possible routes
		zmq::socket_t controlSocket(ctx->getContext(), ZMQ_PAIR);

		std::string controlsockString = std::string("inproc://") + localTarget_ + "_control";
		controlSocket.bind(controlsockString.c_str());

		tx_status = connection_status::connected;
		std::vector<char> buffer;
		std::vector<char> rxbuffer(4096);
		while (1)
		{
			int route_id;
			ActionMessage cmd;
			std::tie(route_id, cmd) = txQueue.pop();
			if (cmd.action() == CMD_PROTOCOL)
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
								auto zsock = zmq::socket_t(ctx->getContext(), ZMQ_PUSH);
								zsock.connect(cmd.payload.c_str());
								routes.emplace(cmd.dest_id, std::move(zsock));
								break;
							}
							catch (zmq::error_t)
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
			cmd.to_vector(buffer);
			if (isPriorityCommand(cmd))
			{
				reqSocket.send(buffer.data(), buffer.size());

				// TODO:: need to figure out how to catch overflow and resize the rxbuffer
				// admittedly this would probably be a very very long name but it could happen
				auto nsize = reqSocket.recv(rxbuffer.data(), rxbuffer.size());
				if ((nsize > 0) && (nsize < rxbuffer.size()))
				{
					ActionMessage rxcmd(rxbuffer.data(), rxbuffer.size());
					ActionCallback(std::move(rxcmd));
				}

			}
			if (route_id == 0)
			{
				if (hasBroker)
				{
					brokerPushSocket.send(buffer.data(), buffer.size());
				}
				
			}
			else if (route_id == -1)
			{
				controlSocket.send(buffer.data(), buffer.size());
			}
			else
			{
				auto rt_find = routes.find(route_id);
				if (rt_find != routes.end())
				{
					rt_find->second.send(buffer.data(), buffer.size());
				}
				else
				{
					if (hasBroker)
					{
						brokerPushSocket.send(buffer.data(), buffer.size());
					}
					
				}
			}
		}
		reqSocket.close();

		brokerPushSocket.close();
		
		
		routes.clear();
		controlSocket.send("close");
		controlSocket.close();
		
		tx_status = connection_status::terminated;
	}
	


	void ZmqComms::closeTransmitter()
	{
		ActionMessage rt(CMD_PROTOCOL);
		rt.index = DISCONNECT;
		transmit(-1, rt);
	}

	void ZmqComms::closeReceiver()
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