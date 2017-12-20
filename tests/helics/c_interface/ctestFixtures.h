/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef TEST_FIXTURES_HEADER_
#define TEST_FIXTURES_HEADER_

#include <memory>
#include <string>
#include <vector>

#include "helics.h"
#include "MessageFederate_c.h"
#include "ValueFederate_c.h"
#include "../core/CoreFactory.h"
#include "../application_api/MessageFederate.h"
//#include "../application_api/MessageFilterFederate.h"
#include "../application_api/ValueFederate.h"
#include "../core/CoreBroker.h"
#include "../core/BrokerFactory.h"

typedef enum {
    valueFederate,
    combinationFederate,
    messageFederate,
}helicsFederateType;

struct ValueFederateTestFixture
{
    ValueFederateTestFixture () = default;
    ~ValueFederateTestFixture();

    void Setup1FederateTest(std::string core_type_name, helics_time_t time_delta = 0.0);
    void Setup2FederateTest(std::string core_type_name, helics_time_t time_delta = 0.0);

    void StartBroker(const std::string &core_type_name, const std::string &initialization_string);

    helics_broker broker;
    helics_value_federate vFed1;
    helics_value_federate vFed2;
};

struct MessageFederateTestFixture
{
    MessageFederateTestFixture () = default;
    ~MessageFederateTestFixture();

    void Setup1FederateTest(const std::string &core_type_name);
    void Setup2FederateTest(const std::string &core_type_name);

    void StartBroker(const std::string &core_type_name, const std::string &initialization_string);

    helics_broker broker;
    helics_message_federate mFed1;
    helics_message_federate mFed2;
};

struct FederateTestFixture
{
    FederateTestFixture () = default;
    ~FederateTestFixture();

    helics_broker AddBroker(const std::string &core_type_name, int count);
    helics_broker AddBroker(const std::string &core_type_name, const std::string &initialization_string);

    template<typename FedType>
    void SetupSingleBrokerTest(std::string core_type_name, int count, helicsFederateType fedType, helics_time_t time_delta =
    0.0, std::string name_prefix = "fed")
    {
        auto broker = AddBroker(core_type_name, count);
        AddFederates<FedType>(core_type_name, count, broker, fedType, time_delta, name_prefix);
    }

    template<typename FedType>
    std::vector<FedType> AddFederates(std::string core_type_name, int count,
    helics_broker broker, helicsFederateType fedType, helics_time_t time_delta = 0.0, std::string
    name_prefix = "fed")
    {
        bool hasIndex = hasIndexCode(core_type_name);
        int setup = (hasIndex) ? getIndexCode(core_type_name) : 1;
        if (hasIndex)
        {
            core_type_name.pop_back();
            core_type_name.pop_back();
        }

    	std::string initString = std::string("--broker=") + broker->getIdentifier() + " --broker_address=" +
    			broker->getAddress();
    	helics_federate_info_t fi = helicsFederateInfoCreate();
    	helicsStatus rv = helicsOK;
    	rv = helicsFederateInfoSetFederateName(fi, std::string("").c_str());
    	rv = helicsFederateInfoSetCoreTypeFromString(fi, core_type_name.c_str());
    	rv = helicsFederateInfoSetTimeDelta(fi, time_delta);
        //helics::FederateInfo fi("", helics::coreTypeFromString(core_type_name));
        //fi.timeDelta = time_delta;

        std::vector<FedType> federates_added;

        size_t offset = federates.size();
        federates.resize(count + offset);

        switch (setup)
        {
        case 1:
        default:
        {
        	std::string coreInitString = initString+ " --federates " + std::to_string(count);
            //fi.coreInitString = initString+ " --federates " + std::to_string(count);

            for (int ii = 0; ii < count; ++ii)
            {
                std::string fedName = name_prefix + std::to_string(ii);
                rv = helicsFederateInfoSetFederateName(fi, fedName.c_str());
            	//fi.name = name_prefix + std::to_string(ii);
                if(fedType == valueFederate) {
                    auto fed = helicsCreateValueFederate(fi);
                } else if (fedType == combinationFederate) {
                    auto fed = helicsCreateCombinationFederate(fi);
               // } else if (fedType == messageFederate) {
                   // auto fed = helicsCreateMessageFederate(fi);
               // } else if (fedType == messageFilterFederate) {
                    //auto fed = helicsCreateMessageFilterFederate(fi);
                }
                federates[ii+offset] = fed;
                federates_added.push_back(fed);
            }
        }
        break;
        case 2:
        { //each federate has its own core
            //auto core_type = helics::coreTypeFromString(core_type_name);

            for (int ii = 0; ii < count; ++ii)
            {
            	std::string coreInitString = initString+ " --federates 1";
            	std::string fedInfoName = name_prefix + std::to_string(ii);
                auto core = helicsCreateCore(core_type_name.c_str(), std::string("").c_str(), coreInitString.c_str());
                rv = helicsFederateInfoSetFederateName(fi, fedInfoName.c_str());
                fi.coreName = core->getIdentifier();
                //fi.name = name_prefix + std::to_string(ii);
                auto fed = FedType (fi);
                if(fedType == valueFederate) {
                    auto fed = helicsCreateValueFederate(fi);
                } else if (fedType == combinationFederate) {
                    auto fed = helicsCreateCombinationFederate(fi);
               // } else if (fedType == messageFederate) {
                   // auto fed = helicsCreateMessageFederate(fi);
               // } else if (fedType == messageFilterFederate) {
                    //auto fed = helicsCreateMessageFilterFederate(fi);
                }
                federates[ii+offset] = fed;
                federates_added.push_back(fed);
            }
        }
        break;
        }

        return federates_added;
    }

    template<typename FedType>
    FedType GetFederateAs(int index)
    {
        return FedType (federates[index]);
    }

    std::vector<helics_broker> brokers;
    std::vector<helics_federate> federates;

    private:
    	bool hasIndexCode(const std::string &type_name);
    	int getIndexCode(const std::string &type_name);
    	auto AddBrokerImp(const std::string &core_type_name, const std::string &initialization_string);
};
#endif
