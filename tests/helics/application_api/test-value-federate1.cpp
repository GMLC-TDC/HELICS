/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include <future>

#include "helics/application_api/ValueFederate.h"
#include "test_configuration.h"
#include "testFixtures.h"
#include "helics/core/CoreFactory.h"
#include "helics/core/BrokerFactory.h"


/** these test cases test out the value federates
 */

BOOST_FIXTURE_TEST_SUITE (value_federate_tests, ValueFederateTestFixture)

namespace bdata = boost::unit_test::data;
const std::string core_types[] = { "test" };


/** test simple creation and destruction*/
BOOST_DATA_TEST_CASE(value_federate_initialize_tests, bdata::make(core_types), core_type)
{
    Setup1FederateTest(core_type);

    vFed1->enterExecutionState ();

    BOOST_CHECK (vFed1->currentState () == helics::Federate::op_states::execution);

    vFed1->finalize ();

    BOOST_CHECK (vFed1->currentState () == helics::Federate::op_states::finalize);
}


BOOST_DATA_TEST_CASE(value_federate_publication_registration, bdata::make(core_types), core_type)
{
    Setup1FederateTest(core_type);

    auto pubid = vFed1->registerPublication<std::string> ("pub1");
    auto pubid2 = vFed1->registerGlobalPublication<int> ("pub2");

    auto pubid3 = vFed1->registerPublication ("pub3", "double", "V");
    vFed1->enterExecutionState ();

    BOOST_CHECK (vFed1->currentState () == helics::Federate::op_states::execution);

    auto sv = vFed1->getPublicationName (pubid);
    auto sv2 = vFed1->getPublicationName (pubid2);
    BOOST_CHECK_EQUAL (sv, "test1/pub1");
    BOOST_CHECK_EQUAL (sv2, "pub2");
    auto pub3name = vFed1->getPublicationName (pubid3);
    BOOST_CHECK_EQUAL (pub3name, "test1/pub3");

    BOOST_CHECK_EQUAL (vFed1->getPublicationType (pubid3), "double");
    BOOST_CHECK_EQUAL (vFed1->getPublicationUnits (pubid3), "V");

	BOOST_CHECK(vFed1->getPublicationId("pub1") == pubid);
	BOOST_CHECK(vFed1->getPublicationId("pub2") == pubid2);
	BOOST_CHECK(vFed1->getPublicationId("test1/pub1") == pubid);
    vFed1->finalize ();

    BOOST_CHECK (vFed1->currentState () == helics::Federate::op_states::finalize);
}


BOOST_DATA_TEST_CASE(value_federate_subscription_registration, bdata::make(core_types), core_type)
{
    Setup1FederateTest(core_type);

    auto subid = vFed1->registerRequiredSubscription ("sub1", "double", "V");
    auto subid2 = vFed1->registerRequiredSubscription<int> ("sub2");

    auto subid3 = vFed1->registerOptionalSubscription ("sub3", "double", "V");
    vFed1->enterExecutionState ();

   // BOOST_CHECK (vFed->currentState () == helics::Federate::op_states::execution);

    auto sv = vFed1->getSubscriptionName (subid);
    auto sv2 = vFed1->getSubscriptionName (subid2);
    BOOST_CHECK_EQUAL (sv, "sub1");
    BOOST_CHECK_EQUAL (sv2, "sub2");
    auto sub3name = vFed1->getSubscriptionName (subid3);

	vFed1->addSubscriptionShortcut(subid, "Shortcut");
    BOOST_CHECK_EQUAL (sub3name, "sub3");

    BOOST_CHECK_EQUAL (vFed1->getSubscriptionType (subid3), "double");
    BOOST_CHECK_EQUAL (vFed1->getSubscriptionUnits (subid3), "V");

	BOOST_CHECK(vFed1->getSubscriptionId("sub1") == subid);
	BOOST_CHECK(vFed1->getSubscriptionId("sub2") == subid2);

	BOOST_CHECK(vFed1->getSubscriptionId("Shortcut") == subid);
	
    vFed1->finalize ();

    BOOST_CHECK (vFed1->currentState () == helics::Federate::op_states::finalize);
}


BOOST_DATA_TEST_CASE(value_federate_subscription_and_publication_registration, bdata::make(core_types), core_type)
{
    Setup1FederateTest(core_type);

    // register the publications
    auto pubid = vFed1->registerPublication<std::string> ("pub1");
    auto pubid2 = vFed1->registerGlobalPublication<int> ("pub2");

    auto pubid3 = vFed1->registerPublication ("pub3", "double", "V");


    auto subid = vFed1->registerOptionalSubscription ("sub1", "double", "V");
    auto subid2 = vFed1->registerOptionalSubscription<int> ("sub2");

    auto subid3 = vFed1->registerOptionalSubscription ("sub3", "double", "V");
    // enter execution
    vFed1->enterExecutionState ();

    BOOST_CHECK (vFed1->currentState () == helics::Federate::op_states::execution);
    // check subscriptions
    auto sv = vFed1->getSubscriptionName (subid);
    auto sv2 = vFed1->getSubscriptionName (subid2);
    BOOST_CHECK_EQUAL (sv, "sub1");
    BOOST_CHECK_EQUAL (sv2, "sub2");
    auto sub3name = vFed1->getSubscriptionName (subid3);
    BOOST_CHECK_EQUAL (sub3name, "sub3");

    BOOST_CHECK_EQUAL (vFed1->getSubscriptionType (subid3), "double");
    BOOST_CHECK_EQUAL (vFed1->getSubscriptionUnits (subid3), "V");


    // check publications

    sv = vFed1->getPublicationName (pubid);
    sv2 = vFed1->getPublicationName (pubid2);
    BOOST_CHECK_EQUAL (sv, "test1/pub1");
    BOOST_CHECK_EQUAL (sv2, "pub2");
    auto pub3name = vFed1->getPublicationName (pubid3);
    BOOST_CHECK_EQUAL (pub3name, "test1/pub3");

    BOOST_CHECK_EQUAL (vFed1->getPublicationType (pubid3), "double");
    BOOST_CHECK_EQUAL (vFed1->getPublicationUnits (pubid3), "V");
    vFed1->finalize ();

    BOOST_CHECK (vFed1->currentState () == helics::Federate::op_states::finalize);
}


BOOST_DATA_TEST_CASE(value_federate_single_transfer, bdata::make(core_types), core_type)
{
    Setup1FederateTest(core_type);
    // register the publications
    auto pubid = vFed1->registerGlobalPublication<std::string> ("pub1");

    auto subid = vFed1->registerRequiredSubscription<std::string> ("pub1");
    vFed1->setTimeDelta (1.0);
    vFed1->enterExecutionState ();
    // publish string1 at time=0.0;
    vFed1->publish (pubid, "string1");
    auto gtime = vFed1->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    std::string s;
    // get the value
    vFed1->getValue (subid, s);
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (s, "string1");
    // publish a second string
    vFed1->publish (pubid, "string2");
    // make sure the value is still what we expect
    vFed1->getValue (subid, s);

    BOOST_CHECK_EQUAL (s, "string1");
    // advance time
    gtime = vFed1->requestTime (2.0);
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);
    vFed1->getValue (subid, s);

    BOOST_CHECK_EQUAL (s, "string2");
}


template <class X>
void runFederateTest (const std::string &core_type_str, const X &defaultValue, const X &testValue1, const X &testValue2)
{
	ValueFederateTestFixture fixture;

	fixture.Setup1FederateTest(core_type_str);

	auto &vFed = fixture.vFed1;
    // register the publications
    auto pubid = vFed->registerGlobalPublication<X> ("pub1");

    auto subid = vFed->registerRequiredSubscription<X> ("pub1");
    vFed->setTimeDelta (1.0);
    vFed->setDefaultValue<X> (subid, defaultValue);
    vFed->enterExecutionState ();
    // publish string1 at time=0.0;
    vFed->publish<X> (pubid, testValue1);

    X val;
    vFed->getValue<X> (subid, val);
    BOOST_CHECK_EQUAL (val, defaultValue);


    auto gtime = vFed->requestTime (1.0);
    BOOST_CHECK_EQUAL (gtime, 1.0);
    // get the value
    vFed->getValue (subid, val);
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (val, testValue1);
    // publish a second string
    vFed->publish (pubid, testValue2);
    // make sure the value is still what we expect
    vFed->getValue (subid, val);
    BOOST_CHECK_EQUAL (val, testValue1);

    // advance time
    gtime = vFed->requestTime (2.0);
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);
    vFed->getValue (subid, val);

    BOOST_CHECK_EQUAL (val, testValue2);

    vFed->finalize ();
}

template <class X>
void runFederateTestv2 (const std::string &core_type_str, const X &defaultValue, const X &testValue1, const X &testValue2)
{
	ValueFederateTestFixture fixture;

	fixture.Setup1FederateTest(core_type_str);

	auto &vFed = fixture.vFed1;
    // register the publications
    auto pubid = vFed->registerGlobalPublication<X> ("pub1");

    auto subid = vFed->registerRequiredSubscription<X> ("pub1");
    vFed->setTimeDelta (1.0);
    vFed->setDefaultValue<X> (subid, defaultValue);
    vFed->enterExecutionState ();
    // publish string1 at time=0.0;
    vFed->publish<X> (pubid, testValue1);

    X val;
    vFed->getValue<X> (subid, val);
    BOOST_CHECK (val == defaultValue);

    auto gtime = vFed->requestTime (1.0);
    BOOST_CHECK_EQUAL (gtime, 1.0);
    // get the value
    vFed->getValue (subid, val);
    // make sure the string is what we expect

    BOOST_CHECK (val == testValue1);
    // publish a second string
    vFed->publish (pubid, testValue2);
    // make sure the value is still what we expect
    vFed->getValue (subid, val);

    BOOST_CHECK (val == testValue1);
    // advance time
    gtime = vFed->requestTime (2.0);
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);
    vFed->getValue (subid, val);
    BOOST_CHECK (val == testValue2);
    vFed->finalize ();
}


BOOST_DATA_TEST_CASE(value_federate_single_transfer_types, bdata::make(core_types), core_type)
{
    runFederateTest<double> (core_type, 10.3, 45.3, 22.7);
	runFederateTest<double>(core_type, 1.0, 0.0, 3.0);
    runFederateTest<int> (core_type, 5, 8, 43);
    runFederateTest<int> (core_type, -5, 1241515, -43);
    runFederateTest<short> (core_type, -5, 23023, -43);
    runFederateTest<uint64_t> (core_type, 234252315, 0xFFF1'2345'7124'1412, 23521513412);
    runFederateTest<float> (core_type, 10.3f, 45.3f, 22.7f);
    runFederateTest<std::string> (core_type, "start", "inside of the functional relationship of helics",
                                  std::string ("I am a string"));
    runFederateTestv2<std::vector<double>> (core_type, {34.3, 24.2}, {12.4, 14.7, 16.34, 18.17}, {9.9999, 8.8888, 7.7777});
    std::vector<std::string> sv1{"hip", "hop"};
    std::vector<std::string> sv2{"this is the first string\n", "this is the second string",
                                 "this is the third\0"
                                 " string"};
    std::vector<std::string> sv3{"string1", "String2", "string3", "string4", "string5", "string6", "string8"};
    runFederateTestv2 (core_type, sv1, sv2, sv3);
    std::complex<double> def = {54.23233, 0.7};
    std::complex<double> v1 = std::polar (10.0, 0.43);
    std::complex<double> v2 = {-3e45, 1e-23};
    runFederateTest<std::complex<double>> (core_type, def, v1, v2);
}


BOOST_DATA_TEST_CASE(value_federate_dual_transfer, bdata::make(core_types), core_type)
{
    Setup2FederateTest(core_type);
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
}


template <class X>
void runDualFederateTest (const std::string &core_type_str, const X &defaultValue, const X &testValue1, const X &testValue2)
{
	ValueFederateTestFixture fixture;

	fixture.Setup2FederateTest(core_type_str);

	auto &fedA = fixture.vFed1;
	auto &fedB = fixture.vFed2;

    // register the publications
    auto pubid = fedA->registerGlobalPublication<X> ("pub1");

    auto subid = fedB->registerRequiredSubscription<X> ("pub1");
    fedA->setTimeDelta (1.0);
    fedB->setTimeDelta (1.0);

    fedB->setDefaultValue<X> (subid, defaultValue);

    auto f1finish = std::async (std::launch::async, [&]() { fedA->enterExecutionState (); });
    fedB->enterExecutionState ();
    f1finish.wait ();
    // publish string1 at time=0.0;
    fedA->publish<X> (pubid, testValue1);

    X val;
    fedB->getValue<X> (subid, val);

    BOOST_CHECK_EQUAL (val, defaultValue);

    auto f1time = std::async (std::launch::async, [&]() { return fedA->requestTime (1.0); });
    auto gtime = fedB->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK_EQUAL (f1time.get (), 1.0);
    // get the value
    fedB->getValue (subid, val);
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (val, testValue1);

    // publish a second string
    fedA->publish (pubid, testValue2);
    // make sure the value is still what we expect
    fedB->getValue (subid, val);

    BOOST_CHECK_EQUAL (val, testValue1);

    // advance time
    f1time = std::async (std::launch::async, [&]() { return fedA->requestTime (2.0); });
    gtime = fedB->requestTime (2.0);

    BOOST_CHECK_EQUAL (gtime, 2.0);
    BOOST_CHECK_EQUAL (f1time.get (), 2.0);

    // make sure the value was updated
    fedB->getValue (subid, val);
    BOOST_CHECK_EQUAL (val, testValue2);
}


template <class X>
void runDualFederateTestv2 (const std::string &core_type_str, X &defaultValue, const X &testValue1, const X &testValue2)
{
	ValueFederateTestFixture fixture;

	fixture.Setup2FederateTest(core_type_str);

	auto &fedA = fixture.vFed1;
	auto &fedB = fixture.vFed2;
    // register the publications
    auto pubid = fedA->registerGlobalPublication<X> ("pub1");

    auto subid = fedB->registerRequiredSubscription<X> ("pub1");
    fedA->setTimeDelta (1.0);
    fedB->setTimeDelta (1.0);

    fedB->setDefaultValue<X> (subid, defaultValue);

    auto f1finish = std::async (std::launch::async, [&]() { fedA->enterExecutionState (); });
    fedB->enterExecutionState ();
    f1finish.wait ();
    // publish string1 at time=0.0;
    fedA->publish<X> (pubid, testValue1);

    X val;
    fedB->getValue<X> (subid, val);
    BOOST_CHECK (val == defaultValue);
    auto f1time = std::async (std::launch::async, [&]() { return fedA->requestTime (1.0); });
    auto gtime = fedB->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK_EQUAL (f1time.get (), 1.0);
    // get the value
    fedB->getValue (subid, val);
    // make sure the string is what we expect
    BOOST_CHECK (val == testValue1);
    // publish a second string
    fedA->publish (pubid, testValue2);
    // make sure the value is still what we expect
    fedB->getValue (subid, val);
    BOOST_CHECK (val == testValue1);
    // advance time
    f1time = std::async (std::launch::async, [&]() { return fedA->requestTime (2.0); });
    gtime = fedB->requestTime (2.0);

    BOOST_CHECK_EQUAL (gtime, 2.0);
    BOOST_CHECK_EQUAL (f1time.get (), 2.0);

    // make sure the value was updated
    fedB->getValue (subid, val);
    BOOST_CHECK (val == testValue2);

}


/** test case checking that the transfer between two federates works as expected
 */
BOOST_DATA_TEST_CASE(value_federate_dual_transfer_types, bdata::make(core_types), core_type)
{
   
    runDualFederateTest<double> (core_type, 10.3, 45.3, 22.7);
    runDualFederateTest<int> (core_type, 5, 8, 43);
    runDualFederateTest<int> (core_type, -5, 1241515, -43);
    runDualFederateTest<char> (core_type, 'c', '\0', '\n');

    runDualFederateTest<uint64_t> (core_type, 234252315, 0xFFF1'2345'7124'1412, 23521513412);
    runDualFederateTest<float> (core_type, 10.3f, 45.3f, 22.7f);
    runDualFederateTest<std::string> (core_type, "start", "inside of the functional relationship of helics",
                                      std::string ("I am a string"));
    // this one is going to test really ugly strings
    runDualFederateTest<std::string> (core_type, std::string (862634, '\0'),
                                      "inside\n\0 of the \0\n functional\r \brelationship of helics\n",
                                      std::string (""));
    std::vector<double> defVec = { 34.3, 24.2 };
    std::vector<double> v1Vec = { 12.4, 14.7, 16.34, 18.17 };
    std::vector<double> v2Vec = { 9.9999, 8.8888, 7.7777 };
    runDualFederateTestv2<std::vector<double>> (core_type, defVec, v1Vec, v2Vec);
    std::complex<double> def = {54.23233, 0.7};
    std::complex<double> v1 = std::polar (10.0, 0.43);
    std::complex<double> v2 = {-3e45, 1e-23};
    runDualFederateTest<std::complex<double>> (core_type, def, v1, v2);
}


BOOST_DATA_TEST_CASE(value_federate_single_init_publish, bdata::make(core_types), core_type)
{
    Setup1FederateTest(core_type);
	// register the publications
	auto pubid = vFed1->registerGlobalPublication<double>("pub1");

	auto subid = vFed1->registerRequiredSubscription<double>("pub1");
	vFed1->setTimeDelta(1.0);
	vFed1->enterInitializationState();
	vFed1->publish(pubid, 1.0);

	vFed1->enterExecutionState();
	// get the value set at initialization
	double val;
	vFed1->getValue(subid, val);
	BOOST_CHECK_EQUAL(val, 1.0);
	// publish string1 at time=0.0;
	vFed1->publish(pubid, 2.0);
	auto gtime = vFed1->requestTime(1.0);
	
	BOOST_CHECK_EQUAL(gtime, 1.0);
	
	// get the value
	vFed1->getValue(subid, val);
	// make sure the string is what we expect
	BOOST_CHECK_EQUAL(val, 2.0);
	// publish a second string
	vFed1->publish(pubid, 3.0);
	// make sure the value is still what we expect
	vFed1->getValue(subid, val);

	BOOST_CHECK_EQUAL(val, 2.0);
	// advance time
	gtime = vFed1->requestTime(2.0);
	// make sure the value was updated
	BOOST_CHECK_EQUAL(gtime, 2.0);
	vFed1->getValue(subid, val);

	BOOST_CHECK_EQUAL(val, 3.0);
}
BOOST_AUTO_TEST_SUITE_END ()