/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "testFixtures.h"
#include <complex>
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

/** these test cases test out the value converters
 */
#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/application_api/ValueConverter.hpp"

#if ENABLE_TEST_TIMEOUTS > 0
namespace utf = boost::unit_test;
#endif

BOOST_FIXTURE_TEST_SUITE (iteration_tests, ValueFederateTestFixture)

/** just a check that in the simple case we do actually get the time back we requested*/
#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (5))
#endif
BOOST_AUTO_TEST_CASE (execution_iteration_test)
{
    Setup1FederateTest ("test");
    // register the publications
    auto pubid = vFed1->registerGlobalPublication<double> ("pub1");

    auto subid = vFed1->registerRequiredSubscription<double> ("pub1");
    vFed1->setTimeDelta (1.0);
    vFed1->enterInitializationState ();
    vFed1->publish (pubid, 27.0);

    auto comp = vFed1->enterExecutionState (helics::iteration_request::iterate_if_needed);

    BOOST_CHECK (comp == helics::iteration_result::iterating);
    auto val = vFed1->getValue<double> (subid);
    BOOST_CHECK_EQUAL (val, 27.0);

    comp = vFed1->enterExecutionState (helics::iteration_request::iterate_if_needed);

    BOOST_CHECK (comp == helics::iteration_result::next_step);

    auto val2 = vFed1->getValue<double> (subid);

    BOOST_CHECK_EQUAL (val2, val);
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (5))
#endif
BOOST_AUTO_TEST_CASE (execution_iteration_test_2fed)
{
    Setup2FederateTest ("test");
    // register the publications
    auto pubid = vFed1->registerGlobalPublication<double> ("pub1");

    auto subid = vFed2->registerRequiredSubscription<double> ("pub1");
    vFed1->setTimeDelta (1.0);
    vFed2->setTimeDelta (1.0);

    vFed1->enterInitializationStateAsync ();
    vFed2->enterInitializationState ();
    vFed1->enterInitializationStateComplete ();
    vFed1->publish (pubid, 27.0);
    vFed1->enterExecutionStateAsync ();
    auto comp = vFed2->enterExecutionState (helics::iteration_request::iterate_if_needed);

    BOOST_CHECK (comp == helics::iteration_result::iterating);
    auto val = vFed2->getValue<double> (subid);
    BOOST_CHECK_EQUAL (val, 27.0);

    comp = vFed2->enterExecutionState (helics::iteration_request::iterate_if_needed);

    BOOST_CHECK (comp == helics::iteration_result::next_step);

    auto val2 = vFed2->getValue<double> (subid);
    vFed1->enterExecutionStateComplete();
    BOOST_CHECK_EQUAL (val2, val);
}

/** just a check that in the simple case we do actually get the time back we requested*/
#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (5))
#endif
BOOST_AUTO_TEST_CASE (time_iteration_test)
{
    Setup1FederateTest ("test");
    // register the publications
    auto pubid = vFed1->registerGlobalPublication<double> ("pub1");

    auto subid = vFed1->registerRequiredSubscription<double> ("pub1");
    vFed1->setPeriod (1.0);
    vFed1->setTimeDelta (1.0);
    vFed1->enterExecutionState ();
    vFed1->publish (pubid, 27.0);

    auto comp = vFed1->requestTimeIterative (1.0, helics::iteration_request::iterate_if_needed);

    BOOST_CHECK (comp.state == helics::iteration_result::iterating);
    BOOST_CHECK_EQUAL (comp.stepTime, helics::timeZero);
    auto val = vFed1->getValue<double> (subid);
    BOOST_CHECK_EQUAL (val, 27.0);

    comp = vFed1->requestTimeIterative (1.0, helics::iteration_request::iterate_if_needed);

    BOOST_CHECK (comp.state == helics::iteration_result::next_step);
    BOOST_CHECK_EQUAL (comp.stepTime, 1.0);
    auto val2 = vFed1->getValue<double> (subid);

    BOOST_CHECK_EQUAL (val2, val);
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (5))
#endif
BOOST_AUTO_TEST_CASE (time_iteration_test_2fed)
{
    Setup2FederateTest ("test");
    // register the publications
    auto pubid = vFed1->registerGlobalPublication<double> ("pub1");

    auto subid = vFed2->registerRequiredSubscription<double> ("pub1");
    vFed1->setTimeDelta (1.0);
    vFed2->setTimeDelta (1.0);
    vFed1->setPeriod (1.0);
    vFed2->setPeriod (1.0);

    vFed1->enterExecutionStateAsync ();
    vFed2->enterExecutionState ();
    vFed1->enterExecutionStateComplete();
    vFed1->publish (pubid, 27.0);

    vFed1->requestTimeAsync (1.0);
    auto comp = vFed2->requestTimeIterative (1.0, helics::iteration_request::iterate_if_needed);

    BOOST_CHECK (comp.state == helics::iteration_result::iterating);
    BOOST_CHECK_EQUAL (comp.stepTime, helics::timeZero);
    auto val = vFed2->getValue<double> (subid);
    BOOST_CHECK_EQUAL (val, 27.0);

    comp = vFed2->requestTimeIterative (1.0, helics::iteration_request::iterate_if_needed);

    BOOST_CHECK (comp.state == helics::iteration_result::next_step);
    BOOST_CHECK_EQUAL (comp.stepTime, 1.0);
    auto val2 = vFed2->getValue<double> (subid);
    vFed1->requestTimeComplete ();

    BOOST_CHECK_EQUAL (val2, val);
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (5))
#endif
BOOST_AUTO_TEST_CASE (test2fed_withSubPub)
{
    Setup2FederateTest ("test");
    // register the publications
    auto pub1 = helics::Publication (helics::GLOBAL, vFed1.get (), "pub1", helics::helicsType_t::helicsDouble);

    auto sub1 = helics::Subscription (vFed2.get (), "pub1");
    vFed1->setTimeDelta (1.0);
    vFed2->setTimeDelta (1.0);
    vFed1->setPeriod (1.0);
    vFed2->setPeriod (1.0);

    vFed1->enterExecutionStateAsync ();
    vFed2->enterExecutionState ();
    vFed1->enterExecutionStateComplete();
    pub1.publish (27.0);

    vFed1->requestTimeAsync (1.0);
    auto comp = vFed2->requestTimeIterative (1.0, helics::iteration_request::iterate_if_needed);

    BOOST_CHECK (comp.state == helics::iteration_result::iterating);
    BOOST_CHECK_EQUAL (comp.stepTime, helics::timeZero);

    BOOST_CHECK (sub1.isUpdated ());
    auto val = sub1.getValue<double> ();
    BOOST_CHECK_EQUAL (val, 27.0);
    BOOST_CHECK (!sub1.isUpdated ());
    comp = vFed2->requestTimeIterative (1.0, helics::iteration_request::iterate_if_needed);

    BOOST_CHECK (comp.state == helics::iteration_result::next_step);
    BOOST_CHECK_EQUAL (comp.stepTime, 1.0);
    BOOST_CHECK (!sub1.isUpdated ());
    auto val2 = sub1.getValue<double> ();
    vFed1->requestTimeComplete ();

    BOOST_CHECK_EQUAL (val2, val);
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (5))
#endif
BOOST_AUTO_TEST_CASE (test_iteration_counter)
{
    Setup2FederateTest ("test");
    // register the publications
    auto pub1 = helics::Publication (helics::GLOBAL, vFed1.get (), "pub1", helics::helicsType_t::helicsInt);

    auto sub1 = helics::Subscription (vFed2.get (), "pub1");

    auto pub2 = helics::Publication (helics::GLOBAL, vFed2.get (), "pub2", helics::helicsType_t::helicsInt);

    auto sub2 = helics::Subscription (vFed1.get (), "pub2");
    vFed1->setPeriod (1.0);
    vFed2->setPeriod (1.0);
    // vFed1->setLoggingLevel(5);
    // vFed2->setLoggingLevel(5);
    vFed1->enterInitializationStateAsync ();
    vFed2->enterInitializationState ();
    vFed1->enterInitializationStateComplete ();
    int64_t c1 = 0;
    int64_t c2 = 0;
    pub1.publish (c1);
    pub2.publish (c2);
    vFed1->enterExecutionStateAsync ();
    vFed2->enterExecutionState ();
    vFed1->enterExecutionStateComplete();
    while (c1 <= 10)
    {
        BOOST_CHECK_EQUAL (sub1.getValue<int64_t> (), c1);
        BOOST_CHECK_EQUAL (sub2.getValue<int64_t> (), c2);
        ++c1;
        ++c2;
        if (c1 <= 10)
        {
            pub1.publish (c1);
            pub2.publish (c2);
        }

        vFed1->requestTimeIterativeAsync (1.0, helics::iteration_request::iterate_if_needed);
        auto res = vFed2->requestTimeIterative (1.0, helics::iteration_request::iterate_if_needed);
        if (c1 <= 10)
        {
            BOOST_CHECK (res.state == helics::iteration_result::iterating);
            BOOST_CHECK_EQUAL (res.stepTime, 0.0);
        }
        else
        {
            BOOST_CHECK (res.state == helics::iteration_result::next_step);
            BOOST_CHECK_EQUAL (res.stepTime, 1.0);
        }
        res = vFed1->requestTimeIterativeComplete ();
        if (c1 <= 10)
        {
            BOOST_CHECK (res.state == helics::iteration_result::iterating);
            BOOST_CHECK_EQUAL (res.stepTime, 0.0);
        }
        else
        {
            BOOST_CHECK (res.state == helics::iteration_result::next_step);
            BOOST_CHECK_EQUAL (res.stepTime, 1.0);
        }
    }
}
BOOST_AUTO_TEST_SUITE_END ()
