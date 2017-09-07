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

static const argDescriptors extraArgs
{
	{ "brokerport", "int", "port number for the broker" },
	{ "port", "int", "port number for the broker" },
};



ZmqCore::ZmqCore() noexcept
{};

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
            auto brstring = vm["broker"].as<std::string> ();
            // tbroker = findTestBroker(brstring);
        }

        if (vm.count ("brokerinit") > 0)
        {
            // tbroker->Initialize(vm["brokerinit"].as<std::string>());
        }
        CommonCore::initializeFromArgs (argc, argv);
    }
}

bool ZmqCore::brokerConnect () 
{ 
	comms = std::make_unique<ZmqComms>(getIdentifier(), brokerAddress);
	comms->setCallback([this](ActionMessage M) {addCommand(std::move(M)); });
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


std::string ZmqCore::getAddress () const { return pullSocketAddress; }



}  // namespace helics
