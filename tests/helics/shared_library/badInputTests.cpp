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

struct function_tests : public FederateTestFixture, public ::testing::Test {
};

TEST_F(function_tests, execution_iteration_test)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);
    // register the publications

    auto pubid = helicsFederateRegisterGlobalPublication(vFed1, "pub1", helics_data_type_double, "", nullptr);
    auto pubid2 = helicsFederateRegisterGlobalPublication(vFed1, "pub1", helics_data_type_double, "", &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(pubid2, nullptr);
    auto subid = helicsFederateRegisterSubscription(vFed1, "pub1", "", nullptr);
    helicsErrorClear(&err);
    helicsFederateSetTimeProperty(vFed1, helics_property_time_delta, -1.0, &err);
    EXPECT_NE(err.error_code, 0);
    helicsFederateSetTimeProperty(vFed1, helics_property_time_delta, 1.0, nullptr);

    helicsFederateEnterInitializingMode(vFed1, nullptr);
    helicsPublicationPublishDouble(pubid, 27.0, nullptr);

    auto comp = helicsFederateEnterExecutingModeIterative(vFed1, helics_iteration_request_iterate_if_needed, nullptr);
    EXPECT_TRUE(comp == helics_iteration_result_iterating);
    auto val = helicsInputGetDouble(subid, nullptr);
    EXPECT_EQ(val, 27.0);

    comp = helicsFederateEnterExecutingModeIterative(vFed1, helics_iteration_request_iterate_if_needed, nullptr);

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

    auto pubid = helicsFederateRegisterGlobalPublication(vFed1, "pub1", helics_data_type_double, "", nullptr);

    auto subid = helicsFederateRegisterInput(vFed1, "inp1", helics_data_type_double, "", nullptr);
    auto subid2 = helicsFederateRegisterInput(vFed1, "inp1", helics_data_type_double, "", &err);
    EXPECT_EQ(subid2, nullptr);
    helicsInputAddTarget(subid, "pub1", nullptr);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    helicsFederateSetTimeProperty(vFed1, helics_property_time_period, 1.0, nullptr);

    helicsFederateEnterInitializingMode(vFed1, nullptr);
    

    helicsPublicationPublishDouble(pubid, 27.0, nullptr);

    auto comp = helicsFederateEnterExecutingModeIterative(vFed1, helics_iteration_request_iterate_if_needed, nullptr);
    EXPECT_TRUE(comp == helics_iteration_result_iterating);
    auto val = helicsInputGetDouble(subid, nullptr);
    EXPECT_EQ(val, 27.0);

    comp = helicsFederateEnterExecutingModeIterative(vFed1, helics_iteration_request_iterate_if_needed, nullptr);

    EXPECT_TRUE(comp == helics_iteration_result_next_step);

    auto val2 = helicsInputGetDouble(subid, nullptr);
    EXPECT_EQ(val2, val);
    //expect error entering initializing Mode again
    helicsFederateEnterInitializingMode(vFed1, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

  
    helicsFederateFinalize(vFed1, nullptr);
}


TEST_F(function_tests, raw)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);
    // register the publications

    auto pubid = helicsFederateRegisterGlobalPublication(vFed1, "pub1", helics_data_type_raw, "", nullptr);

    auto pubid2 = helicsFederateRegisterGlobalPublication(vFed1, "pub2", static_cast<helics_data_type>(6985), "", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_EQ(pubid2, nullptr);

    auto subid = helicsFederateRegisterInput(vFed1, "inp1", helics_data_type_raw, "", nullptr);

    auto subid3 = helicsFederateRegisterInput(vFed1, "inp3", static_cast<helics_data_type>(-6985), "", &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(subid3, nullptr);
    helicsErrorClear(&err);

    helicsInputAddTarget(subid, "pub1", nullptr);

    helicsFederateSetTimeProperty(vFed1, helics_property_time_period, 1.0, nullptr);

    helicsFederateEnterExecutingMode(vFed1, nullptr);

    helicsPublicationPublishDouble(pubid, 27.0, nullptr);
    helicsFederateRequestNextStep(vFed1, nullptr);
    auto val = helicsInputGetDouble(subid, &err);
    EXPECT_NE(val, 0.0);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    auto val2 = helicsInputGetInteger(subid, &err);
    EXPECT_NE(val2, 0);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    char loc[50]="";
    int sz{ 0 };
    //named point can generate a string
    helicsInputGetNamedPoint(subid, loc,50,&sz,&val,&err);
    EXPECT_EQ(err.error_code, 0);

    helicsFederateFinalize(vFed1, nullptr);
}

TEST_F(function_tests, raw2)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);
    // register the publications

    auto pubid = helicsFederateRegisterPublication(vFed1, "pub1", helics_data_type_raw, "", nullptr);

    auto pubid2 = helicsFederateRegisterPublication(vFed1, "pub2", static_cast<helics_data_type>(-6985), "", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_EQ(pubid2, nullptr);
   

    auto subid = helicsFederateRegisterGlobalInput(vFed1, "inp1", helics_data_type_raw, "", nullptr);
    auto subid2 = helicsFederateRegisterGlobalInput(vFed1, "inp1", helics_data_type_raw, "", &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(subid2, nullptr);
    helicsErrorClear(&err);

    auto subid3 = helicsFederateRegisterGlobalInput(vFed1, "inp3", static_cast<helics_data_type>(-6985), "", &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(subid3, nullptr);
    helicsErrorClear(&err);

    helicsPublicationAddTarget(pubid, "inp1", nullptr);

    helicsFederateSetTimeProperty(vFed1, helics_property_time_period, 1.0, nullptr);

    helicsFederateEnterExecutingMode(vFed1, nullptr);

    helicsPublicationPublishDouble(pubid, 27.0, nullptr);
    helicsFederateRequestNextStep(vFed1, nullptr);

    helicsInputGetRawValue(subid, nullptr, 5, nullptr, nullptr);
    helicsInputGetString(subid, nullptr, 5, nullptr, nullptr);
    auto val = helicsInputGetComplexObject(subid, &err);
    EXPECT_NE(val.real, 0.0);

    helicsFederateFinalize(vFed1, nullptr);
}

TEST_F(function_tests, string)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);
    // register the publications

    auto pubid = helicsFederateRegisterPublication(vFed1, "pub1", helics_data_type_string, "", nullptr);

    auto pubid2 = helicsFederateRegisterPublication(vFed1, "pub1", helics_data_type_string, "", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_EQ(pubid2, nullptr);


    auto subid = helicsFederateRegisterGlobalInput(vFed1, "inp1", helics_data_type_string, "", nullptr);
    auto subid2 = helicsFederateRegisterGlobalInput(vFed1, "inp1", helics_data_type_string, "", &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(subid2, nullptr);
    helicsErrorClear(&err);

    helicsInputAddTarget(subid, "fed0/pub1", nullptr);

    helicsFederateSetTimeProperty(vFed1, helics_property_time_period, 1.0, nullptr);

    helicsFederateEnterExecutingMode(vFed1, nullptr);

    helicsPublicationPublishDouble(pubid, 27.0, nullptr);
    helicsFederateRequestNextStep(vFed1, nullptr);
    char str[50] = "";
    int actLen{ 0 };
    helicsInputGetString(subid,str,50,&actLen, &err);
    EXPECT_EQ(str[0], '2');
    EXPECT_EQ(str[1], '7');

    helicsFederateFinalize(vFed1, nullptr);
}

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
    int actLen{ 0 };
    helicsInputGetString(subid, str, 50, &actLen, &err);
    EXPECT_EQ(str[0], '2');
    EXPECT_EQ(str[1], '7');

    helicsFederateFinalize(vFed1, nullptr);
}

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

    auto subid = helicsFederateRegisterTypeInput(vFed1, "inp1", "string", "", nullptr);
    auto subid2 = helicsFederateRegisterTypeInput(vFed1, "inp1", "string", "", &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(subid2, nullptr);
    helicsErrorClear(&err);

    helicsInputAddTarget(subid, "pub1", nullptr);

    helicsFederateSetTimeProperty(vFed1, helics_property_time_period, 1.0, nullptr);

    helicsFederateEnterExecutingMode(vFed1, nullptr);

    helicsPublicationPublishTime(pubid, 27.0, nullptr);

    helicsFederatePublishJSON(vFed1, "unknownfile.json", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsFederateRequestNextStep(vFed1, nullptr);
    char str[50] = "";
    int actLen{ 0 };
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

    helicsPublicationPublishComplex(pubid, 2.5,-9.8, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    double r[2] = { 1.3,2.9 };
    helicsPublicationPublishVector(pubid, r,2, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsPublicationPublishVector(pubid, nullptr, 2, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsPublicationPublishNamedPoint(pubid, nullptr, 2.0, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

}

TEST_F(function_tests, initError)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);
    // register the publications

    helicsFederateRegisterGlobalTypePublication(vFed1, "pub1", "custom1", "", nullptr);

    auto subid=helicsFederateRegisterTypeInput(vFed1, "inp1", "custom2", "", nullptr);

    helicsInputAddTarget(subid, "pub1", nullptr);

    helicsFederateSetTimeProperty(vFed1, helics_property_time_period, 1.0, nullptr);

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

    helicsFederateRegisterGlobalTypePublication(vFed1, "pub1", "custom1", "", nullptr);

    auto subid = helicsFederateRegisterTypeInput(vFed1, "inp1", "custom2", "", nullptr);
    auto k1 = helicsInputGetKey(subid);

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

    auto pub3 = helicsFederateGetPublication(vFed1, "unknown", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_EQ(pub3, nullptr);

    helicsFederateEnterInitializingMode(vFed1, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsFederateRequestTimeAdvance(vFed1,0.1, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    auto inp4 = helicsFederateGetInput(vFed1, "unknown", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_EQ(inp4, nullptr);

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

    helicsFederateRequestTimeIterative(vFed1, 1.0, helics_iteration_request_no_iteration, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsFederateFinalize(vFed1, nullptr);
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

    helicsFederateEnterExecutingModeIterative(vFed1,helics_iteration_request_no_iteration, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsFederateRequestTimeIterativeAsync(vFed1, 1.0, helics_iteration_request_no_iteration, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsFederateFinalize(vFed1, nullptr);
}


TEST_F(function_tests, initError6)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);
    // register the publications

    helicsFederateRegisterGlobalTypePublication(vFed1, "pub1", "custom1", "", nullptr);

    auto subid = helicsFederateRegisterTypeInput(vFed1, "inp1", "custom2", "", nullptr);

    helicsInputAddTarget(subid, "pub1", nullptr);

    helicsFederateSetTimeProperty(vFed1, helics_property_time_period, 1.0, nullptr);

    helicsFederateEnterExecutingModeIterativeAsync(vFed1,helics_iteration_request_no_iteration, &err);
    EXPECT_EQ(err.error_code, 0);
    helicsFederateEnterExecutingModeIterativeComplete(vFed1, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsFederateRequestTimeIterativeAsync(vFed1, 1.0, helics_iteration_request_no_iteration, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    helicsFederateRequestTimeIterativeComplete(vFed1, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsFederateFinalize(vFed1, nullptr);
}
