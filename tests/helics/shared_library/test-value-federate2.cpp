/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details.
*/

#include "ctestFixtures.hpp"

#include <future>
#include <gtest/gtest.h>
#include <iostream>

/** these test cases test out the value converters and some of the other functions
 */
class vfed2_simple_type_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {
};

class vfed2_type_tests: public ::testing::TestWithParam<const char*>, public FederateTestFixture {
};

class vfed2_tests: public FederateTestFixture, public ::testing::Test {
};
// const std::string core_types[] = { "test", "ipc", "zmq", "test_2", "ipc_2", "zmq_2" };

/** test block send and receive*/

TEST_P(vfed2_type_tests, block_send_receive)
{
    std::string s(500, ';');
    int len = static_cast<int>(s.size());
    char val[600] = "";
    int actualLen = 10;
    FederateTestFixture fixture;
    fixture.SetupTest(helicsCreateValueFederate, GetParam(), 1);
    auto vFed1 = fixture.GetFederateAt(0);
    auto pubid1 = helicsFederateRegisterTypePublication(vFed1, "pub1", "string", "", &err);
    EXPECT_TRUE(pubid1 != nullptr);
    auto pubid2 = helicsFederateRegisterGlobalTypePublication(vFed1, "pub2", "integer", "", &err);
    EXPECT_TRUE(pubid2 != nullptr);
    auto pubid3 = helicsFederateRegisterTypePublication(vFed1, "pub3", "", "", &err);
    EXPECT_TRUE(pubid3 != nullptr);
    auto sub1 = helicsFederateRegisterSubscription(vFed1, "fed0/pub3", "", &err);
    CE(helicsFederateSetTimeProperty(vFed1, helics_property_time_delta, 1.0, &err));

    CE(helicsFederateEnterExecutingMode(vFed1, &err));
    CE(helicsPublicationPublishRaw(pubid3, s.data(), len, &err));

    CE(helicsFederateRequestTime(vFed1, 1.0, &err));

    EXPECT_TRUE(helicsInputIsUpdated(sub1));

    int len1 = helicsInputGetRawValueSize(sub1);

    EXPECT_EQ(len1, len);
    CE(helicsInputGetRawValue(sub1, val, 600, &actualLen, &err));
    EXPECT_EQ(actualLen, len);

    len1 = helicsInputGetRawValueSize(sub1);

    EXPECT_EQ(len1, len);

    EXPECT_TRUE(helicsInputIsUpdated(sub1) == false);

    CE(helicsFederateFinalize(vFed1, &err));
}

TEST_P(vfed2_simple_type_tests, async_calls)
{
    helics_time gtime;
    helics_time f1time;
    // helics_federate_state state;
#define STRINGLEN 100
    char s[STRINGLEN] = "";
    FederateTestFixture fixture;
    fixture.SetupTest(helicsCreateValueFederate, GetParam(), 2);
    auto vFed1 = fixture.GetFederateAt(0);
    auto vFed2 = fixture.GetFederateAt(1);

    // register the publications
    auto pubid = helicsFederateRegisterGlobalPublication(
        vFed1, "pub1", helics_data_type_string, nullptr, &err);
    auto subid = helicsFederateRegisterSubscription(vFed2, "pub1", "", &err);
    CE(helicsFederateSetTimeProperty(vFed1, helics_property_time_delta, 1.0, &err));
    CE(helicsFederateSetTimeProperty(vFed2, helics_property_time_delta, 1.0, &err));

    CE(helicsFederateEnterExecutingModeAsync(vFed1, &err));
    CE(helicsFederateEnterExecutingModeAsync(vFed2, &err));
    CE(helicsFederateEnterExecutingModeComplete(vFed1, &err));
    CE(helicsFederateEnterExecutingModeComplete(vFed2, &err));

    // publish string1 at time=0.0;
    CE(helicsPublicationPublishString(pubid, "string1", &err));

    CE(helicsFederateRequestTimeAsync(vFed1, 1.0, &err));
    CE(f1time = helicsFederateRequestTimeComplete(vFed1, &err));
    CE(helicsFederateRequestTimeAsync(vFed2, 1.0, &err));
    CE(gtime = helicsFederateRequestTimeComplete(vFed2, &err));

    EXPECT_EQ(f1time, 1.0);
    EXPECT_EQ(gtime, 1.0);

    // get the value
    CE(helicsInputGetString(subid, s, STRINGLEN, nullptr, &err));

    // make sure the string is what we expect
    EXPECT_STREQ(s, "string1");

    // publish a second string
    CE(helicsPublicationPublishString(pubid, "string2", &err));

    // make sure the value is still what we expect
    CE(helicsInputGetString(subid, s, STRINGLEN, nullptr, &err));
    EXPECT_STREQ(s, "string1");

    // advance time
    CE(helicsFederateRequestTimeAsync(vFed1, 2.0, &err));
    CE(f1time = helicsFederateRequestTimeComplete(vFed1, &err));
    CE(helicsFederateRequestTimeAsync(vFed2, 2.0, &err));
    CE(gtime = helicsFederateRequestTimeComplete(vFed2, &err));

    EXPECT_EQ(f1time, 2.0);
    EXPECT_EQ(gtime, 2.0);

    // make sure the value was updated
    CE(helicsInputGetString(subid, s, STRINGLEN, nullptr, &err));
    EXPECT_STREQ(s, "string2");

    CE(helicsFederateFinalize(vFed1, &err));
    CE(helicsFederateFinalize(vFed2, &err));
}
//

//
TEST_F(vfed2_tests, file_load)
{
    helics_federate vFed;
    // fi = helicsCreateFederateInfo();
    // path of the JSON file is hardcoded for now
    vFed = helicsCreateValueFederateFromConfig(TEST_DIR "/example_value_fed.json", &err);
    EXPECT_EQ(err.error_code, helics_ok);
    ASSERT_FALSE(vFed == nullptr);
    const char* s = helicsFederateGetName(vFed);
    EXPECT_STREQ(s, "valueFed");
    EXPECT_EQ(helicsFederateGetInputCount(vFed), 3);
    EXPECT_EQ(helicsFederateGetPublicationCount(vFed), 2);
    //     helics::ValueFederate vFed(std::string(TEST_DIR) + "/test_files/example_value_fed.json");
    CE(helicsFederateFinalize(vFed, &err));
    //
    //     EXPECT_EQ(vFed.getName(), "fedName");

    //     EXPECT_EQ(vFed.getSubscriptionCount(), 2);
    //     EXPECT_EQ(vFed.getPublicationCount(), 2);
    helicsFederateFree(vFed);
}

TEST_F(vfed2_tests, json_publish)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);
    ASSERT_FALSE(vFed1 == nullptr);
    helicsFederateSetSeparator(vFed1, '/', nullptr);

    helicsFederateRegisterGlobalPublication(vFed1, "pub1", helics_data_type_double, "", nullptr);
    helicsFederateRegisterPublication(vFed1, "pub2", helics_data_type_string, "", nullptr);
    helicsFederateRegisterPublication(vFed1, "group1/pubA", helics_data_type_double, "", nullptr);
    helicsFederateRegisterPublication(vFed1, "group1/pubB", helics_data_type_string, "", nullptr);

    auto s1 = helicsFederateRegisterSubscription(vFed1, "pub1", nullptr, nullptr);
    auto s2 = helicsFederateRegisterSubscription(vFed1, "fed0/pub2", nullptr, nullptr);
    auto s3 = helicsFederateRegisterSubscription(vFed1, "fed0/group1/pubA", nullptr, nullptr);
    auto s4 = helicsFederateRegisterSubscription(vFed1, "fed0/group1/pubB", nullptr, nullptr);
    helicsFederateEnterExecutingMode(vFed1, nullptr);
    helicsFederatePublishJSON(vFed1,
                              (std::string(TEST_DIR) + "example_pub_input1.json").c_str(),
                              nullptr);
    helicsFederateRequestTime(vFed1, 1.0, nullptr);
    EXPECT_EQ(helicsInputGetDouble(s1, nullptr), 99.9);
    char buffer[50];
    int actLen = 0;
    helicsInputGetString(s2, buffer, 50, &actLen, nullptr);
    EXPECT_STREQ(buffer, "things");
    EXPECT_EQ(helicsInputGetDouble(s3, nullptr), 45.7);
    helicsInputGetString(s4, buffer, 50, &actLen, nullptr);
    EXPECT_STREQ(buffer, "count");

    helicsFederatePublishJSON(vFed1,
                              (std::string(TEST_DIR) + "example_pub_input2.json").c_str(),
                              nullptr);
    helicsFederateRequestTime(vFed1, 2.0, nullptr);
    EXPECT_EQ(helicsInputGetDouble(s1, nullptr), 88.2);

    helicsInputGetString(s2, buffer, 50, &actLen, nullptr);
    EXPECT_STREQ(buffer, "items");
    EXPECT_EQ(helicsInputGetDouble(s3, nullptr), 15.0);
    helicsInputGetString(s4, buffer, 50, &actLen, nullptr);
    EXPECT_STREQ(buffer, "count2");

    helicsFederatePublishJSON(vFed1, "{\"pub1\": 77.2}", nullptr);

    helicsFederateRequestTime(vFed1, 3.0, nullptr);
    EXPECT_EQ(helicsInputGetDouble(s1, nullptr), 77.2);

    CE(helicsFederateFinalize(vFed1, &err));
}

TEST_F(vfed2_tests, json_register_publish)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);
    ASSERT_FALSE(vFed1 == nullptr);
    helicsFederateSetSeparator(vFed1, '/', nullptr);

    CE(helicsFederateRegisterFromPublicationJSON(
        vFed1, (std::string(TEST_DIR) + "example_pub_input1.json").c_str(), &err));

    auto s1 = helicsFederateRegisterSubscription(vFed1, "fed0/pub1", nullptr, nullptr);
    auto s2 = helicsFederateRegisterSubscription(vFed1, "fed0/pub2", nullptr, nullptr);
    auto s3 = helicsFederateRegisterSubscription(vFed1, "fed0/group1/pubA", nullptr, nullptr);
    auto s4 = helicsFederateRegisterSubscription(vFed1, "fed0/group1/pubB", nullptr, nullptr);
    helicsFederateEnterExecutingMode(vFed1, nullptr);
    helicsFederatePublishJSON(vFed1,
                              (std::string(TEST_DIR) + "example_pub_input1.json").c_str(),
                              nullptr);
    helicsFederateRequestTime(vFed1, 1.0, nullptr);
    EXPECT_EQ(helicsInputGetDouble(s1, nullptr), 99.9);
    char buffer[50];
    int actLen = 0;
    helicsInputGetString(s2, buffer, 50, &actLen, nullptr);
    EXPECT_STREQ(buffer, "things");
    EXPECT_EQ(helicsInputGetDouble(s3, nullptr), 45.7);
    helicsInputGetString(s4, buffer, 50, &actLen, nullptr);
    EXPECT_STREQ(buffer, "count");

    helicsFederatePublishJSON(vFed1,
                              (std::string(TEST_DIR) + "example_pub_input2.json").c_str(),
                              nullptr);
    helicsFederateRequestTime(vFed1, 2.0, nullptr);
    EXPECT_EQ(helicsInputGetDouble(s1, nullptr), 88.2);

    helicsInputGetString(s2, buffer, 50, &actLen, nullptr);
    EXPECT_STREQ(buffer, "items");
    EXPECT_EQ(helicsInputGetDouble(s3, nullptr), 15.0);
    helicsInputGetString(s4, buffer, 50, &actLen, nullptr);
    EXPECT_STREQ(buffer, "count2");

    CE(helicsFederateFinalize(vFed1, &err));
}

INSTANTIATE_TEST_SUITE_P(vfed_tests,
                         vfed2_simple_type_tests,
                         ::testing::ValuesIn(core_types_simple));
INSTANTIATE_TEST_SUITE_P(vfed_tests, vfed2_type_tests, ::testing::ValuesIn(core_types));
