/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "ctestFixtures.h"
#include "MessageFederate.h"
#include "ValueFederate.h"
#include "helics.h"
#include <cctype>
#include <boost/test/unit_test.hpp>

bool hasIndexCode (const std::string &type_name)
{
    if (std::isdigit (type_name.back ()) == 1)
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
        // auto core_type = helics::coreTypeFromString (new_type);
        return helicsCreateBroker (new_type.c_str (), std::string ("").c_str (), initialization_string.c_str ());
    }
    else
    {
        // auto core_type = helics::coreTypeFromString (core_type_name);
        // return helics::BrokerFactory::create (core_type, initialization_string);
        return helicsCreateBroker (core_type_name.c_str (), std::string ("").c_str (),
                                   initialization_string.c_str ());
    }
}

ValueFederateTestFixture::~ValueFederateTestFixture ()
{
    if (vFed1)
    {
        // vFed1->finalize ();
        helicsFederateFinalize (vFed1);
        helicsFederateFree (vFed1);
    }

    if (vFed2)
    {
        // vFed2->finalize ();
        helicsFederateFinalize (vFed2);
        helicsFederateFree (vFed2);
    }
}

void ValueFederateTestFixture::StartBroker (const std::string &core_type_name,
                                            const std::string &initialization_string)
{
    broker = StartBrokerImp (core_type_name, initialization_string);
}

void ValueFederateTestFixture::Setup1FederateTest (std::string core_type_name, helics_time_t time_delta)
{
    if (hasIndexCode (core_type_name))
    {
        core_type_name.pop_back ();
        core_type_name.pop_back ();
    }

    StartBroker (core_type_name, "1");
    helics_federate_info_t fi = helicsFederateInfoCreate ();
    helicsFederateInfoSetFederateName (fi, std::string ("test1").c_str ());
    helicsFederateInfoSetCoreTypeFromString (fi, core_type_name.c_str ());
    helicsFederateInfoSetTimeDelta (fi, time_delta);
    char buffer[512];
    helicsBrokerGetIdentifier (broker, buffer, 512);
    std::string coreInitString = std::string ("--broker=") + std::string (buffer) + " --broker_address=";
    helicsBrokerGetAddress (broker, buffer, 512);
    coreInitString += std::string (buffer) + " --federates 1";

    helicsFederateInfoSetCoreInitString (fi, coreInitString.c_str ());
    // helics::FederateInfo fi ("test1");
    // fi.coreType = helics::coreTypeFromString (core_type_name);
    // fi.timeDelta = time_delta;
    // fi.coreInitString =
    //  "--broker=" + broker->getIdentifier () + " --broker_address=" + broker->getAddress () + " --federates 1";

    vFed1 = helicsCreateValueFederate (fi);
}

void ValueFederateTestFixture::Setup2FederateTest (std::string core_type_name, helics_time_t time_delta)
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
        helics_federate_info_t fi = helicsFederateInfoCreate ();
        helicsFederateInfoSetFederateName (fi, std::string ("test1").c_str ());
        helicsFederateInfoSetCoreTypeFromString (fi, core_type_name.c_str ());
        helicsFederateInfoSetTimeDelta (fi, time_delta);

        char buffer[512];
        helicsBrokerGetIdentifier (broker, buffer, 512);
        std::string coreInitString = std::string ("--broker=") + std::string (buffer) + " --broker_address=";
        helicsBrokerGetAddress (broker, buffer, 512);
        coreInitString += std::string (buffer) + " --federates 2";

        helicsFederateInfoSetCoreInitString (fi, coreInitString.c_str ());
        // helics::FederateInfo fi ("test1");
        // fi.coreType = helics::coreTypeFromString (core_type_name);
        // fi.timeDelta = time_delta;
        // fi.coreInitString = std::string ("--broker=") + broker->getIdentifier () +
        //                    " --broker_address=" + broker->getAddress () + " --federates 2";

        vFed1 = helicsCreateValueFederate (fi);

        helicsFederateInfoSetFederateName (fi, std::string ("test1").c_str ());
        // fi.name = "test2";
        vFed2 = helicsCreateValueFederate (fi);
    }
    break;
    case 2:
    {
        StartBroker (core_type_name, "2");

        char buffer[512];
        helicsBrokerGetIdentifier (broker, buffer, 512);
        std::string initString = std::string ("--broker=") + std::string (buffer) + " --broker_address=";
        helicsBrokerGetAddress (broker, buffer, 512);
        initString += std::string (buffer) + " --federates 1";

        // auto core_type = helics::coreTypeFromString (core_type_name);
        auto core1 = helicsCreateCore (core_type_name.c_str (), "", initString.c_str ());
        auto core2 = helicsCreateCore (core_type_name.c_str (), "", initString.c_str ());

        helics_federate_info_t fi = helicsFederateInfoCreate ();
        helicsFederateInfoSetFederateName (fi, "test1");
        helicsFederateInfoSetCoreTypeFromString (fi, core_type_name.c_str ());
        helicsFederateInfoSetTimeDelta (fi, time_delta);
        // helics::FederateInfo fi ("test1");

        helicsCoreGetIdentifier (core1, buffer, 512);
        helicsFederateInfoSetCoreName (fi, buffer);
        // fi.coreName = core1->getIdentifier ();
        // fi.coreType = helics::coreTypeFromString (core_type_name);
        // fi.timeDelta = time_delta;

        vFed1 = helicsCreateValueFederate (fi);

        helicsFederateInfoSetFederateName (fi, "test2");
        helicsCoreGetIdentifier (core2, buffer, 512);
        helicsFederateInfoSetCoreName (fi, buffer);
        // fi.name = "test2";
        // fi.coreName = core2->getIdentifier ();
        vFed2 = helicsCreateValueFederate (fi);
    }
    break;
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
    if (hasIndexCode (core_type_name))
    {
        std::string new_type (core_type_name.begin (), core_type_name.end () - 2);
        // auto core_type = helics::coreTypeFromString (new_type);
        return helicsCreateBroker (new_type.c_str (), "", initialization_string.c_str ());
    }
    else
    {
        // auto core_type = helics::coreTypeFromString (core_type_name);
        return helicsCreateBroker (core_type_name.c_str (), "", initialization_string.c_str ());
    }
}

FederateTestFixture::~FederateTestFixture ()
{
    helics_status rv = helics_ok;
    for (auto &fed : federates)
    {
        // if (fed && fed->getCurrentState () != helics::Federate::op_states::finalize)
        if (fed && helicsFederateEnterExecutionMode (fed) != rv)
        {
            // fed->finalize ();
            rv = helicsFederateFinalize (fed);
            helicsFederateFree (fed);
        }
    }

    for (auto &broker : brokers)
    {
        // broker->disconnect ();
        helicsBrokerFree (broker);
    }
}

helics_broker FederateTestFixture::AddBroker (const std::string &core_type_name, int count)
{
    return AddBroker (core_type_name, std::to_string (count));
}

helics_broker
FederateTestFixture::AddBroker (const std::string &core_type_name, const std::string &initialization_string)
{
    auto broker = StartBrokerImp (core_type_name, initialization_string);
    brokers.push_back (broker);
    return broker;
}
