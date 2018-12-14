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
#ifndef DISABLE_TCP_CORE
#include "../core/tcp/TcpComms.h"
#include "../core/tcp/TcpCommsSS.h"
#endif
#if HELICS_HAVE_ZEROMQ!=0
#include "../core/zmq/ZmqComms.h"
#endif
#if HELICS_HAVE_MPI!=0
#include "../core/mpi/MpiComms.h"
#endif
#include "../core/ipc/IpcComms.h"
#include "../common/argParser.h"
#include "../common/stringToCmdLine.h"
#include "../core/NetworkBrokerData.hpp"


using namespace std::string_literals;

namespace helics
{
static void loadTypeSpecificArgs(helics::core_type ctype, CommsInterface *comm, int argc, const char * const *argv)
{
	if (comm == nullptr)
	{
		return;
	}
	switch (ctype)
	{
#ifndef DISABLE_TCP_CORE
	case core_type::TCP_SS:
	{
		static const ArgDescriptors extraArgs{
			{ "connections"s, ArgDescriptor::arg_type_t::vector_string,
			"target link connections"s } };
		variable_map vm;
		argumentParser(argc, argv, vm, extraArgs);

		auto cm = dynamic_cast<tcp::TcpCommsSS *>(comm);
		if (vm.count("connections") > 0)
		{
			cm->addConnections(vm["connections"].as<std::vector<std::string>>());
		}
	}
		break;
#endif
	case core_type::MPI:
		break;
	default:
		break;
	}
}

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
#ifndef DISABLE_TCP_CORE
				comm = std::make_unique<tcp::TcpComms>();
#endif
			break;
        case core_type::DEFAULT:
		case core_type::ZMQ:
#if HELICS_HAVE_ZEROMQ!=0
			comm = std::make_unique<zeromq::ZmqComms>();
#endif
			break;
		case core_type::TCP_SS:
#ifndef DISABLE_TCP_CORE
			comm = std::make_unique<tcp::TcpCommsSS>();
            loadTypeSpecificArgs (ctype, comm.get (), cmdargs.getArgCount (), cmdargs.getArgV ());
#endif
			break;
		case core_type::UDP:
			comm = std::make_unique<udp::UdpComms>();
			break;
		case core_type::IPC:
		case core_type::INTERPROCESS:
			comm = std::make_unique<ipc::IpcComms>();
			break;
		case core_type::MPI:
#if HELICS_HAVE_MPI>0
			comm = std::make_unique<mpi::MpiComms>();
			break;
#endif
		case core_type::HTTP:
		case core_type::ZMQ_TEST:
        case core_type::TEST:
        case core_type::NNG:
        case core_type::UNRECOGNIZED:
			break;

	}
	if (comm)
	{
		comm->loadNetworkInfo(nbdata);
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


void MultiBroker::transmit(route_id rid, const ActionMessage &cmd)
{
	masterComm->transmit(rid, cmd);
}


void MultiBroker::transmit(route_id rid, ActionMessage &&cmd)
{
	masterComm->transmit(rid, std::move(cmd));
}


void MultiBroker::addRoute(route_id rid, const std::string &routeInfo)
{
	masterComm->addRoute(rid, routeInfo);
}

}  // namespace helics
