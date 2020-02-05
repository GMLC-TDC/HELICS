/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/Federate.hpp"
#include "helics/core/BrokerFactory.hpp"
//#include "helics/core/CoreFactory.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/core-exceptions.hpp"

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
    Fed = nullptr; // force the destructor
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
    EXPECT_TRUE(Fed->getCurrentMode() == helics::Federate::modes::finalize);
}

#ifdef ENABLE_ZMQ_CORE
// TODO PT:: make this work for all test types
TEST(federate_tests, bad_broker_error_zmq_ci_skip)
{
    helics::FederateInfo fi(helics::core_type::ZMQ);
    fi.coreInitString = "--broker=b1 --tick=200 --timeout=800 --networktimeout=400";

    EXPECT_THROW(std::make_shared<helics::Federate>("test1", fi), helics::RegistrationFailure);
}

TEST(federate_tests, timeout_error_zmq_ci_skip)
{
    helics::FederateInfo fi(helics::core_type::ZMQ);
    fi.coreInitString = "--tick=200 --timeout=800 --networktimeout=400";

    EXPECT_THROW(std::make_shared<helics::Federate>("test1", fi), helics::RegistrationFailure);
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
    str1 = cr->query("global", "global1");
    EXPECT_EQ(str1, "this is a global1 value");
    str1 = brk->query("global", "global1");
    EXPECT_EQ(str1, "this is a global1 value");

    str1 = Fed1->query("global", "global2");
    EXPECT_EQ(str1, "this is another global value");
    str1 = Fed2->query("global", "global2");
    EXPECT_EQ(str1, "this is another global value");
    str1 = cr->query("global", "global2");
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

INSTANTIATE_TEST_SUITE_P(
    federate_tests,
    federate_global_files,
    ::testing::ValuesIn(simple_global_files));
