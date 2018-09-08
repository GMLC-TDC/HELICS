/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
*/

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include "ctestFixtures.hpp"
#include <future>
#include <iostream>

/** these test cases test out the value converters and some of the other functions
 */
namespace bdata = boost::unit_test::data;
namespace utf = boost::unit_test;

BOOST_FIXTURE_TEST_SUITE (value_federate_tests2, FederateTestFixture, *utf::label("ci"))

// const std::string core_types[] = { "test", "ipc", "zmq", "test_2", "ipc_2", "zmq_2" };

/** test block send and receive*/

BOOST_DATA_TEST_CASE (test_block_send_receive, bdata::make (core_types), core_type)
{
    std::string s (500, ';');
    int len = static_cast<int> (s.size ());
    char val[600] = "";
    int actualLen = 10;
    FederateTestFixture fixture;
    fixture.SetupTest (helicsCreateValueFederate, core_type.c_str (), 1);
    auto vFed1 = fixture.GetFederateAt (0);
    auto pubid1 = helicsFederateRegisterTypePublication (vFed1, "pub1", "string", "",&err);
    BOOST_CHECK (pubid1 != nullptr);
    auto pubid2 = helicsFederateRegisterGlobalTypePublication (vFed1, "pub2", "integer", "",&err);
    BOOST_CHECK (pubid2 != nullptr);
    auto pubid3 = helicsFederateRegisterTypePublication (vFed1, "pub3", "", "",&err);
    BOOST_CHECK (pubid3 != nullptr);
    auto sub1 = helicsFederateRegisterSubscription (vFed1, "fed0/pub3", "",&err);
    CE(helicsFederateSetTimeProperty (vFed1, TIME_DELTA_PROPERTY, 1.0,&err));

    CE(helicsFederateEnterExecutingMode (vFed1,&err));
    CE (helicsPublicationPublishRaw (pubid3, s.data (), len, &err));

    CE(helicsFederateRequestTime(vFed1, 1.0,&err));

    BOOST_CHECK (helicsInputIsUpdated (sub1));

    int len1 = helicsInputGetRawValueSize (sub1);

    BOOST_CHECK_EQUAL (len1, len);
    CE(actualLen=helicsInputGetRawValue (sub1, val, 600, &err));
    BOOST_CHECK_EQUAL (actualLen, len);

    len1 = helicsInputGetRawValueSize (sub1);

    BOOST_CHECK_EQUAL (len1, len);

    BOOST_CHECK (helicsInputIsUpdated (sub1) == false);

    CE(helicsFederateFinalize (vFed1,&err));
}

BOOST_DATA_TEST_CASE (test_async_calls, bdata::make (core_types), core_type)
{
    helics_time_t gtime;
    helics_time_t f1time;
    // federate_state state;
#define STRINGLEN 100
    char s[STRINGLEN] = "";
    FederateTestFixture fixture;
    fixture.SetupTest (helicsCreateValueFederate, core_type.c_str (), 2);
    auto vFed1 = fixture.GetFederateAt (0);
    auto vFed2 = fixture.GetFederateAt (1);

    // register the publications
    auto pubid = helicsFederateRegisterGlobalTypePublication (vFed1, "pub1", HELICS_DATA_TYPE_STRING, nullptr,&err);
    auto subid = helicsFederateRegisterSubscription (vFed2, "pub1", "",&err);
    CE(helicsFederateSetTimeProperty (vFed1, TIME_DELTA_PROPERTY, 1.0,&err));
    CE(helicsFederateSetTimeProperty (vFed2, TIME_DELTA_PROPERTY, 1.0,&err));

    CE(helicsFederateEnterExecutingModeAsync (vFed1,&err));
    CE(helicsFederateEnterExecutingModeAsync (vFed2,&err));
    CE(helicsFederateEnterExecutingModeComplete (vFed1,&err));
    CE(helicsFederateEnterExecutingModeComplete (vFed2,&err));

    // publish string1 at time=0.0;
    CE(helicsPublicationPublishString (pubid, "string1",&err));

    CE(helicsFederateRequestTimeAsync (vFed1, 1.0,&err));
    CE(f1time=helicsFederateRequestTimeComplete (vFed1, &err));
    CE(helicsFederateRequestTimeAsync (vFed2, 1.0,&err));
    CE(gtime=helicsFederateRequestTimeComplete (vFed2, &err));

    BOOST_CHECK_EQUAL (f1time, 1.0);
    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    CE(helicsInputGetString (subid, s, STRINGLEN, &err));

    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (s, "string1");

    // publish a second string
    CE(helicsPublicationPublishString (pubid, "string2",&err));

    // make sure the value is still what we expect
    CE(helicsInputGetString (subid, s, STRINGLEN, &err));
    BOOST_CHECK_EQUAL (s, "string1");

    // advance time
    CE(helicsFederateRequestTimeAsync (vFed1, 2.0,&err));
    CE(f1time=helicsFederateRequestTimeComplete (vFed1, &err));
    CE(helicsFederateRequestTimeAsync (vFed2, 2.0,&err));
    CE(gtime=helicsFederateRequestTimeComplete (vFed2, &err));

    BOOST_CHECK_EQUAL (f1time, 2.0);
    BOOST_CHECK_EQUAL (gtime, 2.0);

    // make sure the value was updated
    CE(helicsInputGetString (subid, s, STRINGLEN,&err));
    BOOST_CHECK_EQUAL (s, "string2");

    CE(helicsFederateFinalize (vFed1,&err));
    CE(helicsFederateFinalize (vFed2,&err));
}
//

//
BOOST_AUTO_TEST_CASE (test_file_load)
{
    helics_federate vFed;
    char s[100] = "";
    // fi = helicsCreateFederateInfo();
    // path of the JSON file is hardcoded for now
    vFed = helicsCreateValueFederateFromConfig (TEST_DIR "/test_files/example_value_fed.json",&err);
    BOOST_CHECK_EQUAL (err.error_code, helics_ok);
    BOOST_REQUIRE (vFed != nullptr);
    CE(helicsFederateGetName (vFed, s, 100,&err));
    BOOST_CHECK_EQUAL (s, "valueFed");
    BOOST_CHECK_EQUAL (helicsFederateGetInputCount (vFed), 2);
    BOOST_CHECK_EQUAL (helicsFederateGetPublicationCount (vFed), 2);
    //	 helics::ValueFederate vFed(std::string(TEST_DIR) + "/test_files/example_value_fed.json");
    CE(helicsFederateFinalize (vFed,&err));
    //
    //	 BOOST_CHECK_EQUAL(vFed.getName(), "fedName");

    //	 BOOST_CHECK_EQUAL(vFed.getSubscriptionCount(), 2);
    //	 BOOST_CHECK_EQUAL(vFed.getPublicationCount(), 2);
    helicsFederateFree (vFed);
}
BOOST_AUTO_TEST_SUITE_END ()
