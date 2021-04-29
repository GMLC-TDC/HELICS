/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "gtest/gtest.h"

#ifdef _MSC_VER
#    pragma warning(push, 0)
#    include "helics/external/filesystem.hpp"
#    pragma warning(pop)
#else
#    include "helics/external/filesystem.hpp"
#endif

#include "helics/application_api/BrokerApp.hpp"
#include "helics/application_api/CoreApp.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/apps/MultiBroker.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/core-exceptions.hpp"

#include <cstdio>
#include <future>

static const bool amb = helics::allowMultiBroker();

TEST(MultiBroker, constructor1)
{
    helics::BrokerApp App(helics::core_type::MULTI, "brk1", "--type test");

    // Brokers connect automatically
    EXPECT_TRUE(App.isConnected());
    EXPECT_TRUE(App.isOpenToNewFederates());
    EXPECT_EQ(App.getIdentifier(), "brk1");
    App.forceTerminate();
    EXPECT_FALSE(App.isConnected());
}

TEST(MultiBroker, connect1)
{
    helics::BrokerApp b(helics::core_type::MULTI, "brk2", "--type test");
    EXPECT_TRUE(b.connect());
    helics::CoreApp c1(helics::core_type::TEST, "--brokername=brk2 --name=core1b");
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

#if defined(ENABLE_ZMQ_CORE)
TEST(MultiBroker, file2)
{
    using helics::core_type;
    const std::string config = "--config=" + std::string(TEST_DIR) + "multiBroker2.json";
    helics::BrokerApp App(core_type::MULTI, "brkf2", config);

    // Brokers connect automatically
    EXPECT_TRUE(App.isConnected());
    EXPECT_TRUE(App.isOpenToNewFederates());
    EXPECT_EQ(App.getIdentifier(), "brkf2");

    auto brk1 = helics::BrokerFactory::findJoinableBrokerOfType(core_type::TEST);
    EXPECT_TRUE(brk1);
    brk1 = helics::BrokerFactory::findJoinableBrokerOfType(core_type::ZMQ);
    EXPECT_TRUE(brk1);
    brk1.reset();
    App.forceTerminate();
    EXPECT_FALSE(App.isConnected());
    App.reset();
    helics::cleanupHelicsLibrary();
}

#    if defined(ENABLE_TCP_CORE) && defined(ENABLE_IPC_CORE)
TEST(MultiBroker, file3)
{
    using helics::core_type;
    const std::string config = "--config=" + std::string(TEST_DIR) + "multiBroker3.json";
    helics::BrokerApp App(core_type::MULTI, "brkf3", config);

    // Brokers connect automatically
    EXPECT_TRUE(App.isConnected());
    EXPECT_TRUE(App.isOpenToNewFederates());
    EXPECT_EQ(App.getIdentifier(), "brkf3");

    auto brk1 = helics::BrokerFactory::findJoinableBrokerOfType(core_type::ZMQ);
    EXPECT_TRUE(brk1);
    brk1 = helics::BrokerFactory::findJoinableBrokerOfType(core_type::IPC);
    EXPECT_TRUE(brk1);
    brk1 = helics::BrokerFactory::findJoinableBrokerOfType(core_type::TCP);
    EXPECT_TRUE(brk1);
    brk1.reset();
    App.forceTerminate();
    EXPECT_FALSE(App.isConnected());
    App.reset();
    helics::cleanupHelicsLibrary();
}

#    endif

#    if defined(ENABLE_ZMQ_CORE) && defined(ENABLE_TCP_CORE)
TEST(MultiBroker, file1)
{
    using helics::core_type;
    const std::string config = "--config=" + std::string(TEST_DIR) + "multiBroker1.json";
    helics::BrokerApp App(core_type::MULTI, "brkf1", config);

    // Brokers connect automatically
    EXPECT_TRUE(App.isConnected());
    EXPECT_TRUE(App.isOpenToNewFederates());
    EXPECT_EQ(App.getIdentifier(), "brkf1");

    auto brk1 = helics::BrokerFactory::findJoinableBrokerOfType(core_type::TEST);
    EXPECT_TRUE(brk1);
    brk1 = helics::BrokerFactory::findJoinableBrokerOfType(core_type::ZMQ);
    EXPECT_TRUE(brk1);
    brk1 = helics::BrokerFactory::findJoinableBrokerOfType(core_type::TCP);
    EXPECT_TRUE(brk1);

    App.forceTerminate();
    EXPECT_FALSE(App.isConnected());
    App.reset();
    helics::cleanupHelicsLibrary();
}

TEST(MultiBroker, file4)
{
    using helics::core_type;
    const std::string config = "--config=" + std::string(TEST_DIR) + "multiBroker4.json";
    helics::BrokerApp App(core_type::MULTI, "brkf4", config);

    // Brokers connect automatically
    EXPECT_TRUE(App.isConnected());
    EXPECT_TRUE(App.isOpenToNewFederates());
    EXPECT_EQ(App.getIdentifier(), "brkf4");

    auto brk1 = helics::BrokerFactory::findJoinableBrokerOfType(core_type::ZMQ);
    EXPECT_TRUE(brk1);
    brk1 = helics::BrokerFactory::findJoinableBrokerOfType(core_type::TEST);
    EXPECT_TRUE(brk1);
    brk1 = helics::BrokerFactory::findJoinableBrokerOfType(core_type::TCP);
    EXPECT_TRUE(brk1);
    brk1.reset();
    App.forceTerminate();
    EXPECT_FALSE(App.isConnected());
    App.reset();
    helics::cleanupHelicsLibrary();
}

TEST(MultiBroker, link2)
{
    using helics::core_type;
    const std::string config = "-f 2 --config=" + std::string(TEST_DIR) + "multiBroker1.json";
    helics::BrokerApp App(core_type::MULTI, "brkmt1", config);

    // Brokers connect automatically
    EXPECT_TRUE(App.isConnected());
    EXPECT_TRUE(App.isOpenToNewFederates());
    EXPECT_EQ(App.getIdentifier(), "brkmt1");

    helics::CoreApp c1(helics::core_type::TEST, "--brokername=brkmt1 --name=core1t");
    EXPECT_TRUE(c1.connect());

    helics::ValueFederate fedA("fedA", c1);
    auto& pub = fedA.registerGlobalPublication<double>("key");
    fedA.enterExecutingModeAsync();

    helics::CoreApp c2(helics::core_type::ZMQ, "--name=core1z");
    EXPECT_TRUE(c2.connect());

    helics::ValueFederate fedB("fedB", c2);
    auto& sub = fedB.registerSubscription("key");
    fedB.enterExecutingMode();
    fedA.enterExecutingModeComplete();

    fedA.publish(pub, 27.045);
    fedA.finalize();
    fedB.requestNextStep();

    EXPECT_DOUBLE_EQ(fedB.getDouble(sub), 27.045);
    fedB.finalize();

    App.waitForDisconnect(std::chrono::milliseconds(300));
    EXPECT_FALSE(App.isConnected());
    App.reset();
    helics::cleanupHelicsLibrary();
}

#    endif
#endif
