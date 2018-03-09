/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.
*/
#pragma once

#include <memory>

#include "helics/application_api/MessageFederate.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"

#ifdef KEY_TESTS
#ifndef DISABLE_TCP_CORE
const std::string core_types[] = { "test",   "ipc",   "tcp",   "zmq",   "udp",
"test_2","zmq_4", "ipc_2","test_4", "tcp_2", "zmq_2", "udp_2","test_3","zmq_3" };
const std::string core_types_single[] = { "test",   "ipc",   "tcp",   "zmq",   "udp",
"test_3","zmq_3" };
#else
const std::string core_types[] = { "test", "ipc", "zmq", "udp", "test_2","zmq_4", "ipc_2","test_4", "zmq_2", "udp_2","test_3","zmq_3" };
const std::string core_types_single[] = { "test", "ipc", "zmq", "udp","test_3","zmq_3" };
#endif
#else // KEY_TESTS
#ifdef QUICK_TESTS_ONLY
#ifndef DISABLE_TCP_CORE
const std::string core_types[] = {"test", "ipc_2", "tcp", "test_2", "zmq", "udp","test_3","zmq_3" };
const std::string core_types_single[] = {"test", "ipc", "tcp", "zmq", "udp","test_3","zmq_3"};
#else
const std::string core_types[] = {"test", "ipc_2", "test_2", "zmq", "udp","test_3","zmq_3" };
const std::string core_types_single[] = {"test", "ipc", "zmq", "udp","test_3","zmq_3" };
#endif
#else // QUICK_TESTS_ONLY
#ifndef DISABLE_TCP_CORE
const std::string core_types[] = {"test",   "ipc",   "zmq",   "udp",   "tcp",
                                  "test_2", "ipc_2", "zmq_2", "udp_2", "tcp_2",
                                "test_3", "zmq_3", "udp_3", "tcp_3" ,
                                "test_4", "zmq_4", "udp_4", "tcp_4" };
const std::string core_types_single[] = {"test", "ipc", "tcp", "zmq", "udp",
                                        "test_3", "zmq_3", "udp_3", "tcp_3" };
#else
const std::string core_types[] = {"test", "ipc", "zmq", "udp", "test_2", "ipc_2", "zmq_2", "udp_2", "test_3", "zmq_3", "udp_3" ,
"test_4", "zmq_4", "udp_4" };
const std::string core_types_single[] = {"test", "ipc", "zmq", "udp","test_3", "zmq_3", "udp_3" };
#endif
#endif // QUICK_TESTS_ONLY

#endif //KEY_TESTS

const std::string defaultNamePrefix = "fed";


struct FederateTestFixture
{
    FederateTestFixture () = default;
    ~FederateTestFixture ();

    std::shared_ptr<helics::Broker> AddBroker (const std::string &core_type_name, int count);
    std::shared_ptr<helics::Broker>
    AddBroker (const std::string &core_type_name, const std::string &initialization_string);

    template <class FedType>
    void SetupTest (const std::string &core_type_name,
                                int count,
                                helics::Time time_delta = helics::timeZero,
                                const std::string &name_prefix = defaultNamePrefix)
    {
        auto broker = AddBroker (core_type_name, count);
        AddFederates<FedType> (core_type_name, count, broker, time_delta, name_prefix);
    }

    template <class FedType>
    std::vector<std::shared_ptr<FedType>> AddFederates (std::string core_type_name,
                                                        int count,
                                                        std::shared_ptr<helics::Broker> broker,
                                                        helics::Time time_delta = helics::timeZero,
                                                        const std::string &name_prefix = defaultNamePrefix)
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

        if (!extraCoreArgs.empty ())
        {
            initString.push_back (' ');
            initString.append (extraCoreArgs);
        }

        helics::FederateInfo fi (std::string(), helics::coreTypeFromString (core_type_name));
        fi.timeDelta = time_delta;

        std::vector<std::shared_ptr<FedType>> federates_added;



        switch (setup)
        {
        case 1:
        default:
        {
            auto core_type = helics::coreTypeFromString(core_type_name);
            auto core = helics::CoreFactory::create(core_type, initString + " --federates "+ std::to_string(count));
            fi.coreName = core->getIdentifier();
            size_t offset = federates.size();
            federates.resize(count + offset);
            for (int ii = 0; ii < count; ++ii)
            {
                fi.name = name_prefix + std::to_string (ii+offset);
                auto fed = std::make_shared<FedType> (fi);
                federates[ii + offset] = fed;
                federates_added.push_back (fed);
            }
        }
        break;
        case 2:
        {  // each federate has its own core
            auto core_type = helics::coreTypeFromString (core_type_name);
            size_t offset = federates.size();
            federates.resize(count + offset);
            for (int ii = 0; ii < count; ++ii)
            {
                auto core = helics::CoreFactory::create (core_type, initString + " --federates 1");
                fi.coreName = core->getIdentifier ();

                fi.name = name_prefix + std::to_string (ii+offset);
                auto fed = std::make_shared<FedType> (fi);
                federates[ii + offset] = fed;
                federates_added.push_back (fed);
            }
        }
        break;
        case 3:
        {
            auto subbroker = AddBroker(core_type_name, initString + " --federates " + std::to_string(count));
            auto newTypeString = core_type_name;
            newTypeString.push_back('_');
            newTypeString.push_back('2');
            for (int ii = 0; ii < count; ++ii)
            {
                AddFederates<FedType>(newTypeString, 1, subbroker, time_delta, name_prefix);
            }
        }
        break;
        case 4:
        {
            auto newTypeString = core_type_name;
            newTypeString.push_back('_');
            newTypeString.push_back('2');
            for (int ii = 0; ii < count; ++ii)
            {
                auto subbroker = AddBroker(core_type_name, initString + " --federates 1");
                AddFederates<FedType>(newTypeString, 1, subbroker, time_delta, name_prefix);
            }
        }
        break;
        case 5: //pairs of federates per core
        {
            auto core_type = helics::coreTypeFromString(core_type_name);
            size_t offset = federates.size();
            federates.resize(count + offset);
            for (int ii = 0; ii < count; ii += 2)
            {
                auto core = helics::CoreFactory::create(core_type, initString + " --federates " + std::to_string((ii < count - 1) ? 2 : 1));
                fi.coreName = core->getIdentifier();

                fi.name = name_prefix + std::to_string(ii + offset);
                auto fed = std::make_shared<FedType>(fi);
                federates[ii + offset] = fed;
                federates_added.push_back(fed);
                if (ii + 1 < count)
                {
                    fi.name = name_prefix + std::to_string(ii + offset + 1);
                    auto fed2 = std::make_shared<FedType>(fi);
                    federates[ii + offset + 1] = fed2;
                    federates_added.push_back(fed2);
                }
            }
        }
            break;
        case 6: //pairs of cores per subbroker
        {
            auto newTypeString = core_type_name;
            newTypeString.push_back('_');
            newTypeString.push_back('5');
            for (int ii = 0; ii < count; ii+=4)
            {
                int fedcnt = (ii > count - 3) ? 4 : (count - ii);
                auto subbroker = AddBroker(core_type_name, initString + " --federates "+std::to_string(fedcnt));
                AddFederates<FedType>(newTypeString, fedcnt, subbroker, time_delta, name_prefix);
            }
        }
        break;
        case 7: //two layers of subbrokers
        {
                auto newTypeString = core_type_name;
                newTypeString.push_back('_');
                newTypeString.push_back('4');
                for (int ii = 0; ii < count; ++ii)
                {
                    auto subbroker = AddBroker(core_type_name, initString + " --federates 1");
                    AddFederates<FedType>(newTypeString, 1, subbroker, time_delta, name_prefix);
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
    std::string extraCoreArgs;
    std::string extraBrokerArgs;

  private:
    bool hasIndexCode (const std::string &type_name);
    int getIndexCode (const std::string &type_name);
    auto AddBrokerImp (const std::string &core_type_name, const std::string &initialization_string);
};

