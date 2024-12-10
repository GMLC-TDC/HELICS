/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "gtest/gtest.h"
#include <cstdio>
#ifndef DISABLE_SYSTEM_CALL_TESTS
#    include "exeTestHelper.h"
#endif

#include "helics/apps/BrokerApp.hpp"
#include "helics/apps/Connector.hpp"
#include "helics/apps/CoreApp.hpp"

#include <fmt/format.h>
#include <future>
#include <string>
#include <thread>

static constexpr std::string_view testdir = TEST_DIR "/connector/";

TEST(connector_file_tests, simple_connector)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccoref1";
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.loadFile(std::string(testdir) + "simple.txt");

    helics::ValueFederate vfed("c1", fedInfo);
    auto& pub1 = vfed.registerGlobalPublication<double>("pub1");
    auto& inp1 = vfed.registerGlobalInput<double>("inp1");

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed.enterExecutingMode();
    const double testValue = 3452.562;
    pub1.publish(testValue);
    auto retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    auto val = inp1.getDouble();
    EXPECT_EQ(val, testValue);

    vfed.finalize();
    fut.get();
    EXPECT_EQ(conn1.madeConnections(), 1);
}

/*
TEST(connector_file_tests, simple_connector_cmd)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccorefile1";
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector2", fmt::format("--corename={} --coretype=TEST
--coreinitstring='{}' --period=1.0
--input={}simple.txt",fedInfo.coreName,fedInfo.coreInitString,testdir));

    helics::ValueFederate vfed("c1", fedInfo);
    auto& pub1 = vfed.registerGlobalPublication<double>("pub1");
    auto& inp1 = vfed.registerGlobalInput<double>("inp1");

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed.enterExecutingMode();
    const double testValue = 3452.562;
    pub1.publish(testValue);
    auto retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    auto val = inp1.getDouble();
    EXPECT_EQ(val, testValue);

    vfed.finalize();
    fut.get();
    EXPECT_EQ(conn1.madeConnections(), 1);
}

*/

TEST(connector_file_tests, simple_connector_quotes)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccoref1q";
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.loadFile(std::string(testdir) + "simple-quotes.txt");

    helics::ValueFederate vfed("c1", fedInfo);
    auto& pub1 = vfed.registerGlobalPublication<double>("pub1");
    auto& inp1 = vfed.registerGlobalInput<double>("inp1");

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed.enterExecutingMode();
    const double testValue = 3452.562;
    pub1.publish(testValue);
    auto retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    auto val = inp1.getDouble();
    EXPECT_EQ(val, testValue);

    vfed.finalize();
    fut.get();
    EXPECT_EQ(conn1.madeConnections(), 1);
}

TEST(connector_file_tests, simple_connector_reverse)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccoref2";
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.loadFile(std::string(testdir) + "simple.json");

    helics::ValueFederate vfed("c1", fedInfo);
    auto& pub1 = vfed.registerGlobalPublication<double>("pub1");
    auto& inp1 = vfed.registerGlobalInput<double>("inp1");

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed.enterExecutingMode();
    const double testValue = 3452.562;
    pub1.publish(testValue);
    auto retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    auto val = inp1.getDouble();
    EXPECT_EQ(val, testValue);

    vfed.finalize();
    fut.get();
}

TEST(connector_file_tests, connector_cascade)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccoref3";
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.loadFile(std::string(testdir) + "cascade.txt");

    helics::ValueFederate vfed("c1", fedInfo);
    auto& pub1 = vfed.registerGlobalPublication<double>("pub1");
    auto& inp1 = vfed.registerGlobalInput<double>("inp1");
    auto& inp2 = vfed.registerGlobalInput<double>("inp2");

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed.enterExecutingMode();
    const double testValue = 3452.562;
    pub1.publish(testValue);
    auto retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    auto val = inp1.getDouble();
    EXPECT_EQ(val, testValue);
    val = inp2.getDouble();
    EXPECT_EQ(val, testValue);
    vfed.finalize();
    fut.get();
}

TEST(connector_file_tests, simple_connector_tags)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccoref4";
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.loadFile(std::string(testdir) + "simple_tags.txt");

    helics::ValueFederate vfed("c1", fedInfo);
    auto& pub1 = vfed.registerGlobalPublication<double>("pub1");
    auto& inp1 = vfed.registerGlobalInput<double>("inp1");
    auto& inp2 = vfed.registerGlobalInput<double>("inp2");
    vfed.setGlobal("tag1", "true");
    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed.enterExecutingMode();
    const double testValue = 3452.562;
    pub1.publish(testValue);
    auto retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    auto val = inp1.getDouble();
    EXPECT_EQ(val, testValue);
    val = inp2.getDouble();
    EXPECT_EQ(val, helics::invalidDouble);
    vfed.finalize();
    fut.get();
    EXPECT_EQ(conn1.madeConnections(), 1);
}

TEST(connector_file_tests, simple_connector_tags_alt)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccoref5";
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.loadFile(std::string(testdir) + "simple_tags.txt");

    helics::ValueFederate vfed("c1", fedInfo);
    auto& pub1 = vfed.registerGlobalPublication<double>("pub1");
    auto& inp1 = vfed.registerGlobalInput<double>("inp1");
    auto& inp2 = vfed.registerGlobalInput<double>("inp2");
    vfed.setGlobal("tag2", "true");
    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed.enterExecutingMode();
    const double testValue = 3452.562;
    pub1.publish(testValue);
    auto retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    auto val = inp1.getDouble();
    EXPECT_EQ(val, helics::invalidDouble);

    val = inp2.getDouble();
    EXPECT_EQ(val, testValue);
    vfed.finalize();
    fut.get();
    EXPECT_GE(conn1.madeConnections(), 1);
}
