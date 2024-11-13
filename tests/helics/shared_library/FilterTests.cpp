/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "ctestFixtures.hpp"
#include "helics/helics.h"

#include <cstdio>
#include <future>
#include <gtest/gtest.h>
#include <string>
#include <thread>
/** these test cases test out the message federates
 */

class filter_simple_type_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {};

/*
class filter_type_tests: public ::testing::TestWithParam<const char*>, public FederateTestFixture {
};
*/

class filter: public FederateTestFixture, public ::testing::Test {};

/** test registration of filters*/

TEST_P(filter_simple_type_tests, registration)
{
    auto broker = AddBroker(GetParam(), 2);
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, HELICS_TIME_ZERO, "filter");
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, HELICS_TIME_ZERO, "message");

    auto fFed = GetFederateAt(0);
    auto mFed = GetFederateAt(1);

    helicsFederateRegisterGlobalEndpoint(mFed, "port1", "", &err);
    helicsFederateRegisterGlobalEndpoint(mFed, "port2", nullptr, &err);
    EXPECT_EQ(err.error_code, HELICS_OK);
    CE(auto filt1 = helicsFederateRegisterFilter(fFed, HELICS_FILTER_TYPE_CUSTOM, "filter1", &err));
    CE(helicsFilterAddSourceTarget(filt1, "port1", &err));
    EXPECT_TRUE(filt1 != nullptr);
    CE(auto filter2 =
           helicsFederateRegisterFilter(fFed, HELICS_FILTER_TYPE_CUSTOM, "filter2", &err));
    CE(helicsFilterAddDestinationTarget(filter2, "port2", &err));
    EXPECT_TRUE(filter2 != filt1);
    CE(auto ep1 = helicsFederateRegisterEndpoint(fFed, "fout", "", &err));
    EXPECT_TRUE(ep1 != nullptr);
    CE(auto filter3 = helicsFederateRegisterFilter(fFed, HELICS_FILTER_TYPE_CUSTOM, "c4", &err));
    EXPECT_EQ(helicsFilterIsValid(filter3), HELICS_TRUE);
    helicsFilterAddSourceTarget(filter3, "filter0/fout", nullptr);
    EXPECT_TRUE(filter3 != filter2);

    auto f1_b = helicsFederateGetFilter(fFed, "filter1", &err);
    const char* tmp;
    tmp = helicsFilterGetName(f1_b);
    EXPECT_STREQ(tmp, "filter0/filter1");

    CE(helicsFilterSetTag(f1_b, "tag1", "tagvalue", &err));
    EXPECT_STREQ(helicsFilterGetTag(f1_b, "tag1"), "tagvalue");

    auto f1_c = helicsFederateGetFilterByIndex(fFed, 2, &err);
    tmp = helicsFilterGetName(f1_c);
    EXPECT_STREQ(tmp, "filter0/c4");

    auto f1_n = helicsFederateGetFilterByIndex(fFed, -2, &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(f1_n, nullptr);
    helicsErrorClear(&err);

    CE(helicsFederateEnterExecutingModeAsync(fFed, &err));
    CE(helicsFederateEnterExecutingMode(mFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(fFed, &err));
    CE(helicsFederateFinalizeAsync(mFed, &err));
    CE(helicsFederateFinalize(fFed, &err));
    CE(helicsFederateFinalizeComplete(mFed, &err));

    CE(HelicsFederateState state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}

/**
Test filter info fields
*/
TEST_P(filter_simple_type_tests, info_tests)
{
    auto broker = AddBroker(GetParam(), 2);
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, HELICS_TIME_ZERO, "filter");
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, HELICS_TIME_ZERO, "message");

    auto fFed = GetFederateAt(0);
    auto mFed = GetFederateAt(1);

    auto port1 = helicsFederateRegisterGlobalEndpoint(mFed, "port1", "", &err);
    auto port2 = helicsFederateRegisterGlobalEndpoint(mFed, "port2", nullptr, &err);

    CE(helicsEndpointSetInfo(port1, "p1_test", &err));
    CE(helicsEndpointSetInfo(port2, "p2_test", &err));

    CE(auto filt1 = helicsFederateRegisterFilter(fFed, HELICS_FILTER_TYPE_CUSTOM, "filter1", &err));
    CE(helicsFilterAddSourceTarget(filt1, "port1", &err));
    CE(helicsFilterSetInfo(filt1, "f1_test", &err));

    CE(auto dfilter =
           helicsFederateRegisterFilter(fFed, HELICS_FILTER_TYPE_CUSTOM, "filter2", &err));
    CE(helicsFilterAddDestinationTarget(dfilter, "port2", &err));
    CE(helicsFilterSetInfo(dfilter, "f2_test", &err));

    CE(auto ep1 = helicsFederateRegisterEndpoint(fFed, "fout", "", &err));
    CE(helicsEndpointSetInfo(ep1, "ep1_test", &err));
    CE(auto filter3 = helicsFederateRegisterFilter(fFed, HELICS_FILTER_TYPE_CUSTOM, "c4", &err));
    helicsFilterAddSourceTarget(filter3, "filter0/fout", nullptr);
    CE(helicsFilterSetInfo(filter3, "f3_test", &err));

    // Check endpoints
    EXPECT_STREQ(helicsEndpointGetInfo(port1), "p1_test");
    EXPECT_STREQ(helicsEndpointGetInfo(port2), "p2_test");
    EXPECT_STREQ(helicsEndpointGetInfo(ep1), "ep1_test");

    // Check filters
    EXPECT_STREQ(helicsFilterGetInfo(filt1), "f1_test");
    EXPECT_STREQ(helicsFilterGetInfo(dfilter), "f2_test");
    EXPECT_STREQ(helicsFilterGetInfo(filter3), "f3_test");

    CE(helicsFederateEnterExecutingModeAsync(fFed, &err));
    CE(helicsFederateEnterExecutingMode(mFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(fFed, &err));

    CE(helicsFederateFinalizeAsync(mFed, &err));
    CE(helicsFederateFinalize(fFed, &err));
    CE(helicsFederateFinalizeComplete(mFed, &err));
}

TEST_F(filter, core_filter_reg)
{
    CE(auto core1 = helicsCreateCore("test", "core1", "--autobroker", &err));

    CE(auto core2 = helicsCoreClone(core1, &err));

    std::string core1IdentifierString = helicsCoreGetIdentifier(core1);

    EXPECT_EQ(core1IdentifierString, "core1");

    CE(auto sourceFilter1 = helicsCoreRegisterFilter(
           core1, HelicsFilterTypes::HELICS_FILTER_TYPE_DELAY, "core1SourceFilter", &err));

    CE(helicsFilterAddSourceTarget(sourceFilter1, "ep1", &err));
    CE(auto destinationFilter1 = helicsCoreRegisterFilter(
           core1, HelicsFilterTypes::HELICS_FILTER_TYPE_DELAY, "core1DestinationFilter", &err));

    helicsFilterAddDestinationTarget(destinationFilter1, "ep2", &err);
    CE(auto cloningFilter1 = helicsCoreRegisterCloningFilter(core1, "ep3", &err));

    helicsFilterRemoveDeliveryEndpoint(cloningFilter1, "ep3", &err);
    int core1IsConnected = helicsCoreIsConnected(core1);
    EXPECT_NE(core1IsConnected, HELICS_FALSE);
    helicsCoreSetReadyToInit(core1, &err);
    helicsCoreDisconnect(core1, &err);
    helicsCoreDisconnect(core2, &err);
    helicsCoreFree(core1);
    helicsCoreFree(core2);
    helicsCloseLibrary();
}

TEST_P(filter_simple_type_tests, message_filter_function)
{
    HelicsBroker broker = AddBroker(GetParam(), 2);
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, 1.0, "filter");
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAt(0);
    auto mFed = GetFederateAt(1);

    CE(helicsFederateSetFlagOption(
        mFed, HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS, HELICS_TRUE, &err));
    auto port1 = helicsFederateRegisterGlobalEndpoint(mFed, "port1", nullptr, &err);
    auto port2 = helicsFederateRegisterGlobalEndpoint(mFed, "port2", "", &err);
    EXPECT_EQ(err.error_code, HELICS_OK);

    auto filter1 = helicsFederateRegisterFilter(fFed, HELICS_FILTER_TYPE_DELAY, "filter1", &err);
    EXPECT_EQ(err.error_code, HELICS_OK);
    CE(helicsFilterAddSourceTarget(filter1, "port1", &err));
    EXPECT_TRUE(filter1 != nullptr);
    CE(helicsFilterSet(filter1, "delay", 2.5, &err));

    CE(helicsFederateEnterExecutingModeAsync(fFed, &err));
    CE(helicsFederateEnterExecutingMode(mFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(fFed, &err));

    CE(HelicsFederateState state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_EXECUTION);
    std::string data(500, 'a');
    CE(helicsEndpointSendBytesTo(
        port1, data.c_str(), static_cast<int>(data.size()), "port2", &err));

    CE(helicsFederateRequestTimeAsync(mFed, 1.0, &err));
    CE(helicsFederateRequestTime(fFed, 1.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));

    auto res = helicsFederateHasMessage(mFed);
    EXPECT_TRUE(!res);

    CE(helicsFederateRequestTimeAsync(mFed, 2.0, &err));
    CE(helicsFederateRequestTime(fFed, 2.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));
    ASSERT_TRUE(!helicsEndpointHasMessage(port2));

    CE(helicsFederateRequestTimeAsync(fFed, 3.0, &err));
    CE(helicsFederateRequestTime(mFed, 3.0, &err));

    ASSERT_TRUE(helicsEndpointHasMessage(port2));

    auto message2 = helicsEndpointGetMessage(port2);
    EXPECT_STREQ(helicsMessageGetSource(message2), "port1");
    EXPECT_STREQ(helicsMessageGetOriginalSource(message2), "port1");
    EXPECT_STREQ(helicsMessageGetDestination(message2), "port2");
    EXPECT_EQ(helicsMessageGetByteCount(message2), static_cast<int>(data.size()));
    EXPECT_EQ(helicsMessageGetTime(message2), 2.5);

    CE(helicsFederateRequestTime(mFed, 3.0, &err));
    CE(helicsFederateRequestTimeComplete(fFed, &err));
    CE(helicsFederateFinalizeAsync(mFed, &err));
    CE(helicsFederateFinalize(fFed, &err));
    CE(helicsFederateFinalizeComplete(mFed, &err));
    CE(state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}

TEST_P(filter_simple_type_tests, function_mObj)
{
    HelicsBroker broker = AddBroker(GetParam(), 2);
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, 1.0, "filter");
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAt(0);
    auto mFed = GetFederateAt(1);

    CE(helicsFederateSetFlagOption(
        mFed, HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS, HELICS_TRUE, &err));
    auto port1 = helicsFederateRegisterGlobalEndpoint(mFed, "port1", nullptr, &err);
    auto port2 = helicsFederateRegisterGlobalEndpoint(mFed, "port2", "", &err);
    EXPECT_EQ(err.error_code, HELICS_OK);

    auto filter1 = helicsFederateRegisterFilter(fFed, HELICS_FILTER_TYPE_DELAY, "filter1", &err);
    EXPECT_EQ(err.error_code, HELICS_OK);
    CE(helicsFilterAddSourceTarget(filter1, "port1", &err));
    EXPECT_TRUE(filter1 != nullptr);
    CE(helicsFilterSet(filter1, "delay", 2.5, &err));
    CE(double val = helicsFilterGetPropertyDouble(filter1, "delay"));
    EXPECT_DOUBLE_EQ(val, 2.5);
    CE(helicsFederateEnterExecutingModeAsync(fFed, &err));
    CE(helicsFederateEnterExecutingMode(mFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(fFed, &err));

    CE(HelicsFederateState state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_EXECUTION);
    std::string data(500, 'a');
    CE(helicsEndpointSendBytesTo(
        port1, data.c_str(), static_cast<int>(data.size()), "port2", &err));

    CE(helicsFederateRequestTimeAsync(mFed, 1.0, &err));
    CE(helicsFederateRequestTime(fFed, 1.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));

    auto res = helicsFederateHasMessage(mFed);
    EXPECT_TRUE(!res);

    CE(helicsFederateRequestTimeAsync(mFed, 2.0, &err));
    CE(helicsFederateRequestTime(fFed, 2.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));
    ASSERT_TRUE(!helicsEndpointHasMessage(port2));

    CE(helicsFederateRequestTimeAsync(fFed, 3.0, &err));
    CE(helicsFederateRequestTime(mFed, 3.0, &err));

    ASSERT_TRUE(helicsEndpointHasMessage(port2));

    auto message2 = helicsEndpointGetMessage(port2);
    EXPECT_STREQ(helicsMessageGetSource(message2), "port1");
    EXPECT_STREQ(helicsMessageGetOriginalSource(message2), "port1");
    EXPECT_STREQ(helicsMessageGetDestination(message2), "port2");
    EXPECT_EQ(helicsMessageGetByteCount(message2), static_cast<int64_t>(data.size()));
    EXPECT_EQ(helicsMessageGetTime(message2), 2.5);

    CE(helicsFederateRequestTime(mFed, 3.0, &err));
    CE(helicsFederateRequestTimeComplete(fFed, &err));
    CE(helicsFederateFinalizeAsync(mFed, &err));
    CE(helicsFederateFinalize(fFed, &err));
    CE(helicsFederateFinalizeComplete(mFed, &err));
    CE(state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}
/** test a filter operator
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the
simulation
*/
TEST_P(filter_simple_type_tests, function_two_stage)
{
    HelicsBroker broker = AddBroker(GetParam(), 3);
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, 1.0, "filter");
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, 1.0, "filter2");
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAt(0);
    auto fFed2 = GetFederateAt(1);
    auto mFed = GetFederateAt(2);

    auto port1 = helicsFederateRegisterGlobalEndpoint(mFed, "port1", "", &err);
    auto port2 = helicsFederateRegisterGlobalEndpoint(mFed, "port2", "", &err);
    EXPECT_EQ(err.error_code, HELICS_OK);

    auto filter1 = helicsFederateRegisterFilter(fFed, HELICS_FILTER_TYPE_DELAY, "filter1", &err);
    EXPECT_EQ(err.error_code, HELICS_OK);
    CE(helicsFilterAddSourceTarget(filter1, "port1", &err));

    EXPECT_TRUE(filter1 != nullptr);
    CE(helicsFilterSet(filter1, "delay", 1.25, &err));

    CE(auto dfilter =
           helicsFederateRegisterFilter(fFed2, HELICS_FILTER_TYPE_DELAY, "filter2", &err));
    CE(helicsFilterAddSourceTarget(dfilter, "port1", &err));
    EXPECT_TRUE(dfilter != nullptr);
    CE(helicsFilterSet(dfilter, "delay", 1.25, &err));

    CE(helicsFederateEnterExecutingModeAsync(fFed, &err));
    CE(helicsFederateEnterExecutingModeAsync(fFed2, &err));
    CE(helicsFederateEnterExecutingMode(mFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(fFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(fFed2, &err));

    CE(HelicsFederateState state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_EXECUTION);
    std::string data(500, 'a');
    CE(helicsEndpointSendBytesTo(
        port1, data.c_str(), static_cast<int>(data.size()), "port2", &err));

    CE(helicsFederateRequestTimeAsync(mFed, .0, &err));
    CE(helicsFederateRequestTimeAsync(fFed, 1.0, &err));
    CE(helicsFederateRequestTime(fFed2, 1.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));
    CE(helicsFederateRequestTimeComplete(fFed, &err));
    auto res = helicsFederateHasMessage(mFed);
    EXPECT_TRUE(!res);

    CE(helicsFederateRequestTimeAsync(mFed, .0, &err));
    CE(helicsFederateRequestTimeAsync(fFed2, 2.0, &err));
    CE(helicsFederateRequestTime(fFed, 2.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));
    CE(helicsFederateRequestTimeComplete(fFed2, &err));
    ASSERT_TRUE(!helicsEndpointHasMessage(port2));

    CE(helicsFederateRequestTimeAsync(fFed, 3.0, &err));
    CE(helicsFederateRequestTimeAsync(fFed2, 3.0, &err));
    CE(helicsFederateRequestTime(mFed, 3.0, &err));
    if (helicsEndpointHasMessage(port2) == HELICS_FALSE) {
        printf("missing message\n");
    }
    ASSERT_TRUE(helicsEndpointHasMessage(port2));

    auto message2 = helicsEndpointGetMessage(port2);
    EXPECT_STREQ(helicsMessageGetSource(message2), "port1");
    EXPECT_STREQ(helicsMessageGetOriginalSource(message2), "port1");
    EXPECT_STREQ(helicsMessageGetDestination(message2), "port2");
    EXPECT_EQ(helicsMessageGetByteCount(message2), static_cast<int64_t>(data.size()));
    EXPECT_EQ(helicsMessageGetTime(message2), 2.5);

    CE(helicsFederateRequestTimeComplete(fFed, &err));
    CE(helicsFederateRequestTimeComplete(fFed2, &err));
    CE(helicsFederateFinalizeAsync(mFed, &err));
    CE(helicsFederateFinalizeAsync(fFed, &err));
    CE(helicsFederateFinalize(fFed2, &err));
    CE(helicsFederateFinalizeComplete(mFed, &err));
    CE(helicsFederateFinalizeComplete(fFed, &err));
    CE(state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}

/** test two filter operators
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the
simulation
*/

TEST_P(filter_simple_type_tests, function2)
{
    HelicsBroker broker = AddBroker(GetParam(), 2);
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, 1.0, "filter");
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAt(0);
    auto mFed = GetFederateAt(1);

    auto port1 = helicsFederateRegisterGlobalEndpoint(mFed, "port1", "", &err);
    auto port2 = helicsFederateRegisterGlobalEndpoint(mFed, "port2", "", &err);
    EXPECT_EQ(err.error_code, HELICS_OK);

    CE(auto filt1 = helicsFederateRegisterFilter(fFed, HELICS_FILTER_TYPE_DELAY, "filter1", &err));
    helicsFilterAddSourceTarget(filt1, "port1", nullptr);
    EXPECT_TRUE(filt1 != nullptr);
    CE(helicsFilterSet(filt1, "delay", 2.5, &err));

    CE(auto dfilter =
           helicsFederateRegisterFilter(fFed, HELICS_FILTER_TYPE_DELAY, "filter2", &err));
    helicsFilterAddSourceTarget(dfilter, "port2", nullptr);
    EXPECT_TRUE(dfilter != nullptr);
    CE(helicsFilterSet(dfilter, "delay", 2.5, &err));
    // this is expected to fail since a regular filter doesn't have a delivery endpoint
    helicsFilterAddDeliveryEndpoint(dfilter, "port1", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    CE(helicsFederateEnterExecutingModeAsync(fFed, &err));
    CE(helicsFederateEnterExecutingMode(mFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(fFed, &err));

    CE(HelicsFederateState state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_EXECUTION);
    std::string data(500, 'a');
    CE(helicsEndpointSendBytesTo(
        port1, data.c_str(), static_cast<int>(data.size()), "port2", &err));

    CE(helicsFederateRequestTimeAsync(mFed, 1.0, &err));
    CE(helicsFederateRequestTime(fFed, 1.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));

    auto res = helicsFederateHasMessage(mFed);
    EXPECT_TRUE(!res);
    CE(helicsEndpointSendBytesTo(
        port2, data.c_str(), static_cast<int>(data.size()), "port1", &err));
    CE(helicsFederateRequestTimeAsync(mFed, 2.0, &err));
    CE(helicsFederateRequestTime(fFed, 2.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));
    ASSERT_TRUE(!helicsEndpointHasMessage(port2));
    // there may be something wrong here yet but this test isn't the one to find it and
    // this may prevent spurious errors for now.
    std::this_thread::yield();
    CE(helicsFederateRequestTime(mFed, 3.0, &err));

    ASSERT_TRUE(helicsEndpointHasMessage(port2));

    auto message2 = helicsEndpointGetMessage(port2);
    EXPECT_STREQ(helicsMessageGetSource(message2), "port1");
    EXPECT_STREQ(helicsMessageGetOriginalSource(message2), "port1");
    EXPECT_STREQ(helicsMessageGetDestination(message2), "port2");
    EXPECT_EQ(helicsMessageGetByteCount(message2), static_cast<int64_t>(data.size()));
    EXPECT_EQ(helicsMessageGetTime(message2), 2.5);

    EXPECT_TRUE(!helicsEndpointHasMessage(port1));
    CE(helicsFederateRequestTime(mFed, 4.0, &err));
    EXPECT_TRUE(helicsEndpointHasMessage(port1));
    CE(helicsFederateFinalizeAsync(mFed, &err));
    CE(helicsFederateFinalize(fFed, &err));
    CE(helicsFederateFinalizeComplete(mFed, &err));
    CE(state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}

TEST_P(filter_simple_type_tests, message_filter_function3)
{
    HelicsBroker broker = AddBroker(GetParam(), 2);
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, 1.0, "filter");
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAt(0);
    auto mFed = GetFederateAt(1);

    auto port1 = helicsFederateRegisterGlobalEndpoint(mFed, "port1", "", &err);
    auto port2 = helicsFederateRegisterGlobalEndpoint(mFed, "port2", "random", &err);
    EXPECT_EQ(err.error_code, HELICS_OK);

    CE(auto filt1 =
           helicsFederateRegisterGlobalFilter(fFed, HELICS_FILTER_TYPE_CUSTOM, "filter1", &err));
    EXPECT_TRUE(filt1 != nullptr);
    helicsFilterAddSourceTarget(filt1, "port1", nullptr);
    CE(auto filt2 =
           helicsFederateRegisterGlobalFilter(fFed, HELICS_FILTER_TYPE_DELAY, "filter2", &err));
    helicsFilterAddSourceTarget(filt2, "port1", nullptr);

    helicsFederateRegisterEndpoint(fFed, "fout", "", &err);
    CE(auto filt3 =
           helicsFederateRegisterFilter(fFed, HELICS_FILTER_TYPE_RANDOM_DELAY, "filter3", &err));
    helicsFilterAddSourceTarget(filt3, "filter0/fout", nullptr);

    CE(helicsFilterSet(filt2, "delay", 2.5, &err));

    CE(helicsFederateEnterExecutingModeAsync(fFed, &err));
    CE(helicsFederateEnterExecutingMode(mFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(fFed, &err));

    CE(HelicsFederateState state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_EXECUTION);
    std::string data = "hello world";
    CE(helicsEndpointSendBytesTo(
        port1, data.c_str(), static_cast<int>(data.size()), "port2", &err));

    CE(helicsFederateRequestTimeAsync(mFed, 1.0, &err));
    CE(helicsFederateRequestTime(fFed, 1.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));

    auto res = helicsFederateHasMessage(mFed);
    EXPECT_TRUE(!res);
    CE(helicsEndpointSendBytesTo(
        port2, data.c_str(), static_cast<int>(data.size()), "port1", &err));
    CE(helicsFederateRequestTimeAsync(mFed, 2.0, &err));
    CE(helicsFederateRequestTime(fFed, 2.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));
    EXPECT_TRUE(!helicsEndpointHasMessage(port2));
    // there may be something wrong here yet but this test isn't the one to find it and
    // this may prevent spurious errors for now.
    std::this_thread::yield();
    CE(helicsFederateRequestTimeAsync(mFed, 3.0, &err));
    CE(helicsFederateRequestTime(fFed, 3.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));

    EXPECT_TRUE(helicsEndpointHasMessage(port2));

    auto message2 = helicsEndpointGetMessage(port2);
    EXPECT_STREQ(helicsMessageGetSource(message2), "port1");
    EXPECT_STREQ(helicsMessageGetOriginalSource(message2), "port1");
    EXPECT_STREQ(helicsMessageGetDestination(message2), "port2");
    EXPECT_EQ(helicsMessageGetByteCount(message2), static_cast<int64_t>(data.size()));
    EXPECT_EQ(helicsMessageGetTime(message2), 2.5);

    EXPECT_TRUE(helicsEndpointHasMessage(port1));
    CE(helicsFederateFinalize(mFed, &err));
    CE(helicsFederateFinalize(fFed, &err));
    CE(state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}

TEST_F(filter, clone_test)
{
    HelicsBroker broker = AddBroker("test", 3);
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "source");
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "dest");
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "dest_clone");

    auto sFed = GetFederateAt(0);
    auto dFed = GetFederateAt(1);
    auto dcFed = GetFederateAt(2);

    auto srcEpt = helicsFederateRegisterGlobalEndpoint(sFed, "src", "", &err);
    auto destEpt = helicsFederateRegisterGlobalEndpoint(dFed, "dest", "", &err);
    auto cmEpt = helicsFederateRegisterGlobalEndpoint(dcFed, "cm", "", &err);

    auto filt1 = helicsFederateRegisterCloningFilter(dcFed, nullptr, &err);
    CE(helicsFilterAddDeliveryEndpoint(filt1, "cm", &err));
    EXPECT_STREQ(helicsFilterGetPropertyString(filt1, "delivery"), "cm");
    EXPECT_TRUE(err.error_code == HELICS_OK);
    CE(helicsFilterAddSourceTarget(filt1, "src", &err));

    CE(helicsFederateEnterExecutingModeAsync(sFed, &err));
    CE(helicsFederateEnterExecutingModeAsync(dcFed, &err));

    CE(helicsFederateEnterExecutingModeAsync(dFed, &err));

    CE(helicsFederateEnterExecutingModeComplete(dFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(sFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(dcFed, &err));

    CE(HelicsFederateState state = helicsFederateGetState(sFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_EXECUTION);
    std::string data(500, 'a');
    CE(helicsEndpointSendBytesTo(
        srcEpt, data.c_str(), static_cast<int>(data.size()), "dest", &err));

    CE(helicsFederateRequestTimeAsync(sFed, 1.0, &err));
    CE(helicsFederateRequestTimeAsync(dcFed, 1.0, &err));
    CE(helicsFederateRequestTime(dFed, 1.0, &err));
    CE(helicsFederateRequestTimeComplete(sFed, &err));
    CE(helicsFederateRequestTimeComplete(dcFed, &err));

    auto res = helicsFederateHasMessage(dFed);
    EXPECT_EQ(res, HELICS_TRUE);

    if (res == HELICS_TRUE) {
        auto message2 = helicsEndpointGetMessage(destEpt);
        EXPECT_STREQ(helicsMessageGetSource(message2), "src");
        EXPECT_STREQ(helicsMessageGetOriginalSource(message2), "src");
        EXPECT_STREQ(helicsMessageGetDestination(message2), "dest");
        EXPECT_EQ(helicsMessageGetByteCount(message2), static_cast<int64_t>(data.size()));
    }

    // now check the message clone
    res = helicsFederateHasMessage(dcFed);
    EXPECT_EQ(res, HELICS_TRUE);

    if (res == HELICS_TRUE) {
        auto message2 = helicsEndpointGetMessage(cmEpt);
        EXPECT_STREQ(helicsMessageGetSource(message2), "src");
        EXPECT_STREQ(helicsMessageGetOriginalSource(message2), "src");
        EXPECT_STREQ(helicsMessageGetDestination(message2), "cm");
        EXPECT_STREQ(helicsMessageGetOriginalDestination(message2), "dest");
        EXPECT_EQ(helicsMessageGetByteCount(message2), static_cast<int64_t>(data.size()));
    }

    CE(helicsFederateFinalizeAsync(sFed, &err));
    CE(helicsFederateFinalizeAsync(dFed, &err));
    CE(helicsFederateFinalize(dcFed, &err));
    CE(helicsFederateFinalizeComplete(sFed, &err));
    CE(helicsFederateFinalizeComplete(dFed, &err));
    CE(state = helicsFederateGetState(sFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}

TEST_F(filter, clone_test_connections)
{
    HelicsBroker broker = AddBroker("test", 3);
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "source");
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "dest");
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "dest_clone");

    auto sFed = GetFederateAt(0);
    auto dFed = GetFederateAt(1);
    auto dcFed = GetFederateAt(2);

    auto srcEpt = helicsFederateRegisterGlobalEndpoint(sFed, "src", "", &err);
    auto destEpt = helicsFederateRegisterGlobalEndpoint(dFed, "dest", "", &err);
    auto cmEpt = helicsFederateRegisterGlobalEndpoint(dcFed, "cm", "", &err);

    auto filt1 = helicsFederateRegisterGlobalCloningFilter(dcFed, "filt1", &err);
    CE(helicsFilterAddDeliveryEndpoint(filt1, "cm", &err));
    EXPECT_TRUE(err.error_code == HELICS_OK);

    auto core = helicsFederateGetCore(sFed, &err);

    CE(helicsCoreAddSourceFilterToEndpoint(core, "filt1", "src", &err));

    // error test
    helicsCoreAddSourceFilterToEndpoint(core, nullptr, "src", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    CE(helicsFederateEnterExecutingModeAsync(sFed, &err));
    CE(helicsFederateEnterExecutingModeAsync(dcFed, &err));
    CE(helicsFederateEnterExecutingMode(dFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(sFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(dcFed, &err));

    // this is testing the filtered_endpoints query for cloning source filters
    auto query = helicsCreateQuery("", "filtered_endpoints");
    std::string filteredEndpoints = helicsQueryExecute(query, sFed, nullptr);
    // std::cout << filteredEndpoints << std::endl;
    EXPECT_TRUE(filteredEndpoints.find("(cloning)") != std::string::npos);
    EXPECT_TRUE(filteredEndpoints.find("srcFilters") != std::string::npos);
    helicsQueryFree(query);

    CE(HelicsFederateState state = helicsFederateGetState(sFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_EXECUTION);
    std::string data(500, 'a');
    CE(helicsEndpointSendBytesTo(
        srcEpt, data.c_str(), static_cast<int>(data.size()), "dest", &err));

    CE(helicsFederateRequestTimeAsync(sFed, 1.0, &err));
    CE(helicsFederateRequestTimeAsync(dcFed, 1.0, &err));
    CE(helicsFederateRequestTime(dFed, 1.0, &err));
    CE(helicsFederateRequestTimeComplete(sFed, &err));
    CE(helicsFederateRequestTimeComplete(dcFed, &err));

    auto res = helicsFederateHasMessage(dFed);
    EXPECT_EQ(res, HELICS_TRUE);

    if (res == HELICS_TRUE) {
        auto message2 = helicsEndpointGetMessage(destEpt);
        EXPECT_STREQ(helicsMessageGetSource(message2), "src");
        EXPECT_STREQ(helicsMessageGetOriginalSource(message2), "src");
        EXPECT_STREQ(helicsMessageGetDestination(message2), "dest");
        EXPECT_EQ(helicsMessageGetByteCount(message2), static_cast<int64_t>(data.size()));
    }

    // now check the message clone
    res = helicsFederateHasMessage(dcFed);
    EXPECT_EQ(res, HELICS_TRUE);

    if (res == HELICS_TRUE) {
        auto message2 = helicsEndpointGetMessage(cmEpt);
        EXPECT_STREQ(helicsMessageGetSource(message2), "src");
        EXPECT_STREQ(helicsMessageGetOriginalSource(message2), "src");
        EXPECT_STREQ(helicsMessageGetDestination(message2), "cm");
        EXPECT_STREQ(helicsMessageGetOriginalDestination(message2), "dest");
        EXPECT_EQ(helicsMessageGetByteCount(message2), static_cast<int64_t>(data.size()));
    }

    CE(helicsFederateFinalizeAsync(sFed, &err));
    CE(helicsFederateFinalizeAsync(dFed, &err));
    CE(helicsFederateFinalize(dcFed, &err));
    CE(helicsFederateFinalizeComplete(sFed, &err));
    CE(helicsFederateFinalizeComplete(dFed, &err));
    CE(state = helicsFederateGetState(sFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}

TEST_F(filter, clone_test_broker_connections)
{
    HelicsBroker broker = AddBroker("test", 3);
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "source");
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "dest");
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "dest_clone");

    auto sFed = GetFederateAt(0);
    auto dFed = GetFederateAt(1);
    auto dcFed = GetFederateAt(2);

    auto srcEpt = helicsFederateRegisterGlobalEndpoint(sFed, "src", "", &err);
    auto destEpt = helicsFederateRegisterGlobalEndpoint(dFed, "dest", "", &err);
    auto cmEpt = helicsFederateRegisterGlobalEndpoint(dcFed, "cm", "", &err);

    auto filt1 = helicsFederateRegisterGlobalCloningFilter(dcFed, "filt1", &err);
    CE(helicsFilterAddDeliveryEndpoint(filt1, "cm", &err));
    EXPECT_TRUE(err.error_code == HELICS_OK);

    CE(helicsBrokerAddSourceFilterToEndpoint(brokers[0], "filt1", "src", &err));

    // error test
    helicsBrokerAddSourceFilterToEndpoint(brokers[0], nullptr, "src", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    CE(helicsFederateEnterExecutingModeAsync(sFed, &err));
    CE(helicsFederateEnterExecutingModeAsync(dcFed, &err));
    CE(helicsFederateEnterExecutingMode(dFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(sFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(dcFed, &err));

    CE(HelicsFederateState state = helicsFederateGetState(sFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_EXECUTION);
    std::string data(500, 'a');
    CE(helicsEndpointSendBytesTo(
        srcEpt, data.c_str(), static_cast<int>(data.size()), "dest", &err));

    CE(helicsFederateRequestTimeAsync(sFed, 1.0, &err));
    CE(helicsFederateRequestTimeAsync(dcFed, 1.0, &err));
    CE(helicsFederateRequestTime(dFed, 1.0, &err));
    CE(helicsFederateRequestTimeComplete(sFed, &err));
    CE(helicsFederateRequestTimeComplete(dcFed, &err));

    auto res = helicsFederateHasMessage(dFed);
    EXPECT_EQ(res, HELICS_TRUE);

    if (res == HELICS_TRUE) {
        auto message2 = helicsEndpointGetMessage(destEpt);
        EXPECT_STREQ(helicsMessageGetSource(message2), "src");
        EXPECT_STREQ(helicsMessageGetOriginalSource(message2), "src");
        EXPECT_STREQ(helicsMessageGetDestination(message2), "dest");
        EXPECT_EQ(helicsMessageGetByteCount(message2), static_cast<int>(data.size()));
    }

    // now check the message clone
    res = helicsFederateHasMessage(dcFed);
    EXPECT_EQ(res, HELICS_TRUE);

    if (res == HELICS_TRUE) {
        auto message2 = helicsEndpointGetMessage(cmEpt);
        EXPECT_STREQ(helicsMessageGetSource(message2), "src");
        EXPECT_STREQ(helicsMessageGetOriginalSource(message2), "src");
        EXPECT_STREQ(helicsMessageGetDestination(message2), "cm");
        EXPECT_STREQ(helicsMessageGetOriginalDestination(message2), "dest");
        EXPECT_EQ(helicsMessageGetByteCount(message2), static_cast<int>(data.size()));
    }

    CE(helicsFederateFinalizeAsync(sFed, &err));
    CE(helicsFederateFinalizeAsync(dFed, &err));
    CE(helicsFederateFinalize(dcFed, &err));
    CE(helicsFederateFinalizeComplete(sFed, &err));
    CE(helicsFederateFinalizeComplete(dFed, &err));
    CE(state = helicsFederateGetState(sFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}

// this tests using a remote core to connect an endpoint to a cloning destination filter
TEST_F(filter, clone_test_dest_connections)
{
    HelicsBroker broker = AddBroker("test", 3);
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "source");
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "dest");
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 2.0, "dest_clone");

    auto sFed = GetFederateAt(0);
    auto dFed = GetFederateAt(1);
    auto dcFed = GetFederateAt(2);
    auto srcEpt = helicsFederateRegisterGlobalEndpoint(sFed, "src", "", &err);
    auto destEpt = helicsFederateRegisterGlobalEndpoint(dFed, "dest", "", &err);
    auto cmEpt = helicsFederateRegisterGlobalEndpoint(dcFed, "cm", "", &err);

    auto filt1 = helicsFederateRegisterGlobalCloningFilter(dcFed, "filt1", &err);
    CE(helicsFilterAddDeliveryEndpoint(filt1, "cm", &err));
    EXPECT_TRUE(err.error_code == HELICS_OK);

    auto core = helicsFederateGetCore(sFed, &err);

    CE(helicsCoreAddDestinationFilterToEndpoint(core, "filt1", "dest", &err));

    // error test
    helicsCoreAddDestinationFilterToEndpoint(core, nullptr, "dest", &err);

    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    helicsCoreFree(core);

    CE(helicsFederateEnterExecutingModeAsync(sFed, &err));
    CE(helicsFederateEnterExecutingModeAsync(dcFed, &err));
    CE(helicsFederateEnterExecutingMode(dFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(sFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(dcFed, &err));

    auto query = helicsCreateQuery("", "filtered_endpoints");
    std::string filteredEndpoints = helicsQueryExecute(query, dFed, nullptr);
    // std::cout << filteredEndpoints << std::endl;
    EXPECT_TRUE(filteredEndpoints.find("cloningdestFilter") != std::string::npos);
    helicsQueryFree(query);

    CE(HelicsFederateState state = helicsFederateGetState(sFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_EXECUTION);
    std::string data(500, 'a');
    CE(helicsEndpointSendBytesTo(
        srcEpt, data.c_str(), static_cast<int>(data.size()), "dest", &err));

    CE(helicsFederateFinalize(sFed, nullptr));

    HelicsMessage message2;
    auto dFedExec = [&]() {
        helicsFederateRequestTime(dFed, 1.0, nullptr);
        message2 = helicsEndpointGetMessage(destEpt);
        helicsFederateFinalize(dFed, nullptr);
    };

    HelicsMessage message3;
    auto dcFedExec = [&]() {
        helicsFederateRequestTime(dcFed, 2.0, nullptr);
        auto res = helicsFederateHasMessage(dcFed);
        if (res == HELICS_FALSE) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            helicsFederateRequestTime(dcFed, 4.0, nullptr);
        }
        message3 = helicsEndpointGetMessage(cmEpt);
        helicsFederateFinalize(dcFed, nullptr);
    };

    auto threaddFed = std::thread(dFedExec);
    auto threaddcFed = std::thread(dcFedExec);

    threaddFed.join();
    EXPECT_STREQ(helicsMessageGetSource(message2), "src");
    EXPECT_STREQ(helicsMessageGetOriginalSource(message2), "src");
    EXPECT_STREQ(helicsMessageGetDestination(message2), "dest");
    EXPECT_EQ(helicsMessageGetByteCount(message2), static_cast<int64_t>(data.size()));

    threaddcFed.join();

    EXPECT_STREQ(helicsMessageGetSource(message3), "src");
    EXPECT_STREQ(helicsMessageGetOriginalSource(message3), "src");
    EXPECT_STREQ(helicsMessageGetDestination(message3), "cm");
    EXPECT_STREQ(helicsMessageGetOriginalDestination(message3), "dest");
    EXPECT_EQ(helicsMessageGetByteCount(message3), static_cast<int64_t>(data.size()));

    CE(state = helicsFederateGetState(sFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}

TEST_F(filter, clone_test_broker_dest_connections)
{
    HelicsBroker broker = AddBroker("test", 3);
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "source");
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "dest");
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "dest_clone");

    auto sFed = GetFederateAt(0);
    auto dFed = GetFederateAt(1);
    auto dcFed = GetFederateAt(2);

    auto srcEpt = helicsFederateRegisterGlobalEndpoint(sFed, "src", "", &err);
    auto destEpt = helicsFederateRegisterGlobalEndpoint(dFed, "dest", "", &err);
    auto cmEpt = helicsFederateRegisterGlobalEndpoint(dcFed, "cm", "", &err);

    auto filt1 = helicsFederateRegisterGlobalCloningFilter(dcFed, "filt1", &err);
    CE(helicsFilterAddDeliveryEndpoint(filt1, "cm", &err));
    EXPECT_TRUE(err.error_code == HELICS_OK);

    CE(helicsBrokerAddDestinationFilterToEndpoint(brokers[0], "filt1", "dest", &err));

    // error test
    helicsBrokerAddDestinationFilterToEndpoint(brokers[0], nullptr, "dest", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    CE(helicsFederateEnterExecutingModeAsync(sFed, &err));
    CE(helicsFederateEnterExecutingModeAsync(dcFed, &err));
    CE(helicsFederateEnterExecutingMode(dFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(sFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(dcFed, &err));

    CE(HelicsFederateState state = helicsFederateGetState(sFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_EXECUTION);
    std::string data(500, 'a');
    CE(helicsEndpointSendBytesTo(
        srcEpt, data.c_str(), static_cast<int>(data.size()), "dest", &err));

    CE(helicsFederateRequestTimeAsync(sFed, 1.0, &err));
    CE(helicsFederateRequestTimeAsync(dcFed, 1.0, &err));
    CE(helicsFederateRequestTime(dFed, 1.0, &err));
    CE(helicsFederateRequestTimeComplete(sFed, &err));
    CE(helicsFederateRequestTimeComplete(dcFed, &err));

    auto res = helicsFederateHasMessage(dFed);
    EXPECT_EQ(res, HELICS_TRUE);

    if (res == HELICS_TRUE) {
        auto message2 = helicsEndpointGetMessage(destEpt);
        EXPECT_STREQ(helicsMessageGetSource(message2), "src");
        EXPECT_STREQ(helicsMessageGetOriginalSource(message2), "src");
        EXPECT_STREQ(helicsMessageGetDestination(message2), "dest");
        EXPECT_EQ(helicsMessageGetByteCount(message2), static_cast<int64_t>(data.size()));
    }
    CE(helicsFederateFinalizeAsync(sFed, &err));
    CE(helicsFederateFinalizeAsync(dFed, &err));

    // now check the message clone
    auto res2 = helicsFederateHasMessage(dcFed);

    if (res2 == HELICS_FALSE) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        CE(helicsFederateRequestTime(dcFed, 2.0, &err));
        res2 = helicsFederateHasMessage(dcFed);
    }

    EXPECT_EQ(res2, HELICS_TRUE);

    if (res2 == HELICS_TRUE) {
        auto message2 = helicsEndpointGetMessage(cmEpt);
        EXPECT_STREQ(helicsMessageGetSource(message2), "src");
        EXPECT_STREQ(helicsMessageGetOriginalSource(message2), "src");
        EXPECT_STREQ(helicsMessageGetDestination(message2), "cm");
        EXPECT_STREQ(helicsMessageGetOriginalDestination(message2), "dest");
        EXPECT_EQ(helicsMessageGetByteCount(message2), static_cast<int64_t>(data.size()));
    }

    CE(helicsFederateFinalize(dcFed, &err));
    CE(helicsFederateFinalizeComplete(sFed, &err));
    CE(helicsFederateFinalizeComplete(dFed, &err));
    CE(state = helicsFederateGetState(sFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}

TEST_F(filter, multi_clone_test)
{
    extraBrokerArgs = " --globaltime";
    HelicsBroker broker = AddBroker("test", 4);
    AddFederates(helicsCreateMessageFederate, "test", 2, broker, 1.0, "source");
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "dest");
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "dest_clone");

    auto sFed = GetFederateAt(0);
    auto sFed2 = GetFederateAt(1);
    auto dFed = GetFederateAt(2);
    auto dcFed = GetFederateAt(3);

    auto srcEpt = helicsFederateRegisterGlobalEndpoint(sFed, "src", "", &err);
    ASSERT_EQ(err.error_code, 0);
    auto src2Ept = helicsFederateRegisterGlobalEndpoint(sFed2, "src2", "", &err);
    ASSERT_EQ(err.error_code, 0);
    auto destEpt = helicsFederateRegisterGlobalEndpoint(dFed, "dest", "", &err);
    ASSERT_EQ(err.error_code, 0);
    auto cmEpt = helicsFederateRegisterGlobalEndpoint(dcFed, "cm", "", &err);
    ASSERT_EQ(err.error_code, 0);

    auto filt1 = helicsFederateRegisterCloningFilter(dcFed, "", &err);
    helicsFilterAddDeliveryEndpoint(filt1, "cm", nullptr);
    ASSERT_EQ(err.error_code, 0);
    CE(helicsFilterAddSourceTarget(filt1, "src", &err));
    CE(helicsFilterAddSourceTarget(filt1, "src2", &err));

    CE(helicsFederateEnterExecutingModeAsync(sFed, &err));
    CE(helicsFederateEnterExecutingModeAsync(sFed2, &err));
    CE(helicsFederateEnterExecutingModeAsync(dcFed, &err));
    CE(helicsFederateEnterExecutingMode(dFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(sFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(sFed2, &err));
    CE(helicsFederateEnterExecutingModeComplete(dcFed, &err));

    CE(HelicsFederateState state = helicsFederateGetState(sFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_EXECUTION);
    std::string data(500, 'a');
    std::string data2(400, 'b');
    CE(helicsEndpointSendBytesTo(
        srcEpt, data.c_str(), static_cast<int>(data.size()), "dest", &err));
    CE(helicsEndpointSendBytesTo(
        src2Ept, data2.c_str(), static_cast<int>(data2.size()), "dest", &err));

    CE(helicsFederateRequestTimeAsync(sFed, 1.0, &err));
    CE(helicsFederateRequestTimeAsync(sFed2, 1.0, &err));
    CE(helicsFederateRequestTimeAsync(dcFed, 1.0, &err));
    CE(helicsFederateRequestTime(dFed, 1.0, &err));
    CE(helicsFederateRequestTimeComplete(sFed, &err));
    CE(helicsFederateRequestTimeComplete(sFed2, &err));
    CE(helicsFederateRequestTimeComplete(dcFed, &err));

    auto mcnt = helicsEndpointPendingMessageCount(destEpt);
    EXPECT_EQ(mcnt, 2);
    auto res = helicsFederateHasMessage(dFed);
    EXPECT_EQ(res, HELICS_TRUE);

    if (res == HELICS_TRUE) {
        auto message2 = helicsEndpointGetMessage(destEpt);
        EXPECT_STREQ(helicsMessageGetSource(message2), "src");
        EXPECT_STREQ(helicsMessageGetOriginalSource(message2), "src");
        EXPECT_STREQ(helicsMessageGetDestination(message2), "dest");
        EXPECT_EQ(helicsMessageGetByteCount(message2), static_cast<int64_t>(data.size()));
        res = helicsFederateHasMessage(dFed);
        EXPECT_EQ(res, HELICS_TRUE);

        if (res == HELICS_TRUE) {
            message2 = helicsFederateGetMessage(dFed);
            EXPECT_STREQ(helicsMessageGetSource(message2), "src2");
            EXPECT_STREQ(helicsMessageGetOriginalSource(message2), "src2");
            EXPECT_STREQ(helicsMessageGetDestination(message2), "dest");
            EXPECT_EQ(helicsMessageGetByteCount(message2), static_cast<int64_t>(data2.size()));
        }
    }

    // now check the message clone
    mcnt = helicsEndpointPendingMessageCount(cmEpt);
    EXPECT_EQ(mcnt, 2);
    res = helicsFederateHasMessage(dcFed);
    EXPECT_EQ(res, HELICS_TRUE);

    if (res != HELICS_FALSE) {
        auto message2 = helicsFederateGetMessage(dcFed);
        EXPECT_STREQ(helicsMessageGetSource(message2), "src");
        EXPECT_STREQ(helicsMessageGetOriginalSource(message2), "src");
        EXPECT_STREQ(helicsMessageGetDestination(message2), "cm");
        EXPECT_STREQ(helicsMessageGetOriginalDestination(message2), "dest");
        EXPECT_EQ(helicsMessageGetByteCount(message2), static_cast<int64_t>(data.size()));
        res = helicsFederateHasMessage(dcFed);
        EXPECT_EQ(res, HELICS_TRUE);

        if (res != HELICS_FALSE) {
            message2 = helicsFederateGetMessage(dcFed);
            EXPECT_STREQ(helicsMessageGetSource(message2), "src2");
            EXPECT_STREQ(helicsMessageGetOriginalSource(message2), "src2");
            EXPECT_STREQ(helicsMessageGetDestination(message2), "cm");
            EXPECT_STREQ(helicsMessageGetOriginalDestination(message2), "dest");
            EXPECT_EQ(helicsMessageGetByteCount(message2), static_cast<int64_t>(data2.size()));
        }
    }

    CE(helicsFederateFinalizeAsync(sFed, &err));
    CE(helicsFederateFinalizeAsync(sFed2, &err));
    CE(helicsFederateFinalizeAsync(dFed, &err));
    CE(helicsFederateFinalize(dcFed, &err));
    CE(helicsFederateFinalizeComplete(sFed, &err));
    CE(helicsFederateFinalizeComplete(sFed2, &err));
    CE(helicsFederateFinalizeComplete(dFed, &err));
    CE(state = helicsFederateGetState(sFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}

TEST_F(filter, file_load)
{
    std::string filename = std::string(TEST_DIR) + "/example_filters.json";
    auto mFed = helicsCreateMessageFederateFromConfig(filename.c_str(), &err);

    const char* name = helicsFederateGetName(mFed);
    EXPECT_STREQ(name, "filterFed");

    EXPECT_EQ(helicsFederateGetEndpointCount(mFed), 3);
    helicsFederateFinalize(mFed, nullptr);
    helicsFederateFree(mFed);
    // auto id = mFed.getEndpointId ("ept1");
    // EXPECT_EQ (mFed.getEndpointType (id), "genmessage");

    // EXPECT_EQ (mFed.filterObjectCount (), 3);

    // auto filt = mFed.getFilterObject (2);

    // auto cloneFilt = std::dynamic_pointer_cast<helics::CloningFilter> (filt);
    // EXPECT_TRUE (cloneFilt);
    // mFed.disconnect ();
}

static HelicsMessage filterFunc1(HelicsMessage mess, void* /*unused*/)
{
    auto time = helicsMessageGetTime(mess);
    helicsMessageSetTime(mess, time + 2.5, nullptr);
    return mess;
}

TEST_F(filter, callbacks)
{
    HelicsBroker broker = AddBroker("test", 2);
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "filter");
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "message");

    auto fFed = GetFederateAt(0);
    auto mFed = GetFederateAt(1);

    CE(helicsFederateSetFlagOption(
        mFed, HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS, HELICS_TRUE, &err));
    auto port1 = helicsFederateRegisterGlobalEndpoint(mFed, "port1", nullptr, &err);
    auto port2 = helicsFederateRegisterGlobalEndpoint(mFed, "port2", "", &err);
    EXPECT_EQ(err.error_code, HELICS_OK);

    auto filter1 = helicsFederateRegisterFilter(fFed, HELICS_FILTER_TYPE_CUSTOM, "filter1", &err);
    auto dfilter = helicsFederateRegisterFilter(mFed, HELICS_FILTER_TYPE_DELAY, "dfilter", &err);

    EXPECT_EQ(err.error_code, HELICS_OK);
    CE(helicsFilterAddSourceTarget(filter1, "port1", &err));
    CE(helicsFilterSetCustomCallback(filter1, filterFunc1, nullptr, &err));
    EXPECT_EQ(err.error_code, 0);

    helicsFilterSetCustomCallback(dfilter, filterFunc1, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    CE(helicsFederateEnterExecutingModeAsync(fFed, &err));
    CE(helicsFederateEnterExecutingMode(mFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(fFed, &err));

    CE(HelicsFederateState state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_EXECUTION);
    std::string data(500, 'a');
    CE(helicsEndpointSendBytesTo(
        port1, data.c_str(), static_cast<int>(data.size()), "port2", &err));

    CE(helicsFederateRequestTimeAsync(mFed, 1.0, &err));
    CE(helicsFederateRequestTime(fFed, 1.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));

    auto res = helicsFederateHasMessage(mFed);
    EXPECT_TRUE(!res);

    CE(helicsFederateRequestTimeAsync(mFed, 2.0, &err));
    CE(helicsFederateRequestTime(fFed, 2.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));
    ASSERT_TRUE(!helicsEndpointHasMessage(port2));

    CE(helicsFederateRequestTimeAsync(fFed, 3.0, &err));
    CE(helicsFederateRequestTime(mFed, 3.0, &err));

    ASSERT_TRUE(helicsEndpointHasMessage(port2));

    auto message2 = helicsEndpointGetMessage(port2);
    EXPECT_STREQ(helicsMessageGetSource(message2), "port1");
    EXPECT_STREQ(helicsMessageGetOriginalSource(message2), "port1");
    EXPECT_STREQ(helicsMessageGetDestination(message2), "port2");
    EXPECT_EQ(helicsMessageGetByteCount(message2), static_cast<int64_t>(data.size()));
    EXPECT_EQ(helicsMessageGetTime(message2), 2.5);

    CE(helicsFederateRequestTime(mFed, 3.0, &err));
    CE(helicsFederateRequestTimeComplete(fFed, &err));
    CE(helicsFederateFinalizeAsync(mFed, &err));
    CE(helicsFederateFinalize(fFed, &err));
    CE(helicsFederateFinalizeComplete(mFed, &err));
    CE(state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}

INSTANTIATE_TEST_SUITE_P(filter, filter_simple_type_tests, ::testing::ValuesIn(CoreTypes_simple));
/*
INSTANTIATE_TEST_SUITE_P(filter, filter_type_tests, ::testing::ValuesIn(CoreTypes));
*/
