/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/CoreApp.hpp"
#include "helics/application_api/Federate.hpp"
#include "helics/application_api/Filters.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/core-exceptions.hpp"
#include "helics/core/helics_definitions.hpp"
#include "testFixtures.hpp"

#include "gmock/gmock.h"
#include <future>
#include <gtest/gtest.h>
/** these test cases test out the value converters
 */

#define CORE_TYPE_TO_TEST helics::core_type::TEST

TEST(federate_tests, federate_initialize_tests)
{
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreInitString = "--autobroker";

    auto Fed = std::make_shared<helics::Federate>("test1", fi);

    auto core = Fed->getCorePointer();
    ASSERT_TRUE((core));

    auto name = std::string(core->getFederateName(Fed->getID()));

    EXPECT_EQ(name, Fed->getName());
    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::modes::startup);
    Fed->enterInitializingMode();
    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::modes::initializing);
    Fed->enterExecutingMode();
    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::modes::executing);

    // const auto& coreName = core->getIdentifier();
    // const auto& fedName = Fed->getName();
    // EXPECT_EQ(fedName+"_core", coreName);
    Fed = nullptr;  // force the destructor
}

TEST(federate_tests, federate_initialize_tests_env)
{
    setEnvironmentVariable("HELICS_LOG_LEVEL", "connections");
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreInitString = "--autobroker";

    auto Fed = std::make_shared<helics::Federate>("test1", fi);

    auto core = Fed->getCorePointer();
    ASSERT_TRUE((core));

    auto cloglevel = core->getIntegerProperty(helics::local_core_id, helics_property_int_log_level);
    EXPECT_EQ(cloglevel, helics_log_level_connections);

    Fed->enterExecutingMode();
    EXPECT_EQ(Fed->getIntegerProperty(helics_property_int_log_level), helics_log_level_connections);
    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::modes::executing);

    Fed->finalize();
    clearEnvironmentVariable("HELICS_LOG_LEVEL");
    // const auto& coreName = core->getIdentifier();
    // const auto& fedName = Fed->getName();
    // EXPECT_EQ(fedName+"_core", coreName);
    Fed = nullptr;  // force the destructor
}

TEST(federate_tests, federate_initialize_tests_env2)
{
    setEnvironmentVariable("HELICS_BROKER_LOG_LEVEL", "3");
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreInitString = "--autobroker";

    auto Fed = std::make_shared<helics::Federate>("test1", fi);

    auto core = Fed->getCorePointer();
    ASSERT_TRUE((core));

    auto cloglevel = core->getIntegerProperty(helics::local_core_id, helics_property_int_log_level);
    EXPECT_EQ(cloglevel, helics_log_level_connections);

    Fed->enterExecutingMode();
    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::modes::executing);

    Fed->finalize();
    clearEnvironmentVariable("HELICS_BROKER_LOG_LEVEL");
    // const auto& coreName = core->getIdentifier();
    // const auto& fedName = Fed->getName();
    // EXPECT_EQ(fedName+"_core", coreName);
    Fed = nullptr;  // force the destructor
}

TEST(federate_tests, time_step_tests)
{
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreInitString = "--autobroker";

    auto Fed = std::make_shared<helics::Federate>("test1", fi);

    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::modes::startup);
    Fed->enterInitializingMode();
    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::modes::initializing);
    Fed->enterExecutingMode();
    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::modes::executing);

    auto res = Fed->requestTime(1.0);
    EXPECT_EQ(res, 1.0);
    res = Fed->requestTime(2.0);
    EXPECT_EQ(res, 2.0);

    res = Fed->requestTime(3.0);
    EXPECT_EQ(res, 3.0);
}

TEST(federate_tests, broker_disconnect_test_ci_skip)
{
    auto brk = helics::BrokerFactory::create(CORE_TYPE_TO_TEST, "b1", "-f 1");
    brk->connect();
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);

    fi.coreInitString = "--broker=b1 --tick=200 --timeout=1000";

    auto Fed = std::make_shared<helics::Federate>("test1", fi);

    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::modes::startup);
    Fed->enterInitializingMode();
    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::modes::initializing);
    Fed->enterExecutingMode();
    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::modes::executing);

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
    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::modes::finished);
}

#ifdef ENABLE_ZMQ_CORE
// TODO(PT): make this work for all test types
TEST(federate_tests, bad_broker_error_zmq_ci_skip)
{
    helics::FederateInfo fi(helics::core_type::ZMQ);
    fi.coreInitString = "--broker=b1 --tick=200 --timeout=800 --networktimeout=400";

    EXPECT_THROW(auto fed = std::make_shared<helics::Federate>("test1", fi),
                 helics::RegistrationFailure);
}

TEST(federate_tests, timeout_error_zmq_ci_skip)
{
    helics::FederateInfo fi(helics::core_type::ZMQ);
    fi.coreInitString = "--tick=200 --timeout=800 --networktimeout=400";

    EXPECT_THROW(auto fed = std::make_shared<helics::Federate>("test1", fi),
                 helics::RegistrationFailure);
}

TEST(federate_tests, timeout_abort_zmq)
{
    std::future<std::shared_ptr<helics::Federate>> fut;
    auto call = []() {
        helics::FederateInfo fi(helics::core_type::ZMQ);
        fi.coreInitString = "";
        auto fed = std::make_shared<helics::Federate>("test1", fi);
        return fed;
    };

    auto cret = std::async(std::launch::async, call);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    while (helics::CoreFactory::getCoreCount() == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    helics::CoreFactory::abortAllCores(helics_error_user_abort, "aborting55");
    try {
        cret.get();
        EXPECT_TRUE(false);
    }
    catch (const helics::HelicsException& he) {
        EXPECT_THAT(he.what(), ::testing::HasSubstr("aborting55"));
    }
}

#endif

#ifdef ENABLE_TCP_CORE
TEST(federate_tests, timeout_abort_tcp)
{
    std::future<std::shared_ptr<helics::Federate>> fut;
    auto call = []() {
        helics::FederateInfo fi(helics::core_type::TCP);
        fi.coreInitString = "";
        auto fed = std::make_shared<helics::Federate>("test1", fi);
        return fed;
    };

    auto cret = std::async(std::launch::async, call);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    while (helics::CoreFactory::getCoreCount() == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    helics::CoreFactory::abortAllCores(helics_error_user_abort, "aborting55");
    try {
        cret.get();
        EXPECT_TRUE(false);
    }
    catch (const helics::HelicsException& he) {
        EXPECT_THAT(he.what(), ::testing::HasSubstr("aborting55"));
    }
}

TEST(federate_tests, timeout_abort_tcpss)
{
    std::future<std::shared_ptr<helics::Federate>> fut;
    auto call = []() {
        helics::FederateInfo fi(helics::core_type::TCP_SS);
        fi.coreInitString = "";
        auto fed = std::make_shared<helics::Federate>("test1", fi);
        return fed;
    };

    auto cret = std::async(std::launch::async, call);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    while (helics::CoreFactory::getCoreCount() == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    helics::CoreFactory::abortAllCores(helics_error_user_abort, "aborting55");
    try {
        cret.get();
        EXPECT_TRUE(false);
    }
    catch (const helics::HelicsException& he) {
        EXPECT_THAT(he.what(), ::testing::HasSubstr("aborting55"));
    }
}
#endif

#ifdef ENABLE_UDP_CORE
TEST(federate_tests, timeout_abort_udp)
{
    std::future<std::shared_ptr<helics::Federate>> fut;
    auto call = []() {
        helics::FederateInfo fi(helics::core_type::UDP);
        fi.coreInitString = "";
        auto fed = std::make_shared<helics::Federate>("test1", fi);
        return fed;
    };

    auto cret = std::async(std::launch::async, call);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    while (helics::CoreFactory::getCoreCount() == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    helics::CoreFactory::abortAllCores(helics_error_user_abort, "aborting55");
    try {
        cret.get();
        EXPECT_TRUE(false);
    }
    catch (const helics::HelicsException& he) {
        EXPECT_THAT(he.what(), ::testing::HasSubstr("aborting55"));
    }
}

#endif

TEST(federate_tests, federate_multiple_federates)
{
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreName = "core1-mult";
    fi.coreInitString = "-f 2 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);

    auto Fed2 = std::make_shared<helics::Federate>("fed2", fi);

    EXPECT_TRUE(Fed1->getCurrentMode() == helics::Federate::modes::startup);
    EXPECT_TRUE(Fed2->getCurrentMode() == helics::Federate::modes::startup);

    EXPECT_TRUE(Fed1->getID() != Fed2->getID());

    auto f1finish = std::async(std::launch::async, [&]() { Fed1->enterInitializingMode(); });
    Fed2->enterInitializingMode();

    f1finish.wait();
    EXPECT_TRUE(Fed1->getCurrentMode() == helics::Federate::modes::initializing);
    EXPECT_TRUE(Fed2->getCurrentMode() == helics::Federate::modes::initializing);

    f1finish = std::async(std::launch::async, [&]() { Fed1->enterExecutingMode(); });
    Fed2->enterExecutingMode();
    f1finish.wait();
    EXPECT_TRUE(Fed1->getCurrentMode() == helics::Federate::modes::executing);
    EXPECT_TRUE(Fed2->getCurrentMode() == helics::Federate::modes::executing);

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
TEST(federate_tests, multiple_federates_multi_cores)
{
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreName = "core_mc1";
    fi.coreInitString = "--autobroker --broker=brk1";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);
    fi.coreName = "core_mc2";

    auto Fed2 = std::make_shared<helics::Federate>("fed2", fi);

    EXPECT_TRUE(Fed1->getCurrentMode() == helics::Federate::modes::startup);
    EXPECT_TRUE(Fed2->getCurrentMode() == helics::Federate::modes::startup);

    auto f1finish = std::async(std::launch::async, [&]() { Fed1->enterInitializingMode(); });
    Fed2->enterInitializingMode();

    f1finish.wait();
    EXPECT_TRUE(Fed1->getCurrentMode() == helics::Federate::modes::initializing);
    EXPECT_TRUE(Fed2->getCurrentMode() == helics::Federate::modes::initializing);

    f1finish = std::async(std::launch::async, [&]() { Fed1->enterExecutingMode(); });
    Fed2->enterExecutingMode();
    f1finish.wait();
    EXPECT_TRUE(Fed1->getCurrentMode() == helics::Federate::modes::executing);
    EXPECT_TRUE(Fed2->getCurrentMode() == helics::Federate::modes::executing);

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

TEST(federate_tests, multiple_federates_async_calls)
{
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreName = "core_async";
    fi.coreInitString = "-f 2 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);

    auto Fed2 = std::make_shared<helics::Federate>("fed2", fi);

    EXPECT_TRUE(Fed1->getCurrentMode() == helics::Federate::modes::startup);
    EXPECT_TRUE(Fed2->getCurrentMode() == helics::Federate::modes::startup);

    EXPECT_NE(Fed1->getID(), Fed2->getID());

    Fed1->enterInitializingModeAsync();
    Fed2->enterInitializingMode();
    EXPECT_NO_THROW(Fed2->enterInitializingMode());

    auto c1 = Fed1->getCorePointer();
    auto c2 = Fed2->getCorePointer();
    EXPECT_EQ(c1->getIdentifier(), c2->getIdentifier());
    c1.reset();
    c2.reset();

    Fed1->enterInitializingModeComplete();

    EXPECT_TRUE(Fed1->getCurrentMode() == helics::Federate::modes::initializing);
    EXPECT_TRUE(Fed2->getCurrentMode() == helics::Federate::modes::initializing);

    Fed1->enterExecutingModeAsync();
    Fed2->enterExecutingMode();
    Fed1->enterExecutingModeComplete();
    EXPECT_TRUE(Fed1->getCurrentMode() == helics::Federate::modes::executing);
    EXPECT_TRUE(Fed2->getCurrentMode() == helics::Federate::modes::executing);

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

TEST(federate_tests, missing_core)
{
    helics::FederateInfo fi(helics::core_type::NULLCORE);
    fi.coreName = "core_missing";
    fi.coreInitString = "-f 1";

    EXPECT_THROW(auto Fed1 = std::make_shared<helics::Federate>("fed1", fi),
                 helics::HelicsException);
}

TEST(federate_tests, not_open)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_full";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);
    Fed1->enterExecutingMode();

    EXPECT_THROW(auto fed2 = std::make_shared<helics::Federate>("fed2", fi),
                 helics::RegistrationFailure);
    Fed1->finalize();
}

TEST(federate_tests, coreApp)
{
    helics::CoreApp capp(helics::core_type::TEST, "corename", "-f 1 --autobroker");
    helics::FederateInfo fi(helics::core_type::TEST);
    auto Fed1 = std::make_shared<helics::Federate>("fed1", capp, fi);
    EXPECT_NO_THROW(Fed1->enterExecutingMode());

    Fed1->finalize();
}

TEST(federate_tests, core_ptr)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_ptr";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", nullptr, fi);
    Fed1->enterExecutingMode();

    EXPECT_THROW(auto fed2 = std::make_shared<helics::Federate>("fed2", nullptr, fi),
                 helics::RegistrationFailure);
    Fed1->finalize();
}

TEST(federate_tests, core_ptr_no_name)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>(std::string{}, nullptr, fi);
    Fed1->enterExecutingMode();
    Fed1->finalize();
}

TEST(federate_tests, from_string)
{
    auto Fed1 = std::make_shared<helics::Federate>(
        "fed1", "--type=TEST --corename core_init --coreinitstring='-f 1 --autobroker'");
    Fed1->enterExecutingMode();

    auto c1 = Fed1->getCorePointer();
    EXPECT_EQ(c1->getIdentifier(), "core_init");
    Fed1->finalize();
    c1.reset();
}

TEST(federate_tests, from_file1)
{
    auto fstr1 = std::string(TEST_DIR) + "example_filters.json";
    auto Fed1 = std::make_shared<helics::Federate>(fstr1);
    EXPECT_NO_THROW(Fed1->enterExecutingMode());
    Fed1->finalize();
}

TEST(federate_tests, from_file3)
{
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
    auto fstr1 = std::string(TEST_DIR) + "example_unusual_filters.json";
    auto Fed1 = std::make_shared<helics::Federate>(fstr1);
    EXPECT_THROW(Fed1->enterExecutingMode(), helics::HelicsException);
    Fed1->finalize();
    Fed1->getCorePointer()->disconnect();
}

TEST(federate_tests, from_file4)
{
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
    auto fstr1 = std::string(TEST_DIR) + "example_unusual_filters2.json";
    auto Fed1 = std::make_shared<helics::Federate>(fstr1);
    EXPECT_NO_THROW(Fed1->enterExecutingMode());
    Fed1->finalize();
}

TEST(federate_tests, from_file2)
{
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
    auto fstr2 = std::string(TEST_DIR) + "example_filters.toml";
    auto Fed1 = std::make_shared<helics::Federate>(fstr2);
    EXPECT_NO_THROW(Fed1->enterExecutingMode());
    Fed1->finalize();
}

TEST(federate_tests, from_file_invalid)
{
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
    auto fstr2 = std::string(TEST_DIR) + "invalid_filter_operation.json";
    std::shared_ptr<helics::Federate> Fed1;
    EXPECT_THROW(Fed1 = std::make_shared<helics::Federate>(fstr2), std::exception);
}

TEST(federate_tests, from_file5)
{
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
    auto fstr2 = "non_existing.toml";
    EXPECT_THROW(auto fed = std::make_shared<helics::Federate>(fstr2), helics::InvalidParameter);
}

TEST(federate_tests, from_file6)
{
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();

    auto fstr2 = std::string(TEST_DIR) + "example_filters.toml";
    auto Fed1 = std::make_shared<helics::Federate>(fstr2);
    EXPECT_THROW(Fed1->registerFilterInterfaces("non_existing.toml"), helics::InvalidParameter);
    EXPECT_THROW(Fed1->registerFilterInterfaces("non_existing.json"), helics::InvalidParameter);
}

TEST(federate_tests, from_file7)
{
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
    auto fstr3 = std::string(TEST_DIR) + "example_unusual_filters3.json";
    auto Fed1 = std::make_shared<helics::Federate>(fstr3);
    EXPECT_NO_THROW(Fed1->enterExecutingMode());
    Fed1->finalize();
}

TEST(federate_tests, from_file8)
{
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
    auto fstr3 = std::string(TEST_DIR) + "unusual_filters.toml";
    auto Fed1 = std::make_shared<helics::Federate>(fstr3);
    EXPECT_NO_THROW(Fed1->enterExecutingMode());
    Fed1->finalize();
}

TEST(federate_tests, from_file9)
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

TEST(federate_tests, from_string2)
{
    auto Fed1 = std::make_shared<helics::Federate>(
        "--name=fed1 --type=TEST --corename core_init --coreinitstring='-f 1 --autobroker'");
    Fed1->enterExecutingMode();

    EXPECT_EQ(Fed1->getName(), "fed1");
    Fed1->finalize();
}

TEST(federate_tests, enterInit)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_full_a";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);
    Fed1->enterInitializingModeAsync();
    // make sure it doesn't error if called twice
    EXPECT_NO_THROW(Fed1->enterInitializingModeAsync());
    EXPECT_NO_THROW(Fed1->isAsyncOperationCompleted());
    EXPECT_NO_THROW(Fed1->enterInitializingMode());
    EXPECT_NO_THROW(Fed1->enterInitializingModeComplete());

    EXPECT_EQ(Fed1->getCurrentMode(), helics::Federate::modes::initializing);
    EXPECT_NO_THROW(Fed1->finalize());
}

TEST(federate_tests, enterInitComplete)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_full_b";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);
    // this should be the same as just calling enterInitializingMode
    EXPECT_NO_THROW(Fed1->enterInitializingModeComplete());

    EXPECT_EQ(Fed1->getCurrentMode(), helics::Federate::modes::initializing);
    Fed1->finalize();
}

TEST(federate_tests, enterExec)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_full_c";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);
    Fed1->enterInitializingModeAsync();
    Fed1->setProperty(helics_properties::helics_property_time_delta, helics::Time(1.0));
    // make sure it doesn't error if called twice
    EXPECT_NO_THROW(Fed1->enterInitializingModeAsync());
    EXPECT_NO_THROW(Fed1->enterExecutingModeAsync());
    EXPECT_NO_THROW(Fed1->isAsyncOperationCompleted());
    EXPECT_NO_THROW(Fed1->enterExecutingMode());
    EXPECT_NO_THROW(Fed1->enterExecutingModeComplete());
    EXPECT_NO_THROW(Fed1->enterExecutingModeAsync());
    EXPECT_EQ(Fed1->getCurrentMode(), helics::Federate::modes::executing);
    Fed1->finalizeComplete();
}

TEST(federate_tests, enterExecAfterFinal)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_full_ec";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);
    Fed1->enterInitializingMode();
    auto cr = Fed1->getCorePointer();
    cr->disconnect();

    auto iterating = Fed1->enterExecutingMode();
    EXPECT_EQ(iterating, helics::iteration_result::halted);

    Fed1->finalize();
}

TEST(federate_tests, enterExecAfterFinalAsync)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_full_eca";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);
    Fed1->enterInitializingMode();
    auto cr = Fed1->getCorePointer();
    cr->disconnect();

    Fed1->enterExecutingModeAsync();
    auto iterating = Fed1->enterExecutingModeComplete();
    EXPECT_EQ(iterating, helics::iteration_result::halted);

    Fed1->finalize();
}

TEST(federate_tests, iterativeTimeRequestHalt)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_full_eca1";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);
    Fed1->enterExecutingMode();

    auto cr = Fed1->getCorePointer();
    cr->disconnect();

    auto itTime = Fed1->requestTimeIterative(2.0, helics::iteration_request::force_iteration);
    EXPECT_EQ(itTime.state, helics::iteration_result::halted);
    EXPECT_EQ(itTime.grantedTime, helics::Time::maxVal());

    Fed1->finalize();
}

TEST(federate_tests, iterativeTimeRequestAsyncHalt)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_full_eca2";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);
    Fed1->enterExecutingMode();

    auto cr = Fed1->getCorePointer();
    cr->disconnect();

    Fed1->requestTimeIterativeAsync(2.0, helics::iteration_request::force_iteration);
    auto itTime = Fed1->requestTimeIterativeComplete();
    EXPECT_EQ(itTime.state, helics::iteration_result::halted);
    EXPECT_EQ(itTime.grantedTime, helics::Time::maxVal());

    Fed1->finalize();
}

TEST(federate_tests, enterExecAsync)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_full_d";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);

    Fed1->enterExecutingModeAsync();
    EXPECT_NO_THROW(Fed1->enterExecutingModeAsync());
    EXPECT_NO_THROW(Fed1->finalize());
}

TEST(federate_tests, enterExecAsyncIterative)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_full_e";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);

    Fed1->enterExecutingModeAsync(helics::iteration_request::force_iteration);
    EXPECT_NO_THROW(Fed1->enterExecutingModeAsync());
    auto res = Fed1->enterExecutingModeComplete();
    EXPECT_EQ(res, helics::iteration_result::iterating);
    EXPECT_EQ(Fed1->getCurrentMode(), helics::Federate::modes::initializing);
    EXPECT_NO_THROW(Fed1->finalizeAsync());
    EXPECT_NO_THROW(Fed1->finalizeAsync());
    EXPECT_NO_THROW(Fed1->finalize());
}

TEST(federate_tests, enterRequestTimeAsyncIterative)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_full_e";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);

    Fed1->enterExecutingMode();
    Fed1->requestTimeAsync(1.0);
    while (!Fed1->isAsyncOperationCompleted()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    EXPECT_TRUE(Fed1->isAsyncOperationCompleted());
    EXPECT_NO_THROW(Fed1->enterExecutingMode());
    EXPECT_EQ(Fed1->getCurrentTime(), 1.0);
    EXPECT_EQ(Fed1->getCurrentMode(), helics::Federate::modes::executing);

    Fed1->requestTimeIterativeAsync(37.0, helics::iteration_request::force_iteration);
    while (!Fed1->isAsyncOperationCompleted()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    auto itime = Fed1->requestTimeIterativeComplete();
    EXPECT_EQ(itime.grantedTime, 1.0);
    EXPECT_EQ(itime.state, helics::iteration_result::iterating);
    Fed1->requestTimeIterativeAsync(1.0, helics::iteration_request::force_iteration);
    Fed1->finalizeAsync();
    while (!Fed1->isAsyncOperationCompleted()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    EXPECT_TRUE(Fed1->isAsyncOperationCompleted());
    Fed1->finalizeComplete();
}

TEST(federate_tests, enterRequestTimeAsyncIterativeFinalize)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_full_e2";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);

    Fed1->enterExecutingMode();
    Fed1->requestTimeIterativeAsync(1.0, helics::iteration_request::force_iteration);
    EXPECT_NO_THROW(Fed1->finalize());
    // check time results after finalize
    auto tm = Fed1->requestTime(3.0);
    EXPECT_EQ(tm, helics::Time::maxVal());
}

TEST(federate_tests, enterRequestTimeAsyncFinalize)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_full_e3";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);

    Fed1->enterExecutingMode();
    Fed1->requestTimeAsync(1.0);
    EXPECT_NO_THROW(Fed1->finalize());
    EXPECT_EQ(Fed1->getCurrentMode(), helics::Federate::modes::finalize);
}

TEST(federate_tests, enterRequestTimeAsyncFinalizeAsync)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_full_e3a";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);

    Fed1->enterExecutingMode();
    Fed1->requestTimeAsync(1.0);
    EXPECT_NO_THROW(Fed1->finalizeAsync());
    Fed1->finalizeComplete();
    EXPECT_EQ(Fed1->getCurrentMode(), helics::Federate::modes::finalize);
}

TEST(federate_tests, enterEnterExecAsyncFinalize)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_full_e4";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);
    Fed1->enterInitializingMode();
    Fed1->enterExecutingModeAsync();
    EXPECT_NO_THROW(Fed1->finalize());
    EXPECT_EQ(Fed1->getCurrentMode(), helics::Federate::modes::finalize);
}

TEST(federate_tests, enterEnterInitAsyncFinalize)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_full_e5";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);
    Fed1->enterInitializingModeAsync();
    EXPECT_NO_THROW(Fed1->finalize());
    EXPECT_EQ(Fed1->getCurrentMode(), helics::Federate::modes::finalize);
}

TEST(federate_tests, enterEnterExecAsyncFinalizeAsync)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_full_e4a";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);
    Fed1->enterInitializingMode();
    Fed1->enterExecutingModeAsync();
    EXPECT_NO_THROW(Fed1->finalizeAsync());
    Fed1->finalizeComplete();
    EXPECT_EQ(Fed1->getCurrentMode(), helics::Federate::modes::finalize);
}

TEST(federate_tests, enterEnterInitAsyncFinalizeAsync)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_full_e5a";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);
    Fed1->enterInitializingModeAsync();
    EXPECT_NO_THROW(Fed1->finalizeAsync());
    Fed1->finalizeComplete();
    EXPECT_EQ(Fed1->getCurrentMode(), helics::Federate::modes::finalize);
}

TEST(federate_tests, enterExecPendingTimeIterative)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_full_epa";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);
    Fed1->enterExecutingMode();
    Fed1->requestTimeIterativeAsync(2.0, helics::iteration_request::force_iteration);
    auto it = Fed1->enterExecutingMode();
    EXPECT_EQ(it, helics::iteration_result::next_step);
    Fed1->finalizeComplete();
    EXPECT_EQ(Fed1->getCurrentMode(), helics::Federate::modes::finalize);
}

TEST(federate_tests, forceErrorExec)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_full_fe";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);

    Fed1->enterExecutingMode();
    Fed1->localError(9827);

    EXPECT_THROW(Fed1->enterInitializingMode(), helics::InvalidFunctionCall);
    EXPECT_THROW(Fed1->enterExecutingMode(), helics::InvalidFunctionCall);

    Fed1->getCorePointer()->disconnect();
}

TEST(federate_tests, forceErrorExecAsync)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_full_fe1";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);

    Fed1->enterExecutingModeAsync(helics::iteration_request::force_iteration);
    Fed1->localError(9827);

    EXPECT_THROW(Fed1->enterInitializingMode(), helics::InvalidFunctionCall);
    EXPECT_THROW(Fed1->enterExecutingMode(), helics::InvalidFunctionCall);

    Fed1->getCorePointer()->disconnect();
}

TEST(federate_tests, forceErrorInitAsync)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_full_fe2";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);

    Fed1->enterInitializingModeAsync();
    Fed1->localError(9827);

    EXPECT_THROW(Fed1->enterInitializingMode(), helics::InvalidFunctionCall);
    EXPECT_THROW(Fed1->enterExecutingMode(), helics::InvalidFunctionCall);

    Fed1->getCorePointer()->disconnect();
}

TEST(federate_tests, forceErrorPendingTimeAsync)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_full_fe3";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);

    Fed1->enterExecutingMode();
    Fed1->requestTimeAsync(2.0);
    Fed1->globalError(9827);

    EXPECT_THROW(Fed1->requestTime(3.0), helics::InvalidFunctionCall);

    Fed1->getCorePointer()->disconnect();
}

TEST(federate_tests, federateGeneratedLocalError)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_full_le";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);

    Fed1->enterExecutingMode();
    Fed1->localError(9827, "user generated error");

    EXPECT_THROW(Fed1->requestTime(3.0), helics::HelicsException);

    Fed1->getCorePointer()->disconnect();
    Fed1->disconnect();
    EXPECT_THROW(Fed1->localError(9827, "user generated error2"), helics::InvalidFunctionCall);
}

TEST(federate_tests, federateGeneratedGlobalError)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_full_ge";
    fi.coreInitString = "-f 1 --autobroker --error_timeout=0";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);

    Fed1->enterExecutingMode();
    Fed1->requestTimeAsync(2.0);
    Fed1->globalError(9827, "user generated global error");

    EXPECT_THROW(Fed1->requestTime(3.0), helics::HelicsException);

    EXPECT_TRUE(Fed1->getCorePointer()->waitForDisconnect(std::chrono::milliseconds(300)));

    Fed1->disconnect();
    EXPECT_THROW(Fed1->globalError(9827, "user generated global error2"),
                 helics::InvalidFunctionCall);
}

TEST(federate_tests, federateGeneratedlocalErrorEscalation)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_full_ge";
    fi.coreInitString = "-f 1 --autobroker --error_timeout=0";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);
    Fed1->setFlagOption(helics::defs::flags::terminate_on_error);
    Fed1->enterExecutingMode();
    Fed1->requestTimeAsync(2.0);
    Fed1->localError(9827, "user generated global error");

    EXPECT_THROW(Fed1->requestTime(3.0), helics::HelicsException);

    EXPECT_TRUE(Fed1->getCorePointer()->waitForDisconnect(std::chrono::milliseconds(300)));

    Fed1->disconnect();
    EXPECT_THROW(Fed1->globalError(9827, "user generated global error2"),
                 helics::InvalidFunctionCall);
}

TEST(federate_tests, queryTest1)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_q";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed_q", fi);

    Fed1->enterExecutingMode();

    auto qres = Fed1->query("name");
    EXPECT_EQ(qres, "fed_q");
    qres = Fed1->query("corename");
    auto cr = Fed1->getCorePointer();
    EXPECT_EQ(qres, cr->getIdentifier());
    qres = Fed1->query("federate", "name");
    EXPECT_EQ(qres, "fed_q");

    cr.reset();
    Fed1->disconnect();
    qres = Fed1->query("corename");
    EXPECT_EQ(qres.front(), '#');
    qres = Fed1->query("subscriptions");
    EXPECT_EQ(qres.front(), '#');
    qres = Fed1->query("root", "subscriptions");
    EXPECT_EQ(qres.front(), '#');

    qres = Fed1->queryComplete(helics::query_id_t{4});
    EXPECT_EQ(qres.front(), '#');

    EXPECT_FALSE(Fed1->isQueryCompleted(helics::query_id_t{2}));
    EXPECT_NO_THROW(Fed1->logMessage(10, "test log message"));
}

TEST(federate_tests, forceErrorPendingTimeIterativeAsync)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_full_fe4";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);

    Fed1->enterExecutingMode();
    Fed1->requestTimeIterativeAsync(2.0, helics::iteration_request::no_iterations);
    Fed1->localError(9827);

    EXPECT_THROW(Fed1->requestTime(3.0), helics::InvalidFunctionCall);

    Fed1->getCorePointer()->disconnect();
}

TEST(federate_tests, forceErrorFinalizeAsync)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_full_fe5";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);

    Fed1->enterExecutingMode();
    Fed1->finalizeAsync();
    Fed1->localError(9827);

    EXPECT_THROW(Fed1->requestTime(3.0), helics::InvalidFunctionCall);

    Fed1->getCorePointer()->disconnect();
}

TEST(federate_tests, error_after_disconnect)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_full_g";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);
    auto& f1 = Fed1->registerGlobalFilter("filt1", "type1", "type2");
    Fed1->enterExecutingMode();
    Fed1->disconnect();

    const auto& Fedref = *Fed1;
    auto& fb = Fedref.getFilter(0);
    auto& fb2 = Fedref.getFilter("filt1");
    auto& fb3 = Fedref.getFilter("notafilter");
    auto& fb4 = Fed1->getFilter("filt1");
    EXPECT_EQ(Fed1->getInterfaceName(fb), Fed1->getInterfaceName(f1));
    EXPECT_EQ(Fed1->getInterfaceName(fb2), Fed1->getInterfaceName(f1));
    EXPECT_EQ(Fed1->getInterfaceName(fb4), Fed1->getInterfaceName(f1));
    EXPECT_FALSE(fb3.isValid());

    EXPECT_THROW(Fed1->setGlobal("global1", "global1"), helics::InvalidFunctionCall);
    EXPECT_THROW(Fed1->addSourceTarget(f1, "ept"), helics::InvalidFunctionCall);
    EXPECT_THROW(Fed1->addDependency("otherFed"), helics::InvalidFunctionCall);

    EXPECT_THROW(Fed1->addDestinationTarget(f1, "ept"), helics::InvalidFunctionCall);
    EXPECT_THROW(Fed1->setFilterOperator(f1, {}), helics::InvalidFunctionCall);

    EXPECT_THROW(Fed1->setInterfaceOption(helics::interface_handle{0}, 0, false),
                 helics::InvalidFunctionCall);
    EXPECT_THROW(Fed1->setInfo(helics::interface_handle{0}, "information"),
                 helics::InvalidFunctionCall);
    EXPECT_THROW(Fed1->localError(99), helics::InvalidFunctionCall);
    EXPECT_THROW(Fed1->globalError(99), helics::InvalidFunctionCall);
}

static constexpr const char* simple_global_files[] = {"example_globals1.json",
                                                      "example_globals1.toml",
                                                      "example_globals2.json"};

class federate_global_files: public ::testing::TestWithParam<const char*> {
};

TEST_P(federate_global_files, global_file_ci_skip)
{
    auto brk = helics::BrokerFactory::create(helics::core_type::TEST, "b1", "-f 2");
    brk->connect();
    auto testFile = std::string(TEST_DIR) + GetParam();
    brk->makeConnections(testFile);

    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreName = "core_global1";
    fi.coreInitString = "-f 2";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);

    auto Fed2 = std::make_shared<helics::Federate>("fed2", fi);

    Fed1->enterInitializingModeAsync();
    Fed2->enterInitializingMode();

    Fed1->enterInitializingModeComplete();

    auto str1 = Fed1->query("global", "global1");
    EXPECT_EQ(str1, "this is a global1 value");
    str1 = Fed2->query("global", "global1");
    EXPECT_EQ(str1, "this is a global1 value");

    str1 = Fed1->query("global", "global2");
    EXPECT_EQ(str1, "this is another global value");
    str1 = Fed2->query("global", "global2");
    EXPECT_EQ(str1, "this is another global value");
    Fed1->finalize();
    Fed2->finalize();
    brk->waitForDisconnect();
}

TEST_P(federate_global_files, core_global_file_ci_skip)
{
    auto brk = helics::BrokerFactory::create(helics::core_type::TEST, "b1", "-f2");
    brk->connect();

    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreName = "core_global3";
    fi.coreInitString = "-f 1";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);
    fi.coreName = "core_global4";
    auto Fed2 = std::make_shared<helics::Federate>("fed2", fi);

    auto cr = Fed1->getCorePointer();
    auto testFile = std::string(TEST_DIR) + GetParam();
    cr->makeConnections(testFile);
    Fed1->enterInitializingModeAsync();
    Fed2->enterInitializingMode();

    Fed1->enterInitializingModeComplete();

    auto str1 = Fed1->query("global", "global1");
    EXPECT_EQ(str1, "this is a global1 value");
    str1 = Fed2->query("global", "global1");
    EXPECT_EQ(str1, "this is a global1 value");
    str1 = cr->query("global", "global1", helics_sequencing_mode_fast);
    EXPECT_EQ(str1, "this is a global1 value");
    str1 = brk->query("global", "global1");
    EXPECT_EQ(str1, "this is a global1 value");

    str1 = Fed1->query("global", "global2");
    EXPECT_EQ(str1, "this is another global value");
    str1 = Fed2->query("global", "global2");
    EXPECT_EQ(str1, "this is another global value");
    str1 = cr->query("global", "global2", helics_sequencing_mode_fast);
    EXPECT_EQ(str1, "this is another global value");
    str1 = brk->query("global", "global2");
    EXPECT_EQ(str1, "this is another global value");

    auto str2 = Fed1->query("global", "list");
    EXPECT_TRUE((str2 == "[global1;global2]") || (str2 == "[global2;global1]"));

    auto str3 = Fed1->query("global", "all");
    EXPECT_NE(str3, "#invalid");
    Fed1->finalize();
    Fed2->finalize();
    cr = nullptr;
    brk->waitForDisconnect();
}

INSTANTIATE_TEST_SUITE_P(federate_tests,
                         federate_global_files,
                         ::testing::ValuesIn(simple_global_files));
