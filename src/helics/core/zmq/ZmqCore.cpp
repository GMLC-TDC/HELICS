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
#include "helics/core/zmq/ZmqCore.h"
#include "helics/common/zmqContextManager.h"
#include "helics/common/zmqSocketDescriptor.h"
#include "helics/common/zmqHelper.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <sstream>
#include <fstream>


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

static const std::string DEFAULT_BROKER = "tcp://localhost";
constexpr int defaultBrokerREPport = 23405;
constexpr int defaultBrokerPULLport = 23406; 


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
		("broker,b", po::value<std::string>(), "identifier for the broker")
		("port", po::value<int>(),"port number for the broker")
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


ZmqCore::ZmqCore(const std::string &core_name) :CommonCore(core_name) {}

void ZmqCore::initializeFromArgs(int argc, char *argv[])
{
	namespace po = boost::program_options;
	if (coreState==created)
	{
		po::variables_map vm;
		argumentParser(argc, argv, vm);

		if (vm.count("broker") > 0)
		{
			auto brstring = vm["broker"].as<std::string>();
			//tbroker = findTestBroker(brstring);
		}
		
		if (vm.count("brokerinit") > 0)
		{
			//tbroker->Initialize(vm["brokerinit"].as<std::string>());
		}
		CommonCore::initializeFromArgs(argc, argv);
	}
}

bool ZmqCore::brokerConnect()
{
	return true;
}

void ZmqCore::brokerDisconnect()
{
	
}

void ZmqCore::transmit(int route_id, const ActionMessage &cmd)
{
	txQueue.push(std::pair<int,ActionMessage>(route_id, cmd));
}

#define NEW_ROUTE 233

void ZmqCore::addRoute(int route_id, const std::string &routeInfo)
{
	ActionMessage rt(CMD_PROTOCOL);
	rt.payload = routeInfo;
	rt.index = NEW_ROUTE;
	rt.source_id = route_id;
	transmit(-1, rt);
}


std::string ZmqCore::getAddress() const
{
	return "";
}

void ZmqCore::transmitData()
{
	auto ctx = zmqContextManager::getContextPointer();
	zmq::socket_t reqSocket(ctx->getContext(), ZMQ_REQ);
	reqSocket.connect(brokerRepAddress.c_str());
	zmq::socket_t brokerPushSocket(ctx->getContext(), ZMQ_PUSH);
	std::map<int,zmq::socket_t> pushSockets; // for all the other possible routes
	while (1)
	{
		auto cmd = txQueue.pop();
		if (cmd.second.action() == CMD_PROTOCOL)
		{

		}

		if (isPriorityCommand(cmd.second))
		{

		}
		if (cmd.first == 0)
		{

		}
		else
		{

		}
	}
	reqSocket.close();
	brokerPushSocket.close();
	pushSockets.clear();
}

void receiveData()
{

}

}  // namespace helics
