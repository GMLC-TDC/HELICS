/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <complex>

/** these test cases test out the value converters
 */
#include "helics/application_api/ValueConverter.hpp"
#include "testFixtures.h"
#include "test_configuration.h"
#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"


BOOST_FIXTURE_TEST_SUITE (timing_tests, FederateTestFixture)

/** just a check that in the simple case we do actually get the time back we requested*/
BOOST_AUTO_TEST_CASE (simple_timing_test) 
{
	SetupSingleBrokerTest<helics::ValueFederate>("test", 2);
	auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
	auto vFed2 = GetFederateAs<helics::ValueFederate>(1);


	vFed1->setPeriod(0.5);
	vFed2->setPeriod(0.5);

	auto pub = helics::make_publication<double>(helics::GLOBAL, vFed1.get(), "pub1");
	auto sub = helics::Subscription(vFed2.get(), "pub1");
	vFed1->enterExecutionStateAsync();
	vFed2->enterExecutionState();
	vFed1->enterExecutionStateFinalize();
	pub->publish(0.27);
	auto res=vFed1->requestTime(2.0);
	BOOST_CHECK_EQUAL(res, 2.0);

	res = vFed2->requestTime(2.0);
	BOOST_CHECK_EQUAL(res, 0.5); //the result should show up at the next available time point
	res = vFed2->requestTime(2.0);
	BOOST_CHECK_EQUAL(res, 2.0);

	vFed1->finalize();
	vFed2->finalize();

}

BOOST_AUTO_TEST_CASE(simple_timing_test2)
{
	SetupSingleBrokerTest<helics::ValueFederate>("test", 2);
	auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
	auto vFed2 = GetFederateAs<helics::ValueFederate>(1);


	vFed1->setPeriod(0.5);
	vFed2->setPeriod(0.5);

	auto pub = helics::make_publication<double>(helics::GLOBAL, vFed1.get(), "pub1");
	auto sub = helics::Subscription(vFed2.get(), "pub1");
	vFed1->enterExecutionStateAsync();
	vFed2->enterExecutionState();
	vFed1->enterExecutionStateFinalize();

	auto res = vFed1->requestTime(0.32);
	//check that the request is only granted at the appropriate period
	BOOST_CHECK_EQUAL(res, 0.5); 
	pub->publish(0.27);
	res = vFed1->requestTime(1.85);
	BOOST_CHECK_EQUAL(res, 2.0);
	res = vFed2->requestTime(1.79);
	BOOST_CHECK_EQUAL(res, 0.5); //the result should show up at the next available time point
	res = vFed2->requestTime(2.0);
	BOOST_CHECK_EQUAL(res, 2.0);

	vFed1->finalize();
	vFed2->finalize();

}

BOOST_AUTO_TEST_SUITE_END ()