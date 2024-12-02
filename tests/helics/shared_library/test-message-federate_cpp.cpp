/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../src/helics/cpp98/MessageFederate.hpp"
#include "cpptestFixtures.hpp"

#include "gtest/gtest.h"
#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <utility>
// these test cases test out the message federates

static const auto testNamer = [](const ::testing::TestParamInfo<const char*>& parameter) {
    return std::string(parameter.param);
};

struct mfed_tests: public FederateTestFixture_cpp, public ::testing::Test {};

class mfed_type_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture_cpp {};

/** test simple creation and destruction*/
TEST_P(mfed_type_tests, message_federate_initialize)
{
    SetupTest<helicscpp::MessageFederate>(GetParam(), 1);
    auto mFed1 = GetFederateAs<helicscpp::MessageFederate>(0);

    mFed1->enterExecutingMode();

    HelicsFederateState mFed1State = mFed1->getCurrentMode();
    EXPECT_TRUE(mFed1State == HELICS_STATE_EXECUTION);

    mFed1->finalize();

    mFed1State = mFed1->getCurrentMode();
    EXPECT_TRUE(mFed1State == HelicsFederateState::HELICS_STATE_FINALIZE);
}

TEST_P(mfed_type_tests, message_federate_endpoint_registration)
{
    SetupTest<helicscpp::MessageFederate>(GetParam(), 1);
    auto mFed1 = GetFederateAs<helicscpp::MessageFederate>(0);

    auto epid = mFed1->registerEndpoint("ep1");
    auto epid2 = mFed1->registerGlobalEndpoint("ep2", "random");

    mFed1->enterExecutingMode();

    HelicsFederateState mFed1State = mFed1->getCurrentMode();
    EXPECT_TRUE(mFed1State == HELICS_STATE_EXECUTION);

    EXPECT_EQ(std::string(epid.getName()), "fed0/ep1");
    EXPECT_EQ(std::string(epid2.getName()), "ep2");

    EXPECT_EQ(std::string(epid.getType()), "");
    EXPECT_EQ(std::string(epid2.getType()), "random");

    mFed1->finalize();

    mFed1State = mFed1->getCurrentMode();
    EXPECT_TRUE(mFed1State == HelicsFederateState::HELICS_STATE_FINALIZE);
}

TEST_P(mfed_type_tests, message_federate_send_receive)
{
    SetupTest<helicscpp::MessageFederate>(GetParam(), 1, 1.0);
    auto mFed1 = GetFederateAs<helicscpp::MessageFederate>(0);

    auto epid = mFed1->registerEndpoint("ep1");
    auto epid2 = mFed1->registerGlobalEndpoint("ep2", "random");

    mFed1->enterExecutingMode();

    std::string data(500, 'a');

    epid.sendToAt(data, "ep2", 0.0);
    HelicsTime time = mFed1->requestTime(1.0);

    EXPECT_EQ(time, 1.0);

    auto res = mFed1->hasMessage();
    EXPECT_TRUE(res);
    res = epid.hasMessage();
    EXPECT_TRUE(res == false);
    res = epid2.hasMessage();
    EXPECT_TRUE(res);

    auto message = epid2.getMessage();
    ASSERT_EQ(message.size(), 500);
    EXPECT_NE(message.data(), nullptr);
    if (message.data() != nullptr) {
        EXPECT_EQ(message.c_str()[245], 'a');
    }
    mFed1->finalize();

    auto mFed1State = mFed1->getCurrentMode();
    EXPECT_TRUE(mFed1State == HelicsFederateState::HELICS_STATE_FINALIZE);
}

INSTANTIATE_TEST_SUITE_P(mfed_tests,
                         mfed_type_tests,
                         ::testing::ValuesIn(CoreTypes_simple),
                         testNamer);

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

    auto message = epid.createMessage();
    std::string data(500, 'a');
    message.data(data).time(0.0).destination("ep2");
    epid.sendMessage(message);
    epid.sendMessageZeroCopy(message);

    mFed1->requestTimeAsync(2.0);
    HelicsTime time = mFed2->requestTime(1.0);

    EXPECT_EQ(time, 1.0);

    auto cnt = epid2.pendingMessageCount();
    EXPECT_EQ(cnt, 2U);

    auto message1 = epid2.getMessage();
    auto message2 = epid2.getMessage();

    std::string M1d(message1.c_str(), message1.size());
    std::string M2d(message2.c_str(), message2.size());
    EXPECT_EQ(M1d, M2d);

    message2.destination(message1.source()).messageID(45);
    epid2.sendMessage(std::move(message2));
    mFed2->finalize();

    time = mFed1->requestTimeComplete();
    EXPECT_DOUBLE_EQ(time, 1.0);
    EXPECT_EQ(epid.pendingMessageCount(), 1U);
    auto message3 = epid.getMessage();
    EXPECT_EQ(message3.messageID(), 45);

    helicscpp::Message message4(message3);
    EXPECT_EQ(message4.messageID(), message3.messageID());
    EXPECT_STREQ(message4.source(), message3.source());

    helicscpp::Message message5(std::move(message3));

    EXPECT_EQ(message5.messageID(), message4.messageID());
    EXPECT_STREQ(message5.source(), message4.source());
    EXPECT_FALSE(message3.isValid());  // NOLINT
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
    HelicsTime time = mFed1->requestTime(1.0);

    EXPECT_EQ(time, 1.0);

    auto res = mFed1->hasMessage();
    EXPECT_TRUE(res);
    res = epid.hasMessage();
    EXPECT_TRUE(res == false);
    res = epid2.hasMessage();
    EXPECT_TRUE(res);

    auto message = epid2.getMessage();
    ASSERT_EQ(message.size(), 500);
    EXPECT_NE(message.data(), nullptr);
    if (message.data() != nullptr) {
        EXPECT_EQ(message.c_str()[245], 'a');
    }
    mFed1->finalize();

    auto mFed1State = mFed1->getCurrentMode();
    EXPECT_TRUE(mFed1State == HelicsFederateState::HELICS_STATE_FINALIZE);
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
    HelicsTime time = mFed1->requestTime(1.0);

    EXPECT_EQ(time, 1.0);

    auto res = mFed1->hasMessage();
    EXPECT_TRUE(res);
    res = epid.hasMessage();
    EXPECT_TRUE(res == false);
    res = epid2.hasMessage();
    EXPECT_TRUE(res);

    auto message = epid2.getMessage();
    ASSERT_EQ(message.size(), 500);
    EXPECT_NE(message.data(), nullptr);
    if (message.data() != nullptr) {
        EXPECT_EQ(message.c_str()[245], 'a');
    }
    mFed1->finalize();

    auto mFed1State = mFed1->getCurrentMode();
    EXPECT_TRUE(mFed1State == HelicsFederateState::HELICS_STATE_FINALIZE);
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
    HelicsTime time = mFed1->requestTime(1.0);

    EXPECT_EQ(time, 1.0);

    auto res = mFed1->hasMessage();
    EXPECT_TRUE(res);
    res = epid.hasMessage();
    EXPECT_TRUE(res == false);
    res = epid2.hasMessage();
    EXPECT_TRUE(res);

    auto message = epid2.getMessage();
    ASSERT_EQ(message.size(), 500);
    EXPECT_NE(message.data(), nullptr);
    if (message.data() != nullptr) {
        EXPECT_EQ(message.c_str()[245], 'a');
    }
    mFed1->finalize();

    auto mFed1State = mFed1->getCurrentMode();
    EXPECT_TRUE(mFed1State == HelicsFederateState::HELICS_STATE_FINALIZE);
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
    HelicsTime time = mFed1->requestTime(1.0);

    EXPECT_EQ(time, 1.0);

    auto res = mFed1->hasMessage();
    EXPECT_TRUE(res);
    res = epid.hasMessage();
    EXPECT_TRUE(res == false);
    res = epid2.hasMessage();
    EXPECT_TRUE(res);

    auto message = epid2.getMessage();
    ASSERT_EQ(message.size(), 500);
    EXPECT_NE(message.data(), nullptr);
    if (message.data() != nullptr) {
        EXPECT_EQ(message.c_str()[245], 'a');
    }
    mFed1->finalize();

    auto mFed1State = mFed1->getCurrentMode();
    EXPECT_TRUE(mFed1State == HelicsFederateState::HELICS_STATE_FINALIZE);
}

TEST_F(mfed_tests, dataBuffer)
{
    SetupTest<helicscpp::MessageFederate>("test", 1, 1.0);
    auto mFed1 = GetFederateAs<helicscpp::MessageFederate>(0);

    auto message1 = helicscpp::Message(*mFed1);

    auto message2 = helicscpp::Message(*mFed1);
    message1.data("raw data");

    EXPECT_EQ(message1.size(), 8);
    EXPECT_TRUE(message1.isValid());
    EXPECT_STREQ(message1.c_str(), "raw data");

    // test the connection between the buffer and message
    EXPECT_EQ(message2.size(), 0);
    EXPECT_FALSE(message2.isValid());
    auto buffer = message2.dataBuffer();

    EXPECT_TRUE(buffer.isValid());

    mFed1->enterExecutingMode();
    mFed1->finalize();
}

TEST(dataBuffer, buffer)
{
    helicscpp::DataBuffer buf1(345);
    EXPECT_GE(buf1.capacity(), 345);
    EXPECT_TRUE(buf1.reserve(1024));
    EXPECT_GE(buf1.capacity(), 1024);
    EXPECT_FALSE(buf1.reserve(-456));
    std::string str1("this is a long string that I want to put in a buffer");
    const char* str2 = "this is another string, that is fairly long that I want to put in a buffer";
    buf1.fill(str1);
    // +1 is to account for newline stringSize is the size required to hold the string
    EXPECT_EQ(buf1.stringSize(), static_cast<int>(str1.size() + 1));
    EXPECT_EQ(buf1.toString(), str1);
    buf1.fill(str2);
    EXPECT_EQ(buf1.toString(), str2);

    EXPECT_EQ(buf1.type(), HELICS_DATA_TYPE_STRING);

    auto buf2 = buf1.clone();
    EXPECT_EQ(buf1.toString(), buf2.toString());

    double tValue = 45.626525;
    buf2.fill(tValue);
    EXPECT_EQ(buf2.type(), HELICS_DATA_TYPE_DOUBLE);

    buf2.convertToType(HELICS_DATA_TYPE_NAMED_POINT);

    EXPECT_EQ(buf2.type(), HELICS_DATA_TYPE_NAMED_POINT);

    buf2.convertToType(HELICS_DATA_TYPE_VECTOR);
    EXPECT_EQ(buf2.vectorSize(), 1);

    EXPECT_EQ(buf2.toDouble(), tValue);
}

TEST(dataBuffer, bufferMemory)
{
    std::string value;
    value.resize(1024, '\0');
    helicscpp::DataBuffer buf1(value.data(), 0, static_cast<int>(value.size()));
    EXPECT_EQ(buf1.capacity(), static_cast<int>(value.size()));
    EXPECT_EQ(buf1.size(), 0);
    buf1.fill(std::vector<double>{34.673, 19.1514, 1e-45});
    helicscpp::DataBuffer buf2(value.data(),
                               static_cast<int>(buf1.size()),
                               static_cast<int>(value.capacity()));

    EXPECT_EQ(buf2.type(), HELICS_DATA_TYPE_VECTOR);

    EXPECT_FALSE(buf1.reserve(2048));
    buf2.convertToType(HELICS_DATA_TYPE_STRING);
    // checking linkage here
    EXPECT_EQ(buf2.type(), HELICS_DATA_TYPE_STRING);
    EXPECT_EQ(buf1.type(), HELICS_DATA_TYPE_STRING);
}
