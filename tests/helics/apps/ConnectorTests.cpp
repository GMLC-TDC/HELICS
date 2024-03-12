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

#include <future>
#include <thread>

TEST(connector_tests, simple_connector)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore1";
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("inp1", "pub1", InterfaceDirection::FROM_TO);

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

TEST(connector_tests, simple_connector_reverse)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore2";
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("pub1", "inp1", InterfaceDirection::FROM_TO);

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

TEST(connector_tests, connector_multiple)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore3";
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("inp1", "pub1", InterfaceDirection::FROM_TO);
    conn1.addConnection("inp1", "pubA", InterfaceDirection::FROM_TO);
    conn1.addConnection("inp2", "pub1", InterfaceDirection::FROM_TO);
    conn1.addConnection("inp2", "pubA", InterfaceDirection::FROM_TO);

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

TEST(connector_tests, connector_cascade)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore4";
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("inp1", "intermediate1", InterfaceDirection::FROM_TO);
    conn1.addConnection("intermediate1", "intermediate1", InterfaceDirection::BIDIRECTIONAL);
    conn1.addConnection("intermediate1", "intermediate2", InterfaceDirection::FROM_TO);
    conn1.addConnection("intermediate2", "intermediate3", InterfaceDirection::FROM_TO);
    conn1.addConnection("intermediate3", "pub1", InterfaceDirection::BIDIRECTIONAL);
    conn1.addConnection("inp2", "intermediate2", InterfaceDirection::FROM_TO);

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

TEST(connector_tests, endpoint_connector)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore5";
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("ept1", "ept12", InterfaceDirection::FROM_TO);

    helics::MessageFederate vfed("c1", fedInfo);
    auto& ept1 = vfed.registerGlobalTargetedEndpoint("ept1");
    auto& ept12 = vfed.registerGlobalTargetedEndpoint("ept12");

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed.enterExecutingMode();
    constexpr std::string_view testValue = "this is a test string";
    ept1.send(testValue);
    auto retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    EXPECT_TRUE(ept12.hasMessage());
    auto message = ept12.getMessage();
    EXPECT_EQ(message->to_string(), testValue);
    vfed.finalize();
    fut.get();
}

TEST(connector_tests, endpoint_connector_no_connection)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore6";
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("ept1", "ept12", InterfaceDirection::FROM_TO);

    helics::MessageFederate vfed("c1", fedInfo);
    auto& ept1 = vfed.registerGlobalTargetedEndpoint("ept1");
    auto& ept12 = vfed.registerGlobalTargetedEndpoint("ept12");

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed.enterExecutingMode();
    constexpr std::string_view testValue = "this is a test string";
    ept12.send(testValue);
    auto retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 5.0);
    EXPECT_FALSE(ept1.hasMessage());

    vfed.finalize();
    fut.get();
}

TEST(connector_tests, endpoint_connector_bidirectional)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore7";
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("ept1", "ept12", InterfaceDirection::BIDIRECTIONAL);

    helics::MessageFederate vfed("c1", fedInfo);
    auto& ept1 = vfed.registerGlobalTargetedEndpoint("ept1");
    auto& ept12 = vfed.registerGlobalTargetedEndpoint("ept12");

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed.enterExecutingMode();
    constexpr std::string_view testValue1 = "this is a test string1";
    constexpr std::string_view testValue2 = "this is a test string2";
    ept1.send(testValue1);
    ept12.send(testValue2);
    auto retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    EXPECT_TRUE(ept1.hasMessage());
    EXPECT_TRUE(ept12.hasMessage());
    auto message = ept12.getMessage();
    EXPECT_EQ(message->to_string(), testValue1);

    message = ept1.getMessage();
    EXPECT_EQ(message->to_string(), testValue2);
    vfed.finalize();
    fut.get();
}

TEST(connector_tests, simple_connector_alias)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore8";
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("inp1", "publication1", InterfaceDirection::FROM_TO);

    helics::ValueFederate vfed("c1", fedInfo);
    auto& pub1 = vfed.registerGlobalPublication<double>("pub1");
    pub1.addAlias("publication1");
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

TEST(connector_tests, simple_connector_alias_reverse)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore9";
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("pub1", "input1", InterfaceDirection::FROM_TO);

    helics::ValueFederate vfed("c1", fedInfo);
    auto& pub1 = vfed.registerGlobalPublication<double>("pub1");

    auto& inp1 = vfed.registerGlobalInput<double>("inp1");
    inp1.addAlias("input1");
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

TEST(connector_tests, dual_alias)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore10";
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("input1", "publication1", InterfaceDirection::FROM_TO);

    helics::ValueFederate vfed("c1", fedInfo);
    auto& pub1 = vfed.registerGlobalPublication<double>("pub1");
    pub1.addAlias("publication1");
    auto& inp1 = vfed.registerGlobalInput<double>("inp1");
    inp1.addAlias("input1");

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

TEST(connector_tests, dual_alias_reverse)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore11";
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("publication1", "input1", InterfaceDirection::FROM_TO);

    helics::ValueFederate vfed("c1", fedInfo);
    auto& pub1 = vfed.registerGlobalPublication<double>("pub1");
    pub1.addAlias("publication1");
    auto& inp1 = vfed.registerGlobalInput<double>("inp1");
    inp1.addAlias("input1");
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

TEST(connector_tests, cascade_dual_alias)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore12";
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("inputA", "publicationA", InterfaceDirection::FROM_TO);

    helics::ValueFederate vfed("c1", fedInfo);
    auto& pub1 = vfed.registerGlobalPublication<double>("pub1");
    pub1.addAlias("publication1");
    helics::CoreApp core(vfed.getCorePointer());
    core.addAlias("publication1", "pubsbb");
    core.addAlias("pubsbb", "publicationA");
    auto& inp1 = vfed.registerGlobalInput<double>("inp1");
    inp1.addAlias("input1");
    core.addAlias("input1", "inpsbb");
    core.addAlias("inpsbb", "inputA");

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

TEST(connector_tests, cascade_dual_alias_intermediary)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore13";
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("publicationA", "inputC", InterfaceDirection::FROM_TO);
    conn1.addConnection("inputC", "inputE", InterfaceDirection::FROM_TO);
    conn1.addConnection("inputE", "inputA", InterfaceDirection::FROM_TO);
    helics::ValueFederate vfed("c1", fedInfo);
    auto& pub1 = vfed.registerGlobalPublication<double>("pub1");
    pub1.addAlias("publication1");
    helics::CoreApp core(vfed.getCorePointer());
    core.addAlias("publication1", "pubsbb");
    core.addAlias("pubsbb", "publicationA");
    auto& inp1 = vfed.registerGlobalInput<double>("inp1");
    inp1.addAlias("input1");
    inp1.addAlias("input1");
    core.addAlias("input1", "inpsbb");
    core.addAlias("inpsbb", "inputA");
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

TEST(connector_tests, endpoint_connector_alias1)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore14";
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("eptA", "eptB", InterfaceDirection::FROM_TO);

    helics::MessageFederate vfed("c1", fedInfo);
    auto& ept1 = vfed.registerGlobalTargetedEndpoint("ept1");
    ept1.addAlias("eptA");
    auto& ept12 = vfed.registerGlobalTargetedEndpoint("ept12");
    ept12.addAlias("eptB");
    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed.enterExecutingMode();
    constexpr std::string_view testValue = "this is a test string";
    ept1.send(testValue);
    auto retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    EXPECT_TRUE(ept12.hasMessage());
    auto message = ept12.getMessage();
    EXPECT_EQ(message->to_string(), testValue);
    vfed.finalize();
    fut.get();
}
