/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "helics/application_api/MessageFederate.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <set>
#include <thread>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/helicsVersion.hpp"

namespace po = boost::program_options;
namespace filesystem = boost::filesystem;

static bool argumentParser (int argc, const char * const *argv, po::variables_map &vm_map);


int main (int argc, char *argv[])
{
    po::variables_map vm;
    if (argumentParser (argc, argv, vm)) {
        return 0;
    }

	std::string targetfederate = "fed";
	if (vm.count("target") > 0)
	{
		targetfederate = vm["target"].as<std::string>();
	}
    std::string target = targetfederate + "/endpoint";
    helics::FederateInfo fi("fed");
    fi.loadInfoFromArgs(argc, argv);
    fi.logLevel = 5;
    std::shared_ptr<helics::Broker> brk;
    if (vm.count("startbroker") > 0)
    {
        brk = helics::BrokerFactory::create(fi.coreType, vm["startbroker"].as<std::string>());
    }
	
    auto mFed = std::make_unique<helics::MessageFederate> (fi);
    auto name = mFed->getName();
	std::cout << " registering endpoint for " << name<<'\n';
    auto id = mFed->registerEndpoint("endpoint", "");

    std::cout << "entering init State\n";
    mFed->enterInitializationState ();
    std::cout << "entered init State\n";
    mFed->enterExecutionState ();
    std::cout << "entered exec State\n";
    for (int i=1; i<10; ++i) {
		std::string message = "message sent from "+name+" to "+target+" at time " + std::to_string(i);
		mFed->sendMessage(id, target, message.data(), message.size());
        auto newTime = mFed->requestTime (i);
		std::cout << "processed time " << static_cast<double> (newTime) << "\n";
		while (mFed->hasMessage(id))
		{
			auto nmessage = mFed->getMessage(id);
			std::cout << "received message from " << nmessage->source << " at " << static_cast<double>(nmessage->time) << " ::" << nmessage->data.to_string() << '\n';
		}
        
    }
    mFed->finalize ();
    if (brk)
    {
        while (brk->isConnected())
        {
            std::this_thread::yield();
        }
        brk = nullptr;
    }
    return 0;
}

bool argumentParser (int argc, const char * const *argv, po::variables_map &vm_map)
{
    po::options_description opt ("options");

    // clang-format off
    // input boost controls
    opt.add_options()
        ("help,h", "produce help message")
        ("startbroker", po::value<std::string>(),"start a broker with the specified arguments")
        ("target,t", po::value<std::string>(), "name of the target federate");

    // clang-format on

    po::variables_map cmd_vm;
    try
    {
        po::store (po::command_line_parser (argc, argv).options (opt).allow_unregistered().run (), cmd_vm);
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
        std::cout << opt << '\n';
        return true;
    }

    if (cmd_vm.count ("version") > 0)
    {
		std::cout << helics::helicsVersionString () << '\n';
        return true;
    }

    po::store (po::command_line_parser (argc, argv).options (opt).allow_unregistered().run (), vm_map);

    po::notify (vm_map);

    return false;
}
