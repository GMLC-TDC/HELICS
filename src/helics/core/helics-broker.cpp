#include "helics/helics-config.h"

#include "BrokerFactory.hpp"
#include "CoreBroker.hpp"
#include "helicsVersion.hpp"
#include <fstream>
#include <iostream>
#include "../common/argParser.h"


static const helics::ArgDescriptors InfoArgs{
    {"name,n", "name of the broker"},
    {"type,t",  "type of the broker (\"(zmq)\", \"ipc\", \"test\", \"mpi\", \"test\", \"tcp\", \"udp\")"}
};

int main (int argc, char *argv[])
{
    helics::variable_map vm;
    auto exit_early = helics::argumentParser (argc, argv, vm,InfoArgs);

    if (exit_early != 0)
    {
        if (exit_early == helics::helpReturn)
        {
            helics::BrokerFactory::displayHelp ();
        }
        else if (exit_early == helics::versionReturn)
        {
            std::cout << helics::versionString << '\n';
        }
        return 0;
    }

    std::string name = (vm.count ("name") > 0) ? vm["name"].as<std::string> () : "";
    std::string btype = (vm.count ("type") > 0) ? vm["type"].as<std::string> () : "zmq";

    helics::core_type type;
    try
    {
        type = helics::coreTypeFromString (btype);
    }
    catch (std::invalid_argument &ie)
    {
        std::cerr << "Unable to generate broker: " << ie.what () << '\n';
        return (-2);
    }
    auto broker = helics::BrokerFactory::create (type,name, argc, argv);
    if (broker->isConnected ())
    {
        do  // sleep until the broker finishes
        {
            std::this_thread::sleep_for (std::chrono::milliseconds (500));

        } while (broker->isConnected ());
    }
    else
    {
        std::cerr << "Broker is unable to connect\n";
        return (-1);
    }
    broker = nullptr;
    while (helics::BrokerFactory::cleanUpBrokers () > 0)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (500));
    }
    return 0;
}

