/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../coreTypeLists.hpp"
#include "helics/cpp98/Broker.hpp"
#include "helics/cpp98/Core.hpp"
#include "helics/cpp98/Federate.hpp"
#include "helics/helics-config.h"

#include <memory>
#include <string>
#include <vector>

#define HELICS_SIZE_MAX 512

struct FederateTestFixture_cpp {
    FederateTestFixture_cpp() = default;
    ~FederateTestFixture_cpp();

    std::shared_ptr<helicscpp::Broker> AddBroker(const std::string& CoreType_name, int count);
    std::shared_ptr<helicscpp::Broker> AddBroker(const std::string& CoreType_name,
                                                 const std::string& initialization_string);

    template<class FedType>
    void SetupTest(const std::string& CoreType_name,
                   int count,
                   HelicsTime time_delta = HELICS_TIME_ZERO,
                   const std::string& name_prefix = "fed")
    {
        ctype = CoreType_name;
        auto broker = AddBroker(CoreType_name, count);
        if (!broker->isConnected()) {
            broker->disconnect();
            broker = nullptr;
            helicsCleanupLibrary();
            broker = AddBroker(CoreType_name, count);
            if (!broker->isConnected()) {
                throw(std::runtime_error("Unable to connect rootbroker"));
            }
        }
        AddFederates<FedType>(CoreType_name, count, *broker, time_delta, name_prefix);
    }

    template<class FedType>
    void AddFederates(std::string CoreType_name,
                      int count,
                      helicscpp::Broker& broker,
                      HelicsTime time_delta = HELICS_TIME_ZERO,
                      const std::string& name_prefix = "fed")
    {
        bool hasIndex = hasIndexCode(CoreType_name);
        int setup = (hasIndex) ? getIndexCode(CoreType_name) : 1;
        if (hasIndex) {
            CoreType_name.pop_back();
            CoreType_name.pop_back();
        }

        std::string initString = std::string("--broker=");
        initString += broker.getIdentifier();
        initString += " --broker_address=";
        initString += broker.getAddress();

        if (!extraCoreArgs.empty()) {
            initString.push_back(' ');
            initString.append(extraCoreArgs);
        }

        helicscpp::FederateInfo fedInfo;

        fedInfo.setCoreType(CoreType_name);
        fedInfo.setProperty(HELICS_PROPERTY_TIME_DELTA, time_delta);
        switch (setup) {
            case 1:
            default: {
                auto init = initString + " --federates " + std::to_string(count);
                helicscpp::Core core(CoreType_name, std::string(), init);
                fedInfo.setCoreName(core.getIdentifier());
                size_t offset = federates.size();
                federates.resize(count + offset);
                for (int ii = 0; ii < count; ++ii) {
                    auto name = name_prefix + std::to_string(ii + offset);
                    auto fed = std::make_shared<FedType>(name, fedInfo);
                    federates[ii + offset] = fed;
                }
            } break;
            case 2: {  // each federate has its own core
                size_t offset = federates.size();
                federates.resize(count + offset);
                for (int ii = 0; ii < count; ++ii) {
                    auto init = initString + " --federates 1";
                    helicscpp::Core core(CoreType_name, std::string(), init);
                    fedInfo.setCoreName(core.getIdentifier());

                    auto name = name_prefix + std::to_string(ii + offset);
                    auto fed = std::make_shared<FedType>(name, fedInfo);
                    federates[ii + offset] = fed;
                }
            } break;
            case 3: {
                auto subbroker =
                    AddBroker(CoreType_name, initString + " --federates " + std::to_string(count));
                auto newTypeString = CoreType_name;
                newTypeString.push_back('_');
                newTypeString.push_back('2');
                for (int ii = 0; ii < count; ++ii) {
                    AddFederates<FedType>(newTypeString, 1, *subbroker, time_delta, name_prefix);
                }
            } break;
            case 4: {
                auto newTypeString = CoreType_name;
                newTypeString.push_back('_');
                newTypeString.push_back('2');
                for (int ii = 0; ii < count; ++ii) {
                    auto subbroker = AddBroker(CoreType_name, initString + " --federates 1");
                    AddFederates<FedType>(newTypeString, 1, *subbroker, time_delta, name_prefix);
                }
            } break;
        }
    }

    template<class FedType>
    std::shared_ptr<FedType> GetFederateAs(int index)
    {
        return std::dynamic_pointer_cast<FedType>(federates[index]);
    }

    std::vector<std::shared_ptr<helicscpp::Broker>> brokers;
    std::vector<std::shared_ptr<helicscpp::Federate>> federates;
    std::string extraCoreArgs;
    std::string extraBrokerArgs;
    std::string ctype;

  private:
    bool hasIndexCode(const std::string& type_name);
    int getIndexCode(const std::string& type_name);
};
