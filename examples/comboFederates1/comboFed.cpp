/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helics/application_api/BrokerApp.hpp"
#include "helics/application_api/CombinationFederate.hpp"
#include "helics/core/helicsCLI11.hpp"
#include "helics/core/helics_definitions.hpp"

#include <iostream>
#include <thread>

int main(int argc, char* argv[])
{
    helics::helicsCLI11App app("Combination Fed", "ComboFed");
    std::string targetEndpoint = "endpoint";
    std::string vtarget = "fed";
    std::string mtarget = "fed";
    std::string myendpoint = "endpoint";
    helics::BrokerApp brk;
    std::string brokerArgs = "";

    app.add_option_function<std::string>(
        "--target,-t",
        [&vtarget, &mtarget](const std::string& name) {
            vtarget = name;
            mtarget = name;
        },
        "name of the federate to target");
    app.add_option("--valuetarget", vtarget, "name of the value federate to target", true);
    app.add_option("--messagetarget", mtarget, "name of the message federate to target", true);
    app.add_option("--endpoint,-e", targetEndpoint, "name of the target endpoint", true);
    app.add_option("--source,-s", myendpoint, "name of the source endpoint", true);
    app.add_option("--startbroker", brokerArgs, "start a broker with the specified arguments");

    auto ret = app.helics_parse(argc, argv);

    helics::FederateInfo fi;
    if (ret == helics::helicsCLI11App::parse_output::help_call) {
        (void)(fi.loadInfoFromArgs("--help"));
        return 0;
    } else if (ret != helics::helicsCLI11App::parse_output::ok) {
        return -1;
    }
    fi.defName = "fed";
    fi.loadInfoFromArgs(app.remainArgs());

    std::string etarget = mtarget + "/" + targetEndpoint;

    fi.setProperty(helics::defs::properties::log_level, 5);
    if (app["--startbroker"]->count() > 0) {
        brk = helics::BrokerApp(fi.coreType, brokerArgs);
    }

    auto cFed = std::make_unique<helics::CombinationFederate>(std::string{}, fi);
    auto name = cFed->getName();
    std::cout << " registering endpoint '" << myendpoint << "' for " << name << '\n';

    // this line actually creates an endpoint
    auto& id = cFed->registerEndpoint(myendpoint);

    auto& pubid = cFed->registerPublication("pub", "double");

    auto& subid = cFed->registerSubscription(vtarget + "/pub", "double");

    cFed->logInfoMessage("Registration Complete");

    std::cout << "entering init State\n";
    cFed->enterInitializingMode();
    std::cout << "entered init State\n";
    cFed->enterExecutingMode();
    std::cout << "entered exec State\n";
    for (int i = 1; i < 10; ++i) {
        std::string message =
            "message sent from " + name + " to " + etarget + " at time " + std::to_string(i);
        cFed->sendMessage(id, etarget, message.data(), message.size());
        cFed->publish(pubid, i);
        std::cout << message << std::endl;
        auto newTime = cFed->requestTime(i);
        std::cout << "processed time " << static_cast<double>(newTime) << "\n";
        while (cFed->hasMessage(id)) {
            auto nmessage = cFed->getMessage(id);
            std::cout << "received message from " << nmessage->source << " at "
                      << static_cast<double>(nmessage->time) << " ::" << nmessage->data.to_string()
                      << '\n';
        }

        if (cFed->isUpdated(subid)) {
            auto val = cFed->getDouble(subid);
            std::cout << "received updated value of " << val << " at " << newTime << " from "
                      << cFed->getTarget(subid) << '\n';
        }
    }
    cFed->logMessage(7, "Process Complete.");
    cFed->logMessage(1, "Reached End of application.");
    cFed->logInfoMessage("Calling Finalize.");
    cFed->finalize();
    return 0;
}
