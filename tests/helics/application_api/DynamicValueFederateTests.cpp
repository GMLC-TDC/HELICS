/*
Copyright (c) 2017-2024,
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

    vFed2->query("root", "global_flush");
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

TEST_F(dynFed, dynamicPubSubs_2fed)
{
    extraBrokerArgs = "--dynamic";
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

    vFed2->query("root", "global_flush");
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
    EXPECT_TRUE(brokers[0]->isOpenToNewFederates());
    AddFederates<helics::ValueFederate>("test_2", 1, brokers[0], 1.0);
    auto vFed3 = GetFederateAs<helics::ValueFederate>(2);
    auto& pub3 = vFed3->registerGlobalPublication<double>("pub3");
    auto& in3 = vFed1->registerSubscription("pub3");
    auto& in1_3 = vFed3->registerSubscription("pub1");
    vFed3->enterExecutingMode();

    auto ctime = vFed3->getCurrentTime();
    // should be granted 1 valid time step behind where its dependencies are currently granted
    EXPECT_EQ(ctime, 1.0);

    pub1.publish(8.5);
    // requesting time 3
    vFed1->requestTimeAsync(helics::timeZero);
    vFed2->requestTimeAsync(helics::timeZero);
    ctime = vFed3->requestNextStep();

    // should be granted time 2 now
    EXPECT_EQ(ctime, 2.0);
    pub3.publish(7.5);
    ctime = vFed3->requestNextStep();
    EXPECT_EQ(ctime, 3.0);
    vFed1->requestTimeComplete();
    tres = vFed2->requestTimeComplete();

    EXPECT_DOUBLE_EQ(in1.getDouble(), 8.5);
    EXPECT_DOUBLE_EQ(in3.getDouble(), 7.5);

    pub1.publish(10.2);
    vFed2->disconnect();
    vFed1->disconnect();
    ctime = vFed3->requestNextStep();
    // due to connections this won't have connected until the next time request
    EXPECT_EQ(ctime, 4.0);
    EXPECT_EQ(in1_3.getDouble(), 10.2);
    vFed3->disconnect();
    std::this_thread::yield();
    int cnt{0};
    while (brokers[0]->isOpenToNewFederates()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        ++cnt;
        if (cnt > 10) {
            break;
        }
    }
    EXPECT_FALSE(brokers[0]->isOpenToNewFederates());
}

TEST_F(dynFed, dynamicPubSubs_2fed_pubStore)
{
    extraBrokerArgs = "--dynamic";
    SetupTest<helics::ValueFederate>("test_2", 2, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    auto& pub1 = vFed1->registerGlobalPublication<double>("pub1");
    pub1.setOption(HELICS_HANDLE_OPTION_BUFFER_DATA);

    auto& in1 = vFed2->registerSubscription("pub1");

    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();

    auto& pub2 = vFed2->registerGlobalPublication<double>("pub2");

    auto& in2 = vFed1->registerSubscription("pub2");

    vFed2->query("root", "global_flush");
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
    EXPECT_TRUE(brokers[0]->isOpenToNewFederates());
    AddFederates<helics::ValueFederate>("test_2", 1, brokers[0], 1.0);
    auto vFed3 = GetFederateAs<helics::ValueFederate>(2);
    auto& pub3 = vFed3->registerGlobalPublication<double>("pub3");
    auto& in3 = vFed1->registerSubscription("pub3");
    auto& in1_3 = vFed3->registerSubscription("pub1");
    vFed3->enterExecutingMode();

    auto ctime = vFed3->getCurrentTime();
    // should be granted 1 valid time step behind where its dependencies are currently granted
    EXPECT_EQ(ctime, 1.0);

    pub1.publish(8.5);
    // requesting time 3
    vFed1->requestTimeAsync(helics::timeZero);
    vFed2->requestTimeAsync(helics::timeZero);
    ctime = vFed3->requestNextStep();

    // should be granted time 2 now
    EXPECT_EQ(ctime, 2.0);
    pub3.publish(7.5);
    ctime = vFed3->requestNextStep();
    EXPECT_EQ(ctime, 3.0);
    vFed1->requestTimeComplete();
    tres = vFed2->requestTimeComplete();

    EXPECT_DOUBLE_EQ(in1.getDouble(), 8.5);
    EXPECT_DOUBLE_EQ(in3.getDouble(), 7.5);
    EXPECT_DOUBLE_EQ(in1_3.getDouble(), 8.5);

    vFed2->disconnect();
    vFed1->disconnect();
    vFed3->disconnect();

    std::this_thread::yield();
    int cnt{0};
    while (brokers[0]->isOpenToNewFederates()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        ++cnt;
        if (cnt > 10) {
            break;
        }
    }
    EXPECT_FALSE(brokers[0]->isOpenToNewFederates());
}
