/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include "testFixtures.h"
#include <complex>

/** these test cases test out the value converters
*/
#include "helics/application_api/ValueConverter.hpp"

BOOST_FIXTURE_TEST_SUITE(iteration_tests, ValueFederateTestFixture)

/** just a check that in the simple case we do actually get the time back we requested*/
BOOST_AUTO_TEST_CASE(execution_iteration_test)
{
	Setup1FederateTest("test");
	// register the publications
	auto pubid = vFed1->registerGlobalPublication<double>("pub1");

	auto subid = vFed1->registerRequiredSubscription<double>("pub1");
	vFed1->setTimeDelta(1.0);
	vFed1->enterInitializationState();
	vFed1->publish(pubid, 27.0);

	auto comp=vFed1->enterExecutionState(helics::convergence_state::nonconverged);

	BOOST_CHECK(comp == helics::convergence_state::nonconverged);
	auto val = vFed1->getValue<double>(subid);
	BOOST_CHECK_EQUAL(val, 27.0);

	comp = vFed1->enterExecutionState(helics::convergence_state::nonconverged);

	BOOST_CHECK(comp == helics::convergence_state::complete);
	
	double val2=vFed1->getValue<double>(subid);

	BOOST_CHECK_EQUAL(val2, val);
}

BOOST_AUTO_TEST_CASE(execution_iteration_test_2fed)
{
	Setup2FederateTest("test");
	// register the publications
	auto pubid = vFed1->registerGlobalPublication<double>("pub1");

	auto subid = vFed2->registerRequiredSubscription<double>("pub1");
	vFed1->setTimeDelta(1.0);
	vFed2->setTimeDelta(1.0);

	vFed1->enterInitializationStateAsync();
	vFed2->enterInitializationState();
	vFed1->enterInitializationStateFinalize();
	vFed1->publish(pubid, 27.0);
	vFed1->enterExecutionStateAsync();
	auto comp = vFed2->enterExecutionState(helics::convergence_state::nonconverged);

	BOOST_CHECK(comp == helics::convergence_state::nonconverged);
	auto val = vFed2->getValue<double>(subid);
	BOOST_CHECK_EQUAL(val, 27.0);

	comp = vFed2->enterExecutionState(helics::convergence_state::nonconverged);

	BOOST_CHECK(comp == helics::convergence_state::complete);

	double val2 = vFed2->getValue<double>(subid);
	vFed1->enterExecutionStateFinalize();
	BOOST_CHECK_EQUAL(val2, val);
}

/** just a check that in the simple case we do actually get the time back we requested*/
BOOST_AUTO_TEST_CASE(time_iteration_test)
{
	Setup1FederateTest("test");
	// register the publications
	auto pubid = vFed1->registerGlobalPublication<double>("pub1");

	auto subid = vFed1->registerRequiredSubscription<double>("pub1");
	vFed1->setPeriod(1.0);
	vFed1->setTimeDelta(1.0);
	vFed1->enterExecutionState();
	vFed1->publish(pubid, 27.0);

	auto comp = vFed1->requestTimeIterative(1.0, helics::convergence_state::nonconverged);

	BOOST_CHECK(comp.state == helics::convergence_state::nonconverged);
	BOOST_CHECK_EQUAL(comp.stepTime,helics::timeZero);
	auto val = vFed1->getValue<double>(subid);
	BOOST_CHECK_EQUAL(val, 27.0);

	comp = vFed1->requestTimeIterative(1.0, helics::convergence_state::nonconverged);

	BOOST_CHECK(comp.state == helics::convergence_state::complete);
	BOOST_CHECK_EQUAL(comp.stepTime,1.0);
	double val2 = vFed1->getValue<double>(subid);

	BOOST_CHECK_EQUAL(val2, val);
}

BOOST_AUTO_TEST_CASE(time_iteration_test_2fed)
{
	Setup2FederateTest("test");
	// register the publications
	auto pubid = vFed1->registerGlobalPublication<double>("pub1");

	auto subid = vFed2->registerRequiredSubscription<double>("pub1");
	vFed1->setTimeDelta(1.0);
	vFed2->setTimeDelta(1.0);
	vFed1->setPeriod(1.0);
	vFed2->setPeriod(1.0);

	vFed1->enterExecutionStateAsync();
	vFed2->enterExecutionState();
	vFed1->enterExecutionStateFinalize();
	vFed1->publish(pubid, 27.0);
	
	vFed1->requestTimeAsync(1.0);
	auto comp = vFed2->requestTimeIterative(1.0, helics::convergence_state::nonconverged);

	BOOST_CHECK(comp.state == helics::convergence_state::nonconverged);
	BOOST_CHECK_EQUAL(comp.stepTime, helics::timeZero);
	auto val = vFed2->getValue<double>(subid);
	BOOST_CHECK_EQUAL(val, 27.0);
	

	comp = vFed2->requestTimeIterative(1.0, helics::convergence_state::nonconverged);

	BOOST_CHECK(comp.state == helics::convergence_state::complete);
	BOOST_CHECK_EQUAL(comp.stepTime, 1.0);
	double val2 = vFed2->getValue<double>(subid);
	vFed1->requestTimeFinalize();

	BOOST_CHECK_EQUAL(val2, val);
}
BOOST_AUTO_TEST_SUITE_END()