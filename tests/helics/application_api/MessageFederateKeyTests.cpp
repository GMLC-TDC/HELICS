/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/BrokerApp.hpp"
#include "helics/application_api/CombinationFederate.hpp"
#include "helics/application_api/CoreApp.hpp"
#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/MessageFederate.hpp"
#include "helics/core/Core.hpp"

#include <gtest/gtest.h>
#ifndef HELICS_SHARED_LIBRARY
#    include "testFixtures.hpp"
#else
#    include "testFixtures_shared.hpp"
#endif

#include <future>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

/** these test cases test out the message federates
 */

class mfed_single_type_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {};

class mfed_type_tests: public ::testing::TestWithParam<const char*>, public FederateTestFixture {};

class mfed_all_type_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {};

class mfed_add_type_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {};

class mfed_tests: public ::testing::Test, public FederateTestFixture {};
/** test simple creation and destruction*/

static const auto testNamer = [](const ::testing::TestParamInfo<const char*>& parameter) {
    return std::string(parameter.param);
};

TEST_P(mfed_single_type_tests, send_receive)
{
    SetupTest<helics::MessageFederate>(GetParam(), 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);

    auto epid = mFed1->registerEndpoint("ep1");
    auto epid2 = mFed1->registerGlobalEndpoint("ep2", "random");
    mFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);

    mFed1->enterExecutingMode();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    helics::SmallBuffer data(500, 'a');

    epid.sendTo(data, "ep2");

    auto time = mFed1->requestTime(1.0);
    EXPECT_EQ(time, 1.0);

    auto res = mFed1->hasMessage();
    EXPECT_TRUE(res);
    res = mFed1->hasMessage(epid);
    EXPECT_TRUE(res == false);
    res = mFed1->hasMessage(epid2);
    EXPECT_TRUE(res);

    auto M = mFed1->getMessage(epid2);
    ASSERT_TRUE(M);
    ASSERT_EQ(M->data.size(), data.size());

    EXPECT_EQ(M->data[245], data[245]);
    mFed1->finalize();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

TEST_P(mfed_single_type_tests, send_receive_obj)
{
    using namespace helics;
    SetupTest<helics::MessageFederate>(GetParam(), 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);

    Endpoint epid(mFed1.get(), "ep1");

    Endpoint epid2(helics::InterfaceVisibility::GLOBAL, mFed1.get(), "ep2", "random");
    mFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);

    mFed1->enterExecutingMode();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    helics::SmallBuffer data(500, 'a');

    epid.sendTo(data, "ep2");

    auto time = mFed1->requestTime(1.0);
    EXPECT_EQ(time, 1.0);

    auto res = mFed1->hasMessage();
    EXPECT_TRUE(res);
    res = epid.hasMessage();
    EXPECT_TRUE(res == false);
    res = epid2.hasMessage();
    EXPECT_TRUE(res);

    auto M = epid2.getMessage();
    ASSERT_TRUE(M);
    ASSERT_EQ(M->data.size(), data.size());

    EXPECT_EQ(M->data[245], data[245]);
    mFed1->finalize();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

TEST_P(mfed_type_tests, send_receive_2fed)
{
    SetupTest<helics::MessageFederate>(GetParam(), 2);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    auto epid = mFed1->registerEndpoint("ep1");
    auto epid2 = mFed2->registerGlobalEndpoint("ep2", "random");

    mFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    mFed2->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);

    auto f1finish = std::async(std::launch::async, [&]() { mFed1->enterExecutingMode(); });
    mFed2->enterExecutingMode();
    f1finish.wait();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    helics::SmallBuffer data(500, 'a');
    helics::SmallBuffer data2(400, 'b');

    epid.sendTo(data, "ep2");
    epid2.sendTo(data2, "fed0/ep1");
    // move the time to 1.0
    auto f1time = std::async(std::launch::async, [&]() { return mFed1->requestTime(1.0); });
    auto gtime = mFed2->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(f1time.get(), 1.0);

    auto res = mFed1->hasMessage();
    EXPECT_TRUE(res);
    res = mFed1->hasMessage(epid);
    EXPECT_TRUE(res);
    res = mFed2->hasMessage(epid2);
    EXPECT_TRUE(res);

    auto M1 = mFed1->getMessage(epid);
    ASSERT_TRUE(M1);
    ASSERT_EQ(M1->data.size(), data2.size());

    EXPECT_EQ(M1->data[245], data2[245]);

    auto M2 = mFed2->getMessage(epid2);
    ASSERT_TRUE(M2);
    ASSERT_EQ(M2->data.size(), data.size());

    EXPECT_EQ(M2->data[245], data[245]);
    mFed1->finalize();
    mFed2->finalize();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::FINALIZE);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

TEST_F(mfed_tests, send_receive_2fed_extra)
{
    SetupTest<helics::MessageFederate>("test_7", 2);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto epid = mFed1->registerEndpoint("ep1");
    auto epid2 = mFed2->registerGlobalEndpoint("ep2", "random");

    mFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    mFed2->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    auto f1finish = std::async(std::launch::async, [&]() { mFed1->enterExecutingMode(); });
    mFed2->enterExecutingMode();
    f1finish.wait();
    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    helics::SmallBuffer data(500, 'a');
    helics::SmallBuffer data2(400, 'b');

    epid.sendTo(data, "ep2");
    epid2.sendTo(data2, "fed0/ep1");
    // move the time to 1.0
    auto f1time = std::async(std::launch::async, [&]() { return mFed1->requestTime(1.0); });
    auto gtime = mFed2->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(f1time.get(), 1.0);

    auto res = mFed1->hasMessage();
    EXPECT_TRUE(res);
    res = mFed1->hasMessage(epid);
    EXPECT_TRUE(res);
    res = mFed2->hasMessage(epid2);
    EXPECT_TRUE(res);

    auto M1 = mFed1->getMessage(epid);
    ASSERT_TRUE(M1);
    ASSERT_EQ(M1->data.size(), data2.size());

    EXPECT_EQ(M1->data[245], data2[245]);

    auto M2 = mFed2->getMessage(epid2);
    ASSERT_TRUE(M2);
    ASSERT_EQ(M2->data.size(), data.size());

    EXPECT_EQ(M2->data[245], data[245]);
    mFed1->finalizeAsync();
    mFed2->finalize();
    mFed1->finalizeComplete();
    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::FINALIZE);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

TEST_F(mfed_tests, send_receive_2fed_extra_alias)
{
    SetupTest<helics::MessageFederate>("test_7", 2);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto epid = mFed1->registerEndpoint("ep1");
    auto epid2 = mFed2->registerGlobalEndpoint("ep2", "random");

    mFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    mFed2->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    mFed2->addAlias("ep2", "endpoint2");
    mFed1->addAlias("fed0/ep1", "magic");
    auto f1finish = std::async(std::launch::async, [&]() { mFed1->enterExecutingMode(); });
    mFed2->enterExecutingMode();
    f1finish.wait();
    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    helics::SmallBuffer data(500, 'a');
    helics::SmallBuffer data2(400, 'b');

    epid.sendTo(data, "endpoint2");
    epid2.sendTo(data2, "magic");
    // move the time to 1.0
    auto f1time = std::async(std::launch::async, [&]() { return mFed1->requestTime(1.0); });
    auto gtime = mFed2->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(f1time.get(), 1.0);

    auto res = mFed1->hasMessage();
    EXPECT_TRUE(res);
    res = mFed1->hasMessage(epid);
    EXPECT_TRUE(res);
    res = mFed2->hasMessage(epid2);
    EXPECT_TRUE(res);

    auto M1 = mFed1->getMessage(epid);
    ASSERT_TRUE(M1);
    ASSERT_EQ(M1->data.size(), data2.size());

    EXPECT_EQ(M1->data[245], data2[245]);

    auto M2 = mFed2->getMessage(epid2);
    ASSERT_TRUE(M2);
    ASSERT_EQ(M2->data.size(), data.size());

    EXPECT_EQ(M2->data[245], data[245]);
    mFed1->finalizeAsync();
    mFed2->finalize();
    mFed1->finalizeComplete();
    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::FINALIZE);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

TEST_P(mfed_type_tests, send_receive_2fed_obj)
{
    using namespace helics;
    SetupTest<MessageFederate>(GetParam(), 2);
    auto mFed1 = GetFederateAs<MessageFederate>(0);
    auto mFed2 = GetFederateAs<MessageFederate>(1);

    Endpoint epid(mFed1, "ep1");

    Endpoint epid2(helics::InterfaceVisibility::GLOBAL, mFed2.get(), "ep2", "random");

    mFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    mFed2->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);

    auto f1finish = std::async(std::launch::async, [&]() { mFed1->enterExecutingMode(); });
    mFed2->enterExecutingMode();
    f1finish.wait();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    helics::SmallBuffer data(500, 'a');
    helics::SmallBuffer data2(400, 'b');

    epid.sendTo(data, "ep2");
    epid2.sendTo(data2, "fed0/ep1");
    // move the time to 1.0
    auto f1time = std::async(std::launch::async, [&]() { return mFed1->requestTime(1.0); });
    auto gtime = mFed2->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(f1time.get(), 1.0);

    auto res = mFed1->hasMessage();
    EXPECT_TRUE(res);
    res = epid.hasMessage();
    EXPECT_TRUE(res);
    epid2.hasMessage();
    EXPECT_TRUE(res);

    auto M1 = epid.getMessage();
    ASSERT_TRUE(M1);
    ASSERT_EQ(M1->data.size(), data2.size());

    EXPECT_EQ(M1->data[245], data2[245]);

    auto M2 = epid2.getMessage();
    ASSERT_TRUE(M2);
    ASSERT_EQ(M2->data.size(), data.size());

    EXPECT_EQ(M2->data[245], data[245]);
    mFed1->finalizeAsync();
    mFed2->finalize();
    mFed1->finalizeComplete();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::FINALIZE);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

TEST_P(mfed_all_type_tests, send_receive_2fed_multisend)
{
    SetupTest<helics::MessageFederate>(GetParam(), 2);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    auto epid = mFed1->registerEndpoint("ep1");
    auto epid2 = mFed2->registerGlobalEndpoint("ep2", "random");
    // mFed1->getCorePointer()->setLoggingLevel(0, 5);
    mFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    mFed2->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    epid.setDefaultDestination("ep2");
    auto f1finish = std::async(std::launch::async, [&]() { mFed1->enterExecutingMode(); });
    mFed2->enterExecutingMode();
    f1finish.wait();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    helics::SmallBuffer data1(500, 'a');
    helics::SmallBuffer data2(400, 'b');
    helics::SmallBuffer data3(300, 'c');
    helics::SmallBuffer data4(200, 'd');

    epid.sendTo(data1, "ep2");
    epid.sendTo(data2, "ep2");
    epid.send(data3);
    epid.send(data4);
    // move the time to 1.0
    auto f1time = std::async(std::launch::async, [&]() { return mFed1->requestTime(1.0); });
    auto gtime = mFed2->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(f1time.get(), 1.0);

    EXPECT_TRUE(!mFed1->hasMessage());

    EXPECT_TRUE(!mFed1->hasMessage(epid));
    auto cnt = mFed2->pendingMessageCount(epid2);
    EXPECT_EQ(cnt, 4);

    EXPECT_EQ(epid.getDefaultDestination(), "ep2");
    auto M1 = mFed2->getMessage(epid2);
    ASSERT_TRUE(M1);
    ASSERT_EQ(M1->data.size(), data1.size());

    EXPECT_EQ(M1->data[245], data1[245]);
    // check the count decremented
    cnt = mFed2->pendingMessageCount(epid2);
    EXPECT_EQ(cnt, 3);
    auto M2 = mFed2->getMessage();
    ASSERT_TRUE(M2);
    ASSERT_EQ(M2->data.size(), data2.size());
    EXPECT_EQ(M2->data[245], data2[245]);
    cnt = mFed2->pendingMessageCount(epid2);
    EXPECT_EQ(cnt, 2);

    auto M3 = mFed2->getMessage();
    auto M4 = mFed2->getMessage(epid2);
    ASSERT_TRUE(M3);
    ASSERT_TRUE(M4);
    EXPECT_EQ(M3->data.size(), data3.size());
    EXPECT_EQ(M4->data.size(), data4.size());

    EXPECT_EQ(M4->source, "fed0/ep1");
    EXPECT_EQ(M4->dest, "ep2");
    EXPECT_EQ(M4->original_source, "fed0/ep1");
    EXPECT_EQ(M4->time, 0.0);
    mFed1->finalizeAsync();
    mFed2->finalize();
    mFed1->finalizeComplete();
    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::FINALIZE);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

TEST_P(mfed_all_type_tests, time_interruptions)
{
    SetupTest<helics::MessageFederate>(GetParam(), 2);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    auto epid = mFed1->registerEndpoint("ep1");
    auto epid2 = mFed2->registerGlobalEndpoint("ep2", "random");
    mFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1);
    mFed2->setProperty(HELICS_PROPERTY_TIME_DELTA, 0.5);

    auto f1finish = std::async(std::launch::async, [&]() { mFed1->enterExecutingMode(); });
    mFed2->enterExecutingMode();
    f1finish.wait();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    helics::SmallBuffer data(500, 'a');
    helics::SmallBuffer data2(400, 'b');

    epid.sendTo(data, "ep2");
    epid2.sendTo(data2, "fed0/ep1");
    // move the time to 1.0
    auto f1time = std::async(std::launch::async, [&]() { return mFed1->requestTime(1.0); });
    auto gtime = mFed2->requestTime(1.0);

    ASSERT_EQ(gtime, 0.5);

    ASSERT_TRUE(mFed2->hasMessage(epid2));

    auto M2 = mFed2->getMessage(epid2);
    ASSERT_EQ(M2->data.size(), data.size());

    EXPECT_EQ(M2->data[245], data[245]);

    gtime = mFed2->requestTime(1.0);
    EXPECT_EQ(gtime, 1.0);

    EXPECT_EQ(f1time.get(), 1.0);
    auto M1 = mFed1->getMessage(epid);
    EXPECT_TRUE(M1);
    if (M1) {
        EXPECT_EQ(M1->data.size(), data2.size());
        if (M1->data.size() > 245) {
            EXPECT_EQ(M1->data[245], data2[245]);
        }
    }

    EXPECT_TRUE(mFed1->hasMessage() == false);
    mFed1->finalizeAsync();

    gtime = mFed2->requestTime(2.0);
    EXPECT_EQ(gtime, 2.0);
    mFed2->finalize();
    mFed1->finalizeComplete();
    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::FINALIZE);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

static bool dual_transfer_test_message(std::shared_ptr<helics::CombinationFederate>& vFed1,
                                       std::shared_ptr<helics::CombinationFederate>& vFed2,
                                       helics::Endpoint& ept1,
                                       helics::Endpoint& ept2)
{
    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    vFed2->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);

    bool correct = true;

    auto f1finish = std::async(std::launch::async, [&]() { vFed1->enterExecutingMode(); });
    vFed2->enterExecutingMode();
    f1finish.wait();
    // publish string1 at time=0.0;
    ept1.send("string1");
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
    auto s = ept2.getMessage();
    if (s) {
        // make sure the string is what we expect
        EXPECT_EQ(s->to_string(), "string1");
        if (s->to_string() != "string1") {
            correct = false;
        }
    } else {
        correct = false;
    }

    // publish a second string
    ept1.send("string2");
    // make sure the value is still what we expect
    s = ept2.getMessage();
    EXPECT_FALSE(s);
    if (s) {
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

    s = ept2.getMessage();
    if (s) {
        EXPECT_EQ(s->to_string(), "string2");
        if (s->to_string() != "string2") {
            correct = false;
        }
    } else {
        correct = false;
    }
    vFed1->finalizeAsync();
    vFed2->finalize();
    vFed1->finalizeComplete();
    return correct;
}

TEST_F(mfed_tests, dual_transfer_message_coreApp_link)
{
    SetupTest<helics::CombinationFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::CombinationFederate>(0);
    auto vFed2 = GetFederateAs<helics::CombinationFederate>(1);

    helics::CoreApp cr(vFed1->getCorePointer());
    cr.linkEndpoints("ept1", "ept2");
    // register the endpoints

    auto& ept1 = vFed1->registerGlobalEndpoint("ept1");

    auto& ept2 = vFed2->registerGlobalEndpoint("ept2");
    bool res = dual_transfer_test_message(vFed1, vFed2, ept1, ept2);
    EXPECT_TRUE(res);
}

TEST_P(mfed_single_type_tests, dual_transfer_message_broker_link_late)
{
    SetupTest<helics::CombinationFederate>(GetParam(), 2);
    auto vFed1 = GetFederateAs<helics::CombinationFederate>(0);
    auto vFed2 = GetFederateAs<helics::CombinationFederate>(1);

    auto& broker = brokers[0];

    // register the publications
    auto& ept1 = vFed1->registerGlobalEndpoint("ept1");

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    broker->linkEndpoints("ept1", "ept2");
    auto& ept2 = vFed2->registerGlobalEndpoint("ept2");
    bool res = dual_transfer_test_message(vFed1, vFed2, ept1, ept2);
    EXPECT_TRUE(res);
}

TEST_P(mfed_all_type_tests, dual_transfer_message_broker_link_direct)
{
    SetupTest<helics::CombinationFederate>(GetParam(), 2);
    auto vFed1 = GetFederateAs<helics::CombinationFederate>(0);
    auto vFed2 = GetFederateAs<helics::CombinationFederate>(1);

    auto& broker = brokers[0];

    // register the endpoints
    auto& ept1 = vFed1->registerGlobalEndpoint("ept1");
    auto& ept2 = vFed2->registerGlobalEndpoint("ept2");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    broker->linkEndpoints("ept1", "ept2");
    broker->query("root", "global_flush");
    bool res = dual_transfer_test_message(vFed1, vFed2, ept1, ept2);
    EXPECT_TRUE(res);
}

static constexpr const char* simple_connection_files[] = {"example_connections5.json",
                                                          "example_connections6.json",
                                                          "example_connections5.toml",
                                                          "example_connections6.toml"};

class mfed_link_file: public ::testing::TestWithParam<const char*>, public FederateTestFixture {};

TEST_P(mfed_link_file, dual_transfer_message_broker_link_file)
{
    SetupTest<helics::CombinationFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::CombinationFederate>(0);
    auto vFed2 = GetFederateAs<helics::CombinationFederate>(1);

    auto& broker = brokers[0];

    auto& ept1 = vFed1->registerGlobalEndpoint("ept1");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    auto testFile = std::string(TEST_DIR) + GetParam();
    broker->makeConnections(testFile);
    // register the publications
    auto& ept2 = vFed2->registerGlobalEndpoint("ept2");
    bool res = dual_transfer_test_message(vFed1, vFed2, ept1, ept2);
    EXPECT_TRUE(res);
}

TEST_F(mfed_tests, dual_transfer_message_broker_link_json_string)
{
    SetupTest<helics::CombinationFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::CombinationFederate>(0);
    auto vFed2 = GetFederateAs<helics::CombinationFederate>(1);

    auto& broker = brokers[0];

    auto& ept1 = vFed1->registerGlobalEndpoint("ept1");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    broker->makeConnections(R"({"links":[["ept1", "ept2"]]})");

    // register the endpoints
    auto& ept2 = vFed2->registerGlobalEndpoint("ept2");
    bool res = dual_transfer_test_message(vFed1, vFed2, ept1, ept2);
    EXPECT_TRUE(res);
}

TEST_P(mfed_single_type_tests, dual_transfer_message_core_link)
{
    SetupTest<helics::CombinationFederate>(GetParam(), 2);
    auto vFed1 = GetFederateAs<helics::CombinationFederate>(0);
    auto vFed2 = GetFederateAs<helics::CombinationFederate>(1);

    auto core = vFed1->getCorePointer();
    core->linkEndpoints("ept1", "ept2");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    core = nullptr;
    // register the endpoints
    auto& ept1 = vFed1->registerGlobalTargetedEndpoint("ept1");
    auto& ept2 = vFed2->registerGlobalTargetedEndpoint("ept2");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    bool res = dual_transfer_test_message(vFed1, vFed2, ept1, ept2);
    EXPECT_TRUE(res);
}

TEST_P(mfed_single_type_tests, dual_transfer_message_core_link_late)
{
    SetupTest<helics::CombinationFederate>(GetParam(), 2);
    auto vFed1 = GetFederateAs<helics::CombinationFederate>(0);
    auto vFed2 = GetFederateAs<helics::CombinationFederate>(1);

    auto core = vFed1->getCorePointer();

    // register the publications
    auto& ept1 = vFed1->registerGlobalEndpoint("ept1");

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    core->linkEndpoints("ept1", "ept2");
    core = nullptr;
    auto& ept2 = vFed2->registerGlobalEndpoint("ept2");
    bool res = dual_transfer_test_message(vFed1, vFed2, ept1, ept2);
    EXPECT_TRUE(res);
}

TEST_P(mfed_single_type_tests, dual_transfer_message_core_link_late_switch)
{
    SetupTest<helics::CombinationFederate>(GetParam(), 2);
    auto vFed1 = GetFederateAs<helics::CombinationFederate>(0);
    auto vFed2 = GetFederateAs<helics::CombinationFederate>(1);

    auto core = vFed1->getCorePointer();

    auto& ept1 = vFed1->registerGlobalEndpoint("ept1");

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    core->linkEndpoints("ept1", "ept2");
    core = nullptr;
    // register the publications
    auto& ept2 = vFed2->registerGlobalEndpoint("ept2");
    bool res = dual_transfer_test_message(vFed1, vFed2, ept1, ept2);
    EXPECT_TRUE(res);
}

TEST_P(mfed_single_type_tests, dual_transfer_message_core_link_direct1)
{
    SetupTest<helics::CombinationFederate>(GetParam(), 2);
    auto vFed1 = GetFederateAs<helics::CombinationFederate>(0);
    auto vFed2 = GetFederateAs<helics::CombinationFederate>(1);

    auto core = vFed1->getCorePointer();

    // register the publications
    auto& ept1 = vFed1->registerGlobalEndpoint("ept1");
    auto& ept2 = vFed2->registerGlobalEndpoint("ept2");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    core->linkEndpoints("ept1", "ept2");
    core = nullptr;
    bool res = dual_transfer_test_message(vFed1, vFed2, ept1, ept2);
    EXPECT_TRUE(res);
}

TEST_P(mfed_single_type_tests, dual_transfer_message_core_link_direct2)
{
    SetupTest<helics::CombinationFederate>(GetParam(), 2);
    auto vFed1 = GetFederateAs<helics::CombinationFederate>(0);
    auto vFed2 = GetFederateAs<helics::CombinationFederate>(1);

    auto core = vFed2->getCorePointer();

    // register the publications
    auto& ept1 = vFed1->registerGlobalEndpoint("ept1");
    auto& ept2 = vFed2->registerGlobalEndpoint("ept2");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    core->linkEndpoints("ept1", "ept2");
    core = nullptr;
    bool res = dual_transfer_test_message(vFed1, vFed2, ept1, ept2);
    EXPECT_TRUE(res);
}

TEST_P(mfed_link_file, dual_transfer_message_core_link_file)
{
    SetupTest<helics::CombinationFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::CombinationFederate>(0);
    auto vFed2 = GetFederateAs<helics::CombinationFederate>(1);

    auto core = vFed1->getCorePointer();

    vFed2->registerGlobalInput<std::string>("inp1");
    auto& ept1 = vFed1->registerGlobalEndpoint("ept1");

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    auto testFile = std::string(TEST_DIR) + GetParam();
    core->makeConnections(testFile);
    core = nullptr;
    // register the publications
    auto& ept2 = vFed2->registerGlobalEndpoint("ept2");
    vFed1->registerGlobalPublication<std::string>("pub1");
    bool res = dual_transfer_test_message(vFed1, vFed2, ept1, ept2);
    EXPECT_TRUE(res);
}

INSTANTIATE_TEST_SUITE_P(mfed_tests, mfed_link_file, ::testing::ValuesIn(simple_connection_files));

TEST_F(mfed_tests, dual_transfer_message_core_link_json_string)
{
    SetupTest<helics::CombinationFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::CombinationFederate>(0);
    auto vFed2 = GetFederateAs<helics::CombinationFederate>(1);

    auto core = vFed1->getCorePointer();

    auto& ept1 = vFed1->registerGlobalEndpoint("ept1");

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    core->makeConnections(R"({"links":[["ept1", "ept2"]]})");
    core = nullptr;
    // register the publications
    auto& ept2 = vFed2->registerGlobalEndpoint("ept2");
    bool res = dual_transfer_test_message(vFed1, vFed2, ept1, ept2);
    EXPECT_TRUE(res);
}

INSTANTIATE_TEST_SUITE_P(mfed_tests,
                         mfed_single_type_tests,
                         ::testing::ValuesIn(CoreTypes_single),
                         testNamer);
INSTANTIATE_TEST_SUITE_P(mfed_tests, mfed_type_tests, ::testing::ValuesIn(CoreTypes), testNamer);
INSTANTIATE_TEST_SUITE_P(mfed_tests,
                         mfed_all_type_tests,
                         ::testing::ValuesIn(CoreTypes_all),
                         testNamer);
