/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "ctestFixtures.hpp"

#include <future>
#include <iostream>
#include <thread>
// these test cases test out the message federates
namespace bdata = boost::unit_test::data;
namespace utf = boost::unit_test;

BOOST_FIXTURE_TEST_SUITE (message_federate_tests, FederateTestFixture, *utf::label ("ci"))

/** test simple creation and destruction*/
BOOST_DATA_TEST_CASE (message_federate_initialize_tests, bdata::make (core_types_simple), core_type)
{
    SetupTest (helicsCreateMessageFederate, core_type, 1);
    auto mFed1 = GetFederateAt (0);

    CE (helicsFederateEnterExecutingMode (mFed1, &err));

    CE (helics_federate_state mFed1State = helicsFederateGetState (mFed1, &err));
    BOOST_CHECK (mFed1State == helics_state_execution);

    CE (helicsFederateFinalize (mFed1, &err));

    CE (mFed1State = helicsFederateGetState (mFed1, &err));
    BOOST_CHECK (mFed1State == helics_federate_state::helics_state_finalize);
}

BOOST_DATA_TEST_CASE (message_federate_endpoint_registration, bdata::make (core_types_simple), core_type)
{
    SetupTest (helicsCreateMessageFederate, core_type, 1);
    auto mFed1 = GetFederateAt (0);

    auto epid = helicsFederateRegisterEndpoint (mFed1, "ep1", NULL, &err);
    auto epid2 = helicsFederateRegisterGlobalEndpoint (mFed1, "ep2", "random", &err);
    BOOST_CHECK_EQUAL (err.error_code, helics_ok);
    CE (helicsFederateEnterExecutingMode (mFed1, &err));

    CE (helics_federate_state mFed1State = helicsFederateGetState (mFed1, &err));
    BOOST_CHECK (mFed1State == helics_state_execution);

    const char *name = helicsEndpointGetName (epid);
    BOOST_CHECK_EQUAL (name, "fed0/ep1");
    name = helicsEndpointGetName (epid2);
    BOOST_CHECK_EQUAL (name, "ep2");

    const char *type = helicsEndpointGetType (epid);
    const char *type2 = helicsEndpointGetType (epid2);
    BOOST_CHECK_EQUAL (type, "");
    BOOST_CHECK_EQUAL (type2, "random");

    auto epid_b = helicsFederateGetEndpoint (mFed1, "ep2", &err);
    type = helicsEndpointGetType (epid_b);
    BOOST_CHECK_EQUAL (type, "random");

    auto epid_c = helicsFederateGetEndpointByIndex (mFed1, 0, &err);
    name = helicsEndpointGetName (epid_c);
    BOOST_CHECK_EQUAL (name, "fed0/ep1");

    CE (helicsFederateFinalize (mFed1, &err));

    CE (mFed1State = helicsFederateGetState (mFed1, &err));
    BOOST_CHECK (mFed1State == helics_federate_state::helics_state_finalize);
}

BOOST_DATA_TEST_CASE (message_federate_send_receive, bdata::make (core_types_simple), core_type)
{
    SetupTest (helicsCreateMessageFederate, core_type, 1);
    auto mFed1 = GetFederateAt (0);

    auto epid = helicsFederateRegisterEndpoint (mFed1, "ep1", NULL, &err);
    auto epid2 = helicsFederateRegisterGlobalEndpoint (mFed1, "ep2", "random", &err);
    BOOST_CHECK_EQUAL (err.error_code, helics_ok);
    CE (helicsFederateSetTimeProperty (mFed1, helics_property_time_delta, 1.0, &err));

    CE (helicsFederateEnterExecutingMode (mFed1, &err));

    CE (helics_federate_state mFed1State = helicsFederateGetState (mFed1, &err));
    BOOST_CHECK (mFed1State == helics_state_execution);
    std::string data (500, 'a');

    CE (helicsEndpointSendEventRaw (epid, "ep2", data.c_str (), 500, 0.0, &err));
    helics_time time;
    CE (time = helicsFederateRequestTime (mFed1, 1.0, &err));
    BOOST_CHECK_EQUAL (time, 1.0);

    auto res = helicsFederateHasMessage (mFed1);
    BOOST_CHECK (res);
    res = helicsEndpointHasMessage (epid);
    BOOST_CHECK (res == false);
    res = helicsEndpointHasMessage (epid2);
    BOOST_CHECK (res);

    auto M = helicsEndpointGetMessage (epid2);
    // BOOST_REQUIRE (M);
    BOOST_REQUIRE_EQUAL (M.length, 500);

    BOOST_CHECK_EQUAL (M.data[245], 'a');
    CE (helicsFederateFinalize (mFed1, &err));

    CE (mFed1State = helicsFederateGetState (mFed1, &err));
    BOOST_CHECK (mFed1State == helics_federate_state::helics_state_finalize);
}

BOOST_DATA_TEST_CASE (message_federate_send_receive_mobj, bdata::make (core_types_simple), core_type)
{
    SetupTest (helicsCreateMessageFederate, core_type, 1);
    auto mFed1 = GetFederateAt (0);

    auto epid = helicsFederateRegisterEndpoint (mFed1, "ep1", NULL, &err);
    auto epid2 = helicsFederateRegisterGlobalEndpoint (mFed1, "ep2", "random", &err);
    BOOST_CHECK_EQUAL (err.error_code, helics_ok);
    CE (helicsFederateSetTimeProperty (mFed1, helics_property_time_delta, 1.0, &err));

    CE (helicsFederateEnterExecutingMode (mFed1, &err));

    CE (helics_federate_state mFed1State = helicsFederateGetState (mFed1, &err));
    BOOST_CHECK (mFed1State == helics_state_execution);
    std::string data (500, 'a');

    CE (helicsEndpointSendEventRaw (epid, "ep2", data.c_str (), 500, 0.0, &err));
    helics_time time;
    CE (time = helicsFederateRequestTime (mFed1, 1.0, &err));
    BOOST_CHECK_EQUAL (time, 1.0);

    auto res = helicsFederateHasMessage (mFed1);
    BOOST_CHECK (res);
    res = helicsEndpointHasMessage (epid);
    BOOST_CHECK (res == false);
    res = helicsEndpointHasMessage (epid2);
    BOOST_CHECK (res);

    auto M = helicsEndpointGetMessageObject (epid2);
    // BOOST_REQUIRE (M);
    BOOST_REQUIRE_EQUAL (helicsMessageGetRawDataSize (M), 500);

    char *rdata = static_cast<char *> (helicsMessageGetRawDataPointer (M));
    BOOST_CHECK_EQUAL (rdata[245], 'a');
    CE (helicsFederateFinalize (mFed1, &err));

    CE (mFed1State = helicsFederateGetState (mFed1, &err));
    BOOST_CHECK (mFed1State == helics_federate_state::helics_state_finalize);
}

BOOST_AUTO_TEST_CASE (message_object_tests)
{
    SetupTest (helicsCreateMessageFederate, "test", 1);
    auto mFed1 = GetFederateAt (0);

    auto epid = helicsFederateRegisterEndpoint (mFed1, "ep1", NULL, &err);
    auto epid2 = helicsFederateRegisterGlobalEndpoint (mFed1, "ep2", "random", &err);
    BOOST_CHECK_EQUAL (err.error_code, helics_ok);
    CE (helicsFederateSetTimeProperty (mFed1, helics_property_time_delta, 1.0, &err));

    CE (helicsFederateEnterExecutingMode (mFed1, &err));

    CE (helics_federate_state mFed1State = helicsFederateGetState (mFed1, &err));
    BOOST_CHECK (mFed1State == helics_state_execution);
    std::string data (500, 'a');

    auto M = helicsFederateCreateMessageObject (mFed1, nullptr);
    helicsMessageSetDestination (M, "ep2", nullptr);
    BOOST_CHECK_EQUAL (helicsMessageGetDestination (M), "ep2");
    helicsMessageSetData (M, data.data (), 500, &err);
    helicsMessageSetTime (M, 0.0, &err);

    CE (helicsEndpointSendMessageObject (epid, M, &err));
    helics_time time;
    CE (time = helicsFederateRequestTime (mFed1, 1.0, &err));
    BOOST_CHECK_EQUAL (time, 1.0);

    auto res = helicsFederateHasMessage (mFed1);
    BOOST_CHECK (res);
    res = helicsEndpointHasMessage (epid);
    BOOST_CHECK (res == false);
    res = helicsEndpointHasMessage (epid2);
    BOOST_CHECK (res);

    M = helicsEndpointGetMessageObject (epid2);
    // BOOST_REQUIRE (M);
    BOOST_REQUIRE_EQUAL (helicsMessageGetRawDataSize (M), 500);

    char *rdata = static_cast<char *> (helicsMessageGetRawDataPointer (M));
    BOOST_CHECK_EQUAL (rdata[245], 'a');
    CE (helicsFederateFinalize (mFed1, &err));

    CE (mFed1State = helicsFederateGetState (mFed1, &err));
    BOOST_CHECK (mFed1State == helics_federate_state::helics_state_finalize);

    helicsMessageSetFlagOption (M, 7, helics_true, &err);
    BOOST_CHECK (helicsMessageCheckFlag (M, 7) == helics_true);
    helicsMessageClearFlags (M);
    BOOST_CHECK (helicsMessageCheckFlag (M, 7) == helics_false);
}

BOOST_DATA_TEST_CASE (message_federate_send_receive_2fed, bdata::make (core_types), core_type)
{
    // extraBrokerArgs = "--loglevel=4";
    SetupTest (helicsCreateMessageFederate, core_type, 2);
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
    BOOST_CHECK (mFed1State == helics_state_execution);
    CE (helics_federate_state mFed2State = helicsFederateGetState (mFed2, &err));
    BOOST_CHECK (mFed2State == helics_state_execution);

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

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK_EQUAL (time, 1.0);

    auto res = helicsFederateHasMessage (mFed1);
    BOOST_CHECK (res);
    res = helicsEndpointHasMessage (epid);
    BOOST_CHECK (res);
    res = helicsEndpointHasMessage (epid2);
    BOOST_CHECK (res);

    auto M1 = helicsEndpointGetMessage (epid);
    // BOOST_REQUIRE(M1);
    BOOST_REQUIRE_EQUAL (M1.length, 400);

    BOOST_CHECK_EQUAL (M1.data[245], 'b');

    auto M2 = helicsEndpointGetMessage (epid2);
    // BOOST_REQUIRE(M2);
    BOOST_REQUIRE_EQUAL (M2.length, 500);

    BOOST_CHECK_EQUAL (M2.data[245], 'a');
    CE (helicsFederateFinalizeAsync (mFed1, &err));
    CE (helicsFederateFinalize (mFed2, &err));
    CE (helicsFederateFinalizeComplete (mFed1, &err));
    CE (mFed1State = helicsFederateGetState (mFed1, &err));
    BOOST_CHECK (mFed1State == helics_federate_state::helics_state_finalize);
    CE (mFed2State = helicsFederateGetState (mFed2, &err));
    BOOST_CHECK (mFed2State == helics_federate_state::helics_state_finalize);
}

BOOST_DATA_TEST_CASE (message_federate_send_receive_2fed_multisend, bdata::make (core_types), core_type)
{
    // extraBrokerArgs = "--loglevel=4";
    SetupTest (helicsCreateMessageFederate, core_type, 2);
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
    BOOST_CHECK (mFed1State == helics_state_execution);
    CE (helics_federate_state mFed2State = helicsFederateGetState (mFed2, &err));
    BOOST_CHECK (mFed2State == helics_state_execution);

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

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK_EQUAL (time, 1.0);

    auto res = helicsEndpointPendingMessages (epid2);
    BOOST_CHECK_EQUAL (res, 3);

    res = helicsFederatePendingMessages (mFed2);
    BOOST_CHECK_EQUAL (res, 3);

    BOOST_CHECK_EQUAL (helicsEndpointGetDefaultDestination (epid), "ep2");

    CE (helicsFederateFinalizeAsync (mFed1, &err));
    CE (helicsFederateFinalize (mFed2, &err));
    CE (helicsFederateFinalizeComplete (mFed1, &err));
    CE (mFed1State = helicsFederateGetState (mFed1, &err));
    BOOST_CHECK (mFed1State == helics_federate_state::helics_state_finalize);
    CE (mFed2State = helicsFederateGetState (mFed2, &err));
    BOOST_CHECK (mFed2State == helics_federate_state::helics_state_finalize);
}

BOOST_AUTO_TEST_SUITE_END ()
