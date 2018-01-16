/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "testFixtures.h"
#include <boost/test/unit_test.hpp>

#include "helics/core/CommonCore.hpp"
#include "helics/core/CoreBroker.hpp"
#include "helics/core/EndpointInfo.hpp"
#include "helics/core/FederateState.hpp"
#include "helics/core/FilterInfo.hpp"
#include "helics/core/PublicationInfo.hpp"
#include "helics/core/SubscriptionInfo.hpp"

federateStateTestFixture::federateStateTestFixture ()
{
    helics::CoreFederateInfo info;
    fs = std::unique_ptr<helics::FederateState> (new helics::FederateState ("fed_name", info));
}

federateStateTestFixture::~federateStateTestFixture () = default;

coreBrokerTestFixture::coreBrokerTestFixture () = default;

coreBrokerTestFixture::~coreBrokerTestFixture () = default;

commonCoreTestFixture::commonCoreTestFixture () = default;

commonCoreTestFixture::~commonCoreTestFixture () = default;
