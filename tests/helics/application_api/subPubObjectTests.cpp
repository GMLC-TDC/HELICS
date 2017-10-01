/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <complex>
/** these test cases test out the value converters
*/
#include <boost/test/floating_point_comparison.hpp>

#include "helics/application_api/Subscriptions.hpp"
#include "helics/application_api/Publications.hpp"
#include "test_configuration.h"
#include <future>


BOOST_AUTO_TEST_SUITE(subPubObject_tests)

BOOST_AUTO_TEST_CASE(subscriptionTObject_tests)
{
	helics::FederateInfo fi("test1");
	fi.coreType = CORE_TYPE_TO_TEST;
	fi.coreInitString = "1";

	auto vFed = std::make_shared<helics::ValueFederate>(fi);
	//register the publications
	auto pubObj = helics::PublicationT<std::string>(helics::GLOBAL, vFed.get(), "pub1");

	auto subObj = helics::SubscriptionT<std::string>(vFed.get(),"pub1");
	vFed->setTimeDelta(1.0);
	vFed->enterExecutionState();
	//publish string1 at time=0.0;
	pubObj.publish("string1");
	auto gtime = vFed->requestTime(1.0);

	BOOST_CHECK_EQUAL(gtime,1.0);
	std::string s = subObj.getValue();
	
	//make sure the string is what we expect
	BOOST_CHECK_EQUAL(s, "string1");
	//publish a second string
	pubObj.publish("string2");
	//make sure the value is still what we expect
	subObj.getValue( s);

	BOOST_CHECK_EQUAL(s, "string1");
	//advance time 
	gtime = vFed->requestTime(2.0);
	//make sure the value was updated
	BOOST_CHECK_EQUAL(gtime, 2.0);
	subObj.getValue(s);

	BOOST_CHECK_EQUAL(s, "string2");
	vFed->finalize();

}

BOOST_AUTO_TEST_CASE(subscriptionObject_tests)
{
	helics::FederateInfo fi("test1");
	fi.coreType = CORE_TYPE_TO_TEST;
	fi.coreInitString = "1";

	auto vFed = std::make_shared<helics::ValueFederate>(fi);
	//register the publications
	auto pubObj = helics::make_publication<std::string>(helics::GLOBAL, vFed.get(), std::string("pub1"));

	auto subObj = helics::Subscription(vFed.get(), "pub1");
	vFed->setTimeDelta(1.0);
	vFed->enterExecutionState();
	//publish string1 at time=0.0;
	pubObj->publish("string1");
	auto gtime = vFed->requestTime(1.0);

	BOOST_CHECK_EQUAL(gtime, 1.0);
	std::string s = subObj.getValue<std::string>();
	//long long val = subObj.getValue<long long>();
	//make sure the string is what we expect
	BOOST_CHECK_EQUAL(s, "string1");
	//publish a second string
	pubObj->publish("string2");
	//make sure the value is still what we expect
	subObj.getValue<std::string>(s);

	BOOST_CHECK_EQUAL(s, "string1");
	//advance time 
	gtime = vFed->requestTime(2.0);
	//make sure the value was updated
	BOOST_CHECK_EQUAL(gtime, 2.0);
	subObj.getValue(s);

	BOOST_CHECK_EQUAL(s, "string2");
	vFed->finalize();

}

BOOST_AUTO_TEST_SUITE_END()
