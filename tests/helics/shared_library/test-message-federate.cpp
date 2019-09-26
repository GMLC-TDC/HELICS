/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include <gtest/gtest.h>

#include "ctestFixtures.hpp"

#include <future>
#include <iostream>
#include <thread>
// these test cases test out the message federates

class mfed_simple_type_tests : public ::testing::TestWithParam<const char *>, public FederateTestFixture
{
};

class mfed_type_tests : public ::testing::TestWithParam<const char *>, public FederateTestFixture
{
};

class mfed_tests : public FederateTestFixture, public ::testing::Test
{
};
/** test simple creation and destruction*/
TEST_P (mfed_simple_type_tests, initialize_tests)
{
    SetupTest (helicsCreateMessageFederate, GetParam (), 1);
    auto mFed1 = GetFederateAt (0);

    CE (helicsFederateEnterExecutingMode (mFed1, &err));

    CE (helics_federate_state mFed1State = helicsFederateGetState (mFed1, &err));
    EXPECT_TRUE (mFed1State == helics_state_execution);

    CE (helicsFederateFinalize (mFed1, &err));

    CE (mFed1State = helicsFederateGetState (mFed1, &err));
    EXPECT_TRUE (mFed1State == helics_federate_state::helics_state_finalize);
}

TEST_P (mfed_simple_type_tests, endpoint_registration)
{
    SetupTest (helicsCreateMessageFederate, GetParam (), 1);
    auto mFed1 = GetFederateAt (0);

    auto epid = helicsFederateRegisterEndpoint (mFed1, "ep1", NULL, &err);
    auto epid2 = helicsFederateRegisterGlobalEndpoint (mFed1, "ep2", "random", &err);
    EXPECT_EQ (err.error_code, helics_ok);
    CE (helicsFederateEnterExecutingMode (mFed1, &err));

    CE (helics_federate_state mFed1State = helicsFederateGetState (mFed1, &err));
    EXPECT_TRUE (mFed1State == helics_state_execution);

    const char *name = helicsEndpointGetName (epid);
    EXPECT_EQ (name, "fed0/ep1");
    name = helicsEndpointGetName (epid2);
    EXPECT_EQ (name, "ep2");

    const char *type = helicsEndpointGetType (epid);
    const char *type2 = helicsEndpointGetType (epid2);
    EXPECT_EQ (type, "");
    EXPECT_EQ (type2, "random");

    auto epid_b = helicsFederateGetEndpoint (mFed1, "ep2", &err);
    type = helicsEndpointGetType (epid_b);
    EXPECT_EQ (type, "random");

    auto epid_c = helicsFederateGetEndpointByIndex (mFed1, 0, &err);
    name = helicsEndpointGetName (epid_c);
    EXPECT_EQ (name, "fed0/ep1");

    CE (helicsFederateFinalize (mFed1, &err));

    CE (mFed1State = helicsFederateGetState (mFed1, &err));
    EXPECT_TRUE (mFed1State == helics_federate_state::helics_state_finalize);
}

TEST_P (mfed_simple_type_tests, send_receive)
{
    SetupTest (helicsCreateMessageFederate, GetParam (), 1);
    auto mFed1 = GetFederateAt (0);

    auto epid = helicsFederateRegisterEndpoint (mFed1, "ep1", NULL, &err);
    auto epid2 = helicsFederateRegisterGlobalEndpoint (mFed1, "ep2", "random", &err);
    EXPECT_EQ (err.error_code, helics_ok);
    CE (helicsFederateSetTimeProperty (mFed1, helics_property_time_delta, 1.0, &err));

    CE (helicsFederateEnterExecutingMode (mFed1, &err));

    CE (helics_federate_state mFed1State = helicsFederateGetState (mFed1, &err));
    EXPECT_TRUE (mFed1State == helics_state_execution);
    std::string data (500, 'a');

    CE (helicsEndpointSendEventRaw (epid, "ep2", data.c_str (), 500, 0.0, &err));
    helics_time time;
    CE (time = helicsFederateRequestTime (mFed1, 1.0, &err));
    EXPECT_EQ (time, 1.0);

    auto res = helicsFederateHasMessage (mFed1);
    EXPECT_TRUE (res);
    res = helicsEndpointHasMessage (epid);
    EXPECT_TRUE (res == false);
    res = helicsEndpointHasMessage (epid2);
    EXPECT_TRUE (res);

    auto M = helicsEndpointGetMessage (epid2);
    // ASSERT_TRUE (M);
    ASSERT_EQ (M.length, 500);

    EXPECT_EQ (M.data[245], 'a');
    CE (helicsFederateFinalize (mFed1, &err));

    CE (mFed1State = helicsFederateGetState (mFed1, &err));
    EXPECT_TRUE (mFed1State == helics_federate_state::helics_state_finalize);
}

TEST_P (mfed_simple_type_tests, send_receive_mobj)
{
    SetupTest (helicsCreateMessageFederate, GetParam (), 1);
    auto mFed1 = GetFederateAt (0);

    auto epid = helicsFederateRegisterEndpoint (mFed1, "ep1", NULL, &err);
    auto epid2 = helicsFederateRegisterGlobalEndpoint (mFed1, "ep2", "random", &err);
    EXPECT_EQ (err.error_code, helics_ok);
    CE (helicsFederateSetTimeProperty (mFed1, helics_property_time_delta, 1.0, &err));

    CE (helicsFederateEnterExecutingMode (mFed1, &err));

    CE (helics_federate_state mFed1State = helicsFederateGetState (mFed1, &err));
    EXPECT_TRUE (mFed1State == helics_state_execution);
    std::string data (500, 'a');

    CE (helicsEndpointSendEventRaw (epid, "ep2", data.c_str (), 500, 0.0, &err));
    helics_time time;
    CE (time = helicsFederateRequestTime (mFed1, 1.0, &err));
    EXPECT_EQ (time, 1.0);

    auto res = helicsFederateHasMessage (mFed1);
    EXPECT_TRUE (res);
    res = helicsEndpointHasMessage (epid);
    EXPECT_TRUE (res == false);
    res = helicsEndpointHasMessage (epid2);
    EXPECT_TRUE (res);

    auto M = helicsEndpointGetMessageObject (epid2);
    // ASSERT_TRUE (M);
    ASSERT_EQ (helicsMessageGetRawDataSize (M), 500);

    char *rdata = static_cast<char *> (helicsMessageGetRawDataPointer (M));
    EXPECT_EQ (rdata[245], 'a');
    CE (helicsFederateFinalize (mFed1, &err));

    CE (mFed1State = helicsFederateGetState (mFed1, &err));
    EXPECT_TRUE (mFed1State == helics_federate_state::helics_state_finalize);
}

TEST_F (mfed_tests, message_object_tests)
{
    SetupTest (helicsCreateMessageFederate, "test", 1);
    auto mFed1 = GetFederateAt (0);

    auto epid = helicsFederateRegisterEndpoint (mFed1, "ep1", NULL, &err);
    auto epid2 = helicsFederateRegisterGlobalEndpoint (mFed1, "ep2", "random", &err);
    EXPECT_EQ (err.error_code, helics_ok);
    CE (helicsFederateSetTimeProperty (mFed1, helics_property_time_delta, 1.0, &err));

    CE (helicsFederateEnterExecutingMode (mFed1, &err));

    CE (helics_federate_state mFed1State = helicsFederateGetState (mFed1, &err));
    EXPECT_TRUE (mFed1State == helics_state_execution);
    std::string data (500, 'a');

    auto M = helicsFederateCreateMessageObject (mFed1, nullptr);
    helicsMessageSetDestination (M, "ep2", nullptr);
    EXPECT_STREQ (helicsMessageGetDestination (M), "ep2");
    helicsMessageSetData (M, data.data (), 500, &err);
    helicsMessageSetTime (M, 0.0, &err);

    CE (helicsEndpointSendMessageObject (epid, M, &err));
    helics_time time;
    CE (time = helicsFederateRequestTime (mFed1, 1.0, &err));
    EXPECT_EQ (time, 1.0);

    auto res = helicsFederateHasMessage (mFed1);
    EXPECT_TRUE (res);
    res = helicsEndpointHasMessage (epid);
    EXPECT_TRUE (res == false);
    res = helicsEndpointHasMessage (epid2);
    EXPECT_TRUE (res);

    M = helicsEndpointGetMessageObject (epid2);
    // ASSERT_TRUE (M);
    ASSERT_EQ (helicsMessageGetRawDataSize (M), 500);

    char *rdata = static_cast<char *> (helicsMessageGetRawDataPointer (M));
    EXPECT_EQ (rdata[245], 'a');
    CE (helicsFederateFinalize (mFed1, &err));

    CE (mFed1State = helicsFederateGetState (mFed1, &err));
    EXPECT_TRUE (mFed1State == helics_federate_state::helics_state_finalize);

    helicsMessageSetFlagOption (M, 7, helics_true, &err);
    EXPECT_TRUE (helicsMessageCheckFlag (M, 7) == helics_true);
    helicsMessageClearFlags (M);
    EXPECT_TRUE (helicsMessageCheckFlag (M, 7) == helics_false);
}

TEST_P (mfed_type_tests, send_receive_2fed)
{
    // extraBrokerArgs = "--loglevel=4";
    SetupTest (helicsCreateMessageFederate, GetParam (), 2);
    auto mFed1 = GetFederateAt (0);
    auto mFed2 = GetFederateAt (1);
    // mFed1->setLoggingLevel(4);
    // mFed2->setLoggingLevel(4);
    CE (auto epid = helicsFederateRegisterEndpoint (mFed1, "ep1", NULL, &err));
    CE (auto epid2 = helicsFederateRegisterGlobalEndpoint (mFed2, "ep2", "random", &err));

    CE (helicsFederateSetTimeProperty (mFed1, helics_property_time_delta, 1.0, &err));
    CE (helicsFederateSetTimeProperty (mFed2, helics_property_time_delta, 1.0, &err));

    CE (helicsFederateEnterExecutingModeAsync (mFed1, &err));
    CE (helicsFederateEnterExecutingMode (mFed2, &err));
    CE (helicsFederateEnterExecutingModeComplete (mFed1, &err));

    CE (helics_federate_state mFed1State = helicsFederateGetState (mFed1, &err));
    EXPECT_TRUE (mFed1State == helics_state_execution);
    CE (helics_federate_state mFed2State = helicsFederateGetState (mFed2, &err));
    EXPECT_TRUE (mFed2State == helics_state_execution);

    std::string data (500, 'a');
    std::string data2 (400, 'b');

    CE (helicsEndpointSendEventRaw (epid, "ep2", data.c_str (), 500, 0.0, &err));
    CE (helicsEndpointSendEventRaw (epid2, "fed0/ep1", data2.c_str (), 400, 0.0, &err));
    // move the time to 1.0
    helics_time time;
    CE (helicsFederateRequestTimeAsync (mFed1, 1.0, &err));
    helics_time gtime;
    CE (gtime = helicsFederateRequestTime (mFed2, 1.0, &err));
    CE (time = helicsFederateRequestTimeComplete (mFed1, &err));

    EXPECT_EQ (gtime, 1.0);
    EXPECT_EQ (time, 1.0);

    auto res = helicsFederateHasMessage (mFed1);
    EXPECT_TRUE (res);
    res = helicsEndpointHasMessage (epid);
    EXPECT_TRUE (res);
    res = helicsEndpointHasMessage (epid2);
    EXPECT_TRUE (res);

    auto M1 = helicsEndpointGetMessage (epid);
    // ASSERT_TRUE(M1);
    ASSERT_EQ (M1.length, 400);

    EXPECT_EQ (M1.data[245], 'b');

    auto M2 = helicsEndpointGetMessage (epid2);
    // ASSERT_TRUE(M2);
    ASSERT_EQ (M2.length, 500);

    EXPECT_EQ (M2.data[245], 'a');
    CE (helicsFederateFinalizeAsync (mFed1, &err));
    CE (helicsFederateFinalize (mFed2, &err));
    CE (helicsFederateFinalizeComplete (mFed1, &err));
    CE (mFed1State = helicsFederateGetState (mFed1, &err));
    EXPECT_TRUE (mFed1State == helics_federate_state::helics_state_finalize);
    CE (mFed2State = helicsFederateGetState (mFed2, &err));
    EXPECT_TRUE (mFed2State == helics_federate_state::helics_state_finalize);
}

TEST_P (mfed_type_tests, send_receive_2fed_multisend)
{
    // extraBrokerArgs = "--loglevel=4";
    SetupTest (helicsCreateMessageFederate, GetParam (), 2);
    auto mFed1 = GetFederateAt (0);
    auto mFed2 = GetFederateAt (1);
    // mFed1->setLoggingLevel(4);
    // mFed2->setLoggingLevel(4);
    CE (auto epid = helicsFederateRegisterEndpoint (mFed1, "ep1", NULL, &err));
    CE (auto epid2 = helicsFederateRegisterGlobalEndpoint (mFed2, "ep2", "random", &err));

    CE (helicsFederateSetTimeProperty (mFed1, helics_property_time_delta, 1.0, &err));
    CE (helicsFederateSetTimeProperty (mFed2, helics_property_time_delta, 1.0, &err));

    CE (helicsFederateEnterExecutingModeAsync (mFed1, &err));
    CE (helicsFederateEnterExecutingMode (mFed2, &err));
    CE (helicsFederateEnterExecutingModeComplete (mFed1, &err));

    CE (helics_federate_state mFed1State = helicsFederateGetState (mFed1, &err));
    EXPECT_TRUE (mFed1State == helics_state_execution);
    CE (helics_federate_state mFed2State = helicsFederateGetState (mFed2, &err));
    EXPECT_TRUE (mFed2State == helics_state_execution);

    std::string data (500, 'a');

    helicsEndpointSetDefaultDestination (epid, "ep2", &err);

    CE (helicsEndpointSendMessageRaw (epid, nullptr, data.c_str (), 500, &err));
    CE (helicsEndpointSendMessageRaw (epid, nullptr, data.c_str (), 400, &err));
    CE (helicsEndpointSendMessageRaw (epid, nullptr, data.c_str (), 300, &err));

    // move the time to 1.0
    helics_time time;
    CE (helicsFederateRequestTimeAsync (mFed1, 1.0, &err));
    helics_time gtime;
    CE (gtime = helicsFederateRequestTime (mFed2, 1.0, &err));
    CE (time = helicsFederateRequestTimeComplete (mFed1, &err));

    EXPECT_EQ (gtime, 1.0);
    EXPECT_EQ (time, 1.0);

    auto res = helicsEndpointPendingMessages (epid2);
    EXPECT_EQ (res, 3);

    res = helicsFederatePendingMessages (mFed2);
    EXPECT_EQ (res, 3);

    EXPECT_EQ (helicsEndpointGetDefaultDestination (epid), "ep2");

    CE (helicsFederateFinalizeAsync (mFed1, &err));
    CE (helicsFederateFinalize (mFed2, &err));
    CE (helicsFederateFinalizeComplete (mFed1, &err));
    CE (mFed1State = helicsFederateGetState (mFed1, &err));
    EXPECT_TRUE (mFed1State == helics_federate_state::helics_state_finalize);
    CE (mFed2State = helicsFederateGetState (mFed2, &err));
    EXPECT_TRUE (mFed2State == helics_federate_state::helics_state_finalize);
}

INSTANTIATE_TEST_SUITE_P (mfed_tests, mfed_simple_type_tests, ::testing::ValuesIn (core_types_simple));
INSTANTIATE_TEST_SUITE_P (mfed_tests, mfed_type_tests, ::testing::ValuesIn (core_types));
