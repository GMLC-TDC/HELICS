/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "test-core.h"
#include "TestBroker.h"

#include "ActionMessage.h"
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

namespace helics
{
using federate_id_t= Core::federate_id_t;
using Handle= Core::Handle;


TestCore::TestCore(std::shared_ptr<TestBroker> nbroker) : tbroker(std::move(nbroker))
{

}

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
		("brokerinit", po::value<int>(), "the initialization string for the broker")
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


void TestCore::initializeFromArgs (int argc, char *argv[])
{
	namespace po = boost::program_options;
	if (!_initialized)
	{
		po::variables_map vm;
		argumentParser(argc, argv, vm);

		if (vm.count("broker") > 0)
		{
			auto brstring = vm["broker"].as<std::string>();
			tbroker = findTestBroker(brstring);
		}
		if (!tbroker)
		{
			tbroker = std::make_shared<TestBroker>();
			

		}
		if (vm.count("brokerinit") > 0)
		{
			tbroker->Initialize(vm["brokerinit"].as<std::string>());
		}
		else
		{
			tbroker->Initialize("");
		}
		CommonCore::initializeFromArgs(argc, argv);
		if (vm.count("register") > 0)
		{
			registerTestCore(shared_from_this());
		}
	}	
}


TestCore::~TestCore() = default;

void TestCore::transmit(int route_id, const ActionMessage &cmd)
{
	if (route_id==0)
	{
		tbroker->addMessage(cmd);
		return;
	}
	if (_operating)
	{
		auto brkfnd = brokerRoutes.find(route_id);
		if (brkfnd != brokerRoutes.end())
		{
			brkfnd->second->addMessage(cmd);
			return;
		}
		auto crfnd = coreRoutes.find(route_id);
		if (crfnd != coreRoutes.end())
		{
			crfnd->second->addCommand(cmd);
			return;
		}
	}

	tbroker->addMessage(cmd);
	
}

void TestCore::addRoute(int route_id, const std::string & routeInfo)
{

	auto brk = findTestBroker(routeInfo);
	if (brk)
	{
		std::lock_guard<std::mutex> lock(routeMutex);
		brokerRoutes.emplace(route_id, std::move(brk));
		return;
	}
	auto tcore = findTestCore(routeInfo);
	if (tcore)
	{
		std::lock_guard<std::mutex> lock(routeMutex);
		coreRoutes.emplace(route_id, std::move(tcore));
		return;
	}
	//the route will default to the central route
}


static std::map<std::string, std::weak_ptr<TestCore>> TestCoreMap;

static std::map<std::string, std::weak_ptr<TestBroker>> TestBrokerMap;

static std::mutex mapLock;  //!<lock for the broker and core maps

std::shared_ptr<TestCore> findTestCore(const std::string &name)
{
	std::lock_guard<std::mutex> lock(mapLock);
	auto fnd = TestCoreMap.find(name);
	if (fnd != TestCoreMap.end())
	{
		return fnd->second.lock();
	}
	return nullptr;
}

std::shared_ptr<TestBroker> findTestBroker(const std::string &brokerName)
{
	std::lock_guard<std::mutex> lock(mapLock);
	auto fnd = TestBrokerMap.find(brokerName);
	if (fnd != TestBrokerMap.end())
	{
		return fnd->second.lock();
	}
	return nullptr;
}

void registerTestCore(std::shared_ptr<TestCore> tcore)
{
	std::lock_guard<std::mutex> lock(mapLock);
	TestCoreMap[tcore->getIdentifier()] = tcore;
}

void registerTestBroker(std::shared_ptr<TestBroker> tbroker)
{
	std::lock_guard<std::mutex> lock(mapLock);
	TestBrokerMap[tbroker->getIdentifier()] = tbroker;
}


/*
void TestCore::computeDependencies()
{
	for (auto &fed : _federates)
	{
		fed->generateKnownDependencies();
	}
	//TODO:: work in the additional rules for endpoints to reduce dependencies
	for (auto &fed : _federates)
	{
		if (fed->hasEndpoints)
		{
			for (auto &fedD : _federates)
			{
				if (fedD->hasEndpoints)
				{
					fed->addDependency(fedD->id);
					fedD->addDependent(fed->id);
				}
			}
		}
	}
}

*/

}  // namespace helics
