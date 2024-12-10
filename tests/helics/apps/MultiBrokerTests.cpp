/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helics/application_api/BrokerApp.hpp"
#include "helics/application_api/CoreApp.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/apps/MultiBroker.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/core-exceptions.hpp"

#include "gtest/gtest.h"
#include <cstdio>
#include <filesystem>
#include <future>
#include <string>

static const bool amb = helics::allowMultiBroker();

TEST(MultiBroker, constructor1)
{
    helics::BrokerApp App(helics::CoreType::MULTI, "brk1", "--coretype test");

    // Brokers connect automatically
    EXPECT_TRUE(App.isConnected());
    EXPECT_TRUE(App.isOpenToNewFederates());
    EXPECT_EQ(App.getIdentifier(), "brk1");
    App.forceTerminate();
    EXPECT_FALSE(App.isConnected());
}

TEST(MultiBroker, connect1)
{
    helics::BrokerApp b(helics::CoreType::MULTI, "brk2", "--coretype test");
    EXPECT_TRUE(b.connect());
    helics::CoreApp c1(helics::CoreType::TEST, "--brokername=brk2 --name=core1b");
    EXPECT_TRUE(c1.connect());

    helics::Federate fedb("fedb", c1);
    fedb.enterExecutingMode();

    fedb.finalize();
    c1->disconnect();
    b.forceTerminate();
    EXPECT_TRUE(b.waitForDisconnect(std::chrono::milliseconds(500)));
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
}

#if defined(HELICS_ENABLE_ZMQ_CORE)
TEST(MultiBroker, file2)
{
    using helics::CoreType;
    const std::string config = "--config=" + std::string(TEST_DIR) + "multiBroker2.json";
    helics::BrokerApp App(CoreType::MULTI, "brkf2", config);

    // Brokers connect automatically
    EXPECT_TRUE(App.isConnected());
    EXPECT_TRUE(App.isOpenToNewFederates());
    EXPECT_EQ(App.getIdentifier(), "brkf2");

    auto brk1 = helics::BrokerFactory::findJoinableBrokerOfType(CoreType::TEST);
    EXPECT_TRUE(brk1);
    brk1 = helics::BrokerFactory::findJoinableBrokerOfType(CoreType::ZMQ);
    EXPECT_TRUE(brk1);
    brk1.reset();
    App.forceTerminate();
    EXPECT_FALSE(App.isConnected());
    App.reset();
    helics::cleanupHelicsLibrary();
}

#    if defined(HELICS_ENABLE_TCP_CORE) && defined(HELICS_ENABLE_IPC_CORE)
TEST(MultiBroker, file3)
{
    using helics::CoreType;
    const std::string config = "--config=" + std::string(TEST_DIR) + "multiBroker3.json";
    helics::BrokerApp App(CoreType::MULTI, "brkf3", config);

    // Brokers connect automatically
    EXPECT_TRUE(App.isConnected());
    EXPECT_TRUE(App.isOpenToNewFederates());
    EXPECT_EQ(App.getIdentifier(), "brkf3");

    auto brk1 = helics::BrokerFactory::findJoinableBrokerOfType(CoreType::ZMQ);
    EXPECT_TRUE(brk1);
    brk1 = helics::BrokerFactory::findJoinableBrokerOfType(CoreType::IPC);
    EXPECT_TRUE(brk1);
    brk1 = helics::BrokerFactory::findJoinableBrokerOfType(CoreType::TCP);
    EXPECT_TRUE(brk1);
    brk1.reset();
    App.forceTerminate();
    EXPECT_FALSE(App.isConnected());
    App.reset();
    helics::cleanupHelicsLibrary();
}

#    endif

#    if defined(HELICS_ENABLE_ZMQ_CORE) && defined(HELICS_ENABLE_TCP_CORE)
TEST(MultiBroker, file1)
{
    using helics::CoreType;
    const std::string config = "--config=" + std::string(TEST_DIR) + "multiBroker1.json";
    helics::BrokerApp App(CoreType::MULTI, "brkf1", config);

    // Brokers connect automatically
    EXPECT_TRUE(App.isConnected());
    EXPECT_TRUE(App.isOpenToNewFederates());
    EXPECT_EQ(App.getIdentifier(), "brkf1");

    auto brk1 = helics::BrokerFactory::findJoinableBrokerOfType(CoreType::TEST);
    EXPECT_TRUE(brk1);
    brk1 = helics::BrokerFactory::findJoinableBrokerOfType(CoreType::ZMQ);
    EXPECT_TRUE(brk1);
    brk1 = helics::BrokerFactory::findJoinableBrokerOfType(CoreType::TCP);
    EXPECT_TRUE(brk1);

    App.forceTerminate();
    EXPECT_FALSE(App.isConnected());
    App.reset();
    helics::cleanupHelicsLibrary();
}

TEST(MultiBroker, file4)
{
    using helics::CoreType;
    const std::string config = "--config=" + std::string(TEST_DIR) + "multiBroker4.json";
    helics::BrokerApp App(CoreType::MULTI, "brkf4", config);

    // Brokers connect automatically
    EXPECT_TRUE(App.isConnected());
    EXPECT_TRUE(App.isOpenToNewFederates());
    EXPECT_EQ(App.getIdentifier(), "brkf4");

    auto brk1 = helics::BrokerFactory::findJoinableBrokerOfType(CoreType::ZMQ);
    EXPECT_TRUE(brk1);
    brk1 = helics::BrokerFactory::findJoinableBrokerOfType(CoreType::TEST);
    EXPECT_TRUE(brk1);
    brk1 = helics::BrokerFactory::findJoinableBrokerOfType(CoreType::TCP);
    EXPECT_TRUE(brk1);
    brk1.reset();
    App.forceTerminate();
    EXPECT_FALSE(App.isConnected());
    App.reset();
    helics::cleanupHelicsLibrary();
}

TEST(MultiBroker, link2)
{
    using helics::CoreType;
    const std::string config = "-f 2 --config=" + std::string(TEST_DIR) + "multiBroker1.json";
    helics::BrokerApp App(CoreType::MULTI, "brkmt1", config);

    // Brokers connect automatically
    EXPECT_TRUE(App.isConnected());
    EXPECT_TRUE(App.isOpenToNewFederates());
    EXPECT_EQ(App.getIdentifier(), "brkmt1");

    helics::CoreApp c1(helics::CoreType::TEST, "--brokername=brkmt1 --name=core1t");
    EXPECT_TRUE(c1.connect());

    helics::ValueFederate fedA("fedA", c1);
    auto& pub = fedA.registerGlobalPublication<double>("key");
    fedA.enterExecutingModeAsync();

    helics::CoreApp c2(helics::CoreType::ZMQ, "--name=core1z");
    EXPECT_TRUE(c2.connect());

    helics::ValueFederate fedB("fedB", c2);
    auto& sub = fedB.registerSubscription("key");
    fedB.enterExecutingMode();
    fedA.enterExecutingModeComplete();

    pub.publish(27.045);
    fedA.finalize();
    fedB.requestNextStep();

    EXPECT_DOUBLE_EQ(sub.getValue<double>(), 27.045);
    fedB.finalize();

    App.waitForDisconnect(std::chrono::milliseconds(300));
    EXPECT_FALSE(App.isConnected());
    App.reset();
    helics::cleanupHelicsLibrary();
}

#    endif
#endif
