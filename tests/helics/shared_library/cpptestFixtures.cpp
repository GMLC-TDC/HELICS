/*
Copyright (c) 2017-2026, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle
Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for
Energy Innovation LLC.  See the top-level NOTICE for additional details. and the Lawrence
Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "cpptestFixtures.hpp"

#include "../src/helics/cpp98/Broker.hpp"

#include <cctype>
#include <memory>
#include <string>

static bool hasIndexCode(const std::string& type_name)
{
    if (std::isdigit(type_name.back()) != 0) {
        if (*(type_name.end() - 2) == '_') {  // this setup ignores the setup mode
            return true;
        }
    }
    return false;
}

static auto startBrokerImp(const std::string& coreTypeName, std::string initialization_string)
{
    if (coreTypeName.starts_with("tcp")) {
        initialization_string += " --reuse_address";
    } else if (coreTypeName.starts_with("ipc")) {
        // this is to use the name instead of the "_ipc_broker" as the queue name
        // since we are linking it directly anyway
        initialization_string += " --client";
    }
    if (hasIndexCode(coreTypeName)) {
        std::string new_type(coreTypeName.begin(), coreTypeName.end() - 2);
        return std::make_shared<helicscpp::Broker>(new_type, std::string(), initialization_string);
    }
    return std::make_shared<helicscpp::Broker>(coreTypeName, std::string(), initialization_string);
}

bool FederateTestFixture_cpp::hasIndexCode(const std::string& type_name)
{
    if (std::isdigit(type_name.back()) != 0) {
        if (*(type_name.end() - 2) == '_') {  // this setup ignores the setup mode
            return true;
        }
    }
    return false;
}

int FederateTestFixture_cpp::getIndexCode(const std::string& type_name)
{
    return static_cast<int>(type_name.back() - '0');
}

FederateTestFixture_cpp::~FederateTestFixture_cpp()
{
    for (auto& fed : federates) {
        fed->finalize();
    }
    federates.clear();
    for (auto& broker : brokers) {
        if (ctype.starts_with("tcp")) {
            broker->waitForDisconnect(2000);
        } else {
            broker->waitForDisconnect(200);
        }

        if (broker->isConnected()) {
            broker->disconnect();
        }
    }
    brokers.clear();
    helicsCleanupLibrary();
}

std::shared_ptr<helicscpp::Broker>
    FederateTestFixture_cpp::AddBroker(const std::string& CoreType_name, int count)
{
    return AddBroker(CoreType_name, std::string("-f") + std::to_string(count));
}

std::shared_ptr<helicscpp::Broker>
    FederateTestFixture_cpp::AddBroker(const std::string& CoreType_name,
                                       const std::string& initialization_string)
{
    std::shared_ptr<helicscpp::Broker> broker;
    if (extraBrokerArgs.empty()) {
        broker = startBrokerImp(CoreType_name, initialization_string);
    } else {
        broker = startBrokerImp(CoreType_name, initialization_string + " " + extraBrokerArgs);
    }
    if (broker) {
        brokers.push_back(broker);
    }
    return broker;
}
