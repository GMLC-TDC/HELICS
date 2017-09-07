/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "helics/config.h"
#include "helics/core/core.h"
#include "helics/core/core-data.h"
#include "helics/core/helics-time.h"
#include "IpcCore.h"
#include "helics/core/core-exceptions.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <fstream>

#include "IpcComms.h"

#include <boost/program_options.hpp>
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

static void argumentParser(int argc, char *argv[], boost::program_options::variables_map &vm_map)
{
	namespace po = boost::program_options;
	po::options_description cmd_only("command line only");
	po::options_description config("configuration");
	po::options_description hidden("hidden");

	// clang-format off
	// input boost controls
	cmd_only.add_options()
		("help,h", "produce help message")
		("config-file", po::value<std::string>(), "specify a configuration file to use");


	config.add_options()
		("queueloc", po::value<std::string>(), "the file location of the shared queue")
		("broker,b", po::value<std::string>(), "identifier for the broker")
		("brokerloc", po::value<std::string>(), "the file location for the broker")
		("broker_auto_start","automatically start the broker")
		("broker_init",po::value<std::string>(),"the init string to pass to the broker upon startup-will only be used if the autostart is activated")
		("register", "register the core for global locating");


	// clang-format on

	po::options_description cmd_line("command line options");
	po::options_description config_file("configuration file options");
	po::options_description visible("allowed options");

	cmd_line.add(cmd_only).add(config);
	config_file.add(config);
	visible.add(cmd_only).add(config);

	//po::positional_options_description p;
	//p.add("input", -1);

	po::variables_map cmd_vm;
	try
	{
		po::store(po::command_line_parser(argc, argv).options(cmd_line).allow_unregistered().run(), cmd_vm);
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		throw (e);
	}

	po::notify(cmd_vm);

	// objects/pointers/variables/constants


	// program options control
	if (cmd_vm.count("help") > 0)
	{
		std::cout << visible << '\n';
		return;
	}

	po::store(po::command_line_parser(argc, argv).options(cmd_line).allow_unregistered().run(), vm_map);

	if (cmd_vm.count("config-file") > 0)
	{
		std::string config_file_name = cmd_vm["config-file"].as<std::string>();
		if (!boost::filesystem::exists(config_file_name))
		{
			std::cerr << "config file " << config_file_name << " does not exist\n";
			throw (std::invalid_argument("unknown config file"));
		}
		else
		{
			std::ifstream fstr(config_file_name.c_str());
			po::store(po::parse_config_file(fstr, config_file), vm_map);
			fstr.close();
		}
	}

	po::notify(vm_map);
}

IpcCore::IpcCore() noexcept
{}

IpcCore::IpcCore(const std::string &core_name) :CommonCore(core_name) {}


IpcCore::~IpcCore()
{
	
}

void IpcCore::initializeFromArgs(int argc, char *argv[])
{
	namespace po = boost::program_options;
	if (coreState==created)
	{
		po::variables_map vm;
		argumentParser(argc, argv, vm);

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

bool IpcCore::brokerConnect()
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
	
	comms = std::make_unique<IpcComms>(fileloc,brokerloc );
	comms->setCallback([this](ActionMessage M) {addCommand(std::move(M)); });
	return comms->connect();
}

void IpcCore::brokerDisconnect()
{
	comms->disconnect();
}

void IpcCore::transmit(int route_id, const ActionMessage &cmd)
{
	
	comms->transmit(route_id, cmd);
	
}

void IpcCore::addRoute(int route_id, const std::string &routeInfo)
{
	
	comms->addRoute(route_id, routeInfo);
	
	
}


std::string IpcCore::getAddress() const
{
	return fileloc;
}


}  // namespace helics
