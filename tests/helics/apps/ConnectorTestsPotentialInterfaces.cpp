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
#include <string>
#include <thread>

static constexpr std::string_view testdir = TEST_DIR "/connector/";

static std::string newCoreName(std::string_view baseName)
{
    static std::atomic<int> count{1};
    int value = ++count;

    return std::string(baseName) + std::to_string(value);
}

TEST(connector_potential_interfaces, simple_connector)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = newCoreName("ccore_pi");
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("inp1", "pub1", InterfaceDirection::FROM_TO);

    helics::ValueFederate vfed("c1", fedInfo);
    vfed.registerInterfaces(std::string(testdir) + "simple_interfaces.json");

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed.enterExecutingMode();
    EXPECT_EQ(vfed.getPublicationCount(), 1);
    EXPECT_EQ(vfed.getInputCount(), 1);
    auto& pub1 = vfed.getPublication(0);
    auto& inp1 = vfed.getInput(0);
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

TEST(connector_potential_interfaces, simple_connector_async)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = newCoreName("ccore_pi");
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("inp1", "pub1", InterfaceDirection::FROM_TO);

    helics::ValueFederate vfed("c1", fedInfo);
    vfed.registerInterfaces(std::string(testdir) + "simple_interfaces.json");

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed.enterInitializingModeAsync();
    vfed.enterInitializingModeComplete();
    vfed.enterExecutingMode();

    EXPECT_EQ(vfed.getPublicationCount(), 1);
    EXPECT_EQ(vfed.getInputCount(), 1);
    auto& pub1 = vfed.getPublication(0);
    auto& inp1 = vfed.getInput(0);
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

TEST(connector_potential_interfaces, simple_connector_init_iteration)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = newCoreName("ccore_pi");
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("inp1", "pub1", InterfaceDirection::FROM_TO);

    helics::ValueFederate vfed("c1", fedInfo);
    vfed.registerInterfaces(std::string(testdir) + "simple_interfaces.json");

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed.enterInitializingModeIterative();
    vfed.enterExecutingMode();

    EXPECT_EQ(vfed.getPublicationCount(), 1);
    EXPECT_EQ(vfed.getInputCount(), 1);
    auto& pub1 = vfed.getPublication(0);
    auto& inp1 = vfed.getInput(0);
    const double testValue = 3452.562;
    const auto& targets = pub1.getDestinationTargets();
    EXPECT_EQ(targets, "inp1");
    pub1.publish(testValue);
    auto retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    auto val = inp1.getDouble();
    EXPECT_EQ(val, testValue);

    vfed.finalize();
    fut.get();
    EXPECT_EQ(conn1.madeConnections(), 1);
}

TEST(connector_potential_interfaces, simple_connector_init_iteration2)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = newCoreName("ccore_pi");
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("inp1", "pub1", InterfaceDirection::FROM_TO);

    helics::ValueFederate vfed("c1", fedInfo);
    vfed.registerInterfaces(std::string(testdir) + "simple_interfaces.json");

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed.enterInitializingModeIterative();
    vfed.enterInitializingModeIterative();
    vfed.enterInitializingModeIterative();
    vfed.enterExecutingMode();

    EXPECT_EQ(vfed.getPublicationCount(), 1);
    EXPECT_EQ(vfed.getInputCount(), 1);
    auto& pub1 = vfed.getPublication(0);
    auto& inp1 = vfed.getInput(0);
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

TEST(connector_potential_interfaces, simple_connector2)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = newCoreName("ccore_pi");
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("inp1", "pub1", InterfaceDirection::FROM_TO);

    helics::ValueFederate vfed("c1", fedInfo);
    vfed.registerInterfaces(std::string(testdir) + "simple_interfaces2.json");

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed.enterExecutingMode();
    EXPECT_EQ(vfed.getPublicationCount(), 1);
    EXPECT_EQ(vfed.getInputCount(), 1);
    auto& pub1 = vfed.getPublication(0);
    auto& inp1 = vfed.getInput(0);
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

TEST(connector_potential_interfaces, endpoint_connector)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = newCoreName("ccore_pi");
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("ept1", "ept12", InterfaceDirection::FROM_TO);

    helics::MessageFederate vfed("c1", fedInfo);

    vfed.registerInterfaces(std::string(testdir) + "simple_interfaces_endpoints.json");

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed.enterExecutingMode();
    EXPECT_EQ(vfed.getEndpointCount(), 2);
    auto& ept1 = vfed.getEndpoint("ept1");
    auto& ept12 = vfed.getEndpoint("ept12");
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

TEST(connector_potential_interfaces, simple_connector_additional_command)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = newCoreName("ccore_pi");
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("inp1", "pub1", InterfaceDirection::FROM_TO);

    helics::ValueFederate vfed("c1", fedInfo);
    vfed.registerInterfaces(std::string(testdir) + "simple_interfaces.json");

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed.sendCommand("c1", "test");
    vfed.enterExecutingMode();
    auto cmd = vfed.getCommand();

    EXPECT_EQ(cmd.first, "test");
    EXPECT_EQ(vfed.getPublicationCount(), 1);
    EXPECT_EQ(vfed.getInputCount(), 1);
    auto& pub1 = vfed.getPublication(0);
    auto& inp1 = vfed.getInput(0);
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

TEST(connector_potential_interfaces, pub_templates)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = newCoreName("ccore_template");
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("obj2/type1", "inp1", InterfaceDirection::FROM_TO);
    helics::ValueFederate vfed("c1", fedInfo);
    vfed.registerInterfaces(std::string(testdir) + "simple_interfaces_templates.json");

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed.enterExecutingMode();
    EXPECT_EQ(vfed.getPublicationCount(), 1);
    EXPECT_EQ(vfed.getInputCount(), 1);
    auto& pub1 = vfed.getPublication(0);
    auto& inp1 = vfed.getInput(0);
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

TEST(connector_potential_interfaces, pub_templates_reverse)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = newCoreName("ccore_template");
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("inp1", "obj2/type1", InterfaceDirection::FROM_TO);
    helics::ValueFederate vfed("c1", fedInfo);
    vfed.registerInterfaces(std::string(testdir) + "simple_interfaces_templates.json");

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed.enterExecutingMode();
    EXPECT_EQ(vfed.getPublicationCount(), 1);
    EXPECT_EQ(vfed.getInputCount(), 1);
    auto& pub1 = vfed.getPublication(0);
    auto& inp1 = vfed.getInput(0);
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

TEST(connector_potential_interfaces, pub_template_alias)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = newCoreName("ccore_template");
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("publicationA", "inp1", InterfaceDirection::FROM_TO);
    helics::ValueFederate vfed("c1", fedInfo);
    vfed.registerInterfaces(std::string(testdir) + "simple_interfaces_templates.json");
    vfed.addAlias("obj2/type1", "publicationA");
    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed.enterExecutingMode();
    EXPECT_EQ(vfed.getPublicationCount(), 1);
    EXPECT_EQ(vfed.getInputCount(), 1);
    auto& pub1 = vfed.getPublication(0);
    auto& inp1 = vfed.getInput(0);
    const double testValue = 3452.562;
    const auto& targets = pub1.getDestinationTargets();
    EXPECT_EQ(targets, "inp1");
    pub1.publish(testValue);
    auto retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    auto val = inp1.getDouble();
    EXPECT_EQ(val, testValue);

    vfed.finalize();
    fut.get();
    EXPECT_EQ(conn1.madeConnections(), 1);
}

TEST(connector_potential_interfaces, pub_input_template)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = newCoreName("ccore_template");
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("obj2/type1", "objA/typeC", InterfaceDirection::FROM_TO);
    helics::ValueFederate vfed("c1", fedInfo);
    vfed.registerInterfaces(std::string(testdir) + "simple_interfaces_templates2.json");
    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed.enterExecutingMode();
    EXPECT_EQ(vfed.getPublicationCount(), 1);
    EXPECT_EQ(vfed.getInputCount(), 1);
    auto& pub1 = vfed.getPublication(0);
    auto& inp1 = vfed.getInput(0);
    const double testValue = 3452.562;
    EXPECT_EQ(pub1.getDestinationTargetCount(), 1U);
    pub1.publish(testValue);
    auto retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    auto val = inp1.getDouble();
    EXPECT_EQ(val, testValue);

    vfed.finalize();
    fut.get();
    EXPECT_EQ(conn1.madeConnections(), 1);
}

TEST(connector_potential_interfaces, pub_input_template3)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = newCoreName("ccore_template");
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("obj2/type1", "objA/typeC", InterfaceDirection::FROM_TO);
    helics::ValueFederate vfed("c1", fedInfo);
    vfed.registerInterfaces(std::string(testdir) + "simple_interfaces_templates3.json");
    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed.enterExecutingMode();
    EXPECT_EQ(vfed.getPublicationCount(), 1);
    EXPECT_EQ(vfed.getInputCount(), 1);
    auto& pub1 = vfed.getPublication(0);
    auto& inp1 = vfed.getInput(0);
    const double testValue = 3452.562;
    EXPECT_EQ(pub1.getDestinationTargetCount(), 1U);
    pub1.publish(testValue);
    auto retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    auto val = inp1.getDouble();
    EXPECT_EQ(val, testValue);

    vfed.finalize();
    fut.get();
    EXPECT_EQ(conn1.madeConnections(), 1);
}

TEST(connector_potential_interfaces, input_pub_template)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = newCoreName("ccore_template");
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("objA/typeC", "obj2/type1", InterfaceDirection::FROM_TO);
    helics::ValueFederate vfed("c1", fedInfo);
    vfed.registerInterfaces(std::string(testdir) + "simple_interfaces_templates2.json");
    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed.enterExecutingMode();
    EXPECT_EQ(vfed.getPublicationCount(), 1);
    EXPECT_EQ(vfed.getInputCount(), 1);
    auto& pub1 = vfed.getPublication(0);
    auto& inp1 = vfed.getInput(0);
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

TEST(connector_potential_interfaces, input_pub_template_alias)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = newCoreName("ccore_template");
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("inputA", "Publication_key", InterfaceDirection::FROM_TO);
    helics::ValueFederate vfed("c1", fedInfo);
    vfed.registerInterfaces(std::string(testdir) + "simple_interfaces_templates2.json");
    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed.addAlias("obj2/type1", "Publication_key");
    vfed.addAlias("objA/typeC", "inputA");
    vfed.enterExecutingMode();
    EXPECT_EQ(vfed.getPublicationCount(), 1);
    EXPECT_EQ(vfed.getInputCount(), 1);
    auto& pub1 = vfed.getPublication(0);
    auto& inp1 = vfed.getInput(0);
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

TEST(connector_potential_interfaces, input_pub_template_potential_match)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = newCoreName("ccore_template");
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("pub1", "objA/typeC", InterfaceDirection::FROM_TO);
    conn1.addConnection("obj1/type1", "inp1", InterfaceDirection::FROM_TO);
    conn1.addConnection("inp2", "obj2/type3", InterfaceDirection::FROM_TO);
    conn1.addConnection("objB/typeA", "pub2", InterfaceDirection::FROM_TO);
    helics::ValueFederate vfed1("c1", fedInfo);
    helics::ValueFederate vfed2("c2", fedInfo);
    vfed1.registerInterfaces(std::string(testdir) + "template_interface_test1.json");
    vfed2.registerInterfaces(std::string(testdir) + "potential_interface_test1.json");

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed2.enterExecutingModeAsync();
    vfed1.enterExecutingMode();
    vfed2.enterExecutingModeComplete();

    EXPECT_EQ(vfed1.getPublicationCount(), 2);
    EXPECT_EQ(vfed1.getInputCount(), 2);
    EXPECT_EQ(vfed2.getPublicationCount(), 2);
    EXPECT_EQ(vfed2.getInputCount(), 2);

    auto& pub1_1 = vfed1.getPublication(0);
    auto& pub1_2 = vfed1.getPublication(1);
    auto& inp1_1 = vfed1.getInput(0);
    auto& inp1_2 = vfed1.getInput(1);

    auto& pub2_1 = vfed2.getPublication(0);
    auto& pub2_2 = vfed2.getPublication(1);
    auto& inp2_1 = vfed2.getInput(0);
    auto& inp2_2 = vfed2.getInput(1);

    const double testValue = 3452.5625;
    pub1_1.publish(testValue);
    pub1_2.publish(testValue + 1.0);
    pub2_1.publish(testValue + 2.0);
    pub2_2.publish(testValue + 3.0);
    vfed2.requestTimeAsync(5);
    auto retTime = vfed1.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    retTime = vfed2.requestTimeComplete();
    EXPECT_EQ(retTime, 1.0);
    auto val = inp1_1.getDouble();
    EXPECT_EQ(val, testValue + 2.0);
    val = inp1_2.getDouble();
    EXPECT_EQ(val, testValue + 3.0);
    val = inp2_1.getDouble();
    EXPECT_EQ(val, testValue + 1.0);
    val = inp2_2.getDouble();
    EXPECT_EQ(val, testValue);

    vfed1.finalize();
    vfed2.finalize();
    fut.get();
    EXPECT_EQ(conn1.madeConnections(), 4);
}

TEST(connector_potential_interfaces, input_pub_template_with_units)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = newCoreName("ccore_template");
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("inputA", "Publication_key", InterfaceDirection::FROM_TO);
    helics::ValueFederate vfed("c1", fedInfo);
    vfed.registerInterfaces(std::string(testdir) + "simple_interfaces_templates_with_units.json");
    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed.addAlias("obj2/type3", "Publication_key");
    vfed.addAlias("objA/typeC", "inputA");
    vfed.enterExecutingMode();
    EXPECT_EQ(vfed.getPublicationCount(), 1);
    EXPECT_EQ(vfed.getInputCount(), 1);
    auto& pub1 = vfed.getPublication(0);
    auto& inp1 = vfed.getInput(0);

    EXPECT_EQ(pub1.getUnits(), "kV");
    EXPECT_EQ(inp1.getUnits(), "V");
    const double testValue = 3452.562;
    pub1.publish(testValue);
    auto retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    auto val = inp1.getDouble();
    EXPECT_EQ(val, testValue * 1000.0);

    vfed.finalize();
    fut.get();
    EXPECT_EQ(conn1.madeConnections(), 1);
}

TEST(connector_potential_interfaces, endpoint_template)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = newCoreName("ccore_template");
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("obj1/eptA", "obj2/eptB", InterfaceDirection::FROM_TO);

    helics::MessageFederate mfed("c1", fedInfo);
    mfed.registerInterfaces(std::string(testdir) + "simple_endpoint_template.json");
    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    mfed.enterExecutingMode();

    EXPECT_EQ(mfed.getEndpointCount(), 2);
    auto& ept1 = mfed.getEndpoint(0);
    auto& ept2 = mfed.getEndpoint(1);

    constexpr std::string_view testValue = "this is a test string";
    ept1.send(testValue);
    auto retTime = mfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    EXPECT_TRUE(ept2.hasMessage());
    auto message = ept2.getMessage();
    mfed.finalize();
    fut.get();
    ASSERT_TRUE(message);
    EXPECT_EQ(message->to_string(), testValue);
}

TEST(connector_potential_interfaces, endpoint_template_alias)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = newCoreName("ccore_template");
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("ept1", "ept2", InterfaceDirection::FROM_TO);

    helics::MessageFederate mfed("c1", fedInfo);
    mfed.registerInterfaces(std::string(testdir) + "simple_endpoint_template.json");
    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    mfed.addAlias("obj1/eptA", "ept1");
    mfed.addAlias("obj2/eptB", "ept2");
    mfed.enterExecutingMode();

    EXPECT_EQ(mfed.getEndpointCount(), 2);
    auto& ept1 = mfed.getEndpoint(0);
    auto& ept2 = mfed.getEndpoint(1);

    constexpr std::string_view testValue = "this is a test string";
    ept1.send(testValue);
    auto retTime = mfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    EXPECT_TRUE(ept2.hasMessage());
    auto message = ept2.getMessage();
    mfed.finalize();
    fut.get();
    ASSERT_TRUE(message);
    EXPECT_EQ(message->to_string(), testValue);
}

TEST(connector_potential_interfaces, big_endpoint_template_alias)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = newCoreName("ccore_template");
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("ept1", "ept2", InterfaceDirection::FROM_TO);

    helics::MessageFederate mfed("c1", fedInfo);
    mfed.registerInterfaces(std::string(testdir) + "complex_endpoint_template.json");
    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    mfed.addAlias("obj1/eptA/typeY/modeN", "ept1");
    mfed.addAlias("obj2/eptB/typeQ/modeR", "ept2");
    mfed.enterExecutingMode();

    EXPECT_EQ(mfed.getEndpointCount(), 2);
    auto& ept1 = mfed.getEndpoint(0);
    auto& ept2 = mfed.getEndpoint(1);

    constexpr std::string_view testValue = "this is a test string";
    ept1.send(testValue);
    auto retTime = mfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    EXPECT_TRUE(ept2.hasMessage());
    auto message = ept2.getMessage();

    mfed.finalize();
    fut.get();
    ASSERT_TRUE(message);
    EXPECT_EQ(message->to_string(), testValue);
}
