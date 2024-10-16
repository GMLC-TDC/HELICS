/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "ValueFederateTestTemplates.hpp"
#include "helics/application_api/BrokerApp.hpp"
#include "helics/application_api/CoreApp.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/helics_definitions.hpp"
#include "helics/helics_enums.h"

#include <future>
#include <gtest/gtest.h>
#include <thread>
#ifndef HELICS_SHARED_LIBRARY
#    include "testFixtures.hpp"
#else
#    include "testFixtures_shared.hpp"
#endif
#include <fstream>
#include <memory>
#include <streambuf>
#include <string>
#include <vector>

/** these test cases test out the value federates
 */
class valuefed_single_type:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {};

class valuefed_all_type_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {};

class valuefed: public ::testing::Test, public FederateTestFixture {};

static const auto testNamer = [](const ::testing::TestParamInfo<const char*>& parameter) {
    return std::string(parameter.param);
};

TEST_P(valuefed_single_type, subscriber_and_publisher_registration)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    using namespace helics;
    SetupTest<ValueFederate>(GetParam(), 1);
    auto vFed1 = GetFederateAs<ValueFederate>(0);

    vFed1->setFlagOption(HELICS_HANDLE_OPTION_CONNECTION_OPTIONAL);

    // register the publications
    Publication pub1(vFed1.get(), "pub1", helicsType<std::string>());
    Publication pub2(helics::InterfaceVisibility::GLOBAL, vFed1, "pub2", helicsType<double>());

    Publication pub3(vFed1, "pub3", helicsType<double>(), "V");

    // these aren't meant to match the publications
    auto& sub1 = vFed1->registerSubscription("sub1");
    sub1.setTag("tag1", "tag1_info");
    EXPECT_EQ(sub1.getTag("tag1"), "tag1_info");

    auto& sub2 = vFed1->registerGlobalInput<int>("inpA");
    sub2.addTarget("sub2");
    sub2.setTag("tag2", "tag2_info");
    EXPECT_EQ(sub2.getTag("tag2"), "tag2_info");
    auto& sub3 = vFed1->registerSubscription("sub3", "V");
    // enter execution
    vFed1->enterExecutingMode();

    EXPECT_TRUE(vFed1->getCurrentMode() == Federate::Modes::EXECUTING);
    // check subscriptions
    const auto& subtarget1 = sub1.getTarget();
    const auto& subtarget2 = sub2.getTarget();
    EXPECT_EQ(subtarget1, "sub1");
    EXPECT_EQ(subtarget2, "sub2");
    const auto& subtarget3 = sub3.getTarget();
    EXPECT_EQ(subtarget3, "sub3");

    EXPECT_EQ(sub2.getName(), "inpA");
    EXPECT_TRUE(sub1.getType().empty());  // def is the default type
    EXPECT_EQ(sub2.getType(), "int32");
    EXPECT_TRUE(sub3.getType().empty());
    EXPECT_EQ(sub3.getUnits(), "V");

    // check publications

    const auto& pubname1 = pub1.getName();
    const auto& pubname2 = pub2.getName();
    EXPECT_EQ(pubname1, "fed0/pub1");
    EXPECT_EQ(pubname2, "pub2");
    const auto& pubname3 = pub3.getName();
    EXPECT_EQ(pubname3, "fed0/pub3");

    EXPECT_EQ(pub3.getType(), "double");
    EXPECT_EQ(pub3.getUnits(), "V");
    vFed1->finalize();

    EXPECT_TRUE(vFed1->getCurrentMode() == Federate::Modes::FINALIZE);
}

TEST_F(valuefed, single_transfer_publisher_alias)
{
    SetupTest<helics::ValueFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    ASSERT_TRUE(vFed1);
    // register the publications
    helics::Publication pub1(helics::InterfaceVisibility::LOCAL,
                             vFed1.get(),
                             "pub1",
                             helics::DataType::HELICS_STRING);

    auto& sub1 = vFed1->registerSubscription("publisher");
    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    vFed1->addAlias(pub1.getName(), "publisher");
    vFed1->enterExecutingMode();
    // publish string1 at time=0.0;
    pub1.publish("string1");
    auto gtime = vFed1->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    std::string string1;
    // get the value
    sub1.getValue(string1);
    // make sure the string is what we expect
    EXPECT_EQ(string1, "string1");
    // publish a second string
    pub1.publish("string2");
    // make sure the value is still what we expect
    sub1.getValue(string1);

    EXPECT_EQ(string1, "string1");
    // advance time
    gtime = vFed1->requestTime(2.0);
    // make sure the value was updated
    EXPECT_EQ(gtime, 2.0);
    sub1.getValue(string1);

    EXPECT_EQ(string1, "string2");
    vFed1->finalize();
}

TEST_F(valuefed, single_transfer_publisher_alias2)
{
    SetupTest<helics::ValueFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    ASSERT_TRUE(vFed1);
    // register the publications
    vFed1->addAlias("pub1", "publisher");

    helics::Publication pub1(helics::InterfaceVisibility::GLOBAL,
                             vFed1.get(),
                             "pub1",
                             helics::DataType::HELICS_STRING);

    auto& sub1 = vFed1->registerSubscription("publisher");
    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);

    vFed1->enterExecutingMode();
    // publish string1 at time=0.0;
    pub1.publish("string1");
    auto gtime = vFed1->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    std::string string1;
    // get the value
    sub1.getValue(string1);
    // make sure the string is what we expect
    EXPECT_EQ(string1, "string1");
    // publish a second string
    pub1.publish("string2");
    // make sure the value is still what we expect
    sub1.getValue(string1);

    EXPECT_EQ(string1, "string1");
    // advance time
    gtime = vFed1->requestTime(2.0);
    // make sure the value was updated
    EXPECT_EQ(gtime, 2.0);
    sub1.getValue(string1);

    EXPECT_EQ(string1, "string2");
    vFed1->finalize();
}

TEST_F(valuefed, regex_link_anon_pub)
{
    SetupTest<helics::ValueFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    ASSERT_TRUE(vFed1);
    // register the publications
    vFed1->addAlias("pub1", "publisher");

    auto& inp1 = vFed1->registerInput<std::string>("input1");
    auto& inp2 = vFed1->registerInput<std::string>("input2");
    auto& inp3 = vFed1->registerInput<std::string>("input3");
    auto& inp4 = vFed1->registerInput<std::string>("input4");

    auto& pub = vFed1->registerPublication<std::string>("");
    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    pub.addTarget("REGEX:fed0/.*");
    vFed1->enterExecutingMode();
    // publish string1 at time=0.0;
    pub.publish("string1");
    auto gtime = vFed1->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    std::string string1;
    // get the value
    inp1.getValue(string1);
    // make sure the string is what we expect
    EXPECT_EQ(string1, "string1");
    // publish a second string
    inp2.getValue(string1);
    EXPECT_EQ(string1, "string1");
    // publish a second string
    inp3.getValue(string1);
    EXPECT_EQ(string1, "string1");
    // publish a second string
    inp4.getValue(string1);
    EXPECT_EQ(string1, "string1");

    const auto& str = pub.getDestinationTargets();
    EXPECT_NE(str.find("input3"), std::string::npos);

    vFed1->finalize();
}

TEST_F(valuefed, regex_link_anon_inp)
{
    SetupTest<helics::ValueFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    ASSERT_TRUE(vFed1);
    // register the publications
    vFed1->addAlias("pub1", "publisher");

    auto& inp1 = vFed1->registerInput<std::string>("");

    auto& pub1 = vFed1->registerPublication<std::string>("pub1");
    auto& pub2 = vFed1->registerPublication<std::string>("pub2");
    auto& pub3 = vFed1->registerPublication<std::string>("pub3");
    auto& pub4 = vFed1->registerPublication<std::string>("pub4");

    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    inp1.addTarget("REGEX:fed0/.*");
    inp1.setOption(HELICS_HANDLE_OPTION_MULTI_INPUT_HANDLING_METHOD,
                   HELICS_MULTI_INPUT_SUM_OPERATION);

    vFed1->enterExecutingMode();
    // publish string1 at time=0.0;
    pub1.publish("string1");
    pub2.publish("string2");
    pub3.publish("string3");
    pub4.publish("string4");

    auto gtime = vFed1->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    std::string string1;
    // get the value
    inp1.getValue(string1);
    // make sure the string is what we expect
    EXPECT_NE(string1.find("string1"), std::string::npos);
    EXPECT_NE(string1.find("string2"), std::string::npos);
    EXPECT_NE(string1.find("string3"), std::string::npos);
    EXPECT_NE(string1.find("string4"), std::string::npos);

    const auto& str = inp1.getSourceTargets();
    EXPECT_NE(str.find("pub2"), std::string::npos);

    vFed1->finalize();
}

TEST_P(valuefed_single_type, single_transfer_publisher)
{
    SetupTest<helics::ValueFederate>(GetParam(), 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    ASSERT_TRUE(vFed1);
    // register the publications
    helics::Publication pub1(helics::InterfaceVisibility::GLOBAL,
                             vFed1.get(),
                             "pub1",
                             helics::DataType::HELICS_STRING);

    auto& sub1 = vFed1->registerSubscription("pub1");
    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    vFed1->enterExecutingMode();
    // publish string1 at time=0.0;
    pub1.publish("string1");
    auto gtime = vFed1->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    std::string string1;
    // get the value
    sub1.getValue(string1);
    // make sure the string is what we expect
    EXPECT_EQ(string1, "string1");
    // publish a second string
    pub1.publish("string2");
    // make sure the value is still what we expect
    sub1.getValue(string1);

    EXPECT_EQ(string1, "string1");
    // advance time
    gtime = vFed1->requestTime(2.0);
    // make sure the value was updated
    EXPECT_EQ(gtime, 2.0);
    sub1.getValue(string1);

    EXPECT_EQ(string1, "string2");
    vFed1->finalize();
}

static bool dualTransfer(std::shared_ptr<helics::ValueFederate>& vFed1,
                         std::shared_ptr<helics::ValueFederate>& vFed2,
                         helics::Publication& pub1,
                         helics::Input& sub1)
{
    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    vFed2->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);

    bool correct = true;

    auto f1finish = std::async(std::launch::async, [&]() { vFed1->enterExecutingMode(); });
    vFed2->enterExecutingMode();
    f1finish.wait();
    // publish string1 at time=0.0;
    pub1.publish("string1");
    auto f1time = std::async(std::launch::async, [&]() { return vFed1->requestTime(1.0); });
    auto gtime = vFed2->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    if (gtime != 1.0) {
        correct = false;
    }
    gtime = f1time.get();
    EXPECT_EQ(gtime, 1.0);
    if (gtime != 1.0) {
        correct = false;
    }
    // get the value
    std::string string1 = sub1.getValue<std::string>();

    // make sure the string is what we expect
    EXPECT_EQ(string1, "string1");
    if (string1 != "string1") {
        correct = false;
    }
    // publish a second string
    pub1.publish("string2");
    // make sure the value is still what we expect
    sub1.getValue(string1);
    EXPECT_EQ(string1, "string1");
    if (string1 != "string1") {
        correct = false;
    }
    // advance time
    f1time = std::async(std::launch::async, [&]() { return vFed1->requestTime(2.0); });
    gtime = vFed2->requestTime(2.0);

    EXPECT_EQ(gtime, 2.0);
    if (gtime != 2.0) {
        correct = false;
    }
    gtime = f1time.get();
    EXPECT_EQ(gtime, 2.0);
    if (gtime != 2.0) {
        correct = false;
    }
    // make sure the value was updated

    sub1.getValue(string1);

    EXPECT_EQ(string1, "string2");
    if (string1 != "string2") {
        correct = false;
    }
    vFed1->finalizeAsync();
    vFed2->finalize();
    vFed1->finalizeComplete();
    return correct;
}

TEST_P(valuefed_all_type_tests, dual_transfer)
{
    SetupTest<helics::ValueFederate>(GetParam(), 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication<std::string>("pub1");

    auto& sub1 = vFed2->registerSubscription("pub1");
    bool res = dualTransfer(vFed1, vFed2, pub1, sub1);
    EXPECT_TRUE(res);
}

TEST_P(valuefed_all_type_tests, dual_transfer_json)
{
    SetupTest<helics::ValueFederate>(GetParam(), 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication("pub1", "json");

    auto& sub1 = vFed2->registerSubscription("pub1");
    bool res = dualTransfer(vFed1, vFed2, pub1, sub1);
    EXPECT_TRUE(res);
}

TEST_P(valuefed_all_type_tests, dual_transfer_inputs)
{
    SetupTest<helics::ValueFederate>(GetParam(), 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication<std::string>("pub1");

    auto& inpid = vFed2->registerInput<std::string>("inp1");

    vFed2->addTarget(inpid, "pub1");
    bool res = dualTransfer(vFed1, vFed2, pub1, inpid);
    EXPECT_TRUE(res);
}

TEST_F(valuefed, dual_transfer_inputs_odd_name)
{
    SetupTest<helics::ValueFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    // register the publications
    std::string oddName(45, static_cast<unsigned char>(201));
    auto& pub1 = vFed1->registerGlobalPublication<std::string>(oddName);

    auto& inpid = vFed2->registerInput<std::string>("inp1");

    vFed2->addTarget(inpid, oddName);
    bool res = dualTransfer(vFed1, vFed2, pub1, inpid);
    EXPECT_TRUE(res);
}

TEST_P(valuefed_all_type_tests, dual_transfer_pubtarget)
{
    SetupTest<helics::ValueFederate>(GetParam(), 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication<std::string>("pub1");
    vFed1->addTarget(pub1, "inp1");

    auto& inpid = vFed2->registerGlobalInput<std::string>("inp1");
    bool res = dualTransfer(vFed1, vFed2, pub1, inpid);
    EXPECT_TRUE(res);
}

TEST_P(valuefed_all_type_tests, dual_transfer_nameless_pub)
{
    SetupTest<helics::ValueFederate>(GetParam(), 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    // register the publications
    auto& pub1 = vFed1->registerPublication<std::string>("");
    vFed1->addTarget(pub1, "inp1");

    auto& inpid = vFed2->registerGlobalInput<std::string>("inp1");
    bool res = dualTransfer(vFed1, vFed2, pub1, inpid);
    EXPECT_TRUE(res);
}

TEST_P(valuefed_all_type_tests, dual_transfer_broker_link)
{
    SetupTest<helics::ValueFederate>(GetParam(), 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    auto& broker = brokers[0];
    broker->dataLink("pub1", "inp1");
    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication<std::string>("pub1");

    auto& inpid = vFed2->registerGlobalInput<std::string>("inp1");
    bool res = dualTransfer(vFed1, vFed2, pub1, inpid);
    EXPECT_TRUE(res);
}

TEST_F(valuefed, dual_transfer_brokerApp_link)
{
    SetupTest<helics::ValueFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    helics::BrokerApp brk(brokers[0]);
    brk.dataLink("pub1", "inp1");
    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication<std::string>("pub1");

    auto& inpid = vFed2->registerGlobalInput<std::string>("inp1");
    bool res = dualTransfer(vFed1, vFed2, pub1, inpid);
    EXPECT_TRUE(res);
}

#ifdef HELICS_ENABLE_ZMQ_CORE
static constexpr const char* config_files[] = {"bes_config.json",
                                               "bes_config.toml",
                                               "bes_config2.json",
                                               "bes_config2.toml"};

class valuefed_flagfile_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {};

TEST_P(valuefed_flagfile_tests, configure_test)
{
    std::string file = std::string(TEST_DIR) + GetParam();
    std::ifstream teststream(file);

    std::string config((std::istreambuf_iterator<char>(teststream)),
                       std::istreambuf_iterator<char>());

    AddBroker("zmq", 1);
    helics::ValueFederate vFed2("", config);
    vFed2.enterExecutingMode();
    auto val = vFed2.getIntegerProperty(helics::defs::Properties::LOG_LEVEL);
    EXPECT_EQ(val, HELICS_LOG_LEVEL_NO_PRINT);
    EXPECT_EQ(vFed2.getName(), "test_bes");
    bool result = vFed2.getFlagOption(HELICS_FLAG_ONLY_TRANSMIT_ON_CHANGE);
    EXPECT_TRUE(result);
    result = vFed2.getFlagOption(HELICS_FLAG_WAIT_FOR_CURRENT_TIME_UPDATE);
    EXPECT_TRUE(result);
    vFed2.finalize();
}

INSTANTIATE_TEST_SUITE_P(valuefed, valuefed_flagfile_tests, ::testing::ValuesIn(config_files));
#endif

TEST_F(valuefed, dual_transfer_coreApp_link)
{
    SetupTest<helics::ValueFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    helics::CoreApp core(vFed1->getCorePointer());
    core.dataLink("pub1", "inp1");
    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication<std::string>("pub1");

    auto& inpid = vFed2->registerGlobalInput<std::string>("inp1");
    bool res = dualTransfer(vFed1, vFed2, pub1, inpid);
    EXPECT_TRUE(res);
}

TEST_P(valuefed_all_type_tests, dual_transfer_broker_link_late)
{
    SetupTest<helics::ValueFederate>(GetParam(), 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    auto& broker = brokers[0];

    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication<std::string>("pub1");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    broker->dataLink("pub1", "inp1");
    auto& inpid = vFed2->registerGlobalInput<std::string>("inp1");
    bool res = dualTransfer(vFed1, vFed2, pub1, inpid);
    EXPECT_TRUE(res);
}

TEST_P(valuefed_all_type_tests, dual_transfer_broker_link_direct)
{
    SetupTest<helics::ValueFederate>(GetParam(), 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    auto& broker = brokers[0];

    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication<std::string>("pub1");

    auto& inpid = vFed2->registerGlobalInput<std::string>("inp1");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    broker->dataLink("pub1", "inp1");
    bool res = dualTransfer(vFed1, vFed2, pub1, inpid);
    EXPECT_TRUE(res);
}

static constexpr const char* simple_connection_files[] = {"example_connections1.json",
                                                          "example_connections2.json",
                                                          "example_connections1.toml",
                                                          "example_connections2.toml",
                                                          "example_connections3.toml",
                                                          "example_connections4.toml"};

class valuefed_link_file:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {};

TEST_P(valuefed_link_file, dual_transfer_broker_link_file)
{
    SetupTest<helics::ValueFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    auto& broker = brokers[0];

    auto& inpid = vFed2->registerGlobalInput<std::string>("inp1");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    auto testFile = std::string(TEST_DIR) + GetParam();
    broker->makeConnections(testFile);
    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication<std::string>("pub1");
    bool res = dualTransfer(vFed1, vFed2, pub1, inpid);
    EXPECT_TRUE(res);
}

TEST_F(valuefed, dual_transfer_broker_link_json_string)
{
    SetupTest<helics::ValueFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    auto& broker = brokers[0];

    auto& inpid = vFed2->registerGlobalInput<std::string>("inp1");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    broker->makeConnections(R"({"connections":[["pub1", "inp1"]]})");

    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication<std::string>("pub1");
    bool res = dualTransfer(vFed1, vFed2, pub1, inpid);
    EXPECT_TRUE(res);
}

TEST_P(valuefed_all_type_tests, dual_transfer_core_link)
{
    SetupTest<helics::ValueFederate>(GetParam(), 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    auto core = vFed1->getCorePointer();
    core->dataLink("pub1", "inp1");
    core = nullptr;
    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication<std::string>("pub1");

    auto& inpid = vFed2->registerGlobalInput<std::string>("inp1");
    bool res = dualTransfer(vFed1, vFed2, pub1, inpid);
    EXPECT_TRUE(res);
}

TEST_P(valuefed_all_type_tests, dual_transfer_core_link_late)
{
    SetupTest<helics::ValueFederate>(GetParam(), 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    auto core = vFed1->getCorePointer();

    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication<std::string>("pub1");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    core->dataLink("pub1", "inp1");
    core = nullptr;
    auto& inpid = vFed2->registerGlobalInput<std::string>("inp1");
    bool res = dualTransfer(vFed1, vFed2, pub1, inpid);
    EXPECT_TRUE(res);
}

TEST_P(valuefed_all_type_tests, dual_transfer_core_link_late_switch)
{
    SetupTest<helics::ValueFederate>(GetParam(), 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    auto core = vFed1->getCorePointer();

    auto& inpid = vFed2->registerGlobalInput<std::string>("inp1");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    core->dataLink("pub1", "inp1");
    core = nullptr;
    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication<std::string>("pub1");
    bool res = dualTransfer(vFed1, vFed2, pub1, inpid);
    EXPECT_TRUE(res);
}

TEST_P(valuefed_all_type_tests, dual_transfer_core_link_direct1)
{
    SetupTest<helics::ValueFederate>(GetParam(), 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    auto core = vFed1->getCorePointer();

    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication<std::string>("pub1");

    auto& inpid = vFed2->registerGlobalInput<std::string>("inp1");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    core->dataLink("pub1", "inp1");
    core = nullptr;
    bool res = dualTransfer(vFed1, vFed2, pub1, inpid);
    EXPECT_TRUE(res);
}

TEST_P(valuefed_all_type_tests, dual_transfer_core_link_direct2)
{
    SetupTest<helics::ValueFederate>(GetParam(), 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    auto core = vFed2->getCorePointer();

    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication<std::string>("pub1");

    auto& inpid = vFed2->registerGlobalInput<std::string>("inp1");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    core->dataLink("pub1", "inp1");
    core = nullptr;
    bool res = dualTransfer(vFed1, vFed2, pub1, inpid);
    EXPECT_TRUE(res);
}

TEST_P(valuefed_link_file, dual_transfer_core_link_file)
{
    SetupTest<helics::ValueFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    auto core = vFed1->getCorePointer();

    auto& inpid = vFed2->registerGlobalInput<std::string>("inp1");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    auto testFile = std::string(TEST_DIR) + GetParam();
    core->makeConnections(testFile);
    core = nullptr;
    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication<std::string>("pub1");
    bool res = dualTransfer(vFed1, vFed2, pub1, inpid);
    EXPECT_TRUE(res);
}

INSTANTIATE_TEST_SUITE_P(valuefed,
                         valuefed_link_file,
                         ::testing::ValuesIn(simple_connection_files));

TEST_F(valuefed, dual_transfer_core_link_json_string)
{
    SetupTest<helics::ValueFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    auto core = vFed1->getCorePointer();

    auto& inpid = vFed2->registerGlobalInput<std::string>("inp1");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    core->makeConnections(R"({"connections":[["pub1", "inp1"]]})");
    core = nullptr;
    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication<std::string>("pub1");
    bool res = dualTransfer(vFed1, vFed2, pub1, inpid);
    EXPECT_TRUE(res);
}

TEST_P(valuefed_single_type, init_publish)
{
    SetupTest<helics::ValueFederate>(GetParam(), 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication<double>("pub1");

    auto& sub1 = vFed1->registerSubscription("pub1");
    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    vFed1->enterInitializingMode();
    pub1.publish(1.0);

    vFed1->enterExecutingMode();
    // get the value set at initialization
    double val = sub1.getDouble();

    EXPECT_EQ(val, 1.0);
    // publish string1 at time=0.0;
    pub1.publish(2.0);
    auto gtime = vFed1->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);

    gtime = vFed1->getCurrentTime();
    EXPECT_EQ(gtime, 1.0);
    // get the value
    sub1.getValue(val);
    // make sure the string is what we expect
    EXPECT_EQ(val, 2.0);
    // publish a second string
    pub1.publish(3.0);
    // make sure the value is still what we expect
    val = sub1.getDouble();

    EXPECT_EQ(val, 2.0);
    // advance time
    gtime = vFed1->requestTime(2.0);
    // make sure the value was updated
    EXPECT_EQ(gtime, 2.0);
    sub1.getValue(val);
    EXPECT_EQ(val, 3.0);
    vFed1->finalize();
}

TEST_P(valuefed_single_type, block_send_receive)
{
    SetupTest<helics::ValueFederate>(GetParam(), 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    vFed1->registerPublication<std::string>("pub1");
    vFed1->registerGlobalPublication<int>("pub2");

    auto& pub3 = vFed1->registerPublication("pub3", "");

    auto& sub1 = vFed1->registerSubscription("fed0/pub3", "");

    helics::SmallBuffer buffer(547, ';');

    vFed1->enterExecutingMode();
    vFed1->publishBytes(pub3, buffer);
    vFed1->requestTime(1.0);
    EXPECT_TRUE(vFed1->isUpdated(sub1));
    auto res = vFed1->getBytes(sub1);
    EXPECT_EQ(res.size(), buffer.size());
    EXPECT_TRUE(vFed1->isUpdated(sub1) == false);
    vFed1->disconnect();
}

/** test the all callback*/

TEST_F(valuefed, all_callback)
{
    SetupTest<helics::ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    auto& pub1 = vFed1->registerPublication<std::string>("pub1");
    auto& pub2 = vFed1->registerGlobalPublication<int>("pub2");

    auto& pub3 = vFed1->registerPublication("pub3", "");

    auto& sub1 = vFed1->registerSubscription("fed0/pub1", "");
    auto& sub2 = vFed1->registerSubscription("pub2", "");
    auto& sub3 = vFed1->registerSubscription("fed0/pub3", "");

    helics::SmallBuffer buffer(547, ';');
    helics::InterfaceHandle lastId;
    helics::Time lastTime;
    vFed1->setInputNotificationCallback([&](const helics::Input& input, helics::Time callTime) {
        lastTime = callTime;
        lastId = input.getHandle();
    });
    vFed1->enterExecutingMode();
    vFed1->publishBytes(pub3, buffer);
    vFed1->requestTime(1.0);
    // the callback should have occurred here
    EXPECT_TRUE(lastId == sub3.getHandle());
    if (lastId == sub3.getHandle()) {
        EXPECT_EQ(lastTime, 1.0);
        EXPECT_EQ(vFed1->getLastUpdateTime(sub3), lastTime);
    } else {
        EXPECT_TRUE(false) << " missed callback\n";
    }

    pub2.publish(4);
    vFed1->requestTime(2.0);
    // the callback should have occurred here
    EXPECT_TRUE(lastId == sub2.getHandle());
    EXPECT_EQ(lastTime, 2.0);
    pub1.publish("this is a test");
    vFed1->requestTime(3.0);
    // the callback should have occurred here
    EXPECT_TRUE(lastId == sub1.getHandle());
    EXPECT_EQ(lastTime, 3.0);

    int ccnt = 0;
    vFed1->setInputNotificationCallback(
        [&](const helics::Input& /*unused*/, helics::Time /*unused*/) { ++ccnt; });

    vFed1->publishBytes(pub3, buffer);
    pub2.publish(4);
    vFed1->requestTime(4.0);
    // the callback should have occurred here
    EXPECT_EQ(ccnt, 2);
    ccnt = 0;  // reset the counter
    vFed1->publishBytes(pub3, buffer);
    pub2.publish(4);
    pub1.publish("test string2");
    vFed1->requestTime(5.0);
    // the callback should have occurred here
    EXPECT_EQ(ccnt, 3);
    vFed1->finalize();
}

TEST_F(valuefed, time_update_callback)
{
    SetupTest<helics::ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    auto& pub1 = vFed1->registerPublication<std::string>("pub1");
    auto& pub2 = vFed1->registerGlobalPublication<int>("pub2");

    auto& pub3 = vFed1->registerPublication("pub3", "");

    auto& sub1 = vFed1->registerSubscription("fed0/pub1", "");
    auto& sub2 = vFed1->registerSubscription("pub2", "");
    auto& sub3 = vFed1->registerSubscription("fed0/pub3", "");

    helics::SmallBuffer buffer(547, ';');
    helics::InterfaceHandle lastId;
    helics::Time lastTime{helics::Time::minVal()};
    int validCount{0};
    vFed1->setInputNotificationCallback([&](const helics::Input& input, helics::Time callTime) {
        lastTime = callTime;
        lastId = input.getHandle();
    });
    vFed1->setTimeUpdateCallback([&](helics::Time newTime, bool iterating) {
        if (newTime > lastTime && !iterating) {
            ++validCount;
        }
    });
    vFed1->enterExecutingMode();
    EXPECT_EQ(validCount, 1);
    vFed1->publishBytes(pub3, buffer);
    vFed1->requestTime(1.0);
    // the callbacks should have occurred here
    EXPECT_EQ(validCount, 2);
    EXPECT_TRUE(lastId == sub3.getHandle());
    if (lastId == sub3.getHandle()) {
        EXPECT_EQ(lastTime, 1.0);
        EXPECT_EQ(vFed1->getLastUpdateTime(sub3), lastTime);
    } else {
        EXPECT_TRUE(false) << " missed callback\n";
    }

    pub2.publish(4);
    vFed1->requestTime(2.0);
    // the callback should have occurred here
    EXPECT_EQ(validCount, 3);
    EXPECT_TRUE(lastId == sub2.getHandle());
    EXPECT_EQ(lastTime, 2.0);
    pub1.publish("this is a test");
    vFed1->requestTime(3.0);
    // the callback should have occurred here
    EXPECT_EQ(validCount, 4);
    EXPECT_TRUE(lastId == sub1.getHandle());
    EXPECT_EQ(lastTime, 3.0);

    vFed1->finalize();
}

TEST_F(valuefed, time_update_callback_single_thread)
{
    extraFederateArgs = "--flags=single_thread_federate";
    SetupTest<helics::ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    EXPECT_TRUE(vFed1->getFlagOption(HELICS_FLAG_SINGLE_THREAD_FEDERATE));
    auto& pub1 = vFed1->registerPublication<std::string>("pub1");
    auto& pub2 = vFed1->registerGlobalPublication<int>("pub2");

    auto& pub3 = vFed1->registerPublication("pub3", "");

    auto& sub1 = vFed1->registerSubscription("fed0/pub1", "");
    auto& sub2 = vFed1->registerSubscription("pub2", "");
    auto& sub3 = vFed1->registerSubscription("fed0/pub3", "");

    helics::SmallBuffer buffer(547, ';');
    helics::InterfaceHandle lastId;
    helics::Time lastTime{helics::Time::minVal()};
    int validCount{0};
    vFed1->setInputNotificationCallback([&](const helics::Input& input, helics::Time callTime) {
        lastTime = callTime;
        lastId = input.getHandle();
    });
    vFed1->setTimeUpdateCallback([&](helics::Time newTime, bool iterating) {
        if (newTime > lastTime && !iterating) {
            ++validCount;
        }
    });
    vFed1->enterExecutingMode();
    EXPECT_EQ(validCount, 1);
    vFed1->publishBytes(pub3, buffer);
    vFed1->requestTime(1.0);
    // the callbacks should have occurred here
    EXPECT_EQ(validCount, 2);
    EXPECT_TRUE(lastId == sub3.getHandle());
    if (lastId == sub3.getHandle()) {
        EXPECT_EQ(lastTime, 1.0);
        EXPECT_EQ(vFed1->getLastUpdateTime(sub3), lastTime);
    } else {
        EXPECT_TRUE(false) << " missed callback\n";
    }

    pub2.publish(4);
    vFed1->requestTime(2.0);
    // the callback should have occurred here
    EXPECT_EQ(validCount, 3);
    EXPECT_TRUE(lastId == sub2.getHandle());
    EXPECT_EQ(lastTime, 2.0);
    pub1.publish("this is a test");
    vFed1->requestTime(3.0);
    // the callback should have occurred here
    EXPECT_EQ(validCount, 4);
    EXPECT_TRUE(lastId == sub1.getHandle());
    EXPECT_EQ(lastTime, 3.0);

    vFed1->finalize();
}

TEST_F(valuefed, mode_update_callback)
{
    using Modes = helics::Federate::Modes;
    SetupTest<helics::ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    helics::Federate::Modes cmode{Modes::STARTUP};

    vFed1->setModeUpdateCallback([&](Modes mNew, Modes /*mOld*/) { cmode = mNew; });
    vFed1->enterInitializingMode();
    EXPECT_EQ(cmode, Modes::INITIALIZING);
    vFed1->enterExecutingMode();
    EXPECT_EQ(cmode, Modes::EXECUTING);
    cmode = Modes::ERROR_STATE;
    vFed1->requestTime(1.0);
    EXPECT_EQ(cmode, Modes::ERROR_STATE);
    vFed1->requestTime(helics::Time::maxVal());
    EXPECT_EQ(cmode, Modes::FINISHED);
    vFed1->finalize();
    EXPECT_EQ(cmode, Modes::FINALIZE);
}

TEST_P(valuefed_single_type, transfer_close)
{
    SetupTest<helics::ValueFederate>(GetParam(), 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication<std::string>("pub1");

    auto& sub1 = vFed1->registerSubscription("pub1");
    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    vFed1->enterExecutingMode();
    // publish string1 at time=0.0;
    pub1.publish("string1");
    auto gtime = vFed1->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    std::string string1 = sub1.getString();
    // get the value
    // make sure the string is what we expect
    EXPECT_EQ(string1, "string1");
    // publish a second string
    pub1.publish("string2");
    // make sure the value is still what we expect
    string1 = sub1.getString();
    EXPECT_EQ(string1, "string1");
    pub1.close();
    // advance time
    gtime = vFed1->requestTime(2.0);
    // make sure the value was updated
    EXPECT_EQ(gtime, 2.0);
    string1 = sub1.getString();

    EXPECT_EQ(string1, "string2");
    pub1.publish("string3");
    // make sure the value is still what we expect

    // advance time
    gtime = vFed1->requestTime(3.0);
    string1 = sub1.getString();
    // make sure we didn't get the last publish
    EXPECT_EQ(string1, "string2");
    vFed1->finalize();
}

TEST_P(valuefed_single_type, transfer_remove_target)
{
    SetupTest<helics::ValueFederate>(GetParam(), 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication<std::string>("pub1");

    auto& sub1 = vFed1->registerSubscription("pub1");
    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    vFed1->enterExecutingMode();
    // publish string1 at time=0.0;
    pub1.publish("string1");
    auto gtime = vFed1->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    std::string string1 = sub1.getString();
    // get the value
    // make sure the string is what we expect
    EXPECT_EQ(string1, "string1");
    // publish a second string
    pub1.publish("string2");
    // make sure the value is still what we expect
    string1 = sub1.getString();
    sub1.getString();
    EXPECT_EQ(string1, "string1");

    sub1.removeTarget("pub1");
    // advance time
    gtime = vFed1->requestTime(2.0);
    // make sure the value was updated
    EXPECT_EQ(gtime, 2.0);
    string1 = sub1.getString();

    EXPECT_EQ(string1, "string2");
    pub1.publish("string3");
    // make sure the value is still what we expect

    // advance time
    gtime = vFed1->requestTime(3.0);
    string1 = sub1.getString();
    // make sure we didn't get the last publish
    EXPECT_EQ(string1, "string2");
    vFed1->finalize();
}

TEST_P(valuefed_all_type_tests, dual_transfer_close)
{
    SetupTest<helics::ValueFederate>(GetParam(), 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication<std::string>("pub1");

    auto& sub1 = vFed2->registerSubscription("pub1");
    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    vFed2->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);

    auto f1finish = std::async(std::launch::async, [&]() { vFed1->enterExecutingMode(); });
    vFed2->enterExecutingMode();
    f1finish.wait();
    // publish string1 at time=0.0;
    pub1.publish("string1");
    auto f1time = std::async(std::launch::async, [&]() { return vFed1->requestTime(1.0); });
    auto gtime = vFed2->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    gtime = f1time.get();
    EXPECT_EQ(gtime, 1.0);
    // get the value
    std::string string1 = sub1.getString();

    // make sure the string is what we expect
    EXPECT_EQ(string1, "string1");
    // publish a second string
    pub1.publish("string2");
    // make sure the value is still what we expect
    sub1.getValue(string1);
    EXPECT_EQ(string1, "string1");
    // advance time
    pub1.close();
    f1time = std::async(std::launch::async, [&]() { return vFed1->requestTime(2.0); });
    gtime = vFed2->requestTime(2.0);

    EXPECT_EQ(gtime, 2.0);
    gtime = f1time.get();
    EXPECT_EQ(gtime, 2.0);
    // make sure the value was updated

    sub1.getValue(string1);

    EXPECT_EQ(string1, "string2");

    pub1.publish("string3");
    // make sure the value is still what we expect

    // advance time
    f1time = std::async(std::launch::async, [&]() { return vFed1->requestTime(3.0); });
    gtime = vFed2->requestTime(3.0);
    string1 = sub1.getString();
    // make sure we didn't get the last publish
    EXPECT_EQ(string1, "string2");
    vFed1->finalize();
    vFed2->finalize();
}

TEST_P(valuefed_all_type_tests, dual_transfer_remove_target)
{
    SetupTest<helics::ValueFederate>(GetParam(), 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication<std::string>("pub1");

    auto& sub1 = vFed2->registerSubscription("pub1");
    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    vFed2->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);

    auto f1finish = std::async(std::launch::async, [&]() { vFed1->enterExecutingMode(); });
    vFed2->enterExecutingMode();
    f1finish.wait();
    // publish string1 at time=0.0;
    pub1.publish("string1");
    auto f1time = std::async(std::launch::async, [&]() { return vFed1->requestTime(1.0); });
    auto gtime = vFed2->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    gtime = f1time.get();
    EXPECT_EQ(gtime, 1.0);
    // get the value
    std::string string1 = sub1.getString();

    // make sure the string is what we expect
    EXPECT_EQ(string1, "string1");
    // publish a second string
    pub1.publish("string2");
    // make sure the value is still what we expect
    sub1.getValue(string1);
    EXPECT_EQ(string1, "string1");
    // the target removal occurs at time 1, thus any message sent after 1.0 should be ignored
    sub1.removeTarget("pub1");
    // advance time
    f1time = std::async(std::launch::async, [&]() { return vFed1->requestTime(2.0); });
    gtime = vFed2->requestTime(2.0);

    EXPECT_EQ(gtime, 2.0);
    gtime = f1time.get();
    EXPECT_EQ(gtime, 2.0);
    // make sure the value was updated

    sub1.getValue(string1);

    EXPECT_EQ(string1, "string2");
    pub1.publish("string3");
    // so in theory the remove target could take a little while since it needs to route through the
    // core on occasion
    f1time = std::async(std::launch::async, [&]() { return vFed1->requestTime(3.0); });
    gtime = vFed2->requestTime(3.0);
    EXPECT_EQ(gtime, 3.0);
    gtime = f1time.get();
    EXPECT_EQ(gtime, 3.0);

    // make sure the value is still what we expect
    string1 = sub1.getString();
    // make sure we didn't get the last publish
    EXPECT_EQ(string1, "string2");
    vFed1->finalize();
    vFed2->finalize();
}

TEST_F(valuefed, rem_target_single_test)
{
    SetupTest<helics::ValueFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication<std::string>("pub1");

    auto& sub1 = vFed2->registerSubscription("pub1");
    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    vFed2->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);

    auto f1finish = std::async(std::launch::async, [&]() { vFed1->enterExecutingMode(); });
    vFed2->enterExecutingMode();
    f1finish.wait();
    // both at executionMode
    // publish string1 at time=0.0;
    pub1.publish("string1");
    auto gtime = vFed1->requestTime(1.0);
    EXPECT_EQ(gtime, 1.0);
    // publish a second string
    pub1.publish("string2");
    gtime = vFed1->requestTime(2.0);
    pub1.publish("string3");
    gtime = vFed1->requestTime(3.0);
    EXPECT_EQ(gtime, 3.0);
    vFed1->finalize();

    // now start on vFed2
    gtime = vFed2->requestTime(1.0);
    EXPECT_EQ(gtime, 1.0);
    // get the value
    std::string string1 = sub1.getString();

    // make sure the string is what we expect
    EXPECT_EQ(string1, "string1");

    // make sure the value is still what we expect
    sub1.getValue(string1);
    EXPECT_EQ(string1, "string1");
    // the target removal occurs at time 1, thus any message sent after 1.0 should be ignored
    sub1.removeTarget("pub1");
    // advance time

    gtime = vFed2->requestTime(2.0);

    EXPECT_EQ(gtime, 2.0);
    // make sure the value was updated

    sub1.getValue(string1);

    EXPECT_EQ(string1, "string2");

    // so in theory the remove target could take a little while since it needs to route through the
    // core on occasion
    gtime = vFed2->requestTime(3.0);
    EXPECT_EQ(gtime, 3.0);

    // make sure the value is still what we expect
    string1 = sub1.getString();
    // make sure we didn't get the last publish
    EXPECT_EQ(string1, "string2");

    vFed2->finalize();
}

TEST_P(valuefed_single_type, dual_transfer_remove_target_input)
{
    SetupTest<helics::ValueFederate>(GetParam(), 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication<std::string>("pub1");

    auto& sub1 = vFed2->registerGlobalInput<std::string>("sub1");
    pub1.addTarget("sub1");
    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    vFed2->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);

    auto f1finish = std::async(std::launch::async, [&]() { vFed1->enterExecutingMode(); });
    vFed2->enterExecutingMode();
    f1finish.wait();
    // publish string1 at time=0.0;
    pub1.publish("string1");
    auto f1time = std::async(std::launch::async, [&]() { return vFed1->requestTime(1.0); });
    auto gtime = vFed2->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    gtime = f1time.get();
    EXPECT_EQ(gtime, 1.0);
    // get the value
    std::string string1 = sub1.getString();

    // make sure the string is what we expect
    EXPECT_EQ(string1, "string1");
    // publish a second string
    pub1.publish("string2");
    // make sure the value is still what we expect
    sub1.getValue(string1);
    EXPECT_EQ(string1, "string1");
    // advance time
    pub1.removeTarget("sub1");
    f1time = std::async(std::launch::async, [&]() { return vFed1->requestTime(2.0); });
    gtime = vFed2->requestTime(2.0);

    EXPECT_EQ(gtime, 2.0);
    gtime = f1time.get();
    EXPECT_EQ(gtime, 2.0);
    // make sure the value was updated

    sub1.getValue(string1);

    EXPECT_EQ(string1, "string2");

    pub1.publish("string3");
    // make sure the value is still what we expect

    // advance time
    f1time = std::async(std::launch::async, [&]() { return vFed1->requestTime(3.0); });
    gtime = vFed2->requestTime(3.0);
    string1 = sub1.getString();
    // make sure we didn't get the last publish
    EXPECT_EQ(string1, "string2");
    vFed1->finalize();
    vFed2->finalize();
}

INSTANTIATE_TEST_SUITE_P(valuefed_key_tests,
                         valuefed_single_type,
                         ::testing::ValuesIn(CoreTypes_single),
                         testNamer);
INSTANTIATE_TEST_SUITE_P(valuefed_key_tests,
                         valuefed_all_type_tests,
                         ::testing::ValuesIn(CoreTypes_all),
                         testNamer);

TEST_F(valuefed, empty_get_default)
{
    SetupTest<helics::ValueFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    auto& sub = vFed1->registerSubscription("test1");
    sub.setOption(helics::defs::Options::CONNECTION_OPTIONAL);
    vFed1->enterExecutingMode();
    vFed1->requestTime(10.0);
    double val1{0.0};
    EXPECT_NO_THROW(val1 = sub.getValue<double>());
    EXPECT_EQ(val1, helics::invalidDouble);

    std::complex<double> valc;
    EXPECT_NO_THROW(valc = sub.getValue<std::complex<double>>());
    EXPECT_EQ(valc, std::complex<double>(helics::invalidDouble, 0));

    vFed1->finalize();
}

TEST_F(valuefed, empty_get_complex)
{
    SetupTest<helics::ValueFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    auto& ipt = vFed1->registerInput("I1", "complex");
    ipt.setOption(helics::defs::Flags::CONNECTIONS_OPTIONAL);
    ipt.addTarget("test_target");
    vFed1->enterExecutingMode();
    vFed1->requestTime(10.0);

    std::complex<double> valc;
    EXPECT_NO_THROW(valc = ipt.getValue<std::complex<double>>());
    EXPECT_EQ(valc, std::complex<double>(helics::invalidDouble, 0));

    double val1;
    EXPECT_NO_THROW(val1 = ipt.getValue<double>());
    EXPECT_EQ(val1, helics::invalidDouble);

    vFed1->finalize();
}

TEST_F(valuefed, publish_time_restrict)
{
    SetupTest<helics::ValueFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication<int>("pub1");

    auto& sub1 = vFed1->registerSubscription("pub1");
    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    pub1.setOption(HELICS_HANDLE_OPTION_TIME_RESTRICTED, 10000);

    vFed1->enterExecutingMode();
    std::vector<int> returned;
    for (auto ii = 0; ii < 200; ++ii) {
        if (sub1.isUpdated()) {
            returned.push_back(sub1.getValue<int>());
        }
        pub1.publish(ii);
        vFed1->requestNextStep();
    }

    vFed1->finalize();
    EXPECT_LE(returned.size(), 21);
    EXPECT_GE(returned.size(), 19);
}

TEST_F(valuefed, input_time_restrict)
{
    SetupTest<helics::ValueFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication<int>("pub1");

    auto& sub1 = vFed1->registerSubscription("pub1");
    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    sub1.setOption(HELICS_HANDLE_OPTION_TIME_RESTRICTED, 10000);

    vFed1->enterExecutingMode();
    std::vector<int> returned;
    for (auto ii = 0; ii < 200; ++ii) {
        if (sub1.isUpdated()) {
            returned.push_back(sub1.getValue<int>());
        }
        pub1.publish(ii);
        vFed1->requestNextStep();
    }

    vFed1->finalize();
    EXPECT_LE(returned.size(), 21);
    EXPECT_GE(returned.size(), 19);
}

TEST_F(valuefed, publish_change_restrict)
{
    SetupTest<helics::ValueFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication<int>("pub1");

    auto& sub1 = vFed1->registerSubscription("pub1");
    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    pub1.setOption(HELICS_HANDLE_OPTION_ONLY_TRANSMIT_ON_CHANGE);

    vFed1->enterExecutingMode();
    std::vector<int> returned;
    for (auto ii = 0; ii < 200; ++ii) {
        if (sub1.isUpdated()) {
            returned.push_back(sub1.getValue<int>());
        }
        pub1.publish(static_cast<int>(ii / 10));
        vFed1->requestNextStep();
    }

    vFed1->finalize();
    EXPECT_LE(returned.size(), 21);
    EXPECT_GE(returned.size(), 19);
}

TEST_F(valuefed, input_change_restrict)
{
    SetupTest<helics::ValueFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication<int>("pub1");

    auto& sub1 = vFed1->registerSubscription("pub1");
    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    sub1.setOption(HELICS_HANDLE_OPTION_ONLY_UPDATE_ON_CHANGE, 1);

    vFed1->enterExecutingMode();
    std::vector<int> returned;
    for (auto ii = 0; ii < 200; ++ii) {
        if (sub1.isUpdated()) {
            returned.push_back(sub1.getValue<int>());
        }
        pub1.publish(static_cast<int>(ii / 10));
        vFed1->requestNextStep();
    }

    vFed1->finalize();
    EXPECT_LE(returned.size(), 21);
    EXPECT_GE(returned.size(), 19);
}
