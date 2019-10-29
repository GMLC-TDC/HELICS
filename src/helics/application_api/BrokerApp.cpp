/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "BrokerApp.hpp"
#include "../core/BrokerFactory.hpp"
#include "../core/CoreBroker.hpp"
#include "../core/core-exceptions.hpp"
#include "../core/coreTypeOperations.hpp"
#include "../core/helicsCLI11.hpp"
#include <fstream>
#include <iostream>

namespace helics
{
BrokerApp::BrokerApp (core_type ctype, std::vector<std::string> args)
{
    auto app = generateParser ();
    app->setDefaultCoreType (ctype);
    if (app->helics_parse (std::move (args)) == helicsCLI11App::parse_output::ok)
    {
        processArgs (app);
    }
}

BrokerApp::BrokerApp (std::vector<std::string> args) : BrokerApp (core_type::DEFAULT, std::move (args)) {}

BrokerApp::BrokerApp (core_type ctype, int argc, char *argv[])
{
    auto app = generateParser ();
    app->setDefaultCoreType (ctype);
    if (app->helics_parse (argc, argv) == helicsCLI11App::parse_output::ok)
    {
        processArgs (app);
    }
}

BrokerApp::BrokerApp (int argc, char *argv[]) : BrokerApp (core_type::DEFAULT, argc, argv) {}

BrokerApp::BrokerApp (core_type ctype, const std::string &argString)
{
    auto app = generateParser ();
    app->setDefaultCoreType (ctype);
    if (app->helics_parse (argString) == helicsCLI11App::parse_output::ok)
    {
        processArgs (app);
    }
}

BrokerApp::BrokerApp (const std::string &argString) : BrokerApp (core_type::DEFAULT, argString) {}

bool BrokerApp::waitForDisconnect (std::chrono::milliseconds waitTime)
{
    if (broker)
    {
        return broker->waitForDisconnect (waitTime);
    }
    return true;
}

std::unique_ptr<helicsCLI11App> BrokerApp::generateParser ()
{
    auto app = std::make_unique<helicsCLI11App> ("Broker application");
    app->addTypeOption ();
    app->add_option ("--name,-n", name, "name of the broker");
    app->allow_extras ();
    auto app_p = app.get ();
    app->footer ([app_p] () {
        auto coreType = coreTypeFromString((*app_p)["--core"]->as<std::string> ());
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

bool BrokerApp::isConnected () const { return ((broker) && (broker->isConnected ())); }

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

void BrokerApp::dataLink (const std::string &source, const std::string &target)
{
    if (broker)
    {
        broker->dataLink (source, target);
    }
}
/** add a source Filter to an endpoint*/
void BrokerApp::addSourceFilterToEndpoint (const std::string &filter, const std::string &endpoint)
{
    if (broker)
    {
        broker->addSourceFilterToEndpoint (filter, endpoint);
    }
}
/** add a destination Filter to an endpoint*/
void BrokerApp::addDestinationFilterToEndpoint (const std::string &filter, const std::string &endpoint)
{
    if (broker)
    {
        broker->addDestinationFilterToEndpoint (filter, endpoint);
    }
}

void BrokerApp::makeConnections(const std::string &file)
{
    if (broker)
    {
        broker->makeConnections (file);
    }
}
static const std::string estring{};
/** get the identifier of the broker*/
const std::string &BrokerApp::getIdentifier () const { return (broker) ? broker->getIdentifier () : estring; }
/** get the network address of the broker*/
const std::string &BrokerApp::getAddress () const { return (broker) ? broker->getAddress () : estring; }
/** make a query at the broker*/
std::string BrokerApp::query (const std::string &target, const std::string &query)
{
    return (broker) ? broker->query (target, query) : std::string ("#error");
}
/** set the log file to use for the broker*/
void BrokerApp::setLogFile (const std::string &logFile)
{
    if (broker)
    {
        broker->setLogFile (logFile);
    }
}

void BrokerApp::reset () { broker = nullptr; }
}  // namespace helics
