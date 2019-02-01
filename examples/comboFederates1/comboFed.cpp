/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. 
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helics/application_api/CombinationFederate.hpp"
#include "helics/apps/BrokerApp.hpp"
#include "helics/common/argParser.h"
#include "helics/core/helics_definitions.hpp"
#include <iostream>
#include <thread>

static const helics::ArgDescriptors InfoArgs{
  {"startbroker", "start a broker with the specified arguments"},
  {"target,t", "name of the target federate"},
  {"valuetarget", "name of the value federate to target"},
  {"messgetarget", "name of the message federate to target"},
  {"endpoint,e", "name of the target endpoint"},
  {"source,s", "name of the source endpoint"}
  // name is captured in the argument processor for federateInfo
};

int main (int argc, char *argv[])
{
    helics::variable_map vm;
    auto parseResult = argumentParser (argc, argv, vm, InfoArgs);
    helics::FederateInfo fi;
    fi.defName = "fed";
    fi.loadInfoFromArgs (argc, argv);
    if (parseResult != 0)
    {
        return 0;
    }

    std::string vtarget = "fed";
    std::string mtarget = "fed";
    if (vm.count ("target") > 0)
    {
        mtarget = vm["target"].as<std::string> ();
        vtarget = mtarget;
    }
    if (vm.count ("valuetarget") > 0)
    {
        vtarget = vm["valuetarget"].as<std::string> ();
    }
    if (vm.count ("messagetarget") > 0)
    {
        mtarget = vm["messagetarget"].as<std::string> ();
    }
    std::string targetEndpoint = "endpoint";
    if (vm.count ("endpoint") > 0)
    {
        targetEndpoint = vm["endpoint"].as<std::string> ();
    }
    std::string etarget = mtarget + "/" + targetEndpoint;
    std::string myendpoint = "endpoint";
    if (vm.count ("source") > 0)
    {
        myendpoint = vm["source"].as<std::string> ();
    }
    fi.setProperty (helics::defs::properties::log_level, 5);
    helics::apps::BrokerApp brk;
    if (vm.count ("startbroker") > 0)
    {
        brk = helics::apps::BrokerApp (fi.coreType, vm["startbroker"].as<std::string> ());
    }

    auto cFed = std::make_unique<helics::CombinationFederate> (std::string (), fi);
    auto name = cFed->getName ();
    std::cout << " registering endpoint '" << myendpoint << "' for " << name << '\n';

    // this line actually creates an endpoint
    auto &id = cFed->registerEndpoint (myendpoint);

    auto &pubid = cFed->registerPublication ("pub", "double");

    auto &subid = cFed->registerSubscription (vtarget + "/pub", "double");
    std::cout << "entering init State\n";
    cFed->enterInitializingMode ();
    std::cout << "entered init State\n";
    cFed->enterExecutingMode ();
    std::cout << "entered exec State\n";
    for (int i = 1; i < 10; ++i)
    {
        std::string message = "message sent from " + name + " to " + etarget + " at time " + std::to_string (i);
        cFed->sendMessage (id, etarget, message.data (), message.size ());
        cFed->publish (pubid, i);
        std::cout << message << std::endl;
        auto newTime = cFed->requestTime (i);
        std::cout << "processed time " << static_cast<double> (newTime) << "\n";
        while (cFed->hasMessage (id))
        {
            auto nmessage = cFed->getMessage (id);
            std::cout << "received message from " << nmessage->source << " at "
                      << static_cast<double> (nmessage->time) << " ::" << nmessage->data.to_string () << '\n';
        }

        if (cFed->isUpdated (subid))
        {
            auto val = cFed->getDouble (subid);
            std::cout << "received updated value of " << val << " at " << newTime << " from "
                      << cFed->getTarget (subid) << '\n';
        }
    }
    cFed->finalize ();
    return 0;
}
