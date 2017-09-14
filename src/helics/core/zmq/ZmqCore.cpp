/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "helics/core/zmq/ZmqCore.h"

#include "helics/config.h"
#include "helics/core/core-data.h"
#include "helics/core/core.h"
#include "helics/core/helics-time.h"
#include "helics/core/zmq/ZmqComms.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <sstream>


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
#define LOG(LEVEL) std::ostringstream ()
#define ENDL std::endl
#endif

static const std::string DEFAULT_BROKER = "tcp://localhost";
constexpr int defaultBrokerREPport = 23405;
constexpr int defaultBrokerPULLport = 23406;


namespace helics
{
using namespace std::string_literals;
static const argDescriptors extraArgs
{
	{ "local_interface"s,"string"s,"the local interface to use for the receive ports"s },
	{ "brokerport"s, "int"s, "port number for the broker priority port"s },
	{"pullport"s,"int"s,"port number for the primary receive port"s},
	{"repport"s,"int"s,"port number for the priority receive port"s},
	{ "port"s, "int"s, "port number for the broker's priority port"s },
};



ZmqCore::ZmqCore() noexcept
{}

ZmqCore::~ZmqCore() = default;

ZmqCore::ZmqCore (const std::string &core_name) : CommonCore (core_name) {}

void ZmqCore::initializeFromArgs (int argc, char *argv[])
{
    namespace po = boost::program_options;
    if (coreState == created)
    {
        po::variables_map vm;
        argumentParser (argc, argv, vm,extraArgs);

        if (vm.count ("broker") > 0)
        {
           auto brkprt=extractInterfaceandPort(vm["broker"].as<std::string> ());
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
		else
		{
			localInterface = "tcp://127.0.0.1";
		}
		if (vm.count("port") > 0)
		{
			brokerReqPort = vm["port"].as<int>();
		}
		if (vm.count("brokerport") > 0)
		{
			brokerReqPort = vm["brokerport"].as<int>();
		}
		if (vm.count("pullport") > 0)
		{
			pullPortNumber = vm["pullport"].as<int>();
		}
		if (vm.count("repport") > 0)
		{
			repPortNumber = vm["repport"].as<int>();
			if (pullPortNumber < 0)
			{
				if (repPortNumber > 0)
				{
					pullPortNumber = repPortNumber + 1;
				}
			}
		}
        
        CommonCore::initializeFromArgs (argc, argv);
    }
}

bool ZmqCore::brokerConnect () 
{ 
	if (brokerAddress.empty()) //cores require a broker
	{
		brokerAddress = "tcp://127.0.0.1";
	}
	comms = std::make_unique<ZmqComms>(localInterface, brokerAddress);
	comms->setCallback([this](ActionMessage M) {addCommand(std::move(M)); });
	comms->setName(getIdentifier());
	if (repPortNumber > 0)
	{
		comms->setPortNumbers(repPortNumber, pullPortNumber);
	}
	comms->setBrokerPorts(brokerReqPort, brokerPushPort);
	return comms->connect();
}


void ZmqCore::brokerDisconnect ()
{
	comms->disconnect();
}

void ZmqCore::transmit (int route_id, const ActionMessage &cmd)
{
	comms->transmit(route_id, cmd);
}


void ZmqCore::addRoute (int route_id, const std::string &routeInfo)
{
	comms->addRoute(route_id, routeInfo);
}


std::string ZmqCore::getAddress () const 
{ 
	return comms->getRequestAddress() + ";" + comms->getPushAddress();
}



}  // namespace helics
