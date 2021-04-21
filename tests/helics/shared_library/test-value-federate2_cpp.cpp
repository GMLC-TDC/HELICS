/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../src/helics/cpp98/ValueFederate.hpp"
#include "cpptestFixtures.hpp"

#include "gtest/gtest.h"
#include <future>
#include <iostream>

/** these test cases test out the value converters and some of the other functions
 */

struct vfed_tests: public FederateTestFixture_cpp, public ::testing::Test {
};

class vfed_type_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture_cpp {
};

// const std::string core_types[] = { "test", "ipc", "zmq", "test_2", "ipc_2", "zmq_2" };

/** test block send and receive*/

TEST_P(vfed_type_tests, test_block_send_receive)
{
    helics_time gtime;
    std::string s(500, ';');
    int len = static_cast<int>(s.size());
    SCOPED_TRACE("calling setup");
    SetupTest<helicscpp::ValueFederate>(GetParam(), 1);
    SCOPED_TRACE("calling get federate");
    auto vFed1 = GetFederateAs<helicscpp::ValueFederate>(0);
    ASSERT_TRUE((vFed1));
    auto pubid1 = vFed1->registerPublication("pub1", "string", "");
    EXPECT_TRUE(pubid1.baseObject() != nullptr);
    auto pubid2 = vFed1->registerGlobalPublication("pub2", helics_data_type_int, "");
    EXPECT_TRUE(pubid2.baseObject() != nullptr);
    auto pubid3 = vFed1->registerPublication("pub3", "");
    EXPECT_TRUE(pubid3.baseObject() != nullptr);
    auto sub1 = vFed1->registerSubscription("fed0/pub3");
    SCOPED_TRACE("reg opt1");
    vFed1->setProperty(helics_property_time_delta, 1.0);
    SCOPED_TRACE("set Delta");
    vFed1->enterExecutingMode();
    SCOPED_TRACE("publish");
    pubid3.publish(s);
    SCOPED_TRACE("reqtime");
    gtime = vFed1->requestTime(1.0);
    EXPECT_EQ(gtime, 1.0);
    EXPECT_TRUE(sub1.isUpdated());

    int len1 = sub1.getRawValueSize();

    EXPECT_EQ(len1, len);
    std::vector<char> rawdata;
    int actualLen = sub1.getRawValue(rawdata);
    EXPECT_EQ(actualLen, len);

    len1 = sub1.getRawValueSize();

    EXPECT_EQ(len1, len);

    EXPECT_FALSE(sub1.isUpdated());

    vFed1->finalize();
}

TEST_P(vfed_type_tests, test_async_calls)
{
    helics_time gtime;
    helics_time f1time;
    // helics_federate_state state;
    SetupTest<helicscpp::ValueFederate>(GetParam(), 2);
    auto vFed1 = GetFederateAs<helicscpp::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helicscpp::ValueFederate>(1);

    // register the publications
    auto pubid = vFed1->registerGlobalPublication("pub1", helics_data_type_string, "");
    auto subid = vFed2->registerSubscription("pub1");
    vFed1->setProperty(helics_property_time_delta, 1.0);
    vFed2->setProperty(helics_property_time_delta, 1.0);

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
    auto s = subid.getString();

    // make sure the string is what we expect
    EXPECT_EQ(s, "string1");

    // publish a second string
    pubid.publish("string2");

    // make sure the value is still what we expect
    s = subid.getString();
    EXPECT_EQ(s, "string1");

    // advance time
    vFed1->requestTimeAsync(2.0);
    vFed2->requestTimeAsync(2.0);
    f1time = vFed1->requestTimeComplete();
    gtime = vFed2->requestTimeComplete();

    EXPECT_EQ(f1time, 2.0);
    EXPECT_EQ(gtime, 2.0);

    // make sure the value was updated
    s = subid.getString();
    EXPECT_EQ(s, "string2");

    vFed1->finalize();
    vFed2->finalize();
}
//
INSTANTIATE_TEST_SUITE_P(vfed_tests, vfed_type_tests, ::testing::ValuesIn(core_types));

//
TEST_F(vfed_tests, test_file_load)
{
    // fi = helicsCreateFederateInfo();
    // path of the JSON file is hardcoded for now
    helicscpp::ValueFederate vFed(TEST_DIR "/example_value_fed.json");
    ASSERT_TRUE(vFed.baseObject() != nullptr);
    std::string s = vFed.getName();
    EXPECT_EQ(s, "valueFed");
    EXPECT_EQ(vFed.getInputCount(), 3);
    EXPECT_EQ(vFed.getPublicationCount(), 2);
    //  helicscpp::ValueFederate vFed(std::string(TEST_DIR) +
    //  "/test_files/example_value_fed.json");
    vFed.finalize();
}

TEST_F(vfed_tests, test_json_register_publish)
{
    SetupTest<helicscpp::ValueFederate>("test", 1);
    auto vFed = GetFederateAs<helicscpp::ValueFederate>(0);
    vFed->setSeparator('/');
    vFed->registerFromPublicationJSON(std::string(TEST_DIR) + "example_pub_input1.json");
    auto s1 = vFed->registerSubscription("fed0/pub1");
    auto s2 = vFed->registerSubscription("fed0/pub2");
    auto s3 = vFed->registerSubscription("fed0/group1/pubA");
    auto s4 = vFed->registerSubscription("fed0/group1/pubB");
    vFed->enterExecutingMode();

    vFed->publishJSON(std::string(TEST_DIR) + "example_pub_input1.json");
    vFed->requestTime(1.0);
    EXPECT_EQ(s1.getDouble(), 99.9);
    EXPECT_EQ(s2.getString(), "things");
    EXPECT_EQ(s3.getDouble(), 45.7);
    EXPECT_EQ(s4.getString(), "count");

    vFed->publishJSON(std::string(TEST_DIR) + "example_pub_input2.json");
    vFed->requestTime(2.0);
    EXPECT_EQ(s1.getDouble(), 88.2);
    EXPECT_EQ(s2.getString(), "items");
    EXPECT_EQ(s3.getDouble(), 15.0);
    EXPECT_EQ(s4.getString(), "count2");

    vFed->finalize();
}
