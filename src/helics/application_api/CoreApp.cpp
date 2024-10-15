/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "CoreApp.hpp"

#include "../common/JsonGeneration.hpp"
#include "../core/CoreFactory.hpp"
#include "../core/core-exceptions.hpp"
#include "../core/coreTypeOperations.hpp"
#include "../core/helicsCLI11.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace helics {
CoreApp::CoreApp(CoreType ctype, std::string_view coreName, std::vector<std::string> args):
    name(coreName)
{
    auto app = generateParser();
    app->setDefaultCoreType(ctype);
    if (app->helics_parse(std::move(args)) == helicsCLI11App::ParseOutput::OK) {
        processArgs(app);
    }
}

CoreApp::CoreApp(CoreType ctype, std::vector<std::string> args)
{
    auto app = generateParser();
    app->setDefaultCoreType(ctype);
    if (app->helics_parse(std::move(args)) == helicsCLI11App::ParseOutput::OK) {
        processArgs(app);
    }
}

CoreApp::CoreApp(std::vector<std::string> args): CoreApp(CoreType::DEFAULT, std::move(args)) {}

CoreApp::CoreApp(CoreType ctype, std::string_view coreName, int argc, char* argv[]): name(coreName)
{
    auto app = generateParser();
    app->setDefaultCoreType(ctype);
    if (app->helics_parse(argc, argv) == helicsCLI11App::ParseOutput::OK) {
        processArgs(app);
    }
}

CoreApp::CoreApp(CoreType ctype, int argc, char* argv[]): CoreApp(ctype, std::string{}, argc, argv)
{
}

CoreApp::CoreApp(int argc, char* argv[]): CoreApp(CoreType::DEFAULT, std::string{}, argc, argv) {}

CoreApp::CoreApp(CoreType ctype, std::string_view coreName, std::string_view argString):
    name(coreName)
{
    auto app = generateParser();
    app->setDefaultCoreType(ctype);
    if (app->helics_parse(std::string{argString}) == helicsCLI11App::ParseOutput::OK) {
        processArgs(app);
    }
}

CoreApp::CoreApp(CoreType ctype, std::string_view argString):
    CoreApp(ctype, std::string_view{}, argString)
{
}

CoreApp::CoreApp(std::string_view argString)
{
    if (argString.find_first_of('-') == std::string::npos) {
        core = CoreFactory::findCore(argString);
        if (core) {
            name = core->getIdentifier();
            return;
        }
    }
    auto app = generateParser();
    if (app->helics_parse(std::string{argString}) == helicsCLI11App::ParseOutput::OK) {
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
        auto coreType = helics::core::coreTypeFromString((*app_p)["--coretype"]->as<std::string>());
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

void CoreApp::linkEndpoints(std::string_view source, std::string_view target)
{
    if (core) {
        core->linkEndpoints(source, target);
    }
}

void CoreApp::dataLink(std::string_view source, std::string_view target)
{
    if (core) {
        core->dataLink(source, target);
    }
}
/** add a source Filter to an endpoint*/
void CoreApp::addSourceFilterToEndpoint(std::string_view filter, std::string_view endpoint)
{
    if (core) {
        core->addSourceFilterToEndpoint(filter, endpoint);
    }
}
/** add a destination Filter to an endpoint*/
void CoreApp::addDestinationFilterToEndpoint(std::string_view filter, std::string_view endpoint)
{
    if (core) {
        core->addDestinationFilterToEndpoint(filter, endpoint);
    }
}

void CoreApp::addAlias(std::string_view interfaceName, std::string_view alias)
{
    if (core) {
        core->addAlias(interfaceName, alias);
    }
}

void CoreApp::makeConnections(const std::string& file)
{
    if (core) {
        core->makeConnections(file);
    }
}

// NOLINTNEXTLINE
static const std::string estring;

/** get the identifier of the core*/
const std::string& CoreApp::getIdentifier() const
{
    return (core) ? core->getIdentifier() : estring;
}

/** get the network address of the core*/
const std::string& CoreApp::getAddress() const
{
    return (core) ? core->getAddress() : estring;
}

/** make a query at the core*/
std::string
    CoreApp::query(std::string_view target, std::string_view queryStr, HelicsSequencingModes mode)
{
    return (core) ? core->query(target, queryStr, mode) :
                    generateJsonErrorResponse(JsonErrorCodes::BAD_GATEWAY, "Core not available");
}

void CoreApp::setTag(std::string_view tag, std::string_view value)
{
    if (core) {
        core->setFederateTag(gLocalCoreId, tag, value);
    }
}

const std::string& CoreApp::getTag(std::string_view tag) const
{
    if (core) {
        return core->getFederateTag(gLocalCoreId, tag);
    }
    return estring;
}

void CoreApp::setGlobal(std::string_view valueName, std::string_view value)
{
    if (core) {
        core->setGlobal(valueName, value);
    }
}

void CoreApp::sendCommand(std::string_view target,
                          std::string_view commandStr,
                          HelicsSequencingModes mode)
{
    if (core) {
        core->sendCommand(target, commandStr, std::string{}, mode);
    }
}

void CoreApp::setLoggingLevel(int loglevel)
{
    if (core) {
        core->setLoggingLevel(loglevel);
    }
}

/** set the log file to use for the core*/
void CoreApp::setLogFile(std::string_view logFile)
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
        core->setFlagOption(gLocalCoreId, HELICS_FLAG_DELAY_INIT_ENTRY, true);
    }
}

void CoreApp::reset()
{
    core.reset();
    name.clear();
}

void CoreApp::globalError(int32_t errorCode, std::string_view errorString)
{
    if (core) {
        core->globalError(gLocalCoreId, errorCode, errorString);
    }
}

}  // namespace helics
