/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/CallbackFederate.hpp"
#include "helics/application_api/CoreApp.hpp"
#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/Publications.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/core-exceptions.hpp"
#include "testFixtures.hpp"

#include <gtest/gtest.h>

class callbackFed: public ::testing::Test, public FederateTestFixture {};

TEST_F(callbackFed, create)
{
    SetupTest<helics::CallbackFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::CallbackFederate>(0);

   EXPECT_TRUE(vFed1->getFlagOption(HELICS_FLAG_CALLBACK_FEDERATE));
   vFed1->disconnect();
}


TEST_F(callbackFed, initialize)
{
    SetupTest<helics::CallbackFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::CallbackFederate>(0);

    vFed1->enterInitializingMode();

}
