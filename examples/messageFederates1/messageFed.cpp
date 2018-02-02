/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "helics/application_api/MessageFederate.hpp"
#include <iostream>
#include <thread>
#include "helics/core/BrokerFactory.hpp"
#include "helics/common/argParser.h"

static const helics::ArgDescriptors InfoArgs{
    {"startbroker","start a broker with the specified arguments"},
    {"target,t", "name of the target federate"}
};

int main (int argc, char *argv[])
{
    helics::FederateInfo fi("fed");
    helics::variable_map vm;
    auto parseResult = argumentParser(argc, argv, vm, InfoArgs);
    fi.loadInfoFromArgs(argc, argv);
    if (parseResult != 0)
    {
        return 0;
    }

	std::string targetfederate = "fed";
	if (vm.count("target") > 0)
	{
		targetfederate = vm["target"].as<std::string>();
	}
    std::string target = targetfederate + "/endpoint";
   
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
