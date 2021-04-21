/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../coreTypeLists.hpp"
#include "../src/helics/cpp98/Broker.hpp"
#include "../src/helics/cpp98/Core.hpp"
#include "../src/helics/cpp98/Federate.hpp"
#include "helics/helics-config.h"

#include <memory>
#include <string>
#include <vector>

#define HELICS_SIZE_MAX 512

struct FederateTestFixture_cpp {
    FederateTestFixture_cpp() = default;
    ~FederateTestFixture_cpp();

    std::shared_ptr<helicscpp::Broker> AddBroker(const std::string& core_type_name, int count);
    std::shared_ptr<helicscpp::Broker> AddBroker(const std::string& core_type_name,
                                                 const std::string& initialization_string);

    template<class FedType>
    void SetupTest(const std::string& core_type_name,
                   int count,
                   helics_time time_delta = helics_time_zero,
                   const std::string& name_prefix = "fed")
    {
        ctype = core_type_name;
        auto broker = AddBroker(core_type_name, count);
        if (!broker->isConnected()) {
            broker->disconnect();
            broker = nullptr;
            helicsCleanupLibrary();
            broker = AddBroker(core_type_name, count);
            if (!broker->isConnected()) {
                throw(std::runtime_error("Unable to connect rootbroker"));
            }
        }
        AddFederates<FedType>(core_type_name, count, *broker, time_delta, name_prefix);
    }

    template<class FedType>
    void AddFederates(std::string core_type_name,
                      int count,
                      helicscpp::Broker& broker,
                      helics_time time_delta = helics_time_zero,
                      const std::string& name_prefix = "fed")
    {
        bool hasIndex = hasIndexCode(core_type_name);
        int setup = (hasIndex) ? getIndexCode(core_type_name) : 1;
        if (hasIndex) {
            core_type_name.pop_back();
            core_type_name.pop_back();
        }

        std::string initString = std::string("--broker=");
        initString += broker.getIdentifier();
        initString += " --broker_address=";
        initString += broker.getAddress();

        if (!extraCoreArgs.empty()) {
            initString.push_back(' ');
            initString.append(extraCoreArgs);
        }

        helicscpp::FederateInfo fi;

        fi.setCoreType(core_type_name);
        fi.setProperty(helics_property_time_delta, time_delta);
        switch (setup) {
            case 1:
            default: {
                auto init = initString + " --federates " + std::to_string(count);
                helicscpp::Core core(core_type_name, std::string(), init);
                fi.setCoreName(core.getIdentifier());
                size_t offset = federates.size();
                federates.resize(count + offset);
                for (int ii = 0; ii < count; ++ii) {
                    auto name = name_prefix + std::to_string(ii + offset);
                    auto fed = std::make_shared<FedType>(name, fi);
                    federates[ii + offset] = fed;
                }
            } break;
            case 2: {  // each federate has its own core
                size_t offset = federates.size();
                federates.resize(count + offset);
                for (int ii = 0; ii < count; ++ii) {
                    auto init = initString + " --federates 1";
                    helicscpp::Core core(core_type_name, std::string(), init);
                    fi.setCoreName(core.getIdentifier());

                    auto name = name_prefix + std::to_string(ii + offset);
                    auto fed = std::make_shared<FedType>(name, fi);
                    federates[ii + offset] = fed;
                }
            } break;
            case 3: {
                auto subbroker =
                    AddBroker(core_type_name, initString + " --federates " + std::to_string(count));
                auto newTypeString = core_type_name;
                newTypeString.push_back('_');
                newTypeString.push_back('2');
                for (int ii = 0; ii < count; ++ii) {
                    AddFederates<FedType>(newTypeString, 1, *subbroker, time_delta, name_prefix);
                }
            } break;
            case 4: {
                auto newTypeString = core_type_name;
                newTypeString.push_back('_');
                newTypeString.push_back('2');
                for (int ii = 0; ii < count; ++ii) {
                    auto subbroker = AddBroker(core_type_name, initString + " --federates 1");
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
