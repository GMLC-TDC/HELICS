/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/
#include "testFixtures.hpp"
#include <boost/test/unit_test.hpp>

#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"
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
        auto core_type = helics::coreTypeFromString (new_type);
        return helics::BrokerFactory::create (core_type, initialization_string);
    }
    auto core_type = helics::coreTypeFromString (core_type_name);
    return helics::BrokerFactory::create (core_type, initialization_string);
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
        auto core_type = helics::coreTypeFromString (new_type);
        return helics::BrokerFactory::create (core_type, initialization_string);
    }

    auto core_type = helics::coreTypeFromString (core_type_name);
    return helics::BrokerFactory::create (core_type, initialization_string);
}

FederateTestFixture::~FederateTestFixture ()
{
    for (auto &fed : federates)
    {
        if (fed && fed->getCurrentState () != helics::Federate::op_states::finalize)
        {
            fed->finalize ();
        }
    }
    federates.clear ();
    for (auto &broker : brokers)
    {
        broker->disconnect ();
    }
    brokers.clear ();
    helics::cleanupHelicsLibrary ();
}

std::shared_ptr<helics::Broker> FederateTestFixture::AddBroker (const std::string &core_type_name, int count)
{
    return AddBroker (core_type_name, std::to_string (count));
}

std::shared_ptr<helics::Broker>
FederateTestFixture::AddBroker (const std::string &core_type_name, const std::string &initialization_string)
{
    std::shared_ptr<helics::Broker> broker;
    if (extraBrokerArgs.empty ())
    {
        broker = StartBrokerImp (core_type_name, initialization_string);
    }
    else
    {
        broker = StartBrokerImp (core_type_name, initialization_string + " " + extraBrokerArgs);
    }
    brokers.push_back (broker);
    return broker;
}

