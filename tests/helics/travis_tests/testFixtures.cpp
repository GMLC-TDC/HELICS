/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "testFixtures.h"
#include <boost/test/unit_test.hpp>

#include "helics/core/BrokerFactory.h"
#include "helics/core/CoreFactory.h"
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

ValueFederateTestFixture::~ValueFederateTestFixture ()
{
    if (vFed1)
    {
        vFed1->finalize ();
    }
    vFed1 = nullptr;
    if (vFed2)
    {
        vFed2->finalize ();
    }
    vFed2 = nullptr;
    for (auto &brk : broker)
    {
        brk->disconnect ();
    }
    broker.clear ();
    helics::cleanupHelicsLibrary ();
}

void ValueFederateTestFixture::StartBroker (const std::string &core_type_name,
                                            const std::string &initialization_string)
{
    broker.push_back (StartBrokerImp (core_type_name, initialization_string));
}

void ValueFederateTestFixture::Setup1FederateTest (std::string core_type_name, helics::Time time_delta)
{
    if (hasIndexCode (core_type_name))
    {
        core_type_name.pop_back ();
        core_type_name.pop_back ();
    }

    StartBroker (core_type_name, "1");

    helics::FederateInfo fi ("test1");
    fi.coreType = helics::coreTypeFromString (core_type_name);
    fi.timeDelta = time_delta;
    fi.coreInitString = "--broker=" + broker[0]->getIdentifier () +
                        " --broker_address=" + broker[0]->getAddress () + " --federates 1";

    vFed1 = std::make_shared<helics::ValueFederate> (fi);
}

void ValueFederateTestFixture::Setup2FederateTest (std::string core_type_name, helics::Time time_delta)
{
    bool hasIndex = hasIndexCode (core_type_name);
    int setup = (hasIndex) ? getIndexCode (core_type_name) : 1;
    if (hasIndex)
    {
        core_type_name.pop_back ();
        core_type_name.pop_back ();
    }
    switch (setup)
    {
    case 1:
    default:
    {
        StartBroker (core_type_name, "2");

        helics::FederateInfo fi ("test1");
        fi.coreType = helics::coreTypeFromString (core_type_name);
        fi.timeDelta = time_delta;
        fi.coreInitString = std::string ("--broker=") + broker[0]->getIdentifier () +
                            " --broker_address=" + broker[0]->getAddress () + " --federates 2";

        vFed1 = std::make_shared<helics::ValueFederate> (fi);

        fi.name = "test2";
        vFed2 = std::make_shared<helics::ValueFederate> (fi);
    }
    break;
    case 2:
    {
        StartBroker (core_type_name, "2");

        std::string initString = std::string ("--broker=") + broker[0]->getIdentifier () +
                                 " --broker_address=" + broker[0]->getAddress () + " --federates 1";
        auto core_type = helics::coreTypeFromString (core_type_name);
        auto core1 = helics::CoreFactory::create (core_type, initString);
        auto core2 = helics::CoreFactory::create (core_type, initString);

        helics::FederateInfo fi ("test1");

        fi.coreName = core1->getIdentifier ();
        fi.coreType = helics::coreTypeFromString (core_type_name);
        fi.timeDelta = time_delta;

        vFed1 = std::make_shared<helics::ValueFederate> (fi);

        fi.name = "test2";
        fi.coreName = core2->getIdentifier ();
        vFed2 = std::make_shared<helics::ValueFederate> (fi);
    }
    case 3:

        break;
    }
}

MessageFederateTestFixture::~MessageFederateTestFixture ()
{
    if (mFed1)
    {
        mFed1->finalize ();
    }
    mFed1 = nullptr;
    if (mFed2)
    {
        mFed2->finalize ();
    }
    mFed2 = nullptr;
    for (auto &brk : broker)
    {
        brk->disconnect ();
    }
    broker.clear ();
    helics::cleanupHelicsLibrary ();
}

void MessageFederateTestFixture::StartBroker (const std::string &core_type_name,
                                              const std::string &initialization_string)
{
    broker.push_back (StartBrokerImp (core_type_name, initialization_string));
}

void MessageFederateTestFixture::Setup1FederateTest (const std::string &core_type_name)
{
    StartBroker (core_type_name, "1");

    helics::FederateInfo fi ("test1");
    fi.coreType = helics::coreTypeFromString (core_type_name);
    fi.coreInitString = std::string ("--broker=") + broker[0]->getIdentifier () +
                        " --broker_address=" + broker[0]->getAddress () + " --federates 1";

    mFed1 = std::make_shared<helics::MessageFederate> (fi);
}

void MessageFederateTestFixture::Setup2FederateTest (const std::string &core_type_name)
{
    StartBroker (core_type_name, "2");

    helics::FederateInfo fi ("test1");
    fi.coreType = helics::coreTypeFromString (core_type_name);
    fi.coreInitString = std::string ("--broker=") + broker[0]->getIdentifier () +
                        " --broker_address=" + broker[0]->getAddress () + " --federates 2";

    mFed1 = std::make_shared<helics::MessageFederate> (fi);

    fi.name = "test2";
    mFed2 = std::make_shared<helics::MessageFederate> (fi);
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
        if (fed && fed->currentState () != helics::Federate::op_states::finalize)
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
    auto broker = StartBrokerImp (core_type_name, initialization_string);
    brokers.push_back (broker);
    return broker;
}