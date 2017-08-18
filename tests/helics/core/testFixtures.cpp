/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "testFixtures.h"
#include <boost/test/unit_test.hpp>

#include "helics/core/SubscriptionInfo.h"
#include "helics/core/PublicationInfo.h"
#include "helics/core/EndpointInfo.h"
#include "helics/core/FilterInfo.h"
#include "helics/core/FederateState.h"
#include "helics/core/CoreBroker.h"
#include "helics/core/CommonCore.h"

federateStateTestFixture::federateStateTestFixture()
{
	helics::CoreFederateInfo info;
	fs = std::unique_ptr<helics::FederateState>(new helics::FederateState("fed_name", info));
}

federateStateTestFixture::~federateStateTestFixture()
{
}

coreBrokerTestFixture::coreBrokerTestFixture()
{
}

coreBrokerTestFixture::~coreBrokerTestFixture()
{
}

commonCoreTestFixture::commonCoreTestFixture()
{
}

commonCoreTestFixture::~commonCoreTestFixture()
{
}
