/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include "../src/helics/cpp98/Broker.hpp"
#include "../src/helics/cpp98/Core.hpp"
#include "../src/helics/cpp98/Federate.hpp"
#include "helics/helics-config.h"
#include <memory>
#include <iostream>

#define HELICS_SIZE_MAX 512

#ifndef DISABLE_TCP_CORE
#ifdef HELICS_HAVE_ZEROMQ
const std::string core_types[] = {"test",   "ipc",   "zmq",   "udp",   "tcp",
                                  "test_2", "ipc_2", "zmq_2", "udp_2", "tcp_2"};
const std::string core_types_single[] = {"test", "ipc", "tcp", "zmq", "udp"};
#else
const std::string core_types[] = {"test", "ipc", "udp", "tcp", "test_2", "ipc_2", "zmq_2", "udp_2", "tcp_2"};
const std::string core_types_single[] = {"test", "ipc", "tcp", "udp"};
#endif
#else
#ifdef HELICS_HAVE_ZEROMQ
const std::string core_types[] = {"test", "ipc", "zmq", "udp", "test_2", "ipc_2", "zmq_2", "udp_2"};
const std::string core_types_single[] = {"test", "ipc", "zmq", "udp"};
#else
const std::string core_types[] = {"test", "ipc", "udp", "test_2", "ipc_2", "zmq_2", "udp_2"};
const std::string core_types_single[] = {"test", "ipc", "udp"};
#endif
#endif

struct FederateTestFixture_cpp
{
    FederateTestFixture_cpp () = default;
    ~FederateTestFixture_cpp ();

    std::shared_ptr<helics98::Broker> AddBroker (const std::string &core_type_name, int count);
    std::shared_ptr<helics98::Broker>
    AddBroker (const std::string &core_type_name, const std::string &initialization_string);

    template <class FedType>
    void SetupTest (const std::string &core_type_name,
                    int count,
                    helics_time_t time_delta = helics_time_zero,
                    const std::string &name_prefix = "fed")
    {
        auto broker = AddBroker (core_type_name, count);
        std::cout << "added broker" << std::endl;
        AddFederates<FedType> (core_type_name, count, *broker, time_delta, name_prefix);
    }

    template <class FedType>
    std::vector<std::shared_ptr<helics98::Federate>> AddFederates (std::string core_type_name,
                                                                 int count,
                                                                 helics98::Broker &broker,
                                                                 helics_time_t time_delta = helics_time_zero,
                                                                 const std::string &name_prefix = "fed")
    {
        bool hasIndex = hasIndexCode (core_type_name);
        int setup = (hasIndex) ? getIndexCode (core_type_name) : 1;
        if (hasIndex)
        {
            core_type_name.pop_back ();
            core_type_name.pop_back ();
        }

        std::string initString = std::string ("--broker=");
        initString += broker.getIdentifier ();
        initString += " --broker_address=";
        initString += broker.getAddress ();

        if (!extraCoreArgs.empty ())
        {
            initString.push_back (' ');
            initString.append (extraCoreArgs);
        }

        helics98::FederateInfo fi;
        fi.setFederateName ("");

        fi.setCoreTypeFromString (core_type_name);
        fi.setTimeDelta (time_delta);
        std::vector<std::shared_ptr<helics98::Federate>> federates_added;
        std::cout << "finished prelim" << std::endl;
        std::cout << initString << " " << broker << std::endl;
        switch (setup)
        {
        case 1:
        default:
        {
            auto init = initString + " --federates " + std::to_string (count);
            std::cout << "ctype="<<core_type_name<<" trying to make core from initString=" << init<<std::endl;
            helics98::Core core (core_type_name, std::string (), init);
            std::cout << "made core" << std::endl;
            fi.setCoreName (core.getIdentifier ());
            size_t offset = federates.size ();
            federates.resize (count + offset);
            for (int ii = 0; ii < count; ++ii)
            {
                auto name = name_prefix + std::to_string (ii + offset);
                fi.setFederateName (name);
                std::cout << "making federate" << std::endl;
                auto fed = std::make_shared<FedType> (fi);
                std::cout << "made federate" << std::endl;
                federates[ii + offset] = fed;
                federates_added.push_back (fed);
                std::cout << "pushed federate" << std::endl;
            }
        }
        break;
        case 2:
        {  // each federate has its own core
            size_t offset = federates.size ();
            federates.resize (count + offset);
            for (int ii = 0; ii < count; ++ii)
            {
                auto init = initString + " --federates 1";
                helics98::Core core (core_type_name, std::string (), init);
                fi.setCoreName (core.getIdentifier ());

                auto name = name_prefix + std::to_string (ii + offset);
                fi.setFederateName (name);
                auto fed = std::make_shared<FedType> (fi);
                federates[ii + offset] = fed;
                federates_added.push_back (fed);
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
                AddFederates<FedType> (newTypeString, 1, *subbroker, time_delta, name_prefix);
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
                AddFederates<FedType> (newTypeString, 1, *subbroker, time_delta, name_prefix);
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

    std::vector<std::shared_ptr<helics98::Broker>> brokers;
    std::vector<std::shared_ptr<helics98::Federate>> federates;
    std::string extraCoreArgs;
    std::string extraBrokerArgs;

  private:
    bool hasIndexCode (const std::string &type_name);
    int getIndexCode (const std::string &type_name);
    auto AddBrokerImp (const std::string &core_type_name, const std::string &initialization_string);
};
