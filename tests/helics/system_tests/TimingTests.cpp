/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include <complex>
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

/** these test cases test out the value converters
 */
#include "../application_api/testFixtures.hpp"
#include "helics/helics.hpp"

namespace utf = boost::unit_test;

BOOST_FIXTURE_TEST_SUITE (timing_tests, FederateTestFixture)

/** just a check that in the simple case we do actually get the time back we requested*/
BOOST_AUTO_TEST_CASE (simple_timing_test, *utf::label ("ci"))
{
    SetupTest<helics::ValueFederate> ("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

    vFed1->setTimeProperty (PERIOD_PROPERTY, 0.5);
    vFed2->setTimeProperty (PERIOD_PROPERTY, 0.5);

    auto pub = helics::make_publication<double> (helics::GLOBAL, vFed1.get (), "pub1");
    vFed2->registerSubscription( "pub1");
    vFed1->enterExecutingModeAsync ();
    vFed2->enterExecutingMode ();
    vFed1->enterExecutingModeComplete ();
    pub->publish (0.27);
    auto res = vFed1->requestTime (2.0);
    BOOST_CHECK_EQUAL (res, 2.0);

    res = vFed2->requestTime (2.0);
    BOOST_CHECK_EQUAL (res, 0.5);  // the result should show up at the next available time point
    res = vFed2->requestTime (2.0);
    BOOST_CHECK_EQUAL (res, 2.0);

    vFed1->finalize ();
    vFed2->finalize ();
}

BOOST_AUTO_TEST_CASE (simple_timing_test2, *utf::label ("ci"))
{
    SetupTest<helics::ValueFederate> ("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

    vFed1->setTimeProperty (PERIOD_PROPERTY, 0.5);
    vFed2->setTimeProperty (PERIOD_PROPERTY, 0.5);

    auto pub = helics::make_publication<double> (helics::GLOBAL, vFed1.get (), "pub1");
    vFed2->registerSubscription("pub1");
    vFed1->enterExecutingModeAsync ();
    vFed2->enterExecutingMode ();
    vFed1->enterExecutingModeComplete ();

    auto res = vFed1->requestTime (0.32);
    // check that the request is only granted at the appropriate period
    BOOST_CHECK_EQUAL (res, 0.5);
    pub->publish (0.27);
    res = vFed1->requestTime (1.85);
    BOOST_CHECK_EQUAL (res, 2.0);
    res = vFed2->requestTime (1.79);
    BOOST_CHECK_EQUAL (res, 0.5);  // the result should show up at the next available time point
    res = vFed2->requestTime (2.0);
    BOOST_CHECK_EQUAL (res, 2.0);

    vFed1->finalize ();
    vFed2->finalize ();
}

BOOST_AUTO_TEST_CASE (simple_timing_test_message, *utf::label ("ci"))
{
    SetupTest<helics::MessageFederate> ("test", 2);
    auto vFed1 = GetFederateAs<helics::MessageFederate> (0);
    auto vFed2 = GetFederateAs<helics::MessageFederate> (1);

    vFed1->setTimeProperty (PERIOD_PROPERTY, 0.6);
    vFed2->setTimeProperty (PERIOD_PROPERTY, 0.45);

    auto &ept1 = vFed1->registerGlobalEndpoint ("e1");
     vFed2->registerGlobalEndpoint ("e2");
    vFed1->enterExecutingModeAsync ();
    vFed2->enterExecutingMode ();
    vFed1->enterExecutingModeComplete ();
    vFed2->requestTimeAsync (3.5);
    auto res = vFed1->requestTime (0.32);
    // check that the request is only granted at the appropriate period
    BOOST_CHECK_EQUAL (res, 0.6);
    ept1.send ("e2", "test1");
    vFed1->requestTimeAsync (1.85);
    res = vFed2->requestTimeComplete ();
    BOOST_CHECK_EQUAL (res, 0.9);  // the message should show up at the next available time point
    vFed2->requestTimeAsync (2.0);
    res = vFed2->requestTimeComplete ();
    BOOST_CHECK_EQUAL (res, 2.25);  // the message should show up at the next available time point
    vFed2->requestTimeAsync (3.0);
    res = vFed1->requestTimeComplete ();
    BOOST_CHECK_EQUAL (res, 2.4);
    vFed1->finalize ();
    vFed2
      ->finalize ();  // this will also test finalizing while a time request is ongoing otherwise it will time out.
}

BOOST_AUTO_TEST_CASE (timing_with_input_delay, *utf::label ("ci"))
{
    SetupTest<helics::MessageFederate> ("test", 2);
    auto vFed1 = GetFederateAs<helics::MessageFederate> (0);
    auto vFed2 = GetFederateAs<helics::MessageFederate> (1);

    vFed1->setTimeProperty (PERIOD_PROPERTY, 0.1);
    vFed2->setTimeProperty (PERIOD_PROPERTY, 0.1);
    vFed2->setTimeProperty (INPUT_DELAY_PROPERTY, 0.1);

    auto &ept1 = vFed1->registerGlobalEndpoint ("e1");
    vFed2->registerGlobalEndpoint ("e2");
    vFed1->enterExecutingModeAsync ();
    vFed2->enterExecutingMode ();
    vFed1->enterExecutingModeComplete ();
    vFed2->requestTimeAsync (2.0);
    auto res = vFed1->requestTime (1.0);
    // check that the request is only granted at the appropriate period
    BOOST_CHECK_EQUAL (res, 1.0);
    ept1.send ("e2", "test1");
    vFed1->requestTimeAsync (1.9);
    res = vFed2->requestTimeComplete ();
    BOOST_CHECK_EQUAL (
      res, 1.1);  // the message should show up at the next available time point after the impact window
    vFed2->requestTimeAsync (2.0);
    res = vFed1->requestTimeComplete ();
    BOOST_CHECK_EQUAL (res, 1.9);
    res = vFed2->requestTimeComplete ();
    BOOST_CHECK_EQUAL (res, 2.0);
    vFed1->finalize ();
    vFed2
      ->finalize ();  // this will also test finalizing while a time request is ongoing otherwise it will time out.
}

BOOST_AUTO_TEST_CASE (timing_with_minDelta_change, *utf::label ("ci"))
{
    SetupTest<helics::ValueFederate> ("test", 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    vFed1->enterExecutingMode ();

    auto res = vFed1->requestTime (1.0);
    // check that the request is only granted at the appropriate period

    BOOST_CHECK_EQUAL (res, 1.0);

    // purposely requesting 1.0 to test min delta
    res = vFed1->requestTime (1.0);
    BOOST_CHECK_EQUAL (res, 2.0);

    vFed1->setTimeProperty (TIME_DELTA_PROPERTY, 0.1);
    res = vFed1->requestTime (res);
    BOOST_CHECK_EQUAL (res, 2.1);
    vFed1->finalize ();
}

BOOST_AUTO_TEST_CASE (timing_with_period_change, *utf::label ("ci"))
{
    SetupTest<helics::ValueFederate> ("test", 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    vFed1->setTimeProperty (PERIOD_PROPERTY, 1.0);
    vFed1->enterExecutingMode ();

    auto res = vFed1->requestTime (1.0);
    // check that the request is only granted at the appropriate period

    BOOST_CHECK_EQUAL (res, 1.0);

    // purposely requesting 1.0 to test min delta
    res = vFed1->requestTime (1.0);
    BOOST_CHECK_EQUAL (res, 2.0);

    vFed1->setTimeProperty (PERIOD_PROPERTY, 0.1);
    res = vFed1->requestTime (res);
    BOOST_CHECK_EQUAL (res, 2.1);
    vFed1->finalize ();
}

BOOST_AUTO_TEST_CASE (sender_finalize_timing_result, *utf::label ("ci"))
{
    SetupTest<helics::ValueFederate> ("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

    helics::Publication sender (helics::interface_visibility::global, vFed1, "pub",
                                helics::helics_type_t::helicsDouble);
   auto &receiver=vFed2->registerSubscription("pub");
    vFed1->enterExecutingModeAsync ();
    vFed2->enterExecutingMode ();
    vFed1->enterExecutingModeComplete ();
    auto granted1 = vFed1->requestTime (1.0);
    BOOST_CHECK_EQUAL (granted1, 1.0);
    sender.publish (1.0);

    granted1 = vFed1->requestTime (2.0);
    BOOST_CHECK_EQUAL (granted1, 2.0);
    // now check that the receiver got the data at time 1.0
    auto granted2 = vFed2->requestTime (400.0);  // request a big time
    BOOST_CHECK_EQUAL (granted2, 1.0);
    BOOST_CHECK (receiver.isUpdated ());
    BOOST_CHECK_EQUAL (receiver.getValue<double> (), 1.0);

    // now do 2 publish cycles in a row
    sender.publish (2.0);
    granted1 = vFed1->requestTime (3.0);
    BOOST_CHECK_EQUAL (granted1, 3.0);

    sender.publish (3.0);
    granted1 = vFed1->requestTime (4.0);
    BOOST_CHECK_EQUAL (granted1, 4.0);
    sender.publish (4.0);

    // check the grant at time 2.0
    granted2 = vFed2->requestTime (400.0);  // request a big time
    BOOST_CHECK_EQUAL (granted2, 2.0);
    BOOST_CHECK (receiver.isUpdated ());
    BOOST_CHECK_EQUAL (receiver.getValue<double> (), 2.0);

    granted1 = vFed1->requestTime (6.0);
    BOOST_CHECK_EQUAL (granted1, 6.0);
    sender.publish (6.0);

    vFed1->finalize ();
    // check the grant at time 2.0
    granted2 = vFed2->requestTime (400.0);  // request a big time
    BOOST_CHECK_EQUAL (granted2, 3.0);
    BOOST_CHECK (receiver.isUpdated ());
    BOOST_CHECK_EQUAL (receiver.getValue<double> (), 3.0);
    // check the grant at time 2.0
    granted2 = vFed2->requestTime (400.0);  // request a big time
    BOOST_CHECK_EQUAL (granted2, 4.0);
    BOOST_CHECK (receiver.isUpdated ());
    BOOST_CHECK_EQUAL (receiver.getValue<double> (), 4.0);
    // check the grant at time 2.0
    granted2 = vFed2->requestTime (400.0);  // request a big time
    BOOST_CHECK_EQUAL (granted2, 6.0);
    BOOST_CHECK (receiver.isUpdated ());
    BOOST_CHECK_EQUAL (receiver.getValue<double> (), 6.0);
    vFed2->finalize ();
}

BOOST_AUTO_TEST_CASE (sender_finalize_timing_result2, *utf::label ("ci"))
{
    SetupTest<helics::ValueFederate> ("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

    helics::Publication sender (helics::interface_visibility::global, vFed1, "pub",
                                helics::helics_type_t::helicsDouble);
    auto &receiver = vFed2->registerSubscription ("pub");
    vFed1->enterExecutingModeAsync ();
    vFed2->enterExecutingMode ();
    vFed1->enterExecutingModeComplete ();
    auto granted1 = vFed1->requestTime (1.0);
    BOOST_CHECK_EQUAL (granted1, 1.0);
    sender.publish (1.0);

    granted1 = vFed1->requestTime (2.0);
    BOOST_CHECK_EQUAL (granted1, 2.0);
    // now check that the receiver got the data at time 1.0
    auto granted2 = vFed2->requestTime (400.0);  // request a big time
    BOOST_CHECK_EQUAL (granted2, 1.0);
    BOOST_CHECK (receiver.isUpdated ());
    BOOST_CHECK_EQUAL (receiver.getValue<double> (), 1.0);

    // now do 2 publish cycles in a row
    sender.publish (2.0);
    granted1 = vFed1->requestTime (3.0);
    BOOST_CHECK_EQUAL (granted1, 3.0);

    sender.publish (3.0);
    granted1 = vFed1->requestTime (4.0);
    BOOST_CHECK_EQUAL (granted1, 4.0);
    sender.publish (4.0);

    // check the grant at time 2.0
    granted2 = vFed2->requestTime (400.0);  // request a big time
    BOOST_CHECK_EQUAL (granted2, 2.0);
    BOOST_CHECK (receiver.isUpdated ());
    BOOST_CHECK_EQUAL (receiver.getValue<double> (), 2.0);

    granted1 = vFed1->requestTime (6.0);
    BOOST_CHECK_EQUAL (granted1, 6.0);
    sender.publish (6.0);

    vFed1->finalize ();
    // check the grant at time 2.0
    granted2 = vFed2->requestTime (400.0);  // request a big time
    BOOST_CHECK_EQUAL (granted2, 3.0);
    BOOST_CHECK (receiver.isUpdated ());
    BOOST_CHECK_EQUAL (receiver.getValue<double> (), 3.0);
    // check the grant at time 2.0
    granted2 = vFed2->requestTime (400.0);  // request a big time
    BOOST_CHECK_EQUAL (granted2, 4.0);
    BOOST_CHECK (receiver.isUpdated ());
    BOOST_CHECK_EQUAL (receiver.getValue<double> (), 4.0);
    // check the grant at time 2.0
    granted2 = vFed2->requestTime (5.0);  // request time of 5
    BOOST_CHECK_EQUAL (granted2, 5.0);
    BOOST_CHECK (!receiver.isUpdated ());  // should not have an update
    BOOST_CHECK_EQUAL (receiver.getValue<double> (), 4.0);  // the get value should be the previous value

    granted2 = vFed2->requestTime (400.0);  // request a big time
    BOOST_CHECK_EQUAL (granted2, 6.0);
    BOOST_CHECK (receiver.isUpdated ());
    BOOST_CHECK_EQUAL (receiver.getValue<double> (), 6.0);
    vFed2->finalize ();
}

BOOST_AUTO_TEST_CASE (fast_sender_tests)
{
    SetupTest<helics::ValueFederate> ("zmq_2", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

    helics::Publication sender (helics::interface_visibility::global, vFed1, "pub",
                                helics::helics_type_t::helicsDouble);
    auto &receiver = vFed2->registerSubscription ("pub");
    vFed1->enterExecutingModeAsync ();
    vFed2->enterExecutingMode ();
    vFed1->enterExecutingModeComplete ();
    const helics::Time endTime (8000.0);
    helics::Time currentTime = 0.0;
    while (currentTime <= endTime)
    {
        currentTime += 1.0;
        currentTime = vFed1->requestTime (currentTime);
        sender.publish (static_cast<double> (currentTime));
    }
    vFed1->finalize ();
    currentTime = 0.0;
    while (currentTime <= endTime)
    {
        currentTime = vFed2->requestTime (endTime + 2000.0);
        if (receiver.isUpdated ())
        {
            double val = receiver.getValue<double> ();
            BOOST_CHECK_EQUAL (val, static_cast<double> (currentTime));
        }
    }
    vFed2->finalize ();
}

BOOST_AUTO_TEST_CASE (dual_fast_sender_tests)
{
    SetupTest<helics::ValueFederate> ("zmq_2", 3);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);
    auto vFed3 = GetFederateAs<helics::ValueFederate> (2);
    helics::Publication sender1 (helics::interface_visibility::global, vFed1, "pub1",
                                 helics::helics_type_t::helicsDouble);
    auto &receiver1 = vFed2->registerSubscription ("pub1");
    helics::Publication sender2 (helics::interface_visibility::global, vFed3, "pub2",
                                 helics::helics_type_t::helicsDouble);
    auto &receiver2 = vFed2->registerSubscription ("pub2");
    vFed1->enterExecutingModeAsync ();
    vFed3->enterExecutingModeAsync ();
    vFed2->enterExecutingMode ();
    vFed1->enterExecutingModeComplete ();
    vFed3->enterExecutingModeComplete ();
    const helics::Time endTime (500.0);
    helics::Time currentTime = helics::timeZero;
    while (currentTime <= endTime)
    {
        currentTime += 1.0;
        currentTime = vFed1->requestTime (currentTime);
        sender1.publish (static_cast<double> (currentTime));
    }
    vFed1->finalize ();
    currentTime = helics::timeZero;
    while (currentTime <= endTime)
    {
        currentTime += 1.0;
        currentTime = vFed3->requestTime (currentTime);
        sender2.publish (static_cast<double> (currentTime));
    }
    vFed3->finalize ();
    currentTime = helics::timeZero;
    while (currentTime <= endTime)
    {
        currentTime = vFed2->requestTime (endTime + 2000.0);
        if (receiver1.isUpdated ())
        {
            double val = receiver1.getValue<double> ();
            BOOST_CHECK_EQUAL (val, static_cast<double> (currentTime));
        }
        if (receiver2.isUpdated ())
        {
            double val = receiver2.getValue<double> ();
            BOOST_CHECK_EQUAL (val, static_cast<double> (currentTime));
        }
    }
    vFed2->finalize ();
}
BOOST_AUTO_TEST_SUITE_END ()
