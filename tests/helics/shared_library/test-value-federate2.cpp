/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details.
*/

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "ctestFixtures.hpp"
#include <future>
#include <iostream>

/** these test cases test out the value converters and some of the other functions
 */
namespace bdata = boost::unit_test::data;
namespace utf = boost::unit_test;

BOOST_FIXTURE_TEST_SUITE (value_federate_tests2, FederateTestFixture, *utf::label ("ci"))

// const std::string core_types[] = { "test", "ipc", "zmq", "test_2", "ipc_2", "zmq_2" };

/** test block send and receive*/

BOOST_DATA_TEST_CASE (test_block_send_receive, bdata::make (core_types), core_type)
{
    std::string s (500, ';');
    int len = static_cast<int> (s.size ());
    char val[600] = "";
    int actualLen = 10;
    FederateTestFixture fixture;
    fixture.SetupTest (helicsCreateValueFederate, core_type, 1);
    auto vFed1 = fixture.GetFederateAt (0);
    auto pubid1 = helicsFederateRegisterTypePublication (vFed1, "pub1", "string", "", &err);
    BOOST_CHECK (pubid1 != nullptr);
    auto pubid2 = helicsFederateRegisterGlobalTypePublication (vFed1, "pub2", "integer", "", &err);
    BOOST_CHECK (pubid2 != nullptr);
    auto pubid3 = helicsFederateRegisterTypePublication (vFed1, "pub3", "", "", &err);
    BOOST_CHECK (pubid3 != nullptr);
    auto sub1 = helicsFederateRegisterSubscription (vFed1, "fed0/pub3", "", &err);
    CE (helicsFederateSetTimeProperty (vFed1, helics_property_time_delta, 1.0, &err));

    CE (helicsFederateEnterExecutingMode (vFed1, &err));
    CE (helicsPublicationPublishRaw (pubid3, s.data (), len, &err));

    CE (helicsFederateRequestTime (vFed1, 1.0, &err));

    BOOST_CHECK (helicsInputIsUpdated (sub1));

    int len1 = helicsInputGetRawValueSize (sub1);

    BOOST_CHECK_EQUAL (len1, len);
    CE (helicsInputGetRawValue (sub1, val, 600, &actualLen, &err));
    BOOST_CHECK_EQUAL (actualLen, len);

    len1 = helicsInputGetRawValueSize (sub1);

    BOOST_CHECK_EQUAL (len1, len);

    BOOST_CHECK (helicsInputIsUpdated (sub1) == false);

    CE (helicsFederateFinalize (vFed1, &err));
}

BOOST_DATA_TEST_CASE (test_async_calls, bdata::make (core_types_simple), core_type)
{
    helics_time gtime;
    helics_time f1time;
    // helics_federate_state state;
#define STRINGLEN 100
    char s[STRINGLEN] = "";
    FederateTestFixture fixture;
    fixture.SetupTest (helicsCreateValueFederate, core_type, 2);
    auto vFed1 = fixture.GetFederateAt (0);
    auto vFed2 = fixture.GetFederateAt (1);

    // register the publications
    auto pubid = helicsFederateRegisterGlobalPublication (vFed1, "pub1", helics_data_type_string, nullptr, &err);
    auto subid = helicsFederateRegisterSubscription (vFed2, "pub1", "", &err);
    CE (helicsFederateSetTimeProperty (vFed1, helics_property_time_delta, 1.0, &err));
    CE (helicsFederateSetTimeProperty (vFed2, helics_property_time_delta, 1.0, &err));

    CE (helicsFederateEnterExecutingModeAsync (vFed1, &err));
    CE (helicsFederateEnterExecutingModeAsync (vFed2, &err));
    CE (helicsFederateEnterExecutingModeComplete (vFed1, &err));
    CE (helicsFederateEnterExecutingModeComplete (vFed2, &err));

    // publish string1 at time=0.0;
    CE (helicsPublicationPublishString (pubid, "string1", &err));

    CE (helicsFederateRequestTimeAsync (vFed1, 1.0, &err));
    CE (f1time = helicsFederateRequestTimeComplete (vFed1, &err));
    CE (helicsFederateRequestTimeAsync (vFed2, 1.0, &err));
    CE (gtime = helicsFederateRequestTimeComplete (vFed2, &err));

    BOOST_CHECK_EQUAL (f1time, 1.0);
    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    CE (helicsInputGetString (subid, s, STRINGLEN, nullptr, &err));

    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (s, "string1");

    // publish a second string
    CE (helicsPublicationPublishString (pubid, "string2", &err));

    // make sure the value is still what we expect
    CE (helicsInputGetString (subid, s, STRINGLEN, nullptr, &err));
    BOOST_CHECK_EQUAL (s, "string1");

    // advance time
    CE (helicsFederateRequestTimeAsync (vFed1, 2.0, &err));
    CE (f1time = helicsFederateRequestTimeComplete (vFed1, &err));
    CE (helicsFederateRequestTimeAsync (vFed2, 2.0, &err));
    CE (gtime = helicsFederateRequestTimeComplete (vFed2, &err));

    BOOST_CHECK_EQUAL (f1time, 2.0);
    BOOST_CHECK_EQUAL (gtime, 2.0);

    // make sure the value was updated
    CE (helicsInputGetString (subid, s, STRINGLEN, nullptr, &err));
    BOOST_CHECK_EQUAL (s, "string2");

    CE (helicsFederateFinalize (vFed1, &err));
    CE (helicsFederateFinalize (vFed2, &err));
}
//

//
BOOST_AUTO_TEST_CASE (test_file_load)
{
    helics_federate vFed;
    // fi = helicsCreateFederateInfo();
    // path of the JSON file is hardcoded for now
    vFed = helicsCreateValueFederateFromConfig (TEST_DIR "/example_value_fed.json", &err);
    BOOST_CHECK_EQUAL (err.error_code, helics_ok);
    BOOST_REQUIRE (vFed != nullptr);
    const char *s = helicsFederateGetName (vFed);
    BOOST_CHECK_EQUAL (s, "valueFed");
    BOOST_CHECK_EQUAL (helicsFederateGetInputCount (vFed), 3);
    BOOST_CHECK_EQUAL (helicsFederateGetPublicationCount (vFed), 2);
    //	 helics::ValueFederate vFed(std::string(TEST_DIR) + "/test_files/example_value_fed.json");
    CE (helicsFederateFinalize (vFed, &err));
    //
    //	 BOOST_CHECK_EQUAL(vFed.getName(), "fedName");

    //	 BOOST_CHECK_EQUAL(vFed.getSubscriptionCount(), 2);
    //	 BOOST_CHECK_EQUAL(vFed.getPublicationCount(), 2);
    helicsFederateFree (vFed);
}

BOOST_AUTO_TEST_CASE (test_json_publish, *utf::label ("ci"))
{
    SetupTest (helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt (0);
    BOOST_REQUIRE (vFed1 != nullptr);
    helicsFederateSetSeparator (vFed1, '/', nullptr);

    helicsFederateRegisterGlobalPublication (vFed1, "pub1", helics_data_type_double, "", nullptr);
    helicsFederateRegisterPublication (vFed1, "pub2", helics_data_type_string, "", nullptr);
    helicsFederateRegisterPublication (vFed1, "group1/pubA", helics_data_type_double, "", nullptr);
    helicsFederateRegisterPublication (vFed1, "group1/pubB", helics_data_type_string, "", nullptr);

    auto s1 = helicsFederateRegisterSubscription (vFed1, "pub1", nullptr, nullptr);
    auto s2 = helicsFederateRegisterSubscription (vFed1, "fed0/pub2", nullptr, nullptr);
    auto s3 = helicsFederateRegisterSubscription (vFed1, "fed0/group1/pubA", nullptr, nullptr);
    auto s4 = helicsFederateRegisterSubscription (vFed1, "fed0/group1/pubB", nullptr, nullptr);
    helicsFederateEnterExecutingMode (vFed1, nullptr);
    helicsFederatePublishJSON (vFed1, (std::string (TEST_DIR) + "example_pub_input1.json").c_str (), nullptr);
    helicsFederateRequestTime (vFed1, 1.0, nullptr);
    BOOST_CHECK_EQUAL (helicsInputGetDouble (s1, nullptr), 99.9);
    char buffer[50];
    int actLen = 0;
    helicsInputGetString (s2, buffer, 50, &actLen, nullptr);
    BOOST_CHECK_EQUAL (buffer, "things");
    BOOST_CHECK_EQUAL (helicsInputGetDouble (s3, nullptr), 45.7);
    helicsInputGetString (s4, buffer, 50, &actLen, nullptr);
    BOOST_CHECK_EQUAL (buffer, "count");

    helicsFederatePublishJSON (vFed1, (std::string (TEST_DIR) + "example_pub_input2.json").c_str (), nullptr);
    helicsFederateRequestTime (vFed1, 2.0, nullptr);
    BOOST_CHECK_EQUAL (helicsInputGetDouble (s1, nullptr), 88.2);

    helicsInputGetString (s2, buffer, 50, &actLen, nullptr);
    BOOST_CHECK_EQUAL (buffer, "items");
    BOOST_CHECK_EQUAL (helicsInputGetDouble (s3, nullptr), 15.0);
    helicsInputGetString (s4, buffer, 50, &actLen, nullptr);
    BOOST_CHECK_EQUAL (buffer, "count2");

    helicsFederatePublishJSON (vFed1, "{\"pub1\": 77.2}", nullptr);

    helicsFederateRequestTime (vFed1, 3.0, nullptr);
    BOOST_CHECK_EQUAL (helicsInputGetDouble (s1, nullptr), 77.2);

    CE (helicsFederateFinalize (vFed1, &err));
}

BOOST_AUTO_TEST_CASE (test_json_register_publish, *utf::label ("ci"))
{
    SetupTest (helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt (0);
    BOOST_REQUIRE (vFed1 != nullptr);
    helicsFederateSetSeparator (vFed1, '/', nullptr);

    CE (helicsFederateRegisterFromPublicationJSON (vFed1,
                                                   (std::string (TEST_DIR) + "example_pub_input1.json").c_str (),
                                                   &err));

    auto s1 = helicsFederateRegisterSubscription (vFed1, "fed0/pub1", nullptr, nullptr);
    auto s2 = helicsFederateRegisterSubscription (vFed1, "fed0/pub2", nullptr, nullptr);
    auto s3 = helicsFederateRegisterSubscription (vFed1, "fed0/group1/pubA", nullptr, nullptr);
    auto s4 = helicsFederateRegisterSubscription (vFed1, "fed0/group1/pubB", nullptr, nullptr);
    helicsFederateEnterExecutingMode (vFed1, nullptr);
    helicsFederatePublishJSON (vFed1, (std::string (TEST_DIR) + "example_pub_input1.json").c_str (), nullptr);
    helicsFederateRequestTime (vFed1, 1.0, nullptr);
    BOOST_CHECK_EQUAL (helicsInputGetDouble (s1, nullptr), 99.9);
    char buffer[50];
    int actLen = 0;
    helicsInputGetString (s2, buffer, 50, &actLen, nullptr);
    BOOST_CHECK_EQUAL (buffer, "things");
    BOOST_CHECK_EQUAL (helicsInputGetDouble (s3, nullptr), 45.7);
    helicsInputGetString (s4, buffer, 50, &actLen, nullptr);
    BOOST_CHECK_EQUAL (buffer, "count");

    helicsFederatePublishJSON (vFed1, (std::string (TEST_DIR) + "example_pub_input2.json").c_str (), nullptr);
    helicsFederateRequestTime (vFed1, 2.0, nullptr);
    BOOST_CHECK_EQUAL (helicsInputGetDouble (s1, nullptr), 88.2);

    helicsInputGetString (s2, buffer, 50, &actLen, nullptr);
    BOOST_CHECK_EQUAL (buffer, "items");
    BOOST_CHECK_EQUAL (helicsInputGetDouble (s3, nullptr), 15.0);
    helicsInputGetString (s4, buffer, 50, &actLen, nullptr);
    BOOST_CHECK_EQUAL (buffer, "count2");

    CE (helicsFederateFinalize (vFed1, &err));
}
BOOST_AUTO_TEST_SUITE_END ()
