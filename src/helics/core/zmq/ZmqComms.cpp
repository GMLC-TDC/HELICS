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

static const int BEGIN_OPEN_PORT_RANGE = 23500;
static const int BEGIN_OPEN_PORT_RANGE_SUBBROKER = 23900;

static const int DEFAULT_BROKER_PULL_PORT_NUMBER = 23405;
static const int DEFAULT_BROKER_REP_PORT_NUMBER = 23404;



namespace helics
{



ZmqComms::ZmqComms(const std::string &brokerTarget, const std::string &localTarget):CommsInterface(brokerTarget,localTarget)
{
	localTarget_.empty();
	localTarget_ = "tcp:/*";
}
/** destructor*/
ZmqComms::~ZmqComms()
{
	disconnect();
}

std::string makePortAddress(const std::string &add, int portNumber)
{
	if (add.compare(0, 3, "ipc") == 0)
	{
		return add;
	}
	if (add.compare(0, 3, "tcp") == 0)
	{
		std::string newAddress = add;
		newAddress.push_back(':');
		newAddress.append(std::to_string(portNumber));
		return newAddress;
	}
	if (add.compare(0, 5, "inproc") == 0)
	{
		return add;
	}
	return add;

}

std::pair<std::string, int> extractInterfaceandPort(const std::string &address)
{
	std::pair<std::string, int> ret;
	auto lastColon = address.find_last_of(':');
	try
	{
		auto val = std::stoi(address.substr(lastColon + 1));
		ret.first = address.substr(0, lastColon);
		ret.second = val;
	}
	catch (const std::invalid_argument &)
	{
		ret = std::make_pair(address, -1);
	}
	return ret;
}

void ZmqComms::setBrokerPorts(int reqPort, int pushPort)
{
	if (rx_status == connection_status::startup)
	{
		brokerReqPort = reqPort;
		if (pushPort < 0)
		{
			if (brokerPushPort < 0)
			{
				brokerPushPort = reqPort + 1;
			}
		}
		else
		{
			brokerPushPort = pushPort;
		}
		
	}

}

std::pair<int, int> ZmqComms::findOpenPorts()
{
	int start = openPortStart;
	if (openPortStart < 0)
	{
		start = (hasBroker) ? BEGIN_OPEN_PORT_RANGE_SUBBROKER : BEGIN_OPEN_PORT_RANGE;
	}
	while (usedPortNumbers.find(start) != usedPortNumbers.end())
	{
		start += 2;
	}
	usedPortNumbers.insert(start);
	usedPortNumbers.insert(start + 1);
	return std::make_pair (start, start + 1);
}

void ZmqComms::setPortNumbers(int repPort, int pullPort)
{
	if (rx_status == connection_status::startup)
	{
		repPortNumber = repPort;
		auto currentPullPort = pullPortNumber.load();
		pullPortNumber = (pullPort > 0) ? pullPort : (currentPullPort<0)?repPort + 1: currentPullPort;
	}
	
}

void ZmqComms::setAutomaticPortStartPort(int startingPort)
{
	openPortStart = startingPort;
}

void ZmqComms::setReplyCallback(std::function<ActionMessage(ActionMessage &&)> callback)
{
	replyCallback = std::move(callback);
}

	void ZmqComms::queue_rx_function()
	{
		
		auto ctx = zmqContextManager::getContextPointer();
		zmq::socket_t pullSocket(ctx->getContext(), ZMQ_PULL);

		
		zmq::socket_t controlSocket(ctx->getContext(), ZMQ_PAIR);
		std::string controlsockString = std::string("inproc://") + name + "_control";
		controlSocket.bind(controlsockString.c_str());

		zmq::socket_t repSocket(ctx->getContext(), ZMQ_REP);
		while (pullPortNumber == -1)
		{
			zmq::message_t msg;
			controlSocket.recv(&msg);
			ActionMessage M(static_cast<char *>(msg.data()), msg.size());
			if (M.action() == CMD_PROTOCOL)
			{
				if (M.index == PORT_DEFINITIONS)
				{
					pullPortNumber = M.dest_id;
					repPortNumber = M.source_handle;
				}
				else if (M.index == DISCONNECT)
				{
					rx_status = connection_status::terminated;
					return;
				}
			}
		}
		try
		{
			pullSocket.bind(makePortAddress(localTarget_, pullPortNumber));
			repSocket.bind(makePortAddress(localTarget_, repPortNumber));
		}
		catch (const zmq::error_t &ze)
		{
			std::cerr << ze.what() << '\n';
			rx_status = connection_status::error;
			return;
		}
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
					if (M.action() == CMD_PROTOCOL)
					{
						switch (M.index)
						{
						case REQUEST_PORTS:
						{
							auto openPorts = findOpenPorts();
							ActionMessage portReply(CMD_PROTOCOL);
							portReply.index = PORT_DEFINITIONS;
							portReply.source_handle = openPorts.first;
							portReply.dest_id = openPorts.second;
							repSocket.send(portReply.to_string());
						}	
							break;
						default:
							M.index = NULL_REPLY;
							repSocket.send(M.to_string());
						}
						continue;
					}
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
				if (M.action() == CMD_PROTOCOL)
				{
					switch (M.index)
					{
					case CLOSE_RECEIVER:
						rx_status = connection_status::terminated;
						return;
					default:
						break;
					}
				}
				ActionCallback(std::move(M));
			}
		}
		rx_status = connection_status::terminated;
	}

	void ZmqComms::queue_tx_function()
	{
		std::vector<char> buffer;
		std::vector<char> rxbuffer(4096);
		auto ctx = zmqContextManager::getContextPointer();
		zmq::socket_t reqSocket(ctx->getContext(), ZMQ_REQ);

		//Setup the control socket for comms with the receiver
		zmq::socket_t controlSocket(ctx->getContext(), ZMQ_PAIR);

		std::string controlsockString = std::string("inproc://") + name + "_control";
		controlSocket.connect(controlsockString);

		if (!brokerTarget_.empty())
		{
			reqSocket.connect(makePortAddress(brokerTarget_,brokerReqPort));
			hasBroker = true;
			int cnt = 0;
			while (pullPortNumber == -1)
			{
				ActionMessage getPorts(CMD_PROTOCOL);
				getPorts.index = REQUEST_PORTS;
				auto str = getPorts.to_string();
				reqSocket.send(str);
				auto nsize = reqSocket.recv(rxbuffer.data(), rxbuffer.size());
				if ((nsize > 0) && (nsize < rxbuffer.size()))
				{
					ActionMessage rxcmd(rxbuffer.data(), rxbuffer.size());
					if (rxcmd.action() == CMD_PROTOCOL)
					{
						if (rxcmd.index == PORT_DEFINITIONS)
						{
							controlSocket.send(rxbuffer.data(), rxbuffer.size());
						}
						else if (rxcmd.index == DISCONNECT)
						{
							controlSocket.send(rxbuffer.data(), rxbuffer.size());
							tx_status = connection_status::terminated;
							return;
						}
					}
				}
				++cnt;
				if (cnt > 10)
				{
					//we can't get the broker to respond with port numbers
					tx_status = connection_status::error;
					return; 
				}
			}
		}
		else
		{
			if ((pullPortNumber == -1)||(repPortNumber==-1))
			{
				if (pullPortNumber == -1)
				{
					pullPortNumber = DEFAULT_BROKER_PULL_PORT_NUMBER;
				}

				if (repPortNumber == -1)
				{
					repPortNumber = DEFAULT_BROKER_REP_PORT_NUMBER;
				}
				ActionMessage setPorts(CMD_PROTOCOL);
				setPorts.index =PORT_DEFINITIONS;
				setPorts.dest_id=pullPortNumber;
				setPorts.source_handle=repPortNumber;
				auto str = setPorts.to_string();
				controlSocket.send(str);
			}
			
		}
		
		
		zmq::socket_t brokerPushSocket(ctx->getContext(), ZMQ_PUSH);
		std::map<int, zmq::socket_t> routes;  // for all the other possible routes
		std::map<int, zmq::socket_t> priority_routes;  //!< for priority message to different routes

		
		if (hasBroker)
		{
			brokerPushSocket.connect(makePortAddress(brokerTarget_, brokerPushPort));
		}
		
		tx_status = connection_status::connected;
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
						
						auto newroute = cmd.payload;
						auto splitPt = newroute.find_first_of(';');
						std::string priority_route;
						std::string push_route;
						if (splitPt != std::string::npos)
						{
							priority_route = newroute.substr(0, splitPt);
							push_route = newroute.substr(splitPt + 1);
						}
						else
						{
							push_route = newroute;
						}
						if (!priority_route.empty())
						{
							try
							{
								auto zsock = zmq::socket_t(ctx->getContext(), ZMQ_REQ);
								zsock.connect(priority_route);
								priority_routes.emplace(cmd.dest_id, std::move(zsock));
							}
							catch (const zmq::error_t &)
							{
								//TODO:: do something???
							}
						}
							try
							{
								auto zsock = zmq::socket_t(ctx->getContext(), ZMQ_PUSH);
								zsock.connect(push_route);
								routes.emplace(cmd.dest_id, std::move(zsock));
							}
							catch (const zmq::error_t &)
							{
								//TODO:: do something???
							}
						
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
				if ((cmd.dest_id == 0)&&(hasBroker))
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
					continue;
				}
				else
				{
					auto rt_find = priority_routes.find(route_id);
					if (rt_find != priority_routes.end())
					{
						rt_find->second.send(buffer.data(), buffer.size());
						auto nsize = rt_find->second.recv(rxbuffer.data(), rxbuffer.size());
						if ((nsize > 0) && (nsize < rxbuffer.size()))
						{
							ActionMessage rxcmd(rxbuffer.data(), rxbuffer.size());
							ActionCallback(std::move(rxcmd));
						}
						continue;
					}
					
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
			{ //send to rx thread loop
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
			//try connecting with the receivers push socket
			auto ctx = zmqContextManager::getContextPointer();
			zmq::socket_t pushSocket(ctx->getContext(), ZMQ_PUSH);
			pushSocket.connect(makePortAddress(localTarget_, pullPortNumber));
			ActionMessage cmd(CMD_PROTOCOL);
			cmd.index = CLOSE_RECEIVER;
			pushSocket.send(cmd.to_string());
		}
	}

	std::string ZmqComms::getPushAddress() const
	{
		return makePortAddress(localTarget_, pullPortNumber);
	}

	std::string ZmqComms::getRequestAddress() const
	{
		return makePortAddress(localTarget_, repPortNumber);
	}
}