/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../src/helics/cpp98/ValueFederate.hpp"
#include "cpptestFixtures.hpp"

#include "gtest/gtest.h"
#include <future>
#include <iostream>
#include <string>
#include <vector>

static const auto testNamer = [](const ::testing::TestParamInfo<const char*>& parameter) {
    return std::string(parameter.param);
};

struct VfedTests: public FederateTestFixture_cpp, public ::testing::Test {};

class VFedTypeTests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture_cpp {};

// const std::string CoreTypes[] = { "test", "ipc", "zmq", "test_2", "ipc_2", "zmq_2" };

/** test block send and receive*/

TEST_P(VFedTypeTests, test_block_send_receive)
{
    HelicsTime gtime;
    std::string value(500, ';');
    int len = static_cast<int>(value.size());
    SCOPED_TRACE("calling setup");
    SetupTest<helicscpp::ValueFederate>(GetParam(), 1);
    SCOPED_TRACE("calling get federate");
    auto vFed1 = GetFederateAs<helicscpp::ValueFederate>(0);
    ASSERT_TRUE((vFed1));
    auto pubid1 = vFed1->registerPublication("pub1", "string", "");
    EXPECT_TRUE(pubid1.baseObject() != nullptr);
    auto pubid2 = vFed1->registerGlobalPublication("pub2", HELICS_DATA_TYPE_INT, "");
    EXPECT_TRUE(pubid2.baseObject() != nullptr);
    auto pubid3 = vFed1->registerPublication("pub3", "");
    EXPECT_TRUE(pubid3.baseObject() != nullptr);
    auto sub1 = vFed1->registerSubscription("fed0/pub3");
    SCOPED_TRACE("reg opt1");
    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    SCOPED_TRACE("set Delta");
    vFed1->enterExecutingMode();
    SCOPED_TRACE("publish");
    pubid3.publish(value);
    SCOPED_TRACE("reqtime");
    gtime = vFed1->requestTime(1.0);
    EXPECT_EQ(gtime, 1.0);
    EXPECT_TRUE(sub1.isUpdated());

    int len1 = sub1.getStringSize();

    EXPECT_EQ(len1, len);
    std::vector<char> rawdata;
    int actualLen = sub1.getBytes(rawdata);
    // raw value has an extra 8 bits
    EXPECT_EQ(actualLen, len + 8);

    len1 = sub1.getByteCount();

    EXPECT_EQ(len1, len + 8);

    EXPECT_FALSE(sub1.isUpdated());

    vFed1->finalize();
}

TEST_P(VFedTypeTests, test_async_calls)
{
    HelicsTime gtime;
    HelicsTime f1time;
    // HelicsFederateState state;
    SetupTest<helicscpp::ValueFederate>(GetParam(), 2);
    auto vFed1 = GetFederateAs<helicscpp::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helicscpp::ValueFederate>(1);

    // register the publications
    auto pubid = vFed1->registerGlobalPublication("pub1", HELICS_DATA_TYPE_STRING, "");
    auto subid = vFed2->registerSubscription("pub1");
    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    vFed2->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);

    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingModeAsync();
    vFed1->enterExecutingModeComplete();
    vFed2->enterExecutingModeComplete();

    // publish string1 at time=0.0;
    pubid.publish("string1");

    vFed1->requestTimeAsync(1.0);
    vFed2->requestTimeAsync(1.0);
    f1time = vFed1->requestTimeComplete();
    gtime = vFed2->requestTimeComplete();

    EXPECT_EQ(f1time, 1.0);
    EXPECT_EQ(gtime, 1.0);

    gtime = vFed1->getCurrentTime();
    EXPECT_EQ(gtime, 1.0);
    // get the value
    auto value = subid.getString();

    // make sure the string is what we expect
    EXPECT_EQ(value, "string1");

    // publish a second string
    pubid.publish("string2");

    // make sure the value is still what we expect
    value = subid.getString();
    EXPECT_EQ(value, "string1");

    // advance time
    vFed1->requestTimeAsync(2.0);
    vFed2->requestTimeAsync(2.0);
    f1time = vFed1->requestTimeComplete();
    gtime = vFed2->requestTimeComplete();

    EXPECT_EQ(f1time, 2.0);
    EXPECT_EQ(gtime, 2.0);

    // make sure the value was updated
    value = subid.getString();
    EXPECT_EQ(value, "string2");

    vFed1->finalize();
    vFed2->finalize();
}
//
INSTANTIATE_TEST_SUITE_P(VfedTests, VFedTypeTests, ::testing::ValuesIn(CoreTypes), testNamer);

//
TEST_F(VfedTests, test_file_load)
{
    // fedInfo = helicsCreateFederateInfo();
    // path of the JSON file is hardcoded for now
    helicscpp::ValueFederate vFed(TEST_DIR "/example_value_fed.json");
    ASSERT_TRUE(vFed.baseObject() != nullptr);
    std::string fedName = vFed.getName();
    EXPECT_EQ(fedName, "valueFed");
    EXPECT_EQ(vFed.getInputCount(), 3);
    EXPECT_EQ(vFed.getPublicationCount(), 2);
    //  helicscpp::ValueFederate vFed(std::string(TEST_DIR) +
    //  "/test_files/example_value_fed.json");
    vFed.finalize();
}

TEST_F(VfedTests, json_register_publish)
{
    SetupTest<helicscpp::ValueFederate>("test", 1);
    auto vFed = GetFederateAs<helicscpp::ValueFederate>(0);
    vFed->setSeparator('/');
    vFed->registerFromPublicationJSON(std::string(TEST_DIR) + "example_pub_input1.json");
    auto sub1 = vFed->registerSubscription("fed0/pub1");
    auto sub2 = vFed->registerSubscription("fed0/pub2");
    auto sub3 = vFed->registerSubscription("fed0/group1/pubA");
    auto sub4 = vFed->registerSubscription("fed0/group1/pubB");
    vFed->enterExecutingMode();

    vFed->publishJSON(std::string(TEST_DIR) + "example_pub_input1.json");
    vFed->requestTime(1.0);
    EXPECT_EQ(sub1.getDouble(), 99.9);
    EXPECT_EQ(sub2.getString(), "things");
    EXPECT_EQ(sub3.getDouble(), 45.7);
    EXPECT_EQ(sub4.getString(), "count");

    vFed->publishJSON(std::string(TEST_DIR) + "example_pub_input2.json");
    vFed->requestTime(2.0);
    EXPECT_EQ(sub1.getDouble(), 88.2);
    EXPECT_EQ(sub2.getString(), "items");
    EXPECT_EQ(sub3.getDouble(), 15.0);
    EXPECT_EQ(sub4.getString(), "count2");

    vFed->finalize();
}

TEST_F(VfedTests, data_buffer)
{
    HelicsTime gtime;
    double val1 = 0;
    const double testValue1{4.565};
    const double testValue2{-2624.262};
    SetupTest<helicscpp::ValueFederate>("test", 1, 1.0);
    auto vFed = GetFederateAs<helicscpp::ValueFederate>(0);
    // register the publications
    auto pubid = vFed->registerGlobalPublication("pub1", HELICS_DATA_TYPE_DOUBLE, "");
    auto subid = vFed->registerSubscription("pub1", "");

    vFed->enterExecutingMode();

    pubid.publish(testValue1);

    gtime = vFed->requestTime(1.0);
    EXPECT_EQ(gtime, 1.0);

    // get the value
    val1 = subid.getDouble();
    // make sure the string is what we expect
    EXPECT_EQ(val1, testValue1);
    {
        // publish a second value
        helicscpp::DataBuffer buf1(20);
        buf1.fill(testValue2);
        pubid.publish(buf1);
    }
    val1 = subid.getDouble();

    EXPECT_EQ(val1, testValue1);

    {
        auto buffer = subid.getDataBuffer();
        EXPECT_TRUE(buffer.isValid());
        EXPECT_EQ(buffer.type(), HELICS_DATA_TYPE_DOUBLE);
        EXPECT_EQ(buffer.toDouble(), testValue1);
    }
    // advance time
    gtime = vFed->requestTime(2.0);
    // make sure the value was updated
    EXPECT_EQ(gtime, 2.0);

    val1 = subid.getDouble();
    EXPECT_EQ(val1, testValue2);

    vFed->finalize();
}
