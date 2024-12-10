/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/BrokerApp.hpp"
#include "helics/application_api/CoreApp.hpp"
#include "helics/application_api/Federate.hpp"
#include "helics/application_api/Filters.hpp"
#include "helics/common/JsonProcessingFunctions.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/core-exceptions.hpp"
#include "helics/core/helics_definitions.hpp"
#include "testFixtures.hpp"

#include "gmock/gmock.h"
#include <future>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <thread>

/** these test cases test out the value converters
 */

#define CORE_TYPE_TO_TEST helics::CoreType::TEST

TEST(federate, federate_initialize_tests)
{
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString = "--autobroker";

    auto Fed = std::make_shared<helics::Federate>("test1", fedInfo);

    auto core = Fed->getCorePointer();
    ASSERT_TRUE((core));

    auto name = std::string(core->getFederateName(Fed->getID()));

    EXPECT_EQ(name, Fed->getName());
    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::Modes::STARTUP);
    Fed->enterInitializingMode();
    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::Modes::INITIALIZING);
    Fed->enterExecutingMode();
    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    // const auto& coreName = core->getIdentifier();
    // const auto& fedName = Fed->getName();
    // EXPECT_EQ(fedName+"_core", coreName);

    // const auto& coreName = core->getIdentifier();
    // const auto& fedName = Fed->getName();
    // EXPECT_EQ(fedName+"_core", coreName);
    Fed = nullptr;  // force the destructor
}

TEST(federate, single_core_federate)
{
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString = "--autobroker";
    fedInfo.setFlagOption(helics::defs::SINGLE_THREAD_FEDERATE);
    auto fed = std::make_shared<helics::Federate>("test1", fedInfo);

    auto core = fed->getCorePointer();
    ASSERT_TRUE((core));

    auto name = std::string(core->getFederateName(fed->getID()));

    EXPECT_EQ(name, fed->getName());
    EXPECT_TRUE(fed->getCurrentMode() == helics::Federate::Modes::STARTUP);
    fed->enterInitializingMode();
    EXPECT_TRUE(fed->getCurrentMode() == helics::Federate::Modes::INITIALIZING);
    fed->enterExecutingMode();
    EXPECT_TRUE(fed->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    EXPECT_TRUE(fed->getFlagOption(helics::defs::SINGLE_THREAD_FEDERATE));
    fed = nullptr;  // force the destructor
}

TEST(federate, renamer)
{
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString = "--autobroker";
    auto fed = std::make_shared<helics::Federate>("test_${#}", fedInfo);

    auto core = fed->getCorePointer();
    ASSERT_TRUE((core));

    auto name = std::string(core->getFederateName(fed->getID()));

    EXPECT_EQ(name, fed->getName());
    EXPECT_EQ(name, "test_1");

    fedInfo.coreInitString.clear();
    auto fed2 = std::make_shared<helics::Federate>("test_${#}", fedInfo);
    EXPECT_EQ("test_2", fed2->getName());

    auto fed3 = std::make_shared<helics::Federate>("test_${#}", fed->getCorePointer(), fedInfo);
    EXPECT_EQ("test_3", fed3->getName());

    fed->enterInitializingModeAsync();
    fed3->enterInitializingModeAsync();
    fed2->enterInitializingMode();
    fed->enterInitializingModeComplete();
    fed3->enterInitializingModeComplete();

    fed = nullptr;  // force the destructor
    fed2 = nullptr;
    fed3 = nullptr;
}

TEST(federate_tests, federate_initialize_iterate)
{
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString = "--autobroker";

    auto Fed = std::make_shared<helics::Federate>("test1", fedInfo);

    auto core = Fed->getCorePointer();
    ASSERT_TRUE((core));

    auto name = std::string(core->getFederateName(Fed->getID()));

    EXPECT_EQ(name, Fed->getName());
    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::Modes::STARTUP);
    Fed->enterInitializingModeIterative();
    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::Modes::STARTUP);
    Fed->enterInitializingModeIterative();
    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::Modes::STARTUP);
    Fed->enterInitializingMode();
    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::Modes::INITIALIZING);
    Fed->enterExecutingMode();
    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    // const auto& coreName = core->getIdentifier();
    // const auto& fedName = Fed->getName();
    // EXPECT_EQ(fedName+"_core", coreName);

    // const auto& coreName = core->getIdentifier();
    // const auto& fedName = Fed->getName();
    // EXPECT_EQ(fedName+"_core", coreName);
    Fed = nullptr;  // force the destructor
}

#ifdef HELICS_ENABLE_ZMQ_CORE
TEST(federate_tests, federate_initialize_json)
{
    helics::BrokerApp brk(helics::CoreType::ZMQ);

    helics::FederateInfo fedInfo(helics::CoreType::ZMQ);
    fedInfo.coreInitString = "--json";

    auto Fed = std::make_shared<helics::Federate>("test1", fedInfo);

    auto core = Fed->getCorePointer();
    ASSERT_TRUE((core));

    auto name = std::string(core->getFederateName(Fed->getID()));

    EXPECT_EQ(name, Fed->getName());
    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::Modes::STARTUP);
    Fed->enterInitializingMode();
    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::Modes::INITIALIZING);
    Fed->enterExecutingMode();
    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    Fed = nullptr;  // force the destructor
    brk.waitForDisconnect();
}

TEST(federate_tests, federate_initialize_iteration_multiple)
{
    helics::BrokerApp brk(helics::CoreType::ZMQ);

    helics::FederateInfo fedInfo(helics::CoreType::ZMQ);

    auto Fed1 = std::make_shared<helics::Federate>("test1", fedInfo);
    auto Fed2 = std::make_shared<helics::Federate>("test2", fedInfo);

    Fed1->enterInitializingModeAsync();
    Fed2->enterInitializingModeIterative();
    EXPECT_EQ(Fed1->getCurrentMode(), helics::Federate::Modes::PENDING_INIT);
    EXPECT_EQ(Fed2->getCurrentMode(), helics::Federate::Modes::STARTUP);
    EXPECT_FALSE(Fed1->isAsyncOperationCompleted());

    Fed2->enterInitializingModeIterativeAsync();
    EXPECT_TRUE(Fed2->getCurrentMode() == helics::Federate::Modes::PENDING_ITERATIVE_INIT ||
                Fed2->getCurrentMode() == helics::Federate::Modes::STARTUP);
    std::this_thread::yield();
    int count{0};
    while (!Fed2->isAsyncOperationCompleted()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if (++count > 20) {
            break;
        }
    }
    EXPECT_TRUE(Fed2->isAsyncOperationCompleted());
    Fed2->enterInitializingModeIterativeComplete();
    EXPECT_EQ(Fed2->getCurrentMode(), helics::Federate::Modes::STARTUP);
    Fed2->enterInitializingMode();
    Fed1->enterInitializingModeComplete();
    EXPECT_EQ(Fed1->getCurrentMode(), helics::Federate::Modes::INITIALIZING);
    EXPECT_EQ(Fed2->getCurrentMode(), helics::Federate::Modes::INITIALIZING);
    Fed1->disconnect();
    Fed2->disconnect();
    brk.waitForDisconnect();
}

#endif

TEST(federate, federate_initialize_tests_env)
{
    setEnvironmentVariable("HELICS_LOG_LEVEL", "connections");
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString = "--autobroker";

    auto Fed = std::make_shared<helics::Federate>("test1", fedInfo);

    auto core = Fed->getCorePointer();
    ASSERT_TRUE((core));

    auto cloglevel = core->getIntegerProperty(helics::gLocalCoreId, HELICS_PROPERTY_INT_LOG_LEVEL);
    EXPECT_EQ(cloglevel, HELICS_LOG_LEVEL_CONNECTIONS);

    Fed->enterExecutingMode();
    EXPECT_EQ(Fed->getIntegerProperty(HELICS_PROPERTY_INT_LOG_LEVEL), HELICS_LOG_LEVEL_CONNECTIONS);
    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    Fed->finalize();
    clearEnvironmentVariable("HELICS_LOG_LEVEL");
    // const auto& coreName = core->getIdentifier();
    // const auto& fedName = Fed->getName();
    // EXPECT_EQ(fedName+"_core", coreName);
    Fed = nullptr;  // force the destructor
}
/*  re-enable this test once log levels support numerical values again
TEST(federate, federate_initialize_tests_env2)
{
    setEnvironmentVariable("HELICS_BROKER_LOG_LEVEL", std::to_string(HELICS_LOG_LEVEL_CONNECTIONS));
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString = "--autobroker";

    auto Fed = std::make_shared<helics::Federate>("test1", fedInfo);

    auto core = Fed->getCorePointer();
    ASSERT_TRUE((core));

    auto cloglevel = core->getIntegerProperty(helics::gLocalCoreId, HELICS_PROPERTY_INT_LOG_LEVEL);
    EXPECT_EQ(cloglevel, HELICS_LOG_LEVEL_CONNECTIONS);

    Fed->enterExecutingMode();
    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    Fed->finalize();
    clearEnvironmentVariable("HELICS_BROKER_LOG_LEVEL");
    // const auto& coreName = core->getIdentifier();
    // const auto& fedName = Fed->getName();
    // EXPECT_EQ(fedName+"_core", coreName);
    Fed = nullptr;  // force the destructor
}
*/
TEST(federate, time_step_tests)
{
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString = "--autobroker";

    auto Fed = std::make_shared<helics::Federate>("test1", fedInfo);

    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::Modes::STARTUP);
    Fed->enterInitializingMode();
    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::Modes::INITIALIZING);
    Fed->enterExecutingMode();
    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    auto res = Fed->requestTime(1.0);
    EXPECT_EQ(res, 1.0);
    res = Fed->requestTime(2.0);
    EXPECT_EQ(res, 2.0);

    res = Fed->requestTime(3.0);
    EXPECT_EQ(res, 3.0);
}

TEST(federate, dynamic_join)
{
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString = "--autobroker --broker_init_string='--dynamic --name=dyn_broker'";

    auto fed = std::make_shared<helics::Federate>("test1", fedInfo);

    EXPECT_TRUE(fed->getCurrentMode() == helics::Federate::Modes::STARTUP);
    fed->enterInitializingMode();
    EXPECT_TRUE(fed->getCurrentMode() == helics::Federate::Modes::INITIALIZING);
    fed->enterExecutingMode();
    EXPECT_TRUE(fed->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    auto res = fed->requestTime(1.0);
    EXPECT_EQ(res, 1.0);
    res = fed->requestTime(2.0);
    EXPECT_EQ(res, 2.0);
    // now join a dynamic broker
    fedInfo.coreInitString.clear();
    fedInfo.broker = "dyn_broker";
    decltype(fed) fedDyn;
    EXPECT_NO_THROW(fedDyn = std::make_shared<helics::Federate>("test_dyn", fedInfo));

    fedDyn->enterInitializingMode();
    EXPECT_TRUE(fedDyn->getCurrentMode() == helics::Federate::Modes::INITIALIZING);
    fedDyn->enterExecutingMode();
    EXPECT_TRUE(fedDyn->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    res = fed->requestTime(3.0);
    EXPECT_EQ(res, 3.0);

    res = fedDyn->requestTime(2.5);
    EXPECT_EQ(res, 2.5);
    fed->disconnect();
    fedDyn->disconnect();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    helics::cleanupHelicsLibrary();
}

TEST(federate, broker_disconnect_ci_skip)
{
    auto brk = helics::BrokerFactory::create(CORE_TYPE_TO_TEST, "b1", "-f 1");
    brk->connect();
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);

    fedInfo.coreInitString = "--broker=b1 --tick=200 --timeout=1000";

    auto Fed = std::make_shared<helics::Federate>("test1", fedInfo);

    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::Modes::STARTUP);
    Fed->enterInitializingMode();
    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::Modes::INITIALIZING);
    Fed->enterExecutingMode();
    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    auto res = Fed->requestTime(1.0);
    EXPECT_EQ(res, 1.0);
    res = Fed->requestTime(2.0);
    EXPECT_EQ(res, 2.0);

    res = Fed->requestTime(3.0);
    EXPECT_EQ(res, 3.0);
    brk->disconnect();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto cptr = Fed->getCorePointer();
    EXPECT_TRUE(!cptr->isConnected());
    res = Fed->requestTime(4.0);
    EXPECT_EQ(res, helics::Time::maxVal());
    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::Modes::FINISHED);
}

TEST(federate, index_groups)
{
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString = "--autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_INT_INDEX_GROUP, 3);
    auto fed1 = std::make_shared<helics::Federate>("test1", fedInfo);

    helics::FederateInfo fi2(CORE_TYPE_TO_TEST);
    auto fed2 = std::make_shared<helics::Federate>("test2", fi2);

    fed1->enterInitializingModeAsync();
    fed2->enterInitializingMode();
    fed1->enterInitializingModeComplete();

    auto qres = fed1->query("root", "federate_map");
    auto val = helics::fileops::loadJsonStr(qres);
    EXPECT_GT(val["cores"][0]["federates"][0]["attributes"]["id"].get<int64_t>(),
              val["cores"][0]["federates"][1]["attributes"]["id"].get<int64_t>());
    fed1->finalize();
    fed2->finalize();
}

#ifdef HELICS_ENABLE_ZMQ_CORE
// TODO(PT): make this work for all test types
TEST(federate, bad_broker_error_zmq_ci_skip)
{
    helics::FederateInfo fedInfo(helics::CoreType::ZMQ);
    fedInfo.coreInitString = "--broker=b1 --tick=200 --timeout=800 --networktimeout=400";

    EXPECT_THROW(auto fed = std::make_shared<helics::Federate>("test1", fedInfo),
                 helics::RegistrationFailure);
}

TEST(federate, timeout_error_zmq_ci_skip_nosan)
{
    helics::FederateInfo fedInfo(helics::CoreType::ZMQ);
    fedInfo.coreInitString = "--tick=200 --timeout=800 --networktimeout=400";

    EXPECT_THROW(auto fed = std::make_shared<helics::Federate>("test1", fedInfo),
                 helics::RegistrationFailure);
}

TEST(federate, timeout_abort_zmq_ci_skip_nosan_nocov)
{
    std::future<std::shared_ptr<helics::Federate>> fut;
    auto call = []() {
        helics::FederateInfo fedInfo(helics::CoreType::ZMQ);
        fedInfo.coreInitString = "";
        auto fed = std::make_shared<helics::Federate>("test1", fedInfo);
        return fed;
    };

    auto cret = std::async(std::launch::async, call);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    while (helics::CoreFactory::getCoreCount() == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    helics::CoreFactory::abortAllCores(HELICS_ERROR_USER_ABORT, "aborting55");
    try {
        cret.get();
        EXPECT_TRUE(false);
    }
    catch (const helics::HelicsException& he) {
        EXPECT_THAT(he.what(), ::testing::HasSubstr("aborting55"));
    }
}

#endif

#ifdef HELICS_ENABLE_TCP_CORE
TEST(federate, timeout_abort_tcp_ci_skip_nosan_nocov)
{
    std::future<std::shared_ptr<helics::Federate>> fut;
    auto call = []() {
        helics::FederateInfo fedInfo(helics::CoreType::TCP);
        fedInfo.coreInitString = "";
        auto fed = std::make_shared<helics::Federate>("test1", fedInfo);
        return fed;
    };

    auto cret = std::async(std::launch::async, call);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    while (helics::CoreFactory::getCoreCount() == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    helics::CoreFactory::abortAllCores(HELICS_ERROR_USER_ABORT, "aborting55");
    try {
        cret.get();
        EXPECT_TRUE(false);
    }
    catch (const helics::HelicsException& he) {
        EXPECT_THAT(he.what(), ::testing::HasSubstr("aborting55"));
    }
}

TEST(federate, timeout_abort_tcpss_ci_skip_nosan_nocov)
{
    std::future<std::shared_ptr<helics::Federate>> fut;
    auto call = []() {
        helics::FederateInfo fedInfo(helics::CoreType::TCP_SS);
        fedInfo.coreInitString = "";
        auto fed = std::make_shared<helics::Federate>("test1", fedInfo);
        return fed;
    };

    auto cret = std::async(std::launch::async, call);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    while (helics::CoreFactory::getCoreCount() == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    helics::CoreFactory::abortAllCores(HELICS_ERROR_USER_ABORT, "aborting55");
    try {
        cret.get();
        EXPECT_TRUE(false);
    }
    catch (const helics::HelicsException& he) {
        EXPECT_THAT(he.what(), ::testing::HasSubstr("aborting55"));
    }
}
#endif

#ifdef HELICS_ENABLE_UDP_CORE
TEST(federate, timeout_abort_udp_ci_skip_nosan_nocov)
{
    std::future<std::shared_ptr<helics::Federate>> fut;
    auto call = []() {
        helics::FederateInfo fedInfo(helics::CoreType::UDP);
        fedInfo.coreInitString = "";
        auto fed = std::make_shared<helics::Federate>("test1", fedInfo);
        return fed;
    };

    auto cret = std::async(std::launch::async, call);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    while (helics::CoreFactory::getCoreCount() == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    helics::CoreFactory::abortAllCores(HELICS_ERROR_USER_ABORT, "aborting55");
    try {
        cret.get();
        EXPECT_TRUE(false);
    }
    catch (const helics::HelicsException& he) {
        EXPECT_THAT(he.what(), ::testing::HasSubstr("aborting55"));
    }
}

#endif

TEST(federate, federate_multiple_federates)
{
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreName = "core1-mult";
    fedInfo.coreInitString = "-f 2 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);

    auto Fed2 = std::make_shared<helics::Federate>("fed2", fedInfo);

    EXPECT_TRUE(Fed1->getCurrentMode() == helics::Federate::Modes::STARTUP);
    EXPECT_TRUE(Fed2->getCurrentMode() == helics::Federate::Modes::STARTUP);

    EXPECT_TRUE(Fed1->getID() != Fed2->getID());

    auto f1finish = std::async(std::launch::async, [&]() { Fed1->enterInitializingMode(); });
    Fed2->enterInitializingMode();

    f1finish.wait();
    EXPECT_TRUE(Fed1->getCurrentMode() == helics::Federate::Modes::INITIALIZING);
    EXPECT_TRUE(Fed2->getCurrentMode() == helics::Federate::Modes::INITIALIZING);

    f1finish = std::async(std::launch::async, [&]() { Fed1->enterExecutingMode(); });
    Fed2->enterExecutingMode();
    f1finish.wait();
    EXPECT_TRUE(Fed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    EXPECT_TRUE(Fed2->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    auto f1step = std::async(std::launch::async, [&]() { return Fed1->requestTime(1.0); });
    auto f2step = Fed2->requestTime(1.0);

    auto f1stepVal = f1step.get();
    EXPECT_EQ(f2step, 1.0);
    EXPECT_EQ(f1stepVal, 1.0);

    EXPECT_EQ(Fed1->getCurrentTime(), 1.0);

    f1step = std::async(std::launch::async, [&]() { return Fed1->requestTime(3.0); });
    f2step = Fed2->requestTime(3.0);

    f1stepVal = f1step.get();
    EXPECT_EQ(f2step, 3.0);
    EXPECT_EQ(f1stepVal, 3.0);

    EXPECT_THROW(Fed1->enterInitializingMode(), helics::InvalidFunctionCall);
    Fed1->finalize();
    Fed2->finalize();
}

/** the same as the previous test except with multiple cores and a single broker*/
TEST(federate, multiple_federates_multi_cores)
{
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreName = "core_mc1";
    fedInfo.coreInitString = "--autobroker --broker=brk1";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);
    fedInfo.coreName = "core_mc2";

    auto Fed2 = std::make_shared<helics::Federate>("fed2", fedInfo);

    EXPECT_TRUE(Fed1->getCurrentMode() == helics::Federate::Modes::STARTUP);
    EXPECT_TRUE(Fed2->getCurrentMode() == helics::Federate::Modes::STARTUP);

    auto f1finish = std::async(std::launch::async, [&]() { Fed1->enterInitializingMode(); });
    Fed2->enterInitializingMode();

    f1finish.wait();
    EXPECT_TRUE(Fed1->getCurrentMode() == helics::Federate::Modes::INITIALIZING);
    EXPECT_TRUE(Fed2->getCurrentMode() == helics::Federate::Modes::INITIALIZING);

    f1finish = std::async(std::launch::async, [&]() { Fed1->enterExecutingMode(); });
    Fed2->enterExecutingMode();
    f1finish.wait();
    EXPECT_TRUE(Fed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    EXPECT_TRUE(Fed2->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    auto f1step = std::async(std::launch::async, [&]() { return Fed1->requestTime(1.0); });
    auto f2step = Fed2->requestTime(1.0);

    auto f1stepVal = f1step.get();
    EXPECT_EQ(f2step, 1.0);
    EXPECT_EQ(f1stepVal, 1.0);

    EXPECT_EQ(Fed1->getCurrentTime(), 1.0);

    f1step = std::async(std::launch::async, [&]() { return Fed1->requestTime(3.0); });
    f2step = Fed2->requestTime(3.0);

    f1stepVal = f1step.get();
    EXPECT_EQ(f2step, 3.0);
    EXPECT_EQ(f1stepVal, 3.0);

    EXPECT_THROW(Fed1->enterInitializingMode(), helics::InvalidFunctionCall);
    Fed1->finalize();
    Fed2->finalize();
}

TEST(federate, multiple_federates_async_calls)
{
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreName = "core_async";
    fedInfo.coreInitString = "-f 2 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);

    auto Fed2 = std::make_shared<helics::Federate>("fed2", fedInfo);

    EXPECT_TRUE(Fed1->getCurrentMode() == helics::Federate::Modes::STARTUP);
    EXPECT_TRUE(Fed2->getCurrentMode() == helics::Federate::Modes::STARTUP);

    EXPECT_NE(Fed1->getID(), Fed2->getID());

    Fed1->enterInitializingModeAsync();
    Fed2->enterInitializingMode();
    EXPECT_NO_THROW(Fed2->enterInitializingMode());

    auto core1 = Fed1->getCorePointer();
    auto core2 = Fed2->getCorePointer();
    EXPECT_EQ(core1->getIdentifier(), core2->getIdentifier());
    core1.reset();
    core2.reset();

    Fed1->enterInitializingModeComplete();

    EXPECT_TRUE(Fed1->getCurrentMode() == helics::Federate::Modes::INITIALIZING);
    EXPECT_TRUE(Fed2->getCurrentMode() == helics::Federate::Modes::INITIALIZING);

    Fed1->enterExecutingModeAsync();
    Fed2->enterExecutingMode();
    Fed1->enterExecutingModeComplete();
    EXPECT_TRUE(Fed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    EXPECT_TRUE(Fed2->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    Fed1->requestTimeAsync(1.0);
    auto f2step = Fed2->requestTime(1.0);

    auto f1stepVal = Fed1->requestTimeComplete();
    EXPECT_EQ(f2step, 1.0);
    EXPECT_EQ(f1stepVal, 1.0);

    EXPECT_EQ(Fed1->getCurrentTime(), 1.0);

    Fed1->requestTimeAsync(3.0);
    f2step = Fed2->requestTime(3.0);

    f1stepVal = Fed1->requestTimeComplete();
    EXPECT_EQ(f2step, 3.0);
    EXPECT_EQ(f1stepVal, 3.0);

    EXPECT_THROW(Fed1->enterInitializingMode(), helics::InvalidFunctionCall);
    EXPECT_THROW(Fed1->requestTimeComplete(), helics::InvalidFunctionCall);
    Fed1->finalize();
    Fed2->finalize();
}

TEST(federate, missing_core)
{
    helics::FederateInfo fedInfo(helics::CoreType::NULLCORE);
    fedInfo.coreName = "core_missing";
    fedInfo.coreInitString = "-f 1";

    EXPECT_THROW(auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo),
                 helics::HelicsException);
}

TEST(federate, not_open)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_full";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);
    Fed1->enterExecutingMode();

    EXPECT_THROW(auto fed2 = std::make_shared<helics::Federate>("fed2", fedInfo),
                 helics::RegistrationFailure);
    Fed1->finalize();
}

TEST(federate, coreApp)
{
    helics::CoreApp capp(helics::CoreType::TEST, "corename", "-f 1 --autobroker");
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    auto Fed1 = std::make_shared<helics::Federate>("fed1", capp, fedInfo);
    EXPECT_NO_THROW(Fed1->enterExecutingMode());

    Fed1->finalize();
}

TEST(federate, core_ptr)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_ptr";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", nullptr, fedInfo);
    Fed1->enterExecutingMode();

    EXPECT_THROW(auto fed2 = std::make_shared<helics::Federate>("fed2", nullptr, fedInfo),
                 helics::RegistrationFailure);
    Fed1->finalize();
}

TEST(federate, core_ptr_no_name)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>(std::string{}, nullptr, fedInfo);
    Fed1->enterExecutingMode();
    Fed1->finalize();
}

TEST(federate, from_string)
{
    auto Fed1 = std::make_shared<helics::Federate>(
        "fed1", "--coretype=TEST --corename core_init --coreinitstring='-f 1 --autobroker'");
    Fed1->enterExecutingMode();

    auto core = Fed1->getCorePointer();
    EXPECT_EQ(core->getIdentifier(), "core_init");
    Fed1->finalize();
    core.reset();
}

TEST(federate, from_file1)
{
    auto fstr1 = std::string(TEST_DIR) + "example_filters.json";
    auto Fed1 = std::make_shared<helics::Federate>(fstr1);
    EXPECT_NO_THROW(Fed1->enterExecutingMode());
    Fed1->finalize();
}

TEST(federate, from_file3)
{
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
    auto fstr1 = std::string(TEST_DIR) + "example_unusual_filters.json";
    auto Fed1 = std::make_shared<helics::Federate>(fstr1);
    EXPECT_THROW(Fed1->enterExecutingMode(), helics::HelicsException);
    Fed1->finalize();
    Fed1->getCorePointer()->disconnect();
}

TEST(federate, from_file4)
{
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
    auto fstr1 = std::string(TEST_DIR) + "example_unusual_filters2.json";
    auto Fed1 = std::make_shared<helics::Federate>(fstr1);
    EXPECT_NO_THROW(Fed1->enterExecutingMode());
    Fed1->finalize();
}

TEST(federate, from_file2)
{
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
    auto fstr2 = std::string(TEST_DIR) + "example_filters.toml";
    auto Fed1 = std::make_shared<helics::Federate>(fstr2);
    EXPECT_NO_THROW(Fed1->enterExecutingMode());
    Fed1->finalize();
}

TEST(federate, from_file_invalid)
{
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
    auto fstr2 = std::string(TEST_DIR) + "invalid_filter_operation.json";
    std::shared_ptr<helics::Federate> Fed1;
    EXPECT_THROW(Fed1 = std::make_shared<helics::Federate>(fstr2), std::exception);
}

TEST(federate, from_file_invalid_translator)
{
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
    auto fstr2 = std::string(TEST_DIR) + "invalid_translator_operation.json";
    std::shared_ptr<helics::Federate> Fed1;
    EXPECT_THROW(Fed1 = std::make_shared<helics::Federate>(fstr2), std::exception);
}

TEST(federate, from_file5)
{
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
    auto fstr2 = "non_existing.toml";
    EXPECT_THROW(auto fed = std::make_shared<helics::Federate>(fstr2), helics::InvalidParameter);
}

TEST(federate, from_file6)
{
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();

    auto fstr2 = std::string(TEST_DIR) + "example_filters.toml";
    auto Fed1 = std::make_shared<helics::Federate>(fstr2);
    EXPECT_THROW(Fed1->registerFilterInterfaces("non_existing.toml"), helics::InvalidParameter);
    EXPECT_THROW(Fed1->registerFilterInterfaces("non_existing.json"), helics::InvalidParameter);
}

TEST(federate, from_file7)
{
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
    auto fstr3 = std::string(TEST_DIR) + "example_unusual_filters3.json";
    auto Fed1 = std::make_shared<helics::Federate>(fstr3);
    EXPECT_NO_THROW(Fed1->enterExecutingMode());
    Fed1->finalize();
}

TEST(federate, from_file8)
{
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
    auto fstr3 = std::string(TEST_DIR) + "unusual_filters.toml";
    auto Fed1 = std::make_shared<helics::Federate>(fstr3);
    EXPECT_NO_THROW(Fed1->enterExecutingMode());
    Fed1->finalize();
}

TEST(federate, from_file9)
{
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
    auto fstr4 = std::string(TEST_DIR) + "example_filters.toml";
    auto Fed1 = std::make_shared<helics::Federate>(fstr4);
    EXPECT_NO_THROW(Fed1->enterExecutingMode());
    EXPECT_THROW(Fed1->registerFilterInterfaces(std::string(TEST_DIR) + "unusual_filters2.toml"),
                 helics::InvalidParameter);
    Fed1->finalize();
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
}

TEST(federate, from_file10)
{
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();

    auto fstr2 = std::string(LONG_TEST_NAME);
    if (fstr2.size() < FILENAME_MAX) {
        // this test would fail if the file name exceeds the max filename length
        std::shared_ptr<helics::Federate> Fed1;
        EXPECT_NO_THROW(Fed1 = std::make_shared<helics::Federate>(fstr2));
        if (Fed1) {
            Fed1->finalize();
        }
    }
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
}

TEST(federate, from_string2)
{
    auto Fed1 = std::make_shared<helics::Federate>(
        "--name=fed1 --coretype=TEST --corename core_init --coreinitstring='-f 1 --autobroker'");
    Fed1->enterExecutingMode();

    EXPECT_EQ(Fed1->getName(), "fed1");
    Fed1->finalize();
}

TEST(federate, enterInit)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_full_a";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);
    Fed1->enterInitializingModeAsync();
    // make sure it doesn't error if called twice
    EXPECT_NO_THROW(Fed1->enterInitializingModeAsync());
    EXPECT_NO_THROW(Fed1->isAsyncOperationCompleted());
    EXPECT_NO_THROW(Fed1->enterInitializingMode());
    EXPECT_NO_THROW(Fed1->enterInitializingModeComplete());

    EXPECT_EQ(Fed1->getCurrentMode(), helics::Federate::Modes::INITIALIZING);
    EXPECT_NO_THROW(Fed1->finalize());
}

TEST(federate, enterInitComplete)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_full_b";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);
    // this should be the same as just calling enterInitializingMode
    EXPECT_NO_THROW(Fed1->enterInitializingModeComplete());

    EXPECT_EQ(Fed1->getCurrentMode(), helics::Federate::Modes::INITIALIZING);
    Fed1->finalize();
}

TEST(federate, enterExec)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_full_c";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);
    Fed1->enterInitializingModeAsync();
    Fed1->setProperty(helics::defs::Properties::TIME_DELTA, helics::Time(1.0));
    // make sure it doesn't error if called twice
    EXPECT_NO_THROW(Fed1->enterInitializingModeAsync());
    EXPECT_NO_THROW(Fed1->enterExecutingModeAsync());
    EXPECT_NO_THROW(Fed1->isAsyncOperationCompleted());
    EXPECT_NO_THROW(Fed1->enterExecutingMode());
    EXPECT_NO_THROW(Fed1->enterExecutingModeComplete());
    EXPECT_NO_THROW(Fed1->enterExecutingModeAsync());
    EXPECT_EQ(Fed1->getCurrentMode(), helics::Federate::Modes::EXECUTING);
    Fed1->finalizeComplete();
}

TEST(federate, enterExecAfterFinal)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_full_ec";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);
    Fed1->enterInitializingMode();
    auto core = Fed1->getCorePointer();
    core->disconnect();

    auto iterating = Fed1->enterExecutingMode();
    EXPECT_EQ(iterating, helics::IterationResult::HALTED);

    Fed1->finalize();
}

TEST(federate, enterExecAfterFinalAsync)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_full_eca";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);
    Fed1->enterInitializingMode();
    auto core = Fed1->getCorePointer();
    core->disconnect();

    Fed1->enterExecutingModeAsync();
    auto iterating = Fed1->enterExecutingModeComplete();
    EXPECT_EQ(iterating, helics::IterationResult::HALTED);

    Fed1->finalize();
}

TEST(federate, iterativeTimeRequestHalt)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_full_eca1";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);
    Fed1->enterExecutingMode();

    auto core = Fed1->getCorePointer();
    core->disconnect();

    auto itTime = Fed1->requestTimeIterative(2.0, helics::IterationRequest::FORCE_ITERATION);
    EXPECT_EQ(itTime.state, helics::IterationResult::HALTED);
    EXPECT_EQ(itTime.grantedTime, helics::Time::maxVal());

    Fed1->finalize();
}

TEST(federate, iterativeTimeRequestAsyncHalt)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_full_eca2";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);
    Fed1->enterExecutingMode();

    auto core = Fed1->getCorePointer();
    core->disconnect();

    Fed1->requestTimeIterativeAsync(2.0, helics::IterationRequest::FORCE_ITERATION);
    auto itTime = Fed1->requestTimeIterativeComplete();
    EXPECT_EQ(itTime.state, helics::IterationResult::HALTED);
    EXPECT_EQ(itTime.grantedTime, helics::Time::maxVal());

    Fed1->finalize();
}

TEST(federate, enterExecAsync)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_full_d";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);

    Fed1->enterExecutingModeAsync();
    EXPECT_NO_THROW(Fed1->enterExecutingModeAsync());
    EXPECT_NO_THROW(Fed1->finalize());
}

TEST(federate, enterExecAsyncIterative)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_full_e";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);

    Fed1->enterExecutingModeAsync(helics::IterationRequest::FORCE_ITERATION);
    EXPECT_NO_THROW(Fed1->enterExecutingModeAsync());
    auto res = Fed1->enterExecutingModeComplete();
    EXPECT_EQ(res, helics::IterationResult::ITERATING);
    EXPECT_EQ(Fed1->getCurrentMode(), helics::Federate::Modes::INITIALIZING);
    EXPECT_NO_THROW(Fed1->finalizeAsync());
    EXPECT_NO_THROW(Fed1->finalizeAsync());
    EXPECT_NO_THROW(Fed1->finalize());
}

TEST(federate, enterRequestTimeAsyncIterative)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_full_e";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);

    Fed1->enterExecutingMode();
    Fed1->requestTimeAsync(1.0);
    while (!Fed1->isAsyncOperationCompleted()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    EXPECT_TRUE(Fed1->isAsyncOperationCompleted());
    EXPECT_NO_THROW(Fed1->enterExecutingMode());
    EXPECT_EQ(Fed1->getCurrentTime(), 1.0);
    EXPECT_EQ(Fed1->getCurrentMode(), helics::Federate::Modes::EXECUTING);

    Fed1->requestTimeIterativeAsync(37.0, helics::IterationRequest::FORCE_ITERATION);
    while (!Fed1->isAsyncOperationCompleted()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    auto itime = Fed1->requestTimeIterativeComplete();
    EXPECT_EQ(itime.grantedTime, 1.0);
    EXPECT_EQ(itime.state, helics::IterationResult::ITERATING);
    Fed1->requestTimeIterativeAsync(1.0, helics::IterationRequest::FORCE_ITERATION);
    Fed1->finalizeAsync();
    while (!Fed1->isAsyncOperationCompleted()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    EXPECT_TRUE(Fed1->isAsyncOperationCompleted());
    Fed1->finalizeComplete();
}

TEST(federate, enterRequestTimeAsyncIterativeFinalize)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_full_e2";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);

    Fed1->enterExecutingMode();
    Fed1->requestTimeIterativeAsync(1.0, helics::IterationRequest::FORCE_ITERATION);
    EXPECT_NO_THROW(Fed1->finalize());
    // check time results after finalize
    auto time = Fed1->requestTime(3.0);
    EXPECT_EQ(time, helics::Time::maxVal());
}

TEST(federate, enterRequestTimeAsyncFinalize)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_full_e3";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);

    Fed1->enterExecutingMode();
    Fed1->requestTimeAsync(1.0);
    EXPECT_NO_THROW(Fed1->finalize());
    EXPECT_EQ(Fed1->getCurrentMode(), helics::Federate::Modes::FINALIZE);
}

TEST(federate, enterRequestTimeAsyncFinalizeAsync)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_full_e3a";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);

    Fed1->enterExecutingMode();
    Fed1->requestTimeAsync(1.0);
    EXPECT_NO_THROW(Fed1->finalizeAsync());
    Fed1->finalizeComplete();
    EXPECT_EQ(Fed1->getCurrentMode(), helics::Federate::Modes::FINALIZE);
}

TEST(federate, enterEnterExecAsyncFinalize)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_full_e4";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);
    Fed1->enterInitializingMode();
    Fed1->enterExecutingModeAsync();
    EXPECT_NO_THROW(Fed1->finalize());
    EXPECT_EQ(Fed1->getCurrentMode(), helics::Federate::Modes::FINALIZE);
}

TEST(federate, enterEnterInitAsyncFinalize)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_full_e5";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);
    Fed1->enterInitializingModeAsync();
    EXPECT_NO_THROW(Fed1->finalize());
    EXPECT_EQ(Fed1->getCurrentMode(), helics::Federate::Modes::FINALIZE);
}

TEST(federate, enterEnterExecAsyncFinalizeAsync)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_full_e4a";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);
    Fed1->enterInitializingMode();
    Fed1->enterExecutingModeAsync();
    EXPECT_NO_THROW(Fed1->finalizeAsync());
    Fed1->finalizeComplete();
    EXPECT_EQ(Fed1->getCurrentMode(), helics::Federate::Modes::FINALIZE);
}

TEST(federate, enterEnterInitAsyncFinalizeAsync)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_full_e5a";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);
    Fed1->enterInitializingModeAsync();
    EXPECT_NO_THROW(Fed1->finalizeAsync());
    Fed1->finalizeComplete();
    EXPECT_EQ(Fed1->getCurrentMode(), helics::Federate::Modes::FINALIZE);
}

TEST(federate, enterExecPendingTimeIterative)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_full_epa";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);
    Fed1->enterExecutingMode();
    Fed1->requestTimeIterativeAsync(2.0, helics::IterationRequest::FORCE_ITERATION);
    auto result = Fed1->enterExecutingMode();
    EXPECT_EQ(result, helics::IterationResult::NEXT_STEP);
    Fed1->finalizeComplete();
    EXPECT_EQ(Fed1->getCurrentMode(), helics::Federate::Modes::FINALIZE);
}

TEST(federate, forceErrorExec)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_full_fe";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);

    Fed1->enterExecutingMode();
    Fed1->localError(9827);

    EXPECT_THROW(Fed1->enterInitializingMode(), helics::InvalidFunctionCall);
    EXPECT_THROW(Fed1->enterExecutingMode(), helics::InvalidFunctionCall);

    Fed1->getCorePointer()->disconnect();
}

TEST(federate, forceErrorExecAsync)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_full_fe1";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);

    Fed1->enterExecutingModeAsync(helics::IterationRequest::FORCE_ITERATION);
    Fed1->localError(9827);

    EXPECT_THROW(Fed1->enterInitializingMode(), helics::InvalidFunctionCall);
    EXPECT_THROW(Fed1->enterExecutingMode(), helics::InvalidFunctionCall);

    Fed1->getCorePointer()->disconnect();
}

TEST(federate, forceErrorInitAsync)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_full_fe2";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);

    Fed1->enterInitializingModeAsync();
    Fed1->localError(9827);

    EXPECT_THROW(Fed1->enterInitializingMode(), helics::InvalidFunctionCall);
    EXPECT_THROW(Fed1->enterExecutingMode(), helics::InvalidFunctionCall);

    Fed1->getCorePointer()->disconnect();
}

TEST(federate, forceErrorPendingTimeAsync)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_full_fe3";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);

    Fed1->enterExecutingMode();
    Fed1->requestTimeAsync(2.0);
    Fed1->globalError(9827);

    EXPECT_THROW(Fed1->requestTime(3.0), helics::InvalidFunctionCall);

    Fed1->getCorePointer()->disconnect();
}

TEST(federate, federateGeneratedLocalError)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_full_le";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);

    Fed1->enterExecutingMode();
    Fed1->localError(9827, "user generated error");

    EXPECT_THROW(Fed1->requestTime(3.0), helics::HelicsException);

    Fed1->getCorePointer()->disconnect();
    Fed1->disconnect();
    EXPECT_THROW(Fed1->localError(9827, "user generated error2"), helics::FederateError);
}

TEST(federate, federateGeneratedGlobalError)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_full_ge";
    fedInfo.coreInitString = "-f 1 --autobroker --error_timeout=0";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);

    Fed1->enterExecutingMode();
    Fed1->requestTimeAsync(2.0);
    Fed1->globalError(9827, "user generated global error");

    EXPECT_THROW(Fed1->requestTime(3.0), helics::HelicsException);

    EXPECT_TRUE(Fed1->getCorePointer()->waitForDisconnect(std::chrono::milliseconds(300)));

    Fed1->disconnect();
    EXPECT_THROW(Fed1->globalError(9827, "user generated global error2"), helics::FederateError);
}

TEST(federate, federateGeneratedlocalErrorEscalation)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_full_ge";
    fedInfo.coreInitString = "-f 1 --autobroker --error_timeout=0";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);
    Fed1->setFlagOption(helics::defs::Flags::TERMINATE_ON_ERROR);
    Fed1->enterExecutingMode();
    Fed1->requestTimeAsync(2.0);
    Fed1->localError(9827, "user generated global error");

    EXPECT_THROW(Fed1->requestTime(3.0), helics::HelicsException);

    EXPECT_TRUE(Fed1->getCorePointer()->waitForDisconnect(std::chrono::milliseconds(300)));

    Fed1->disconnect();
    EXPECT_THROW(Fed1->globalError(9827, "user generated global error2"), helics::FederateError);
}

TEST(federate, queryTest1)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_q";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed_q", fedInfo);

    Fed1->enterExecutingMode();

    auto qres = Fed1->query("name");
    EXPECT_EQ(qres, "\"fed_q\"");
    qres = Fed1->query("corename");
    auto core = Fed1->getCorePointer();
    EXPECT_EQ(qres, std::string("\"") + core->getIdentifier() + '"');
    qres = Fed1->query("federate", "name");
    EXPECT_EQ(qres, "\"fed_q\"");

    core.reset();
    Fed1->disconnect();
    qres = Fed1->query("corename");
    // core name should be empty after disconnect
    EXPECT_NE(qres.find("\"\""), std::string::npos);
    qres = Fed1->query("subscriptions");
    EXPECT_NE(qres.find("error"), std::string::npos);
    qres = Fed1->query("root", "subscriptions");
    EXPECT_NE(qres.find("error"), std::string::npos);

    qres = Fed1->queryComplete(helics::QueryId{4});
    EXPECT_NE(qres.find("error"), std::string::npos);

    EXPECT_FALSE(Fed1->isQueryCompleted(helics::QueryId{2}));
    EXPECT_NO_THROW(Fed1->logMessage(10, "test log message"));
}

TEST(federate, forceErrorPendingTimeIterativeAsync)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_full_fe4";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);

    Fed1->enterExecutingMode();
    Fed1->requestTimeIterativeAsync(2.0, helics::IterationRequest::NO_ITERATIONS);
    Fed1->localError(9827);

    EXPECT_THROW(Fed1->requestTime(3.0), helics::InvalidFunctionCall);

    Fed1->getCorePointer()->disconnect();
}

TEST(federate, forceErrorFinalizeAsync)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_full_fe5";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);

    Fed1->enterExecutingMode();
    Fed1->finalizeAsync();
    Fed1->localError(9827);

    EXPECT_THROW(Fed1->requestTime(3.0), helics::InvalidFunctionCall);

    Fed1->getCorePointer()->disconnect();
}

TEST(federate, error_after_disconnect)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_full_g";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);
    auto& filt1 = Fed1->registerGlobalFilter("filt1", "type1", "type2");
    Fed1->enterExecutingMode();
    Fed1->disconnect();

    const auto& Fedref = *Fed1;
    auto& filt1ref1 = Fedref.getFilter(0);
    auto& filt1ref2 = Fedref.getFilter("filt1");
    auto& filt1ref3 = Fedref.getFilter("notafilter");
    auto& filt1ref4 = Fed1->getFilter("filt1");
    EXPECT_EQ(filt1ref1.getName(), filt1.getName());
    EXPECT_EQ(filt1ref2.getName(), filt1.getName());
    EXPECT_EQ(filt1ref4.getName(), filt1.getName());
    EXPECT_FALSE(filt1ref3.isValid());

    EXPECT_NO_THROW(Fed1->setGlobal("global1", "global1"));
    EXPECT_THROW(filt1.addSourceTarget("ept"), helics::InvalidFunctionCall);
    EXPECT_NO_THROW(Fed1->addDependency("otherFed"));

    EXPECT_THROW(filt1.addDestinationTarget("ept"), helics::InvalidFunctionCall);
    EXPECT_NO_THROW(filt1.setOperator({}));

    EXPECT_THROW(Fed1->localError(99), helics::FederateError);
    EXPECT_THROW(Fed1->globalError(99), helics::FederateError);
}

static constexpr const char* simple_global_files[] = {"example_globals1.json",
                                                      "example_globals1.toml",
                                                      "example_globals2.json"};

class FederateGlobalFiles: public ::testing::TestWithParam<const char*> {};

TEST_P(FederateGlobalFiles, global_file_ci_skip)
{
    auto brk = helics::BrokerFactory::create(helics::CoreType::TEST, "b1", "-f 2");
    brk->connect();
    auto testFile = std::string(TEST_DIR) + GetParam();
    brk->makeConnections(testFile);

    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreName = "core_global1";
    fedInfo.coreInitString = "-f 2";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);

    auto Fed2 = std::make_shared<helics::Federate>("fed2", fedInfo);

    Fed1->enterInitializingModeAsync();
    Fed2->enterInitializingMode();

    Fed1->enterInitializingModeComplete();

    auto str1 = Fed1->query("global_value", "global1");
    EXPECT_EQ(str1, "this is a global1 value");
    str1 = Fed2->query("global_value", "global1");
    EXPECT_EQ(str1, "this is a global1 value");

    str1 = Fed1->query("global_value", "global2");
    EXPECT_EQ(str1, "this is another global value");
    str1 = Fed2->query("global_value", "global2");
    EXPECT_EQ(str1, "this is another global value");
    Fed1->finalize();
    Fed2->finalize();
    brk->waitForDisconnect();
}

TEST(federate, broker_global_file_ci_skip)
{
    auto testFile = std::string(TEST_DIR) + "example_global_broker.json";
    auto brk = helics::BrokerFactory::create(helics::CoreType::EXTRACT, "", testFile);
    brk->connect();

    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreName = "core_globalc";
    fedInfo.coreInitString = "-f 2 --broker=brk_globalb";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);

    auto Fed2 = std::make_shared<helics::Federate>("fed2", fedInfo);

    Fed1->enterInitializingModeAsync();
    Fed2->enterInitializingMode();

    Fed1->enterInitializingModeComplete();

    auto str1 = Fed1->query("global_value", "global1");
    EXPECT_EQ(str1, "this is a global1 value");
    str1 = Fed2->query("global_value", "global1");
    EXPECT_EQ(str1, "this is a global1 value");

    str1 = Fed1->query("global_value", "global2");
    EXPECT_EQ(str1, "this is another global value");
    str1 = Fed2->query("global_value", "global2");
    EXPECT_EQ(str1, "this is another global value");
    Fed1->finalize();
    Fed2->finalize();
    brk->waitForDisconnect();
}

TEST_P(FederateGlobalFiles, core_global_file_ci_skip)
{
    auto brk = helics::BrokerFactory::create(helics::CoreType::TEST, "b1", "-f2");
    brk->connect();

    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreName = "core_global3";
    fedInfo.coreInitString = "-f 1";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fedInfo);
    fedInfo.coreName = "core_global4";
    auto Fed2 = std::make_shared<helics::Federate>("fed2", fedInfo);

    auto core = Fed1->getCorePointer();
    auto testFile = std::string(TEST_DIR) + GetParam();
    core->makeConnections(testFile);
    Fed1->enterInitializingModeAsync();
    Fed2->enterInitializingMode();

    Fed1->enterInitializingModeComplete();

    auto str1 = Fed1->query("global_value", "global1");
    EXPECT_EQ(str1, "this is a global1 value");
    str1 = Fed2->query("global_value", "global1");
    EXPECT_EQ(str1, "this is a global1 value");
    str1 = core->query("global_value", "global1", HELICS_SEQUENCING_MODE_FAST);
    EXPECT_EQ(str1, "this is a global1 value");
    str1 = brk->query("global_value", "global1");
    EXPECT_EQ(str1, "this is a global1 value");

    str1 = Fed1->query("global_value", "global2");
    EXPECT_EQ(str1, "this is another global value");
    str1 = Fed2->query("global_value", "global2");
    EXPECT_EQ(str1, "this is another global value");
    str1 = core->query("global_value", "global2", HELICS_SEQUENCING_MODE_FAST);
    EXPECT_EQ(str1, "this is another global value");
    str1 = brk->query("global_value", "global2");
    EXPECT_EQ(str1, "this is another global value");

    auto str2 = Fed1->query("global_value", "list");
    EXPECT_TRUE((str2 == "[\"global1\",\"global2\"]") || (str2 == "[\"global2\",\"global1\"]"));

    auto str3 = Fed1->query("global_value", "all");
    EXPECT_NE(str3, "#invalid");
    Fed1->finalize();
    Fed2->finalize();
    core = nullptr;
    brk->waitForDisconnect();
}

INSTANTIATE_TEST_SUITE_P(federate, FederateGlobalFiles, ::testing::ValuesIn(simple_global_files));
