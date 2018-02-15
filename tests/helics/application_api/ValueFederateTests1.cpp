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

#include <future>

#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"
#include "testFixtures.hpp"
#include "test_configuration.h"

using namespace std::string_literals;
/** these test cases test out the value federates
 */

BOOST_FIXTURE_TEST_SUITE (value_federate_tests, FederateTestFixture)

namespace bdata = boost::unit_test::data;
#if ENABLE_TEST_TIMEOUTS > 0
namespace utf = boost::unit_test;
#endif

// const std::string core_types[] = {"udp" };
/** test simple creation and destruction*/
BOOST_DATA_TEST_CASE (value_federate_initialize_tests, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    vFed1->enterExecutionState ();

    BOOST_CHECK (vFed1->getCurrentState () == helics::Federate::op_states::execution);

    vFed1->finalize ();

    BOOST_CHECK (vFed1->getCurrentState () == helics::Federate::op_states::finalize);
}

BOOST_DATA_TEST_CASE (value_federate_publication_registration, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    auto pubid = vFed1->registerPublication<std::string> ("pub1");
    auto pubid2 = vFed1->registerGlobalPublication<int> ("pub2");

    auto pubid3 = vFed1->registerPublication ("pub3", "double", "V");
    vFed1->enterExecutionState ();

    BOOST_CHECK (vFed1->getCurrentState () == helics::Federate::op_states::execution);

    auto sv = vFed1->getPublicationKey (pubid);
    auto sv2 = vFed1->getPublicationKey (pubid2);
    BOOST_CHECK_EQUAL (sv, "fed0/pub1");
    BOOST_CHECK_EQUAL (sv2, "pub2");
    auto pub3name = vFed1->getPublicationKey (pubid3);
    BOOST_CHECK_EQUAL (pub3name, "fed0/pub3");

    BOOST_CHECK_EQUAL (vFed1->getPublicationType (pubid3), "double");
    BOOST_CHECK_EQUAL (vFed1->getPublicationUnits (pubid3), "V");

    BOOST_CHECK (vFed1->getPublicationId ("pub1") == pubid);
    BOOST_CHECK (vFed1->getPublicationId ("pub2") == pubid2);
    BOOST_CHECK (vFed1->getPublicationId ("fed0/pub1") == pubid);
    vFed1->finalize ();

    BOOST_CHECK (vFed1->getCurrentState () == helics::Federate::op_states::finalize);
}

BOOST_DATA_TEST_CASE (value_federate_publisher_registration, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    helics::Publication pubid (vFed1.get (), "pub1", helics::helicsType<std::string> ());
    helics::PublicationT<int> pubid2 (helics::GLOBAL, vFed1.get (), "pub2");

    helics::Publication pubid3 (vFed1.get (), "pub3", helics::helicsType<double> (), "V");
    vFed1->enterExecutionState ();

    BOOST_CHECK (vFed1->getCurrentState () == helics::Federate::op_states::execution);

    auto sv = pubid.getKey ();
    auto sv2 = pubid2.getKey ();
    BOOST_CHECK_EQUAL (sv, "fed0/pub1");
    BOOST_CHECK_EQUAL (sv2, "pub2");
    auto pub3name = pubid3.getKey ();
    BOOST_CHECK_EQUAL (pub3name, "fed0/pub3");

    BOOST_CHECK_EQUAL (pubid3.getType (), "double");
    BOOST_CHECK_EQUAL (pubid3.getUnits (), "V");

    BOOST_CHECK (vFed1->getPublicationId ("pub1") == pubid.getID ());
    BOOST_CHECK (vFed1->getPublicationId ("pub2") == pubid2.getID ());
    BOOST_CHECK (vFed1->getPublicationId ("fed0/pub1") == pubid.getID ());
    vFed1->finalize ();

    BOOST_CHECK (vFed1->getCurrentState () == helics::Federate::op_states::finalize);
}

BOOST_DATA_TEST_CASE (value_federate_subscription_registration, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    auto subid = vFed1->registerRequiredSubscription ("sub1", "double", "V");
    auto subid2 = vFed1->registerRequiredSubscription<int> ("sub2");

    auto subid3 = vFed1->registerOptionalSubscription ("sub3", "double", "V");
    vFed1->enterExecutionState ();

    // BOOST_CHECK (vFed->getCurrentState () == helics::Federate::op_states::execution);

    auto sv = vFed1->getSubscriptionKey (subid);
    auto sv2 = vFed1->getSubscriptionKey (subid2);
    BOOST_CHECK_EQUAL (sv, "sub1");
    BOOST_CHECK_EQUAL (sv2, "sub2");
    auto sub3name = vFed1->getSubscriptionKey (subid3);

    vFed1->addSubscriptionShortcut (subid, "Shortcut");
    BOOST_CHECK_EQUAL (sub3name, "sub3");

    BOOST_CHECK_EQUAL (vFed1->getSubscriptionType (subid3), "double");
    BOOST_CHECK_EQUAL (vFed1->getSubscriptionUnits (subid3), "V");

    BOOST_CHECK (vFed1->getSubscriptionId ("sub1") == subid);
    BOOST_CHECK (vFed1->getSubscriptionId ("sub2") == subid2);

    BOOST_CHECK (vFed1->getSubscriptionId ("Shortcut") == subid);

    vFed1->finalize ();

    BOOST_CHECK (vFed1->getCurrentState () == helics::Federate::op_states::finalize);
    helics::cleanupHelicsLibrary ();
}

BOOST_DATA_TEST_CASE (value_federate_subscription_and_publication_registration,
                      bdata::make (core_types_single),
                      core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    // register the publications
    auto pubid = vFed1->registerPublication<std::string> ("pub1");
    auto pubid2 = vFed1->registerGlobalPublication<int> ("pub2");

    auto pubid3 = vFed1->registerPublication ("pub3", "double", "V");

    auto subid = vFed1->registerOptionalSubscription ("sub1", "double", "V");
    auto subid2 = vFed1->registerOptionalSubscription<int> ("sub2");

    auto subid3 = vFed1->registerOptionalSubscription ("sub3", "double", "V");
    // enter execution
    vFed1->enterExecutionState ();

    BOOST_CHECK (vFed1->getCurrentState () == helics::Federate::op_states::execution);
    // check subscriptions
    auto sv = vFed1->getSubscriptionKey (subid);
    auto sv2 = vFed1->getSubscriptionKey (subid2);
    BOOST_CHECK_EQUAL (sv, "sub1");
    BOOST_CHECK_EQUAL (sv2, "sub2");
    auto sub3name = vFed1->getSubscriptionKey (subid3);
    BOOST_CHECK_EQUAL (sub3name, "sub3");

    BOOST_CHECK_EQUAL (vFed1->getSubscriptionType (subid3), "double");
    BOOST_CHECK_EQUAL (vFed1->getSubscriptionUnits (subid3), "V");

    // check publications

    sv = vFed1->getPublicationKey (pubid);
    sv2 = vFed1->getPublicationKey (pubid2);
    BOOST_CHECK_EQUAL (sv, "fed0/pub1");
    BOOST_CHECK_EQUAL (sv2, "pub2");
    auto pub3name = vFed1->getPublicationKey (pubid3);
    BOOST_CHECK_EQUAL (pub3name, "fed0/pub3");

    BOOST_CHECK_EQUAL (vFed1->getPublicationType (pubid3), "double");
    BOOST_CHECK_EQUAL (vFed1->getPublicationUnits (pubid3), "V");
    vFed1->finalize ();

    BOOST_CHECK (vFed1->getCurrentState () == helics::Federate::op_states::finalize);
    helics::cleanupHelicsLibrary ();
}

BOOST_DATA_TEST_CASE (value_federate_subscriber_and_publisher_registration,
                      bdata::make (core_types_single),
                      core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    // register the publications
    helics::Publication pubid (vFed1.get (), "pub1", helics::helicsType<std::string> ());
    helics::PublicationT<int> pubid2 (helics::GLOBAL, vFed1.get (), "pub2");

    helics::Publication pubid3 (vFed1.get (), "pub3", helics::helicsType<double> (), "V");

    // these aren't meant to match the publications
    helics::Subscription subid1 (helics::OPTIONAL, vFed1.get (), "sub1");

    helics::SubscriptionT<int> subid2 (helics::OPTIONAL, vFed1.get (), "sub2");

    helics::Subscription subid3 (helics::OPTIONAL, vFed1.get (), "sub3", "V");
    // enter execution
    vFed1->enterExecutionState ();

    BOOST_CHECK (vFed1->getCurrentState () == helics::Federate::op_states::execution);
    // check subscriptions
    const auto &sv = subid1.getName ();
    const auto &sv2 = subid2.getName ();
    BOOST_CHECK_EQUAL (sv, "sub1");
    BOOST_CHECK_EQUAL (sv2, "sub2");
    const auto &sub3name = subid3.getKey ();
    BOOST_CHECK_EQUAL (sub3name, "sub3");

    BOOST_CHECK_EQUAL (subid1.getType (), "def");  // def is the default type
    BOOST_CHECK_EQUAL (subid2.getType (), "int32");
    BOOST_CHECK_EQUAL (subid3.getType (), "def");
    BOOST_CHECK_EQUAL (subid3.getUnits (), "V");

    // check publications

    auto pk = pubid.getKey ();
    auto pk2 = pubid2.getKey ();
    BOOST_CHECK_EQUAL (pk, "fed0/pub1");
    BOOST_CHECK_EQUAL (pk2, "pub2");
    auto pub3name = pubid3.getKey ();
    BOOST_CHECK_EQUAL (pub3name, "fed0/pub3");

    BOOST_CHECK_EQUAL (pubid3.getType (), "double");
    BOOST_CHECK_EQUAL (pubid3.getUnits (), "V");
    vFed1->finalize ();

    BOOST_CHECK (vFed1->getCurrentState () == helics::Federate::op_states::finalize);
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (5))
#endif
BOOST_DATA_TEST_CASE (value_federate_single_transfer, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

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

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (5))
#endif
BOOST_DATA_TEST_CASE (value_federate_single_transfer_publisher, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    // register the publications
    helics::Publication pubid (helics::GLOBAL, vFed1.get (), "pub1", helics::helics_type_t::helicsString);

    helics::Subscription subid (vFed1.get (), "pub1");
    vFed1->setTimeDelta (1.0);
    vFed1->enterExecutionState ();
    // publish string1 at time=0.0;
    pubid.publish ("string1");
    auto gtime = vFed1->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    std::string s;
    // get the value
    subid.getValue (s);
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (s, "string1");
    // publish a second string
    pubid.publish ("string2");
    // make sure the value is still what we expect
    subid.getValue (s);

    BOOST_CHECK_EQUAL (s, "string1");
    // advance time
    gtime = vFed1->requestTime (2.0);
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);
    subid.getValue (s);

    BOOST_CHECK_EQUAL (s, "string2");
}

template <class X>
void runFederateTest (const std::string &core_type_str,
                      const X &defaultValue,
                      const X &testValue1,
                      const X &testValue2)
{
    FederateTestFixture fixture;

    fixture.SetupTest<helics::ValueFederate> (core_type_str, 1);
    auto vFed = fixture.GetFederateAs<helics::ValueFederate> (0);

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
    BOOST_CHECK (vFed->getCurrentState () == helics::Federate::op_states::finalize);
    helics::cleanupHelicsLibrary ();
}

template <class X>
void runFederateTestObj (const std::string &core_type_str,
                         const X &defaultValue,
                         const X &testValue1,
                         const X &testValue2)
{
    FederateTestFixture fixture;

    fixture.SetupTest<helics::ValueFederate> (core_type_str, 1);
    auto vFed = fixture.GetFederateAs<helics::ValueFederate> (0);

    // register the publications
    helics::PublicationT<X> pubid (helics::GLOBAL, vFed.get (), "pub1");

    helics::SubscriptionT<X> subid (vFed.get (), "pub1");
    vFed->setTimeDelta (1.0);
    subid.setDefault (defaultValue);
    vFed->enterExecutionState ();
    // publish string1 at time=0.0;
    pubid.publish (testValue1);
    X val;
    subid.getValue (val);
    BOOST_CHECK_EQUAL (val, defaultValue);

    auto gtime = vFed->requestTime (1.0);
    BOOST_CHECK_EQUAL (gtime, 1.0);
    // get the value
    subid.getValue (val);
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (val, testValue1);
    // publish a second string
    pubid.publish (testValue2);
    // make sure the value is still what we expect
    val = subid.getValue ();
    BOOST_CHECK_EQUAL (val, testValue1);

    // advance time
    gtime = vFed->requestTime (2.0);
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);
    val = subid.getValue ();
    BOOST_CHECK_EQUAL (val, testValue2);

    vFed->finalize ();
}

template <class X>
void runFederateTestv2 (const std::string &core_type_str,
                        const X &defaultValue,
                        const X &testValue1,
                        const X &testValue2)
{
    FederateTestFixture fixture;

    fixture.SetupTest<helics::ValueFederate> (core_type_str, 1);
    auto vFed = fixture.GetFederateAs<helics::ValueFederate> (0);

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
    helics::cleanupHelicsLibrary ();
}

#ifndef QUICK_TESTS_ONLY

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (35))
#endif
BOOST_DATA_TEST_CASE (value_federate_single_transfer_types1, bdata::make (core_types_single), core_type)
{
    runFederateTest<double> (core_type, 10.3, 45.3, 22.7);
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (35))
#endif
BOOST_DATA_TEST_CASE (value_federate_single_transfer_types2, bdata::make (core_types_single), core_type)
{
    runFederateTest<double> (core_type, 1.0, 0.0, 3.0);
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (35))
#endif
BOOST_DATA_TEST_CASE (value_federate_single_transfer_types3, bdata::make (core_types_single), core_type)
{
    runFederateTest<int> (core_type, 5, 8, 43);
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (35))
#endif
BOOST_DATA_TEST_CASE (value_federate_single_transfer_types4, bdata::make (core_types_single), core_type)
{
    runFederateTest<int> (core_type, -5, 1241515, -43);
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (35))
#endif
BOOST_DATA_TEST_CASE (value_federate_single_transfer_types5, bdata::make (core_types_single), core_type)
{
    runFederateTest<int16_t> (core_type, -5, 23023, -43);
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (35))
#endif
BOOST_DATA_TEST_CASE (value_federate_single_transfer_types6, bdata::make (core_types_single), core_type)
{
    runFederateTest<uint64_t> (core_type, 234252315, 0xFFF1'2345'7124'1412, 23521513412);
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (35))
#endif
BOOST_DATA_TEST_CASE (value_federate_single_transfer_types7, bdata::make (core_types_single), core_type)
{
    runFederateTest<float> (core_type, 10.3f, 45.3f, 22.7f);
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (35))
#endif
BOOST_DATA_TEST_CASE (value_federate_single_transfer_types8, bdata::make (core_types_single), core_type)
{
    runFederateTest<std::string> (core_type, "start", "inside of the functional relationship of helics",
                                  std::string ("I am a string"));
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (35))
#endif
BOOST_DATA_TEST_CASE (value_federate_single_transfer_types9, bdata::make (core_types_single), core_type)
{
    runFederateTestv2<std::vector<double>> (core_type, {34.3, 24.2}, {12.4, 14.7, 16.34, 18.17},
                                            {9.9999, 8.8888, 7.7777});
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (35))
#endif
BOOST_DATA_TEST_CASE (value_federate_single_transfer_types10, bdata::make (core_types_single), core_type)
{
    std::vector<std::string> sv1{"hip", "hop"};
    std::vector<std::string> sv2{"this is the first string\n", "this is the second string",
                                 "this is the third\0"s
                                 " string"};
    std::vector<std::string> sv3{"string1", "String2", "string3", "string4", "string5", "string6", "string8"};
    runFederateTestv2 (core_type, sv1, sv2, sv3);
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (35))
#endif
BOOST_DATA_TEST_CASE (value_federate_single_transfer_types11, bdata::make (core_types_single), core_type)
{
    std::complex<double> def = {54.23233, 0.7};
    std::complex<double> v1 = std::polar (10.0, 0.43);
    std::complex<double> v2 = {-3e45, 1e-23};
    runFederateTest<std::complex<double>> (core_type, def, v1, v2);
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (5))
#endif
BOOST_DATA_TEST_CASE (value_federate_single_transfer_types_publishers1, bdata::make (core_types_single), core_type)
{
    runFederateTestObj<double> (core_type, 10.3, 45.3, 22.7);
}
#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (5))
#endif
BOOST_DATA_TEST_CASE (value_federate_single_transfer_types_publishers2, bdata::make (core_types_single), core_type)
{
    runFederateTestObj<double> (core_type, 1.0, 0.0, 3.0);
}
#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (5))
#endif
BOOST_DATA_TEST_CASE (value_federate_single_transfer_types_publishers3, bdata::make (core_types_single), core_type)
{
    runFederateTestObj<int> (core_type, 5, 8, 43);
}
#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (5))
#endif
BOOST_DATA_TEST_CASE (value_federate_single_transfer_types_publishers4, bdata::make (core_types_single), core_type)
{
    runFederateTestObj<int> (core_type, -5, 1241515, -43);
}
#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (5))
#endif
BOOST_DATA_TEST_CASE (value_federate_single_transfer_types_publishers5, bdata::make (core_types_single), core_type)
{
    runFederateTestObj<int16_t> (core_type, -5, 23023, -43);
}
#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (5))
#endif
BOOST_DATA_TEST_CASE (value_federate_single_transfer_types_publishers6, bdata::make (core_types_single), core_type)
{
    runFederateTestObj<uint64_t> (core_type, 234252315, 0xFFF1'2345'7124'1412, 23521513412);
}
#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (5))
#endif
BOOST_DATA_TEST_CASE (value_federate_single_transfer_types_publishers7, bdata::make (core_types_single), core_type)
{
    runFederateTestObj<float> (core_type, 10.3f, 45.3f, 22.7f);
}
#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (5))
#endif
BOOST_DATA_TEST_CASE (value_federate_single_transfer_types_publishers8, bdata::make (core_types_single), core_type)
{
    runFederateTestObj<std::string> (core_type, "start", "inside of the functional relationship of helics",
                                     std::string ("I am a string"));
}
#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (5))
#endif
BOOST_DATA_TEST_CASE (value_federate_single_transfer_types_publishers9, bdata::make (core_types_single), core_type)
{
    std::complex<double> def = {54.23233, 0.7};
    std::complex<double> v1 = std::polar (10.0, 0.43);
    std::complex<double> v2 = {-3e45, 1e-23};
    runFederateTestObj<std::complex<double>> (core_type, def, v1, v2);
}

#endif
#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (5))
#endif
BOOST_DATA_TEST_CASE (value_federate_dual_transfer, bdata::make (core_types), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

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
void runDualFederateTest (const std::string &core_type_str,
                          const X &defaultValue,
                          const X &testValue1,
                          const X &testValue2)
{
    FederateTestFixture fixture;

    fixture.SetupTest<helics::ValueFederate> (core_type_str, 2);
    auto fedA = fixture.GetFederateAs<helics::ValueFederate> (0);
    auto fedB = fixture.GetFederateAs<helics::ValueFederate> (1);

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
    fedA->finalize ();
    fedB->finalize ();
    helics::cleanupHelicsLibrary ();
}

template <class X>
void runDualFederateTestv2 (const std::string &core_type_str,
                            X &defaultValue,
                            const X &testValue1,
                            const X &testValue2)
{
    FederateTestFixture fixture;

    fixture.SetupTest<helics::ValueFederate> (core_type_str, 2);
    auto fedA = fixture.GetFederateAs<helics::ValueFederate> (0);
    auto fedB = fixture.GetFederateAs<helics::ValueFederate> (1);

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
    fedA->finalize ();
    fedB->finalize ();
    helics::cleanupHelicsLibrary ();
}

template <class X>
void runDualFederateTestObj (const std::string &core_type_str,
                             const X &defaultValue,
                             const X &testValue1,
                             const X &testValue2)
{
    FederateTestFixture fixture;
    using namespace helics;
    fixture.SetupTest<helics::ValueFederate> (core_type_str, 2);
    auto fedA = fixture.GetFederateAs<helics::ValueFederate> (0);
    auto fedB = fixture.GetFederateAs<helics::ValueFederate> (1);

    // register the publications
    PublicationT<X> pubid (GLOBAL, fedA.get (), "pub1");

    SubscriptionT<X> subid (fedB.get (), "pub1");
    fedA->setTimeDelta (1.0);
    fedB->setTimeDelta (1.0);

    subid.setDefault (defaultValue);

    auto f1finish = std::async (std::launch::async, [&]() { fedA->enterExecutionState (); });
    fedB->enterExecutionState ();
    f1finish.wait ();
    // publish string1 at time=0.0;
    pubid.publish (testValue1);

    X val = subid.getValue ();

    BOOST_CHECK_EQUAL (val, defaultValue);

    auto f1time = std::async (std::launch::async, [&]() { return fedA->requestTime (1.0); });
    auto gtime = fedB->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK_EQUAL (f1time.get (), 1.0);
    // get the value
    subid.getValue (val);
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (val, testValue1);

    // publish a second string
    pubid.publish (testValue2);

    subid.getValue (val);
    BOOST_CHECK_EQUAL (val, testValue1);

    // advance time
    f1time = std::async (std::launch::async, [&]() { return fedA->requestTime (2.0); });
    gtime = fedB->requestTime (2.0);

    BOOST_CHECK_EQUAL (gtime, 2.0);
    BOOST_CHECK_EQUAL (f1time.get (), 2.0);

    // make sure the value was updated
    fedB->getValue (subid.getID (), val);
    BOOST_CHECK_EQUAL (val, testValue2);
    fedA->finalize ();
    fedB->finalize ();
    helics::cleanupHelicsLibrary ();
}

#ifndef QUICK_TESTS_ONLY
/** test case checking that the transfer between two federates works as expected
 */
#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (10))
#endif
BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types1, bdata::make (core_types), core_type)
{
    runDualFederateTest<double> (core_type, 10.3, 45.3, 22.7);
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (10))
#endif
BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types2, bdata::make (core_types), core_type)
{
    runDualFederateTest<int> (core_type, 5, 8, 43);
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (10))
#endif
BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types3, bdata::make (core_types), core_type)
{
    runDualFederateTest<int> (core_type, -5, 1241515, -43);
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (10))
#endif
BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types4, bdata::make (core_types), core_type)
{
    runDualFederateTest<char> (core_type, 'c', '\0', '\n');
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (10))
#endif
BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types5, bdata::make (core_types), core_type)
{
    runDualFederateTest<uint64_t> (core_type, 234252315, 0xFFF1'2345'7124'1412, 23521513412);
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (10))
#endif
BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types6, bdata::make (core_types), core_type)
{
    runDualFederateTest<float> (core_type, 10.3f, 45.3f, 22.7f);
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (10))
#endif
BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types7, bdata::make (core_types), core_type)
{
    runDualFederateTest<std::string> (core_type, "start", "inside of the functional relationship of helics",
                                      std::string ("I am a string"));
}

#endif /*ifndef QUICK_TEST_ONLY*/

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (10))
#endif
BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types8, bdata::make (core_types), core_type)
{
    // this one is going to test really ugly strings
    runDualFederateTest<std::string> (core_type, std::string (86263, '\0'),
                                      "inside\n\0 of the \0\n functional\r \brelationship of helics\n"s,
                                      std::string (""));
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (10))
#endif
BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types9, bdata::make (core_types), core_type)
{
    std::vector<double> defVec = {34.3, 24.2};
    std::vector<double> v1Vec = {12.4, 14.7, 16.34, 18.17};
    std::vector<double> v2Vec = {9.9999, 8.8888, 7.7777};
    runDualFederateTestv2<std::vector<double>> (core_type, defVec, v1Vec, v2Vec);
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (5))
#endif
BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types10, bdata::make (core_types), core_type)
{
    std::complex<double> def = {54.23233, 0.7};
    std::complex<double> v1 = std::polar (10.0, 0.43);
    std::complex<double> v2 = {-3e45, 1e-23};
    runDualFederateTest<std::complex<double>> (core_type, def, v1, v2);
}

#ifndef QUICK_TESTS_ONLY
/** test case checking that the transfer between two federates works as expected with publication and subscription
 * objects
 */
#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (6))
#endif
BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types_obj1, bdata::make (core_types), core_type)
{
    runDualFederateTestObj<double> (core_type, 10.3, 45.3, 22.7);
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (6))
#endif
BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types_obj2, bdata::make (core_types), core_type)
{
    runDualFederateTestObj<int> (core_type, 5, 8, 43);
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (6))
#endif
BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types_obj3, bdata::make (core_types), core_type)
{
    runDualFederateTestObj<int> (core_type, -5, 1241515, -43);
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (6))
#endif
BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types_obj4, bdata::make (core_types), core_type)
{
    runDualFederateTestObj<char> (core_type, 'c', '\0', '\n');
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (6))
#endif
BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types_obj5, bdata::make (core_types), core_type)
{
    runDualFederateTestObj<uint64_t> (core_type, 234252315, 0xFFF1'2345'7124'1412, 23521513412);
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (6))
#endif
BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types_obj6, bdata::make (core_types), core_type)
{
    runDualFederateTestObj<float> (core_type, 10.3f, 45.3f, 22.7f);
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (6))
#endif
BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types_obj7, bdata::make (core_types), core_type)
{
    runDualFederateTestObj<std::string> (core_type, "start", "inside of the functional relationship of helics",
                                         std::string ("I am a string"));
    // this one is going to test really ugly strings
}

#endif /** ifndef QUICK_TESTS_ONLY*/

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (6))
#endif
BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types_obj8, bdata::make (core_types), core_type)
{
    runDualFederateTestObj<std::string> (core_type, std::string (86263, '\0'),
                                         "inside\n\0 of the \0\n functional\r \brelationship of helics\n"s,
                                         std::string (""));
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (6))
#endif
BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types_obj9, bdata::make (core_types), core_type)
{
    std::complex<double> def = {54.23233, 0.7};
    std::complex<double> v1 = std::polar (10.0, 0.43);
    std::complex<double> v2 = {-3e45, 1e-23};
    runDualFederateTestObj<std::complex<double>> (core_type, def, v1, v2);
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (40))
#endif
BOOST_DATA_TEST_CASE (value_federate_single_init_publish, bdata::make (core_types), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    // register the publications
    auto pubid = vFed1->registerGlobalPublication<double> ("pub1");

    auto subid = vFed1->registerRequiredSubscription<double> ("pub1");
    vFed1->setTimeDelta (1.0);
    vFed1->enterInitializationState ();
    vFed1->publish (pubid, 1.0);

    vFed1->enterExecutionState ();
    // get the value set at initialization
    double val;
    vFed1->getValue (subid, val);
    BOOST_CHECK_EQUAL (val, 1.0);
    // publish string1 at time=0.0;
    vFed1->publish (pubid, 2.0);
    auto gtime = vFed1->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    vFed1->getValue (subid, val);
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (val, 2.0);
    // publish a second string
    vFed1->publish (pubid, 3.0);
    // make sure the value is still what we expect
    vFed1->getValue (subid, val);

    BOOST_CHECK_EQUAL (val, 2.0);
    // advance time
    gtime = vFed1->requestTime (2.0);
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);
    vFed1->getValue (subid, val);

    BOOST_CHECK_EQUAL (val, 3.0);
}
BOOST_AUTO_TEST_SUITE_END ()