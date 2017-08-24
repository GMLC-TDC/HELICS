/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "helics/config.h"
#include "helics/core/core.h"
#include "helics/core/core-data.h"
#include "helics/core/helics-time.h"
#include "helics/core/ipc/IpcCore.h"
#include "helics/core/core-exceptions.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <fstream>


#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#define CLOSE_IPC 23425215

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

constexpr size_t maxMessageSize = 16 * 1024;

constexpr size_t maxMessageCount = 1024;

using ipc_queue = boost::interprocess::message_queue;

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
		("brokerloc", po::value<int>(), "the file location for the broker")
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


IpcCore::IpcCore(const std::string &core_name) :CommonCore(core_name) {}


IpcCore::~IpcCore()
{
	if (queue_watcher.joinable())
	{
		ActionMessage cmd(CMD_PROTOCOL);
		cmd.index = CLOSE_IPC;
		transmit(-1, cmd);
		queue_watcher.join();
		ipc_queue::remove(fileloc.c_str());
	}
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
		auto tempPath = boost::filesystem::temp_directory_path();
		auto tname = tempPath / (getIdentifier() + "_queue.hqf");
		fileloc = tname.string();
	}

		try
		{
			rxQueue = std::make_unique<ipc_queue>(boost::interprocess::create_only, fileloc.c_str(), maxMessageCount, maxMessageSize);
		}
		catch (boost::interprocess::interprocess_exception const& ipe)
		{
			throw(connectionFailure("failed open receive queue"));
		}
	queue_watcher = std::thread(&IpcCore::queue_rx_function, this);
	if (brokerloc.empty())
	{
		auto tempPath = boost::filesystem::temp_directory_path();
		if (brokername.empty())
		{
			brokername = "_ipc";
		}
		auto tname = tempPath / (brokername + "_queue.hqf");
		brokerloc = tname.string();
	}
	int sleep_counter = 50;
	while (!boost::filesystem::exists(brokerloc))
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(sleep_counter));
		sleep_counter *= 2;
		if (sleep_counter > 1700)
		{
			break;
		}
	}
		try
		{
			brokerQueue = std::make_unique<ipc_queue>(boost::interprocess::open_only, brokerloc.c_str());
			return true;
		}
		catch (boost::interprocess::interprocess_exception const& ipe)
		{
			
			return false;
		}

}

void IpcCore::brokerDisconnect()
{
	if (queue_watcher.joinable())
	{
		ActionMessage cmd(CMD_PROTOCOL);
		cmd.index = CLOSE_IPC;
		transmit(-1, cmd);
		queue_watcher.join();
		ipc_queue::remove(fileloc.c_str());
	}
	brokerQueue = nullptr;
	rxQueue = nullptr;
	ipc_queue::remove(fileloc.c_str());
}

void IpcCore::transmit(int route_id, const ActionMessage &cmd)
{
	std::string buffer = cmd.to_string();
	if (route_id == 0)
	{
		brokerQueue->send(buffer.data(), buffer.size(), 1);
	}
	else if (route_id == -1)
	{
		rxQueue->send(buffer.data(), buffer.size(), 1);
	}
	else
	{
		auto routeFnd = routes.find(route_id);
		if (routeFnd != routes.end())
		{
			routeFnd->second->send(buffer.data(), buffer.size(), 1);
		}
	}
}

void IpcCore::addRoute(int route_id, const std::string &routeInfo)
{
	if (boost::filesystem::exists(routeInfo))
	{
		auto newQueue = std::make_unique<ipc_queue>(boost::interprocess::open_only, routeInfo.c_str());
		routes.emplace(route_id, std::move(newQueue));
	}
}


std::string IpcCore::getAddress() const
{
	return fileloc;
}

void IpcCore::queue_rx_function()
{
	unsigned int priority;
	size_t rx_size;
	char buffer[maxMessageSize];

	while (1)
	{
		rxQueue->receive(buffer, maxMessageSize, rx_size, priority);
		ActionMessage cmd(buffer, rx_size);
		if ((cmd.action() == CMD_PROTOCOL) || (cmd.action() == CMD_PROTOCOL_BIG))
		{
			if (cmd.index == CLOSE_IPC)
			{
				return;
			}
			continue;
		}

		addCommand(cmd);
	}
}

}  // namespace helics
