/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.
This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <boost/filesystem.hpp>

#include <future>
#include <iostream>
#include "ctestFixtures.hpp"

#include "test_configuration.h"

/** these test cases test out the value converters and some of the other functions
*/

// BOOST_FIXTURE_TEST_SUITE (value_federate_tests2, FederateTestFixture)
BOOST_AUTO_TEST_SUITE(value_federate_tests2)

namespace bdata = boost::unit_test::data;
//const std::string core_types[] = { "test", "ipc", "zmq", "test_2", "ipc_2", "zmq_2" };

/** test block send and receive*/

BOOST_DATA_TEST_CASE(test_block_send_receive, bdata::make(core_types), core_type)
{
	helics_status status;
	helics_federate_info_t fi;
	helics_broker broker;
	helics_federate vFed1;
	helics_publication pubid1, pubid2, pubid3;
	helics_subscription sub1;
	helics_time_t gtime;
	const char s[] = ";";
	int len = static_cast<int>(strlen(s));
	char val[100] = "";
	int actualLen = 10;
	broker = helicsCreateBroker(core_type.c_str(), "", "--federates=1");
	fi = helicsFederateInfoCreate();
	status = helicsFederateInfoSetFederateName(fi, "fed0");
	status = helicsFederateInfoSetCoreTypeFromString(fi, core_type.c_str());
	vFed1 = helicsCreateValueFederate(fi);

	pubid1 = helicsFederateRegisterPublication(vFed1, "pub1", "string", "");
	pubid2 = helicsFederateRegisterGlobalPublication(vFed1, "pub2", "integer", "");
	pubid3 = helicsFederateRegisterPublication(vFed1, "pub3", "", "");
	sub1 = helicsFederateRegisterOptionalSubscription(vFed1, "fed0/pub3", "", "");
	status = helicsFederateSetTimeDelta(vFed1, 1.0);

	status = helicsFederateEnterExecutionMode(vFed1);
	status = helicsPublicationPublish(pubid3, s, len);

	status = helicsFederateRequestTime(vFed1, 1.0, &gtime);

	BOOST_CHECK(helicsSubscriptionIsUpdated(sub1));

    int len1 = helicsSubscriptionGetValueSize(sub1);

    BOOST_CHECK_EQUAL(len1, len);
	status = helicsSubscriptionGetValue(sub1, val, 100, &actualLen);
    BOOST_CHECK_EQUAL(actualLen, len);

    len1 = helicsSubscriptionGetValueSize(sub1);

    BOOST_CHECK_EQUAL(len1, len);

	

	BOOST_CHECK(helicsSubscriptionIsUpdated(sub1) == false);

	helicsFederateInfoFree(fi);
	helicsFederateFree(vFed1);
	helicsBrokerFree(broker);
	helicsCloseLibrary();

	//SetupSingleBrokerTest<helics::ValueFederate> (core_type, 1);
	//auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

	//vFed1->registerPublication<std::string> ("pub1");
	//vFed1->registerGlobalPublication<int> ("pub2");

	//auto pubid3 = vFed1->registerPublication ("pub3", "");

	//auto sub1 = vFed1->registerOptionalSubscription ("fed0/pub3", "");

	//helics::data_block db (547, ';');

	//vFed1->enterExecutionState ();
	//vFed1->publish (pubid3, db);
	//vFed1->requestTime (1.0);
	//BOOST_CHECK (vFed1->isUpdated (sub1));
	//auto res = vFed1->getValueRaw (sub1);
	//BOOST_CHECK_EQUAL (res.size (), db.size ());
	//BOOST_CHECK (vFed1->isUpdated (sub1) == false);
}
// /** test the all callback*/
//#if ENABLE_TEST_TIMEOUTS > 0
// BOOST_TEST_DECORATOR(*utf::timeout(5))
//#endif
//	 BOOST_DATA_TEST_CASE(test_all_callback, bdata::make(core_types_single), core_type)
// {
//	 SetupSingleBrokerTest<helics::ValueFederate>(core_type, 1, 1.0);
//	 auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
//
//	 auto pubid1 = vFed1->registerPublication<std::string>("pub1");
//	 auto pubid2 = vFed1->registerGlobalPublication<int>("pub2");
//
//	 auto pubid3 = vFed1->registerPublication("pub3", "");
//
//	 auto sub1 = vFed1->registerOptionalSubscription("fed0/pub1", "");
//	 auto sub2 = vFed1->registerOptionalSubscription("pub2", "");
//	 auto sub3 = vFed1->registerOptionalSubscription("fed0/pub3", "");
//
//	 helics::data_block db(547, ';');
//	 helics::subscription_id_t lastId;
//	 helics::Time lastTime;
//	 vFed1->registerSubscriptionNotificationCallback([&](helics::subscription_id_t subid, helics::Time callTime) {
//		 lastTime = callTime;
//		 lastId = subid;
//	 });
//	 vFed1->enterExecutionState();
//	 vFed1->publish(pubid3, db);
//	 vFed1->requestTime(1.0);
//	 // the callback should have occurred here
//	 BOOST_CHECK(lastId == sub3);
//	 BOOST_CHECK_EQUAL(lastTime, 1.0);
//	 BOOST_CHECK_EQUAL(vFed1->getLastUpdateTime(sub3), lastTime);
//	 vFed1->publish(pubid2, 4);
//	 vFed1->requestTime(2.0);
//	 // the callback should have occurred here
//	 BOOST_CHECK(lastId == sub2);
//	 BOOST_CHECK_EQUAL(lastTime, 2.0);
//	 vFed1->publish(pubid1, "this is a test");
//	 vFed1->requestTime(3.0);
//	 // the callback should have occurred here
//	 BOOST_CHECK(lastId == sub1);
//	 BOOST_CHECK_EQUAL(lastTime, 3.0);
//
//	 int ccnt = 0;
//	 vFed1->registerSubscriptionNotificationCallback([&](helics::subscription_id_t, helics::Time) { ++ccnt; });
//
//	 vFed1->publish(pubid3, db);
//	 vFed1->publish(pubid2, 4);
//	 vFed1->requestTime(4.0);
//	 // the callback should have occurred here
//	 BOOST_CHECK_EQUAL(ccnt, 2);
//	 ccnt = 0;  // reset the counter
//	 vFed1->publish(pubid3, db);
//	 vFed1->publish(pubid2, 4);
//	 vFed1->publish(pubid1, "test string2");
//	 vFed1->requestTime(5.0);
//	 // the callback should have occurred here
//	 BOOST_CHECK_EQUAL(ccnt, 3);
//	 vFed1->finalize();
// }
//
// /** test the callback specification with a vector list*/
//#if ENABLE_TEST_TIMEOUTS > 0
// BOOST_TEST_DECORATOR(*utf::timeout(5))
//#endif
//	 BOOST_DATA_TEST_CASE(test_vector_callback_lists, bdata::make(core_types_single), core_type)
// {
//	 SetupSingleBrokerTest<helics::ValueFederate>(core_type, 1, 1.0);
//	 auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
//
//	 auto pubid1 = vFed1->registerPublication<std::string>("pub1");
//	 auto pubid2 = vFed1->registerGlobalPublication<int>("pub2");
//
//	 auto pubid3 = vFed1->registerPublication("pub3", "");
//
//	 auto sub1 = vFed1->registerOptionalSubscription("fed0/pub1", "");
//	 auto sub2 = vFed1->registerOptionalSubscription("pub2", "");
//	 auto sub3 = vFed1->registerOptionalSubscription("fed0/pub3", "");
//
//	 helics::data_block db(547, ';');
//	 helics::subscription_id_t lastId;
//	 int ccnt = 0;
//	 // set subscriptions 1 and 2 to have callbacks
//	 vFed1->registerSubscriptionNotificationCallback({ sub1, sub2 },
//		 [&](helics::subscription_id_t, helics::Time) { ++ccnt; });
//	 vFed1->enterExecutionState();
//	 vFed1->publish(pubid3, db);
//	 vFed1->requestTime(1.0);
//	 // callbacks here
//	 BOOST_CHECK_EQUAL(ccnt, 0);
//
//	 vFed1->publish(pubid1, "this is a test");
//	 vFed1->requestTime(3.0);
//	 BOOST_CHECK_EQUAL(ccnt, 1);
//
//	 ccnt = 0;  // reset the counter
//	 vFed1->publish(pubid3, db);
//	 vFed1->publish(pubid2, 4);
//	 vFed1->publish(pubid1, "test string2");
//	 vFed1->requestTime(5.0);
//	 BOOST_CHECK_EQUAL(ccnt, 2);
//
//	 BOOST_CHECK_CLOSE(static_cast<double> (vFed1->getLastUpdateTime(sub3)), 3.0, 0.000001);
//	 vFed1->finalize();
// }
//
// /** test the publish/subscribe to a vectorized array*/
//#if ENABLE_TEST_TIMEOUTS > 0
// BOOST_TEST_DECORATOR(*utf::timeout(5))
//#endif
//	 BOOST_DATA_TEST_CASE(test_indexed_pubs_subs, bdata::make(core_types_single), core_type)
// {
//	 SetupSingleBrokerTest<helics::ValueFederate>(core_type, 1);
//	 auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
//
//	 auto pubid1 = vFed1->registerPublicationIndexed<double>("pub1", 0);
//	 auto pubid2 = vFed1->registerPublicationIndexed<double>("pub1", 1);
//
//	 auto pubid3 = vFed1->registerPublicationIndexed<double>("pub1", 2);
//
//	 auto sub1 = vFed1->registerOptionalSubscriptionIndexed<double>("pub1", 0);
//	 auto sub2 = vFed1->registerOptionalSubscriptionIndexed<double>("pub1", 1);
//	 auto sub3 = vFed1->registerOptionalSubscriptionIndexed<double>("pub1", 2);
//	 vFed1->enterExecutionState();
//
//	 vFed1->publish(pubid1, 10.0);
//	 vFed1->publish(pubid2, 20.0);
//	 vFed1->publish(pubid3, 30.0);
//	 vFed1->requestTime(2.0);
//	 auto v1 = vFed1->getValue<double>(sub1);
//	 auto v2 = vFed1->getValue<double>(sub2);
//	 auto v3 = vFed1->getValue<double>(sub3);
//
//	 BOOST_CHECK_CLOSE(10.0, v1, 0.00000001);
//	 BOOST_CHECK_CLOSE(20.0, v2, 0.00000001);
//	 BOOST_CHECK_CLOSE(30.0, v3, 0.00000001);
// }
//
// /** test the publish/subscribe to a vectorized array*/
//#if ENABLE_TEST_TIMEOUTS > 0
// BOOST_TEST_DECORATOR(*utf::timeout(5))
//#endif
BOOST_DATA_TEST_CASE(test_async_calls, bdata::make(core_types), core_type)
{
	helics_status status;
	helics_federate_info_t fi, fi1;
	helics_broker broker;
	helics_federate vFed1, vFed2;
	helics_publication pubid;
	helics_subscription subid;
	helics_time_t gtime;
	helics_time_t f1time;
	//federate_state state;
	char s[100] = "";
	broker = helicsCreateBroker(core_type.c_str(), "", "--federates=2");
	fi = helicsFederateInfoCreate();
	fi1 = helicsFederateInfoCreate();
	status = helicsFederateInfoSetFederateName(fi, "fed0");
	status = helicsFederateInfoSetFederateName(fi1, "fed1");
	status = helicsFederateInfoSetCoreTypeFromString(fi, core_type.c_str());
	//status = helicsFederateInfoSetTimeDelta(fi, 1.0);
	vFed1 = helicsCreateValueFederate(fi);
	vFed2 = helicsCreateValueFederate(fi1);

	// register the publications
	pubid = helicsFederateRegisterGlobalPublication(vFed1, "pub1", "string", "");
	subid = helicsFederateRegisterSubscription(vFed2, "pub1", "string", "");
	status = helicsFederateSetTimeDelta(vFed1, 1.0);
	status = helicsFederateSetTimeDelta(vFed2, 1.0);

	status = helicsFederateEnterExecutionModeAsync(vFed1);
	status = helicsFederateEnterExecutionModeAsync(vFed2);
	status = helicsFederateEnterExecutionModeComplete(vFed1);
	status = helicsFederateEnterExecutionModeComplete(vFed2);

	// publish string1 at time=0.0;
	status = helicsPublicationPublishString(pubid, "string1");

	status = helicsFederateRequestTimeAsync(vFed1, 1.0);
	status = helicsFederateRequestTimeComplete(vFed1, &f1time);
	status = helicsFederateRequestTimeAsync(vFed2, 1.0);
	status = helicsFederateRequestTimeComplete(vFed2, &gtime);

	BOOST_CHECK_EQUAL(f1time, 1.0);
	BOOST_CHECK_EQUAL(gtime, 1.0);

	// get the value
	status = helicsSubscriptionGetString(subid, s, 100);

	// make sure the string is what we expect
	BOOST_CHECK_EQUAL(s, "string1");

	// publish a second string
	status = helicsPublicationPublishString(pubid, "string2");

	// make sure the value is still what we expect
	status = helicsSubscriptionGetString(subid, s, 100);
	BOOST_CHECK_EQUAL(s, "string1");

	// advance time
	status = helicsFederateRequestTimeAsync(vFed1, 2.0);
	status = helicsFederateRequestTimeComplete(vFed1, &f1time);
	status = helicsFederateRequestTimeAsync(vFed2, 2.0);
	status = helicsFederateRequestTimeComplete(vFed2, &gtime);

	BOOST_CHECK_EQUAL(f1time, 2.0);
	BOOST_CHECK_EQUAL(gtime, 2.0);

	// make sure the value was updated
	status = helicsSubscriptionGetString(subid, s, 100);
	BOOST_CHECK_EQUAL(s, "string2");

	status = helicsFederateFinalize(vFed1);
	status = helicsFederateFinalize(vFed2);

	helicsFederateInfoFree(fi);
	helicsFederateInfoFree(fi1);
	helicsFederateFree(vFed1);
	helicsFederateFree(vFed2);
	helicsBrokerFree(broker);
	helicsCloseLibrary();

	std::cout << "test_async_calls" << " core_type " << core_type.c_str() << "\n";

	//	 SetupSingleBrokerTest<helics::ValueFederate>(core_type, 2);
	//	 auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
	//	 auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
	//
	//	 // register the publications
	//	 auto pubid = vFed1->registerGlobalPublication<std::string>("pub1");
	//
	//	 auto subid = vFed2->registerRequiredSubscription<std::string>("pub1");
	//	 vFed1->setTimeDelta(1.0);
	//	 vFed2->setTimeDelta(1.0);
	//
	//	 vFed1->enterExecutionStateAsync();
	//	 BOOST_CHECK(!vFed1->isAsyncOperationCompleted());
	//	 vFed2->enterExecutionStateAsync();
	//	 vFed1->enterExecutionStateComplete();
	//	 vFed2->enterExecutionStateComplete();
	//	 // publish string1 at time=0.0;
	//	 vFed1->publish(pubid, "string1");
	//	 vFed1->requestTimeAsync(1.0);
	//	 vFed2->requestTimeAsync(1.0);
	//
	//	 auto f1time = vFed1->requestTimeComplete();
	//	 auto gtime = vFed2->requestTimeComplete();
	//
	//	 BOOST_CHECK_EQUAL(gtime, 1.0);
	//	 BOOST_CHECK_EQUAL(f1time, 1.0);
	//	 std::string s;
	//	 // get the value
	//	 vFed2->getValue(subid, s);
	//	 // make sure the string is what we expect
	//	 BOOST_CHECK_EQUAL(s, "string1");
	//	 // publish a second string
	//	 vFed1->publish(pubid, "string2");
	//	 // make sure the value is still what we expect
	//	 vFed2->getValue(subid, s);
	//
	//	 BOOST_CHECK_EQUAL(s, "string1");
	//	 // advance time
	//	 vFed1->requestTimeAsync(2.0);
	//	 vFed2->requestTimeAsync(2.0);
	//	 f1time = vFed1->requestTimeComplete();
	//	 gtime = vFed2->requestTimeComplete();
	//
	//	 BOOST_CHECK_EQUAL(gtime, 2.0);
	//	 BOOST_CHECK_EQUAL(f1time, 2.0);
	//
	//	 // make sure the value was updated
	//
	//	 vFed2->getValue(subid, s);
	//
	//	 BOOST_CHECK_EQUAL(s, "string2");
	//	 vFed1->finalize();
	//	 vFed2->finalize();
}
//

//
BOOST_AUTO_TEST_CASE(test_file_load)
{
	helics_status status;
	helics_federate_info_t fi;
	helics_federate vFed;
	char s[100] = "";
	fi = helicsFederateInfoCreate();
	// path of the json file is hardcoded for now
	vFed = helicsCreateValueFederateFromJson("C:/HELICS/HELICS-src/tests/helics/c_interface/test_files/example_value_fed.json");
	status = helicsFederateGetName(vFed, s, 200);
	BOOST_CHECK_EQUAL(s, "valueFed");
	BOOST_CHECK_EQUAL(helicsFederateGetSubscriptionCount(vFed), 2);
	BOOST_CHECK_EQUAL(helicsFederateGetPublicationCount(vFed), 2);
	//	 helics::ValueFederate vFed(std::string(TEST_DIR) + "/test_files/example_value_fed.json");
	//
	//	 BOOST_CHECK_EQUAL(vFed.getName(), "fedName");

	//	 BOOST_CHECK_EQUAL(vFed.getSubscriptionCount(), 2);
	//	 BOOST_CHECK_EQUAL(vFed.getPublicationCount(), 2);
}
BOOST_AUTO_TEST_SUITE_END()
