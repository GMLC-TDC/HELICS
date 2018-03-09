/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "ctestFixtures.hpp"
#include "helics/shared_api_library/internal/api_objects.h"
#include <boost/test/unit_test.hpp>

#include <cctype>

bool hasIndexCode (const std::string &type_name)
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

int getIndexCode (const std::string &type_name) { return static_cast<int> (type_name.back () - '0'); }

auto StartBrokerImp (const std::string &core_type_name, const std::string &initialization_string)
{
    if (hasIndexCode (core_type_name))
    {
        std::string new_type (core_type_name.begin (), core_type_name.end () - 2);
        return helicsCreateBroker (new_type.c_str(), NULL, initialization_string.c_str());
    }
    return helicsCreateBroker (core_type_name.c_str(), NULL, initialization_string.c_str());
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

auto FederateTestFixture::AddBrokerImp (const std::string &core_type_name,
                                        const std::string &initialization_string)
{
    if (hasIndexCode (core_type_name))
    {
        std::string new_type (core_type_name.begin (), core_type_name.end () - 2);
        return helicsCreateBroker (new_type.c_str(), NULL, initialization_string.c_str());
    }

    return helicsCreateBroker (core_type_name.c_str(), NULL, initialization_string.c_str());
}

FederateTestFixture::~FederateTestFixture ()
{
    for (auto &fed : federates)
    {
        if (fed)
        {
            federate_state state;
            CE (helicsFederateGetState (fed, &state));
            if (state != helics_finalize_state)
            {
                CE (helicsFederateFinalize (fed));
            }
            helicsFederateFree (fed);
        }
    }
    federates.clear ();
    for (auto &broker : brokers)
    {
        CE (helicsBrokerDisconnect (broker));
        helicsBrokerFree (broker);
    }
    brokers.clear ();
    helicsCleanupHelicsLibrary ();
}

helics_broker FederateTestFixture::AddBroker (const std::string &core_type_name, int count)
{
    return AddBroker (core_type_name, std::to_string (count));
}

helics_broker FederateTestFixture::AddBroker (const std::string &core_type_name, const std::string &initialization_string)
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
    BOOST_CHECK(nullptr != broker);
    auto BrokerObj = reinterpret_cast<helics::BrokerObject *> (broker);
    BOOST_CHECK(BrokerObj->valid == brokerValidationIdentifier);
    brokers.push_back (broker);
    return broker;
}

