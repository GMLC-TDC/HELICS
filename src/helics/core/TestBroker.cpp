/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "TestBroker.h"
#include "TestCore.h"
#include "BrokerFactory.h"
#include "CoreFactory.h"

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <fstream>

namespace helics
{

TestBroker::TestBroker(bool rootBroker) noexcept:CoreBroker(rootBroker)
{}

TestBroker::TestBroker(const std::string &broker_name) :CoreBroker(broker_name)
{}

TestBroker::TestBroker (std::shared_ptr<TestBroker> nbroker) : tbroker (std::move (nbroker)) {}

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


void TestBroker::InitializeFromArgs (int argc, char *argv[])
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
      
        if (vm.count ("brokerinit") > 0)
        {
			brokerInitString=vm["brokerinit"].as<std::string> ();
        }
        CoreBroker::InitializeFromArgs (argc, argv);
       
		
    };
}


bool TestBroker::brokerConnect()
{
	if (!tbroker)
	{
		if (_isRoot)
		{
			return true;
		}
		if ((brokerName.empty()) && (brokerInitString.empty()))
		{
			_isRoot = true;
			return true;
		}
		else
		{
			tbroker = findBroker(brokerName);
			if (!tbroker)
			{
				tbroker = BrokerFactory::create(helics_core_type::HELICS_TEST, brokerName, brokerInitString);
			}
		}
		
	}

	return static_cast<bool>(tbroker);
}

void TestBroker::brokerDisconnect()
{
	if (!_isRoot)
	{
		tbroker = nullptr;
	}
}

void TestBroker::transmit (int32_t route_id, const ActionMessage &cmd)
{
    if ((!_isRoot) && (route_id == 0))
    {
		
        tbroker->addMessage (cmd);
        return;
    }
    // only activate the lock if we not in an operating state
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

    if (!_isRoot)
    {
        tbroker->addMessage (cmd);
    }
}

void TestBroker::addRoute (int route_id, const std::string &routeInfo)
{
    auto brk = findBroker (routeInfo);
    if (brk)
    {
        std::lock_guard<std::mutex> lock (routeMutex);
        brokerRoutes.emplace (route_id, std::move (brk));
        return;
    }
    auto tcore = CoreFactory::findCore (routeInfo);
    if (tcore)
    {
        std::lock_guard<std::mutex> lock (routeMutex);
        coreRoutes.emplace (route_id, std::move (tcore));
        return;
    }
    // the route will default to the central route
}


std::string TestBroker::getAddress() const
{
	return getIdentifier();
}

}  // namespace helics