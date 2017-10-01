/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef TEST_FIXTURES_HEADER_
#define TEST_FIXTURES_HEADER_

#include <memory>

#include "helics/core/CoreBroker.h"
#include "helics/core/CoreFactory.h"
#include "helics/application_api/MessageFederate.h"
#include "helics/application_api/MessageFilterFederate.h"
#include "helics/application_api/ValueFederate.h"

struct ValueFederateTestFixture
{
	ValueFederateTestFixture() = default;
	~ValueFederateTestFixture();

    void Setup1FederateTest(std::string core_type_name, helics::Time time_delta = helics::timeZero);
    void Setup2FederateTest(std::string core_type_name, helics::Time time_delta = helics::timeZero);

    void StartBroker(const std::string &core_type_name, const std::string &initialization_string);

    std::shared_ptr<helics::CoreBroker> broker;
    std::shared_ptr<helics::ValueFederate> vFed1;
    std::shared_ptr<helics::ValueFederate> vFed2;
};

struct MessageFederateTestFixture
{
    MessageFederateTestFixture() = default;
    ~MessageFederateTestFixture();

    void Setup1FederateTest(const std::string &core_type_name);
    void Setup2FederateTest(const std::string &core_type_name);

    void StartBroker(const std::string &core_type_name, const std::string &initialization_string);

    std::shared_ptr<helics::CoreBroker> broker;
    std::shared_ptr<helics::MessageFederate> mFed1;
    std::shared_ptr<helics::MessageFederate> mFed2;
};

struct FederateTestFixture
{
    FederateTestFixture() = default;
    ~FederateTestFixture();

    std::shared_ptr<helics::CoreBroker> AddBroker(const std::string &core_type_name, int count);
    std::shared_ptr<helics::CoreBroker> AddBroker(const std::string &core_type_name, const std::string &initialization_string);

    template<class FedType>
    void SetupSingleBrokerTest(std::string core_type_name, int count, helics::Time time_delta = helics::timeZero, std::string name_prefix = "fed")
    {
        auto broker = AddBroker(core_type_name, count);
        AddFederates<FedType>(core_type_name, count, broker, time_delta, name_prefix);
    }

    template<class FedType>
    std::vector<std::shared_ptr<FedType>> AddFederates(std::string core_type_name, int count, std::shared_ptr<helics::CoreBroker> broker, helics::Time time_delta = helics::timeZero, std::string name_prefix = "fed")
    {
        bool hasIndex = hasIndexCode(core_type_name);
        int setup = (hasIndex) ? getIndexCode(core_type_name) : 1;
        if (hasIndex)
        {
            core_type_name.pop_back();
            core_type_name.pop_back();
        }

        std::string initString = std::string("--broker=") + broker->getIdentifier() + " --broker_address=" + broker->getAddress() + " --federates " + std::to_string(count);

        helics::FederateInfo fi("", helics::coreTypeFromString(core_type_name));
        fi.timeDelta = time_delta;

        std::vector<std::shared_ptr<FedType>> federates_added;

        size_t offset = federates.size();
        federates.resize(count + offset);

        switch (setup)
        {
        case 1:
        default:
        {
            fi.coreInitString = initString;

            for (int ii = 0; ii < count; ++ii)
            {
                fi.name = name_prefix + std::to_string(ii);
                auto fed = std::make_shared<FedType>(fi);
                federates[ii+offset] = fed;
                federates_added.push_back(fed);
            }
        }
        break;
        case 2:
        {
            auto core_type = helics::coreTypeFromString(core_type_name);

            federates.resize(count+federates.size());
            for (int ii = 0; ii < count; ++ii)
            {
                auto core = helics::CoreFactory::create(core_type, initString);
                fi.coreName = core->getIdentifier();

                fi.name = name_prefix + std::to_string(ii);
                auto fed = std::make_shared<FedType>(fi);
                federates[ii+offset] = fed;
                federates_added.push_back(fed);
            }
        }
        break;
        }

        return federates_added;
    }

    template<class FedType>
    std::shared_ptr<FedType> GetFederateAs(int index)
    {
        return std::dynamic_pointer_cast<FedType>(federates[index]);
    }

    std::vector<std::shared_ptr<helics::CoreBroker>> brokers;
    std::vector<std::shared_ptr<helics::Federate>> federates;

private:
    bool hasIndexCode(const std::string &type_name);
    int getIndexCode(const std::string &type_name);
    auto AddBrokerImp(const std::string &core_type_name, const std::string &initialization_string);
};

#endif