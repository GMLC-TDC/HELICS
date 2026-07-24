/*
Copyright (c) 2017-2026,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Energy
Innovation LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreBroker.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/CoreFederateInfo.hpp"
#include "helics/helics-config.h"

#include "gtest/gtest.h"
#include <future>
#include <string>

/** test the assignment and retrieval of global value from a broker object*/
TEST(brokers, global_value)
{
    auto brk = helics::BrokerFactory::create(helics::CoreType::TEST, "gbroker", "-f2 --root");
    constexpr std::string_view globalVal = "this is a string constant that functions as a global";
    constexpr std::string_view globalVal2 =
        "this is a second string constant that functions as a global";
    brk->setGlobal("testglobal", globalVal);
    auto res = brk->query("global_value", "testglobal");
    EXPECT_EQ(res, globalVal);
    brk->setGlobal("testglobal2", globalVal2);

    res = brk->query("global_value", "testglobal");
    EXPECT_EQ(res, globalVal);
    res = brk->query("global_value", "testglobal2");
    EXPECT_EQ(res, globalVal2);
    brk->disconnect();
    EXPECT_FALSE(brk->isConnected());
}

/** test the assignment and retrieval of global value from a broker object*/
TEST(brokers, subbroker_min)
{
    auto brk = helics::BrokerFactory::create(helics::CoreType::TEST,
                                             "gbroker2",
                                             "--sub_brokers 2 -f 2 --root");

    auto brk2 = helics::BrokerFactory::create(helics::CoreType::TEST, "gb2", "--broker=gbroker2");

    auto cr1 = helics::CoreFactory::create(helics::CoreType::TEST, "c1", "--broker=gb2");

    helics::CoreFederateInfo cf1;
    auto fid1 = cr1->registerFederate("fed1", cf1);
    auto fid2 = cr1->registerFederate("fed2", cf1);

    auto fut1 =
        std::async(std::launch::async, [fid1, &cr1]() { cr1->enterInitializingMode(fid1); });

    auto fut2 =
        std::async(std::launch::async, [fid2, &cr1]() { cr1->enterInitializingMode(fid2); });

    auto res = fut1.wait_for(std::chrono::milliseconds(100));
    // this should not allow initializingMode entry since only 1 subbroker
    EXPECT_EQ(res, std::future_status::timeout);

    auto cr2 = helics::CoreFactory::create(helics::CoreType::TEST, "c2", "--broker=gb2");
    auto fid3 = cr2->registerFederate("fed3", cf1);

    auto fut3 =
        std::async(std::launch::async, [fid3, &cr2]() { cr2->enterInitializingMode(fid3); });

    res = fut1.wait_for(std::chrono::milliseconds(100));
    // this should still not allow initializingMode entry since still only 1 subbroker
    EXPECT_EQ(res, std::future_status::timeout);

    auto brk3 = helics::BrokerFactory::create(helics::CoreType::TEST, "gb3", "--broker=gbroker2");

    auto cr3 = helics::CoreFactory::create(helics::CoreType::TEST, "c3", "--broker=gb3");

    auto fid4 = cr3->registerFederate("fed4", cf1);

    auto fut4 =
        std::async(std::launch::async, [fid4, &cr3]() { cr3->enterInitializingMode(fid4); });

    // now it should grant
    fut1.get();
    fut2.get();
    fut3.get();
    fut4.get();

    EXPECT_FALSE(brk->isOpenToNewFederates());
    EXPECT_FALSE(cr3->isOpenToNewFederates());

    brk->disconnect();

    EXPECT_TRUE(brk3->waitForDisconnect());
}

TEST(brokers, local_federates_min)
{
    auto brk = helics::BrokerFactory::create(helics::CoreType::TEST,
                                             "local_fed_root",
                                             "--root -f 2 --local_federates 2");

    auto subbrk =
        helics::BrokerFactory::create(helics::CoreType::TEST,
                                      "local_fed_sub",
                                      "--broker=local_fed_root -f 1");
    auto localCore1 = helics::CoreFactory::create(helics::CoreType::TEST,
                                                  "local_fed_core1",
                                                  "--broker=local_fed_root");
    auto remoteCore = helics::CoreFactory::create(helics::CoreType::TEST,
                                                  "local_fed_remote_core",
                                                  "--broker=local_fed_sub");

    helics::CoreFederateInfo cf1;
    auto localFed1 = localCore1->registerFederate("local_fed1", cf1);
    auto remoteFed = remoteCore->registerFederate("local_remote_fed", cf1);

    auto localFuture1 = std::async(std::launch::async, [localFed1, &localCore1]() {
        localCore1->enterInitializingMode(localFed1);
    });
    auto remoteFuture = std::async(std::launch::async, [remoteFed, &remoteCore]() {
        remoteCore->enterInitializingMode(remoteFed);
    });

    auto res = localFuture1.wait_for(std::chrono::milliseconds(100));
    EXPECT_EQ(res, std::future_status::timeout);

    auto localCore2 = helics::CoreFactory::create(helics::CoreType::TEST,
                                                  "local_fed_core2",
                                                  "--broker=local_fed_root");
    auto localFed2 = localCore2->registerFederate("local_fed2", cf1);
    auto localFuture2 = std::async(std::launch::async, [localFed2, &localCore2]() {
        localCore2->enterInitializingMode(localFed2);
    });

    localFuture1.get();
    localFuture2.get();
    remoteFuture.get();

    brk->disconnect();
    EXPECT_TRUE(subbrk->waitForDisconnect());
}

TEST(brokers, local_subbrokers_min)
{
    auto brk = helics::BrokerFactory::create(helics::CoreType::TEST,
                                             "local_sub_root",
                                             "--root -f 2 --local_subbrokers 2");

    auto subbrk1 =
        helics::BrokerFactory::create(helics::CoreType::TEST,
                                      "local_sub1",
                                      "--broker=local_sub_root -f 1");
    auto remoteCore1 = helics::CoreFactory::create(helics::CoreType::TEST,
                                                   "local_sub_core1",
                                                   "--broker=local_sub1");

    helics::CoreFederateInfo cf1;
    auto fed1 = remoteCore1->registerFederate("local_sub_fed1", cf1);
    auto future1 = std::async(std::launch::async, [fed1, &remoteCore1]() {
        remoteCore1->enterInitializingMode(fed1);
    });

    auto res = future1.wait_for(std::chrono::milliseconds(100));
    EXPECT_EQ(res, std::future_status::timeout);

    auto subbrk2 =
        helics::BrokerFactory::create(helics::CoreType::TEST,
                                      "local_sub2",
                                      "--broker=local_sub_root -f 1");
    auto remoteCore2 = helics::CoreFactory::create(helics::CoreType::TEST,
                                                   "local_sub_core2",
                                                   "--broker=local_sub2");
    auto fed2 = remoteCore2->registerFederate("local_sub_fed2", cf1);
    auto future2 = std::async(std::launch::async, [fed2, &remoteCore2]() {
        remoteCore2->enterInitializingMode(fed2);
    });

    future1.get();
    future2.get();

    brk->disconnect();
    EXPECT_TRUE(subbrk1->waitForDisconnect());
    EXPECT_TRUE(subbrk2->waitForDisconnect());
}

TEST(brokers, required_federates_broker)
{
    auto brk = helics::BrokerFactory::create(helics::CoreType::TEST,
                                             "required_fed_broker",
                                             "--root -f 1 --required_federates fed_required");
    auto cr1 = helics::CoreFactory::create(helics::CoreType::TEST,
                                           "required_core1",
                                           "--broker=required_fed_broker");

    helics::CoreFederateInfo cf1;
    auto fid1 = cr1->registerFederate("fed_other", cf1);
    auto fut1 =
        std::async(std::launch::async, [fid1, &cr1]() { cr1->enterInitializingMode(fid1); });

    auto res = fut1.wait_for(std::chrono::milliseconds(100));
    EXPECT_EQ(res, std::future_status::timeout);

    auto cr2 = helics::CoreFactory::create(helics::CoreType::TEST,
                                           "required_core2",
                                           "--broker=required_fed_broker");
    auto fid2 = cr2->registerFederate("fed_required", cf1);
    auto fut2 =
        std::async(std::launch::async, [fid2, &cr2]() { cr2->enterInitializingMode(fid2); });

    fut1.get();
    fut2.get();

    brk->disconnect();
}

TEST(brokers, required_federates_core)
{
    auto brk = helics::BrokerFactory::create(helics::CoreType::TEST,
                                             "required_fed_core_broker",
                                             "-f 2 --root");
    auto cr1 =
        helics::CoreFactory::create(helics::CoreType::TEST,
                                    "required_fed_core",
                                    "--broker=required_fed_core_broker -f 1 "
                                    "--required_federates fed_required_core");

    helics::CoreFederateInfo cf1;
    auto fid1 = cr1->registerFederate("fed_other_core", cf1);
    auto fut1 =
        std::async(std::launch::async, [fid1, &cr1]() { cr1->enterInitializingMode(fid1); });

    auto res = fut1.wait_for(std::chrono::milliseconds(100));
    EXPECT_EQ(res, std::future_status::timeout);

    auto fid2 = cr1->registerFederate("fed_required_core", cf1);
    auto fut2 =
        std::async(std::launch::async, [fid2, &cr1]() { cr1->enterInitializingMode(fid2); });

    fut1.get();
    fut2.get();

    brk->disconnect();
}

/** test the assignment and retrieval of global value from a broker object*/
TEST(brokers, subbroker_min_files)
{
    auto brk = helics::BrokerFactory::create(helics::CoreType::EXTRACT,
                                             "",
                                             std::string(TEST_DIR) + "broker_test_subbroker.json");

    EXPECT_EQ(brk->getIdentifier(), "gbroker_f1");

    auto brk2 =
        helics::BrokerFactory::create(helics::CoreType::EXTRACT,
                                      "",
                                      std::string(TEST_DIR) + "broker_test_subbroker2.toml");

    auto cr1 = helics::CoreFactory::create(helics::CoreType::TEST, "--name=cf1 --broker=gbf2");

    helics::CoreFederateInfo cf1;
    auto fid1 = cr1->registerFederate("fed1", cf1);
    auto fid2 = cr1->registerFederate("fed2", cf1);

    auto fut1 =
        std::async(std::launch::async, [fid1, &cr1]() { cr1->enterInitializingMode(fid1); });

    auto fut2 =
        std::async(std::launch::async, [fid2, &cr1]() { cr1->enterInitializingMode(fid2); });

    auto res = fut1.wait_for(std::chrono::milliseconds(100));
    // this should not allow initializingMode entry since only 1 subbroker
    EXPECT_EQ(res, std::future_status::timeout);

    auto cr2 = helics::CoreFactory::create(helics::CoreType::EXTRACT,
                                           std::string(TEST_DIR) + "broker_test_core.json");
    auto fid3 = cr2->registerFederate("fed3", cf1);

    auto fut3 =
        std::async(std::launch::async, [fid3, &cr2]() { cr2->enterInitializingMode(fid3); });

    res = fut1.wait_for(std::chrono::milliseconds(100));
    // this should still not allow initializingMode entry since still only 1 subbroker
    EXPECT_EQ(res, std::future_status::timeout);

    auto brk3 =
        helics::BrokerFactory::create(helics::CoreType::TEST, "gbf3", R"({"broker":"gbroker_f1"})");

    auto cr3 = helics::CoreFactory::create(helics::CoreType::TEST, "cf3", "broker=\"gbf3\"");
    auto fid4 = cr3->registerFederate("fed4", cf1);

    auto fut4 =
        std::async(std::launch::async, [fid4, &cr3]() { cr3->enterInitializingMode(fid4); });

    // now it should grant
    fut1.get();
    fut2.get();
    fut3.get();
    fut4.get();

    EXPECT_FALSE(brk->isOpenToNewFederates());
    EXPECT_FALSE(cr3->isOpenToNewFederates());

    brk->disconnect();

    EXPECT_TRUE(brk3->waitForDisconnect());
}

/** This test should be removed once log levels with numbers is re-enabled ~helics 3.3 */
TEST(brokers, broker_log_command_failures)
{
    EXPECT_THROW(helics::BrokerFactory::create(helics::CoreType::TEST,
                                               "brokerlog1",
                                               "--loglevel=6 --root"),
                 std::exception);

    EXPECT_THROW(helics::BrokerFactory::create(helics::CoreType::TEST,
                                               "brokerlog2",
                                               "--consoleloglevel=3 --root"),
                 std::exception);

    EXPECT_THROW(helics::BrokerFactory::create(helics::CoreType::TEST,
                                               "brokerlog3",
                                               "--fileloglevel=-4 --root"),
                 std::exception);
}

#ifdef HELICS_ENABLE_ZMQ_CORE
TEST(brokers, force_override_ci_skip_nocov)
{
    auto brk = helics::BrokerFactory::create(helics::CoreType::ZMQ, "gbroker_f1", "");
    EXPECT_TRUE(brk->isConnected());
    auto cr1 = helics::CoreFactory::create(helics::CoreType::ZMQ, "c1", "");
    EXPECT_TRUE(cr1->connect());
    EXPECT_TRUE(cr1->isConnected());

    auto brk2 = helics::BrokerFactory::create(helics::CoreType::ZMQ, "gbroker_f2", "");
    EXPECT_FALSE(brk2->isConnected());
    decltype(brk2) brk3;
    EXPECT_NO_THROW(
        brk3 = helics::BrokerFactory::create(helics::CoreType::ZMQ, "gbroker_f3", "--force"));
    // EXPECT_TRUE(brk3->isConnected());
    // NOTE(PT) the test for connection on the third broker is unreliable due to the nature of ZMQ
    // reapers. the test for connection on the first broker should work reliably.
    EXPECT_FALSE(brk->isConnected());
    EXPECT_FALSE(cr1->isConnected());
    if (brk3 && brk3->isConnected()) {
        EXPECT_TRUE(brk->isConnected());
    }
}
#endif
