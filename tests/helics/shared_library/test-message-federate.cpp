/*
Copyright (c) 2017-2024,
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
    public FederateTestFixture {};

class mfed_type_tests: public ::testing::TestWithParam<const char*>, public FederateTestFixture {};

class mfed_tests: public FederateTestFixture, public ::testing::Test {};
/** test simple creation and destruction*/
TEST_P(mfed_simple_type_tests, initialize_tests)
{
    SetupTest(helicsCreateMessageFederate, GetParam(), 1);
    auto mFed1 = GetFederateAt(0);

    CE(helicsFederateEnterExecutingMode(mFed1, &err));

    CE(HelicsFederateState mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == HELICS_STATE_EXECUTION);

    CE(helicsFederateFinalize(mFed1, &err));

    CE(mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == HelicsFederateState::HELICS_STATE_FINALIZE);
}

TEST_P(mfed_simple_type_tests, endpoint_registration)
{
    SetupTest(helicsCreateMessageFederate, GetParam(), 1);
    auto mFed1 = GetFederateAt(0);

    auto epid = helicsFederateRegisterEndpoint(mFed1, "ep1", nullptr, &err);
    auto epid2 = helicsFederateRegisterGlobalEndpoint(mFed1, "ep2", "random", &err);
    EXPECT_EQ(err.error_code, HELICS_OK);
    CE(helicsFederateEnterExecutingMode(mFed1, &err));

    CE(HelicsFederateState mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == HELICS_STATE_EXECUTION);

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

    CE(helicsEndpointSetTag(epid_b, "tag1", "tagvalue", &err));
    EXPECT_STREQ(helicsEndpointGetTag(epid_b, "tag1"), "tagvalue");

    CE(helicsFederateFinalize(mFed1, &err));

    CE(mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == HelicsFederateState::HELICS_STATE_FINALIZE);
}

TEST_P(mfed_simple_type_tests, send_receive)
{
    SetupTest(helicsCreateMessageFederate, GetParam(), 1);
    auto mFed1 = GetFederateAt(0);

    auto epid = helicsFederateRegisterEndpoint(mFed1, "ep1", nullptr, &err);
    auto epid2 = helicsFederateRegisterGlobalEndpoint(mFed1, "ep2", "random", &err);
    EXPECT_EQ(err.error_code, HELICS_OK);
    CE(helicsFederateSetTimeProperty(mFed1, HELICS_PROPERTY_TIME_DELTA, 1.0, &err));

    CE(helicsFederateEnterExecutingMode(mFed1, &err));

    CE(HelicsFederateState mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == HELICS_STATE_EXECUTION);
    std::string data(500, 'a');

    CE(helicsEndpointSendBytesToAt(epid, data.c_str(), 500, "ep2", 0.0, &err));
    HelicsTime time;
    CE(time = helicsFederateRequestTime(mFed1, 1.0, &err));
    EXPECT_EQ(time, 1.0);

    auto res = helicsFederateHasMessage(mFed1);
    EXPECT_TRUE(res);
    res = helicsEndpointHasMessage(epid);
    EXPECT_TRUE(res == false);
    res = helicsEndpointHasMessage(epid2);
    EXPECT_TRUE(res);

    auto M = helicsEndpointGetMessage(epid2);
    // ASSERT_TRUE (M);
    ASSERT_EQ(helicsMessageGetByteCount(M), 500);
    char* dptr = reinterpret_cast<char*>(helicsMessageGetBytesPointer(M));
    if (dptr != nullptr) {
        EXPECT_EQ(dptr[245], 'a');
    } else {
        ASSERT_TRUE(false) << "data is nullptr";
    }
    CE(helicsFederateFinalize(mFed1, &err));

    CE(mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == HelicsFederateState::HELICS_STATE_FINALIZE);
}

TEST_P(mfed_simple_type_tests, send_receive_mobj)
{
    SetupTest(helicsCreateMessageFederate, GetParam(), 1);
    auto mFed1 = GetFederateAt(0);

    auto epid = helicsFederateRegisterEndpoint(mFed1, "ep1", nullptr, &err);
    auto epid2 = helicsFederateRegisterGlobalEndpoint(mFed1, "ep2", "random", &err);
    EXPECT_EQ(err.error_code, HELICS_OK);
    CE(helicsFederateSetTimeProperty(mFed1, HELICS_PROPERTY_TIME_DELTA, 1.0, &err));

    CE(helicsFederateEnterExecutingMode(mFed1, &err));

    CE(HelicsFederateState mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == HELICS_STATE_EXECUTION);
    std::string data(500, 'a');

    CE(helicsEndpointSendBytesToAt(epid, data.c_str(), 500, "ep2", 0.0, &err));
    HelicsTime time;
    CE(time = helicsFederateRequestTime(mFed1, 1.0, &err));
    EXPECT_EQ(time, 1.0);

    auto res = helicsFederateHasMessage(mFed1);
    EXPECT_TRUE(res);
    res = helicsEndpointHasMessage(epid);
    EXPECT_TRUE(res == false);
    res = helicsEndpointHasMessage(epid2);
    EXPECT_TRUE(res);

    auto M = helicsEndpointGetMessage(epid2);
    // ASSERT_TRUE (M);
    ASSERT_EQ(helicsMessageGetByteCount(M), 500);

    char* rdata = static_cast<char*>(helicsMessageGetBytesPointer(M));
    if (rdata != nullptr) {
        EXPECT_EQ(rdata[245], 'a');
    } else {
        EXPECT_TRUE(false) << "data is null";
    }
    CE(helicsFederateFinalize(mFed1, &err));

    CE(mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == HelicsFederateState::HELICS_STATE_FINALIZE);
}

TEST_F(mfed_tests, message_object)
{
    SetupTest(helicsCreateMessageFederate, "test", 1);
    auto mFed1 = GetFederateAt(0);

    auto epid = helicsFederateRegisterEndpoint(mFed1, "ep1", nullptr, &err);
    auto epid2 = helicsFederateRegisterGlobalEndpoint(mFed1, "ep2", "random", &err);
    EXPECT_EQ(err.error_code, HELICS_OK);
    CE(helicsFederateSetTimeProperty(mFed1, HELICS_PROPERTY_TIME_DELTA, 1.0, &err));

    CE(helicsFederateEnterExecutingMode(mFed1, &err));

    CE(HelicsFederateState mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == HELICS_STATE_EXECUTION);
    std::string data(500, 'a');

    auto M = helicsFederateCreateMessage(mFed1, nullptr);
    helicsMessageSetDestination(M, "ep2", nullptr);
    EXPECT_STREQ(helicsMessageGetDestination(M), "ep2");
    helicsMessageSetData(M, data.data(), 500, &err);
    helicsMessageSetTime(M, 0.0, &err);

    CE(helicsEndpointSendMessage(epid, M, &err));
    HelicsTime time;
    CE(time = helicsFederateRequestTime(mFed1, 1.0, &err));
    EXPECT_EQ(time, 1.0);

    auto res = helicsFederateHasMessage(mFed1);
    EXPECT_TRUE(res);
    res = helicsEndpointHasMessage(epid);
    EXPECT_TRUE(res == false);
    res = helicsEndpointHasMessage(epid2);
    EXPECT_TRUE(res);

    M = helicsEndpointGetMessage(epid2);
    // ASSERT_TRUE (M);
    ASSERT_EQ(helicsMessageGetByteCount(M), 500);

    char* rdata = static_cast<char*>(helicsMessageGetBytesPointer(M));
    EXPECT_NE(rdata, nullptr);
    if (rdata != nullptr) {
        EXPECT_EQ(rdata[245], 'a');
    }

    CE(helicsFederateFinalize(mFed1, &err));

    CE(mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == HelicsFederateState::HELICS_STATE_FINALIZE);

    helicsMessageSetFlagOption(M, 7, HELICS_TRUE, &err);
    EXPECT_TRUE(helicsMessageGetFlagOption(M, 7) == HELICS_TRUE);
    helicsMessageClearFlags(M);
    EXPECT_TRUE(helicsMessageGetFlagOption(M, 7) == HELICS_FALSE);
}

TEST_P(mfed_type_tests, send_receive_2fed)
{
    // extraBrokerArgs = "--loglevel=4";
    SetupTest(helicsCreateMessageFederate, GetParam(), 2);
    auto mFed1 = GetFederateAt(0);
    auto mFed2 = GetFederateAt(1);
    helicsFederateSetIntegerProperty(mFed1, HELICS_PROPERTY_INT_CONSOLE_LOG_LEVEL, 0, &err);
    // mFed2->setLoggingLevel(4);
    CE(auto epid = helicsFederateRegisterEndpoint(mFed1, "ep1", nullptr, &err));
    CE(auto epid2 = helicsFederateRegisterGlobalEndpoint(mFed2, "ep2", "random", &err));

    helicsEndpointSetOption(epid, HELICS_HANDLE_OPTION_IGNORE_INTERRUPTS, HELICS_TRUE, &err);
    EXPECT_EQ(err.error_code, 0);

    CE(helicsFederateSetTimeProperty(mFed1, HELICS_PROPERTY_TIME_DELTA, 1.0, &err));
    CE(helicsFederateSetTimeProperty(mFed2, HELICS_PROPERTY_TIME_DELTA, 1.0, &err));

    CE(helicsFederateEnterExecutingModeAsync(mFed1, &err));
    CE(helicsFederateEnterExecutingMode(mFed2, &err));
    CE(helicsFederateEnterExecutingModeComplete(mFed1, &err));

    CE(HelicsFederateState mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == HELICS_STATE_EXECUTION);
    CE(HelicsFederateState mFed2State = helicsFederateGetState(mFed2, &err));
    EXPECT_TRUE(mFed2State == HELICS_STATE_EXECUTION);

    int val = helicsFederateGetIntegerProperty(mFed1, HELICS_PROPERTY_INT_CONSOLE_LOG_LEVEL, &err);
    EXPECT_EQ(val, 0);

    std::string data(500, 'a');
    std::string data2(400, 'b');

    CE(helicsEndpointSendBytesToAt(epid, data.c_str(), 500, "ep2", 0.0, &err));
    CE(helicsEndpointSendBytesToAt(epid2, data2.c_str(), 400, "fed0/ep1", 0.0, &err));
    // move the time to 1.0
    HelicsTime time;
    CE(helicsFederateRequestTimeAsync(mFed1, 1.0, &err));
    HelicsTime gtime;
    CE(gtime = helicsFederateRequestTime(mFed2, 1.0, &err));
    CE(time = helicsFederateRequestTimeComplete(mFed1, &err));

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(time, 1.0);

    auto opt_val = helicsEndpointGetOption(epid, HELICS_HANDLE_OPTION_IGNORE_INTERRUPTS);
    // someday this might get implemented but for now it isn't so we expect false
    EXPECT_EQ(opt_val, false);

    auto res = helicsFederateHasMessage(mFed1);
    EXPECT_TRUE(res);
    res = helicsEndpointHasMessage(epid);
    EXPECT_TRUE(res);
    res = helicsEndpointHasMessage(epid2);
    EXPECT_TRUE(res);

    auto M1 = helicsEndpointGetMessage(epid);
    // ASSERT_TRUE(M1);
    EXPECT_EQ(helicsMessageGetByteCount(M1), 400);
    auto dptr = static_cast<char*>(helicsMessageGetBytesPointer(M1));
    EXPECT_NE(dptr, nullptr);
    if (dptr != nullptr) {
        EXPECT_EQ(dptr[245], 'b');
    }

    auto M2 = helicsEndpointGetMessage(epid2);
    // ASSERT_TRUE(M2);
    EXPECT_EQ(helicsMessageGetByteCount(M2), 500);
    dptr = static_cast<char*>(helicsMessageGetBytesPointer(M2));
    EXPECT_NE(dptr, nullptr);
    if (dptr != nullptr) {
        EXPECT_EQ(dptr[245], 'a');
    }
    CE(helicsFederateFinalizeAsync(mFed1, &err));
    CE(helicsFederateFinalize(mFed2, &err));
    CE(helicsFederateFinalizeComplete(mFed1, &err));
    CE(mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == HelicsFederateState::HELICS_STATE_FINALIZE);
    CE(mFed2State = helicsFederateGetState(mFed2, &err));
    EXPECT_TRUE(mFed2State == HelicsFederateState::HELICS_STATE_FINALIZE);
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

    EXPECT_EQ(helicsEndpointIsValid(epid), HELICS_TRUE);
    CE(helicsFederateSetTimeProperty(mFed1, HELICS_PROPERTY_TIME_DELTA, 1.0, &err));
    CE(helicsFederateSetTimeProperty(mFed2, HELICS_PROPERTY_TIME_DELTA, 1.0, &err));

    helicsEndpointSetDefaultDestination(epid, "ep2", &err);

    CE(helicsFederateEnterExecutingModeAsync(mFed1, &err));
    CE(helicsFederateEnterExecutingMode(mFed2, &err));
    CE(helicsFederateEnterExecutingModeComplete(mFed1, &err));

    CE(HelicsFederateState mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == HELICS_STATE_EXECUTION);
    CE(HelicsFederateState mFed2State = helicsFederateGetState(mFed2, &err));
    EXPECT_TRUE(mFed2State == HELICS_STATE_EXECUTION);

    std::string data(500, 'a');

    CE(helicsEndpointSendBytesTo(epid, data.c_str(), 500, nullptr, &err));
    CE(helicsEndpointSendBytesTo(epid, data.c_str(), 400, nullptr, &err));
    CE(helicsEndpointSendBytesTo(epid, data.c_str(), 300, nullptr, &err));

    // move the time to 1.0
    HelicsTime time;
    CE(helicsFederateRequestTimeAsync(mFed1, 1.0, &err));
    HelicsTime gtime;
    CE(gtime = helicsFederateRequestTime(mFed2, 1.0, &err));
    CE(time = helicsFederateRequestTimeComplete(mFed1, &err));

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(time, 1.0);

    auto res = helicsEndpointPendingMessageCount(epid2);
    EXPECT_EQ(res, 3);

    res = helicsFederatePendingMessageCount(mFed2);
    EXPECT_EQ(res, 3);

    EXPECT_STREQ(helicsEndpointGetDefaultDestination(epid), "ep2");

    CE(helicsFederateFinalizeAsync(mFed1, &err));
    CE(helicsFederateFinalize(mFed2, &err));
    CE(helicsFederateFinalizeComplete(mFed1, &err));
    CE(mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == HelicsFederateState::HELICS_STATE_FINALIZE);
    CE(mFed2State = helicsFederateGetState(mFed2, &err));
    EXPECT_TRUE(mFed2State == HelicsFederateState::HELICS_STATE_FINALIZE);
}

INSTANTIATE_TEST_SUITE_P(mfed_tests, mfed_simple_type_tests, ::testing::ValuesIn(CoreTypes_simple));
INSTANTIATE_TEST_SUITE_P(mfed_tests, mfed_type_tests, ::testing::ValuesIn(CoreTypes));

// a series of tests exercising the different aspects of message object setting and retrieval
TEST(message_object, test1_nosan)
{
    auto brk = helicsCreateBroker("test", "brk1", "", nullptr);

    auto fi = helicsCreateFederateInfo();
    helicsFederateInfoSetBroker(fi, "brk1", nullptr);
    helicsFederateInfoSetCoreType(fi, HELICS_CORE_TYPE_TEST, nullptr);

    auto fed = helicsCreateMessageFederate("fed1", fi, nullptr);

    auto fed2 = helicsCreateCombinationFederate("fed2", fi, nullptr);

    auto m1 = helicsFederateCreateMessage(fed, nullptr);
    EXPECT_NE(m1, nullptr);

    auto m2 = helicsFederateCreateMessage(fed2, nullptr);
    EXPECT_NE(m2, nullptr);

    helicsMessageSetOriginalDestination(m1, "a happy place", nullptr);
    EXPECT_STREQ(helicsMessageGetOriginalDestination(m1), "a happy place");

    helicsMessageSetOriginalSource(m1, "osource", nullptr);
    EXPECT_STREQ(helicsMessageGetOriginalSource(m1), "osource");

    helicsMessageSetMessageID(m1, 10, nullptr);
    EXPECT_EQ(helicsMessageGetMessageID(m1), 10);

    // 89 is an invalid flag
    EXPECT_EQ(helicsMessageGetFlagOption(m1, 89), HELICS_FALSE);

    helicsMessageSetString(m2, "raw data", nullptr);
    EXPECT_STREQ(helicsMessageGetString(m2), "raw data");

    auto err = helicsErrorInitialize();

    char data[20];
    int actSize = 10;
    helicsMessageGetBytes(m2, nullptr, 0, &actSize, &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(actSize, 0);
    helicsErrorClear(&err);

    EXPECT_EQ(helicsMessageGetBytesPointer(nullptr), nullptr);

    helicsMessageGetBytes(m2, data, 20, &actSize, &err);
    EXPECT_EQ(err.error_code, 0);
    EXPECT_EQ(actSize, 8);
    EXPECT_EQ(std::string(data, data + actSize), "raw data");

    EXPECT_EQ(helicsMessageIsValid(m2), HELICS_TRUE);

    helicsMessageSetSource(m1, "source", nullptr);
    EXPECT_STREQ(helicsMessageGetSource(m1), "source");

    helicsMessageSetFlagOption(m1, 4, HELICS_TRUE, nullptr);
    EXPECT_EQ(helicsMessageGetFlagOption(m1, 4), HELICS_TRUE);

    helicsMessageSetFlagOption(m1, 4, HELICS_FALSE, nullptr);
    EXPECT_EQ(helicsMessageGetFlagOption(m1, 4), HELICS_FALSE);

    helicsMessageSetFlagOption(m1, 22, HELICS_TRUE, &err);
    EXPECT_EQ(helicsMessageGetFlagOption(m1, 22), HELICS_FALSE);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsMessageResize(m2, 500, nullptr);
    auto s2 = helicsMessageGetByteCount(m2);
    EXPECT_EQ(s2, 500);

    helicsMessageReserve(m2, 2000, nullptr);

    helicsMessageAppendData(m2, " more data", 8, nullptr);

    s2 = helicsMessageGetByteCount(m2);
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
    helicsCleanupLibrary();
    auto brk = helicsCreateBroker("test", "brk_mcpy", "", nullptr);

    auto fedInfo = helicsCreateFederateInfo();
    helicsFederateInfoSetCoreType(fedInfo, HELICS_CORE_TYPE_TEST, nullptr);
    helicsFederateInfoSetBroker(fedInfo, "brk_mcpy", nullptr);
    helicsFederateInfoLoadFromString(fedInfo, "--force_new_core", nullptr);
    auto fed = helicsCreateMessageFederate("fed1", fedInfo, nullptr);

    helicsFederateInfoFree(fedInfo);
    auto m1 = helicsFederateCreateMessage(fed, nullptr);
    EXPECT_NE(m1, nullptr);

    auto m2 = helicsFederateCreateMessage(fed, nullptr);
    EXPECT_NE(m2, nullptr);

    helicsMessageSetDestination(m1, "a small town", nullptr);
    helicsMessageSetSource(m1, "toledo", nullptr);

    helicsMessageSetOriginalDestination(m1, "a happy place", nullptr);
    helicsMessageSetOriginalSource(m1, "osource", nullptr);
    helicsMessageSetMessageID(m1, 10, nullptr);
    helicsMessageSetFlagOption(m1, 4, HELICS_TRUE, nullptr);
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

    helicsMessageGetBytes(m2, data, 20, &actSize, &err);
    EXPECT_EQ(err.error_code, 0);
    EXPECT_EQ(actSize, 8);
    EXPECT_EQ(std::string(data, data + actSize), "raw data");

    EXPECT_EQ(helicsMessageIsValid(m2), HELICS_TRUE);

    EXPECT_EQ(helicsMessageGetFlagOption(m2, 4), HELICS_TRUE);

    helicsMessageClear(m2, &err);

    EXPECT_EQ(helicsMessageIsValid(m2), HELICS_FALSE);

    EXPECT_EQ(helicsMessageGetFlagOption(m2, 4), HELICS_FALSE);

    EXPECT_EQ(helicsMessageGetByteCount(m2), 0);

    helicsFederateEnterExecutingMode(fed, nullptr);
    helicsFederateFinalize(fed, nullptr);
    helicsBrokerDisconnect(brk, nullptr);
}

TEST(message_object, dataBuffer)
{
    auto brk = helicsCreateBroker("test", "brk_db", "", nullptr);

    auto fedInfo = helicsCreateFederateInfo();
    helicsFederateInfoSetCoreType(fedInfo, HELICS_CORE_TYPE_TEST, nullptr);

    helicsFederateInfoSetBroker(fedInfo, "brk_db", nullptr);
    helicsFederateInfoLoadFromString(fedInfo, "--force_new_core", nullptr);

    auto fed = helicsCreateMessageFederate("fed1", fedInfo, nullptr);
    helicsFederateInfoFree(fedInfo);
    auto m1 = helicsFederateCreateMessage(fed, nullptr);
    EXPECT_NE(m1, nullptr);

    auto m2 = helicsFederateCreateMessage(fed, nullptr);
    EXPECT_NE(m2, nullptr);

    helicsMessageSetString(m1, "raw data", nullptr);

    auto err = helicsErrorInitialize();

    EXPECT_EQ(helicsDataBufferIsValid(m1), HELICS_TRUE);

    char data[20];
    int actSize{10};

    EXPECT_EQ(helicsDataBufferSize(m1), 8);
    helicsDataBufferToRawString(m1, data, 20, &actSize);
    EXPECT_EQ(actSize, 8);
    EXPECT_EQ(std::string(data, data + actSize), "raw data");

    helicsMessageClear(m2, &err);
    // test the connection between the buffer and message
    EXPECT_EQ(helicsDataBufferSize(m2), 0);
    EXPECT_EQ(helicsMessageIsValid(m2), HELICS_FALSE);
    EXPECT_EQ(helicsDataBufferIsValid(m2), HELICS_TRUE);

    helicsFederateEnterExecutingMode(fed, nullptr);
    helicsFederateFinalize(fed, nullptr);
    helicsBrokerDisconnect(brk, nullptr);
}
