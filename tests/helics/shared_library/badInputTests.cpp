/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "ctestFixtures.hpp"

#include <gtest/gtest.h>

struct bad_input_tests: public FederateTestFixture, public ::testing::Test {
};

/** test simple creation and destruction*/
TEST_F(bad_input_tests, test_bad_fed)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);

    // this number is a completely garbage value to test bad input and not give a system fault
    auto vFed2 = reinterpret_cast<helics_federate>(reinterpret_cast<uint64_t>(vFed1) + 8);
    // register the publications

    CE(helicsFederateEnterInitializingMode(vFed1, &err));
    helicsFederateEnterInitializingMode(vFed2, &err);
    EXPECT_EQ(err.error_code, helics_error_invalid_object);
    helicsErrorClear(&err);
    // auto core = helicsFederateGetCoreObject(vFed1);

    CE(helicsFederateFinalize(vFed1, &err));
    helicsFederateFinalize(vFed2, &err);
    EXPECT_EQ(err.error_code, helics_error_invalid_object);

    helicsFederateFree(vFed1);
    helicsFederateGetCurrentTime(vFed1, &err);
    EXPECT_EQ(err.error_code, helics_error_invalid_object);
    helicsErrorClear(&err);
    // just make sure this doesn't crash
    helicsFederateFree(vFed1);
    // and make sure this doesn't crash
    helicsFederateFree(vFed2);
}

/** test simple creation and destruction*/
TEST_F(bad_input_tests, test_mistaken_free)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);
    auto fi = helicsCreateFederateInfo();
    CE(helicsFederateInfoSetBroker(fi, "broker test", &err));
    CE(helicsFederateEnterInitializingMode(vFed1, &err));
    CE(helicsFederateFinalize(vFed1, &err));

    helicsFederateInfoFree(vFed1); // this is totally wrong but we are testing it
    helicsFederateFree(fi); // this is also backwards

    helicsQueryFree(fi); // also bad
    helicsQueryFree(vFed1);

    helicsFederateInfoFree(fi); // now do the correct frees
    helicsFederateFree(vFed1);
}

/** test simple creation and destruction*/
TEST_F(bad_input_tests, test_mistaken_finalize)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);
    auto fi = helicsCreateFederateInfo();
    CE(helicsFederateInfoSetBroker(fi, "broker test", &err));
    CE(helicsFederateEnterInitializingMode(vFed1, &err));
    helicsFederateFinalize(fi, &err);

    EXPECT_NE(err.error_code, 0);

    helicsFederateInfoFree(vFed1); // this is totally wrong but we are testing it
    helicsFederateFree(fi); // this is also backwards

    helicsQueryFree(fi); // also bad
    helicsQueryFree(vFed1);

    helicsFederateInfoFree(fi); // now do the correct frees
    helicsFederateFree(vFed1);
}

TEST(error_tests, unavailable_core_type)
{
    auto err = helicsErrorInitialize();
    auto core = helicsCreateCore("nng", "test", "", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    auto brk = helicsCreateBroker("nng", "test", "", &err);
    EXPECT_NE(err.error_code, 0);

    EXPECT_EQ(helicsIsCoreTypeAvailable("nng"), helics_false);
    EXPECT_EQ(helicsIsCoreTypeAvailable(nullptr), helics_false);

    EXPECT_EQ(helicsIsCoreTypeAvailable("blahblah"), helics_false);
    helicsCoreDestroy(core);
    helicsBrokerDestroy(brk);
}

struct function_tests: public FederateTestFixture, public ::testing::Test {
};

TEST_F(function_tests, execution_iteration_test)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);
    // register the publications

    auto pubid = helicsFederateRegisterGlobalPublication(
        vFed1, "pub1", helics_data_type_double, "", nullptr);
    auto pubid2 =
        helicsFederateRegisterGlobalPublication(vFed1, "pub1", helics_data_type_double, "", &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(pubid2, nullptr);
    auto subid = helicsFederateRegisterSubscription(vFed1, "pub1", "", nullptr);
    helicsErrorClear(&err);
    helicsFederateSetTimeProperty(vFed1, helics_property_time_delta, -1.0, &err);
    EXPECT_NE(err.error_code, 0);
    helicsFederateSetTimeProperty(vFed1, helics_property_time_delta, 1.0, nullptr);

    helicsFederateEnterInitializingMode(vFed1, nullptr);
    helicsPublicationPublishDouble(pubid, 27.0, nullptr);

    auto comp = helicsFederateEnterExecutingModeIterative(
        vFed1, helics_iteration_request_iterate_if_needed, nullptr);
    EXPECT_TRUE(comp == helics_iteration_result_iterating);
    auto val = helicsInputGetDouble(subid, nullptr);
    EXPECT_EQ(val, 27.0);

    comp = helicsFederateEnterExecutingModeIterative(
        vFed1, helics_iteration_request_iterate_if_needed, nullptr);

    EXPECT_TRUE(comp == helics_iteration_result_next_step);

    auto val2 = helicsInputGetDouble(subid, nullptr);

    EXPECT_EQ(val2, val);
    helicsFederateFinalize(vFed1, nullptr);
}

TEST_F(function_tests, input_test)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);
    // register the publications

    auto pubid = helicsFederateRegisterGlobalPublication(
        vFed1, "pub1", helics_data_type_double, "", nullptr);

    auto subid = helicsFederateRegisterInput(vFed1, "inp1", helics_data_type_double, "", nullptr);
    auto subid2 = helicsFederateRegisterInput(vFed1, "inp1", helics_data_type_double, "", &err);

    EXPECT_EQ(subid2, nullptr);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsInputAddTarget(subid, "pub1", nullptr);

    auto vf2 = helicsFederateClone(vFed1, nullptr);
    EXPECT_NE(vf2, nullptr);
    EXPECT_STREQ(helicsFederateGetName(vFed1), helicsFederateGetName(vf2));

    helicsFederateSetTimeProperty(vFed1, helics_property_time_period, 1.0, nullptr);

    auto ept1 = helicsFederateRegisterEndpoint(vFed1, "ept1", "", &err);
    EXPECT_EQ(ept1, nullptr);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsFederateEnterInitializingMode(vFed1, nullptr);

    helicsPublicationPublishDouble(pubid, 27.0, nullptr);

    auto comp = helicsFederateEnterExecutingModeIterative(
        vFed1, helics_iteration_request_force_iteration, nullptr);
    EXPECT_TRUE(comp == helics_iteration_result_iterating);
    auto val = helicsInputGetDouble(subid, nullptr);
    EXPECT_EQ(val, 27.0);

    comp = helicsFederateEnterExecutingModeIterative(
        vFed1, helics_iteration_request_iterate_if_needed, nullptr);

    EXPECT_TRUE(comp == helics_iteration_result_next_step);

    auto val2 = helicsInputGetDouble(subid, nullptr);
    EXPECT_EQ(val2, val);
    //expect error entering initializing Mode again
    helicsFederateEnterInitializingMode(vFed1, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    //expect error entering initializing Mode again
    helicsFederateEnterInitializingModeAsync(vFed1, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    //expect error entering initializing Mode again
    helicsFederateEnterInitializingModeComplete(vFed1, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsFederateFinalize(vFed1, nullptr);
}

// test registrations with invalid types and some improper getValue calls
TEST_F(function_tests, raw)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);
    // register the publications

    auto pubid =
        helicsFederateRegisterGlobalPublication(vFed1, "pub1", helics_data_type_raw, "", nullptr);

    auto pubid2 = helicsFederateRegisterGlobalPublication(
        vFed1, "pub2", static_cast<helics_data_type>(6985), "", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_EQ(pubid2, nullptr);

    auto subid = helicsFederateRegisterInput(vFed1, "inp1", helics_data_type_raw, "", nullptr);

    // this is just a random bad data type
    auto subid3 =
        helicsFederateRegisterInput(vFed1, "inp3", static_cast<helics_data_type>(-6985), "", &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(subid3, nullptr);
    helicsErrorClear(&err);

    helicsInputAddTarget(subid, "pub1", nullptr);

    helicsFederateSetTimeProperty(vFed1, helics_property_time_period, 1.0, nullptr);

    helicsFederateEnterExecutingMode(vFed1, nullptr);

    helicsPublicationPublishDouble(pubid, 27.0, nullptr);
    helicsFederateRequestNextStep(vFed1, nullptr);
    // doubles and ints are not recognized
    auto val = helicsInputGetDouble(subid, &err);
    EXPECT_NE(val, 0.0);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    auto val2 = helicsInputGetInteger(subid, &err);
    EXPECT_NE(val2, 0);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    char loc[50] = "";
    int sz{0};
    //named point can generate a string
    helicsInputGetNamedPoint(subid, loc, 50, &sz, &val, &err);
    EXPECT_EQ(err.error_code, 0);

    helicsFederateFinalize(vFed1, nullptr);
}

// test registrations with invalid types 
TEST_F(function_tests, raw2)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);
    // register the publications

    auto pubid =
        helicsFederateRegisterPublication(vFed1, "pub1", helics_data_type_raw, "", nullptr);

    auto pubid2 = helicsFederateRegisterPublication(
        vFed1, "pub2", static_cast<helics_data_type>(-6985), "", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_EQ(pubid2, nullptr);

    auto subid =
        helicsFederateRegisterGlobalInput(vFed1, "inp1", helics_data_type_raw, "", nullptr);
    auto subid2 = helicsFederateRegisterGlobalInput(vFed1, "inp1", helics_data_type_raw, "", &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(subid2, nullptr);
    helicsErrorClear(&err);

    auto subid3 = helicsFederateRegisterGlobalInput(
        vFed1, "inp3", static_cast<helics_data_type>(-6985), "", &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(subid3, nullptr);
    helicsErrorClear(&err);

    helicsPublicationAddTarget(pubid, "inp1", nullptr);

    helicsFederateSetTimeProperty(vFed1, helics_property_time_period, 1.0, nullptr);

    helicsFederateEnterExecutingMode(vFed1, nullptr);

    helicsPublicationPublishDouble(pubid, 27.0, nullptr);
    helicsFederateRequestNextStep(vFed1, nullptr);
    // we are just making sure these don't blow up and cause a seg fault
    helicsInputGetRawValue(subid, nullptr, 5, nullptr, nullptr);
    helicsInputGetString(subid, nullptr, 5, nullptr, nullptr);
    auto val = helicsInputGetComplexObject(subid, &err);
    EXPECT_NE(val.real, 0.0);

    helicsFederateFinalize(vFed1, nullptr);

    helics_iteration_result res;
    helicsFederateRequestTimeIterative(
        vFed1, 1.0, helics_iteration_request_no_iteration, &res, &err);
    EXPECT_EQ(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_EQ(res, helics_iteration_result_halted);
}

//check string conversions and duplicate publication errors
TEST_F(function_tests, string)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);
    // register the publications

    auto pubid =
        helicsFederateRegisterPublication(vFed1, "pub1", helics_data_type_string, "", nullptr);

    auto pubid2 =
        helicsFederateRegisterPublication(vFed1, "pub1", helics_data_type_string, "", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_EQ(pubid2, nullptr);

    auto subid =
        helicsFederateRegisterGlobalInput(vFed1, "inp1", helics_data_type_string, "", nullptr);
    auto subid2 =
        helicsFederateRegisterGlobalInput(vFed1, "inp1", helics_data_type_string, "", &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(subid2, nullptr);
    helicsErrorClear(&err);

    helicsInputAddTarget(subid, "fed0/pub1", nullptr);

    helicsFederateSetTimeProperty(vFed1, helics_property_time_period, 1.0, nullptr);

    helicsFederateEnterExecutingMode(vFed1, nullptr);

    helicsPublicationPublishDouble(pubid, 27.0, nullptr);
    helicsFederateRequestNextStep(vFed1, nullptr);
    char str[50] = "";
    int actLen{0};
    helicsInputGetString(subid, str, 50, &actLen, &err);
    EXPECT_EQ(str[0], '2');
    EXPECT_EQ(str[1], '7');

    helicsFederateFinalize(vFed1, nullptr);
}

//check duplicate publication and inputs error pathways
TEST_F(function_tests, typePub)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);
    // register the publications

    auto pubid = helicsFederateRegisterTypePublication(vFed1, "pub1", "string", "", nullptr);

    auto pubid2 = helicsFederateRegisterTypePublication(vFed1, "pub1", "string", "", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_EQ(pubid2, nullptr);

    auto subid = helicsFederateRegisterGlobalTypeInput(vFed1, "inp1", "string", "", nullptr);
    auto subid2 = helicsFederateRegisterGlobalTypeInput(vFed1, "inp1", "string", "", &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(subid2, nullptr);
    helicsErrorClear(&err);

    helicsInputAddTarget(subid, "fed0/pub1", nullptr);

    helicsFederateSetTimeProperty(vFed1, helics_property_time_period, 1.0, nullptr);

    helicsFederateEnterExecutingMode(vFed1, nullptr);

    helicsPublicationPublishDouble(pubid, 27.0, nullptr);
    helicsFederateRequestNextStep(vFed1, nullptr);
    char str[50] = "";
    int actLen{0};
    helicsInputGetString(subid, str, 50, &actLen, &err);
    EXPECT_EQ(str[0], '2');
    EXPECT_EQ(str[1], '7');

    helicsFederateFinalize(vFed1, nullptr);
}

//check duplicate GlobalTypePublications and inputs and failures in register interface functions
TEST_F(function_tests, typePub2)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);
    // register the publications

    auto pubid = helicsFederateRegisterGlobalTypePublication(vFed1, "pub1", "string", "", nullptr);

    auto pubid2 = helicsFederateRegisterGlobalTypePublication(vFed1, "pub1", "string", "", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_EQ(pubid2, nullptr);

    helicsFederateRegisterFromPublicationJSON(vFed1, "unknownfile.json", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsFederateRegisterInterfaces(vFed1, "unknownfile.json", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsFederateRegisterInterfaces(vFed1, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    auto subid = helicsFederateRegisterTypeInput(vFed1, "inp1", "string", "", nullptr);
    auto subid2 = helicsFederateRegisterTypeInput(vFed1, "inp1", "string", "", &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(subid2, nullptr);
    helicsErrorClear(&err);

    helicsInputAddTarget(subid, "pub1", nullptr);

    helicsFederateSetTimeProperty(vFed1, helics_property_time_period, 1.0, nullptr);

    helicsFederateEnterExecutingModeIterativeAsync(
        vFed1, helics_iteration_request_no_iteration, nullptr);
    auto res = helicsFederateEnterExecutingModeIterativeComplete(vFed1, nullptr);
    EXPECT_EQ(res, helics_iteration_result_next_step);

    helicsPublicationPublishTime(pubid, 27.0, nullptr);

    helicsFederatePublishJSON(vFed1, "unknownfile.json", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsFederateRequestNextStep(vFed1, nullptr);
    char str[50] = "";
    int actLen{0};
    helicsInputGetString(subid, str, 50, &actLen, &err);
    EXPECT_EQ(str[0], '2');
    EXPECT_EQ(str[1], '7');
    helicsFederateClearUpdates(vFed1);

    helicsFederateFinalize(vFed1, nullptr);
    //run a bunch of publish fails
    helicsPublicationPublishRaw(pubid, str, actLen, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    helicsPublicationPublishString(pubid, str, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    helicsPublicationPublishInteger(pubid, 5, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    helicsPublicationPublishBoolean(pubid, helics_true, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    helicsPublicationPublishDouble(pubid, 39.2, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    helicsPublicationPublishTime(pubid, 19.2, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsPublicationPublishChar(pubid, 'a', &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsPublicationPublishComplex(pubid, 2.5, -9.8, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    double r[2] = {1.3, 2.9};
    helicsPublicationPublishVector(pubid, r, 2, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsPublicationPublishVector(pubid, nullptr, 2, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsPublicationPublishNamedPoint(pubid, nullptr, 2.0, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
}
// the init error tests use a mismatch in the publication and input to
// generate an error in the init calls so these tests go through a series of different ways
// and functions that error can get propagated to test the error paths of the different functions

TEST_F(function_tests, initError)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);
    // register the publications

    // the types here don't match which causes an error when initializing the federation
    helicsFederateRegisterGlobalTypePublication(vFed1, "pub1", "custom1", "", nullptr);

    auto subid = helicsFederateRegisterTypeInput(vFed1, "inp1", "custom2", "", nullptr);

    helicsInputAddTarget(subid, "pub1", nullptr);

    helicsFederateSetTimeProperty(vFed1, helics_property_time_period, 1.0, nullptr);

    // we are still in init mode so the next series of call should all fail
    auto tm = helicsFederateRequestTime(vFed1, 2.0, &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(tm, helics_time_invalid);
    helicsErrorClear(&err);

    helicsFederateRequestTimeAsync(vFed1, 2.0, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    tm = helicsFederateRequestTimeComplete(vFed1, &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(tm, helics_time_invalid);
    helicsErrorClear(&err);

    // this one will generate an error from the mismatch types
    helicsFederateEnterExecutingMode(vFed1, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsFederateRequestNextStep(vFed1, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsFederateFinalize(vFed1, nullptr);
}

TEST_F(function_tests, initError2)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);
    // register the publications

     // the types here don't match which causes an error when initializing the federation
    helicsFederateRegisterGlobalTypePublication(vFed1, "pub1", "custom1", "", nullptr);

    auto subid = helicsFederateRegisterTypeInput(vFed1, "inp1", "custom2", "", nullptr);
    auto k1 = helicsInputGetKey(subid);

    // check some other calls
    auto inp2 = helicsFederateGetInput(vFed1, "inp1", &err);
    EXPECT_EQ(err.error_code, 0);
    auto k2 = helicsInputGetKey(inp2);

    EXPECT_STREQ(k1, k2);

    auto inp3 = helicsFederateGetInputByIndex(vFed1, 0, &err);
    EXPECT_EQ(err.error_code, 0);
    auto k3 = helicsInputGetKey(inp3);

    EXPECT_STREQ(k1, k3);

    helicsInputAddTarget(subid, "pub1", nullptr);

    helicsFederateSetTimeProperty(vFed1, helics_property_time_period, 1.0, nullptr);

    //unknown publication 
    auto pub3 = helicsFederateGetPublication(vFed1, "unknown", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_EQ(pub3, nullptr);

    // error in this call from the mismatch 
    helicsFederateEnterInitializingMode(vFed1, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsFederateRequestTimeAdvance(vFed1, 0.1, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    //unknown input
    auto inp4 = helicsFederateGetInput(vFed1, "unknown", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_EQ(inp4, nullptr);

    //invalid input index
    auto inp5 = helicsFederateGetInputByIndex(vFed1, 4, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_EQ(inp5, nullptr);

    helicsFederateFinalize(vFed1, nullptr);
}

TEST_F(function_tests, initError3)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);
    // register the publications

      // the types here don't match which causes an error when initializing the federation
    helicsFederateRegisterGlobalTypePublication(vFed1, "pub1", "custom1", "", nullptr);

    auto subid = helicsFederateRegisterTypeInput(vFed1, "inp1", "custom2", "", nullptr);

    helicsInputAddTarget(subid, "pub1", nullptr);

    auto inp3 = helicsFederateGetSubscription(vFed1, "unknown", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_EQ(inp3, nullptr);

    helicsFederateSetTimeProperty(vFed1, helics_property_time_period, 1.0, nullptr);

    helicsFederateEnterInitializingModeAsync(vFed1, &err);
    EXPECT_EQ(err.error_code, 0);
    helicsFederateEnterInitializingModeComplete(vFed1, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsFederateFinalize(vFed1, nullptr);
}

TEST_F(function_tests, initError4)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);
    // register the publications

    helicsFederateRegisterGlobalTypePublication(vFed1, "pub1", "custom1", "", nullptr);

    auto subid = helicsFederateRegisterTypeInput(vFed1, "inp1", "custom2", "", nullptr);

    helicsInputAddTarget(subid, "pub1", nullptr);

    helicsFederateSetTimeProperty(vFed1, helics_property_time_period, 1.0, nullptr);

    helicsFederateEnterExecutingModeAsync(vFed1, &err);
    EXPECT_EQ(err.error_code, 0);

    helicsFederateEnterExecutingModeComplete(vFed1, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsFederateIsAsyncOperationCompleted(vFed1, &err);
    EXPECT_EQ(err.error_code, 0);

    helics_iteration_result res;
    helicsFederateRequestTimeIterative(
        vFed1, 1.0, helics_iteration_request_no_iteration, &res, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_EQ(res, helics_iteration_result_error);

    helicsFederateFinalize(vFed1, nullptr);

    helicsFederateEnterExecutingModeAsync(vFed1, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsFederateEnterExecutingModeIterativeAsync(
        vFed1, helics_iteration_request_force_iteration, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST_F(function_tests, initError5)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);
    // register the publications

    helicsFederateRegisterGlobalTypePublication(vFed1, "pub1", "custom1", "", nullptr);

    auto subid = helicsFederateRegisterTypeInput(vFed1, "inp1", "custom2", "", nullptr);

    helicsInputAddTarget(subid, "pub1", nullptr);

    helicsFederateSetTimeProperty(vFed1, helics_property_time_period, 1.0, nullptr);

    helicsFederateEnterExecutingModeIterative(vFed1, helics_iteration_request_no_iteration, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsFederateRequestTimeIterativeAsync(
        vFed1, 1.0, helics_iteration_request_no_iteration, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsFederateFinalize(vFed1, nullptr);
}

TEST_F(function_tests, initError6)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);
    // register the publications

      // the types here don't match which causes an error when initializing the federation
    helicsFederateRegisterGlobalTypePublication(vFed1, "pub1", "custom1", "", nullptr);

    auto subid = helicsFederateRegisterTypeInput(vFed1, "inp1", "custom2", "", nullptr);

    helicsInputAddTarget(subid, "pub1", nullptr);

    helicsFederateSetTimeProperty(vFed1, helics_property_time_period, 1.0, nullptr);

    helicsFederateEnterExecutingModeIterativeAsync(
        vFed1, helics_iteration_request_no_iteration, &err);
    EXPECT_EQ(err.error_code, 0);
    helicsFederateEnterExecutingModeIterativeComplete(vFed1, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsFederateRequestTimeIterativeAsync(
        vFed1, 1.0, helics_iteration_request_no_iteration, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    helicsFederateRequestTimeIterativeComplete(vFed1, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsFederateFinalize(vFed1, nullptr);
}

// the next series of tests out error paths and different functions related to the message federate
TEST_F(function_tests, messageFed)
{
    SetupTest(helicsCreateMessageFederate, "test", 1);
    auto mFed1 = GetFederateAt(0);

    // duplicate names for endpoints are not allowed
    auto ept1 = helicsFederateRegisterEndpoint(mFed1, "ept1", "", nullptr);
    EXPECT_NE(ept1, nullptr);
    auto ept2 = helicsFederateRegisterEndpoint(mFed1, "ept1", "", &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(ept2, nullptr);
    helicsErrorClear(&err);
    //not a valid endpoint
    auto ept3 = helicsFederateGetEndpoint(mFed1, "invalid", &err);
    EXPECT_EQ(ept3, nullptr);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    auto ept4 = helicsFederateGetEndpointByIndex(mFed1, 5, &err);
    EXPECT_EQ(ept4, nullptr);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    // can't register a publication on a message federate
    auto subid = helicsFederateRegisterPublication(mFed1, "key", helics_data_type_double, "", &err);
    EXPECT_EQ(subid, nullptr);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsFederateEnterExecutingMode(mFed1, nullptr);
    helicsEndpointSetDefaultDestination(ept1, "fed0/ept1", nullptr);

    //test out messages without specifying endpoints
    helicsEndpointSendMessageRaw(ept1, nullptr, nullptr, 0, &err);

    helicsEndpointSendMessageRaw(ept1, "fed0/ept1", nullptr, 0, &err);

    helicsFederateRequestNextStep(mFed1, nullptr);
    //make sure the message got through
    auto cnt = helicsEndpointPendingMessages(ept1);
    EXPECT_EQ(cnt, 2);

    helicsFederateFinalize(mFed1, nullptr);
    helicsEndpointSendMessageRaw(ept1, "fed0/ept1", nullptr, 0, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST_F(function_tests, messageFed_event)
{
    SetupTest(helicsCreateMessageFederate, "test", 1);
    auto mFed1 = GetFederateAt(0);

    auto ept1 = helicsFederateRegisterGlobalEndpoint(mFed1, "ept1", "", nullptr);
    EXPECT_NE(ept1, nullptr);
    auto ept2 = helicsFederateRegisterGlobalEndpoint(mFed1, "ept1", "", &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(ept2, nullptr);
    helicsErrorClear(&err);

    helicsFederateEnterExecutingMode(mFed1, nullptr);

    //send events without destinations
    helicsEndpointSetDefaultDestination(ept1, "ept1", nullptr);

    helicsEndpointSendEventRaw(ept1, nullptr, nullptr, 0, 0.0, &err);

    helicsEndpointSendEventRaw(ept1, "ept1", nullptr, 0, 0.0, &err);

    char data[5] = "test";
    helicsEndpointSendEventRaw(ept1, nullptr, data, 4, 0.0, &err);
    helicsFederateRequestNextStep(mFed1, nullptr);
    auto cnt = helicsEndpointPendingMessages(ept1);
    EXPECT_EQ(cnt, 3);

    helicsFederateFinalize(mFed1, nullptr);
    //  can't send an event after the federate is finalized
    helicsEndpointSendEventRaw(ept1, nullptr, data, 4, 0.0, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST_F(function_tests, messageFed_message)
{
    SetupTest(helicsCreateMessageFederate, "test", 1);
    auto mFed1 = GetFederateAt(0);

    auto ept1 = helicsFederateRegisterGlobalEndpoint(mFed1, "ept1", "", nullptr);
    EXPECT_NE(ept1, nullptr);
    auto ept2 = helicsFederateRegisterGlobalEndpoint(mFed1, "ept1", "", &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(ept2, nullptr);
    helicsErrorClear(&err);

    helicsFederateEnterExecutingMode(mFed1, nullptr);
    helicsEndpointSetDefaultDestination(ept1, "ept1", nullptr);

    // if we are sending a message it can't be null
    helicsEndpointSendMessage(ept1, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helics_message mess0 = helicsEndpointGetMessage(ept1);
    EXPECT_EQ(mess0.length, 0);

    mess0 = helicsFederateGetMessage(mFed1);
    EXPECT_EQ(mess0.length, 0);

    //send a series of different messages testing different code paths
    helics_message mess1;

    mess1.time = 0.0;
    mess1.original_source = nullptr;
    mess1.dest = nullptr;
    mess1.data = nullptr;
    mess1.length = 0;
    mess1.source = nullptr;
    mess1.original_dest = nullptr;
    helicsEndpointSendMessage(ept1, &mess1, &err);
    mess1.dest = "ept1";
    helicsEndpointSendMessage(ept1, &mess1, &err);
    mess1.original_source = "ept4";

    char data[5] = "test";
    helicsEndpointSendMessage(ept1, &mess1, &err);

    mess1.data = data;
    mess1.length = 4;
    helicsEndpointSendMessage(ept1, &mess1, &err);

    helicsFederateRequestNextStep(mFed1, nullptr);
    auto cnt = helicsEndpointPendingMessages(ept1);
    EXPECT_EQ(cnt, 4);

    helicsFederateFinalize(mFed1, nullptr);
    // can't send a message after the federate is finalized
    helicsEndpointSendMessage(ept1, &mess1, &err);
    EXPECT_NE(err.error_code, 0);
}

// test different paths for sending message objects
TEST_F(function_tests, messageFed_message_object)
{
    SetupTest(helicsCreateMessageFederate, "test", 1);
    auto mFed1 = GetFederateAt(0);

    auto ept1 = helicsFederateRegisterGlobalEndpoint(mFed1, "ept1", "", nullptr);
    EXPECT_NE(ept1, nullptr);
    auto ept2 = helicsFederateRegisterGlobalEndpoint(mFed1, "ept1", "", &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(ept2, nullptr);
    helicsErrorClear(&err);

    helicsEndpointSubscribe(ept1, "key", nullptr);
    helicsFederateEnterExecutingMode(mFed1, nullptr);
    helicsEndpointSetDefaultDestination(ept1, "ept1", nullptr);
    auto mess0 = helicsEndpointGetMessageObject(ept1);
    EXPECT_EQ(helicsMessageGetRawDataSize(mess0), 0);

    mess0 = helicsFederateGetMessageObject(mFed1);
    EXPECT_EQ(helicsMessageGetRawDataSize(mess0), 0);

    helicsEndpointSendMessageObject(ept1, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    auto mess1 = helicsFederateCreateMessageObject(mFed1, nullptr);
    helicsMessageSetDestination(mess1, "ept1", nullptr);
    helicsEndpointSendMessageObject(ept1, mess1, &err);

    helicsFederateRequestNextStep(mFed1, nullptr);
    auto cnt = helicsEndpointPendingMessages(ept1);
    EXPECT_EQ(cnt, 1);

    auto m3 = helicsFederateGetMessageObject(mFed1);
    EXPECT_NE(m3, nullptr);

    helicsFederateFinalize(mFed1, nullptr);

    helicsEndpointSendMessageObject(ept1, mess1, &err);
    EXPECT_NE(err.error_code, 0);

    helicsFederateClearMessages(mFed1);
    helicsEndpointClearMessages(ept1);
}

// test error paths for filters
TEST_F(function_tests, filter_tests)
{
    SetupTest(helicsCreateMessageFederate, "test", 1);

    auto mFed1 = GetFederateAt(0);

    auto filt1 = helicsFederateRegisterFilter(mFed1, helics_filter_type_delay, "filt1", nullptr);
    EXPECT_NE(filt1, nullptr);
    auto filt2 = helicsFederateRegisterFilter(mFed1, helics_filter_type_delay, "filt1", &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(filt2, nullptr);

    helicsFederateFinalize(mFed1, nullptr);
}

// test some other filter creation functions and error paths
TEST_F(function_tests, filter_tests2)
{
    SetupTest(helicsCreateMessageFederate, "test", 1);

    auto mFed1 = GetFederateAt(0);

    auto filt1 =
        helicsFederateRegisterGlobalFilter(mFed1, helics_filter_type_delay, "filt1", nullptr);
    EXPECT_NE(filt1, nullptr);
    auto filt2 = helicsFederateRegisterGlobalFilter(mFed1, helics_filter_type_delay, "filt1", &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(filt2, nullptr);
    helicsErrorClear(&err);

    auto f3 = helicsFederateGetFilter(mFed1, "unknown", &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(f3, nullptr);

    auto f4 = helicsFederateGetFilterByIndex(mFed1, 10, &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(f4, nullptr);

    helicsErrorClear(&err);

    helicsFederateFinalize(mFed1, nullptr);
}

TEST_F(function_tests, filter_tests3)
{
    SetupTest(helicsCreateMessageFederate, "test", 1);

    auto mFed1 = GetFederateAt(0);

    auto filt1 = helicsFederateRegisterGlobalCloningFilter(mFed1, "filt1", nullptr);
    EXPECT_NE(filt1, nullptr);
    auto filt2 = helicsFederateRegisterGlobalCloningFilter(mFed1, "filt1", &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(filt2, nullptr);

    helicsFederateFinalize(mFed1, nullptr);
}

TEST_F(function_tests, filter_tests4)
{
    SetupTest(helicsCreateMessageFederate, "test", 1);

    auto mFed1 = GetFederateAt(0);

    auto filt1 = helicsFederateRegisterCloningFilter(mFed1, "filt1", nullptr);
    EXPECT_NE(filt1, nullptr);
    auto filt2 = helicsFederateRegisterCloningFilter(mFed1, "filt1", &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(filt2, nullptr);
    helicsErrorClear(&err);
    helicsFilterSetString(filt1, "unknown", "string", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsFederateRegisterGlobalEndpoint(mFed1, "ept1", "", nullptr);

    helicsFilterAddDeliveryEndpoint(filt1, "ept1", nullptr);
    helicsFilterAddSourceTarget(filt1, "ept1", &err);
    EXPECT_EQ(err.error_code, 0);
    helicsFilterAddDestinationTarget(filt1, "ept1", &err);
    EXPECT_EQ(err.error_code, 0);
    helicsFilterRemoveTarget(filt1, "ept1", &err);
    EXPECT_EQ(err.error_code, 0);

    helicsFilterSet(filt1, "unknown", 10.0, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    helicsFederateFinalize(mFed1, nullptr);
}

// test filter creation in a core and error pathways
TEST_F(function_tests, filter_core_tests)
{
    SetupTest(helicsCreateMessageFederate, "test", 1);

    auto mFed1 = GetFederateAt(0);

    auto cr = helicsFederateGetCoreObject(mFed1, nullptr);

    auto filt1 = helicsCoreRegisterFilter(cr, helics_filter_type_delay, "filt1", nullptr);
    EXPECT_NE(filt1, nullptr);
    auto filt2 = helicsCoreRegisterFilter(cr, helics_filter_type_delay, "filt1", &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(filt2, nullptr);
    helicsErrorClear(&err);
    helicsFilterSetOption(filt1, helics_handle_option_connection_optional, helics_true, &err);
    EXPECT_EQ(err.error_code, 0);
    helicsFilterGetOption(filt1, helics_handle_option_connection_optional);
    EXPECT_EQ(err.error_code, 0);
    helicsFederateFinalize(mFed1, nullptr);
    helicsCoreDestroy(cr);
}

// test cloning filter creation from a core and some error paths
TEST_F(function_tests, filter_core_tests2)
{
    SetupTest(helicsCreateMessageFederate, "test", 1);

    auto mFed1 = GetFederateAt(0);

    auto cr = helicsFederateGetCoreObject(mFed1, nullptr);

    auto filt1 = helicsCoreRegisterCloningFilter(cr, "filt1", nullptr);
    EXPECT_NE(filt1, nullptr);
    auto filt2 = helicsCoreRegisterCloningFilter(cr, "filt1", &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(filt2, nullptr);

    helicsFederateFinalize(mFed1, nullptr);
    helicsCoreDestroy(cr);
}
