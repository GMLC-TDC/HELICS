/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <complex>

/** these test cases test out the value converters
 */
#include "helics/helics.hpp"
#include "testFixtures.hpp"
#include <future>
BOOST_FIXTURE_TEST_SUITE (timing_tests2, FederateTestFixture)



/** just a check that in the simple case we do actually get the time back we requested*/


BOOST_AUTO_TEST_CASE (small_time_test)
{
    SetupTest<helics::ValueFederate> ("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

    auto pub1_a = helics::make_publication<double> (helics::GLOBAL, vFed1, "pub1_a");
    auto pub1_b = helics::make_publication<double>(helics::GLOBAL, vFed1, "pub1_b");
    auto pub2_a = helics::make_publication<double>(helics::GLOBAL, vFed2, "pub2_a");
    auto pub2_b = helics::make_publication<double>(helics::GLOBAL, vFed2, "pub2_b");
    
    
    auto sub1_a = helics::Subscription( vFed2, "pub1_a");
    auto sub1_b = helics::Subscription( vFed2, "pub1_b");
    auto sub2_a = helics::Subscription( vFed1, "pub2_a");
    auto sub2_b = helics::Subscription( vFed1, "pub2_b");
    vFed1->enterExecutionStateAsync ();
    vFed2->enterExecutionState ();
    vFed1->enterExecutionStateComplete ();
    auto echoRun = [&]() {helics::Time grantedTime = helics::timeZero;
    helics::Time stopTime(100, timeUnits::ns);
    while (grantedTime < stopTime)
    {
        grantedTime = vFed2->requestTime(stopTime);
        if (sub1_a.isUpdated())
        {
            auto val = sub1_a.getValue<double>();
            pub2_a->publish(val);
        }
        if (sub1_b.isUpdated())
        {
            auto val = sub1_b.getValue<double>();
            pub2_b->publish(val);
        }
    }};
    
    auto fut = std::async(echoRun);
    helics::Time grantedTime = helics::timeZero;
    helics::Time requestedTime(10, timeUnits::ns);
    helics::Time stopTime(100, timeUnits::ns);
    while (grantedTime < stopTime)
    {
        grantedTime=vFed1->requestTime(requestedTime);
        if (grantedTime == requestedTime)
        {
           
            pub1_a->publish(10.3);
            pub1_b->publish(11.2);
            requestedTime += helics::Time(10, timeUnits::ns);
        }
        else
        {
            BOOST_CHECK(grantedTime == requestedTime - helics::Time(9, timeUnits::ns));
            printf("grantedTime=%e\n", static_cast<double>(grantedTime));
            if (sub2_a.isUpdated())
            {
                BOOST_CHECK_EQUAL(sub2_a.getValue<double>(), 10.3);
            }
            if (sub2_b.isUpdated())
            {
                BOOST_CHECK_EQUAL(sub2_b.getValue<double>(), 11.2);
            }
        }
    }
    vFed1->finalize ();
    fut.get();
    vFed2->finalize ();
}


BOOST_AUTO_TEST_SUITE_END ()
