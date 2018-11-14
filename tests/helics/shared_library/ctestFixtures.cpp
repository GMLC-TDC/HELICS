/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "ctestFixtures.hpp"
#include "helics/shared_api_library/internal/api_objects.h"
#include <boost/test/unit_test.hpp>

#include <cctype>


#ifndef DISABLE_TCP_CORE
#ifdef HELICS_HAVE_ZEROMQ
const std::vector<std::string> core_types = {"test",   "ipc",   "zmq",   "udp",   "tcp",
                                  "test_2", "ipc_2", "zmq_2", "udp_2", "tcp_2"};
const std::vector<std::string> core_types_single = {"test", "ipc", "tcp", "zmq", "udp"};
#else
const std::vector<std::string> core_types = {"test",  "ipc",   "udp",   "tcp",  "test_2",
                                               "ipc_2", "zmq_2", "udp_2", "tcp_2"};
const std::vector<std::string> core_types_single = {"test", "ipc", "tcp", "udp"};
#endif
#else
#ifdef HELICS_HAVE_ZEROMQ
const std::vector<std::string> core_types = {"test", "ipc", "zmq", "udp", "test_2", "ipc_2", "zmq_2", "udp_2"};
const std::vector<std::string> core_types_single = {"test", "ipc", "zmq", "udp"};
#else
const std::vector<std::string> core_types = {"test", "ipc", "udp", "test_2", "ipc_2", "zmq_2", "udp_2"};
const std::vector<std::string> core_types_single = {"test", "ipc", "udp"};
#endif
#endif

static bool hasIndexCode (const std::string &type_name)
{
    if (std::isdigit (type_name.back ()) != 0)
    {
        if (*(type_name.end () - 2) == '_')
        {  // this setup ignores the setup mode
            return true;
        }
    }
    return false;
}

static auto StartBrokerImp (const std::string &core_type_name, std::string initialization_string)
{
    if (core_type_name.compare(0, 3, "tcp") == 0)
    {
        initialization_string += " --reuse_address";
    }
	else if (core_type_name.compare(0, 3, "ipc") == 0)
	{
        initialization_string += " --client";
	}
    if (hasIndexCode (core_type_name))
    {
        std::string new_type (core_type_name.begin (), core_type_name.end () - 2);
        return helicsCreateBroker (new_type.c_str (), NULL, initialization_string.c_str (),0);
    }
    return helicsCreateBroker (core_type_name.c_str (), NULL, initialization_string.c_str (),0);
}

bool FederateTestFixture::hasIndexCode (const std::string &type_name)
{
    if (std::isdigit (type_name.back ()) != 0)
    {
        if (*(type_name.end () - 2) == '_')
        {  // this setup ignores the setup mode
            return true;
        }
    }
    return false;
}

int FederateTestFixture::getIndexCode (const std::string &type_name)
{
    return static_cast<int> (type_name.back () - '0');
}

FederateTestFixture::FederateTestFixture () { helicsErrorClear (&err); }
FederateTestFixture::~FederateTestFixture ()
{
    for (auto &fed : federates)
    {
        if (fed)
        {
            federate_state state = helicsFederateGetState (fed, nullptr);
            helics_core core = helicsFederateGetCoreObject (fed,nullptr);
            if (state != helics_state_finalize)
            {
                helicsFederateFinalize (fed, nullptr);
            }
            helicsFederateFree (fed);
            if (helicsCoreIsValid(core))
            {
                helicsCoreDisconnect (core,nullptr);
            }
            helicsCoreFree (core);
            
        }
    }
    federates.clear ();
    

    for (auto &broker : brokers)
    {
        helics_bool_t res;
        if (ctype.compare(0, 3, "tcp") == 0)
        {
            res=helicsBrokerWaitForDisconnect(broker, 2000,nullptr);
        }
        else
        {
            res=helicsBrokerWaitForDisconnect(broker, 200,nullptr);
        }
        
		if (res != helics_true)
		{
            printf ("forcing disconnect\n");
            helicsBrokerDisconnect (broker,nullptr);
		}
        
        helicsBrokerFree (broker);
    }
    brokers.clear ();
    helicsCleanupLibrary ();
}

helics_broker FederateTestFixture::AddBroker (const std::string &core_type_name, int count)
{
    return AddBroker (core_type_name, std::to_string (count));
}

helics_broker
FederateTestFixture::AddBroker (const std::string &core_type_name, const std::string &initialization_string)
{
    helics_broker broker;
    if (extraBrokerArgs.empty ())
    {
        broker = StartBrokerImp (core_type_name, initialization_string);
    }
    else
    {
        broker = StartBrokerImp (core_type_name, initialization_string + " " + extraBrokerArgs);
    }
    BOOST_CHECK (nullptr != broker);
    auto BrokerObj = reinterpret_cast<helics::BrokerObject *> (broker);
    BOOST_CHECK (BrokerObj->valid == brokerValidationIdentifier);
    brokers.push_back (broker);
    return broker;
}


void FederateTestFixture::SetupTest (FedCreator ctor,
                const std::string &core_type_name,
                int count,
                helics_time_t time_delta,
                const std::string &name_prefix)
{
    ctype = core_type_name;
    helics_broker broker = AddBroker (core_type_name, count);
    BOOST_CHECK (nullptr != broker);
    AddFederates (ctor, core_type_name, count, broker, time_delta, name_prefix);
}

std::vector<helics_federate> FederateTestFixture::AddFederates (FedCreator ctor,
                                           std::string core_type_name,
                                           int count,
                                           helics_broker broker,
                                           helics_time_t time_delta,
                                           const std::string &name_prefix)
{
    bool hasIndex = hasIndexCode (core_type_name);
    int setup = (hasIndex) ? getIndexCode (core_type_name) : 1;
    if (hasIndex)
    {
        core_type_name.pop_back ();
        core_type_name.pop_back ();
    }

    std::string initString = std::string ("--broker=");
    initString += helicsBrokerGetIdentifier (broker);
    initString += " --broker_address=";
    initString += helicsBrokerGetAddress (broker);

    if (!extraCoreArgs.empty ())
    {
        initString.push_back (' ');
        initString.append (extraCoreArgs);
    }

    helics_federate_info_t fi = helicsCreateFederateInfo ();
    CE (helicsFederateInfoSetCoreTypeFromString (fi, core_type_name.c_str (), &err));
    CE (helicsFederateInfoSetTimeProperty (fi, TIME_DELTA_PROPERTY, time_delta, &err));

    std::vector<helics_federate> federates_added;

    switch (setup)
    {
    case 1:
    default:
    {
        auto init = initString + " --federates " + std::to_string (count);
        auto core = helicsCreateCore (core_type_name.c_str (), NULL, init.c_str (), &err);
        BOOST_REQUIRE_EQUAL (err.error_code, 0);
        CE (helicsFederateInfoSetCoreName (fi, helicsCoreGetIdentifier (core), &err));
        size_t offset = federates.size ();
        federates.resize (count + offset);
        for (int ii = 0; ii < count; ++ii)
        {
            auto name = name_prefix + std::to_string (ii + offset);
            auto fed = ctor (name.c_str (), fi, &err);
            BOOST_CHECK_EQUAL (err.error_code, 0);
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
            auto core = helicsCreateCore (core_type_name.c_str (), NULL, init.c_str (), &err);
            BOOST_REQUIRE_EQUAL (err.error_code, 0);
            CE (helicsFederateInfoSetCoreName (fi, helicsCoreGetIdentifier (core), &err));

            auto name = name_prefix + std::to_string (ii + offset);
            auto fed = ctor (name.c_str (), fi, &err);
            BOOST_CHECK_EQUAL (err.error_code, 0);
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
    helicsFederateInfoFree (fi);
    return federates_added;
}

helics_federate FederateTestFixture::GetFederateAt (int index)
{
    if (index < static_cast<int> (federates.size ()))
    {
        return federates.at (index);
    }
    return nullptr;
}
