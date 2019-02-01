/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. 
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../application_api/testFixtures.hpp"
#include <complex>
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/floating_point_comparison.hpp>

/** these test cases test out the value converters
 */
#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/application_api/ValueConverter.hpp"
#include <future>

namespace bdata = boost::unit_test::data;
namespace utf = boost::unit_test;

BOOST_FIXTURE_TEST_SUITE (iteration_tests, FederateTestFixture)

/** just a check that in the simple case we do actually get the time back we requested*/

BOOST_TEST_DECORATOR (*utf::label ("ci"))
BOOST_AUTO_TEST_CASE (execution_iteration_test)
{
    SetupTest<helics::ValueFederate> ("test", 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    // register the publications
    auto &pubid = vFed1->registerGlobalPublication<double> ("pub1");

    auto &subid = vFed1->registerSubscription ("pub1");
    vFed1->setProperty (helics_property_time_delta, 1.0);
    vFed1->enterInitializingMode ();
    vFed1->publish (pubid, 27.0);

    auto comp = vFed1->enterExecutingMode (helics::iteration_request::iterate_if_needed);

    BOOST_CHECK (comp == helics::iteration_result::iterating);
    auto val = subid.getValue<double> ();
    BOOST_CHECK_EQUAL (val, 27.0);

    comp = vFed1->enterExecutingMode (helics::iteration_request::iterate_if_needed);

    BOOST_CHECK (comp == helics::iteration_result::next_step);

    auto val2 = subid.getValue<double> ();

    BOOST_CHECK_EQUAL (val2, val);
}

std::pair<double, int> runInitIterations (helics::ValueFederate *vfed, int index, int total)
{
    using namespace helics;
    Publication pub (vfed, "pub", data_type::helics_double);
    pub.setMinimumChange (0.001);
    std::string low_target = "fed";
    low_target += std::to_string ((index == 0) ? total - 1 : index - 1);
    low_target += "/pub";
    std::string high_target = "fed";
    high_target += std::to_string ((index == total - 1) ? (0) : index + 1);
    high_target += "/pub";
    auto &sub_low = vfed->registerSubscription (low_target);
    auto &sub_high = vfed->registerSubscription (high_target);
    sub_low.setDefault (static_cast<double> (2 * index));
    sub_high.setDefault (static_cast<double> (2 * index + 1));
    vfed->enterInitializingMode ();
    auto cval = static_cast<double> (2 * index) + 0.5;

    auto itres = iteration_result::iterating;
    int itcount = 0;
    while (itres == iteration_result::iterating)
    {
        pub.publish (cval);
        itres = vfed->enterExecutingMode (iteration_request::iterate_if_needed);
        auto val1 = sub_high.getValue<double> ();
        auto val2 = sub_low.getValue<double> ();
        cval = (val1 + val2) / 2.0;
        ++itcount;
        //   printf("[%d]<%d> (%d)=%f,(%d)=%f, curr=%f\n", itcount,index, (index == 0) ? total - 1 : index -
        //   1,val2, (index == total - 1) ? (0) : index + 1, val1, cval);
    }
    return {cval, itcount};
}

std::vector<std::pair<double, int>>
run_iteration_round_robin (std::vector<std::shared_ptr<helics::ValueFederate>> &fedVec)
{
    auto N = static_cast<int> (fedVec.size ());
    std::vector<std::future<std::pair<double, int>>> futures;
    for (decltype (N) ii = 0; ii < N; ++ii)
    {
        auto vFed = fedVec[ii].get ();
        futures.push_back (
          std::async (std::launch::async, [vFed, ii, N]() { return runInitIterations (vFed, ii, N); }));
    }
    std::vector<std::pair<double, int>> results (N);
    for (decltype (N) ii = 0; ii < N; ++ii)
    {
        results[ii] = futures[ii].get ();
    }
    return results;
}

BOOST_DATA_TEST_CASE (execution_iteration_round_robin, bdata::make (core_types_all), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 3);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0).get ();
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1).get ();
    auto vFed3 = GetFederateAs<helics::ValueFederate> (2).get ();
    auto fut1 = std::async (std::launch::async, [vFed1]() { return runInitIterations (vFed1, 0, 3); });
    auto fut2 = std::async (std::launch::async, [vFed2]() { return runInitIterations (vFed2, 1, 3); });

    auto res3 = runInitIterations (vFed3, 2, 3);
    auto res2 = fut2.get ();
    auto res1 = fut1.get ();
    BOOST_CHECK_CLOSE (res3.first, 2.5, 0.1);
    BOOST_CHECK_CLOSE (res2.first, 2.5, 0.1);
    BOOST_CHECK_CLOSE (res1.first, 2.5, 0.1);
}

BOOST_AUTO_TEST_CASE (execution_iteration_loop3, *utf::label ("ci"))
{
    int N = 5;
    SetupTest<helics::ValueFederate> ("test", N);
    std::vector<std::shared_ptr<helics::ValueFederate>> vfeds (N);
    for (int ii = 0; ii < N; ++ii)
    {
        vfeds[ii] = GetFederateAs<helics::ValueFederate> (ii);
    }
    auto results = run_iteration_round_robin (vfeds);
    for (int ii = 1; ii < N; ++ii)
    {
        if (results[ii].second < 50)
        {
            BOOST_CHECK_CLOSE (results[ii].first, results[0].first, 0.1);
        }
    }
}

BOOST_TEST_DECORATOR (*utf::label ("ci"))
BOOST_AUTO_TEST_CASE (execution_iteration_test_2fed)
{
    SetupTest<helics::ValueFederate> ("test", 2, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);
    // register the publications
    auto pubid = vFed1->registerGlobalPublication<double> ("pub1");

    auto subid = vFed2->registerSubscription ("pub1");

    vFed1->enterInitializingModeAsync ();
    vFed2->enterInitializingMode ();
    vFed1->enterInitializingModeComplete ();
    vFed1->publish (pubid, 27.0);
    vFed1->enterExecutingModeAsync ();
    auto comp = vFed2->enterExecutingMode (helics::iteration_request::iterate_if_needed);

    BOOST_CHECK (comp == helics::iteration_result::iterating);
    auto val = vFed2->getDouble (subid);
    BOOST_CHECK_EQUAL (val, 27.0);

    comp = vFed2->enterExecutingMode (helics::iteration_request::iterate_if_needed);

    BOOST_CHECK (comp == helics::iteration_result::next_step);

    auto val2 = vFed2->getDouble (subid);
    vFed1->enterExecutingModeComplete ();
    BOOST_CHECK_EQUAL (val2, val);
}

/** just a check that in the simple case we do actually get the time back we requested*/

BOOST_TEST_DECORATOR (*utf::label ("ci"))
BOOST_AUTO_TEST_CASE (time_iteration_test)
{
    SetupTest<helics::ValueFederate> ("test", 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    // register the publications
    auto pubid = vFed1->registerGlobalPublication<double> ("pub1");

    auto subid = vFed1->registerSubscription ("pub1");
    vFed1->setProperty (helics_property_time_period, 1.0);
    vFed1->setProperty (helics_property_time_delta, 1.0);
    vFed1->enterExecutingMode ();
    vFed1->publish (pubid, 27.0);

    auto comp = vFed1->requestTimeIterative (1.0, helics::iteration_request::iterate_if_needed);

    BOOST_CHECK (comp.state == helics::iteration_result::iterating);
    BOOST_CHECK_EQUAL (comp.grantedTime, helics::timeZero);
    auto val = vFed1->getDouble (subid);
    BOOST_CHECK_EQUAL (val, 27.0);

    comp = vFed1->requestTimeIterative (1.0, helics::iteration_request::iterate_if_needed);

    BOOST_CHECK (comp.state == helics::iteration_result::next_step);
    BOOST_CHECK_EQUAL (comp.grantedTime, 1.0);
    auto val2 = vFed1->getDouble (subid);

    BOOST_CHECK_EQUAL (val2, val);
}

BOOST_AUTO_TEST_CASE (time_iteration_test_2fed)
{
    SetupTest<helics::ValueFederate> ("test", 2, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);
    // register the publications
    auto pubid = vFed1->registerGlobalPublication<double> ("pub1");

    auto subid = vFed2->registerSubscription ("pub1");

    vFed1->setProperty (helics_property_time_period, 1);
    vFed2->setProperty (helics_property_time_period, 1.0);

    vFed1->enterExecutingModeAsync ();
    vFed2->enterExecutingMode ();
    vFed1->enterExecutingModeComplete ();
    vFed1->publish (pubid, 27.0);

    vFed1->requestTimeAsync (1.0);
    auto comp = vFed2->requestTimeIterative (1.0, helics::iteration_request::iterate_if_needed);

    BOOST_CHECK (comp.state == helics::iteration_result::iterating);
    BOOST_CHECK_EQUAL (comp.grantedTime, helics::timeZero);
    auto val = vFed2->getDouble (subid);
    BOOST_CHECK_EQUAL (val, 27.0);

    comp = vFed2->requestTimeIterative (1.0, helics::iteration_request::iterate_if_needed);

    BOOST_CHECK (comp.state == helics::iteration_result::next_step);
    BOOST_CHECK_EQUAL (comp.grantedTime, 1.0);
    auto val2 = vFed2->getDouble (subid);
    vFed1->requestTimeComplete ();

    BOOST_CHECK_EQUAL (val2, val);
}

BOOST_TEST_DECORATOR (*utf::label ("ci"))
BOOST_AUTO_TEST_CASE (test2fed_withSubPub)
{
    SetupTest<helics::ValueFederate> ("test", 2, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);
    // register the publications
    auto pub1 = helics::Publication (helics::GLOBAL, vFed1.get (), "pub1", helics::data_type::helics_double);

    auto &sub1 = vFed2->registerSubscription ("pub1");
    vFed1->setProperty (helics_property_time_delta, 1.0);
    vFed2->setProperty (helics_property_time_delta, 1.0);
    vFed1->setProperty (helics_property_time_period, 1.0);
    vFed2->setProperty (helics_property_time_period, 1.0);

    vFed1->enterExecutingModeAsync ();
    vFed2->enterExecutingMode ();
    vFed1->enterExecutingModeComplete ();
    pub1.publish (27.0);

    vFed1->requestTimeAsync (1.0);
    auto comp = vFed2->requestTimeIterative (1.0, helics::iteration_request::iterate_if_needed);

    BOOST_CHECK (comp.state == helics::iteration_result::iterating);
    BOOST_CHECK_EQUAL (comp.grantedTime, helics::timeZero);

    BOOST_CHECK (sub1.isUpdated ());
    auto val = sub1.getValue<double> ();
    BOOST_CHECK_EQUAL (val, 27.0);
    BOOST_CHECK (!sub1.isUpdated ());
    comp = vFed2->requestTimeIterative (1.0, helics::iteration_request::iterate_if_needed);

    BOOST_CHECK (comp.state == helics::iteration_result::next_step);
    BOOST_CHECK_EQUAL (comp.grantedTime, 1.0);
    BOOST_CHECK (!sub1.isUpdated ());
    auto val2 = sub1.getValue<double> ();
    vFed1->requestTimeComplete ();

    BOOST_CHECK_EQUAL (val2, val);
}

BOOST_TEST_DECORATOR (*utf::label ("ci"))
BOOST_AUTO_TEST_CASE (test_iteration_counter)
{
    SetupTest<helics::ValueFederate> ("test", 2, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);
    // register the publications
    auto pub1 = helics::Publication (helics::GLOBAL, vFed1.get (), "pub1", helics::data_type::helics_int);

    auto &sub1 = vFed2->registerSubscription ("pub1");

    auto pub2 = helics::Publication (helics::GLOBAL, vFed2.get (), "pub2", helics::data_type::helics_int);

    auto &sub2 = vFed1->registerSubscription ("pub2");
    vFed1->setProperty (helics_property_time_period, 1.0);
    vFed2->setProperty (helics_property_time_period, 1.0);
    // vFed1->setLoggingLevel(5);
    // vFed2->setLoggingLevel(5);
    vFed1->enterInitializingModeAsync ();
    vFed2->enterInitializingMode ();
    vFed1->enterInitializingModeComplete ();
    int64_t c1 = 0;
    int64_t c2 = 0;
    pub1.publish (c1);
    pub2.publish (c2);
    vFed1->enterExecutingModeAsync ();
    vFed2->enterExecutingMode ();
    vFed1->enterExecutingModeComplete ();
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
            BOOST_CHECK_EQUAL (res.grantedTime, 0.0);
        }
        else
        {
            BOOST_CHECK (res.state == helics::iteration_result::next_step);
            BOOST_CHECK_EQUAL (res.grantedTime, 1.0);
        }
        res = vFed1->requestTimeIterativeComplete ();
        if (c1 <= 10)
        {
            BOOST_CHECK (res.state == helics::iteration_result::iterating);
            BOOST_CHECK_EQUAL (res.grantedTime, 0.0);
        }
        else
        {
            BOOST_CHECK (res.state == helics::iteration_result::next_step);
            BOOST_CHECK_EQUAL (res.grantedTime, 1.0);
        }
    }
}
BOOST_AUTO_TEST_SUITE_END ()
