/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef TEST_FIXTURES_HEADER_
#define TEST_FIXTURES_HEADER_

#include <memory>

#include "helics/application_api/MessageFederate.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"

#ifndef DISABLE_TCP_CORE
const std::string travis_core_types[] = {"test",   "ipc",   "tcp",   "zmq",   "udp",
                                  "test_2", "ipc_2", "tcp_2", "zmq_2", "udp_2"};
#else
const std::string travis_core_types[] = {"test", "ipc", "zmq", "udp", "test_2", "ipc_2", "zmq_2", "udp_2"};
#endif

struct FederateTestFixture
{
    FederateTestFixture () = default;
    ~FederateTestFixture ();

    std::shared_ptr<helics::Broker> AddBroker (const std::string &core_type_name, int count);
    std::shared_ptr<helics::Broker>
    AddBroker (const std::string &core_type_name, const std::string &initialization_string);

    template <class FedType>
    void SetupSingleBrokerTest (std::string core_type_name,
                                int count,
                                helics::Time time_delta = helics::timeZero,
                                std::string name_prefix = "fed")
    {
        auto broker = AddBroker (core_type_name, count);
        AddFederates<FedType> (core_type_name, count, broker, time_delta, name_prefix);
    }

    template <class FedType>
    std::vector<std::shared_ptr<FedType>> AddFederates (std::string core_type_name,
                                                        int count,
                                                        std::shared_ptr<helics::Broker> broker,
                                                        helics::Time time_delta = helics::timeZero,
                                                        std::string name_prefix = "fed")
    {
        bool hasIndex = hasIndexCode (core_type_name);
        int setup = (hasIndex) ? getIndexCode (core_type_name) : 1;
        if (hasIndex)
        {
            core_type_name.pop_back ();
            core_type_name.pop_back ();
        }

        std::string initString =
          std::string ("--broker=") + broker->getIdentifier () + " --broker_address=" + broker->getAddress ();

        helics::FederateInfo fi ("", helics::coreTypeFromString (core_type_name));
        fi.timeDelta = time_delta;

        std::vector<std::shared_ptr<FedType>> federates_added;

        size_t offset = federates.size ();
        federates.resize (count + offset);

        switch (setup)
        {
        case 1:
        default:
        {
            fi.coreInitString = initString + " --federates " + std::to_string (count);

            for (int ii = 0; ii < count; ++ii)
            {
                fi.name = name_prefix + std::to_string (ii);
                auto fed = std::make_shared<FedType> (fi);
                federates[ii + offset] = fed;
                federates_added.push_back (fed);
            }
        }
        break;
        case 2:
        {  // each federate has its own core
            auto core_type = helics::coreTypeFromString (core_type_name);

            for (int ii = 0; ii < count; ++ii)
            {
                auto core = helics::CoreFactory::create (core_type, initString + " --federates 1");
                fi.coreName = core->getIdentifier ();

                fi.name = name_prefix + std::to_string (ii);
                auto fed = std::make_shared<FedType> (fi);
                federates[ii + offset] = fed;
                federates_added.push_back (fed);
            }
        }
        break;
        }

        return federates_added;
    }

    template <class FedType>
    std::shared_ptr<FedType> GetFederateAs (int index)
    {
        return std::dynamic_pointer_cast<FedType> (federates[index]);
    }

    std::vector<std::shared_ptr<helics::Broker>> brokers;
    std::vector<std::shared_ptr<helics::Federate>> federates;

  private:
    bool hasIndexCode (const std::string &type_name);
    int getIndexCode (const std::string &type_name);
    auto AddBrokerImp (const std::string &core_type_name, const std::string &initialization_string);
};

#endif