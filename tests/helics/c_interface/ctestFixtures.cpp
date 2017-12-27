/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "ctestFixtures.h"
#include <boost/test/unit_test.hpp>
#include "../core/CoreBroker.h"
#include "../core/BrokerFactory.h"
#include <cctype>

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
        //auto core_type = helics::coreTypeFromString (new_type);
        return helicsCreateBroker(new_type.c_str(), std::string("").c_str(), initialization_string.c_str());
    }
    else
    {
        //auto core_type = helics::coreTypeFromString (core_type_name);
        //return helics::BrokerFactory::create (core_type, initialization_string);
        return helicsCreateBroker(core_type_name.c_str(), std::string("").c_str(), initialization_string.c_str());
    }
}

 ValueFederateTestFixture::~ValueFederateTestFixture ()
{
    helicsStatus rv = helicsOK;
    if (vFed1)
    {
        //vFed1->finalize ();
        rv = helicsFederateFinalize(vFed1);
        helicsFreeFederate(vFed1);
    }

    if (vFed2)
    {
        //vFed2->finalize ();
        rv = helicsFederateFinalize(vFed2);
        helicsFreeFederate(vFed2);
    }
}

 void ValueFederateTestFixture::StartBroker (const std::string &core_type_name,
                                            const std::string &initialization_string)
{
    broker = StartBrokerImp (core_type_name, initialization_string);
}

 void ValueFederateTestFixture::Setup1FederateTest (std::string core_type_name, helics_time_t time_delta)
{
	helicsStatus rv = helicsOK;
    if (hasIndexCode (core_type_name))
    {
        core_type_name.pop_back ();
        core_type_name.pop_back ();
    }

    StartBroker (core_type_name, "1");
    helics_federate_info_t fi = helicsFederateInfoCreate();
    rv = helicsFederateInfoSetFederateName(fi, std::string("test1").c_str());
    rv = helicsFederateInfoSetCoreTypeFromString(fi, core_type_name.c_str());
    rv = helicsFederateInfoSetTimeDelta(fi, time_delta);
    //std::string coreInitString = "--broker=" + broker->getIdentifier() + " --broker_address=" + broker->getAddress () + " --federates 1"; 
	std::string coreInitString=" --federates 1";
	rv = helicsFederateInfoSetCoreInitString(fi, coreInitString.c_str());
    //helics::FederateInfo fi ("test1");
    //fi.coreType = helics::coreTypeFromString (core_type_name);
    //fi.timeDelta = time_delta;
    //fi.coreInitString =
    //  "--broker=" + broker->getIdentifier () + " --broker_address=" + broker->getAddress () + " --federates 1";

    vFed1 = helicsCreateValueFederate(fi);
}

 void ValueFederateTestFixture::Setup2FederateTest (std::string core_type_name, helics_time_t time_delta)
{
	helicsStatus rv = helicsOK;
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
        helics_federate_info_t fi = helicsFederateInfoCreate();
        rv = helicsFederateInfoSetFederateName(fi, std::string("test1").c_str());
        rv = helicsFederateInfoSetCoreTypeFromString(fi, core_type_name.c_str());
        rv = helicsFederateInfoSetTimeDelta(fi, time_delta);
        //std::string coreInitString = "--broker=" + broker->getIdentifier () + " --broker_address=" + broker->getAddress () + " --federates 2";
		std::string coreInitString = " --federates 2";
        rv = helicsFederateInfoSetCoreInitString(fi, coreInitString.c_str());
        //helics::FederateInfo fi ("test1");
        //fi.coreType = helics::coreTypeFromString (core_type_name);
        //fi.timeDelta = time_delta;
        //fi.coreInitString = std::string ("--broker=") + broker->getIdentifier () +
        //                    " --broker_address=" + broker->getAddress () + " --federates 2";

        vFed1 = helicsCreateValueFederate(fi);

        rv = helicsFederateInfoSetFederateName(fi, std::string("test1").c_str());
        //fi.name = "test2";
        vFed2 = helicsCreateValueFederate(fi);
    }
    break;
    case 2:
    {
        StartBroker (core_type_name, "2");

        /*std::string initString = std::string ("--broker=") + broker->getIdentifier () +
                                 " --broker_address=" + broker->getAddress () + " --federates 1";*/
	    std::string initString = "--federates 1";
        //auto core_type = helics::coreTypeFromString (core_type_name);
        auto core1 = helicsCreateCore (core_type_name.c_str(), std::string("").c_str(), initString.c_str());
        auto core2 = helicsCreateCore (core_type_name.c_str(), std::string("").c_str(), initString.c_str());

        helics_federate_info_t fi = helicsFederateInfoCreate();
        rv = helicsFederateInfoSetFederateName(fi, std::string("test1").c_str());
        rv = helicsFederateInfoSetCoreTypeFromString(fi, core_type_name.c_str());
        rv = helicsFederateInfoSetTimeDelta(fi, time_delta);
        //helics::FederateInfo fi ("test1");

        //fi.coreName = core1->getIdentifier ();
        //fi.coreType = helics::coreTypeFromString (core_type_name);
        //fi.timeDelta = time_delta;

        vFed1 = helicsCreateValueFederate(fi);

        rv = helicsFederateInfoSetFederateName(fi, std::string("test2").c_str());
        //fi.name = "test2";
        //fi.coreName = core2->getIdentifier ();
        vFed2 = helicsCreateValueFederate(fi);
    }
    break;
    }
}

 MessageFederateTestFixture::~MessageFederateTestFixture ()
{
	helicsStatus rv = helicsOK;
    if (mFed1)
    {
        //mFed1->finalize ();
        rv = helicsFederateFinalize(mFed1);
        helicsFreeFederate(mFed1);
    }

    if (mFed2)
    {
        //mFed2->finalize ();
        rv = helicsFederateFinalize(mFed1);
        helicsFreeFederate(mFed1);
    }
}

 void MessageFederateTestFixture::StartBroker (const std::string &core_type_name,
                                              const std::string &initialization_string)
{
    broker = StartBrokerImp (core_type_name, initialization_string);
}

 void MessageFederateTestFixture::Setup1FederateTest (const std::string &core_type_name)
{
	helicsStatus rv = helicsOK;
    StartBroker (core_type_name, "1");
    helics_federate_info_t fi = helicsFederateInfoCreate();
    rv = helicsFederateInfoSetFederateName(fi, std::string("test1").c_str());
    rv = helicsFederateInfoSetCoreTypeFromString(fi, core_type_name.c_str());
    //std::string coreInitString = "--broker=" + broker->getIdentifier () + " --broker_address=" + broker->getAddress () + " --federates 1";
	std::string coreInitString = " --federates 1";
	rv = helicsFederateInfoSetCoreInitString(fi, coreInitString.c_str());
    //helics::FederateInfo fi ("test1");
    //fi.coreType = helics::coreTypeFromString (core_type_name);
    //fi.coreInitString = std::string ("--broker=") + broker->getIdentifier () +
    //                    " --broker_address=" + broker->getAddress () + " --federates 1";

    //mFed1 = std::make_shared<helics_message_federate> (fi);
	mFed1 = reinterpret_cast<helics_message_federate *>(fi);
}

 void MessageFederateTestFixture::Setup2FederateTest (const std::string &core_type_name)
{
	helicsStatus rv = helicsOK;
    StartBroker (core_type_name, "2");

    helics_federate_info_t fi = helicsFederateInfoCreate();
	rv = helicsFederateInfoSetFederateName(fi, std::string("test1").c_str());
	rv = helicsFederateInfoSetCoreTypeFromString(fi, core_type_name.c_str());
	//std::string coreInitString = "--broker=" + broker->getIdentifier () + " --broker_address=" + broker->getAddress () + " --federates 2";
	std::string coreInitString = " --federates 2";
	rv = helicsFederateInfoSetCoreInitString(fi, coreInitString.c_str());
    //helics::FederateInfo fi ("test1");
    //fi.coreType = helics::coreTypeFromString (core_type_name);
    //fi.coreInitString = std::string ("--broker=") + broker->getIdentifier () +
    //                    " --broker_address=" + broker->getAddress () + " --federates 2";

    //mFed1 = std::make_shared<helics_message_federate> (fi);
	mFed1 = reinterpret_cast<helics_message_federate *>(fi);

    rv = helicsFederateInfoSetFederateName(fi, std::string("test2").c_str());
    //fi.name = "test2";
    //mFed2 = std::make_shared<helics_message_federate> (fi);
	mFed2 = reinterpret_cast<helics_message_federate *>(fi);
}

 bool FederateTestFixture::hasIndexCode (const std::string &type_name)
{
    if (std::isdigit (type_name.back ()) != 0)
    {
        if (*(type_name.end () - 2) == '_')
        {   //this setup ignores the setup mode
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
        //auto core_type = helics::coreTypeFromString (new_type);
        return helicsCreateBroker(new_type.c_str(), std::string("").c_str(), initialization_string.c_str());
    }
    else
    {
        //auto core_type = helics::coreTypeFromString (core_type_name);
        return helicsCreateBroker(core_type_name.c_str(), std::string("").c_str(), initialization_string.c_str());
    }
}

 FederateTestFixture::~FederateTestFixture ()
{
    helicsStatus rv = helicsOK;
	for (auto &fed : federates)
    {
        //if (fed && fed->getCurrentState () != helics::Federate::op_states::finalize)
		if (fed && helicsEnterExecutionMode(fed) != rv)
        {
            //fed->finalize ();
            rv = helicsFederateFinalize(fed);
            helicsFreeFederate(fed);
        }
    }

    for (auto &broker : brokers)
    {
        //broker->disconnect ();
        helicsFreeBroker(broker);
    }
}

helics_broker FederateTestFixture::AddBroker (const std::string &core_type_name, int count)
{
    return AddBroker (core_type_name, std::to_string (count));
}

helics_broker FederateTestFixture::AddBroker (const std::string &core_type_name, const std::string &initialization_string)
{
    auto broker = StartBrokerImp (core_type_name, initialization_string);
    brokers.push_back (broker);
    return broker;
}
