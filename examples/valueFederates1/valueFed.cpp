/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "helics/application_api/ValueFederate.hpp"
#include <thread>
#include <iostream>
#include "helics/core/BrokerFactory.hpp"
#include "helics/common/argParser.h"

static const helics::ArgDescriptors InfoArgs{
    { "startbroker","start a broker with the specified arguments" },
    { "valuetarget", "name of the target federate, same as target" },
    { "target,t", "name of the target federate" }
};

int main (int argc, const char * const *argv)
{
    helics::FederateInfo fi("fed");
    helics::variable_map vm;
    auto parseResult = argumentParser(argc, argv, vm, InfoArgs);
    fi.loadInfoFromArgs(argc, argv);
    if (parseResult != 0)
    {
        return 0;
    }

	fi.logLevel = 5;
    std::shared_ptr<helics::Broker> brk;
    if (vm.count("startbroker") > 0)
    {
        brk = helics::BrokerFactory::create(fi.coreType, vm["startbroker"].as<std::string>());
    }

    std::string target = "fed";
    if (vm.count("target") > 0)
    {
        target = vm["target"].as<std::string>();
    }
    if (vm.count("valuetarget") > 0)
    {
        target = vm["valuetarget"].as<std::string>();
    }
    auto vFed = std::make_unique<helics::ValueFederate> (fi);

    auto id = vFed->registerPublication ("pub", "double");

    auto subid = vFed->registerOptionalSubscription(target + "/pub","double");
    std::cout << "entering init State\n";
    vFed->enterInitializationState ();
    std::cout << "entered init State\n";
    vFed->enterExecutionState ();
    std::cout << "entered exec State\n";
    for (int i=1; i<10; ++i) {
        vFed->publish(id, i);
        auto newTime = vFed->requestTime (i);
        if (vFed->isUpdated(subid))
        {
            auto val = vFed->getValue<double>(subid);
            std::cout << "received updated value of " << val << " at "<< newTime << " from " << vFed->getSubscriptionKey(subid) << '\n';
        }
        
        std::cout << "processed time " << static_cast<double> (newTime) << "\n";
    }
    vFed->finalize ();
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

