/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include "../src/helics/cpp98/ValueFederate.hpp"
#include "cpptestFixtures.hpp"
#include <future>
#include <iostream>

/** these test cases test out the value converters and some of the other functions
 */
namespace bdata = boost::unit_test::data;
namespace utf = boost::unit_test;

BOOST_FIXTURE_TEST_SUITE (value_federate_tests2_cpp, FederateTestFixture_cpp, *utf::label("ci"))

// const std::string core_types[] = { "test", "ipc", "zmq", "test_2", "ipc_2", "zmq_2" };

/** test block send and receive*/

BOOST_DATA_TEST_CASE (test_block_send_receive, bdata::make (core_types), core_type)
{
    helics_time_t gtime;
    std::string s (500, ';');
    int len = static_cast<int> (s.size ());
    SetupTest<helics::ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto pubid1 = vFed1->registerPublication ("pub1", "string", "");
    BOOST_CHECK (pubid1.baseObject () != nullptr);
    auto pubid2 = vFed1->registerGlobalPublication ("pub2", "integer", "");
    BOOST_CHECK (pubid2.baseObject () != nullptr);
    auto pubid3 = vFed1->registerPublication ("pub3", "", "");
    BOOST_CHECK (pubid3.baseObject () != nullptr);
    auto sub1 = vFed1->registerOptionalSubscription ("fed0/pub3", "", "");
    vFed1->setTimeDelta (1.0);

    vFed1->enterExecutionMode ();
    pubid3.publish (s);

    gtime = vFed1->requestTime (1.0);
    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK (sub1.isUpdated ());

    int len1 = sub1.getValueSize ();

    BOOST_CHECK_EQUAL (len1, len);
    std::vector<char> rawdata;
    int actualLen = sub1.getRawValue (rawdata);
    BOOST_CHECK_EQUAL (actualLen, len);

    len1 = sub1.getValueSize ();

    BOOST_CHECK_EQUAL (len1, len);

    BOOST_CHECK (sub1.isUpdated () == false);

    vFed1->finalize ();
}

BOOST_DATA_TEST_CASE (test_async_calls, bdata::make (core_types), core_type)
{
    helics_time_t gtime;
    helics_time_t f1time;
    // federate_state state;
    SetupTest<helics::ValueFederate> (core_type, 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

    // register the publications
    auto pubid = vFed1->registerGlobalTypePublication ("pub1", HELICS_DATA_TYPE_STRING, "");
    auto subid = vFed2->registerSubscription ("pub1", "string", "");
    vFed1->setTimeDelta (1.0);
    vFed2->setTimeDelta (1.0);

    vFed1->enterExecutionModeAsync ();
    vFed2->enterExecutionModeAsync ();
    vFed1->enterExecutionModeComplete ();
    vFed2->enterExecutionModeComplete ();

    // publish string1 at time=0.0;
    pubid.publish ("string1");

    vFed1->requestTimeAsync (1.0);
    vFed2->requestTimeAsync (1.0);
    f1time = vFed1->requestTimeComplete ();
    gtime = vFed2->requestTimeComplete ();

    BOOST_CHECK_EQUAL (f1time, 1.0);
    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    auto s = subid.getString ();

    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (s, "string1");

    // publish a second string
    pubid.publish ("string2");

    // make sure the value is still what we expect
    s = subid.getString ();
    BOOST_CHECK_EQUAL (s, "string1");

    // advance time
    vFed1->requestTimeAsync (2.0);
    vFed2->requestTimeAsync (2.0);
    f1time = vFed1->requestTimeComplete ();
    gtime = vFed2->requestTimeComplete ();

    BOOST_CHECK_EQUAL (f1time, 2.0);
    BOOST_CHECK_EQUAL (gtime, 2.0);

    // make sure the value was updated
    s = subid.getString ();
    BOOST_CHECK_EQUAL (s, "string2");

    vFed1->finalize ();
    vFed2->finalize ();
}
//

//
BOOST_AUTO_TEST_CASE (test_file_load)
{
    // fi = helicsFederateInfoCreate();
    // path of the json file is hardcoded for now
    helics::ValueFederate vFed (TEST_DIR "/test_files/example_value_fed.json");
    BOOST_REQUIRE (vFed.baseObject () != nullptr);
    auto s = vFed.getName ();
    BOOST_CHECK_EQUAL (s, "valueFed");
    BOOST_CHECK_EQUAL (vFed.getSubscriptionCount (), 2);
    BOOST_CHECK_EQUAL (vFed.getPublicationCount (), 2);
    //	 helics::ValueFederate vFed(std::string(TEST_DIR) + "/test_files/example_value_fed.json");
    vFed.finalize ();
}

BOOST_AUTO_TEST_SUITE_END ()
