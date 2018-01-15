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
    helics_federate vFed1;
    helics_federate vFed2;
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

        char buffer[512];
        helicsBrokerGetIdentifier(broker, buffer, 512);
        std::string initString = std::string("--broker=") + std::string(buffer) + " --broker_address=";
        helicsBrokerGetAddress(broker, buffer, 512);
        initString += std::string(buffer);
    	helics_federate_info_t fi = helicsFederateInfoCreate();
    	helics_status rv = helics_ok;
    	rv = helicsFederateInfoSetFederateName(fi, nullptr);
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
                FedType fed;
                if(fedType == valueFederate) {
                   fed = helicsCreateValueFederate(fi);
                } else if (fedType == combinationFederate) {
                    fed = helicsCreateCombinationFederate(fi);
                } else if (fedType == messageFederate) {
                   fed = helicsCreateMessageFederate(fi);
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
                auto core = helicsCreateCore(core_type_name.c_str(), "", coreInitString.c_str());
                helicsFederateInfoSetFederateName(fi, fedInfoName.c_str());

                helicsCoreGetIdentifier(core, buffer, 512);
                helicsFederateInfoSetCoreName(fi, buffer);
                //fi.coreName = core->getIdentifier();
                //fi.name = name_prefix + std::to_string(ii);
                FedType fed;
                if(fedType == valueFederate) {
                    fed = helicsCreateValueFederate(fi);
                } else if (fedType == combinationFederate) {
                    fed = helicsCreateCombinationFederate(fi);
                } else if (fedType == messageFederate) {
                   fed = helicsCreateMessageFederate(fi);
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
