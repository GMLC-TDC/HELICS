/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "testFixtures_shared.hpp"

#include "helics/application_api/typeOperations.hpp"

#include <cctype>
#include <iostream>
#include <string>

bool hasIndexCode(std::string_view type_name)
{
    if (std::isdigit(type_name.back()) != 0) {
        if (*(type_name.end() - 2) == '_') {  // this check ignores the setup mode
            return true;
        }
    }
    return false;
}

int getIndexCode(std::string_view type_name)
{
    return static_cast<int>(type_name.back() - '0');
}

auto StartBrokerImp(std::string_view CoreType_name, const std::string& initialization_string)
{
    helics::CoreType type;
    if (hasIndexCode(CoreType_name)) {
        std::string new_type(CoreType_name.begin(), CoreType_name.end() - 2);
        type = helics::coreTypeFromString(new_type);
    } else {
        type = helics::coreTypeFromString(std::string(CoreType_name));
    }
    helics::BrokerApp broker;
    switch (type) {
        case helics::CoreType::TCP:
            broker = helics::BrokerApp(type, initialization_string + " --reuse_address");
            break;
        case helics::CoreType::IPC:
        case helics::CoreType::INTERPROCESS:
            broker = helics::BrokerApp(type, initialization_string + " --client");
            break;
        default:
            broker = helics::BrokerApp(type, initialization_string);
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
            broker.waitForDisconnect(std::chrono::milliseconds(2000));
        } else {
            broker.waitForDisconnect(std::chrono::milliseconds(200));
        }

        if (broker.isConnected()) {
            std::cout << "forcing disconnect\n";
            broker.forceTerminate();
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
        if (broker.isConnected()) {
            broker.forceTerminate();
        }
    }
    brokers.clear();
    helics::cleanupHelicsLibrary();
}

helics::BrokerApp FederateTestFixture::AddBroker(std::string_view CoreType_name, int count)
{
    return AddBroker(CoreType_name, std::string("-f ") + std::to_string(count));
}

helics::BrokerApp FederateTestFixture::AddBroker(std::string_view CoreType_name,
                                                 const std::string& initialization_string)
{
    helics::BrokerApp broker;
    if (extraBrokerArgs.empty()) {
        broker = StartBrokerImp(CoreType_name, initialization_string);
    } else {
        broker = StartBrokerImp(CoreType_name, initialization_string + " " + extraBrokerArgs);
    }
    // broker->setLoggingLevel (0);
    brokers.push_back(broker);
    return broker;
}
