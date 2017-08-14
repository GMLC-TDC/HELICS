/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "TestCore.h"
#include "BrokerFactory.h"
#include "CoreFactory.h"
#include "CoreBroker.h"
#include "ActionMessage.h"
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

namespace helics
{
using federate_id_t = Core::federate_id_t;
using Handle = Core::Handle;

TestCore::TestCore(const std::string &core_name) :CommonCore(core_name)
{}

TestCore::TestCore (std::shared_ptr<CoreBroker> nbroker) : tbroker (std::move (nbroker)) {}

static void argumentParser (int argc, char *argv[], boost::program_options::variables_map &vm_map)
{
    namespace po = boost::program_options;
    po::options_description cmd_only ("command line only");
    po::options_description config ("configuration");
    po::options_description hidden ("hidden");

    // clang-format off
	// input boost controls
	cmd_only.add_options()
		("help,h", "produce help message")
		("config-file", po::value<std::string>(), "specify a configuration file to use");


	config.add_options()
		("broker,b", po::value<std::string>(), "identifier for the broker")
		("brokerinit", po::value<int>(), "the initialization string for the broker");


    // clang-format on

    po::options_description cmd_line ("command line options");
    po::options_description config_file ("configuration file options");
    po::options_description visible ("allowed options");

    cmd_line.add (cmd_only).add (config);
    config_file.add (config);
    visible.add (cmd_only).add (config);

    // po::positional_options_description p;
    // p.add("input", -1);

    po::variables_map cmd_vm;
    try
    {
        po::store (po::command_line_parser (argc, argv).options (cmd_line).allow_unregistered ().run (), cmd_vm);
    }
    catch (std::exception &e)
    {
        std::cerr << e.what () << std::endl;
        throw (e);
    }

    po::notify (cmd_vm);

    // objects/pointers/variables/constants


    // program options control
    if (cmd_vm.count ("help") > 0)
    {
        std::cout << visible << '\n';
        return;
    }

    po::store (po::command_line_parser (argc, argv).options (cmd_line).allow_unregistered ().run (), vm_map);

    if (cmd_vm.count ("config-file") > 0)
    {
        std::string config_file_name = cmd_vm["config-file"].as<std::string> ();
        if (!boost::filesystem::exists (config_file_name))
        {
            std::cerr << "config file " << config_file_name << " does not exist\n";
            throw (std::invalid_argument ("unknown config file"));
        }
        else
        {
            std::ifstream fstr (config_file_name.c_str ());
            po::store (po::parse_config_file (fstr, config_file), vm_map);
            fstr.close ();
        }
    }

    po::notify (vm_map);
}


void TestCore::initializeFromArgs (int argc, char *argv[])
{
    namespace po = boost::program_options;
    if (!_initialized)
    {
        po::variables_map vm;
        argumentParser (argc, argv, vm);

        if (vm.count ("broker") > 0)
        {
            brokerName = vm["broker"].as<std::string> ();
        }
			if (vm.count("brokerinit") > 0)
			{
				brokerInitString=vm["brokerinit"].as<std::string>();
			}
        CommonCore::initializeFromArgs (argc, argv);

    }
}

bool TestCore::brokerConnect()
{
	if (!tbroker)
	{
		tbroker = findBroker(brokerName);
		if (!tbroker)
		{
			tbroker = BrokerFactory::create(helics_core_type::HELICS_TEST, brokerName, brokerInitString);
		}
		
	}
	if (tbroker)
	{
		tbroker->connect();
	}
	return static_cast<bool>(tbroker);
}

void TestCore::brokerDisconnect()
{
	tbroker = nullptr;
}

TestCore::~TestCore () = default;

void TestCore::transmit (int route_id, const ActionMessage &cmd)
{
    if (route_id == 0)
    {
        tbroker->addMessage (cmd);
        return;
    }
    auto lock = (_operating) ? std::unique_lock<std::mutex> (routeMutex, std::defer_lock) :
                               std::unique_lock<std::mutex> (routeMutex);
    auto brkfnd = brokerRoutes.find (route_id);
    if (brkfnd != brokerRoutes.end ())
    {
        brkfnd->second->addMessage (cmd);
        return;
    }
    auto crfnd = coreRoutes.find (route_id);
    if (crfnd != coreRoutes.end ())
    {
        crfnd->second->addCommand (cmd);
        return;
    }

    tbroker->addMessage (cmd);
}

void TestCore::addRoute (int route_id, const std::string &routeInfo)
{
    auto brk = findBroker (routeInfo);
    if (brk)
    {
        std::lock_guard<std::mutex> lock (routeMutex);
        brokerRoutes.emplace (route_id, std::move (brk));
        return;
    }
    auto tcore = findCore (routeInfo);
    if (tcore)
    {
        std::lock_guard<std::mutex> lock (routeMutex);
        coreRoutes.emplace (route_id, std::move (tcore));
        return;
    }
    // the route will default to the central route
}


std::string TestCore::getAddress() const
{
	return getIdentifier();
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
