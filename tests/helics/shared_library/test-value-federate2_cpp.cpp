/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include <boost/test/unit_test.hpp>
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

BOOST_FIXTURE_TEST_SUITE (value_federate_tests2_cpp, FederateTestFixture_cpp, *utf::label ("ci"))

// const std::string core_types[] = { "test", "ipc", "zmq", "test_2", "ipc_2", "zmq_2" };

/** test block send and receive*/

BOOST_DATA_TEST_CASE (test_block_send_receive, bdata::make (core_types), core_type)
{
    helics_time gtime;
    std::string s (500, ';');
    int len = static_cast<int> (s.size ());
    BOOST_TEST_CHECKPOINT ("calling setup");
    SetupTest<helicscpp::ValueFederate> (core_type, 1);
    BOOST_TEST_CHECKPOINT ("calling get federate");
    auto vFed1 = GetFederateAs<helicscpp::ValueFederate> (0);
    BOOST_REQUIRE ((vFed1));
    auto pubid1 = vFed1->registerTypePublication ("pub1", "string", "");
    BOOST_CHECK (pubid1.baseObject () != nullptr);
    auto pubid2 = vFed1->registerGlobalPublication ("pub2", helics_data_type_int, "");
    BOOST_CHECK (pubid2.baseObject () != nullptr);
    auto pubid3 = vFed1->registerTypePublication ("pub3", "");
    BOOST_CHECK (pubid3.baseObject () != nullptr);
    auto sub1 = vFed1->registerSubscription ("fed0/pub3");
    BOOST_TEST_CHECKPOINT ("reg opt1");
    vFed1->setProperty (helics_property_time_delta, 1.0);
    BOOST_TEST_CHECKPOINT ("set Delta");
    vFed1->enterExecutingMode ();
    BOOST_TEST_CHECKPOINT ("publish");
    pubid3.publish (s);
    BOOST_TEST_CHECKPOINT ("reqtime");
    gtime = vFed1->requestTime (1.0);
    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK (sub1.isUpdated ());

    int len1 = sub1.getRawValueSize ();

    BOOST_CHECK_EQUAL (len1, len);
    std::vector<char> rawdata;
    int actualLen = sub1.getRawValue (rawdata);
    BOOST_CHECK_EQUAL (actualLen, len);

    len1 = sub1.getRawValueSize ();

    BOOST_CHECK_EQUAL (len1, len);

    BOOST_CHECK (sub1.isUpdated () == false);

    vFed1->finalize ();
}

BOOST_DATA_TEST_CASE (test_async_calls, bdata::make (core_types), core_type)
{
    helics_time gtime;
    helics_time f1time;
    // helics_federate_state state;
    SetupTest<helicscpp::ValueFederate> (core_type, 2);
    auto vFed1 = GetFederateAs<helicscpp::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helicscpp::ValueFederate> (1);

    // register the publications
    auto pubid = vFed1->registerGlobalPublication ("pub1", helics_data_type_string, "");
    auto subid = vFed2->registerSubscription ("pub1");
    vFed1->setProperty (helics_property_time_delta, 1.0);
    vFed2->setProperty (helics_property_time_delta, 1.0);

    vFed1->enterExecutingModeAsync ();
    vFed2->enterExecutingModeAsync ();
    vFed1->enterExecutingModeComplete ();
    vFed2->enterExecutingModeComplete ();

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
    // fi = helicsCreateFederateInfo();
    // path of the JSON file is hardcoded for now
    helicscpp::ValueFederate vFed (TEST_DIR "/example_value_fed.json");
    BOOST_REQUIRE (vFed.baseObject () != nullptr);
    auto s = vFed.getName ();
    BOOST_CHECK_EQUAL (s, "valueFed");
    BOOST_CHECK_EQUAL (vFed.getInputCount (), 3);
    BOOST_CHECK_EQUAL (vFed.getPublicationCount (), 2);
    //	 helicscpp::ValueFederate vFed(std::string(TEST_DIR) + "/test_files/example_value_fed.json");
    vFed.finalize ();
}

BOOST_AUTO_TEST_SUITE_END ()
