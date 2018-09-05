/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "testFixtures.hpp"
#include <boost/test/unit_test.hpp>

#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"
#include <cctype>
#include <iostream>

bool hasIndexCode (const std::string &type_name)
{
    if (std::isdigit (type_name.back ()) != 0)
    {
        if (*(type_name.end () - 2) == '_')
        {  // this check ignores the setup mode
            return true;
        }
    }
    return false;
}

int getIndexCode (const std::string &type_name) { return static_cast<int> (type_name.back () - '0'); }

auto StartBrokerImp (const std::string &core_type_name, const std::string &initialization_string)
{
    helics::core_type type;
    if (hasIndexCode (core_type_name))
    {
        std::string new_type (core_type_name.begin (), core_type_name.end () - 2);
        type = helics::coreTypeFromString (new_type);
    }
    else
    {
        type = helics::coreTypeFromString(core_type_name);
    }
    if (type == helics::core_type::TCP)
    {
        return helics::BrokerFactory::create(type, initialization_string+" --reuse_address");
    }
    else
    {
        return helics::BrokerFactory::create(type, initialization_string);
    }
    
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
    helics::core_type type;
    if (hasIndexCode (core_type_name))
    {
        std::string new_type (core_type_name.begin (), core_type_name.end () - 2);
        type = helics::coreTypeFromString (new_type);
    }
    else
    {
        type = helics::coreTypeFromString(core_type_name);
    }
    if (type == helics::core_type::TCP)
    {
        return helics::BrokerFactory::create(type, initialization_string + " --reuse_address");
    }
    else
    {
        return helics::BrokerFactory::create(type, initialization_string);
    }
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
        if (ctype.compare(0, 3, "tcp") == 0)
        {
            broker->waitForDisconnect(2000);
        }
        else
        {
            broker->waitForDisconnect(200);
        }
        
		if (broker->isConnected())
		{
            std::cout << "forcing disconnect\n";
            broker->disconnect ();
		}
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
