/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "CoreApp.hpp"

#include "../core/CoreFactory.hpp"
#include "../core/core-exceptions.hpp"
#include "../core/coreTypeOperations.hpp"
#include "../core/helicsCLI11.hpp"

#include <fstream>
#include <iostream>
#include <utility>

namespace helics {
CoreApp::CoreApp(core_type ctype, const std::string& coreName, std::vector<std::string> args):
    name(coreName)
{
    auto app = generateParser();
    app->setDefaultCoreType(ctype);
    if (app->helics_parse(std::move(args)) == helicsCLI11App::parse_output::ok) {
        processArgs(app);
    }
}

CoreApp::CoreApp(core_type ctype, std::vector<std::string> args)
{
    auto app = generateParser();
    app->setDefaultCoreType(ctype);
    if (app->helics_parse(std::move(args)) == helicsCLI11App::parse_output::ok) {
        processArgs(app);
    }
}

CoreApp::CoreApp(std::vector<std::string> args): CoreApp(core_type::DEFAULT, std::move(args)) {}

CoreApp::CoreApp(core_type ctype, const std::string& coreName, int argc, char* argv[]):
    name(coreName)
{
    auto app = generateParser();
    app->setDefaultCoreType(ctype);
    if (app->helics_parse(argc, argv) == helicsCLI11App::parse_output::ok) {
        processArgs(app);
    }
}

CoreApp::CoreApp(core_type ctype, int argc, char* argv[]): CoreApp(ctype, std::string{}, argc, argv)
{
}

CoreApp::CoreApp(int argc, char* argv[]): CoreApp(core_type::DEFAULT, std::string{}, argc, argv) {}

CoreApp::CoreApp(core_type ctype, const std::string& coreName, const std::string& argString):
    name(coreName)
{
    auto app = generateParser();
    app->setDefaultCoreType(ctype);
    if (app->helics_parse(argString) == helicsCLI11App::parse_output::ok) {
        processArgs(app);
    }
}

CoreApp::CoreApp(core_type ctype, const std::string& argString):
    CoreApp(ctype, std::string{}, argString)
{
}

CoreApp::CoreApp(const std::string& argString)
{
    if (argString.find_first_of('-') == std::string::npos) {
        core = CoreFactory::findCore(argString);
        if (core) {
            name = core->getIdentifier();
            return;
        }
    }
    auto app = generateParser();
    if (app->helics_parse(argString) == helicsCLI11App::parse_output::ok) {
        processArgs(app);
    }
}

CoreApp::CoreApp(std::shared_ptr<Core> cr): core(std::move(cr))
{
    if (core) {
        name = core->getIdentifier();
    }
}

std::unique_ptr<helicsCLI11App> CoreApp::generateParser()
{
    auto app = std::make_unique<helicsCLI11App>("Broker application");
    app->addTypeOption();
    if (name.empty()) {
        app->add_option("--name,-n", name, "name of the core");
    }
    app->allow_extras();
    auto* app_p = app.get();
    app->footer([app_p]() {
        auto coreType = helics::core::coreTypeFromString((*app_p)["--core"]->as<std::string>());
        CoreFactory::displayHelp(coreType);
        return std::string{};
    });
    return app;
}

void CoreApp::processArgs(std::unique_ptr<helicsCLI11App>& app)
{
    auto remArgs = app->remaining_for_passthrough();
    try {
        core = CoreFactory::create(app->getCoreType(), name, remArgs);
    }
    catch (...) {
        if (!remArgs.empty()) {
            name = remArgs.front();
        }
        if (!name.empty()) {
            core = CoreFactory::findCore(name);
            if (core) {
                // LCOV_EXCL_START
                name = core->getIdentifier();
                return;
                // LCOV_EXCL_STOP
            }
        }
    }
    if (!core) {
        throw(ConnectionFailure("Unable to create core\n"));
    }
}

bool CoreApp::isConnected() const
{
    return ((core) && (core->isConnected()));
}

bool CoreApp::connect()
{
    return (core) ? core->connect() : false;
}

bool CoreApp::isOpenToNewFederates() const
{
    return ((core) && (core->isOpenToNewFederates()));
}

void CoreApp::forceTerminate()
{
    if (!core) {
        return;
    }
    if (core->isConnected()) {
        core->disconnect();
    }
}

bool CoreApp::waitForDisconnect(std::chrono::milliseconds waitTime)
{
    if (core) {
        return core->waitForDisconnect(waitTime);
    }
    return true;
}

void CoreApp::dataLink(const std::string& source, const std::string& target)
{
    if (core) {
        core->dataLink(source, target);
    }
}
/** add a source Filter to an endpoint*/
void CoreApp::addSourceFilterToEndpoint(const std::string& filter, const std::string& endpoint)
{
    if (core) {
        core->addSourceFilterToEndpoint(filter, endpoint);
    }
}
/** add a destination Filter to an endpoint*/
void CoreApp::addDestinationFilterToEndpoint(const std::string& filter, const std::string& endpoint)
{
    if (core) {
        core->addDestinationFilterToEndpoint(filter, endpoint);
    }
}

void CoreApp::makeConnections(const std::string& file)
{
    if (core) {
        core->makeConnections(file);
    }
}

/** get the identifier of the core*/
const std::string& CoreApp::getIdentifier() const
{
    static const std::string estring;
    return (core) ? core->getIdentifier() : estring;
}

/** get the network address of the core*/
const std::string& CoreApp::getAddress() const
{
    static const std::string estring;
    return (core) ? core->getAddress() : estring;
}

/** make a query at the core*/
std::string CoreApp::query(const std::string& target,
                           const std::string& queryStr,
                           helics_sequencing_mode mode)
{
    return (core) ? core->query(target, queryStr, mode) : std::string("#error");
}

void CoreApp::setGlobal(const std::string& valueName, const std::string& value)
{
    if (core) {
        core->setGlobal(valueName, value);
    }
}

void CoreApp::setLoggingLevel(int loglevel)
{
    if (core) {
        core->setLoggingLevel(loglevel);
    }
}

/** set the log file to use for the core*/
void CoreApp::setLogFile(const std::string& logFile)
{
    if (core) {
        core->setLogFile(logFile);
    }
}

void CoreApp::setReadyToInit()
{
    if (core) {
        core->setCoreReadyToInit();
    }
}
void CoreApp::haltInit()
{
    if (core) {
        core->setFlagOption(local_core_id, helics_flag_delay_init_entry, true);
    }
}

void CoreApp::reset()
{
    core.reset();
    name.clear();
}

void CoreApp::globalError(int32_t errorCode, const std::string& errorString)
{
    if (core) {
        core->globalError(local_core_id, errorCode, errorString);
    }
}

}  // namespace helics
