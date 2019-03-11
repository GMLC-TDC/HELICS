/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "testFixtures.h"
#include <boost/test/unit_test.hpp>

#include "helics/core/CommonCore.hpp"
#include "helics/core/CoreBroker.hpp"
#include "helics/core/EndpointInfo.hpp"
#include "helics/core/FederateState.hpp"
#include "helics/core/FilterInfo.hpp"
#include "helics/core/NamedInputInfo.hpp"
#include "helics/core/PublicationInfo.hpp"

federateStateTestFixture::federateStateTestFixture ()
    : fs (std::make_unique<helics::FederateState> ("fed_name", helics::CoreFederateInfo ()))
{
}

federateStateTestFixture::~federateStateTestFixture () = default;

// coreBrokerTestFixture::coreBrokerTestFixture () = default;

// coreBrokerTestFixture::~coreBrokerTestFixture () = default;

// commonCoreTestFixture::commonCoreTestFixture () = default;

// commonCoreTestFixture::~commonCoreTestFixture () = default;
