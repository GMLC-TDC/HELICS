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
#include "helics/core/zmq/ZmqBroker.h"
#include "helics/core/zmq/ZmqComms.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <fstream>


#include "helics/core/argParser.h"

#define USE_LOGGING 1
#if USE_LOGGING
#if HELICS_HAVE_GLOG
#include <glog/logging.h>
#define ENDL ""
#else
#define LOG(LEVEL) std::cout
#define ENDL std::endl
#endif
#else
#define LOG(LEVEL) std::ostringstream()
#define ENDL std::endl
#endif


namespace helics
{


using namespace std::string_literals;
static const argDescriptors extraArgs
{
	{ "local_interface"s,"string"s,"the local interface to use for the receive ports"s },
	{ "brokerport"s, "int"s, "port number for the broker priority port"s },
	{ "brokerpushport"s, "int"s, "port number for the broker primary push port"s },
	{ "pullport"s,"int"s,"port number for the primary receive port"s },
	{ "repport"s,"int"s,"port number for the priority receive port"s },
	{ "port"s, "int"s, "port number for the broker's priority port"s },
	{"portstart"s,"int"s,"starting port for automatic port definitions"s}
};

ZmqBroker::ZmqBroker(bool rootBroker) noexcept:CoreBroker(rootBroker)
{}

ZmqBroker::ZmqBroker(const std::string &broker_name) :CoreBroker(broker_name) {}

ZmqBroker::~ZmqBroker() = default;

void ZmqBroker::InitializeFromArgs(int argc, char *argv[])
{
	namespace po = boost::program_options;
	if (brokerState == broker_state_t::created)
	{
		po::variables_map vm;
		argumentParser(argc, argv, vm,extraArgs);

		if (vm.count("broker") > 0)
		{
			auto brkprt = extractInterfaceandPort(vm["broker"].as<std::string>());
			brokerAddress = brkprt.first;
			brokerReqPort = brkprt.second;

			// tbroker = findTestBroker(brstring);
		}
		if (vm.count("local_interface") > 0)
		{
			auto localprt = extractInterfaceandPort(vm["local_interface"].as<std::string>());
			localInterface = localprt.first;
			repPortNumber = localprt.second;
		}
		if (vm.count("port") > 0)
		{
			brokerReqPort = vm["port"].as<int>();
		}
		if (vm.count("brokerport") > 0)
		{
			brokerReqPort = vm["brokerport"].as<int>();
		}
		if (vm.count("brokerpushport") > 0)
		{
			brokerPushPort = vm["brokerpushport"].as<int>();
		}
		if (vm.count("pullport") > 0)
		{
			pullPortNumber = vm["pullport"].as<int>();
		}
		if (vm.count("repport") > 0)
		{
			repPortNumber = vm["repport"].as<int>();
		}
		if (vm.count("portstart") > 0)
		{
			portStart = vm["portstart"].as<int>();
		}
		CoreBroker::InitializeFromArgs(argc, argv);
	}
}

bool ZmqBroker::brokerConnect()
{
	if (brokerAddress.empty())
	{
		setAsRoot();
	}
	comms = std::make_unique<ZmqComms>(localInterface, brokerAddress);
	comms->setCallback([this](ActionMessage M) {addCommand(std::move(M)); });
	comms->setName(getIdentifier());
	if ((repPortNumber > 0) || (pullPortNumber > 0))
	{
		comms->setPortNumbers(repPortNumber, pullPortNumber);
	}
	if ((brokerReqPort > 0) || (brokerPushPort > 0))
	{
		comms->setBrokerPorts(brokerReqPort, brokerPushPort);
	}
	
	if (portStart > 0)
	{
		comms->setAutomaticPortStartPort(portStart);
	}
	//comms->setMessageSize(maxMessageSize, maxMessageCount);
	auto res = comms->connect();
	if (res)
	{
		if (repPortNumber < 0)
		{
			repPortNumber = comms->getRequestPort();
		}
		if (pullPortNumber < 0)
		{
			pullPortNumber = comms->getPushPort();
		}
	}
	return res;
}

void ZmqBroker::brokerDisconnect()
{
	comms->disconnect();
}

void ZmqBroker::transmit(int route_id, const ActionMessage &cmd)
{
	comms->transmit(route_id, cmd);
}

void ZmqBroker::addRoute(int route_id, const std::string &routeInfo)
{
	comms->addRoute(route_id, routeInfo);
}


std::string ZmqBroker::getAddress() const
{
	return comms->getRequestAddress() + ";" + comms->getPushAddress();
}
}  // namespace helics
