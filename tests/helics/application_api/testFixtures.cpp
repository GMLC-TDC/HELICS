/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "testFixtures.hpp"

#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"

#include <cctype>
#include <iostream>
#include <spdlog/spdlog.h>
#include <string>

bool hasIndexCode(const std::string& type_name)
{
    if (std::isdigit(type_name.back()) != 0) {
        if (*(type_name.end() - 2) == '_') {  // this check ignores the setup mode
            return true;
        }
    }
    return false;
}

int getIndexCode(const std::string& type_name)
{
    return static_cast<int>(type_name.back() - '0');
}

auto StartBrokerImp(const std::string& CoreType_name, const std::string& initialization_string)
{
    helics::CoreType type;
    if (hasIndexCode(CoreType_name)) {
        std::string new_type(CoreType_name.begin(), CoreType_name.end() - 2);
        type = helics::core::coreTypeFromString(new_type);
    } else {
        type = helics::core::coreTypeFromString(CoreType_name);
    }
    std::shared_ptr<helics::Broker> broker;
    switch (type) {
        case helics::CoreType::TCP:
            broker =
                helics::BrokerFactory::create(type, initialization_string + " --reuse_address");
            break;
        case helics::CoreType::IPC:
        case helics::CoreType::INTERPROCESS:
            broker = helics::BrokerFactory::create(type, initialization_string + " --client");
            break;
        default:
            broker = helics::BrokerFactory::create(type, initialization_string);
    }
    return broker;
}

FederateTestFixture::~FederateTestFixture()
{
    for (auto& fed : federates) {
        if (fed &&
            (!((fed->getCurrentMode() == helics::Federate::Modes::FINALIZE) ||
               (fed->getCurrentMode() == helics::Federate::Modes::ERROR_STATE)))) {
            fed->finalize();
        }
    }
    federates.clear();
    for (auto& broker : brokers) {
        if (ctype.compare(0, 3, "tcp") == 0) {
            broker->waitForDisconnect(std::chrono::milliseconds(2000));
        } else {
            broker->waitForDisconnect(std::chrono::milliseconds(200));
        }

        if (broker->isConnected()) {
            spdlog::info("forcing disconnect");
            broker->disconnect();
        }
    }
    brokers.clear();
    helics::cleanupHelicsLibrary();
}

void FederateTestFixture::FullDisconnect()
{
    for (auto& fed : federates) {
        if (fed && fed->getCurrentMode() != helics::Federate::Modes::FINALIZE) {
            fed->finalize();
        }
    }
    federates.clear();
    for (auto& broker : brokers) {
        if (broker->isConnected()) {
            broker->disconnect();
        }
    }
    brokers.clear();
    helics::cleanupHelicsLibrary();
}

std::shared_ptr<helics::Broker> FederateTestFixture::AddBroker(const std::string& CoreType_name,
                                                               int count)
{
    return AddBroker(CoreType_name, std::string("-f ") + std::to_string(count));
}

std::shared_ptr<helics::Broker>
    FederateTestFixture::AddBroker(const std::string& CoreType_name,
                                   const std::string& initialization_string)
{
    std::shared_ptr<helics::Broker> broker;
    if (extraBrokerArgs.empty()) {
        broker = StartBrokerImp(CoreType_name, initialization_string);
    } else {
        broker = StartBrokerImp(CoreType_name, initialization_string + " " + extraBrokerArgs);
    }
    broker->setLoggingLevel(0);
    brokers.push_back(broker);
    return broker;
}
