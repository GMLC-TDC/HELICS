/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
*/

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <boost/filesystem.hpp>

#include <future>
#include <iostream>
#include "ctestFixtures.hpp"

/** these test cases test out the value converters and some of the other functions
*/

BOOST_FIXTURE_TEST_SUITE(value_federate_tests2, FederateTestFixture)

namespace bdata = boost::unit_test::data;
//const std::string core_types[] = { "test", "ipc", "zmq", "test_2", "ipc_2", "zmq_2" };

/** test block send and receive*/

BOOST_DATA_TEST_CASE(test_block_send_receive, bdata::make(core_types), core_type)
{
	helics_time_t gtime;
    std::string s(500,';');
	int len = static_cast<int>(s.size());
	char val[600] = "";
	int actualLen = 10;
    FederateTestFixture fixture;
    fixture.SetupTest(helicsCreateValueFederate, core_type.c_str(), 1);
    auto vFed1 = fixture.GetFederateAt(0);
	auto pubid1 = helicsFederateRegisterPublication(vFed1, "pub1", "string", "");
    BOOST_CHECK(pubid1 != nullptr);
	auto pubid2 = helicsFederateRegisterGlobalPublication(vFed1, "pub2", "integer", "");
    BOOST_CHECK(pubid2 != nullptr);
	auto pubid3 = helicsFederateRegisterPublication(vFed1, "pub3", "", "");
    BOOST_CHECK(pubid3 != nullptr);
	auto sub1 = helicsFederateRegisterOptionalSubscription(vFed1, "fed0/pub3", "", "");
    CE(helicsFederateSetTimeDelta(vFed1, 1.0));

    CE(helicsFederateEnterExecutionMode(vFed1));
    CE(helicsPublicationPublish(pubid3, s.data(), len));

    CE(helicsFederateRequestTime(vFed1, 1.0, &gtime));

	BOOST_CHECK(helicsSubscriptionIsUpdated(sub1));

    int len1 = helicsSubscriptionGetValueSize(sub1);

    BOOST_CHECK_EQUAL(len1, len);
    CE(helicsSubscriptionGetValue(sub1, val, 600, &actualLen));
    BOOST_CHECK_EQUAL(actualLen, len);

    len1 = helicsSubscriptionGetValueSize(sub1);

    BOOST_CHECK_EQUAL(len1, len);



	BOOST_CHECK(helicsSubscriptionIsUpdated(sub1) == false);

    CE(helicsFederateFinalize(vFed1));

	
}

BOOST_DATA_TEST_CASE(test_async_calls, bdata::make(core_types), core_type)
{
	
	helics_time_t gtime;
	helics_time_t f1time;
	//federate_state state;
	char s[100] = "";
    FederateTestFixture fixture;
    fixture.SetupTest(helicsCreateValueFederate, core_type.c_str(), 2);
    auto vFed1 = fixture.GetFederateAt(0);
    auto vFed2 = fixture.GetFederateAt(1);


	// register the publications
	auto pubid = helicsFederateRegisterGlobalTypePublication(vFed1, "pub1", HELICS_DATA_TYPE_STRING, nullptr);
	auto subid = helicsFederateRegisterSubscription(vFed2, "pub1", "string", "");
    CE(helicsFederateSetTimeDelta(vFed1, 1.0));
    CE(helicsFederateSetTimeDelta(vFed2, 1.0));

    CE(helicsFederateEnterExecutionModeAsync(vFed1));
    CE(helicsFederateEnterExecutionModeAsync(vFed2));
    CE(helicsFederateEnterExecutionModeComplete(vFed1));
    CE(helicsFederateEnterExecutionModeComplete(vFed2));

	// publish string1 at time=0.0;
    CE(helicsPublicationPublishString(pubid, "string1"));

    CE(helicsFederateRequestTimeAsync(vFed1, 1.0));
    CE(helicsFederateRequestTimeComplete(vFed1, &f1time));
    CE(helicsFederateRequestTimeAsync(vFed2, 1.0));
    CE(helicsFederateRequestTimeComplete(vFed2, &gtime));

	BOOST_CHECK_EQUAL(f1time, 1.0);
	BOOST_CHECK_EQUAL(gtime, 1.0);

	// get the value
    CE(helicsSubscriptionGetString(subid, s, 100));

	// make sure the string is what we expect
	BOOST_CHECK_EQUAL(s, "string1");

	// publish a second string
    CE(helicsPublicationPublishString(pubid, "string2"));

	// make sure the value is still what we expect
    CE(helicsSubscriptionGetString(subid, s, 100));
	BOOST_CHECK_EQUAL(s, "string1");

	// advance time
    CE(helicsFederateRequestTimeAsync(vFed1, 2.0));
    CE(helicsFederateRequestTimeComplete(vFed1, &f1time));
    CE(helicsFederateRequestTimeAsync(vFed2, 2.0));
    CE(helicsFederateRequestTimeComplete(vFed2, &gtime));

	BOOST_CHECK_EQUAL(f1time, 2.0);
	BOOST_CHECK_EQUAL(gtime, 2.0);

	// make sure the value was updated
    CE(helicsSubscriptionGetString(subid, s, 100));
	BOOST_CHECK_EQUAL(s, "string2");

    CE(helicsFederateFinalize(vFed1));
    CE(helicsFederateFinalize(vFed2));
}
//

//
BOOST_AUTO_TEST_CASE(test_file_load)
{
	helics_federate vFed;
	char s[100] = "";
	//fi = helicsFederateInfoCreate();
	// path of the json file is hardcoded for now
	vFed = helicsCreateValueFederateFromJson(TEST_DIR "/test_files/example_value_fed.json");
    BOOST_REQUIRE(vFed!=nullptr);
	CE (helicsFederateGetName(vFed, s, 100));
	BOOST_CHECK_EQUAL(s, "valueFed");
	BOOST_CHECK_EQUAL(helicsFederateGetSubscriptionCount(vFed), 2);
	BOOST_CHECK_EQUAL(helicsFederateGetPublicationCount(vFed), 2);
	//	 helics::ValueFederate vFed(std::string(TEST_DIR) + "/test_files/example_value_fed.json");
    CE(helicsFederateFinalize(vFed));
	//
	//	 BOOST_CHECK_EQUAL(vFed.getName(), "fedName");

	//	 BOOST_CHECK_EQUAL(vFed.getSubscriptionCount(), 2);
	//	 BOOST_CHECK_EQUAL(vFed.getPublicationCount(), 2);
    helicsFederateFree(vFed);
}
BOOST_AUTO_TEST_SUITE_END()

