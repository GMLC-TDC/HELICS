/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include "helics/chelics.h"

#define CE(status) BOOST_CHECK_EQUAL (status, helics_ok)
#define HELICS_SIZE_MAX 512

#ifndef DISABLE_TCP_CORE
const std::string core_types[] = {"test",   "ipc",   "zmq",   "udp",   "tcp",
                                  "test_2", "ipc_2", "zmq_2", "udp_2", "tcp_2"};
const std::string core_types_single[] = {"test", "ipc", "tcp", "zmq", "udp"};
#else
const std::string core_types[] = {"test",  "ipc",    "zmq",   "udp",   "test_2", "ipc_2", "zmq_2",
                                  "udp_2", "test_3", "zmq_3", "udp_3", "test_4", "zmq_4", "udp_4"};
const std::string core_types_single[] = {"test", "ipc", "zmq", "udp", "test_3", "zmq_3", "udp_3"};
#endif

typedef helics_federate (*FedCreator) (helics_federate_info_t);

struct FederateTestFixture
{
    FederateTestFixture () = default;
    ~FederateTestFixture ();

    helics_broker AddBroker (const std::string &core_type_name, int count);
    helics_broker AddBroker (const std::string &core_type_name, const std::string &initialization_string);

    void SetupTest (FedCreator ctor,
                    const std::string &core_type_name,
                    int count,
                    helics_time_t time_delta = helics_time_zero,
                    const std::string &name_prefix = "fed")
    {
        helics_broker broker = AddBroker (core_type_name, count);
        BOOST_CHECK (nullptr != broker);
        AddFederates (ctor, core_type_name, count, broker, time_delta, name_prefix);
    }

    std::vector<helics_federate> AddFederates (FedCreator ctor,
                                               std::string core_type_name,
                                               int count,
                                               helics_broker broker,
                                               helics_time_t time_delta = helics_time_zero,
                                               const std::string &name_prefix = "fed")
    {
        char tmp[HELICS_SIZE_MAX];
        bool hasIndex = hasIndexCode (core_type_name);
        int setup = (hasIndex) ? getIndexCode (core_type_name) : 1;
        if (hasIndex)
        {
            core_type_name.pop_back ();
            core_type_name.pop_back ();
        }

        std::string initString = std::string ("--broker=");
        CE (helicsBrokerGetIdentifier (broker, tmp, HELICS_SIZE_MAX));
        initString += tmp;
        initString += " --broker_address=";
        CE (helicsBrokerGetAddress (broker, tmp, HELICS_SIZE_MAX));
        initString += tmp;

        if (!extraCoreArgs.empty ())
        {
            initString.push_back (' ');
            initString.append (extraCoreArgs);
        }

        helics_federate_info_t fi = helicsFederateInfoCreate ();
        CE (helicsFederateInfoSetFederateName (fi, ""));
        CE (helicsFederateInfoSetCoreTypeFromString (fi, core_type_name.c_str ()));
        CE (helicsFederateInfoSetTimeDelta (fi, time_delta));

        std::vector<helics_federate> federates_added;

        switch (setup)
        {
        case 1:
        default:
        {
            auto init = initString + " --federates " + std::to_string (count);
            auto core = helicsCreateCore (core_type_name.c_str (), NULL, init.c_str ());
            CE (helicsCoreGetIdentifier (core, tmp, HELICS_SIZE_MAX));
            CE (helicsFederateInfoSetCoreName (fi, tmp));
            size_t offset = federates.size ();
            federates.resize (count + offset);
            for (int ii = 0; ii < count; ++ii)
            {
                auto name = name_prefix + std::to_string (ii + offset);
                CE (helicsFederateInfoSetFederateName (fi, name.c_str ()));
                auto fed = ctor (fi);
                federates[ii + offset] = fed;
                federates_added.push_back (fed);
            }
            helicsCoreFree (core);
        }
        break;
        case 2:
        {  // each federate has its own core
            size_t offset = federates.size ();
            federates.resize (count + offset);
            for (int ii = 0; ii < count; ++ii)
            {
                auto init = initString + " --federates 1";
                auto core = helicsCreateCore (core_type_name.c_str (), NULL, init.c_str ());
                CE (helicsCoreGetIdentifier (core, tmp, HELICS_SIZE_MAX));
                CE (helicsFederateInfoSetCoreName (fi, tmp));

                auto name = name_prefix + std::to_string (ii + offset);
                CE (helicsFederateInfoSetFederateName (fi, name.c_str ()));
                auto fed = ctor (fi);
                federates[ii + offset] = fed;
                federates_added.push_back (fed);
                helicsCoreFree (core);
            }
        }
        break;
        case 3:
        {
            auto subbroker = AddBroker (core_type_name, initString + " --federates " + std::to_string (count));
            auto newTypeString = core_type_name;
            newTypeString.push_back ('_');
            newTypeString.push_back ('2');
            for (int ii = 0; ii < count; ++ii)
            {
                AddFederates (ctor, newTypeString, 1, subbroker, time_delta, name_prefix);
            }
        }
        break;
        case 4:
        {
            auto newTypeString = core_type_name;
            newTypeString.push_back ('_');
            newTypeString.push_back ('2');
            for (int ii = 0; ii < count; ++ii)
            {
                auto subbroker = AddBroker (core_type_name, initString + " --federates 1");
                AddFederates (ctor, newTypeString, 1, subbroker, time_delta, name_prefix);
            }
        }
        break;
        }

        return federates_added;
    }

    helics_federate GetFederateAt (int index)
    {
        if (index < static_cast<int> (federates.size ()))
        {
            return federates.at (index);
        }
        return nullptr;
    }

    std::vector<helics_broker> brokers;
    std::vector<helics_federate> federates;
    std::string extraCoreArgs;
    std::string extraBrokerArgs;

  private:
    bool hasIndexCode (const std::string &type_name);
    int getIndexCode (const std::string &type_name);
    auto AddBrokerImp (const std::string &core_type_name, const std::string &initialization_string);
};
