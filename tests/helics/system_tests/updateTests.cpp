/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "../application_api/testFixtures.hpp"
#include "helics/ValueFederates.hpp"
#include <future>
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

namespace bdata = boost::unit_test::data;
namespace utf = boost::unit_test;

/** tests for the different flag options and considerations*/

BOOST_FIXTURE_TEST_SUITE (update_tests, FederateTestFixture)

/** test simple creation and destruction*/
BOOST_AUTO_TEST_CASE (test_single_update)
{
    using namespace helics;
    SetupTest<ValueFederate> ("test", 2);
    auto fedA = GetFederateAs<ValueFederate> (0);
    auto fedB = GetFederateAs<ValueFederate> (1);

    // register the publications
    auto pub = fedA->registerGlobalPublication<std::string> ("pub1");

    auto sub = fedB->registerSubscription ("pub1");
    fedA->setProperty (helics_property_time_delta, 1.0);
    fedB->setProperty (helics_property_time_delta, 1.0);

    sub.setDefault (3.1);
    double testValue = 4.79;
    auto f1finish = std::async (std::launch::async, [&] () { fedA->enterExecutingMode (); });
    fedB->enterExecutingMode ();
    f1finish.wait ();
    // publish string1 at time=0.0;
    pub.publish (testValue);

    double val;
    sub.getValue (val);

    BOOST_CHECK_EQUAL (val, 3.1);

    auto f1time = std::async (std::launch::async, [&] () { return fedA->requestTime (1.0); });
    auto gtime = fedB->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK_EQUAL (f1time.get (), 1.0);
    BOOST_CHECK (sub.isUpdated ());
    // get the value
    sub.getValue (val);
    BOOST_CHECK (!sub.isUpdated ());
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (val, testValue);
    sub.getValue (val);
    BOOST_CHECK (!sub.isUpdated ());

    fedA->finalizeAsync ();
    fedB->finalize ();
    fedA->finalizeComplete ();
    helics::cleanupHelicsLibrary ();
}

/** test simple creation and destruction*/
BOOST_AUTO_TEST_CASE (test_single_update_string)
{
    using namespace helics;
    SetupTest<ValueFederate> ("test", 2);
    auto fedA = GetFederateAs<ValueFederate> (0);
    auto fedB = GetFederateAs<ValueFederate> (1);

    // register the publications
    auto pub = fedA->registerGlobalPublication<double> ("pub1");

    auto sub = fedB->registerSubscription ("pub1");
    fedA->setProperty (helics_property_time_delta, 1.0);
    fedB->setProperty (helics_property_time_delta, 1.0);

    sub.setDefault (3.1);
    double testValue = 4.79;
    double testValue2 = 9.34;
    auto f1finish = std::async (std::launch::async, [&] () { fedA->enterExecutingMode (); });
    fedB->enterExecutingMode ();
    f1finish.wait ();
    // publish string1 at time=0.0;
    pub.publish (testValue);

    std::string val;
    sub.getValue (val);

    BOOST_CHECK_EQUAL (val, "3.100000");

    auto f1time = std::async (std::launch::async, [&] () { return fedA->requestTime (1.0); });
    auto gtime = fedB->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK_EQUAL (f1time.get (), 1.0);
    BOOST_CHECK (sub.isUpdated ());
    // get the value
    sub.getValue (val);
    BOOST_CHECK (!sub.isUpdated ());
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (val, "4.790000");
    sub.getValue (val);
    BOOST_CHECK (!sub.isUpdated ());
    pub.publish (testValue2);

    f1time = std::async (std::launch::async, [&] () { return fedA->requestTime (2.0); });
    gtime = fedB->requestTime (2.0);
    BOOST_CHECK_EQUAL (f1time.get (), 2.0);
    BOOST_CHECK (sub.isUpdated ());
    // get the value
    sub.getValue (val);
    BOOST_CHECK (!sub.isUpdated ());
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (val, "9.340000");
    sub.getValue (val);
    BOOST_CHECK (!sub.isUpdated ());
    double v2;
    sub.getValue (v2);
    BOOST_CHECK (!sub.isUpdated ());

    fedA->finalizeAsync ();
    fedB->finalize ();
    fedA->finalizeComplete ();
    helics::cleanupHelicsLibrary ();
}

/** test simple creation and destruction*/
BOOST_AUTO_TEST_CASE (test_single_update_vector)
{
    using namespace helics;
    SetupTest<ValueFederate> ("test", 2);
    auto fedA = GetFederateAs<ValueFederate> (0);
    auto fedB = GetFederateAs<ValueFederate> (1);

    // register the publications
    auto pub = fedA->registerGlobalPublication<std::vector<double>> ("pub1");

    auto sub = fedB->registerSubscription ("pub1");
    fedA->setProperty (helics_property_time_delta, 1.0);
    fedB->setProperty (helics_property_time_delta, 1.0);

    sub.setDefault (3.1);
    double testValue = 4.79;
    double testValue2 = 9.34;
    auto f1finish = std::async (std::launch::async, [&] () { fedA->enterExecutingMode (); });
    fedB->enterExecutingMode ();
    f1finish.wait ();
    // publish string1 at time=0.0;
    pub.publish (testValue);

    std::string val;
    sub.getValue (val);

    BOOST_CHECK_EQUAL (val, "3.100000");

    auto f1time = std::async (std::launch::async, [&] () { return fedA->requestTime (1.0); });
    auto gtime = fedB->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK_EQUAL (f1time.get (), 1.0);
    BOOST_CHECK (sub.isUpdated ());
    // get the value
    sub.getValue (val);
    BOOST_CHECK (!sub.isUpdated ());
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (val, "v1[4.790000]");
    sub.getValue (val);
    BOOST_CHECK (!sub.isUpdated ());
    pub.publish (testValue2);

    f1time = std::async (std::launch::async, [&] () { return fedA->requestTime (2.0); });
    gtime = fedB->requestTime (2.0);
    BOOST_CHECK_EQUAL (f1time.get (), 2.0);
    BOOST_CHECK (sub.isUpdated ());
    // get the value
    sub.getValue (val);
    BOOST_CHECK (!sub.isUpdated ());
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (val, "v1[9.340000]");
    sub.getValue (val);
    BOOST_CHECK (!sub.isUpdated ());
    double v2;
    sub.getValue (v2);
    BOOST_CHECK (!sub.isUpdated ());

    fedA->finalizeAsync ();
    fedB->finalize ();
    fedA->finalizeComplete ();
    helics::cleanupHelicsLibrary ();
}

/** test simple creation and destruction*/
BOOST_AUTO_TEST_CASE (test_single_update_vector_char_ptr)
{
    using namespace helics;
    SetupTest<ValueFederate> ("test", 2);
    auto fedA = GetFederateAs<ValueFederate> (0);
    auto fedB = GetFederateAs<ValueFederate> (1);

    // register the publications
    auto pub = fedA->registerGlobalPublication<std::vector<double>> ("pub1");

    auto sub = fedB->registerSubscription ("pub1");
    fedA->setProperty (helics_property_time_delta, 1.0);
    fedB->setProperty (helics_property_time_delta, 1.0);

    sub.setDefault (3.1);
    double testValue = 4.79;
    double testValue2 = 9.34;
    auto f1finish = std::async (std::launch::async, [&] () { fedA->enterExecutingMode (); });
    fedB->enterExecutingMode ();
    f1finish.wait ();
    // publish string1 at time=0.0;
    pub.publish (testValue);

    std::array<char, 50> val;
    sub.getValue (val.data (), 50);

    BOOST_CHECK_EQUAL (val.data (), "3.100000");

    auto f1time = std::async (std::launch::async, [&] () { return fedA->requestTime (1.0); });
    auto gtime = fedB->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK_EQUAL (f1time.get (), 1.0);
    BOOST_CHECK (sub.isUpdated ());
    // get the value
    sub.getValue (val.data (), 50);
    BOOST_CHECK (!sub.isUpdated ());
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (val.data (), "v1[4.790000]");
    sub.getValue (val.data (), 50);
    BOOST_CHECK (!sub.isUpdated ());
    pub.publish (testValue2);

    f1time = std::async (std::launch::async, [&] () { return fedA->requestTime (2.0); });
    gtime = fedB->requestTime (2.0);
    BOOST_CHECK_EQUAL (f1time.get (), 2.0);
    BOOST_CHECK (sub.isUpdated ());
    // get the value
    sub.getValue (val.data (), 50);
    BOOST_CHECK (!sub.isUpdated ());
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (val.data (), "v1[9.340000]");
    sub.getValue (val.data (), 50);
    BOOST_CHECK (!sub.isUpdated ());
    double v2;
    sub.getValue (v2);
    BOOST_CHECK (!sub.isUpdated ());

    fedA->finalizeAsync ();
    fedB->finalize ();
    fedA->finalizeComplete ();
    helics::cleanupHelicsLibrary ();
}

/** test simple creation and destruction*/
BOOST_AUTO_TEST_CASE (test_single_update_vector_ptr)
{
    using namespace helics;
    SetupTest<ValueFederate> ("test", 2);
    auto fedA = GetFederateAs<ValueFederate> (0);
    auto fedB = GetFederateAs<ValueFederate> (1);

    // register the publications
    auto pub = fedA->registerGlobalPublication<std::vector<double>> ("pub1");

    auto sub = fedB->registerSubscription ("pub1");
    fedA->setProperty (helics_property_time_delta, 1.0);
    fedB->setProperty (helics_property_time_delta, 1.0);

    sub.setDefault (3.1);
    double testValue = 4.79;
    double testValue2 = 9.34;
    auto f1finish = std::async (std::launch::async, [&] () { fedA->enterExecutingMode (); });
    fedB->enterExecutingMode ();
    f1finish.wait ();
    // publish string1 at time=0.0;
    pub.publish (testValue);

    std::vector<double> val (3, 0.0);
    sub.getValue (val.data (), 3);

    BOOST_CHECK_EQUAL (val[0], 3.100000);

    auto f1time = std::async (std::launch::async, [&] () { return fedA->requestTime (1.0); });
    auto gtime = fedB->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK_EQUAL (f1time.get (), 1.0);
    BOOST_CHECK (sub.isUpdated ());
    // get the value
    sub.getValue (val.data (), 3);
    BOOST_CHECK (!sub.isUpdated ());
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (val[0], 4.790000);
    sub.getValue (val.data (), 3);
    BOOST_CHECK (!sub.isUpdated ());
    pub.publish (testValue2);

    f1time = std::async (std::launch::async, [&] () { return fedA->requestTime (2.0); });
    gtime = fedB->requestTime (2.0);
    BOOST_CHECK_EQUAL (f1time.get (), 2.0);
    BOOST_CHECK (sub.isUpdated ());
    // get the value
    sub.getValue (val.data (), 3);
    BOOST_CHECK (!sub.isUpdated ());
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (val[0], 9.340000);
    sub.getValue (val.data (), 50);
    BOOST_CHECK (!sub.isUpdated ());
    double v2;
    sub.getValue (v2);
    BOOST_CHECK (!sub.isUpdated ());

    fedA->finalizeAsync ();
    fedB->finalize ();
    fedA->finalizeComplete ();
    helics::cleanupHelicsLibrary ();
}
BOOST_AUTO_TEST_SUITE_END ()
