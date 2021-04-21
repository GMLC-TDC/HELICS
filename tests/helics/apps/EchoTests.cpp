/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "exeTestHelper.h"
#include "helics/application_api/Endpoints.hpp"
#include "helics/apps/Echo.hpp"
#include "helics/core/BrokerFactory.hpp"

#include "gtest/gtest.h"
#include <cstdio>
#include <future>

// this test will test basic echo functionality
TEST(echo_tests, echo_test1)
{
    helics::FederateInfo fi(helics::core_type::TEST);

    fi.coreName = "ecore1";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Echo echo1("echo1", fi);

    echo1.addEndpoint("test");
    // fi.logLevel = 4;
    helics::MessageFederate mfed("source", fi);
    helics::Endpoint ep1(&mfed, "src");
    auto fut = std::async(std::launch::async, [&echo1]() { echo1.runTo(5.0); });
    mfed.enterExecutingMode();
    ep1.send("test", "hello world");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto retTime = mfed.requestTime(1.0);
    EXPECT_TRUE(ep1.hasMessage());
    EXPECT_LT(retTime, 1.0);
    auto m = ep1.getMessage();
    ASSERT_TRUE(m);
    EXPECT_EQ(m->data.to_string(), "hello world");
    EXPECT_EQ(m->source, "test");
    mfed.finalize();
    fut.get();
}

TEST(echo_tests, echo_test_delay)
{
    helics::FederateInfo fi(helics::core_type::TEST);

    fi.coreName = "ecore2";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Echo echo1("echo1", fi);

    echo1.addEndpoint("test");
    echo1.setEchoDelay(1.2);
    helics::MessageFederate mfed("source", fi);
    helics::Endpoint ep1(&mfed, "src");
    auto fut = std::async(std::launch::async, [&echo1]() { echo1.runTo(5.0); });
    mfed.enterExecutingMode();
    ep1.send("test", "hello world");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    mfed.requestTime(1.0);
    EXPECT_FALSE(ep1.hasMessage());
    auto ntime = mfed.requestTime(2.0);
    EXPECT_EQ(ntime, helics::Time::epsilon() + 1.2);
    EXPECT_TRUE(ep1.hasMessage());
    auto m = ep1.getMessage();
    ASSERT_TRUE(m);
    EXPECT_EQ(m->data.to_string(), "hello world");
    EXPECT_EQ(m->source, "test");
    mfed.finalize();
    fut.get();
}

TEST(echo_tests, echo_test_delay_period)
{
    helics::FederateInfo fi(helics::core_type::TEST);

    fi.coreName = "ecore3";
    fi.coreInitString = "-f 2 --autobroker";
    fi.setProperty(helics_property_time_period, 1.1);
    helics::apps::Echo echo1("echo1", fi);
    fi.setProperty(helics_property_time_period, 0);

    echo1.addEndpoint("test");
    echo1.setEchoDelay(1.2);
    helics::MessageFederate mfed("source", fi);
    helics::Endpoint ep1(&mfed, "src");
    auto fut = std::async(std::launch::async, [&echo1]() { echo1.runTo(5.0); });
    mfed.enterExecutingMode();
    ep1.send("test", "hello world");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    mfed.requestTime(1.0);
    EXPECT_FALSE(ep1.hasMessage());
    auto ntime = mfed.requestTime(4.0);
    EXPECT_EQ(ntime, 2.3);
    EXPECT_TRUE(ep1.hasMessage());
    auto m = ep1.getMessage();
    ASSERT_TRUE(m);
    EXPECT_EQ(m->data.to_string(), "hello world");
    EXPECT_EQ(m->source, "test");
    mfed.finalize();
    fut.get();
}

TEST(echo_tests, echo_test_multiendpoint)
{
    helics::FederateInfo fi(helics::core_type::TEST);

    fi.coreName = "ecore4";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Echo echo1("echo1", fi);
    echo1.addEndpoint("test");
    echo1.addEndpoint("test2");
    echo1.setEchoDelay(1.2);
    helics::MessageFederate mfed("source", fi);
    helics::Endpoint ep1(&mfed, "src");
    auto fut = std::async(std::launch::async, [&echo1]() { echo1.runTo(5.0); });
    mfed.enterExecutingMode();
    ep1.send("test", "hello world");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    mfed.requestTime(1.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ep1.send("test2", "hello again");
    EXPECT_TRUE(!ep1.hasMessage());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto ntime = mfed.requestTime(2.0);
    EXPECT_EQ(ntime, helics::Time::epsilon() + 1.2);
    EXPECT_TRUE(ep1.hasMessage());
    auto m = ep1.getMessage();
    ASSERT_TRUE(m);
    EXPECT_EQ(m->data.to_string(), "hello world");
    EXPECT_EQ(m->source, "test");
    m = ep1.getMessage();
    EXPECT_FALSE(m);

    ntime = mfed.requestTime(3.0);
    EXPECT_EQ(ntime, 2.2);
    EXPECT_TRUE(ep1.hasMessage());
    m = ep1.getMessage();
    ASSERT_TRUE(m);
    EXPECT_EQ(m->data.to_string(), "hello again");
    EXPECT_EQ(m->source, "test2");
    mfed.finalize();
    fut.get();
}

TEST(echo_tests, echo_test_fileload)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "ecore4-file";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Echo echo1("echo1", fi);
    echo1.loadFile(std::string(TEST_DIR) + "/echo_example.json");

    helics::MessageFederate mfed("source", fi);
    helics::Endpoint ep1(&mfed, "src");
    auto fut = std::async(std::launch::async, [&echo1]() { echo1.runTo(5.0); });
    mfed.enterExecutingMode();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ep1.send("test", "hello world");
    mfed.requestTime(1.0);
    ep1.send("test2", "hello again");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(!ep1.hasMessage());
    auto ntime = mfed.requestTime(2.0);
    EXPECT_EQ(ntime, helics::timeEpsilon + 1.2);
    EXPECT_TRUE(ep1.hasMessage());
    auto m = ep1.getMessage();
    ASSERT_TRUE(m);
    EXPECT_EQ(m->data.to_string(), "hello world");
    EXPECT_EQ(m->source, "test");

    ntime = mfed.requestTime(3.0);
    EXPECT_EQ(ntime, 2.2);
    EXPECT_TRUE(ep1.hasMessage());
    m = ep1.getMessage();
    ASSERT_TRUE(m);
    EXPECT_EQ(m->data.to_string(), "hello again");
    EXPECT_EQ(m->source, "test2");
    mfed.finalize();
    fut.get();
}
