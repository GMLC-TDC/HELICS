/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "helics/config.h"
#include "helics/core/core.h"
#include "helics/core/core-data.h"
#include "helics/core/helics-time.h"
#include "MpiCore.h"
#include "helics/core/core-exceptions.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <fstream>

#include "MpiComms.h"

#include "helics/core/argParser.h"
#include <boost/filesystem.hpp>



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

static const std::vector<std::tuple<std::string, std::string, std::string>> extraArgs
{
	{ "queueloc", "string", "the file location of the shared queue" },
	{ "brokerloc", "string", "the file location for the broker" },
	{ "broker_auto_start", "","automatically start the broker" },
	{ "broker_init", "string", "the init string to pass to the broker upon startup-will only be used if the autostart is activated" },
	{ "brokername", "string", "identifier for the broker-same as broker" },
	{ "brokerinit", "string", "the initialization string for the broker" }
};


MpiCore::MpiCore() noexcept
{}

MpiCore::MpiCore(const std::string &core_name) :CommonCore(core_name) {}


MpiCore::~MpiCore()
{

}

void MpiCore::initializeFromArgs(int argc, char *argv[])
{
	namespace po = boost::program_options;
	if (coreState == created)
	{
		po::variables_map vm;
		argumentParser(argc, argv, vm, extraArgs);

		if (vm.count("broker") > 0)
		{
			brokername = vm["broker"].as<std::string>();
		}

		if (vm.count("brokerloc") > 0)
		{
			brokerloc = vm["brokerloc"].as<std::string>();
		}

		if (vm.count("fileloc") > 0)
		{
			fileloc = vm["fileloc"].as<std::string>();
		}

		CommonCore::initializeFromArgs(argc, argv);
	}
}

bool MpiCore::brokerConnect()
{
	if (fileloc.empty())
	{
		fileloc = getIdentifier() + "_queue.hqf";
	}



	if (brokerloc.empty())
	{
		if (brokername.empty())
		{
			brokername = "_ipc";
		}
		brokerloc = brokername + "_queue.hqf";
	}

	comms = std::make_unique<MpiComms>(fileloc, brokerloc);
	comms->setCallback([this](ActionMessage M) {addCommand(std::move(M)); });
	return comms->connect();
}

void MpiCore::brokerDisconnect()
{
	comms->disconnect();
}

void MpiCore::transmit(int route_id, const ActionMessage &cmd)
{

	comms->transmit(route_id, cmd);

}

void MpiCore::addRoute(int route_id, const std::string &routeInfo)
{

	comms->addRoute(route_id, routeInfo);


}


std::string MpiCore::getAddress() const
{
	return fileloc;
}


}  // namespace helics

