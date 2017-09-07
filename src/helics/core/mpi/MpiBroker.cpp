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
#include "MpiBroker.h"
#include "MpiComms.h"

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

static const std::string DEFAULT_BROKER = "tcp://localhost:5555";



namespace helics
{


static const std::vector<std::tuple<std::string, std::string, std::string>> extraArgs
{
	{ "brokerinit", "string", "the initialization string for the broker" }
};

MpiBroker::MpiBroker(bool rootBroker) noexcept:CoreBroker(rootBroker)
{}

MpiBroker::MpiBroker(const std::string &broker_name) : CoreBroker(broker_name) {}

MpiBroker::~MpiBroker() = default;

void MpiBroker::InitializeFromArgs(int argc, char *argv[])
{
	namespace po = boost::program_options;
	if (brokerState == broker_state_t::created)
	{
		po::variables_map vm;
		argumentParser(argc, argv, vm, extraArgs);

		if (vm.count("broker") > 0)
		{
			auto brstring = vm["broker"].as<std::string>();
			//tbroker = findTestBroker(brstring);
		}

		if (vm.count("brokerinit") > 0)
		{
			//tbroker->Initialize(vm["brokerinit"].as<std::string>());
		}
		CoreBroker::InitializeFromArgs(argc, argv);
	}
}

bool MpiBroker::brokerConnect()
{

	comms = std::make_unique<MpiComms>("", "");
	comms->setCallback([this](ActionMessage M) {addCommand(std::move(M)); });
	//comms->setMessageSize(maxMessageSize, maxMessageCount);
	return comms->connect();
}

void MpiBroker::brokerDisconnect()
{
	comms->disconnect();
}

void MpiBroker::transmit(int route_id, const ActionMessage &cmd)
{
	comms->transmit(route_id, cmd);
}

void MpiBroker::addRoute(int route_id, const std::string &routeInfo)
{
	comms->addRoute(route_id, routeInfo);
}


std::string MpiBroker::getAddress() const
{
	return "";
}
}  // namespace helics
