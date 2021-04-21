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
#include <thread>
// these test cases test out the message federates

class mfed_simple_type_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {
};

class mfed_type_tests: public ::testing::TestWithParam<const char*>, public FederateTestFixture {
};

class mfed_tests: public FederateTestFixture, public ::testing::Test {
};
/** test simple creation and destruction*/
TEST_P(mfed_simple_type_tests, initialize_tests)
{
    SetupTest(helicsCreateMessageFederate, GetParam(), 1);
    auto mFed1 = GetFederateAt(0);

    CE(helicsFederateEnterExecutingMode(mFed1, &err));

    CE(helics_federate_state mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == helics_state_execution);

    CE(helicsFederateFinalize(mFed1, &err));

    CE(mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == helics_federate_state::helics_state_finalize);
}

TEST_P(mfed_simple_type_tests, endpoint_registration)
{
    SetupTest(helicsCreateMessageFederate, GetParam(), 1);
    auto mFed1 = GetFederateAt(0);

    auto epid = helicsFederateRegisterEndpoint(mFed1, "ep1", nullptr, &err);
    auto epid2 = helicsFederateRegisterGlobalEndpoint(mFed1, "ep2", "random", &err);
    EXPECT_EQ(err.error_code, helics_ok);
    CE(helicsFederateEnterExecutingMode(mFed1, &err));

    CE(helics_federate_state mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == helics_state_execution);

    const char* name = helicsEndpointGetName(epid);
    EXPECT_STREQ(name, "fed0/ep1");
    name = helicsEndpointGetName(epid2);
    EXPECT_STREQ(name, "ep2");

    const char* type = helicsEndpointGetType(epid);
    const char* type2 = helicsEndpointGetType(epid2);
    EXPECT_STREQ(type, "");
    EXPECT_STREQ(type2, "random");

    auto epid_b = helicsFederateGetEndpoint(mFed1, "ep2", &err);
    type = helicsEndpointGetType(epid_b);
    EXPECT_STREQ(type, "random");

    auto epid_c = helicsFederateGetEndpointByIndex(mFed1, 0, &err);
    name = helicsEndpointGetName(epid_c);
    EXPECT_STREQ(name, "fed0/ep1");

    CE(helicsFederateFinalize(mFed1, &err));

    CE(mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == helics_federate_state::helics_state_finalize);
}

TEST_P(mfed_simple_type_tests, send_receive)
{
    SetupTest(helicsCreateMessageFederate, GetParam(), 1);
    auto mFed1 = GetFederateAt(0);

    auto epid = helicsFederateRegisterEndpoint(mFed1, "ep1", nullptr, &err);
    auto epid2 = helicsFederateRegisterGlobalEndpoint(mFed1, "ep2", "random", &err);
    EXPECT_EQ(err.error_code, helics_ok);
    CE(helicsFederateSetTimeProperty(mFed1, helics_property_time_delta, 1.0, &err));

    CE(helicsFederateEnterExecutingMode(mFed1, &err));

    CE(helics_federate_state mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == helics_state_execution);
    std::string data(500, 'a');

    CE(helicsEndpointSendEventRaw(epid, "ep2", data.c_str(), 500, 0.0, &err));
    helics_time time;
    CE(time = helicsFederateRequestTime(mFed1, 1.0, &err));
    EXPECT_EQ(time, 1.0);

    auto res = helicsFederateHasMessage(mFed1);
    EXPECT_TRUE(res);
    res = helicsEndpointHasMessage(epid);
    EXPECT_TRUE(res == false);
    res = helicsEndpointHasMessage(epid2);
    EXPECT_TRUE(res);

    auto M = helicsEndpointGetMessageObject(epid2);
    // ASSERT_TRUE (M);
    ASSERT_EQ(helicsMessageGetRawDataSize(M), 500);
    char* dptr = reinterpret_cast<char*>(helicsMessageGetRawDataPointer(M));
    if (dptr != nullptr) {
        EXPECT_EQ(dptr[245], 'a');
    } else {
        ASSERT_TRUE(false) << "data is nullptr";
    }
    CE(helicsFederateFinalize(mFed1, &err));

    CE(mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == helics_federate_state::helics_state_finalize);
}

TEST_P(mfed_simple_type_tests, send_receive_mobj)
{
    SetupTest(helicsCreateMessageFederate, GetParam(), 1);
    auto mFed1 = GetFederateAt(0);

    auto epid = helicsFederateRegisterEndpoint(mFed1, "ep1", nullptr, &err);
    auto epid2 = helicsFederateRegisterGlobalEndpoint(mFed1, "ep2", "random", &err);
    EXPECT_EQ(err.error_code, helics_ok);
    CE(helicsFederateSetTimeProperty(mFed1, helics_property_time_delta, 1.0, &err));

    CE(helicsFederateEnterExecutingMode(mFed1, &err));

    CE(helics_federate_state mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == helics_state_execution);
    std::string data(500, 'a');

    CE(helicsEndpointSendEventRaw(epid, "ep2", data.c_str(), 500, 0.0, &err));
    helics_time time;
    CE(time = helicsFederateRequestTime(mFed1, 1.0, &err));
    EXPECT_EQ(time, 1.0);

    auto res = helicsFederateHasMessage(mFed1);
    EXPECT_TRUE(res);
    res = helicsEndpointHasMessage(epid);
    EXPECT_TRUE(res == false);
    res = helicsEndpointHasMessage(epid2);
    EXPECT_TRUE(res);

    auto M = helicsEndpointGetMessageObject(epid2);
    // ASSERT_TRUE (M);
    ASSERT_EQ(helicsMessageGetRawDataSize(M), 500);

    char* rdata = static_cast<char*>(helicsMessageGetRawDataPointer(M));
    if (rdata != nullptr) {
        EXPECT_EQ(rdata[245], 'a');
    } else {
        EXPECT_TRUE(false) << "data is null";
    }
    CE(helicsFederateFinalize(mFed1, &err));

    CE(mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == helics_federate_state::helics_state_finalize);
}

TEST_F(mfed_tests, message_object_tests)
{
    SetupTest(helicsCreateMessageFederate, "test", 1);
    auto mFed1 = GetFederateAt(0);

    auto epid = helicsFederateRegisterEndpoint(mFed1, "ep1", nullptr, &err);
    auto epid2 = helicsFederateRegisterGlobalEndpoint(mFed1, "ep2", "random", &err);
    EXPECT_EQ(err.error_code, helics_ok);
    CE(helicsFederateSetTimeProperty(mFed1, helics_property_time_delta, 1.0, &err));

    CE(helicsFederateEnterExecutingMode(mFed1, &err));

    CE(helics_federate_state mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == helics_state_execution);
    std::string data(500, 'a');

    auto M = helicsFederateCreateMessageObject(mFed1, nullptr);
    helicsMessageSetDestination(M, "ep2", nullptr);
    EXPECT_STREQ(helicsMessageGetDestination(M), "ep2");
    helicsMessageSetData(M, data.data(), 500, &err);
    helicsMessageSetTime(M, 0.0, &err);

    CE(helicsEndpointSendMessageObject(epid, M, &err));
    helics_time time;
    CE(time = helicsFederateRequestTime(mFed1, 1.0, &err));
    EXPECT_EQ(time, 1.0);

    auto res = helicsFederateHasMessage(mFed1);
    EXPECT_TRUE(res);
    res = helicsEndpointHasMessage(epid);
    EXPECT_TRUE(res == false);
    res = helicsEndpointHasMessage(epid2);
    EXPECT_TRUE(res);

    M = helicsEndpointGetMessageObject(epid2);
    // ASSERT_TRUE (M);
    ASSERT_EQ(helicsMessageGetRawDataSize(M), 500);

    char* rdata = static_cast<char*>(helicsMessageGetRawDataPointer(M));
    EXPECT_NE(rdata, nullptr);
    if (rdata != nullptr) {
        EXPECT_EQ(rdata[245], 'a');
    }

    CE(helicsFederateFinalize(mFed1, &err));

    CE(mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == helics_federate_state::helics_state_finalize);

    helicsMessageSetFlagOption(M, 7, helics_true, &err);
    EXPECT_TRUE(helicsMessageCheckFlag(M, 7) == helics_true);
    helicsMessageClearFlags(M);
    EXPECT_TRUE(helicsMessageCheckFlag(M, 7) == helics_false);
}

TEST_P(mfed_type_tests, send_receive_2fed)
{
    // extraBrokerArgs = "--loglevel=4";
    SetupTest(helicsCreateMessageFederate, GetParam(), 2);
    auto mFed1 = GetFederateAt(0);
    auto mFed2 = GetFederateAt(1);
    helicsFederateSetIntegerProperty(mFed1, helics_property_int_console_log_level, 0, &err);
    // mFed2->setLoggingLevel(4);
    CE(auto epid = helicsFederateRegisterEndpoint(mFed1, "ep1", nullptr, &err));
    CE(auto epid2 = helicsFederateRegisterGlobalEndpoint(mFed2, "ep2", "random", &err));

    helicsEndpointSetOption(epid, helics_handle_option_ignore_interrupts, helics_true, &err);
    EXPECT_EQ(err.error_code, 0);

    CE(helicsFederateSetTimeProperty(mFed1, helics_property_time_delta, 1.0, &err));
    CE(helicsFederateSetTimeProperty(mFed2, helics_property_time_delta, 1.0, &err));

    CE(helicsFederateEnterExecutingModeAsync(mFed1, &err));
    CE(helicsFederateEnterExecutingMode(mFed2, &err));
    CE(helicsFederateEnterExecutingModeComplete(mFed1, &err));

    CE(helics_federate_state mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == helics_state_execution);
    CE(helics_federate_state mFed2State = helicsFederateGetState(mFed2, &err));
    EXPECT_TRUE(mFed2State == helics_state_execution);

    int val = helicsFederateGetIntegerProperty(mFed1, helics_property_int_console_log_level, &err);
    EXPECT_EQ(val, 0);

    std::string data(500, 'a');
    std::string data2(400, 'b');

    CE(helicsEndpointSendEventRaw(epid, "ep2", data.c_str(), 500, 0.0, &err));
    CE(helicsEndpointSendEventRaw(epid2, "fed0/ep1", data2.c_str(), 400, 0.0, &err));
    // move the time to 1.0
    helics_time time;
    CE(helicsFederateRequestTimeAsync(mFed1, 1.0, &err));
    helics_time gtime;
    CE(gtime = helicsFederateRequestTime(mFed2, 1.0, &err));
    CE(time = helicsFederateRequestTimeComplete(mFed1, &err));

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(time, 1.0);

    auto opt_val = helicsEndpointGetOption(epid, helics_handle_option_ignore_interrupts);
    // someday this might get implemented but for now it isn't so we expect false
    EXPECT_EQ(opt_val, false);

    auto res = helicsFederateHasMessage(mFed1);
    EXPECT_TRUE(res);
    res = helicsEndpointHasMessage(epid);
    EXPECT_TRUE(res);
    res = helicsEndpointHasMessage(epid2);
    EXPECT_TRUE(res);

    auto M1 = helicsEndpointGetMessageObject(epid);
    // ASSERT_TRUE(M1);
    EXPECT_EQ(helicsMessageGetRawDataSize(M1), 400);
    auto dptr = static_cast<char*>(helicsMessageGetRawDataPointer(M1));
    EXPECT_NE(dptr, nullptr);
    if (dptr != nullptr) {
        EXPECT_EQ(dptr[245], 'b');
    }

    auto M2 = helicsEndpointGetMessageObject(epid2);
    // ASSERT_TRUE(M2);
    EXPECT_EQ(helicsMessageGetRawDataSize(M2), 500);
    dptr = static_cast<char*>(helicsMessageGetRawDataPointer(M2));
    EXPECT_NE(dptr, nullptr);
    if (dptr != nullptr) {
        EXPECT_EQ(dptr[245], 'a');
    }
    CE(helicsFederateFinalizeAsync(mFed1, &err));
    CE(helicsFederateFinalize(mFed2, &err));
    CE(helicsFederateFinalizeComplete(mFed1, &err));
    CE(mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == helics_federate_state::helics_state_finalize);
    CE(mFed2State = helicsFederateGetState(mFed2, &err));
    EXPECT_TRUE(mFed2State == helics_federate_state::helics_state_finalize);
}

TEST_P(mfed_type_tests, send_receive_2fed_multisend)
{
    // extraBrokerArgs = "--loglevel=4";
    SetupTest(helicsCreateMessageFederate, GetParam(), 2);
    auto mFed1 = GetFederateAt(0);
    auto mFed2 = GetFederateAt(1);
    // mFed1->setLoggingLevel(4);
    // mFed2->setLoggingLevel(4);
    CE(auto epid = helicsFederateRegisterEndpoint(mFed1, "ep1", nullptr, &err));
    CE(auto epid2 = helicsFederateRegisterGlobalEndpoint(mFed2, "ep2", "random", &err));

    EXPECT_EQ(helicsEndpointIsValid(epid), helics_true);
    CE(helicsFederateSetTimeProperty(mFed1, helics_property_time_delta, 1.0, &err));
    CE(helicsFederateSetTimeProperty(mFed2, helics_property_time_delta, 1.0, &err));

    CE(helicsFederateEnterExecutingModeAsync(mFed1, &err));
    CE(helicsFederateEnterExecutingMode(mFed2, &err));
    CE(helicsFederateEnterExecutingModeComplete(mFed1, &err));

    CE(helics_federate_state mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == helics_state_execution);
    CE(helics_federate_state mFed2State = helicsFederateGetState(mFed2, &err));
    EXPECT_TRUE(mFed2State == helics_state_execution);

    std::string data(500, 'a');

    helicsEndpointSetDefaultDestination(epid, "ep2", &err);

    CE(helicsEndpointSendMessageRaw(epid, nullptr, data.c_str(), 500, &err));
    CE(helicsEndpointSendMessageRaw(epid, nullptr, data.c_str(), 400, &err));
    CE(helicsEndpointSendMessageRaw(epid, nullptr, data.c_str(), 300, &err));

    // move the time to 1.0
    helics_time time;
    CE(helicsFederateRequestTimeAsync(mFed1, 1.0, &err));
    helics_time gtime;
    CE(gtime = helicsFederateRequestTime(mFed2, 1.0, &err));
    CE(time = helicsFederateRequestTimeComplete(mFed1, &err));

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(time, 1.0);

    auto res = helicsEndpointPendingMessages(epid2);
    EXPECT_EQ(res, 3);

    res = helicsFederatePendingMessages(mFed2);
    EXPECT_EQ(res, 3);

    EXPECT_STREQ(helicsEndpointGetDefaultDestination(epid), "ep2");

    CE(helicsFederateFinalizeAsync(mFed1, &err));
    CE(helicsFederateFinalize(mFed2, &err));
    CE(helicsFederateFinalizeComplete(mFed1, &err));
    CE(mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == helics_federate_state::helics_state_finalize);
    CE(mFed2State = helicsFederateGetState(mFed2, &err));
    EXPECT_TRUE(mFed2State == helics_federate_state::helics_state_finalize);
}

INSTANTIATE_TEST_SUITE_P(mfed_tests,
                         mfed_simple_type_tests,
                         ::testing::ValuesIn(core_types_simple));
INSTANTIATE_TEST_SUITE_P(mfed_tests, mfed_type_tests, ::testing::ValuesIn(core_types));

// a series of tests exercising the different aspects of message object setting and retrieval
TEST(message_object, test1)
{
    auto brk = helicsCreateBroker("test", "brk1", "", nullptr);

    auto fi = helicsCreateFederateInfo();
    helicsFederateInfoSetCoreType(fi, helics_core_type_test, nullptr);

    auto fed = helicsCreateMessageFederate("fed1", fi, nullptr);

    auto fed2 = helicsCreateCombinationFederate("fed2", fi, nullptr);

    auto m1 = helicsFederateCreateMessageObject(fed, nullptr);
    EXPECT_NE(m1, nullptr);

    auto m2 = helicsFederateCreateMessageObject(fed2, nullptr);
    EXPECT_NE(m2, nullptr);

    helicsMessageSetOriginalDestination(m1, "a happy place", nullptr);
    EXPECT_STREQ(helicsMessageGetOriginalDestination(m1), "a happy place");

    helicsMessageSetOriginalSource(m1, "osource", nullptr);
    EXPECT_STREQ(helicsMessageGetOriginalSource(m1), "osource");

    helicsMessageSetMessageID(m1, 10, nullptr);
    EXPECT_EQ(helicsMessageGetMessageID(m1), 10);

    // 89 is an invalid flag
    EXPECT_EQ(helicsMessageCheckFlag(m1, 89), helics_false);

    helicsMessageSetString(m2, "raw data", nullptr);
    EXPECT_STREQ(helicsMessageGetString(m2), "raw data");

    auto err = helicsErrorInitialize();

    char data[20];
    int actSize = 10;
    helicsMessageGetRawData(m2, nullptr, 0, &actSize, &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(actSize, 0);
    helicsErrorClear(&err);

    EXPECT_EQ(helicsMessageGetRawDataPointer(nullptr), nullptr);

    helicsMessageGetRawData(m2, data, 20, &actSize, &err);
    EXPECT_EQ(err.error_code, 0);
    EXPECT_EQ(actSize, 8);
    EXPECT_EQ(std::string(data, data + actSize), "raw data");

    EXPECT_EQ(helicsMessageIsValid(m2), helics_true);

    helicsMessageSetSource(m1, "source", nullptr);
    EXPECT_STREQ(helicsMessageGetSource(m1), "source");

    helicsMessageSetFlagOption(m1, 4, helics_true, nullptr);
    EXPECT_EQ(helicsMessageCheckFlag(m1, 4), helics_true);

    helicsMessageSetFlagOption(m1, 4, helics_false, nullptr);
    EXPECT_EQ(helicsMessageCheckFlag(m1, 4), helics_false);

    helicsMessageSetFlagOption(m1, 22, helics_true, &err);
    EXPECT_EQ(helicsMessageCheckFlag(m1, 22), helics_false);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsMessageResize(m2, 500, nullptr);
    auto s2 = helicsMessageGetRawDataSize(m2);
    EXPECT_EQ(s2, 500);

    helicsMessageReserve(m2, 2000, nullptr);

    helicsMessageAppendData(m2, " more data", 8, nullptr);

    s2 = helicsMessageGetRawDataSize(m2);
    EXPECT_EQ(s2, 508);

    // this should generate an out of memory exception
    helicsMessageResize(m2, -8, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    // this should generate an out of memory exception
    helicsMessageReserve(m2, -2, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    helicsFederateInfoFree(fi);
    helicsFederateFinalize(fed, nullptr);
    helicsFederateFinalize(fed2, nullptr);
    helicsBrokerDisconnect(brk, nullptr);
}

TEST(message_object, copy)
{
    auto brk = helicsCreateBroker("test", "brk1", "", nullptr);

    auto fi = helicsCreateFederateInfo();
    helicsFederateInfoSetCoreType(fi, helics_core_type_test, nullptr);
    auto fed = helicsCreateMessageFederate("fed1", fi, nullptr);

    helicsFederateInfoFree(fi);
    auto m1 = helicsFederateCreateMessageObject(fed, nullptr);
    EXPECT_NE(m1, nullptr);

    auto m2 = helicsFederateCreateMessageObject(fed, nullptr);
    EXPECT_NE(m2, nullptr);

    helicsMessageSetDestination(m1, "a small town", nullptr);
    helicsMessageSetSource(m1, "toledo", nullptr);

    helicsMessageSetOriginalDestination(m1, "a happy place", nullptr);
    helicsMessageSetOriginalSource(m1, "osource", nullptr);
    helicsMessageSetMessageID(m1, 10, nullptr);
    helicsMessageSetFlagOption(m1, 4, helics_true, nullptr);
    helicsMessageSetString(m1, "raw data", nullptr);
    helicsMessageSetTime(m1, 3.65, nullptr);
    auto err = helicsErrorInitialize();

    helicsMessageCopy(m1, m2, &err);
    EXPECT_EQ(err.error_code, 0);

    helicsMessageCopy(m1, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    EXPECT_STREQ(helicsMessageGetString(m2), "raw data");
    EXPECT_STREQ(helicsMessageGetOriginalSource(m2), "osource");
    EXPECT_STREQ(helicsMessageGetSource(m2), "toledo");
    EXPECT_STREQ(helicsMessageGetDestination(m2), "a small town");
    EXPECT_EQ(helicsMessageGetMessageID(m2), 10);
    EXPECT_DOUBLE_EQ(helicsMessageGetTime(m2), 3.65);

    EXPECT_STREQ(helicsMessageGetOriginalDestination(m2), "a happy place");

    char data[20];
    int actSize = 10;

    helicsMessageGetRawData(m2, data, 20, &actSize, &err);
    EXPECT_EQ(err.error_code, 0);
    EXPECT_EQ(actSize, 8);
    EXPECT_EQ(std::string(data, data + actSize), "raw data");

    EXPECT_EQ(helicsMessageIsValid(m2), helics_true);

    EXPECT_EQ(helicsMessageCheckFlag(m2, 4), helics_true);

    helicsMessageClear(m2, &err);

    EXPECT_EQ(helicsMessageIsValid(m2), helics_false);

    EXPECT_EQ(helicsMessageCheckFlag(m2, 4), helics_false);

    EXPECT_EQ(helicsMessageGetRawDataSize(m2), 0);

    helicsFederateEnterExecutingMode(fed, nullptr);
    helicsFederateFinalize(fed, nullptr);
    helicsBrokerDisconnect(brk, nullptr);
}
