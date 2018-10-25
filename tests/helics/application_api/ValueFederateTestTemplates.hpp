/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "testFixtures.hpp"
#include <future>
#include <boost/test/unit_test.hpp>

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

    auto subid = vFed->registerSubscription ("pub1");
    vFed->setTimeProperty (TIME_DELTA_PROPERTY, 1.0);
    vFed->setDefaultValue<X> (subid, defaultValue);
    vFed->enterExecutingMode ();
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

    auto subid=helics::make_subscription<X>(*vFed, "pub1");

    vFed->setTimeProperty (TIME_DELTA_PROPERTY, 1.0);
    subid.setDefault (defaultValue);
    vFed->enterExecutingMode ();
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

    auto subid = vFed->registerSubscription ("pub1");
    vFed->setTimeProperty (TIME_DELTA_PROPERTY, 1.0);
    vFed->setDefaultValue<X> (subid, defaultValue);
    vFed->enterExecutingMode ();
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

template <class X>
void runFederateTestObjv2 (const std::string &core_type_str,
                           const X &defaultValue,
                           const X &testValue1,
                           const X &testValue2)
{
    FederateTestFixture fixture;

    fixture.SetupTest<helics::ValueFederate> (core_type_str, 1);
    auto vFed = fixture.GetFederateAs<helics::ValueFederate> (0);

    // register the publications
    helics::PublicationT<X> pubid (helics::GLOBAL, vFed.get (), "pub1");

    auto sub =helics::make_subscription<X>(vFed.get (), "pub1");
    vFed->setTimeProperty (TIME_DELTA_PROPERTY, 1.0);
    sub.setDefault (defaultValue);
    vFed->enterExecutingMode ();
    // publish string1 at time=0.0;
    pubid.publish (testValue1);
    X val=sub.getValue ();
    BOOST_CHECK (val == defaultValue);

    auto gtime = vFed->requestTime (1.0);
    BOOST_CHECK_EQUAL (gtime, 1.0);
    // get the value
    sub.getValue (val);
    // make sure the string is what we expect
    BOOST_CHECK (val == testValue1);
    // publish a second string
    pubid.publish (testValue2);
    // make sure the value is still what we expect
    val = sub.getValue ();
    BOOST_CHECK (val == testValue1);

    // advance time
    gtime = vFed->requestTime (2.0);
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);
    val = sub.getValue ();
    BOOST_CHECK (val == testValue2);

    vFed->finalize ();
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

    auto subid = fedB->registerSubscription("pub1");
    fedA->setTimeProperty (TIME_DELTA_PROPERTY, 1.0);
    fedB->setTimeProperty (TIME_DELTA_PROPERTY, 1.0);

    fedB->setDefaultValue<X> (subid, defaultValue);

    auto f1finish = std::async (std::launch::async, [&]() { fedA->enterExecutingMode (); });
    fedB->enterExecutingMode ();
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

    auto subid = fedB->registerSubscription ("pub1");
    fedA->setTimeProperty (TIME_DELTA_PROPERTY, 1.0);
    fedB->setTimeProperty (TIME_DELTA_PROPERTY, 1.0);

    fedB->setDefaultValue<X> (subid, defaultValue);

    auto f1finish = std::async (std::launch::async, [&]() { fedA->enterExecutingMode (); });
    fedB->enterExecutingMode ();
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
    PublicationT<X> pubid (GLOBAL, fedA, "pub1");

    auto subid =make_subscription<X> (*fedB, "pub1");
    fedA->setTimeProperty (TIME_DELTA_PROPERTY, 1.0);
    fedB->setTimeProperty (TIME_DELTA_PROPERTY, 1.0);

    subid.setDefault (defaultValue);

    auto f1finish = std::async (std::launch::async, [&]() { fedA->enterExecutingMode (); });
    fedB->enterExecutingMode ();
    f1finish.wait ();
    // publish string1 at time=0.0;
    pubid.publish (testValue1);

    X val;
    subid.getValue (val);

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
    fedB->getValue (subid, val);
    BOOST_CHECK_EQUAL (val, testValue2);
    fedA->finalize ();
    fedB->finalize ();
    helics::cleanupHelicsLibrary ();
}

template <class X>
void runDualFederateTestObjv2 (const std::string &core_type_str,
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

    auto subid =helics::make_subscription<X>(fedB.get (), "pub1");
    fedA->setTimeProperty (TIME_DELTA_PROPERTY, 1.0);
    fedB->setTimeProperty (TIME_DELTA_PROPERTY, 1.0);

    subid.setDefault (defaultValue);

    auto f1finish = std::async (std::launch::async, [&]() { fedA->enterExecutingMode (); });
    fedB->enterExecutingMode ();
    f1finish.wait ();
    // publish string1 at time=0.0;
    pubid.publish (testValue1);

    X val = subid.getValue ();

    BOOST_CHECK (val == defaultValue);

    auto f1time = std::async (std::launch::async, [&]() { return fedA->requestTime (1.0); });
    auto gtime = fedB->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK_EQUAL (f1time.get (), 1.0);
    // get the value
    subid.getValue (val);
    // make sure the string is what we expect
    BOOST_CHECK (val == testValue1);

    // publish a second string
    pubid.publish (testValue2);

    subid.getValue (val);
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
