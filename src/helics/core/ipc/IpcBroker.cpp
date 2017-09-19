/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "helics/config.h"
#include "helics/common/blocking_queue.h"
#include "helics/core/core.h"
#include "helics/core/core-data.h"
#include "helics/core/helics-time.h"
#include "IpcBroker.h"

#include "IpcComms.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <fstream>


#include "helics/core/argParser.h"

#include <boost/filesystem.hpp>


static const std::string DEFAULT_BROKER = "tcp://localhost:5555";

constexpr size_t maxMessageSize = 16 * 1024;

constexpr size_t maxMessageCount = 256;

#define CLOSE_IPC 23425
namespace helics
{

using namespace std::string_literals;
static const argDescriptors extraArgs
{
	{"queueloc"s, "string"s, "the named location of the shared queue"s},
	{ "brokerinit"s, "string"s, "the initialization string for the broker"s }
};

IpcBroker::IpcBroker(bool rootBroker) noexcept:CoreBroker(rootBroker)
{}

IpcBroker::IpcBroker(const std::string &broker_name) :CoreBroker(broker_name) {}

IpcBroker::~IpcBroker()
{
	
}

void IpcBroker::InitializeFromArgs(int argc, char *argv[])
{
	namespace po = boost::program_options;
	if (brokerState == broker_state_t::created)
	{
		po::variables_map vm;
		argumentParser(argc, argv, vm,extraArgs);

		if (vm.count("broker") > 0)
		{
			brokername = vm["broker"].as<std::string>();
		}

		if (vm.count("broker_address") > 0)
		{
			brokerloc = vm["broker_address"].as<std::string>();
		}

		if (vm.count("fileloc") > 0)
		{
			fileloc = vm["fileloc"].as<std::string>();
		}
		
		CoreBroker::InitializeFromArgs(argc, argv);
		if (getIdentifier().empty())
		{
			setIdentifier("_ipc_broker");
		}
	}
}

bool IpcBroker::brokerConnect()
{
	if (fileloc.empty())
	{
		fileloc = getIdentifier() + "_queue.hqf";
	}
	
	if ((brokerloc.empty()) && (brokername.empty()))
	{
		setAsRoot();
	}
	else if (brokerloc.empty())
	{
		brokerloc = brokername + "_queue.hqf";
	}
	comms = std::make_unique<IpcComms>(fileloc,brokerloc);
	comms->setCallback([this](ActionMessage M) {addActionMessage(std::move(M)); });
	comms->setMessageSize(maxMessageSize, maxMessageCount);
	return comms->connect();
}

void IpcBroker::brokerDisconnect()
{
	comms->disconnect();
}

void IpcBroker::transmit(int route_id, const ActionMessage &cmd)
{
	comms->transmit(route_id, cmd);
}

void IpcBroker::addRoute(int route_id, const std::string &routeInfo)
{
	comms->addRoute(route_id, routeInfo);
}


std::string IpcBroker::getAddress() const
{
	return fileloc;
}

}  // namespace helics
