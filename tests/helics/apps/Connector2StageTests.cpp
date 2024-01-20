/*
Copyright (c) 2017-2023,
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
#include "helics/common/JsonProcessingFunctions.hpp"

#include <future>
#include <thread>

class CheckFed {
  public:
    CheckFed(std::string_view name, const helics::FederateInfo& fedInfo)
    {
        vFed = std::make_shared<helics::CombinationFederate>(name, fedInfo);
    }
    void initialize()
    {
        vFed->setQueryCallback(
            [this](std::string_view query) { return generateQueryResponse(query); });
        vFed->enterInitializingModeIterative();
        // this must be done twice
        vFed->enterInitializingModeIterative();
        bool hasCommand{true};
        while (hasCommand) {
            auto cmd = vFed->getCommand();
            hasCommand = !cmd.first.empty();
            if (hasCommand) {
                auto json = helics::fileops::loadJsonStr(cmd.first);
                if (json.isMember("command")) {
                    if (json["command"] == "register_interfaces") {
                        if (json.isMember("publications")) {
                            for (const auto& pub : json["publications"]) {
                                const std::string pubName = pub.asString();
                                if (std::find(potentialPubs.begin(),
                                              potentialPubs.end(),
                                              pubName) != potentialPubs.end()) {
                                    vFed->registerGlobalPublication<double>(pubName);
                                }
                            }
                        }
                        if (json.isMember("inputs")) {
                            for (const auto& input : json["inputs"]) {
                                const std::string inputName = input.asString();
                                if (std::find(potentialInputs.begin(),
                                              potentialInputs.end(),
                                              inputName) != potentialInputs.end()) {
                                    vFed->registerGlobalInput<double>(inputName);
                                }
                            }
                        }
                        if (json.isMember("endpoints")) {
                            for (const auto& endpoint : json["endpoints"]) {
                                const std::string endpointName = endpoint.asString();
                                if (std::find(potentialEndpoints.begin(),
                                              potentialEndpoints.end(),
                                              endpointName) != potentialEndpoints.end()) {
                                    vFed->registerGlobalTargetedEndpoint(endpointName);
                                }
                            }
                        }
                    }
                }
            }
        }
        values.resize(vFed->getInputCount());
        messages.resize(vFed->getEndpointCount());
        vFed->enterInitializingMode();
    }

    void executing() { vFed->enterExecutingMode(); }

    void run(helics::Time endTime)
    {
        helics::Time time = helics::timeZero;

        while (time < endTime) {
            for (int ii = 0; ii < vFed->getPublicationCount(); ++ii) {
                vFed->getPublication(ii).publish(time);
            }
            for (int ii = 0; ii < vFed->getInputCount(); ++ii) {
                auto& ipt = vFed->getInput(ii);
                if (ipt.isUpdated()) {
                    values[ii].push_back(ipt.getDouble());
                }
            }
            for (int ii = 0; ii < vFed->getEndpointCount(); ++ii) {
                auto& ept = vFed->getEndpoint(ii);
                while (ept.hasMessage()) {
                    messages[ii].emplace_back(ept.getMessage()->to_string());
                }
                ept.send("message@" + std::to_string(static_cast<double>(time)));
            }
            time = vFed->requestTime(endTime);
        }
    }

    void finalize() { vFed->finalize(); }
    std::string generateQueryResponse(std::string_view query)
    {
        if (query == "potential_interfaces") {
            Json::Value interfaces;
            if (!potentialInputs.empty()) {
                interfaces["inputs"] = Json::arrayValue;
                for (const auto& pInput : potentialInputs) {
                    interfaces["inputs"].append(pInput);
                }
            }
            if (!potentialPubs.empty()) {
                interfaces["publications"] = Json::arrayValue;
                for (const auto& pPub : potentialPubs) {
                    interfaces["publications"].append(pPub);
                }
            }
            if (!potentialEndpoints.empty()) {
                interfaces["endpoints"] = Json::arrayValue;
                for (const auto& pEpt : potentialEndpoints) {
                    interfaces["endpoints"].append(pEpt);
                }
            }
            return helics::fileops::generateJsonString(interfaces);
        }
        return std::string{};
    }
    void addPotentialInputs(const std::vector<std::string>& potInputs)
    {
        potentialInputs.insert(potentialInputs.end(), potInputs.begin(), potInputs.end());
    }

    void addPotentialPubs(const std::vector<std::string>& potPubs)
    {
        potentialPubs.insert(potentialPubs.end(), potPubs.begin(), potPubs.end());
    }

    void addPotentialEndpoints(const std::vector<std::string>& potEndpoints)
    {
        potentialEndpoints.insert(potentialEndpoints.end(),
                                  potEndpoints.begin(),
                                  potEndpoints.end());
    }
    /** get the values array*/
    const auto& getValues() { return values; }
    /** get the values array*/
    const auto& getMessages() { return messages; }

  private:
    std::shared_ptr<helics::CombinationFederate> vFed;
    std::vector<std::string> potentialInputs;
    std::vector<std::string> potentialPubs;
    std::vector<std::string> potentialEndpoints;
    std::vector<std::vector<double>> values;
    std::vector<std::vector<std::string>> messages;
};

TEST(connector_2stage, simple_connector)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore1";
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("inp1", "pub1", InterfaceDirection::FROM_TO);

    fedInfo.coreInitString = "";
    CheckFed cfed1("c1", fedInfo);
    cfed1.addPotentialInputs({"inp1", "inp2"});
    cfed1.addPotentialPubs({"pub1", "pub3"});

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    cfed1.initialize();
    cfed1.executing();
    cfed1.run(5);
    cfed1.finalize();
    fut.get();
    ASSERT_EQ(cfed1.getValues().size(), 1);
    EXPECT_FALSE(cfed1.getValues()[0].empty());
    EXPECT_EQ(conn1.madeConnections(), 1);
}

TEST(connector_2stage, simple_endpoint_connector)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccoree1";
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connectore1", fedInfo);
    conn1.addConnection("ept1", "ept2", InterfaceDirection::BIDIRECTIONAL);

    fedInfo.coreInitString = "";
    CheckFed cfed1("c1", fedInfo);
    cfed1.addPotentialEndpoints({"ept1", "ept2"});

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    cfed1.initialize();
    cfed1.executing();
    cfed1.run(5);
    cfed1.finalize();
    fut.get();
    ASSERT_EQ(cfed1.getMessages().size(), 2);
    EXPECT_FALSE(cfed1.getMessages()[0].empty());
    EXPECT_FALSE(cfed1.getMessages()[1].empty());
    EXPECT_EQ(conn1.madeConnections(), 2);
}

TEST(connector_2stage, simple_endpoint_connector_one_way)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccoree1";
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connectore1", fedInfo);
    conn1.addConnection("ept1", "ept2", InterfaceDirection::FROM_TO);

    fedInfo.coreInitString = "";
    CheckFed cfed1("c1", fedInfo);
    cfed1.addPotentialEndpoints({"ept1", "ept2"});

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    cfed1.initialize();
    cfed1.executing();
    cfed1.run(5);
    cfed1.finalize();
    fut.get();
    ASSERT_EQ(cfed1.getMessages().size(), 2);
    EXPECT_TRUE(cfed1.getMessages()[0].empty());
    EXPECT_FALSE(cfed1.getMessages()[1].empty());
    EXPECT_EQ(conn1.madeConnections(), 1);
}

TEST(connector_2stage, simple_endpoint_connector_one_way_reverse)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccoree3";
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connectore3", fedInfo);
    conn1.addConnection("ept1", "ept2", InterfaceDirection::TO_FROM);

    fedInfo.coreInitString = "";
    CheckFed cfed1("c1", fedInfo);
    cfed1.addPotentialEndpoints({"ept1", "ept2"});

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    cfed1.initialize();
    cfed1.executing();
    cfed1.run(5);
    cfed1.finalize();
    fut.get();
    ASSERT_EQ(cfed1.getMessages().size(), 2);
    EXPECT_FALSE(cfed1.getMessages()[0].empty());
    EXPECT_TRUE(cfed1.getMessages()[1].empty());
    EXPECT_EQ(conn1.madeConnections(), 1);
}

TEST(connector_2stage, three_fed)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore2";
    fedInfo.coreInitString = "-f3 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector2", fedInfo);
    conn1.addConnection("inp1", "pub1", InterfaceDirection::FROM_TO);

    fedInfo.coreInitString = "";
    CheckFed cfed1("c1", fedInfo);
    cfed1.addPotentialInputs({"inp1", "inp2"});

    helics::ValueFederate vFed2("v2", fedInfo);
    auto& pub1 = vFed2.registerGlobalPublication<double>("pub1");
    vFed2.enterExecutingModeAsync();

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    auto fut2 = std::async(std::launch::async, [&cfed1]() {
        cfed1.initialize();
        cfed1.executing();
        cfed1.run(5);
        cfed1.finalize();
    });
    vFed2.enterExecutingModeComplete();
    pub1.publish(1.0);
    vFed2.requestTime(2.0);
    pub1.publish(2.0);
    vFed2.requestTime(3.0);
    pub1.publish(3.0);
    vFed2.disconnect();
    fut.get();
    fut2.get();
    ASSERT_EQ(cfed1.getValues().size(), 1);
    EXPECT_EQ(cfed1.getValues()[0].size(), 3);
    EXPECT_EQ(conn1.madeConnections(), 1);
}

TEST(connector_2stage, three_fed_endpoint)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccoree5";
    fedInfo.coreInitString = "-f3 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connectore5", fedInfo);
    conn1.addConnection("ept1", "oept1", InterfaceDirection::TO_FROM);

    fedInfo.coreInitString = "";
    CheckFed cfed1("c1", fedInfo);
    cfed1.addPotentialEndpoints({"ept1", "ept2", "ept3"});

    helics::MessageFederate mFed2("m2", fedInfo);
    auto& ept1 = mFed2.registerGlobalTargetedEndpoint("oept1");
    mFed2.enterExecutingModeAsync();

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    auto fut2 = std::async(std::launch::async, [&cfed1]() {
        cfed1.initialize();
        cfed1.executing();
        cfed1.run(5);
        cfed1.finalize();
    });
    mFed2.enterExecutingModeComplete();
    ept1.send("string1");
    mFed2.requestTime(2.0);
    ept1.send("string2");
    mFed2.requestTime(3.0);
    ept1.send("string3");
    mFed2.disconnect();
    fut.get();
    fut2.get();
    ASSERT_EQ(cfed1.getMessages().size(), 1);
    EXPECT_EQ(cfed1.getMessages()[0].size(), 3);
    EXPECT_EQ(conn1.madeConnections(), 1);
}

TEST(connector_2stage, three_fed_endpoint_bi)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccoree6";
    fedInfo.coreInitString = "-f3 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connectore6", fedInfo);
    conn1.addConnection("ept1", "oept1", InterfaceDirection::BIDIRECTIONAL);

    fedInfo.coreInitString = "";
    CheckFed cfed1("c1", fedInfo);
    cfed1.addPotentialEndpoints({"ept1", "ept2", "ept3"});

    helics::MessageFederate mFed2("m2", fedInfo);
    auto& ept1 = mFed2.registerGlobalTargetedEndpoint("oept1");
    mFed2.enterExecutingModeAsync();

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    auto fut2 = std::async(std::launch::async, [&cfed1]() {
        cfed1.initialize();
        cfed1.executing();
        cfed1.run(5);
        cfed1.finalize();
    });
    mFed2.enterExecutingModeComplete();
    ept1.send("string1");
    mFed2.requestTime(2.0);
    ept1.send("string2");
    mFed2.requestTime(3.0);
    ept1.send("string3");
    mFed2.disconnect();
    fut.get();
    fut2.get();
    ASSERT_EQ(cfed1.getMessages().size(), 1);
    EXPECT_EQ(cfed1.getMessages()[0].size(), 3);
    EXPECT_EQ(conn1.madeConnections(), 2);
}

TEST(connector_2stage, three_fed_reverse)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore3";
    fedInfo.coreInitString = "-f3 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector3", fedInfo);
    conn1.addConnection("pub1", "inp1", InterfaceDirection::FROM_TO);

    fedInfo.coreInitString = "";
    CheckFed cfed1("c1", fedInfo);
    cfed1.addPotentialInputs({"inp1", "inp2"});

    helics::ValueFederate vFed2("v2", fedInfo);
    auto& pub1 = vFed2.registerGlobalPublication<double>("pub1");
    vFed2.enterExecutingModeAsync();

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    auto fut2 = std::async(std::launch::async, [&cfed1]() {
        cfed1.initialize();
        cfed1.executing();
        cfed1.run(5);
        cfed1.finalize();
    });
    vFed2.enterExecutingModeComplete();
    pub1.publish(1.0);
    vFed2.requestTime(2.0);
    pub1.publish(2.0);
    vFed2.requestTime(3.0);
    pub1.publish(3.0);
    vFed2.disconnect();
    fut.get();
    fut2.get();
    ASSERT_EQ(cfed1.getValues().size(), 1);
    EXPECT_EQ(cfed1.getValues()[0].size(), 3);
    EXPECT_EQ(conn1.madeConnections(), 1);
}

TEST(connector_2stage, three_fed_input)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore4";
    fedInfo.coreInitString = "-f3 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector4", fedInfo);
    conn1.addConnection("inp1", "publication1", InterfaceDirection::FROM_TO);
    conn1.addConnection("publication1", "pub1", InterfaceDirection::FROM_TO);
    fedInfo.coreInitString = "";
    CheckFed cfed1("c1", fedInfo);
    cfed1.addPotentialPubs({"pub1", "pub2"});

    helics::ValueFederate vFed2("v2", fedInfo);
    auto& inp1 = vFed2.registerGlobalInput<double>("inp1");
    vFed2.enterExecutingModeAsync();

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    auto fut2 = std::async(std::launch::async, [&cfed1]() {
        cfed1.initialize();
        cfed1.executing();
        cfed1.run(5);
        cfed1.finalize();
    });
    vFed2.enterExecutingModeComplete();
    std::vector<double> data;
    if (inp1.isUpdated()) {
        data.push_back(inp1.getDouble());
    }
    vFed2.requestTime(2.0);
    if (inp1.isUpdated()) {
        data.push_back(inp1.getDouble());
    }
    vFed2.requestTime(3.0);
    if (inp1.isUpdated()) {
        data.push_back(inp1.getDouble());
    }
    vFed2.disconnect();
    fut.get();
    fut2.get();
    EXPECT_GE(data.size(), 1);
    EXPECT_EQ(conn1.madeConnections(), 1);
}


TEST(connector_2stage, three_fed_input_regex)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore4r";
    fedInfo.coreInitString = "-f3 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector4", fedInfo);
    conn1.addConnection("REGEX:Battery/EV(?<ev_num>.)_input_voltage", "REGEX:Charger/EV(?<ev_num>.)_output_voltage", InterfaceDirection::FROM_TO);
    fedInfo.coreInitString = "";
    CheckFed cfed1("c1", fedInfo);
    cfed1.addPotentialPubs({"Charger/EV1_output_voltage", "pub2"});

    helics::ValueFederate vFed2("Battery", fedInfo);
    auto& inp1 = vFed2.registerInput<double>("EV1_input_voltage");
    vFed2.enterExecutingModeAsync();

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    auto fut2 = std::async(std::launch::async, [&cfed1]() {
        cfed1.initialize();
        cfed1.executing();
        cfed1.run(5);
        cfed1.finalize();
        });
    vFed2.enterExecutingModeComplete();
    std::vector<double> data;
    if (inp1.isUpdated()) {
        data.push_back(inp1.getDouble());
    }
    vFed2.requestTime(2.0);
    if (inp1.isUpdated()) {
        data.push_back(inp1.getDouble());
    }
    vFed2.requestTime(3.0);
    if (inp1.isUpdated()) {
        data.push_back(inp1.getDouble());
    }
    vFed2.disconnect();
    fut.get();
    fut2.get();
    EXPECT_GE(data.size(), 1);
    EXPECT_EQ(conn1.madeConnections(), 1);
}

TEST(connector_2stage, three_fed_input_alias)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore5";
    fedInfo.coreInitString = "-f3 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector5", fedInfo);
    conn1.addConnection("input1", "pub1", InterfaceDirection::FROM_TO);
    fedInfo.coreInitString = "";
    CheckFed cfed1("c1", fedInfo);
    cfed1.addPotentialPubs({"pub1", "pub2"});

    helics::ValueFederate vFed2("v2", fedInfo);
    auto& inp1 = vFed2.registerGlobalInput<double>("inp1");
    inp1.addAlias("input1");
    vFed2.enterExecutingModeAsync();

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    auto fut2 = std::async(std::launch::async, [&cfed1]() {
        cfed1.initialize();
        cfed1.executing();
        cfed1.run(5);
        cfed1.finalize();
    });
    vFed2.enterExecutingModeComplete();
    std::vector<double> data;
    if (inp1.isUpdated()) {
        data.push_back(inp1.getDouble());
    }
    vFed2.requestTime(2.0);
    if (inp1.isUpdated()) {
        data.push_back(inp1.getDouble());
    }
    vFed2.requestTime(3.0);
    if (inp1.isUpdated()) {
        data.push_back(inp1.getDouble());
    }
    vFed2.disconnect();
    fut.get();
    fut2.get();
    EXPECT_GE(data.size(), 1);
    EXPECT_EQ(conn1.madeConnections(), 1);
}

TEST(connector_2stage, three_fed_alias_reverse)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore6";
    fedInfo.coreInitString = "-f3 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector6", fedInfo);
    conn1.addConnection("publication1", "inp1", InterfaceDirection::FROM_TO);

    fedInfo.coreInitString = "";
    CheckFed cfed1("c1", fedInfo);
    cfed1.addPotentialInputs({"inp1", "inp2"});

    helics::ValueFederate vFed2("v2", fedInfo);
    auto& pub1 = vFed2.registerGlobalPublication<double>("pub1");
    pub1.addAlias("publication1");
    vFed2.enterExecutingModeAsync();

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    auto fut2 = std::async(std::launch::async, [&cfed1]() {
        cfed1.initialize();
        cfed1.executing();
        cfed1.run(5);
        cfed1.finalize();
    });
    vFed2.enterExecutingModeComplete();
    pub1.publish(1.0);
    vFed2.requestTime(2.0);
    pub1.publish(2.0);
    vFed2.requestTime(3.0);
    pub1.publish(3.0);
    vFed2.disconnect();
    fut.get();
    fut2.get();
    ASSERT_EQ(cfed1.getValues().size(), 1);
    EXPECT_EQ(cfed1.getValues()[0].size(), 3);
    EXPECT_EQ(conn1.madeConnections(), 1);
}

TEST(connector_2stage, three_fed_potential_alias)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore7";
    fedInfo.coreInitString = "-f3 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector7", fedInfo);
    conn1.addConnection("publication1", "input1", InterfaceDirection::FROM_TO);

    fedInfo.coreInitString = "";
    CheckFed cfed1("c1", fedInfo);
    cfed1.addPotentialInputs({"inp1", "inp2"});

    helics::ValueFederate vFed2("v2", fedInfo);

    helics::CoreApp core(vFed2.getCorePointer());
    core.addAlias("inp1", "input1");
    auto& pub1 = vFed2.registerGlobalPublication<double>("pub1");
    pub1.addAlias("publication1");
    vFed2.enterExecutingModeAsync();

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    auto fut2 = std::async(std::launch::async, [&cfed1]() {
        cfed1.initialize();
        cfed1.executing();
        cfed1.run(5);
        cfed1.finalize();
    });
    vFed2.enterExecutingModeComplete();
    pub1.publish(1.0);
    vFed2.requestTime(2.0);
    pub1.publish(2.0);
    vFed2.requestTime(3.0);
    pub1.publish(3.0);
    vFed2.disconnect();
    fut.get();
    fut2.get();
    ASSERT_EQ(cfed1.getValues().size(), 1);
    EXPECT_EQ(cfed1.getValues()[0].size(), 3);
    EXPECT_EQ(conn1.madeConnections(), 1);
}

TEST(connector_2stage, three_fed_potential_alias_reverse)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore8";
    fedInfo.coreInitString = "-f3 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector8", fedInfo);
    conn1.addConnection("input1", "publication1", InterfaceDirection::FROM_TO);

    fedInfo.coreInitString = "";
    CheckFed cfed1("c1", fedInfo);
    cfed1.addPotentialInputs({"inp1", "inp2"});

    helics::ValueFederate vFed2("v2", fedInfo);

    helics::CoreApp core(vFed2.getCorePointer());
    core.addAlias("inp1", "input1");
    auto& pub1 = vFed2.registerGlobalPublication<double>("pub1");
    pub1.addAlias("publication1");
    vFed2.enterExecutingModeAsync();

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    auto fut2 = std::async(std::launch::async, [&cfed1]() {
        cfed1.initialize();
        cfed1.executing();
        cfed1.run(5);
        cfed1.finalize();
    });
    vFed2.enterExecutingModeComplete();
    pub1.publish(1.0);
    vFed2.requestTime(2.0);
    pub1.publish(2.0);
    vFed2.requestTime(3.0);
    pub1.publish(3.0);
    vFed2.disconnect();
    fut.get();
    fut2.get();
    ASSERT_EQ(cfed1.getValues().size(), 1);
    EXPECT_EQ(cfed1.getValues()[0].size(), 3);
    EXPECT_EQ(conn1.madeConnections(), 1);
}

TEST(connector_2stage, three_fed_potential_cascade_alias_reverse)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore9";
    fedInfo.coreInitString = "-f3 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector9", fedInfo);
    conn1.addConnection("inputA", "publicationA", InterfaceDirection::FROM_TO);

    fedInfo.coreInitString = "";
    CheckFed cfed1("c1", fedInfo);
    cfed1.addPotentialInputs({"inp1", "inp2"});

    helics::ValueFederate vFed2("v2", fedInfo);

    helics::CoreApp core(vFed2.getCorePointer());
    core.addAlias("inp1", "input1");
    core.addAlias("publicationA", "publication1");
    core.addAlias("input1", "inputA");
    auto& pub1 = vFed2.registerGlobalPublication<double>("pub1");
    pub1.addAlias("publication1");
    vFed2.enterExecutingModeAsync();

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    auto fut2 = std::async(std::launch::async, [&cfed1]() {
        cfed1.initialize();
        cfed1.executing();
        cfed1.run(5);
        cfed1.finalize();
    });
    vFed2.enterExecutingModeComplete();
    pub1.publish(1.0);
    vFed2.requestTime(2.0);
    pub1.publish(2.0);
    vFed2.requestTime(3.0);
    pub1.publish(3.0);
    vFed2.disconnect();
    fut.get();
    fut2.get();
    ASSERT_EQ(cfed1.getValues().size(), 1);
    EXPECT_EQ(cfed1.getValues()[0].size(), 3);
    EXPECT_EQ(conn1.madeConnections(), 1);
}

TEST(connector_2stage, three_fed_alias_unmatched_connection)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore10";
    fedInfo.coreInitString = "-f3 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector10", fedInfo);

    fedInfo.coreInitString = "";
    CheckFed cfed1("c1", fedInfo);
    cfed1.addPotentialInputs({"inp1", "inp2"});

    helics::ValueFederate vFed2("v2", fedInfo);
    auto& pub1 = vFed2.registerGlobalPublication<double>("pub1");
    pub1.addAlias("publication1");
    helics::CoreApp core(vFed2.getCorePointer());
    core.dataLink("publication1", "input1");
    core.addAlias("inp1", "input1");
    vFed2.enterExecutingModeAsync();

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    auto fut2 = std::async(std::launch::async, [&cfed1]() {
        cfed1.initialize();
        cfed1.executing();
        cfed1.run(5);
        cfed1.finalize();
    });
    vFed2.enterExecutingModeComplete();
    pub1.publish(1.0);
    vFed2.requestTime(2.0);
    pub1.publish(2.0);
    vFed2.requestTime(3.0);
    pub1.publish(3.0);
    vFed2.disconnect();
    fut.get();
    fut2.get();
    ASSERT_EQ(cfed1.getValues().size(), 1);
    EXPECT_EQ(cfed1.getValues()[0].size(), 3);
    // no connections through the connector made
    EXPECT_EQ(conn1.madeConnections(), 0);
}

TEST(connector_2stage, three_fed_unknown_pub_alias)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore11";
    fedInfo.coreInitString = "-f3 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector11", fedInfo);
    fedInfo.coreInitString = "";
    CheckFed cfed1("c1", fedInfo);
    cfed1.addPotentialPubs({"pub1", "pub2"});

    helics::ValueFederate vFed2("v2", fedInfo);
    auto& inp1 = vFed2.registerGlobalInput<double>("inp1");
    inp1.addAlias("input1");
    helics::CoreApp core(vFed2.getCorePointer());
    core.dataLink("publication1", "input1");
    core.addAlias("pub1", "publication1");

    inp1.addAlias("input1");
    vFed2.enterExecutingModeAsync();

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    auto fut2 = std::async(std::launch::async, [&cfed1]() {
        cfed1.initialize();
        cfed1.executing();
        cfed1.run(5);
        cfed1.finalize();
    });
    vFed2.enterExecutingModeComplete();
    std::vector<double> data;
    if (inp1.isUpdated()) {
        data.push_back(inp1.getDouble());
    }
    vFed2.requestTime(2.0);
    if (inp1.isUpdated()) {
        data.push_back(inp1.getDouble());
    }
    vFed2.requestTime(3.0);
    if (inp1.isUpdated()) {
        data.push_back(inp1.getDouble());
    }
    vFed2.disconnect();
    fut.get();
    fut2.get();
    EXPECT_GE(data.size(), 1);
    EXPECT_EQ(conn1.madeConnections(), 0);
}

TEST(connector_2stage, three_fed_endpoint_bi_alias)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccoree7";
    fedInfo.coreInitString = "-f3 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connectore7", fedInfo);
    conn1.addConnection("bigEndpoint", "origin", InterfaceDirection::BIDIRECTIONAL);

    fedInfo.coreInitString = "";
    CheckFed cfed1("c1", fedInfo);
    cfed1.addPotentialEndpoints({"ept1", "ept2", "ept3"});

    helics::MessageFederate mFed2("m2", fedInfo);

    helics::CoreApp core(mFed2.getCorePointer());
    auto& ept1 = mFed2.registerGlobalTargetedEndpoint("oept1");
    ept1.addAlias("origin");
    core.addAlias("ept1", "bigEndpoint");
    mFed2.enterExecutingModeAsync();

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    auto fut2 = std::async(std::launch::async, [&cfed1]() {
        cfed1.initialize();
        cfed1.executing();
        cfed1.run(5);
        cfed1.finalize();
    });
    mFed2.enterExecutingModeComplete();
    ept1.send("string1");
    mFed2.requestTime(2.0);
    ept1.send("string2");
    mFed2.requestTime(3.0);
    ept1.send("string3");
    mFed2.disconnect();
    fut.get();
    fut2.get();
    ASSERT_EQ(cfed1.getMessages().size(), 1);
    EXPECT_EQ(cfed1.getMessages()[0].size(), 3);
    EXPECT_EQ(conn1.madeConnections(), 2);
}

TEST(connector_2stage, three_fed_endpoint_dual_bi_alias)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccoree8";
    fedInfo.coreInitString = "-f3 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connectore8", fedInfo);
    conn1.addConnection("bigEndpoint", "origin", InterfaceDirection::BIDIRECTIONAL);
    conn1.addConnection("origin", "secondary", InterfaceDirection::BIDIRECTIONAL);
    conn1.allowMultipleConnections();
    fedInfo.coreInitString = "";
    CheckFed cfed1("c1", fedInfo);
    cfed1.addPotentialEndpoints({"ept1", "ept2", "ept3"});

    helics::MessageFederate mFed2("m2", fedInfo);

    helics::CoreApp core(mFed2.getCorePointer());
    auto& ept1 = mFed2.registerGlobalTargetedEndpoint("oept1");
    ept1.addAlias("origin");
    core.addAlias("ept1", "bigEndpoint");
    core.addAlias("ept2", "secondary");
    mFed2.enterExecutingModeAsync();

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    auto fut2 = std::async(std::launch::async, [&cfed1]() {
        cfed1.initialize();
        cfed1.executing();
        cfed1.run(5);
        cfed1.finalize();
    });
    mFed2.enterExecutingModeComplete();
    ept1.send("string1");
    mFed2.requestTime(2.0);
    ept1.send("string2");
    mFed2.requestTime(3.0);
    ept1.send("string3");
    mFed2.disconnect();
    fut.get();
    fut2.get();
    ASSERT_EQ(cfed1.getMessages().size(), 2);
    EXPECT_EQ(cfed1.getMessages()[0].size(), 7);
    EXPECT_EQ(conn1.madeConnections(), 6);
}

TEST(connector_2stage, two_sided_broker_connection)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore15";
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);

    fedInfo.coreInitString = "";
    CheckFed cfed1("c1", fedInfo);
    cfed1.addPotentialInputs({"inp1", "inp2"});
    cfed1.addPotentialPubs({"pub1", "pub2", "pub3"});

    helics::CoreApp core("ccore15");
    core.dataLink("pub1", "inp1");
    core.dataLink("pub2", "inp2");

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    cfed1.initialize();
    cfed1.executing();
    cfed1.run(5);
    cfed1.finalize();
    fut.get();
    ASSERT_EQ(cfed1.getValues().size(), 2);
    EXPECT_FALSE(cfed1.getValues()[0].empty());
    EXPECT_FALSE(cfed1.getValues()[1].empty());
    // not making any connections
    EXPECT_EQ(conn1.madeConnections(), 0);
}

TEST(connector_2stage, two_sided_broker_connection_alias)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore16";
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);

    fedInfo.coreInitString = "";
    CheckFed cfed1("c1", fedInfo);
    cfed1.addPotentialInputs({"inp1", "inp2"});
    cfed1.addPotentialPubs({"pub1", "pub2", "pub3"});

    helics::CoreApp core("ccore16");
    core.addAlias("pub1", "publication1");
    core.addAlias("inp1", "input1");
    core.addAlias("pub2", "publication2");
    core.addAlias("inp2", "input2");
    core.dataLink("publication1", "input1");
    core.dataLink("publication2", "input2");

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    cfed1.initialize();
    cfed1.executing();
    cfed1.run(5);
    cfed1.finalize();
    fut.get();
    ASSERT_EQ(cfed1.getValues().size(), 2);
    EXPECT_FALSE(cfed1.getValues()[0].empty());
    EXPECT_FALSE(cfed1.getValues()[1].empty());
    // not making any connections
    EXPECT_EQ(conn1.madeConnections(), 0);
}

TEST(connector_2stage, two_sided_broker_connection_endpoints)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore17";
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);

    fedInfo.coreInitString = "";
    CheckFed cfed1("c1", fedInfo);
    cfed1.addPotentialEndpoints({"e1", "e2", "e3", "e4"});

    helics::CoreApp core("ccore17");
    core.linkEndpoints("e1", "e2");
    core.linkEndpoints("e3", "e4");

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    cfed1.initialize();
    cfed1.executing();
    cfed1.run(5);
    cfed1.finalize();
    fut.get();
    ASSERT_EQ(cfed1.getMessages().size(), 4);
    EXPECT_TRUE(cfed1.getMessages()[0].empty());
    EXPECT_FALSE(cfed1.getMessages()[1].empty());
    EXPECT_TRUE(cfed1.getMessages()[2].empty());
    EXPECT_FALSE(cfed1.getMessages()[3].empty());
    // not making any connections
    EXPECT_EQ(conn1.madeConnections(), 0);
}

TEST(connector_2stage, two_sided_broker_connection_endpoints_alias)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore18";
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);

    fedInfo.coreInitString = "";
    CheckFed cfed1("c1", fedInfo);
    cfed1.addPotentialEndpoints({"e1", "e2", "e3", "e4"});

    helics::CoreApp core("ccore18");
    core.linkEndpoints("end1", "end2");
    core.linkEndpoints("end3", "end4");
    core.addAlias("e1", "end1");
    core.addAlias("e2", "end2");
    core.addAlias("e3", "end3");
    core.addAlias("e4", "end4");

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    cfed1.initialize();
    cfed1.executing();
    cfed1.run(5);
    cfed1.finalize();
    fut.get();
    ASSERT_EQ(cfed1.getMessages().size(), 4);
    EXPECT_TRUE(cfed1.getMessages()[0].empty());
    EXPECT_FALSE(cfed1.getMessages()[1].empty());
    EXPECT_TRUE(cfed1.getMessages()[2].empty());
    EXPECT_FALSE(cfed1.getMessages()[3].empty());
    // not making any connections
    EXPECT_EQ(conn1.madeConnections(), 0);
}
