/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "MultiBroker.hpp"
#include "../core/CommsInterface.hpp"
#include <atomic>
#include <mutex>
#include <thread>
#include "../core/udp/UdpComms.h"
#include "../core/tcp/TcpComms.h"
#include "../core/tcp/TcpCommsSS.h"
#include "../core/zmq/ZmqComms.h"
#include "../core/ipc/IpcComms.h"
#include "../common/argParser.h"
#include "../common/stringToCmdLine.h"
#include "../core/NetworkBrokerData.hpp"

namespace helics
{

static std::unique_ptr<CommsInterface> generateComms(const std::string &type, const std::string &initString = std::string())
{
	auto ctype = coreTypeFromString(type);
	StringToCmdLine cmdargs(initString);
	NetworkBrokerData nbdata;
	nbdata.initializeFromArgs(cmdargs.getArgCount(), cmdargs.getArgV(), "localhost");
	std::unique_ptr<CommsInterface> comm;
	switch (ctype)
	{
		case core_type::TCP:
			comm = std::make_unique<tcp::TcpComms>();
			comm->loadNetworkInfo(nbdata);
			break;
		case core_type::ZMQ:
			comm = std::make_unique<zeromq::ZmqComms>();
			comm->loadNetworkInfo(nbdata);
			break;
		case core_type::TCP_SS:
			comm = std::make_unique<tcp::TcpCommsSS>();
			comm->loadNetworkInfo(nbdata);
			break;
		case core_type::UDP:
			comm = std::make_unique<udp::UdpComms>();
			comm->loadNetworkInfo(nbdata);
			break;
		case core_type::IPC:
			comm = std::make_unique<ipc::IpcComms>();
			break;
		case core_type::MPI:
		case core_type::HTTP:
		case core_type::ZMQ_TEST:
			break;

	}
	return comm;
}

MultiBroker::MultiBroker() noexcept
{
	loadComms();
}

MultiBroker::MultiBroker(int /*argc*/, char * /*argv*/ [])
{
	loadComms();
}


MultiBroker::MultiBroker(const std::string & /*configFile*/)
{
	loadComms();
}

void MultiBroker::loadComms()
{
	masterComm = generateComms("def");
	masterComm->setCallback([this](ActionMessage &&M) { BrokerBase::addActionMessage(std::move(M)); });
}


MultiBroker::~MultiBroker()
{
	BrokerBase::haltOperations = true;
	int exp = 2;
	while (!disconnectionStage.compare_exchange_weak(exp, 3))
	{
		if (exp == 0)
		{
			commDisconnect();
			exp = 1;
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	}
	masterComm = nullptr;  // need to ensure the comms are deleted before the callbacks become invalid
	BrokerBase::joinAllThreads();
}

void MultiBroker::brokerDisconnect()
{
	commDisconnect();
}


void MultiBroker::commDisconnect()
{
	int exp = 0;
	if (disconnectionStage.compare_exchange_strong(exp, 1))
	{
		masterComm->disconnect();
		disconnectionStage = 2;
	}
}


bool MultiBroker::tryReconnect()
{
	return masterComm->reconnect();
}


void MultiBroker::transmit(int route_id, const ActionMessage &cmd)
{
	masterComm->transmit(route_id, cmd);
}


void MultiBroker::transmit(int route_id, ActionMessage &&cmd)
{
	masterComm->transmit(route_id, std::move(cmd));
}


void MultiBroker::addRoute(int route_id, const std::string &routeInfo)
{
	masterComm->addRoute(route_id, routeInfo);
}

}  // namespace helics
