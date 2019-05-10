/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "BrokerApp.hpp"
#include "../core/BrokerFactory.hpp"
#include "../core/CoreBroker.hpp"
#include "../core/core-exceptions.hpp"
#include "../core/helicsCLI11.hpp"
#include <fstream>
#include <iostream>

namespace helics
{
namespace apps
{
BrokerApp::BrokerApp (core_type ctype, std::vector<std::string> args) : type (ctype)
{
    auto app = generateParser ();
    if (app->helics_parse (std::move (args)) == helicsCLI11App::parse_return::ok)
    {
        processArgs (app);
    }
}

BrokerApp::BrokerApp (std::vector<std::string> args) : BrokerApp (core_type::DEFAULT, std::move (args)) {}

BrokerApp::BrokerApp (core_type ctype, int argc, char *argv[]) : type (ctype)
{
    auto app = generateParser ();
    if (app->helics_parse (argc, argv) == helicsCLI11App::parse_return::ok)
    {
        processArgs (app);
    }
}

BrokerApp::BrokerApp (int argc, char *argv[]) : BrokerApp (core_type::DEFAULT, argc, argv) {}

BrokerApp::BrokerApp (core_type ctype, const std::string &argString) : type (ctype)
{
    auto app = generateParser ();
    if (app->helics_parse (argString) == helicsCLI11App::parse_return::ok)
    {
        processArgs (app);
    }
}

BrokerApp::BrokerApp (const std::string &argString) : BrokerApp (core_type::DEFAULT, argString) {}

BrokerApp::~BrokerApp ()
{
    if (!broker)
    {
        return;
    }
    // this sleeps until disconnected
    broker->waitForDisconnect ();
    broker = nullptr;
    helics::BrokerFactory::cleanUpBrokers (std::chrono::milliseconds (500));
}

std::unique_ptr<helicsCLI11App> BrokerApp::generateParser ()
{
    auto app = std::make_unique<helicsCLI11App> ("Broker application");
    app->addTypeOption ();
    app->add_option ("--name,-n", name, "name of the broker");
    app->allow_extras ();
    auto app_p = app.get ();
    app->footer ([app_p] () {
        auto coreType = helics::coreTypeFromString ((*app_p)["--core"]->as<std::string> ());
        BrokerFactory::displayHelp (coreType);
        return std::string ();
    });
    return app;
}

void BrokerApp::processArgs (std::unique_ptr<helicsCLI11App> &app)
{
    auto remArgs = app->remaining_for_passthrough ();
    broker = BrokerFactory::create (app->getCoreType (), name, remArgs);
    if (!broker->isConnected ())
    {
        throw (ConnectionFailure ("Broker is unable to connect\n"));
    }
}

/** run the Echo federate until the specified time
@param stopTime_input the desired stop time
*/
/** check if the Broker is running*/
bool BrokerApp::isActive () const { return ((broker) && (broker->isConnected ())); }

/** forceably disconnect the broker*/
void BrokerApp::forceTerminate ()
{
    if (!broker)
    {
        return;
    }
    if (broker->isConnected ())
    {
        broker->disconnect ();
    }
}

}  // namespace apps
}  // namespace helics
