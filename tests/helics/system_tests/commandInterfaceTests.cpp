/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../application_api/testFixtures.hpp"

#include "gtest/gtest.h"
#include <complex>

/** these test cases test out the value converters
 */
#include "helics/application_api/BrokerApp.hpp"
#include "helics/application_api/CoreApp.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/application_api/ValueConverter.hpp"
#include "helics/application_api/ValueFederate.hpp"

#include <future>

struct command_tests: public FederateTestFixture, public ::testing::Test {
};

/** just a check that in the simple case we do actually get the time back we requested*/

TEST_F(command_tests, federate_federate_command)
{
    SetupTest<helics::ValueFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    // register the publications
    vFed1->registerGlobalPublication<double>("pub1");

    vFed2->registerSubscription("pub1");
    vFed1->setProperty(helics_property_time_delta, 1.0);
    vFed1->sendCommand(vFed2->getName(), "test");
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();

    auto cmd2 = vFed2->getCommand();
    EXPECT_EQ(cmd2.first, "test");
    EXPECT_EQ(cmd2.second, vFed1->getName());
    vFed1->finalize();
    vFed2->finalize();
}

TEST_F(command_tests, federate_federate_command2)
{
    SetupTest<helics::ValueFederate>("test_2", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    // register the publications
    vFed1->registerGlobalPublication<double>("pub1");

    vFed2->registerSubscription("pub1");
    vFed1->setProperty(helics_property_time_delta, 1.0);
    vFed1->sendCommand(vFed2->getName(), "test");
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();

    auto cmd2 = vFed2->getCommand();
    EXPECT_EQ(cmd2.first, "test");
    vFed1->finalize();
    vFed2->finalize();
}

TEST_F(command_tests, federate_federate_command4)
{
    SetupTest<helics::ValueFederate>("test_4", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    // register the publications
    vFed1->registerGlobalPublication<double>("pub1");

    vFed2->registerSubscription("pub1");
    vFed1->setProperty(helics_property_time_delta, 1.0);
    vFed1->sendCommand(vFed2->getName(), "test");
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();

    auto cmd2 = vFed2->getCommand();
    EXPECT_EQ(cmd2.first, "test");
    vFed1->finalize();
    vFed2->finalize();
}

TEST_F(command_tests, core_federate_command)
{
    SetupTest<helics::ValueFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    // register the publications
    vFed1->registerGlobalPublication<double>("pub1");

    vFed2->registerSubscription("pub1");
    auto cr = vFed1->getCorePointer();

    cr->sendCommand(vFed2->getName(), "test", "");
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();

    auto cmd2 = vFed2->getCommand();
    EXPECT_EQ(cmd2.first, "test");
    cmd2 = vFed2->getCommand();
    EXPECT_TRUE(cmd2.first.empty());
    vFed1->finalize();
    vFed2->finalize();
}

TEST_F(command_tests, coreapp_federate_command)
{
    SetupTest<helics::ValueFederate>("test_2", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    // register the publications
    vFed1->registerGlobalPublication<double>("pub1");

    vFed2->registerSubscription("pub1");
    helics::CoreApp cr(vFed1->getCorePointer());

    cr.sendCommand(vFed2->getName(), "test");
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();

    auto cmd2 = vFed2->getCommand();
    EXPECT_EQ(cmd2.first, "test");
    vFed1->finalize();
    vFed2->finalize();
}

TEST_F(command_tests, broker_federate_command)
{
    SetupTest<helics::ValueFederate>("test_2", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    // register the publications
    vFed1->registerGlobalPublication<double>("pub1");

    vFed2->registerSubscription("pub1");

    brokers[0]->sendCommand(vFed2->getName(), "test");
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();

    auto cmd2 = vFed2->getCommand();
    EXPECT_EQ(cmd2.first, "test");
    vFed1->finalize();
    vFed2->finalize();
}

TEST_F(command_tests, brokerapp_federate_command)
{
    SetupTest<helics::ValueFederate>("test_4", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    // register the publications
    vFed1->registerGlobalPublication<double>("pub1");

    vFed2->registerSubscription("pub1");
    helics::BrokerApp brk1(brokers[0]);
    helics::BrokerApp brk2(brokers[1]);
    helics::BrokerApp brk3(brokers[2]);
    brk1.sendCommand(vFed2->getName(), "test");
    brk2.sendCommand(vFed2->getName(), "test");
    brk3.sendCommand(vFed2->getName(), "test");
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();

    auto cmd2 = vFed2->getCommand();
    EXPECT_EQ(cmd2.first, "test");
    cmd2 = vFed2->getCommand();
    EXPECT_EQ(cmd2.first, "test");
    cmd2 = vFed2->getCommand();
    EXPECT_EQ(cmd2.first, "test");
    cmd2 = vFed2->getCommand();
    EXPECT_TRUE(cmd2.first.empty());
    vFed1->finalize();
    vFed2->finalize();
}

TEST_F(command_tests, federation_finalize_command)
{
    SetupTest<helics::ValueFederate>("test_2", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    // register the publications
    vFed1->registerGlobalPublication<double>("pub1");

    vFed2->registerSubscription("pub1");
    helics::CoreApp cr(vFed1->getCorePointer());

    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();
    vFed1->sendCommand("federation", "terminate");

    EXPECT_TRUE(cr.waitForDisconnect(std::chrono::milliseconds(600)));
    vFed1->finalize();
    vFed2->finalize();
}

TEST_F(command_tests, federation_finalize_core_command)
{
    SetupTest<helics::ValueFederate>("test_2", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    // register the publications
    vFed1->registerGlobalPublication<double>("pub1");

    vFed2->registerSubscription("pub1");
    helics::CoreApp cr(vFed1->getCorePointer());

    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();
    vFed2->sendCommand(cr.getIdentifier(), "terminate");

    EXPECT_TRUE(cr.waitForDisconnect(std::chrono::milliseconds(600)));
    vFed1->finalize();
    vFed2->finalize();
}

TEST_F(command_tests, core_echo_command)
{
    SetupTest<helics::ValueFederate>("test_2", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    // register the publications
    vFed1->registerGlobalPublication<double>("pub1");

    vFed2->registerSubscription("pub1");
    auto cr = vFed1->getCorePointer();
    vFed1->sendCommand(cr->getIdentifier(), "echo");
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();

    auto cmd = vFed1->getCommand();
    EXPECT_EQ(cmd.first, "echo_reply");
    EXPECT_EQ(cmd.second, cr->getIdentifier());

    vFed1->finalize();
    vFed2->finalize();
}

TEST_F(command_tests, broker_echo_command)
{
    SetupTest<helics::ValueFederate>("test_2", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    // register the publications
    vFed1->registerGlobalPublication<double>("pub1");

    vFed2->registerSubscription("pub1");
    auto br = brokers[0];
    vFed1->sendCommand(br->getIdentifier(), "echo");
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();

    auto cmd = vFed1->getCommand();
    EXPECT_EQ(cmd.first, "echo_reply");
    EXPECT_EQ(cmd.second, br->getIdentifier());

    vFed1->finalize();
    vFed2->finalize();
}

TEST_F(command_tests, fed_echo_command)
{
    SetupTest<helics::ValueFederate>("test_4", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    // register the publications
    vFed1->registerGlobalPublication<double>("pub1");

    vFed2->registerSubscription("pub1");
    vFed1->sendCommand(vFed2->getName(), "echo");
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();

    auto cmd = vFed1->getCommand();
    EXPECT_EQ(cmd.first, "echo_reply");
    EXPECT_EQ(cmd.second, vFed2->getName());

    vFed1->finalize();
    vFed2->finalize();
}

TEST_F(command_tests, fed_notify_command)
{
    SetupTest<helics::ValueFederate>("test_4", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    // register the publications
    vFed1->registerGlobalPublication<double>("pub1");

    vFed2->registerSubscription("pub1");
    vFed1->sendCommand(vFed2->getName(), "notify");
    vFed1->sendCommand(vFed2->getName(), "command_status");
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();

    auto cmd = vFed2->getCommand();
    EXPECT_TRUE(cmd.first.empty());
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    cmd = vFed1->getCommand();
    EXPECT_EQ(cmd.first.compare(0, 2, "\"1"), 0);
    EXPECT_EQ(cmd.second, vFed2->getName());
    cmd = vFed1->getCommand();
    EXPECT_EQ(cmd.first, "notify_response");
    EXPECT_EQ(cmd.second, vFed2->getName());
    vFed1->finalize();
    vFed2->finalize();
}
