/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "testFixtures.h"
#include <boost/test/unit_test.hpp>

#include "helics/core/CoreFactory.h"
#include "helics/core/BrokerFactory.h"

ValueFederateTestFixture::~ValueFederateTestFixture()
{
    if (vFed1 != nullptr)
    {
        vFed1->finalize();
    }

    if (vFed2 != nullptr)
    {
        vFed2->finalize();
    }
}

void ValueFederateTestFixture::StartBroker(const std::string &core_type_name, const std::string &initialization_string)
{
    auto core_type = helics::coreTypeFromString(core_type_name);
    broker = helics::BrokerFactory::create(core_type, initialization_string);
}

void ValueFederateTestFixture::Setup1FederateTest(const std::string &core_type_name, helics::Time time_delta)
{
    StartBroker(core_type_name, "1");

    helics::FederateInfo fi("test1");
    fi.coreType = core_type_name;
    fi.timeDelta = time_delta;
    fi.coreInitString = "--broker=" + broker->getIdentifier() + " --broker_address="+broker->getAddress()+" --federates 1";

    vFed1 = std::make_shared<helics::ValueFederate>(fi);
}

void ValueFederateTestFixture::Setup2FederateTest(const std::string &core_type_name, helics::Time time_delta)
{
    StartBroker(core_type_name, "2");

    helics::FederateInfo fi("test1");
    fi.coreType = core_type_name;
    fi.timeDelta = time_delta;
    fi.coreInitString = "--broker=" + broker->getIdentifier() + " --broker_address=" + broker->getAddress() + " --federates 2";

    vFed1 = std::make_shared<helics::ValueFederate>(fi);

    fi.name = "test2";
    vFed2 = std::make_shared<helics::ValueFederate>(fi);
}



MessageFederateTestFixture::~MessageFederateTestFixture()
{
    if (mFed1 != nullptr)
    {
        mFed1->finalize();
    }

    if (mFed2 != nullptr)
    {
        mFed2->finalize();
    }
}

void MessageFederateTestFixture::StartBroker(const std::string &core_type_name, const std::string &initialization_string)
{
    auto core_type = helics::coreTypeFromString(core_type_name);
    broker = helics::BrokerFactory::create(core_type, initialization_string);
}

void MessageFederateTestFixture::Setup1FederateTest(const std::string &core_type_name)
{
    StartBroker(core_type_name, "1");

    helics::FederateInfo fi("test1");
    fi.coreType = core_type_name;
    fi.coreInitString = "--broker=" + broker->getIdentifier() + " --federates 1";

    mFed1 = std::make_shared<helics::MessageFederate>(fi);
}

void MessageFederateTestFixture::Setup2FederateTest(const std::string &core_type_name)
{
    StartBroker(core_type_name, "2");

    helics::FederateInfo fi("test1");
    fi.coreType = core_type_name;
    fi.coreInitString = "--broker=" + broker->getIdentifier() + " --federates 2";

    mFed1 = std::make_shared<helics::MessageFederate>(fi);

    fi.name = "test2";
    mFed2 = std::make_shared<helics::MessageFederate>(fi);
}