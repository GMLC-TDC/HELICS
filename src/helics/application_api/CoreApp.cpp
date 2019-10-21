/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "CoreApp.hpp"
#include "../core/CoreFactory.hpp"
#include "../core/core-exceptions.hpp"
#include "../core/helicsCLI11.hpp"
#include <fstream>
#include <iostream>

namespace helics
{
namespace apps
{
CoreApp::CoreApp (core_type ctype, std::vector<std::string> args)
{
    auto app = generateParser ();
    app->setDefaultCoreType (ctype);
    if (app->helics_parse (std::move (args)) == helicsCLI11App::parse_output::ok)
    {
        processArgs (app);
    }
}

CoreApp::CoreApp (std::vector<std::string> args) : CoreApp (core_type::DEFAULT, std::move (args)) {}

CoreApp::CoreApp (core_type ctype, int argc, char *argv[])
{
    auto app = generateParser ();
    app->setDefaultCoreType (ctype);
    if (app->helics_parse (argc, argv) == helicsCLI11App::parse_output::ok)
    {
        processArgs (app);
    }
}

CoreApp::CoreApp (int argc, char *argv[]) : CoreApp (core_type::DEFAULT, argc, argv) {}

CoreApp::CoreApp (core_type ctype, const std::string &argString)
{
    auto app = generateParser ();
    app->setDefaultCoreType (ctype);
    if (app->helics_parse (argString) == helicsCLI11App::parse_output::ok)
    {
        processArgs (app);
    }
}

CoreApp::CoreApp (const std::string &argString) : CoreApp (core_type::DEFAULT, argString) {}

CoreApp::~CoreApp ()
{
    if (!core)
    {
        return;
    }
    // this sleeps until disconnected
    core->waitForDisconnect ();
    core = nullptr;
    helics::CoreFactory::cleanUpCores (std::chrono::milliseconds (500));
}

std::unique_ptr<helicsCLI11App> CoreApp::generateParser ()
{
    auto app = std::make_unique<helicsCLI11App> ("Broker application");
    app->addTypeOption ();
    app->add_option ("--name,-n", name, "name of the broker");
    app->allow_extras ();
    auto app_p = app.get ();
    app->footer ([app_p]() {
        auto coreType = helics::coreTypeFromString ((*app_p)["--core"]->as<std::string> ());
        //CoreFactory:: (coreType);
        return std::string ();
    });
    return app;
}

void CoreApp::processArgs (std::unique_ptr<helicsCLI11App> &app)
{
    auto remArgs = app->remaining_for_passthrough ();
    core = CoreFactory::create (app->getCoreType (), name, remArgs);
    if (!core->isConnected ())
    {
        throw (ConnectionFailure ("Core is unable to connect\n"));
    }
}

bool CoreApp::isActive () const { return ((core) && (core->isConnected ())); }

void CoreApp::forceTerminate ()
{
    if (!core)
    {
        return;
    }
    if (core->isConnected ())
    {
        core->disconnect ();
    }
}

}  // namespace apps
}  // namespace helics
