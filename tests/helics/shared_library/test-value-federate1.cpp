/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "ctestFixtures.hpp"

#include <future>
#include <gtest/gtest.h>
#include <iostream>

/** these test cases test out the value federates
 */

class vfed_simple_type_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {
};

class vfed_type_tests: public ::testing::TestWithParam<const char*>, public FederateTestFixture {
};

class vfed_single_tests: public ::testing::Test, public FederateTestFixture {
};

/** test simple creation and destruction*/
TEST_P(vfed_simple_type_tests, initialize_tests)
{
    SetupTest(helicsCreateValueFederate, GetParam(), 1);
    auto vFed1 = GetFederateAt(0);
    ASSERT_FALSE(vFed1 == nullptr);
    CE(helicsFederateEnterExecutingMode(vFed1, &err));

    helics_federate_state state;
    CE(state = helicsFederateGetState(vFed1, &err));
    EXPECT_TRUE(state == helics_state_execution);

    CE(helicsFederateFinalize(vFed1, &err));

    CE(state = helicsFederateGetState(vFed1, &err));
    EXPECT_TRUE(state == helics_state_finalize);
}

TEST_F(vfed_single_tests, publication_registration)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);

    auto pubid =
        helicsFederateRegisterPublication(vFed1, "pub1", helics_data_type_string, "", &err);
    auto pubid2 = helicsFederateRegisterGlobalTypePublication(vFed1, "pub2", "int", "", &err);

    auto pubid3 =
        helicsFederateRegisterPublication(vFed1, "pub3", helics_data_type_double, "V", &err);
    CE(helicsFederateEnterExecutingMode(vFed1, &err));

    helics_federate_state state;
    CE(state = helicsFederateGetState(vFed1, &err));
    EXPECT_TRUE(state == helics_state_execution);

    auto sv = helicsPublicationGetKey(pubid);
    EXPECT_STREQ(sv, "fed0/pub1");
    auto sv2 = helicsPublicationGetKey(pubid2);
    EXPECT_STREQ(sv2, "pub2");
    auto pub3name = helicsPublicationGetKey(pubid3);
    EXPECT_STREQ(pub3name, "fed0/pub3");

    auto type = helicsPublicationGetType(pubid3);
    EXPECT_STREQ(type, "double");
    const char* units = helicsPublicationGetUnits(pubid3);
    EXPECT_STREQ(units, "V");

    // EXPECT_TRUE (vFed1->getPublicationId ("pub1") == pubid);
    // EXPECT_TRUE (vFed1->getPublicationId ("pub2") == pubid2);
    // EXPECT_TRUE (vFed1->getPublicationId ("fed0/pub1") == pubid);
    CE(helicsFederateFinalize(vFed1, &err));

    CE(state = helicsFederateGetState(vFed1, &err));
    EXPECT_TRUE(state == helics_state_finalize);
}

TEST_F(vfed_single_tests, publisher_registration)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);

    auto pubid =
        helicsFederateRegisterPublication(vFed1, "pub1", helics_data_type_string, "", &err);
    auto pubid2 =
        helicsFederateRegisterGlobalPublication(vFed1, "pub2", helics_data_type_int, "", &err);
    auto pubid3 =
        helicsFederateRegisterPublication(vFed1, "pub3", helics_data_type_double, "V", &err);
    CE(helicsFederateEnterExecutingMode(vFed1, &err));

    helics_federate_state state;
    CE(state = helicsFederateGetState(vFed1, &err));
    EXPECT_TRUE(state == helics_state_execution);

    auto sv = helicsPublicationGetKey(pubid);
    auto sv2 = helicsPublicationGetKey(pubid2);
    EXPECT_STREQ(sv, "fed0/pub1");
    EXPECT_STREQ(sv2, "pub2");
    auto pub3name = helicsPublicationGetKey(pubid3);
    EXPECT_STREQ(pub3name, "fed0/pub3");

    auto type = helicsPublicationGetType(pubid3);
    EXPECT_STREQ(type, "double");
    auto units = helicsPublicationGetUnits(pubid3);
    EXPECT_STREQ(units, "V");

    // EXPECT_TRUE (vFed1->getPublicationId ("pub1") == pubid.getID ());
    // EXPECT_TRUE (vFed1->getPublicationId ("pub2") == pubid2.getID ());
    // EXPECT_TRUE (vFed1->getPublicationId ("fed0/pub1") == pubid.getID ());
    CE(helicsFederateFinalize(vFed1, &err));

    CE(state = helicsFederateGetState(vFed1, &err));
    EXPECT_TRUE(state == helics_state_finalize);
}

TEST_F(vfed_single_tests, subscription_registration)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);

    helicsFederateSetFlagOption(vFed1, helics_handle_option_connection_optional, helics_true, &err);
    auto subid = helicsFederateRegisterSubscription(vFed1, "sub1", "V", &err);
    auto subid2 = helicsFederateRegisterSubscription(vFed1, "sub2", "", &err);

    auto subid3 = helicsFederateRegisterSubscription(vFed1, "sub3", "V", &err);
    CE(helicsFederateEnterExecutingMode(vFed1, &err));

    helics_federate_state state;
    CE(state = helicsFederateGetState(vFed1, &err));
    EXPECT_TRUE(state == helics_state_execution);

    auto sv = helicsSubscriptionGetKey(subid);
    auto sv2 = helicsSubscriptionGetKey(subid2);
    EXPECT_STREQ(sv, "sub1");
    EXPECT_STREQ(sv2, "sub2");

    auto sub3name = helicsSubscriptionGetKey(subid3);

    // vFed1->addSubscriptionShortcut (subid, "Shortcut");
    EXPECT_STREQ(sub3name, "sub3");

    auto units = helicsInputGetUnits(subid3);
    EXPECT_STREQ(units, "V");

    // EXPECT_TRUE (vFed1->getSubscriptionId ("sub1") == subid);
    // EXPECT_TRUE (vFed1->getSubscriptionId ("sub2") == subid2);

    // EXPECT_TRUE (vFed1->getSubscriptionId ("Shortcut") == subid);

    CE(helicsFederateFinalize(vFed1, &err));

    CE(state = helicsFederateGetState(vFed1, &err));
    EXPECT_TRUE(state == helics_state_finalize);
}

TEST_F(vfed_single_tests, subscription_and_publication_registration)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);
    helicsFederateSetFlagOption(vFed1, helics_handle_option_connection_optional, helics_true, &err);
    // register the publications
    auto pubid =
        helicsFederateRegisterPublication(vFed1, "pub1", helics_data_type_string, "", &err);
    auto pubid2 =
        helicsFederateRegisterGlobalPublication(vFed1, "pub2", helics_data_type_int, "volts", &err);

    auto pubid3 = helicsFederateRegisterTypePublication(vFed1, "pub3", "double", "V", &err);

    auto subid = helicsFederateRegisterSubscription(vFed1, "sub1", "V", &err);
    auto subid2 = helicsFederateRegisterSubscription(vFed1, "sub2", "", &err);

    auto subid3 = helicsFederateRegisterSubscription(vFed1, "sub3", "V", &err);
    // enter execution
    CE(helicsFederateEnterExecutingMode(vFed1, &err));

    helics_federate_state state;
    CE(state = helicsFederateGetState(vFed1, &err));
    EXPECT_TRUE(state == helics_state_execution);

    auto sv = helicsSubscriptionGetKey(subid);
    auto sv2 = helicsSubscriptionGetKey(subid2);

    EXPECT_STREQ(sv, "sub1");
    EXPECT_STREQ(sv2, "sub2");
    auto sub3name = helicsSubscriptionGetKey(subid3);
    EXPECT_STREQ(sub3name, "sub3");

    auto units = helicsInputGetUnits(subid3);
    EXPECT_STREQ(units, "V");

    // check the getSubscription function
    auto subid_b = helicsFederateGetSubscription(vFed1, "sub1", &err);
    const char* tmp = helicsSubscriptionGetKey(subid_b);
    EXPECT_STREQ(tmp, "sub1");
    // check the getSubscriptionByIndex function
    auto subid_c = helicsFederateGetInputByIndex(vFed1, 2, &err);
    tmp = helicsInputGetUnits(subid_c);
    EXPECT_STREQ(tmp, "V");
    // check publications

    sv = helicsPublicationGetKey(pubid);
    sv2 = helicsPublicationGetKey(pubid2);
    EXPECT_STREQ(sv, "fed0/pub1");
    EXPECT_STREQ(sv2, "pub2");
    auto pub3name = helicsPublicationGetKey(pubid3);
    EXPECT_STREQ(pub3name, "fed0/pub3");

    const char* type = helicsPublicationGetType(pubid3);
    EXPECT_STREQ(type, "double");
    units = helicsPublicationGetUnits(pubid3);
    EXPECT_STREQ(units, "V");

    // check the getSubscription function

    auto pubid_b = helicsFederateGetPublication(vFed1, "fed0/pub1", &err);
    tmp = helicsPublicationGetType(pubid_b);
    EXPECT_STREQ(tmp, "string");
    // check the getSubscriptionByIndex function
    auto pubid_c = helicsFederateGetPublicationByIndex(vFed1, 1, &err);
    tmp = helicsPublicationGetUnits(pubid_c);
    EXPECT_STREQ(tmp, "volts");

    // this one should be invalid
    auto pubid_d = helicsFederateGetPublicationByIndex(vFed1, 5, &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(pubid_d, nullptr);
    helicsErrorClear(&err);

    CE(helicsFederateFinalize(vFed1, &err));

    CE(state = helicsFederateGetState(vFed1, &err));
    EXPECT_TRUE(state == helics_state_finalize);
}

TEST_F(vfed_single_tests, default_value_tests)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);

    auto inp_raw1 = helicsFederateRegisterInput(vFed1, "key1", helics_data_type_raw, "raw", &err);
    auto inp_raw2 = helicsFederateRegisterInput(vFed1, "key2", helics_data_type_raw, "raw", &err);

    auto inp_bool = helicsFederateRegisterInput(vFed1, "key3", helics_data_type_boolean, "", &err);

    auto inp_time = helicsFederateRegisterInput(vFed1, "key4", helics_data_type_time, "", &err);

    auto inp_char = helicsFederateRegisterInput(vFed1, "key5", helics_data_type_char, "", &err);

    auto inp_vect = helicsFederateRegisterInput(vFed1, "key6", helics_data_type_vector, "V", &err);

    auto inp_double =
        helicsFederateRegisterInput(vFed1, "key7", helics_data_type_double, "kW", &err);

    auto inp_double2 =
        helicsFederateRegisterInput(vFed1, "key8", helics_data_type_double, "", &err);

    auto inp_np =
        helicsFederateRegisterInput(vFed1, "key9", helics_data_type_named_point, "", &err);

    helicsInputSetMinimumChange(inp_double, 1100.0, &err);
    helicsInputSetDefaultDouble(inp_double, 10000.0, &err);

    helicsInputSetOption(inp_double2, helics_handle_option_connection_required, helics_true, &err);
    EXPECT_EQ(helicsInputIsValid(inp_double2), helics_true);
    // anonymous publication
    auto pub = helicsFederateRegisterPublication(vFed1, nullptr, helics_data_type_int, "MW", &err);
    helicsPublicationSetOption(pub, helics_handle_option_connection_required, helics_true, &err);
    helicsPublicationAddTarget(pub, "fed0/key7", &err);
    helicsPublicationAddTarget(pub, "fed0/key8", &err);
    EXPECT_EQ(helicsPublicationIsValid(pub), helics_true);

    helicsInputSetDefaultRaw(inp_raw1, nullptr, -2, &err);
    EXPECT_EQ(err.error_code, 0);
    char data[256] = "this is a string";
    helicsInputSetDefaultRaw(inp_raw2, data, 30, &err);

    helicsInputSetDefaultBoolean(inp_bool, helics_true, &err);

    helicsInputSetDefaultTime(inp_time, 12.3, &err);
    helicsInputSetDefaultChar(inp_char, 'q', &err);
    // this should be ok since the data is NULL regardless of specified length
    helicsInputSetDefaultVector(inp_vect, nullptr, 7, &err);
    helicsInputSetDefaultNamedPoint(inp_np, data, 15.7, &err);

    helicsFederateEnterExecutingMode(vFed1, &err);
    EXPECT_EQ(err.error_code, 0);
    EXPECT_STREQ(helicsInputGetInjectionUnits(inp_double), "MW");
    EXPECT_STREQ(helicsInputGetInjectionUnits(inp_double2), "MW");
    EXPECT_STREQ(helicsInputGetType(inp_double), "double");
    EXPECT_STREQ(helicsInputGetPublicationType(inp_double), "int64");

    auto c2 = helicsInputGetChar(inp_char, &err);
    EXPECT_EQ(c2, 'q');
    int actSize = 56;
    // this should not be an error
    helicsInputGetVector(inp_vect, nullptr, 5, &actSize, &err);
    EXPECT_EQ(err.error_code, 0);
    EXPECT_EQ(actSize, 0);

    auto optset = helicsInputGetOption(inp_double2, helics_handle_option_connection_required);
    EXPECT_EQ(optset, helics_true);

    optset = helicsPublicationGetOption(pub, helics_handle_option_connection_required);
    EXPECT_EQ(optset, helics_true);
    helicsPublicationPublishInteger(pub, 12, &err);

    helicsFederateRequestNextStep(vFed1, &err);
    EXPECT_DOUBLE_EQ(helicsInputGetDouble(inp_double, &err), 12000.0);
    EXPECT_DOUBLE_EQ(helicsInputGetDouble(inp_double2, &err), 12.0);

    helicsPublicationPublishInteger(pub, 13, &err);

    helicsFederateRequestNextStep(vFed1, &err);
    EXPECT_EQ(helicsInputIsUpdated(inp_double), helics_false);
    EXPECT_EQ(helicsInputIsUpdated(inp_double2), helics_true);

    EXPECT_DOUBLE_EQ(helicsInputGetDouble(inp_double, &err), 12000.0);
    EXPECT_DOUBLE_EQ(helicsInputGetDouble(inp_double2, &err), 13.0);

    helicsPublicationPublishInteger(pub, 15, &err);

    helicsFederateRequestNextStep(vFed1, &err);

    EXPECT_EQ(helicsInputIsUpdated(inp_double), helics_true);
    EXPECT_EQ(helicsInputIsUpdated(inp_double2), helics_true);

    helicsInputClearUpdate(inp_double);
    helicsInputClearUpdate(inp_double2);

    EXPECT_EQ(helicsInputIsUpdated(inp_double), helics_false);
    EXPECT_EQ(helicsInputIsUpdated(inp_double2), helics_false);

    char out[8] = "";
    int actLen = 66;
    double rval = 19.0;
    helicsInputGetNamedPoint(inp_np, nullptr, 5, &actLen, &rval, &err);
    EXPECT_EQ(err.error_code, 0);
    EXPECT_EQ(actLen, 0);
    EXPECT_DOUBLE_EQ(rval, 15.7);
    rval = 19.0;
    helicsInputGetNamedPoint(inp_np, out, 8, &actLen, &rval, &err);
    EXPECT_EQ(err.error_code, 0);
    EXPECT_STREQ(out, "this is");
    EXPECT_EQ(actLen, 8);
    EXPECT_DOUBLE_EQ(rval, 15.7);

    helicsFederateFinalize(vFed1, &err);
}
TEST_P(vfed_type_tests, single_transfer)
{
    // helics_time stime = 1.0;
    helics_time gtime;
#define STRINGLEN 100
    char s[STRINGLEN] = "n2";

    SetupTest(helicsCreateValueFederate, GetParam(), 1, 1.0);
    auto vFed = GetFederateAt(0);

    // register the publications
    auto pubid =
        helicsFederateRegisterGlobalPublication(vFed, "pub1", helics_data_type_string, "", &err);
    auto subid = helicsFederateRegisterSubscription(vFed, "pub1", nullptr, &err);

    CE(helicsFederateEnterExecutingMode(vFed, &err));

    CE(helicsPublicationPublishString(pubid, "string1", &err));

    CE(gtime = helicsFederateRequestTime(vFed, 1.0, &err));
    EXPECT_EQ(gtime, 1.0);

    // get the value
    CE(helicsInputGetString(subid, s, STRINGLEN, nullptr, &err));

    // make sure the string is what we expect
    EXPECT_STREQ(s, "string1");
    // check the time
    auto time = helicsInputLastUpdateTime(subid);
    EXPECT_EQ(time, 1.0);
    // publish a second string
    CE(helicsPublicationPublishString(pubid, "string2", &err));

    int actualLen;
    // make sure the value is still what we expect
    CE(helicsInputGetRawValue(subid, s, STRINGLEN, &actualLen, &err));
    EXPECT_EQ(std::string(s, actualLen), "string1");

    // advance time
    CE(gtime = helicsFederateRequestTime(vFed, 2.0, &err));

    // make sure the value was updated
    EXPECT_EQ(gtime, 2.0);

    // make sure the string is what we expect
    CE(helicsInputGetRawValue(subid, s, STRINGLEN, &actualLen, &err));

    EXPECT_EQ(actualLen, 7);
    EXPECT_EQ(std::string(s, actualLen), "string2");

    CE(helicsFederateFinalize(vFed, &err));
}

// template <class X>
void runFederateTestDouble(const char* core,
                           double defaultValue,
                           double testValue1,
                           double testValue2)
{
    helics_time gtime;
    double val1 = 0;
    double* val = &val1;
    helics_error err = helicsErrorInitialize();
    FederateTestFixture fixture;
    fixture.SetupTest(helicsCreateValueFederate, core, 1, 1.0);
    auto vFed = fixture.GetFederateAt(0);
    // register the publications
    auto pubid =
        helicsFederateRegisterGlobalPublication(vFed, "pub1", helics_data_type_double, "", nullptr);
    auto subid = helicsFederateRegisterSubscription(vFed, "pub1", "", nullptr);
    CE(helicsInputSetDefaultDouble(subid, defaultValue, &err));

    CE(helicsFederateEnterExecutingMode(vFed, &err));

    // publish string1 at time=0.0;
    CE(helicsPublicationPublishDouble(pubid, testValue1, &err));

    CE(*val = helicsInputGetDouble(subid, &err));
    EXPECT_EQ(*val, defaultValue);

    CE(gtime = helicsFederateRequestTime(vFed, 1.0, &err));
    EXPECT_EQ(gtime, 1.0);

    // get the value
    CE(*val = helicsInputGetDouble(subid, &err));
    // make sure the string is what we expect
    EXPECT_EQ(*val, testValue1);

    // publish a second string
    CE(helicsPublicationPublishDouble(pubid, testValue2, &err));

    // make sure the value is still what we expect
    CE(*val = helicsInputGetDouble(subid, &err));
    EXPECT_EQ(*val, testValue1);
    // advance time
    CE(gtime = helicsFederateRequestTime(vFed, 2.0, &err));
    // make sure the value was updated
    EXPECT_EQ(gtime, 2.0);

    CE(*val = helicsInputGetDouble(subid, &err));
    EXPECT_EQ(*val, testValue2);

    CE(helicsFederateFinalize(vFed, &err));
}

void runFederateTestComplex(const char* core,
                            double defaultValue_r,
                            double defaultValue_i,
                            double testValue1_r,
                            double testValue1_i,
                            double testValue2_r,
                            double testValue2_i)
{
    helics_time gtime;
    double val1_r = 0.0;
    double val1_i = 0.0;
    helics_error err = helicsErrorInitialize();
    FederateTestFixture fixture;
    fixture.SetupTest(helicsCreateValueFederate, core, 1, 1.0);
    auto vFed = fixture.GetFederateAt(0);

    // register the publications
    auto pubid =
        helicsFederateRegisterGlobalPublication(vFed, "pub1", helics_data_type_complex, "", &err);
    auto subid = helicsFederateRegisterSubscription(vFed, "pub1", "", &err);
    CE(helicsInputSetDefaultComplex(subid, defaultValue_r, defaultValue_i, &err));

    CE(helicsFederateEnterExecutingMode(vFed, &err));

    // publish string1 at time=0.0;
    CE(helicsPublicationPublishComplex(pubid, testValue1_r, testValue1_i, &err));

    CE(helicsInputGetComplex(subid, &val1_r, &val1_i, &err));
    EXPECT_EQ(val1_r, defaultValue_r);
    EXPECT_EQ(val1_i, defaultValue_i);

    CE(gtime = helicsFederateRequestTime(vFed, 1.0, &err));
    EXPECT_EQ(gtime, 1.0);

    // get the value
    CE(helics_complex hc = helicsInputGetComplexObject(subid, &err));
    // make sure the string is what we expect
    EXPECT_EQ(hc.real, testValue1_r);
    EXPECT_EQ(hc.imag, testValue1_i);

    // publish a second value
    CE(helicsPublicationPublishComplex(pubid, testValue2_r, testValue2_i, &err));

    // make sure the value is still what we expect
    CE(helicsInputGetComplex(subid, &val1_r, &val1_i, &err));
    EXPECT_EQ(val1_r, testValue1_r);
    EXPECT_EQ(val1_i, testValue1_i);
    // advance time
    CE(gtime = helicsFederateRequestTimeAdvance(vFed, 1.0, &err));
    // make sure the value was updated
    EXPECT_EQ(gtime, 2.0);

    CE(hc = helicsInputGetComplexObject(subid, &err));
    EXPECT_EQ(hc.real, testValue2_r);
    EXPECT_EQ(hc.imag, testValue2_i);

    CE(helicsFederateFinalize(vFed, &err));
}

void runFederateTestComplex2(const char* core,
                             double defaultValue_r,
                             double defaultValue_i,
                             double testValue1_r,
                             double testValue1_i,
                             double testValue2_r,
                             double testValue2_i)
{
    helics_time gtime;
    double val1_r = 0.0;
    double val1_i = 0.0;
    helics_error err = helicsErrorInitialize();
    FederateTestFixture fixture;
    fixture.SetupTest(helicsCreateValueFederate, core, 1, 1.0);
    auto vFed = fixture.GetFederateAt(0);

    // register the publications
    auto pubid = helicsFederateRegisterGlobalTypePublication(vFed, "pub1", "complex", "", &err);
    auto subid = helicsFederateRegisterSubscription(vFed, "pub1", "complex", &err);
    CE(helicsInputSetDefaultComplex(subid, defaultValue_r, defaultValue_i, &err));

    CE(helicsFederateEnterExecutingMode(vFed, &err));

    // publish string1 at time=0.0;
    CE(helicsPublicationPublishComplex(pubid, testValue1_r, testValue1_i, &err));

    CE(helicsInputGetComplex(subid, &val1_r, &val1_i, &err));
    EXPECT_EQ(val1_r, defaultValue_r);
    EXPECT_EQ(val1_i, defaultValue_i);

    CE(gtime = helicsFederateRequestTime(vFed, 1.0, &err));
    EXPECT_EQ(gtime, 1.0);

    // get the value
    CE(helics_complex hc = helicsInputGetComplexObject(subid, &err));
    // make sure the string is what we expect
    EXPECT_EQ(hc.real, testValue1_r);
    EXPECT_EQ(hc.imag, testValue1_i);

    // publish a second value
    CE(helicsPublicationPublishComplex(pubid, testValue2_r, testValue2_i, &err));

    // make sure the value is still what we expect
    CE(helicsInputGetComplex(subid, &val1_r, &val1_i, &err));
    EXPECT_EQ(val1_r, testValue1_r);
    EXPECT_EQ(val1_i, testValue1_i);
    // advance time
    CE(gtime = helicsFederateRequestTimeAdvance(vFed, 1.0, &err));
    // make sure the value was updated
    EXPECT_EQ(gtime, 2.0);

    CE(hc = helicsInputGetComplexObject(subid, &err));
    EXPECT_EQ(hc.real, testValue2_r);
    EXPECT_EQ(hc.imag, testValue2_i);

    CE(helicsFederateFinalize(vFed, &err));
}

void runFederateTestInteger(const char* core,
                            int64_t defaultValue,
                            int64_t testValue1,
                            int64_t testValue2)
{
    helics_time gtime;
    int64_t val1 = 0;
    int64_t* val = &val1;

    FederateTestFixture fixture;
    fixture.SetupTest(helicsCreateValueFederate, core, 1, 1.0);
    auto vFed = fixture.GetFederateAt(0);
    helics_error err = helicsErrorInitialize();
    // register the publications
    auto pubid =
        helicsFederateRegisterGlobalPublication(vFed, "pub1", helics_data_type_int, "", &err);
    auto subid = helicsFederateRegisterSubscription(vFed, "pub1", "", &err);
    CE(helicsInputSetDefaultInteger(subid, defaultValue, &err));
    CE(helicsFederateEnterExecutingMode(vFed, &err));

    // publish string1 at time=0.0;
    CE(helicsPublicationPublishInteger(pubid, testValue1, &err));
    CE(*val = helicsInputGetInteger(subid, &err));

    EXPECT_EQ(*val, defaultValue);

    CE(gtime = helicsFederateRequestTime(vFed, 1.0, &err));
    EXPECT_EQ(gtime, 1.0);

    // get the value
    CE(*val = helicsInputGetInteger(subid, &err));
    // make sure the string is what we expect
    EXPECT_EQ(*val, testValue1);

    // publish a second string
    CE(helicsPublicationPublishInteger(pubid, testValue2, &err));

    // make sure the value is still what we expect
    CE(*val = helicsInputGetInteger(subid, &err));
    EXPECT_EQ(*val, testValue1);
    // advance time
    CE(gtime = helicsFederateRequestTime(vFed, 2.0, &err));
    // make sure the value was updated
    EXPECT_EQ(gtime, 2.0);

    CE(*val = helicsInputGetInteger(subid, &err));
    EXPECT_EQ(*val, testValue2);

    CE(helicsFederateFinalize(vFed, &err));
}

void runFederateTestBool(const char* core, bool defaultValue, bool testValue1, bool testValue2)
{
    helics_time gtime;
    helics_bool val1 = 0;
    helics_bool* val = &val1;

    FederateTestFixture fixture;
    helics_error err = helicsErrorInitialize();
    fixture.SetupTest(helicsCreateValueFederate, core, 1, 1.0);
    auto vFed = fixture.GetFederateAt(0);
    // register the publications
    auto pubid =
        helicsFederateRegisterGlobalPublication(vFed, "pub1", helics_data_type_boolean, "", &err);
    auto subid = helicsFederateRegisterSubscription(vFed, "pub1", "", &err);
    CE(helicsInputSetDefaultDouble(subid, defaultValue ? helics_true : helics_false, &err));
    CE(helicsFederateEnterExecutingMode(vFed, &err));

    // publish string1 at time=0.0;
    CE(helicsPublicationPublishBoolean(pubid, testValue1 ? helics_true : helics_false, &err));
    CE(*val = helicsInputGetBoolean(subid, &err));

    EXPECT_EQ(*val, defaultValue ? helics_true : helics_false);

    CE(gtime = helicsFederateRequestTime(vFed, 1.0, &err));
    EXPECT_EQ(gtime, 1.0);

    // get the value
    CE(*val = helicsInputGetBoolean(subid, &err));
    // make sure the string is what we expect
    EXPECT_EQ(*val, testValue1 ? helics_true : helics_false);

    // publish a second string
    CE(helicsPublicationPublishBoolean(pubid, testValue2 ? helics_true : helics_false, &err));

    // make sure the value is still what we expect
    CE(*val = helicsInputGetBoolean(subid, &err));
    EXPECT_EQ(*val, testValue1 ? helics_true : helics_false);
    // advance time
    CE(gtime = helicsFederateRequestTime(vFed, 2.0, &err));
    // make sure the value was updated
    EXPECT_EQ(gtime, 2.0);

    CE(*val = helicsInputGetBoolean(subid, &err));
    EXPECT_EQ(*val, testValue2 ? helics_true : helics_false);

    CE(helicsFederateFinalize(vFed, &err));
}

void runFederateTestString(const char* core,
                           const char* defaultValue,
                           const char* testValue1,
                           const char* testValue2)
{
    helics_time gtime;
#define STRINGSIZE 100
    char str[STRINGSIZE] = "";
    helics_error err = helicsErrorInitialize();
    FederateTestFixture fixture;
    fixture.SetupTest(helicsCreateValueFederate, core, 1, 1.0);
    auto vFed = fixture.GetFederateAt(0);
    // register the publications
    auto pubid =
        helicsFederateRegisterGlobalPublication(vFed, "pub1", helics_data_type_string, "", &err);
    auto subid = helicsFederateRegisterSubscription(vFed, "pub1", "", &err);
    CE(helicsInputSetDefaultString(subid, defaultValue, &err));

    CE(helicsFederateEnterExecutingMode(vFed, &err));

    // publish string1 at time=0.0;
    CE(helicsPublicationPublishString(pubid, testValue1, &err));

    CE(helicsInputGetString(subid, str, STRINGSIZE, nullptr, &err));

    EXPECT_STREQ(str, defaultValue);

    CE(gtime = helicsFederateRequestTime(vFed, 1.0, &err));

    EXPECT_EQ(gtime, 1.0);

    // get the value
    CE(helicsInputGetString(subid, str, STRINGSIZE, nullptr, &err));

    // make sure the string is what we expect
    EXPECT_STREQ(str, testValue1);

    // publish a second string
    CE(helicsPublicationPublishString(pubid, testValue2, &err));

    // make sure the value is still what we expect
    CE(helicsInputGetString(subid, str, STRINGSIZE, nullptr, &err));
    EXPECT_STREQ(str, testValue1);

    // advance time
    CE(gtime = helicsFederateRequestTime(vFed, 2.0, &err));
    // make sure the value was updated
    EXPECT_EQ(gtime, 2.0);

    CE(helicsInputGetString(subid, str, STRINGSIZE, nullptr, &err));
    EXPECT_STREQ(str, testValue2);

    CE(helicsFederateFinalize(vFed, &err));
}

void runFederateTestVectorD(const char* core,
                            const double defaultValue[],
                            const double testValue1[],
                            const double testValue2[],
                            int len,
                            int len1,
                            int len2)
{
    helics_time gtime;
    int maxlen = (len1 > len2) ? len1 : len2;
    maxlen = (maxlen > len) ? maxlen : len;
    auto* val = new double[maxlen];
    helics_error err = helicsErrorInitialize();
    FederateTestFixture fixture;
    fixture.SetupTest(helicsCreateValueFederate, core, 1, 1.0);
    auto vFed = fixture.GetFederateAt(0);
    // register the interfaces
    auto pubid =
        helicsFederateRegisterGlobalPublication(vFed, "pub1", helics_data_type_vector, "", &err);
    auto subid = helicsFederateRegisterSubscription(vFed, "pub1", nullptr, &err);
    CE(helicsInputSetDefaultVector(subid, defaultValue, len, &err));
    CE(helicsFederateEnterExecutingMode(vFed, &err));

    // publish string1 at time=0.0;
    CE(helicsPublicationPublishVector(pubid, testValue1, len1, &err));

    int actualLen = helicsInputGetVectorSize(subid);
    EXPECT_EQ(actualLen, len);

    CE(helicsInputGetVector(subid, val, maxlen, &actualLen, &err));

    EXPECT_EQ(actualLen, len);
    for (int i = 0; i < len; i++) {
        EXPECT_EQ(val[i], defaultValue[i]);
        // std::cout << defaultValue[i] << "\n";
    }

    CE(gtime = helicsFederateRequestTime(vFed, 1.0, &err));
    EXPECT_EQ(gtime, 1.0);

    // get the value

    CE(helicsInputGetVector(subid, val, maxlen, &actualLen, &err));
    EXPECT_EQ(actualLen, len1);
    // make sure the vector is what we expect
    for (int i = 0; i < len1; i++) {
        EXPECT_EQ(val[i], testValue1[i]);
        // std::cout << testValue1[i] << "\n";
    }

    // test getting a vector as a string
    actualLen = helicsInputGetStringSize(subid);
    std::string buf;
    buf.resize(static_cast<size_t>(actualLen) + 2);
    CE(helicsInputGetString(subid, &(buf[0]), static_cast<int>(buf.size()), &actualLen, &err));
    buf.resize(static_cast<size_t>(actualLen) - 1);
    EXPECT_EQ(buf[0], 'v');
    EXPECT_EQ(buf.back(), ']');

    // publish a second vector
    CE(helicsPublicationPublishVector(pubid, testValue2, len2, &err));

    // make sure the value is still what we expect
    CE(helicsInputGetVector(subid, val, maxlen, &actualLen, &err));
    EXPECT_EQ(actualLen, len1);
    for (int i = 0; i < len1; i++) {
        EXPECT_NEAR(val[i], testValue1[i], 0.0001);
        //  std::cout << testValue1[i] << "\n";
    }

    // advance time
    CE(gtime = helicsFederateRequestTime(vFed, 2.0, &err));
    // make sure the value was updated
    EXPECT_EQ(gtime, 2.0);

    CE(helicsInputGetVector(subid, val, maxlen, &actualLen, &err));

    EXPECT_EQ(actualLen, len2);
    for (int i = 0; i < len2; i++) {
        EXPECT_EQ(val[i], testValue2[i]);
        //  std::cout << testValue2[i] << "\n";
    }

    CE(helicsFederateFinalize(vFed, &err));
    delete[] val;
}

void runFederateTestNamedPoint(const char* core,
                               const char* defaultValue,
                               double defVal,
                               const char* testValue1,
                               double testVal1,
                               const char* testValue2,
                               double testVal2)
{
    helics_time gtime;
#define STRINGSIZE 100
    char str[STRINGSIZE] = "";

    FederateTestFixture fixture;
    helics_error err = helicsErrorInitialize();
    fixture.SetupTest(helicsCreateValueFederate, core, 1, 1.0);
    auto vFed = fixture.GetFederateAt(0);
    // register the publications
    auto pubid = helicsFederateRegisterGlobalPublication(
        vFed, "pub1", helics_data_type_named_point, "", &err);
    auto subid = helicsFederateRegisterSubscription(vFed, "pub1", "", &err);
    CE(helicsInputSetDefaultNamedPoint(subid, defaultValue, defVal, &err));

    CE(helicsFederateEnterExecutingMode(vFed, &err));

    // publish string1 at time=0.0;
    CE(helicsPublicationPublishNamedPoint(pubid, testValue1, testVal1, &err));

    double val;
    CE(helicsInputGetNamedPoint(subid, str, STRINGSIZE, nullptr, &val, &err));

    EXPECT_EQ(std::string(str), std::string(defaultValue));
    EXPECT_EQ(val, defVal);
    CE(gtime = helicsFederateRequestTime(vFed, 1.0, &err));

    EXPECT_EQ(gtime, 1.0);

    // get the value
    CE(helicsInputGetNamedPoint(subid, str, STRINGSIZE, nullptr, &val, &err));

    // make sure the string is what we expect
    EXPECT_EQ(std::string(str), std::string(testValue1));
    EXPECT_EQ(val, testVal1);

    // publish a second string
    CE(helicsPublicationPublishNamedPoint(pubid, testValue2, testVal2, &err));

    // make sure the value is still what we expect
    CE(helicsInputGetNamedPoint(subid, str, STRINGSIZE, nullptr, &val, &err));
    EXPECT_EQ(std::string(str), std::string(testValue1));
    EXPECT_EQ(val, testVal1);

    // advance time
    CE(gtime = helicsFederateRequestTime(vFed, 2.0, &err));
    // make sure the value was updated
    EXPECT_EQ(gtime, 2.0);

    CE(helicsInputGetNamedPoint(subid, str, STRINGSIZE, nullptr, &val, &err));
    EXPECT_EQ(std::string(str), std::string(testValue2));
    EXPECT_EQ(val, testVal2);

    CE(helicsFederateFinalize(vFed, &err));
}

TEST_P(vfed_type_tests, single_transfer_double1)
{
    runFederateTestDouble(GetParam(), 10.3, 45.3, 22.7);
}

TEST_P(vfed_type_tests, single_transfer_double2)
{
    runFederateTestDouble(GetParam(), 1.0, 0.0, 3.0);
}

TEST_P(vfed_type_tests, single_transfer_integer1)
{
    runFederateTestInteger(GetParam(), 5, 8, 43);
}

TEST_P(vfed_type_tests, single_transfer_integer2)
{
    runFederateTestInteger(GetParam(), -5, 1241515, -43);
}

TEST_P(vfed_type_tests, single_transfer_complex)
{
    runFederateTestComplex(GetParam(), 54.23233, 0.7, -9.7, 3.2, -3e45, 1e-23);
}

TEST_F(vfed_single_tests, single_transfer_complex2)
{
    runFederateTestComplex2("test", 54.23233, 0.7, -9.7, 3.2, -3e45, 1e-23);
}
TEST_P(vfed_type_tests, single_transfer_string)
{
    runFederateTestString(GetParam(),
                          "start",
                          "inside of the functional relationship of helics",
                          "I am a string");
}

TEST_P(vfed_type_tests, single_transfer_named_point)
{
    runFederateTestNamedPoint(GetParam(),
                              "start",
                              5.3,
                              "inside of the functional relationship of helics",
                              45.7823,
                              "I am a string",
                              0.0);
}

TEST_P(vfed_type_tests, single_transfer_boolean)
{
    runFederateTestBool(GetParam(), true, true, false);
}

TEST_P(vfed_type_tests, single_transfer_vector)
{
    const double val1[] = {34.3, 24.2};
    const double val2[] = {12.4, 14.7, 16.34, 18.17};
    const double val3[] = {9.9999, 8.8888, 7.7777};
    runFederateTestVectorD(GetParam(), val1, val2, val3, 2, 4, 3);
}

TEST_P(vfed_type_tests, single_transfer_vector2)
{
    std::vector<double> V1(34, 39.4491966662);
    std::vector<double> V2(100, 45.236262626221);
    std::vector<double> V3(452, -25.25263858741);
    runFederateTestVectorD(GetParam(), V1.data(), V2.data(), V3.data(), 34, 100, 452);
}

TEST_P(vfed_type_tests, subscriber_and_publisher_registration)
{
    SetupTest(helicsCreateValueFederate, GetParam(), 1, 1.0);
    auto vFed = GetFederateAt(0);

    helicsFederateSetFlagOption(vFed, helics_handle_option_connection_optional, helics_true, &err);
    // register the publications
    helics_publication pubid = helicsFederateRegisterTypePublication(vFed, "pub1", "", "", &err);
    helics_publication pubid2 =
        helicsFederateRegisterGlobalTypePublication(vFed, "pub2", "int", "", &err);
    helics_publication pubid3 =
        helicsFederateRegisterPublication(vFed, "pub3", helics_data_type_double, "V", &err);
    EXPECT_EQ(err.error_code, helics_ok);
    // these aren't meant to match the publications
    auto subid = helicsFederateRegisterSubscription(vFed, "sub1", "", &err);
    auto subid2 = helicsFederateRegisterSubscription(vFed, "sub2", "", &err);
    auto subid3 = helicsFederateRegisterSubscription(vFed, "sub3", "V", &err);
    EXPECT_EQ(err.error_code, helics_ok);
    // enter execution
    CE(helicsFederateEnterExecutingMode(vFed, &err));

    // check subscriptions
    const char* subname = helicsSubscriptionGetKey(subid);
    const char* subname2 = helicsSubscriptionGetKey(subid2);

    EXPECT_STREQ(subname, "sub1");
    EXPECT_STREQ(subname2, "sub2");
    const char* subname3 = helicsSubscriptionGetKey(subid3);
    EXPECT_STREQ(subname3, "sub3");

    // subtype=helicsInputGetType (subid);
    // EXPECT_EQ (subtype, "def");
    // subtype2=helicsInputGetType (subid2);
    // EXPECT_EQ (subtype2, "int64");
    // subtype3=helicsInputGetType (subid3);
    // EXPECT_EQ (subtype3, "def");
    auto subunit3 = helicsInputGetUnits(subid3);
    EXPECT_STREQ(subunit3, "V");

    // check publications
    const char* pubname = helicsPublicationGetKey(pubid);
    const char* pubname2 = helicsPublicationGetKey(pubid2);

    EXPECT_STREQ(pubname, "fed0/pub1");
    EXPECT_STREQ(pubname2, "pub2");
    const char* pubname3 = helicsPublicationGetKey(pubid3);
    EXPECT_STREQ(pubname3, "fed0/pub3");

    const char* pubtype = helicsPublicationGetType(pubid3);
    EXPECT_STREQ(pubtype, "double");
    const char* pubunit3 = helicsPublicationGetUnits(pubid3);
    EXPECT_STREQ(pubunit3, "V");

    CE(helicsFederateFinalize(vFed, &err));
}

TEST_P(vfed_type_tests, single_transfer_publisher)
{
    //    helics_time stime = 1.0;
    helics_time gtime;

    char s[STRINGLEN] = "n2";
    int len = 0;
    SetupTest(helicsCreateValueFederate, GetParam(), 1, 1.0);
    auto vFed = GetFederateAt(0);

    // register the publications

    auto pubid = helicsFederateRegisterGlobalTypePublication(vFed, "pub1", "string", "", &err);
    auto subid = helicsFederateRegisterSubscription(vFed, "pub1", "", &err);
    CE(helicsFederateEnterExecutingMode(vFed, &err));

    // publish string1 at time=0.0;
    CE(helicsPublicationPublishString(pubid, "string1", &err));
    CE(gtime = helicsFederateRequestTime(vFed, 1.0, &err));
    EXPECT_EQ(gtime, 1.0);

    // get the value
    CE(helicsInputGetString(subid, s, STRINGLEN, &len, &err));
    // make sure the string is what we expect
    EXPECT_STREQ(s, "string1");

    // publish a second string
    CE(helicsPublicationPublishString(pubid, "string2", &err));

    // make sure the value is still what we expect
    CE(helicsInputGetString(subid, s, STRINGLEN, &len, &err));
    EXPECT_STREQ(s, "string1");
    EXPECT_EQ(len - 1, static_cast<int>(strlen("string1")));

    // advance time
    CE(gtime = helicsFederateRequestTime(vFed, 2.0, &err));
    // make sure the value was updated
    EXPECT_EQ(gtime, 2.0);
    CE(helicsInputGetString(subid, s, STRINGLEN, &len, &err));
    EXPECT_STREQ(s, "string2");

    CE(helicsFederateFinalize(vFed, &err));
}

/** test info field for multiple publications */
TEST_P(vfed_simple_type_tests, test_info_field)
{
    SetupTest(helicsCreateValueFederate, GetParam(), 1, 1.0);
    auto vFed = GetFederateAt(0);
    helicsFederateSetFlagOption(vFed, helics_handle_option_connection_optional, helics_true, &err);
    // register the publications/subscriptions

    auto subid1 = helicsFederateRegisterSubscription(vFed, "sub1", "", &err);
    auto pubid1 = helicsFederateRegisterTypePublication(vFed, "pub1", "string", "", &err);
    auto pubid2 = helicsFederateRegisterGlobalTypePublication(vFed, "pub2", "string", "", &err);

    // Set info fields
    CE(helicsInputSetInfo(subid1, "sub1_test", &err));
    CE(helicsPublicationSetInfo(pubid1, "pub1_test", &err));
    CE(helicsPublicationSetInfo(pubid2, "pub2_test", &err));
    CE(helicsFederateEnterExecutingMode(vFed, &err));

    EXPECT_STREQ(helicsInputGetInfo(subid1), "sub1_test");
    EXPECT_STREQ(helicsPublicationGetInfo(pubid1), "pub1_test");
    EXPECT_STREQ(helicsPublicationGetInfo(pubid2), "pub2_test");

    CE(auto cr = helicsFederateGetCoreObject(vFed, &err));
    CE(helicsFederateFinalize(vFed, &err));

    CE(auto wait = helicsCoreWaitForDisconnect(cr, 70, &err));
    if (wait == helics_false) {
        wait = helicsCoreWaitForDisconnect(cr, 500, &err);
    }
    EXPECT_EQ(wait, helics_true);
}

INSTANTIATE_TEST_SUITE_P(vfed_tests,
                         vfed_simple_type_tests,
                         ::testing::ValuesIn(core_types_simple));
INSTANTIATE_TEST_SUITE_P(vfed_tests, vfed_type_tests, ::testing::ValuesIn(core_types));
