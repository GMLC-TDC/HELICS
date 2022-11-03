/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "ValueFederateTestTemplates.hpp"
#include "helics/application_api/BrokerApp.hpp"
#include "helics/application_api/CoreApp.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/helics_definitions.hpp"
#include "helics/helics_enums.h"

#include <future>
#include <gtest/gtest.h>
#include <thread>
#ifndef HELICS_SHARED_LIBRARY
#    include "testFixtures.hpp"
#else
#    include "testFixtures_shared.hpp"
#endif
#include <fstream>
#include <streambuf>

/** these test cases test out the value federates
 */
class dynFed: public ::testing::Test, public FederateTestFixture {};

static const auto testNamer = [](const ::testing::TestParamInfo<const char*>& parameter) {
    return std::string(parameter.param);
};

TEST_F(dynFed, initPubSubs)
{
    SetupTest<helics::ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    auto& pub1 = vFed1->registerGlobalPublication<double>("pub1");

    auto& in1 = vFed1->registerSubscription("pub1");

    vFed1->enterInitializingMode();

    auto& pub2 = vFed1->registerGlobalPublication<double>("pub2");

    auto& in2 = vFed1->registerSubscription("pub2");

    vFed1->enterExecutingMode();

    pub1.publish(12.0);
    pub2.publish(13.3);
    vFed1->requestNextStep();
    EXPECT_DOUBLE_EQ(in1.getDouble(), 12.0);
    EXPECT_DOUBLE_EQ(in2.getDouble(), 13.3);

    vFed1->disconnect();
}

TEST_F(dynFed, execPubSubs)
{
    SetupTest<helics::ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    auto& pub1 = vFed1->registerGlobalPublication<double>("pub1");

    auto& in1 = vFed1->registerSubscription("pub1");

    vFed1->enterExecutingMode();

    auto& pub2 = vFed1->registerGlobalPublication<double>("pub2");

    auto& in2 = vFed1->registerSubscription("pub2");

    vFed1->requestNextStep();

    pub1.publish(12.0);
    pub2.publish(13.3);
    vFed1->requestNextStep();
    EXPECT_DOUBLE_EQ(in1.getDouble(), 12.0);
    EXPECT_DOUBLE_EQ(in2.getDouble(), 13.3);

    vFed1->disconnect();
}

TEST_F(dynFed, initPub_disable)
{
    extraCoreArgs = "--disable_dynamic_sources";
    SetupTest<helics::ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    vFed1->enterInitializingMode();

    EXPECT_THROW(vFed1->registerGlobalPublication<double>("pub2"), helics::RegistrationFailure);

    vFed1->disconnect();
}

TEST_F(dynFed, execPubSubs_disable)
{
    extraBrokerArgs = "--disable_dynamic_sources";
    SetupTest<helics::ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    vFed1->registerGlobalPublication<double>("pub1");

    vFed1->registerSubscription("pub1");

    vFed1->enterExecutingMode();

    vFed1->registerGlobalPublication<double>("pub2");

    vFed1->registerSubscription("pub2");
    vFed1->query("root", "global_flush");
    EXPECT_THROW(vFed1->requestNextStep(), helics::RegistrationFailure);
    vFed1->disconnect();
}

TEST_F(dynFed, execPubSubs_2fed)
{
    SetupTest<helics::ValueFederate>("test_2", 2, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    auto& pub1 = vFed1->registerGlobalPublication<double>("pub1");

    auto& in1 = vFed2->registerSubscription("pub1");

    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();

    auto& pub2 = vFed2->registerGlobalPublication<double>("pub2");

    auto& in2 = vFed1->registerSubscription("pub2");

    auto res = vFed2->query("root", "global_flush");
    vFed1->requestTimeAsync(helics::timeZero);
    vFed2->requestNextStep();
    auto tres = vFed1->requestTimeComplete();
    EXPECT_EQ(tres, 1.0);

    pub1.publish(12.0);
    pub2.publish(13.3);
    vFed1->requestTimeAsync(helics::timeZero);
    vFed2->requestNextStep();
    tres = vFed1->requestTimeComplete();
    EXPECT_EQ(tres, 2.0);
    EXPECT_DOUBLE_EQ(in1.getDouble(), 12.0);
    EXPECT_DOUBLE_EQ(in2.getDouble(), 13.3);

    vFed1->disconnect();
}
