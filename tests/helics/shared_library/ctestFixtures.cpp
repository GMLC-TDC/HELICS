/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "ctestFixtures.hpp"

#include <cassert>
#include <cctype>

static bool hasIndexCode(const std::string& type_name)
{
    if (std::isdigit(type_name.back()) != 0) {
        if (*(type_name.end() - 2) == '_') {  // this setup ignores the setup mode
            return true;
        }
    }
    return false;
}

static auto StartBrokerImp(const std::string& CoreType_name, std::string initialization_string)
{
    if (CoreType_name.compare(0, 3, "tcp") == 0) {
        initialization_string += " --reuse_address";
    } else if (CoreType_name.compare(0, 3, "ipc") == 0) {
        initialization_string += " --client";
    }
    if (hasIndexCode(CoreType_name)) {
        std::string new_type(CoreType_name.begin(), CoreType_name.end() - 2);
        return helicsCreateBroker(new_type.c_str(), NULL, initialization_string.c_str(), 0);
    }
    return helicsCreateBroker(CoreType_name.c_str(), NULL, initialization_string.c_str(), 0);
}

bool FederateTestFixture::hasIndexCode(const std::string& type_name)
{
    if (std::isdigit(type_name.back()) != 0) {
        if (*(type_name.end() - 2) == '_') {  // this setup ignores the setup mode
            return true;
        }
    }
    return false;
}

int FederateTestFixture::getIndexCode(const std::string& type_name)
{
    return static_cast<int>(type_name.back() - '0');
}

FederateTestFixture::FederateTestFixture()
{
    helicsErrorClear(&err);
}
FederateTestFixture::~FederateTestFixture()
{
    for (auto& fed : federates) {
        if (fed) {
            helics_federate_state state = helicsFederateGetState(fed, nullptr);
            helics_core core = helicsFederateGetCore(fed, nullptr);
            if (state != helics_state_finalize) {
                helicsFederateFinalize(fed, nullptr);
            }
            helicsFederateFree(fed);
            if (helicsCoreIsValid(core)) {
                helicsCoreDisconnect(core, nullptr);
            }
            helicsCoreFree(core);
        }
    }
    federates.clear();

    for (auto& broker : brokers) {
        if (helicsBrokerIsValid(broker) == HELICS_FALSE) {
            continue;
        }
        helics_bool res;
        if (ctype.compare(0, 3, "tcp") == 0) {
            res = helicsBrokerWaitForDisconnect(broker, 2000, nullptr);
        } else {
            res = helicsBrokerWaitForDisconnect(broker, 200, nullptr);
        }

        if (res != helics_true) {
            printf("forcing disconnect\n");
            helicsBrokerDisconnect(broker, nullptr);
        }

        helicsBrokerFree(broker);
    }
    brokers.clear();
    helicsCleanupLibrary();
}

helics_broker FederateTestFixture::AddBroker(const std::string& CoreType_name, int count)
{
    return AddBroker(CoreType_name, std::string("-f") + std::to_string(count));
}

helics_broker FederateTestFixture::AddBroker(const std::string& CoreType_name,
                                             const std::string& initialization_string)
{
    helics_broker broker;
    if (extraBrokerArgs.empty()) {
        broker = StartBrokerImp(CoreType_name, initialization_string);
    } else {
        broker = StartBrokerImp(CoreType_name, initialization_string + " " + extraBrokerArgs);
    }
    assert(helicsBrokerIsValid(broker) == helics_true);
    brokers.push_back(broker);
    return broker;
}

void FederateTestFixture::SetupTest(FedCreator ctor,
                                    const std::string& CoreType_name,
                                    int count,
                                    helics_time time_delta,
                                    const std::string& name_prefix)
{
    ctype = CoreType_name;
    helics_broker broker = AddBroker(CoreType_name, count);
    assert(helicsBrokerIsValid(broker) == helics_true);
    AddFederates(ctor, CoreType_name, count, broker, time_delta, name_prefix);
}

void FederateTestFixture::AddFederates(FedCreator ctor,
                                       std::string CoreType_name,
                                       int count,
                                       helics_broker broker,
                                       helics_time time_delta,
                                       const std::string& name_prefix)
{
    bool hasIndex = hasIndexCode(CoreType_name);
    int setup = (hasIndex) ? getIndexCode(CoreType_name) : 1;
    if (hasIndex) {
        CoreType_name.pop_back();
        CoreType_name.pop_back();
    }

    std::string initString = std::string("--broker=");
    initString += helicsBrokerGetIdentifier(broker);
    initString += " --broker_address=";
    initString += helicsBrokerGetAddress(broker);

    if (!extraCoreArgs.empty()) {
        initString.push_back(' ');
        initString.append(extraCoreArgs);
    }

    helics_federate_info fi = helicsCreateFederateInfo();
    helicsFederateInfoSetCoreTypeFromString(fi, CoreType_name.c_str(), &err);
    helicsFederateInfoSetTimeProperty(fi, HELICS_PROPERTY_TIME_delta, time_delta, &err);

    switch (setup) {
        case 1:
        default: {
            auto init = initString + " --federates " + std::to_string(count);
            auto core = helicsCreateCore(CoreType_name.c_str(), NULL, init.c_str(), &err);

            helicsFederateInfoSetCoreName(fi, helicsCoreGetIdentifier(core), &err);
            assert(err.errorCode == 0);
            size_t offset = federates.size();
            federates.resize(count + offset);
            for (int ii = 0; ii < count; ++ii) {
                auto name = name_prefix + std::to_string(ii + offset);
                auto fed = ctor(name.c_str(), fi, &err);
                assert(err.errorCode == 0);
                federates[ii + offset] = fed;
            }
            helicsCoreFree(core);
        } break;
        case 2: {  // each federate has its own core
            size_t offset = federates.size();
            federates.resize(count + offset);
            for (int ii = 0; ii < count; ++ii) {
                auto init = initString + " --federates 1";
                auto core = helicsCreateCore(CoreType_name.c_str(), NULL, init.c_str(), &err);

                helicsFederateInfoSetCoreName(fi, helicsCoreGetIdentifier(core), &err);
                assert(err.errorCode == 0);
                auto name = name_prefix + std::to_string(ii + offset);
                auto fed = ctor(name.c_str(), fi, &err);
                assert(err.errorCode == 0);
                federates[ii + offset] = fed;
                helicsCoreFree(core);
            }
        } break;
        case 3: {
            auto subbroker =
                AddBroker(CoreType_name, initString + " --federates " + std::to_string(count));
            auto newTypeString = CoreType_name;
            newTypeString.push_back('_');
            newTypeString.push_back('2');
            for (int ii = 0; ii < count; ++ii) {
                AddFederates(ctor, newTypeString, 1, subbroker, time_delta, name_prefix);
            }
        } break;
        case 4: {
            auto newTypeString = CoreType_name;
            newTypeString.push_back('_');
            newTypeString.push_back('2');
            for (int ii = 0; ii < count; ++ii) {
                auto subbroker = AddBroker(CoreType_name, initString + " --federates 1");
                AddFederates(ctor, newTypeString, 1, subbroker, time_delta, name_prefix);
            }
        } break;
    }
    helicsFederateInfoFree(fi);
}

helics_federate FederateTestFixture::GetFederateAt(int index)
{
    if (index < static_cast<int>(federates.size())) {
        return federates.at(index);
    }
    return nullptr;
}
