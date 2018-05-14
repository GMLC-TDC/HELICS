/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "helics/helics-config.h"
#include "BrokerApp.hpp"
#include "../common/argParser.h"
#include "../core/BrokerFactory.hpp"
#include "../core/CoreBroker.hpp"
#include "../core/helicsVersion.hpp"
#include "../core/core-exceptions.hpp"
#include <fstream>
#include <iostream>

static const helics::ArgDescriptors InfoArgs{
    { "name,n", "name of the broker" },
    { "type,t", "type of the broker (\"(zmq)\", \"ipc\", \"test\", \"mpi\", \"test\", \"tcp\", \"udp\")" } };

namespace helics
{
namespace apps
{
BrokerApp::BrokerApp(int argc, char *argv[])
{
    helics::variable_map vm;
    auto exit_early = helics::argumentParser(argc, argv, vm, InfoArgs);

    if (exit_early != 0)
    {
        if (exit_early == helics::helpReturn)
        {
            helics::BrokerFactory::displayHelp();
        }
        else if (exit_early == helics::versionReturn)
        {
            std::cout << helics::versionString << '\n';
        }
        return;
    }

    std::string name = (vm.count("name") > 0) ? vm["name"].as<std::string>() : "";
    std::string btype = (vm.count("type") > 0) ? vm["type"].as<std::string>() : "zmq";

    helics::core_type type;
    try
    {
        type = coreTypeFromString(btype);
    }
    catch (std::invalid_argument &ie)
    {
        std::cerr << "Unable to generate broker from specified type: " << ie.what() << '\n';
        throw;
    }
    broker = BrokerFactory::create(type, name, argc, argv);
    if (!broker->isConnected())
    {
        throw(ConnectionFailure("Broker is unable to connect\n"));
    }
}

BrokerApp::~BrokerApp()
{
    if (!broker)
    {
        return;
    }
    if (broker->isConnected())
    {
        do  // sleep until the broker finishes
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

        } while (broker->isConnected());
    }
    broker = nullptr;
    helics::BrokerFactory::cleanUpBrokers(500);
}

/** run the Echo federate until the specified time
@param stopTime_input the desired stop time
*/
/** check if the Broker is running*/
bool BrokerApp::isActive() const
{
    return ((broker) && (broker->isConnected()));
}

/** forceably disconnect the broker*/
void BrokerApp::forceTerminate()
{
    if (!broker)
    {
        return;
    }
    if (broker->isConnected())
    {
        broker->disconnect();
    }
}

} // namespace apps
} //namespace helics
