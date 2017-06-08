/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>

#include "helics/application_api/ValueFederate.h"
#include "helics/application_api/Message.h"
#include "test_configuration.h"
#include <future>
/** these test cases test out the value converters and some of the other functions
*/

BOOST_AUTO_TEST_SUITE(value_federate_tests2)

/** test block send and receive*/
BOOST_AUTO_TEST_CASE(test_block_send_receive)
{
	helics::FederateInfo_app fi("test1");
	fi.coreType = CORE_TYPE_TO_TEST;
	fi.coreInitString = "1";

	auto vFed = std::make_shared<helics::ValueFederate>(fi);

	vFed->registerPublication<std::string>("pub1");
	vFed->registerGlobalPublication<int>("pub2");

	auto pubid3 = vFed->registerPublication("pub3", "");

	auto sub1 = vFed->registerOptionalSubscription("test1/pub3","");

	helics::data_block db(547, ';');

	vFed->enterExecutionState();
	vFed->publish(pubid3, db);
	vFed->requestTime(1.0);
	BOOST_CHECK(vFed->isUpdated(sub1));
	auto res = vFed->getValueRaw(sub1);
	BOOST_CHECK_EQUAL(res.size(), db.size());
	BOOST_CHECK(vFed->isUpdated(sub1)==false);

}

/** test the all callback*/
BOOST_AUTO_TEST_CASE(test_all_callback)
{
	helics::FederateInfo_app fi("test1");
	fi.coreType = CORE_TYPE_TO_TEST;
	fi.coreInitString = "1";

	auto vFed = std::make_shared<helics::ValueFederate>(fi);

	auto pubid1 = vFed->registerPublication<std::string>("pub1");
	auto pubid2 = vFed->registerGlobalPublication<int>("pub2");

	auto pubid3 = vFed->registerPublication("pub3", "");

	auto sub1 = vFed->registerOptionalSubscription("test1/pub1", "");
	auto sub2 = vFed->registerOptionalSubscription("pub2", "");
	auto sub3 = vFed->registerOptionalSubscription("test1/pub3", "");

	helics::data_block db(547, ';');
	helics::subscription_id_t lastId;
	helics::Time lastTime;
	vFed->registerSubscriptionNotificationCallback([&](helics::subscription_id_t subid, helics::Time callTime) {lastTime = callTime; lastId = subid; });
	vFed->enterExecutionState();
	vFed->publish(pubid3, db);
	vFed->requestTime(1.0);
	//the callback should have occured here
	BOOST_CHECK(lastId == sub3);
	BOOST_CHECK_EQUAL(lastTime, 1.0);
	BOOST_CHECK_EQUAL(vFed->getLastUpdateTime(sub3), lastTime);
	vFed->publish(pubid2, 4);
	vFed->requestTime(2.0);
	//the callback should have occured here
	BOOST_CHECK(lastId == sub2);
	BOOST_CHECK_EQUAL(lastTime, 2.0);
	vFed->publish(pubid1, "this is a test");
	vFed->requestTime(3.0);
	//the callback should have occured here
	BOOST_CHECK(lastId == sub1);
	BOOST_CHECK_EQUAL(lastTime, 3.0);

	int ccnt = 0;
	vFed->registerSubscriptionNotificationCallback([&](helics::subscription_id_t , helics::Time ) {++ccnt; });

	vFed->publish(pubid3, db);
	vFed->publish(pubid2, 4);
	vFed->requestTime(4.0);
	//the callback should have occured here
	BOOST_CHECK_EQUAL(ccnt,2);
	ccnt = 0; //reset the counter
	vFed->publish(pubid3, db);
	vFed->publish(pubid2, 4);
	vFed->publish(pubid1, "test string2");
	vFed->requestTime(5.0);
	//the callback should have occured here
	BOOST_CHECK_EQUAL(ccnt, 3);
	vFed->finalize();
}


/** test the callback specification with a vector list*/
BOOST_AUTO_TEST_CASE(test_vector_callback_lists)
{
	helics::FederateInfo_app fi("test1");
	fi.coreType = CORE_TYPE_TO_TEST;
	fi.coreInitString = "1";

	auto vFed = std::make_shared<helics::ValueFederate>(fi);

	auto pubid1 = vFed->registerPublication<std::string>("pub1");
	auto pubid2 = vFed->registerGlobalPublication<int>("pub2");

	auto pubid3 = vFed->registerPublication("pub3", "");

	auto sub1 = vFed->registerOptionalSubscription("test1/pub1", "");
	auto sub2 = vFed->registerOptionalSubscription("pub2", "");
	auto sub3 = vFed->registerOptionalSubscription("test1/pub3", "");

	helics::data_block db(547, ';');
	helics::subscription_id_t lastId;
	int ccnt = 0;
	//set subscriptions 1 and 2 to have callbacks
	vFed->registerSubscriptionNotificationCallback({ sub1,sub2 },[&](helics::subscription_id_t, helics::Time ) {++ccnt; });
	vFed->enterExecutionState();
	vFed->publish(pubid3, db);
	vFed->requestTime(1.0);
	//callbacks here
	BOOST_CHECK_EQUAL(ccnt, 0);
	
	vFed->publish(pubid1, "this is a test");
	vFed->requestTime(3.0);
	BOOST_CHECK_EQUAL(ccnt, 1);

	ccnt = 0; //reset the counter
	vFed->publish(pubid3, db);
	vFed->publish(pubid2, 4);
	vFed->publish(pubid1, "test string2");
	vFed->requestTime(5.0);
	BOOST_CHECK_EQUAL(ccnt, 2);

	BOOST_CHECK_CLOSE(static_cast<double>(vFed->getLastUpdateTime(sub3)), 3.0,0.000001);
	vFed->finalize();
}


/** test the publish/subscribe to a vectorized array*/
BOOST_AUTO_TEST_CASE(test_indexed_pubs_subs)
{
	helics::FederateInfo_app fi("test1");
	fi.coreType = CORE_TYPE_TO_TEST;
	fi.coreInitString = "1";

	auto vFed = std::make_shared<helics::ValueFederate>(fi);

	auto pubid1 = vFed->registerPublication<double>("pub1",0);
	auto pubid2 = vFed->registerPublication<double>("pub1",1);

	auto pubid3 = vFed->registerPublication<double>("pub1", 2);

	auto sub1 = vFed->registerOptionalSubscription<double>("pub1", 0);
	auto sub2 = vFed->registerOptionalSubscription<double>("pub1", 1);
	auto sub3 = vFed->registerOptionalSubscription<double>("pub1", 2);
	vFed->enterExecutionState();

	vFed->publish(pubid1, 10.0);
	vFed->publish(pubid2, 20.0);
	vFed->publish(pubid3, 30.0);
	vFed->requestTime(2.0);
	auto v1 = vFed->getValue<double>(sub1);
	auto v2 = vFed->getValue<double>(sub2);
	auto v3 = vFed->getValue<double>(sub3);

	BOOST_CHECK_CLOSE(10.0, v1, 0.00000001);
	BOOST_CHECK_CLOSE(20.0, v2, 0.00000001);
	BOOST_CHECK_CLOSE(30.0, v3, 0.00000001);
	vFed->finalize();
}

/** test the publish/subscribe to a vectorized array*/
BOOST_AUTO_TEST_CASE(test_async_calls)
{
	helics::FederateInfo_app fi("test1");
	fi.coreType = CORE_TYPE_TO_TEST;
	fi.coreInitString = "2";

	auto vFed1 = std::make_shared<helics::ValueFederate>(fi);
	fi.name = "test2";
	auto vFed2 = std::make_shared<helics::ValueFederate>(fi);
	// register the publications
	auto pubid = vFed1->registerGlobalPublication<std::string>("pub1");

	auto subid = vFed2->registerRequiredSubscription<std::string>("pub1");
	vFed1->setTimeDelta(1.0);
	vFed2->setTimeDelta(1.0);

	vFed1->enterExecutionStateAsync();
	BOOST_CHECK(!vFed1->asyncOperationCompleted());
	vFed2->enterExecutionStateAsync();
	vFed1->enterExecutionStateFinalize();
	vFed2->enterExecutionStateFinalize();
	// publish string1 at time=0.0;
	vFed1->publish(pubid, "string1");
	vFed1->requestTimeAsync(1.0);
	vFed2->requestTimeAsync(1.0);

	auto f1time = vFed1->requestTimeFinalize();
	auto gtime = vFed2->requestTimeFinalize();

	BOOST_CHECK_EQUAL(gtime, 1.0);
	BOOST_CHECK_EQUAL(f1time, 1.0);
	std::string s;
	// get the value
	vFed2->getValue(subid, s);
	// make sure the string is what we expect
	BOOST_CHECK_EQUAL(s, "string1");
	// publish a second string
	vFed1->publish(pubid, "string2");
	// make sure the value is still what we expect
	vFed2->getValue(subid, s);

	BOOST_CHECK_EQUAL(s, "string1");
	// advance time
	vFed1->requestTimeAsync(2.0);
	vFed2->requestTimeAsync(2.0);
	f1time = vFed1->requestTimeFinalize();
	gtime = vFed2->requestTimeFinalize();

	BOOST_CHECK_EQUAL(gtime, 2.0);
	BOOST_CHECK_EQUAL(f1time, 2.0);

	// make sure the value was updated

	vFed2->getValue(subid, s);

	BOOST_CHECK_EQUAL(s, "string2");
	vFed1->finalize();
	vFed2->finalize();
}

/** test the default constructor and move constructor and move assignment*/
BOOST_AUTO_TEST_CASE(test_move_calls)
{
	helics::ValueFederate vFed;
	
	helics::FederateInfo_app fi("test1", CORE_TYPE_TO_TEST);
	fi.coreInitString = "3";
	vFed = helics::ValueFederate(fi);
	BOOST_CHECK(vFed.getName() == "test1");

	helics::ValueFederate vFed2(std::move(vFed));
	BOOST_CHECK(vFed2.getName() == "test1");
	BOOST_CHECK(vFed.getName().empty());
}


BOOST_AUTO_TEST_SUITE_END()