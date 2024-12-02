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
#include <string>
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
    helicsCleanupLibrary();
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
    helicsCleanupLibrary();
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

    auto message = helicsEndpointGetMessage(epid2);
    // ASSERT_TRUE (message);
    ASSERT_EQ(helicsMessageGetByteCount(message), 500);
    char* dptr = reinterpret_cast<char*>(helicsMessageGetBytesPointer(message));
    if (dptr != nullptr) {
        EXPECT_EQ(dptr[245], 'a');
    } else {
        ASSERT_TRUE(false) << "data is nullptr";
    }
    CE(helicsFederateFinalize(mFed1, &err));

    CE(mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == HelicsFederateState::HELICS_STATE_FINALIZE);
    helicsCleanupLibrary();
}

TEST_F(mfed_tests, send_receive_string)
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

    CE(helicsEndpointSendStringToAt(epid, data.c_str(), "ep2", 0.0, &err));
    HelicsTime time;
    CE(time = helicsFederateRequestTime(mFed1, 1.0, &err));
    EXPECT_EQ(time, 1.0);

    auto res = helicsFederateHasMessage(mFed1);
    EXPECT_TRUE(res);
    res = helicsEndpointHasMessage(epid);
    EXPECT_TRUE(res == false);
    res = helicsEndpointHasMessage(epid2);
    EXPECT_TRUE(res);

    auto message = helicsEndpointGetMessage(epid2);
    // ASSERT_TRUE (message);
    ASSERT_EQ(helicsMessageGetByteCount(message), 500);
    char* dptr = reinterpret_cast<char*>(helicsMessageGetBytesPointer(message));
    if (dptr != nullptr) {
        EXPECT_EQ(dptr[245], 'a');
    } else {
        ASSERT_TRUE(false) << "data is nullptr";
    }
    CE(helicsFederateFinalize(mFed1, &err));

    CE(mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == HelicsFederateState::HELICS_STATE_FINALIZE);
    helicsCleanupLibrary();
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

    auto message = helicsEndpointGetMessage(epid2);
    // ASSERT_TRUE (message);
    ASSERT_EQ(helicsMessageGetByteCount(message), 500);

    char* rdata = static_cast<char*>(helicsMessageGetBytesPointer(message));
    if (rdata != nullptr) {
        EXPECT_EQ(rdata[245], 'a');
    } else {
        EXPECT_TRUE(false) << "data is null";
    }
    CE(helicsFederateFinalize(mFed1, &err));

    CE(mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == HelicsFederateState::HELICS_STATE_FINALIZE);
    helicsCleanupLibrary();
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

    auto message = helicsFederateCreateMessage(mFed1, nullptr);
    helicsMessageSetDestination(message, "ep2", nullptr);
    EXPECT_STREQ(helicsMessageGetDestination(message), "ep2");
    helicsMessageSetData(message, data.data(), 500, &err);
    helicsMessageSetTime(message, 0.0, &err);

    CE(helicsEndpointSendMessage(epid, message, &err));
    HelicsTime time;
    CE(time = helicsFederateRequestTime(mFed1, 1.0, &err));
    EXPECT_EQ(time, 1.0);

    auto res = helicsFederateHasMessage(mFed1);
    EXPECT_TRUE(res);
    res = helicsEndpointHasMessage(epid);
    EXPECT_TRUE(res == false);
    res = helicsEndpointHasMessage(epid2);
    EXPECT_TRUE(res);

    message = helicsEndpointGetMessage(epid2);
    // ASSERT_TRUE (message);
    ASSERT_EQ(helicsMessageGetByteCount(message), 500);

    char* rdata = static_cast<char*>(helicsMessageGetBytesPointer(message));
    EXPECT_NE(rdata, nullptr);
    if (rdata != nullptr) {
        EXPECT_EQ(rdata[245], 'a');
    }

    CE(helicsFederateFinalize(mFed1, &err));

    CE(mFed1State = helicsFederateGetState(mFed1, &err));
    EXPECT_TRUE(mFed1State == HelicsFederateState::HELICS_STATE_FINALIZE);

    helicsMessageSetFlagOption(message, 7, HELICS_TRUE, &err);
    EXPECT_TRUE(helicsMessageGetFlagOption(message, 7) == HELICS_TRUE);
    helicsMessageClearFlags(message);
    EXPECT_TRUE(helicsMessageGetFlagOption(message, 7) == HELICS_FALSE);
    helicsCleanupLibrary();
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

    auto message1 = helicsEndpointGetMessage(epid);
    // ASSERT_TRUE(message1);
    EXPECT_EQ(helicsMessageGetByteCount(message1), 400);
    auto dptr = static_cast<char*>(helicsMessageGetBytesPointer(message1));
    EXPECT_NE(dptr, nullptr);
    if (dptr != nullptr) {
        EXPECT_EQ(dptr[245], 'b');
    }

    auto message2 = helicsEndpointGetMessage(epid2);
    // ASSERT_TRUE(message2);
    EXPECT_EQ(helicsMessageGetByteCount(message2), 500);
    dptr = static_cast<char*>(helicsMessageGetBytesPointer(message2));
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
    helicsCleanupLibrary();
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
    helicsCleanupLibrary();
}

INSTANTIATE_TEST_SUITE_P(mfed_tests, mfed_simple_type_tests, ::testing::ValuesIn(CoreTypes_simple));
INSTANTIATE_TEST_SUITE_P(mfed_tests, mfed_type_tests, ::testing::ValuesIn(CoreTypes));

// a series of tests exercising the different aspects of message object setting and retrieval
TEST(message_object, test1_nosan)
{
    auto brk = helicsCreateBroker("test", "brk1", "", nullptr);

    auto fedInfo = helicsCreateFederateInfo();
    helicsFederateInfoSetBroker(fedInfo, "brk1", nullptr);
    helicsFederateInfoSetCoreType(fedInfo, HELICS_CORE_TYPE_TEST, nullptr);

    auto fed = helicsCreateMessageFederate("fed1", fedInfo, nullptr);

    auto fed2 = helicsCreateCombinationFederate("fed2", fedInfo, nullptr);

    auto message1 = helicsFederateCreateMessage(fed, nullptr);
    EXPECT_NE(message1, nullptr);

    auto message2 = helicsFederateCreateMessage(fed2, nullptr);
    EXPECT_NE(message2, nullptr);

    helicsMessageSetOriginalDestination(message1, "a happy place", nullptr);
    EXPECT_STREQ(helicsMessageGetOriginalDestination(message1), "a happy place");

    helicsMessageSetOriginalSource(message1, "osource", nullptr);
    EXPECT_STREQ(helicsMessageGetOriginalSource(message1), "osource");

    helicsMessageSetMessageID(message1, 10, nullptr);
    EXPECT_EQ(helicsMessageGetMessageID(message1), 10);

    // 89 is an invalid flag
    EXPECT_EQ(helicsMessageGetFlagOption(message1, 89), HELICS_FALSE);

    helicsMessageSetString(message2, "raw data", nullptr);
    EXPECT_STREQ(helicsMessageGetString(message2), "raw data");

    auto err = helicsErrorInitialize();

    char data[20];
    int actSize = 10;
    helicsMessageGetBytes(message2, nullptr, 0, &actSize, &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(actSize, 0);
    helicsErrorClear(&err);

    EXPECT_EQ(helicsMessageGetBytesPointer(nullptr), nullptr);

    helicsMessageGetBytes(message2, data, 20, &actSize, &err);
    EXPECT_EQ(err.error_code, 0);
    EXPECT_EQ(actSize, 8);
    EXPECT_EQ(std::string(data, data + actSize), "raw data");

    EXPECT_EQ(helicsMessageIsValid(message2), HELICS_TRUE);

    helicsMessageSetSource(message1, "source", nullptr);
    EXPECT_STREQ(helicsMessageGetSource(message1), "source");

    helicsMessageSetFlagOption(message1, 4, HELICS_TRUE, nullptr);
    EXPECT_EQ(helicsMessageGetFlagOption(message1, 4), HELICS_TRUE);

    helicsMessageSetFlagOption(message1, 4, HELICS_FALSE, nullptr);
    EXPECT_EQ(helicsMessageGetFlagOption(message1, 4), HELICS_FALSE);

    helicsMessageSetFlagOption(message1, 22, HELICS_TRUE, &err);
    EXPECT_EQ(helicsMessageGetFlagOption(message1, 22), HELICS_FALSE);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsMessageResize(message2, 500, nullptr);
    auto messageSize = helicsMessageGetByteCount(message2);
    EXPECT_EQ(messageSize, 500);

    helicsMessageReserve(message2, 2000, nullptr);

    helicsMessageAppendData(message2, " more data", 8, nullptr);

    messageSize = helicsMessageGetByteCount(message2);
    EXPECT_EQ(messageSize, 508);

    // this should generate an out of memory exception
    helicsMessageResize(message2, -8, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    // this should generate an out of memory exception
    helicsMessageReserve(message2, -2, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    helicsFederateInfoFree(fedInfo);
    helicsFederateFinalize(fed, nullptr);
    helicsFederateFinalize(fed2, nullptr);
    helicsBrokerDisconnect(brk, nullptr);

    helicsCleanupLibrary();
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
    auto message1 = helicsFederateCreateMessage(fed, nullptr);
    EXPECT_NE(message1, nullptr);

    auto message2 = helicsFederateCreateMessage(fed, nullptr);
    EXPECT_NE(message2, nullptr);

    helicsMessageSetDestination(message1, "a small town", nullptr);
    helicsMessageSetSource(message1, "toledo", nullptr);

    helicsMessageSetOriginalDestination(message1, "a happy place", nullptr);
    helicsMessageSetOriginalSource(message1, "osource", nullptr);
    helicsMessageSetMessageID(message1, 10, nullptr);
    helicsMessageSetFlagOption(message1, 4, HELICS_TRUE, nullptr);
    helicsMessageSetString(message1, "raw data", nullptr);
    helicsMessageSetTime(message1, 3.65, nullptr);
    auto err = helicsErrorInitialize();

    helicsMessageCopy(message1, message2, &err);
    EXPECT_EQ(err.error_code, 0);

    helicsMessageCopy(message1, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    EXPECT_STREQ(helicsMessageGetString(message2), "raw data");
    EXPECT_STREQ(helicsMessageGetOriginalSource(message2), "osource");
    EXPECT_STREQ(helicsMessageGetSource(message2), "toledo");
    EXPECT_STREQ(helicsMessageGetDestination(message2), "a small town");
    EXPECT_EQ(helicsMessageGetMessageID(message2), 10);
    EXPECT_DOUBLE_EQ(helicsMessageGetTime(message2), 3.65);

    EXPECT_STREQ(helicsMessageGetOriginalDestination(message2), "a happy place");

    char data[20];
    int actSize = 10;

    helicsMessageGetBytes(message2, data, 20, &actSize, &err);
    EXPECT_EQ(err.error_code, 0);
    EXPECT_EQ(actSize, 8);
    EXPECT_EQ(std::string(data, data + actSize), "raw data");

    EXPECT_EQ(helicsMessageIsValid(message2), HELICS_TRUE);

    EXPECT_EQ(helicsMessageGetFlagOption(message2, 4), HELICS_TRUE);

    helicsMessageClear(message2, &err);

    EXPECT_EQ(helicsMessageIsValid(message2), HELICS_FALSE);

    EXPECT_EQ(helicsMessageGetFlagOption(message2, 4), HELICS_FALSE);

    EXPECT_EQ(helicsMessageGetByteCount(message2), 0);

    helicsFederateEnterExecutingMode(fed, nullptr);
    helicsFederateFinalize(fed, nullptr);
    helicsBrokerDisconnect(brk, nullptr);
    helicsCleanupLibrary();
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
    auto message1 = helicsFederateCreateMessage(fed, nullptr);
    EXPECT_NE(message1, nullptr);

    auto message2 = helicsFederateCreateMessage(fed, nullptr);
    EXPECT_NE(message2, nullptr);

    helicsMessageSetString(message1, "raw data", nullptr);

    auto err = helicsErrorInitialize();

    EXPECT_EQ(helicsDataBufferIsValid(message1), HELICS_TRUE);

    char data[20];
    int actSize{10};

    EXPECT_EQ(helicsDataBufferSize(message1), 8);
    helicsDataBufferToRawString(message1, data, 20, &actSize);
    EXPECT_EQ(actSize, 8);
    EXPECT_EQ(std::string(data, data + actSize), "raw data");

    helicsMessageClear(message2, &err);
    // test the connection between the buffer and message
    EXPECT_EQ(helicsDataBufferSize(message2), 0);
    EXPECT_EQ(helicsMessageIsValid(message2), HELICS_FALSE);
    EXPECT_EQ(helicsDataBufferIsValid(message2), HELICS_TRUE);

    helicsFederateEnterExecutingMode(fed, nullptr);
    helicsFederateFinalize(fed, nullptr);
    helicsBrokerDisconnect(brk, nullptr);
    helicsCleanupLibrary();
}
