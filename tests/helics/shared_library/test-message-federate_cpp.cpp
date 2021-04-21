/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../src/helics/cpp98/MessageFederate.hpp"
#include "cpptestFixtures.hpp"

#include "gtest/gtest.h"
#include <future>
#include <iostream>
#include <thread>
// these test cases test out the message federates

struct mfed_tests: public FederateTestFixture_cpp, public ::testing::Test {
};

class mfed_type_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture_cpp {
};

/** test simple creation and destruction*/
TEST_P(mfed_type_tests, message_federate_initialize_tests)
{
    SetupTest<helicscpp::MessageFederate>(GetParam(), 1);
    auto mFed1 = GetFederateAs<helicscpp::MessageFederate>(0);

    mFed1->enterExecutingMode();

    helics_federate_state mFed1State = mFed1->getCurrentMode();
    EXPECT_TRUE(mFed1State == helics_state_execution);

    mFed1->finalize();

    mFed1State = mFed1->getCurrentMode();
    EXPECT_TRUE(mFed1State == helics_federate_state::helics_state_finalize);
}

TEST_P(mfed_type_tests, message_federate_endpoint_registration)
{
    SetupTest<helicscpp::MessageFederate>(GetParam(), 1);
    auto mFed1 = GetFederateAs<helicscpp::MessageFederate>(0);

    auto epid = mFed1->registerEndpoint("ep1");
    auto epid2 = mFed1->registerGlobalEndpoint("ep2", "random");

    mFed1->enterExecutingMode();

    helics_federate_state mFed1State = mFed1->getCurrentMode();
    EXPECT_TRUE(mFed1State == helics_state_execution);

    EXPECT_EQ(std::string(epid.getName()), "fed0/ep1");
    EXPECT_EQ(std::string(epid2.getName()), "ep2");

    EXPECT_EQ(std::string(epid.getType()), "");
    EXPECT_EQ(std::string(epid2.getType()), "random");

    mFed1->finalize();

    mFed1State = mFed1->getCurrentMode();
    EXPECT_TRUE(mFed1State == helics_federate_state::helics_state_finalize);
}

TEST_P(mfed_type_tests, message_federate_send_receive)
{
    SetupTest<helicscpp::MessageFederate>(GetParam(), 1, 1.0);
    auto mFed1 = GetFederateAs<helicscpp::MessageFederate>(0);

    auto epid = mFed1->registerEndpoint("ep1");
    auto epid2 = mFed1->registerGlobalEndpoint("ep2", "random");

    mFed1->enterExecutingMode();

    std::string data(500, 'a');

    epid.sendMessage("ep2", data, 0.0);
    helics_time time = mFed1->requestTime(1.0);

    EXPECT_EQ(time, 1.0);

    auto res = mFed1->hasMessage();
    EXPECT_TRUE(res);
    res = epid.hasMessage();
    EXPECT_TRUE(res == false);
    res = epid2.hasMessage();
    EXPECT_TRUE(res);

    auto M = epid2.getMessage();
    // BOOST_REQUIRE (M);
    ASSERT_EQ(M.size(), 500);
    EXPECT_NE(M.data(), nullptr);
    if (M.data() != nullptr) {
        EXPECT_EQ(M.c_str()[245], 'a');
    }
    mFed1->finalize();

    auto mFed1State = mFed1->getCurrentMode();
    EXPECT_TRUE(mFed1State == helics_federate_state::helics_state_finalize);
}

INSTANTIATE_TEST_SUITE_P(mfed_tests, mfed_type_tests, ::testing::ValuesIn(core_types_simple));

TEST_F(mfed_tests, Message)
{
    SetupTest<helicscpp::MessageFederate>("test_2", 2, 1.0);
    auto mFed1 = GetFederateAs<helicscpp::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helicscpp::MessageFederate>(1);

    auto epid = mFed1->registerEndpoint("ep1");
    auto epid2 = mFed2->registerGlobalEndpoint("ep2", "random");

    mFed1->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();

    auto m1 = epid.createMessage();
    std::string data(500, 'a');
    m1.data(data).time(0.0).destination("ep2");
    epid.sendMessage(m1);
    epid.sendMessageZeroCopy(m1);

    mFed1->requestTimeAsync(2.0);
    helics_time time = mFed2->requestTime(1.0);

    EXPECT_EQ(time, 1.0);

    auto cnt = epid2.pendingMessages();
    EXPECT_EQ(cnt, 2U);

    auto M1 = epid2.getMessage();
    auto M2 = epid2.getMessage();
    EXPECT_STREQ(M1.c_str(), M2.c_str());

    M2.destination(M1.source()).messageID(45);
    epid2.sendMessage(std::move(M2));
    mFed2->finalize();

    time = mFed1->requestTimeComplete();
    EXPECT_DOUBLE_EQ(time, 1.0);
    EXPECT_EQ(epid.pendingMessages(), 1U);
    auto M3 = epid.getMessage();
    EXPECT_EQ(M3.messageID(), 45);

    helicscpp::Message M4(M3);
    EXPECT_EQ(M4.messageID(), M3.messageID());
    EXPECT_STREQ(M4.source(), M3.source());

    helicscpp::Message M5(std::move(M3));

    EXPECT_EQ(M5.messageID(), M4.messageID());
    EXPECT_STREQ(M5.source(), M4.source());
    EXPECT_FALSE(M3.isValid());  // NOLINT
    mFed1->finalize();
}

TEST_F(mfed_tests, message_create_from_fed)
{
    SetupTest<helicscpp::MessageFederate>("test", 1, 1.0);
    auto mFed1 = GetFederateAs<helicscpp::MessageFederate>(0);

    auto epid = mFed1->registerEndpoint("ep1");
    auto epid2 = mFed1->registerGlobalEndpoint("ep2", "random");

    mFed1->enterExecutingMode();

    helicscpp::Message mess(*mFed1);

    std::string data(500, 'a');

    mess.destination("ep2");
    mess.data(data);

    epid.sendMessage(mess);
    helics_time time = mFed1->requestTime(1.0);

    EXPECT_EQ(time, 1.0);

    auto res = mFed1->hasMessage();
    EXPECT_TRUE(res);
    res = epid.hasMessage();
    EXPECT_TRUE(res == false);
    res = epid2.hasMessage();
    EXPECT_TRUE(res);

    auto M = epid2.getMessage();
    // BOOST_REQUIRE (M);
    ASSERT_EQ(M.size(), 500);
    EXPECT_NE(M.data(), nullptr);
    if (M.data() != nullptr) {
        EXPECT_EQ(M.c_str()[245], 'a');
    }
    mFed1->finalize();

    auto mFed1State = mFed1->getCurrentMode();
    EXPECT_TRUE(mFed1State == helics_federate_state::helics_state_finalize);
}

TEST_F(mfed_tests, message_create_from_ept)
{
    SetupTest<helicscpp::MessageFederate>("test", 1, 1.0);
    auto mFed1 = GetFederateAs<helicscpp::MessageFederate>(0);

    auto epid = mFed1->registerEndpoint("ep1");
    auto epid2 = mFed1->registerGlobalEndpoint("ep2", "random");

    mFed1->enterExecutingMode();

    helicscpp::Message mess(epid);

    std::string data(500, 'a');

    mess.destination("ep2");
    mess.data(data);

    epid.sendMessage(mess);
    helics_time time = mFed1->requestTime(1.0);

    EXPECT_EQ(time, 1.0);

    auto res = mFed1->hasMessage();
    EXPECT_TRUE(res);
    res = epid.hasMessage();
    EXPECT_TRUE(res == false);
    res = epid2.hasMessage();
    EXPECT_TRUE(res);

    auto M = epid2.getMessage();
    // BOOST_REQUIRE (M);
    ASSERT_EQ(M.size(), 500);
    EXPECT_NE(M.data(), nullptr);
    if (M.data() != nullptr) {
        EXPECT_EQ(M.c_str()[245], 'a');
    }
    mFed1->finalize();

    auto mFed1State = mFed1->getCurrentMode();
    EXPECT_TRUE(mFed1State == helics_federate_state::helics_state_finalize);
}

TEST_F(mfed_tests, message_create_from_fed_after)
{
    SetupTest<helicscpp::MessageFederate>("test", 1, 1.0);
    auto mFed1 = GetFederateAs<helicscpp::MessageFederate>(0);

    auto epid = mFed1->registerEndpoint("ep1");
    auto epid2 = mFed1->registerGlobalEndpoint("ep2", "random");

    mFed1->enterExecutingMode();

    helicscpp::Message mess(*mFed1);

    mess.newMessageObject(*mFed1);
    std::string data(500, 'a');

    mess.destination("ep2");
    mess.data(data);

    epid.sendMessage(mess);
    helics_time time = mFed1->requestTime(1.0);

    EXPECT_EQ(time, 1.0);

    auto res = mFed1->hasMessage();
    EXPECT_TRUE(res);
    res = epid.hasMessage();
    EXPECT_TRUE(res == false);
    res = epid2.hasMessage();
    EXPECT_TRUE(res);

    auto M = epid2.getMessage();
    // BOOST_REQUIRE (M);
    ASSERT_EQ(M.size(), 500);
    EXPECT_NE(M.data(), nullptr);
    if (M.data() != nullptr) {
        EXPECT_EQ(M.c_str()[245], 'a');
    }
    mFed1->finalize();

    auto mFed1State = mFed1->getCurrentMode();
    EXPECT_TRUE(mFed1State == helics_federate_state::helics_state_finalize);
}

TEST_F(mfed_tests, message_create_from_ept_after)
{
    SetupTest<helicscpp::MessageFederate>("test", 1, 1.0);
    auto mFed1 = GetFederateAs<helicscpp::MessageFederate>(0);

    auto epid = mFed1->registerEndpoint("ep1");
    auto epid2 = mFed1->registerGlobalEndpoint("ep2", "random");

    mFed1->enterExecutingMode();

    helicscpp::Message mess(epid);
    mess.newMessageObject(epid);
    std::string data(500, 'a');

    mess.destination("ep2");
    mess.data(data);

    epid.sendMessage(mess);
    helics_time time = mFed1->requestTime(1.0);

    EXPECT_EQ(time, 1.0);

    auto res = mFed1->hasMessage();
    EXPECT_TRUE(res);
    res = epid.hasMessage();
    EXPECT_TRUE(res == false);
    res = epid2.hasMessage();
    EXPECT_TRUE(res);

    auto M = epid2.getMessage();
    // BOOST_REQUIRE (M);
    ASSERT_EQ(M.size(), 500);
    EXPECT_NE(M.data(), nullptr);
    if (M.data() != nullptr) {
        EXPECT_EQ(M.c_str()[245], 'a');
    }
    mFed1->finalize();

    auto mFed1State = mFed1->getCurrentMode();
    EXPECT_TRUE(mFed1State == helics_federate_state::helics_state_finalize);
}
