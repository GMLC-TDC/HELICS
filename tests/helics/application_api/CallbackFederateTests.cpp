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
#include <future>

class callbackFed: public ::testing::Test, public FederateTestFixture {};

TEST_F(callbackFed, create)
{
    SetupTest<helics::CallbackFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::CallbackFederate>(0);

   EXPECT_TRUE(vFed1->getFlagOption(HELICS_FLAG_CALLBACK_FEDERATE));
   vFed1->disconnect();
}

// test that the initialize callback gets executed and handles termination properly
TEST_F(callbackFed, initialize)
{
    SetupTest<helics::CallbackFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::CallbackFederate>(0);
    int cb{0};
    vFed1->setInitializeCallback([&cb](){++cb;return helics::IterationRequest::HALT_OPERATIONS;});

    auto  v=std::make_shared<std::promise<int>>();
    auto vfut=v->get_future();
    vFed1->setCosimulationTerminatedCallback([v = std::move(v)](){v->set_value(5); });
    
    vFed1->enterInitializingMode();

    auto res=vfut.get();
    EXPECT_EQ(cb,1);
    EXPECT_EQ(res,5);
    EXPECT_TRUE(vFed1->isAsyncOperationCompleted());
}


// test that the initialize callback gets executed and handles termination properly
TEST_F(callbackFed, execute)
{
    SetupTest<helics::CallbackFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::CallbackFederate>(0);
    int cb{0};

    vFed1->setExecutingEntryCallback([&cb](){++cb;});
    vFed1->setNextTimeIterativeCallback([](auto /*t*/) {
        return std::make_pair(helics::Time::maxVal(), helics::IterationRequest::HALT_OPERATIONS); }
    );
    auto  v=std::make_shared<std::promise<int>>();
    auto vfut=v->get_future();
    vFed1->setCosimulationTerminatedCallback([v = std::move(v)](){v->set_value(5); });

    vFed1->enterInitializingMode();

    auto res=vfut.get();
    EXPECT_EQ(cb,1);
    EXPECT_EQ(res,5);
    EXPECT_TRUE(vFed1->isAsyncOperationCompleted());
}
