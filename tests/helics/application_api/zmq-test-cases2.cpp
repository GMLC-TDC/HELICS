/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>

#include "helics/application_api/ValueFederate.h"
#include "test_configuration.h"
#include "zmqBrokerRunner.h"
#include <future>
/** these test cases test out the value federates
 */

BOOST_AUTO_TEST_SUITE (zmq_value_federate_tests2)

static const BrokerRunner zmqRunner(HELICSINSTALL_LOCATION, HELICS_ZMQ_BROKER_LOCATION, "helics_broker");
#define ZMQ_CORE_TYPE "zmq"


BOOST_AUTO_TEST_CASE (value_federate_dual_transfer)
{
	//launch the broker
	auto fut = std::async(std::launch::async, [&]() {return zmqRunner.run("1"); });
    helics::FederateInfo_app fi ("test1");
    fi.coreType = ZMQ_CORE_TYPE;
    fi.coreInitString = "1";

    auto vFed1 = std::make_shared<helics::ValueFederate> (fi);
    fi.name = "test2";
    auto vFed2 = std::make_shared<helics::ValueFederate> (fi);
    // register the publications
    auto pubid = vFed1->registerGlobalPublication<std::string> ("pub1");

    auto subid = vFed2->registerRequiredSubscription<std::string> ("pub1");
    vFed1->setTimeDelta (1.0);
    vFed2->setTimeDelta (1.0);

    auto f1finish = std::async (std::launch::async, [&]() { vFed1->enterExecutionState (); });
    vFed2->enterExecutionState ();
    f1finish.wait ();
    // publish string1 at time=0.0;
    vFed1->publish (pubid, "string1");
    auto f1time = std::async (std::launch::async, [&]() { return vFed1->requestTime (1.0); });
    auto gtime = vFed2->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK_EQUAL (f1time.get (), 1.0);
    std::string s;
    // get the value
    vFed2->getValue (subid, s);
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (s, "string1");
    // publish a second string
    vFed1->publish (pubid, "string2");
    // make sure the value is still what we expect
    vFed2->getValue (subid, s);

    BOOST_CHECK_EQUAL (s, "string1");
    // advance time
    f1time = std::async (std::launch::async, [&]() { return vFed1->requestTime (2.0); });
    gtime = vFed2->requestTime (2.0);

    BOOST_CHECK_EQUAL (gtime, 2.0);
    BOOST_CHECK_EQUAL (f1time.get (), 2.0);
    // make sure the value was updated

    vFed2->getValue (subid, s);

    BOOST_CHECK_EQUAL (s, "string2");
    vFed1->finalize ();
    vFed2->finalize ();
	auto val = fut.get();
	BOOST_CHECK_EQUAL(val, 0);
}


template <class X>
void runDualFederateTest (const X &defaultValue, const X &testValue1, const X &testValue2)
{
	//launch the broker
	auto fut = std::async(std::launch::async, [&]() {return zmqRunner.run("1"); });
    helics::FederateInfo_app fi ("test1");
    fi.coreType = ZMQ_CORE_TYPE;
    fi.coreInitString = "1";

    auto vFed1 = std::make_shared<helics::ValueFederate> (fi);
    fi.name = "test2";
    auto vFed2 = std::make_shared<helics::ValueFederate> (fi);
    // register the publications
    auto pubid = vFed1->registerGlobalPublication<X> ("pub1");

    auto subid = vFed2->registerRequiredSubscription<X> ("pub1");
    vFed1->setTimeDelta (1.0);
    vFed2->setTimeDelta (1.0);

    vFed2->setDefaultValue<X> (subid, defaultValue);

    auto f1finish = std::async (std::launch::async, [&]() { vFed1->enterExecutionState (); });
    vFed2->enterExecutionState ();
    f1finish.wait ();
    // publish string1 at time=0.0;
    vFed1->publish<X> (pubid, testValue1);

    X val;
    vFed2->getValue<X> (subid, val);

    BOOST_CHECK_EQUAL (val, defaultValue);

    auto f1time = std::async (std::launch::async, [&]() { return vFed1->requestTime (1.0); });
    auto gtime = vFed2->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK_EQUAL (f1time.get (), 1.0);
    // get the value
    vFed2->getValue (subid, val);
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (val, testValue1);

    // publish a second string
    vFed1->publish (pubid, testValue2);
    // make sure the value is still what we expect
    vFed2->getValue (subid, val);

    BOOST_CHECK_EQUAL (val, testValue1);

    // advance time
    f1time = std::async (std::launch::async, [&]() { return vFed1->requestTime (2.0); });
    gtime = vFed2->requestTime (2.0);

    BOOST_CHECK_EQUAL (gtime, 2.0);
    BOOST_CHECK_EQUAL (f1time.get (), 2.0);

    // make sure the value was updated
    vFed2->getValue (subid, val);
    BOOST_CHECK_EQUAL (val, testValue2);

    vFed1->finalize ();
    vFed2->finalize ();
	auto finalval = fut.get();
	BOOST_CHECK_EQUAL(finalval, 0);
}
template <class X>
void runDualFederateTestv2 (const X &defaultValue, const X &testValue1, const X &testValue2)
{
	//launch the broker
	auto fut = std::async(std::launch::async, [&]() {return zmqRunner.run("1"); });
    helics::FederateInfo_app fi ("test1");
    fi.coreType = ZMQ_CORE_TYPE;
    fi.coreInitString = "1";

    auto vFed1 = std::make_shared<helics::ValueFederate> (fi);
    fi.name = "test2";
    auto vFed2 = std::make_shared<helics::ValueFederate> (fi);
    // register the publications
    auto pubid = vFed1->registerGlobalPublication<X> ("pub1");

    auto subid = vFed2->registerRequiredSubscription<X> ("pub1");
    vFed1->setTimeDelta (1.0);
    vFed2->setTimeDelta (1.0);

    vFed2->setDefaultValue<X> (subid, defaultValue);

    auto f1finish = std::async (std::launch::async, [&]() { vFed1->enterExecutionState (); });
    vFed2->enterExecutionState ();
    f1finish.wait ();
    // publish string1 at time=0.0;
    vFed1->publish<X> (pubid, testValue1);

    X val;
    vFed2->getValue<X> (subid, val);
    BOOST_CHECK (val == defaultValue);
    auto f1time = std::async (std::launch::async, [&]() { return vFed1->requestTime (1.0); });
    auto gtime = vFed2->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK_EQUAL (f1time.get (), 1.0);
    // get the value
    vFed2->getValue (subid, val);
    // make sure the string is what we expect
    BOOST_CHECK (val == testValue1);
    // publish a second string
    vFed1->publish (pubid, testValue2);
    // make sure the value is still what we expect
    vFed2->getValue (subid, val);
    BOOST_CHECK (val == testValue1);
    // advance time
    f1time = std::async (std::launch::async, [&]() { return vFed1->requestTime (2.0); });
    gtime = vFed2->requestTime (2.0);

    BOOST_CHECK_EQUAL (gtime, 2.0);
    BOOST_CHECK_EQUAL (f1time.get (), 2.0);

    // make sure the value was updated
    vFed2->getValue (subid, val);
    BOOST_CHECK (val == testValue2);
    vFed1->finalize ();
    vFed2->finalize ();
	auto finalval = fut.get();
	BOOST_CHECK_EQUAL(finalval, 0);
}

/** test case checking that the transfer between two federates works as expected
 */
BOOST_AUTO_TEST_CASE (value_federate_dual_transfer_types)
{
    runDualFederateTest<double> (10.3, 45.3, 22.7);
    runDualFederateTest<int> (5, 8, 43);
    runDualFederateTest<int> (-5, 1241515, -43);
    runDualFederateTest<char> ('c', '\0', '\n');

    runDualFederateTest<uint64_t> (234252315, 0xFFF1'2345'7124'1412, 23521513412);
    runDualFederateTest<float> (10.3f, 45.3f, 22.7f);
    runDualFederateTest<std::string> ("start", "inside of the functional relationship of helics",
                                      std::string ("I am a string"));
    // this one is going to test really ugly strings
    runDualFederateTest<std::string> (std::string (862634, '\0'),
                                      "inside\n\0 of the \0\n functional\r \brelationship of helics\n",
                                      std::string (""));
    runDualFederateTestv2<std::vector<double>> ({34.3, 24.2}, {12.4, 14.7, 16.34, 18.17},
                                                {9.9999, 8.8888, 7.7777});
    std::complex<double> def = {54.23233, 0.7};
    std::complex<double> v1 = std::polar (10.0, 0.43);
    std::complex<double> v2 = {-3e45, 1e-23};
    runDualFederateTest<std::complex<double>> (def, v1, v2);

}

BOOST_AUTO_TEST_CASE(value_federate_single_init_publish)
{
	//launch the broker
	auto fut = std::async(std::launch::async, [&]() {return zmqRunner.run("1"); });
	helics::FederateInfo_app fi("test1");
	fi.coreType = ZMQ_CORE_TYPE;
	fi.coreInitString = "1";

	auto vFed = std::make_shared<helics::ValueFederate>(fi);
	// register the publications
	auto pubid = vFed->registerGlobalPublication<double>("pub1");

	auto subid = vFed->registerRequiredSubscription<double>("pub1");
	vFed->setTimeDelta(1.0);
	vFed->enterInitializationState();
	vFed->publish(pubid, 1.0);

	vFed->enterExecutionState();
	// get the value set at initialization
	double val;
	vFed->getValue(subid, val);
	BOOST_CHECK_EQUAL(val, 1.0);
	// publish string1 at time=0.0;
	vFed->publish(pubid, 2.0);
	auto gtime = vFed->requestTime(1.0);
	
	BOOST_CHECK_EQUAL(gtime, 1.0);
	
	// get the value
	vFed->getValue(subid, val);
	// make sure the string is what we expect
	BOOST_CHECK_EQUAL(val, 2.0);
	// publish a second string
	vFed->publish(pubid, 3.0);
	// make sure the value is still what we expect
	vFed->getValue(subid, val);

	BOOST_CHECK_EQUAL(val, 2.0);
	// advance time
	gtime = vFed->requestTime(2.0);
	// make sure the value was updated
	BOOST_CHECK_EQUAL(gtime, 2.0);
	vFed->getValue(subid, val);

	BOOST_CHECK_EQUAL(val, 3.0);
	vFed->finalize();

	auto finalval = fut.get();
	BOOST_CHECK_EQUAL(finalval, 0);
}
BOOST_AUTO_TEST_SUITE_END ()