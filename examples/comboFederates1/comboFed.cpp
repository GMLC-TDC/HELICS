/*
Copyright (c) 2017-2023,
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

// NOLINTNEXTLINE
int main(int argc, char* argv[])
{
    helics::helicsCLI11App app("Combination Fed", "ComboFed");
    std::string targetEndpoint = "endpoint";
    std::string vtarget = "fed";
    std::string mtarget = "fed";
    std::string myendpoint = "endpoint";
    helics::BrokerApp brk;
    std::string brokerArgs;

    app.add_option_function<std::string>(
        "--target,-t",
        [&vtarget, &mtarget](const std::string& name) {
            vtarget = name;
            mtarget = name;
        },
        "name of the federate to target");
    app.add_option("--valuetarget", vtarget, "name of the value federate to target")
        ->capture_default_str();
    app.add_option("--messagetarget", mtarget, "name of the message federate to target")
        ->capture_default_str();
    app.add_option("--endpoint,-e", targetEndpoint, "name of the target endpoint")
        ->capture_default_str();
    app.add_option("--source,-s", myendpoint, "name of the source endpoint")->capture_default_str();
    app.add_option("--startbroker", brokerArgs, "start a broker with the specified arguments");
    app.allow_extras();

    app.allow_extras();
    auto ret = app.helics_parse(argc, argv);

    helics::FederateInfo fi;
    if (ret == helics::helicsCLI11App::ParseOutput::HELP_CALL) {
        (void)(fi.loadInfoFromArgs("--help"));
        return 0;
    }
    if (ret != helics::helicsCLI11App::ParseOutput::OK) {
        return -1;
    }
    fi.defName = "fed";
    fi.loadInfoFromArgs(app.remainArgs());

    std::string etarget = mtarget + "/" + targetEndpoint;

    fi.setProperty(helics::defs::Properties::LOG_LEVEL, HELICS_LOG_LEVEL_TIMING);
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
        std::string message = std::string("message sent from ") + name;
        message.append(" to ");
        message.append(etarget);
        message.append(" at time ");
        message.append(std::to_string(i));
        id.sendTo(message.data(), message.size(), etarget);
        pubid.publish(i);
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
            auto val = subid.getValue<double>();
            std::cout << "received updated value of " << val << " at " << newTime << " from "
                      << cFed->getTarget(subid) << '\n';
        }
    }
    cFed->logMessage(HELICS_LOG_LEVEL_TRACE, "Process Complete.");
    cFed->logMessage(HELICS_LOG_LEVEL_WARNING, "Reached End of application.");
    cFed->logInfoMessage("Calling Finalize.");
    cFed->finalize();
    return 0;
}
