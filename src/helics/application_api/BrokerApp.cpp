/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
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
#include <utility>

namespace helics {

BrokerApp::BrokerApp(core_type ctype,
                     const std::string& broker_name,
                     std::vector<std::string> args):
    name(broker_name)
{
    auto app = generateParser(ctype == core_type::MULTI);
    app->setDefaultCoreType(ctype);
    app->passConfig = true;
    if (app->helics_parse(std::move(args)) == helicsCLI11App::parse_output::ok) {
        processArgs(app);
    }
}

BrokerApp::BrokerApp(core_type ctype, std::vector<std::string> args):
    BrokerApp(ctype, std::string{}, std::move(args))
{
}

BrokerApp::BrokerApp(std::vector<std::string> args):
    BrokerApp(core_type::DEFAULT, std::string{}, std::move(args))
{
}

BrokerApp::BrokerApp(core_type ctype, const std::string& brokerName, int argc, char* argv[]):
    name(brokerName)
{
    auto app = generateParser(ctype == core_type::MULTI);
    app->setDefaultCoreType(ctype);
    app->passConfig = true;
    if (app->helics_parse(argc, argv) == helicsCLI11App::parse_output::ok) {
        processArgs(app);
    }
}

BrokerApp::BrokerApp(core_type ctype, int argc, char* argv[]):
    BrokerApp(ctype, std::string{}, argc, argv)
{
}

BrokerApp::BrokerApp(int argc, char* argv[]):
    BrokerApp(core_type::DEFAULT, std::string{}, argc, argv)
{
}

BrokerApp::BrokerApp(core_type ctype, const std::string& brokerName, const std::string& argString):
    name(brokerName)
{
    auto app = generateParser(ctype == core_type::MULTI);
    app->setDefaultCoreType(ctype);
    app->passConfig = true;
    if (app->helics_parse(argString) == helicsCLI11App::parse_output::ok) {
        processArgs(app);
    }
}

BrokerApp::BrokerApp(core_type ctype, const std::string& argString):
    BrokerApp(ctype, std::string{}, argString)
{
}

BrokerApp::BrokerApp(const std::string& argString)
{
    if (argString.find_first_of('-') == std::string::npos) {
        broker = BrokerFactory::findBroker(argString);
        if (broker) {
            name = broker->getIdentifier();
            return;
        }
    }
    auto app = generateParser();
    if (app->helics_parse(argString) == helicsCLI11App::parse_output::ok) {
        processArgs(app);
    }
}

BrokerApp::BrokerApp(std::shared_ptr<Broker> brk): broker(std::move(brk))
{
    if (broker) {
        name = broker->getIdentifier();
    }
}

bool BrokerApp::waitForDisconnect(std::chrono::milliseconds waitTime)
{
    if (broker) {
        return broker->waitForDisconnect(waitTime);
    }
    return true;
}

std::unique_ptr<helicsCLI11App> BrokerApp::generateParser(bool noTypeOption)
{
    auto app = std::make_unique<helicsCLI11App>("Broker application");
    if (!noTypeOption) {
        app->addTypeOption();
    }

    if (name.empty()) {
        app->add_option("--name,-n", name, "name of the broker");
    }
    app->allow_extras();
    auto* app_p = app.get();
    app->footer([app_p]() {
        auto coreType = coreTypeFromString((*app_p)["--core"]->as<std::string>());
        BrokerFactory::displayHelp(coreType);
        return std::string();
    });
    return app;
}

void BrokerApp::processArgs(std::unique_ptr<helicsCLI11App>& app)
{
    auto& remArgs = app->remainArgs();
    try {
        broker = BrokerFactory::create(app->getCoreType(), name, remArgs);
    }
    catch (const helics::RegistrationFailure&) {
        if (!name.empty()) {
            broker = BrokerFactory::findBroker(name);
            if (broker) {
                return;
            }
        }
    }
    if (!broker || !broker->isConnected()) {
        throw(ConnectionFailure("Broker is unable to connect\n"));
    }
}

bool BrokerApp::isConnected() const
{
    return ((broker) && (broker->isConnected()));
}

bool BrokerApp::connect()
{
    return ((broker) && (broker->connect()));
}

bool BrokerApp::isOpenToNewFederates() const
{
    return ((broker) && (broker->isOpenToNewFederates()));
}

void BrokerApp::forceTerminate()
{
    if (!broker) {
        return;
    }
    if (broker->isConnected()) {
        broker->disconnect();
    }
}

void BrokerApp::dataLink(const std::string& source, const std::string& target)
{
    if (broker) {
        broker->dataLink(source, target);
    }
}
/** add a source Filter to an endpoint*/
void BrokerApp::addSourceFilterToEndpoint(const std::string& filter, const std::string& endpoint)
{
    if (broker) {
        broker->addSourceFilterToEndpoint(filter, endpoint);
    }
}
/** add a destination Filter to an endpoint*/
void BrokerApp::addDestinationFilterToEndpoint(const std::string& filter,
                                               const std::string& endpoint)
{
    if (broker) {
        broker->addDestinationFilterToEndpoint(filter, endpoint);
    }
}

void BrokerApp::makeConnections(const std::string& file)
{
    if (broker) {
        broker->makeConnections(file);
    }
}

/** get the identifier of the broker*/
const std::string& BrokerApp::getIdentifier() const
{
    static const std::string estring;
    return (broker) ? broker->getIdentifier() : estring;
}
/** get the network address of the broker*/
const std::string& BrokerApp::getAddress() const
{
    static const std::string estring;
    return (broker) ? broker->getAddress() : estring;
}
/** make a query at the broker*/
std::string BrokerApp::query(const std::string& target,
                             const std::string& queryStr,
                             helics_sequencing_mode mode)
{
    return (broker) ? broker->query(target, queryStr, mode) : std::string("#error");
}

void BrokerApp::setGlobal(const std::string& valueName, const std::string& value)
{
    if (broker) {
        broker->setGlobal(valueName, value);
    }
}

void BrokerApp::setLoggingLevel(int loglevel)
{
    if (broker) {
        broker->setLoggingLevel(loglevel);
    }
}

void BrokerApp::setTimeBarrier(Time barrierTime)
{
    if (broker) {
        broker->setTimeBarrier(barrierTime);
    }
}

void BrokerApp::clearTimeBarrier()
{
    if (broker) {
        broker->clearTimeBarrier();
    }
}

void BrokerApp::globalError(int32_t errorCode, const std::string& errorString)
{
    if (broker) {
        broker->globalError(errorCode, errorString);
    }
}

/** set the log file to use for the broker*/
void BrokerApp::setLogFile(const std::string& logFile)
{
    if (broker) {
        broker->setLogFile(logFile);
    }
}

void BrokerApp::reset()
{
    broker = nullptr;
}
}  // namespace helics
