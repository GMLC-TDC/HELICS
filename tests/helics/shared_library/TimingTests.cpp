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
#include "ctestFixtures.hpp"
namespace utf = boost::unit_test;

BOOST_FIXTURE_TEST_SUITE (timing_tests, FederateTestFixture, *utf::label("ci"))

/** just a check that in the simple case we do actually get the time back we requested*/
BOOST_AUTO_TEST_CASE (simple_timing_test)
{
    SetupTest (helicsCreateValueFederate, "test", 2);
    auto vFed1 = GetFederateAt (0);
    auto vFed2 = GetFederateAt (1);

	CE (helicsFederateSetPeriod (vFed1, 0.5,0.0));
    CE (helicsFederateSetPeriod (vFed2, 0.5, 0.0));

    auto pub=helicsFederateRegisterGlobalPublication (vFed1, "pub1", "double", "");
    helicsFederateRegisterSubscription (vFed2, "pub1", "", "");
    CE (helicsFederateEnterExecutionModeAsync (vFed1));
    CE (helicsFederateEnterExecutionMode (vFed2));
    CE (helicsFederateEnterExecutionModeComplete (vFed1));
    CE (helicsPublicationPublishDouble (pub, 0.27));
    helics_time_t gtime;
    CE (helicsFederateRequestTime (vFed1, 2.0, &gtime));
    BOOST_CHECK_EQUAL (gtime, 2.0);

	 CE (helicsFederateRequestTime (vFed2, 2.0, &gtime));
    BOOST_CHECK_EQUAL (gtime, 0.5);

   CE (helicsFederateRequestTime (vFed2, 2.0, &gtime));
    BOOST_CHECK_EQUAL (gtime, 2.0);

    CE (helicsFederateFinalize (vFed1));
    CE (helicsFederateFinalize (vFed2));
}

BOOST_AUTO_TEST_CASE (simple_timing_test2)
{
    SetupTest (helicsCreateValueFederate, "test", 2);
    auto vFed1 = GetFederateAt (0);
    auto vFed2 = GetFederateAt (1);

    CE (helicsFederateSetPeriod (vFed1, 0.5, 0.0));
    CE (helicsFederateSetPeriod (vFed1, 0.5, 0.0));

   auto pub = helicsFederateRegisterGlobalPublication (vFed1, "pub1", "double", "");
    helicsFederateRegisterSubscription (vFed2, "pub1", "", "");

    CE (helicsFederateEnterExecutionModeAsync (vFed1));
    CE (helicsFederateEnterExecutionMode (vFed2));
    CE (helicsFederateEnterExecutionModeComplete (vFed1));

	helics_time_t gtime;
	 CE (helicsFederateRequestTime (vFed1, 0.32, &gtime));

    // check that the request is only granted at the appropriate period
    BOOST_CHECK_EQUAL (gtime, 0.5);
     CE (helicsPublicationPublishDouble (pub, 0.27));
    CE (helicsFederateRequestTime (vFed1, 1.85, &gtime));
    BOOST_CHECK_EQUAL (gtime, 2.0);
    CE (helicsFederateRequestTime (vFed2, 1.79, &gtime));
    BOOST_CHECK_EQUAL (gtime, 0.5);  // the result should show up at the next available time point
    CE (helicsFederateRequestTime (vFed2, 2.0, &gtime));
    BOOST_CHECK_EQUAL (gtime, 2.0);

     CE (helicsFederateFinalize (vFed1));
    CE (helicsFederateFinalize (vFed2));
}

BOOST_AUTO_TEST_CASE (simple_timing_test_message)
{
    SetupTest (helicsCreateMessageFederate, "test", 2);
    auto vFed1 = GetFederateAt (0);
    auto vFed2 = GetFederateAt (1);

    CE (helicsFederateSetPeriod (vFed1, 0.6, 0.0));
    CE (helicsFederateSetPeriod (vFed2, 0.45, 0.0));

	auto ept1 = helicsFederateRegisterGlobalEndpoint (vFed1, "e1", "");
    helicsFederateRegisterGlobalEndpoint (vFed2, "e2", "");

    CE (helicsFederateEnterExecutionModeAsync (vFed1));
    CE (helicsFederateEnterExecutionMode (vFed2));
    CE (helicsFederateEnterExecutionModeComplete (vFed1));
    CE (helicsFederateRequestTimeAsync (vFed2, 3.5));

    helics_time_t gtime;
    CE (helicsFederateRequestTime (vFed1, 0.32, &gtime));

    // check that the request is only granted at the appropriate period
    BOOST_CHECK_CLOSE (gtime, 0.6,0.000000001);
    CE (helicsEndpointSendMessageRaw (ept1, "e2", "test1", 5));

    CE (helicsFederateRequestTimeAsync (vFed1, 1.85));

	CE (helicsFederateRequestTimeComplete (vFed2, &gtime));

    BOOST_CHECK_EQUAL (gtime, 0.9);  // the message should show up at the next available time point
    CE (helicsFederateRequestTimeAsync (vFed2, 2.0));
    CE (helicsFederateRequestTimeComplete (vFed2, &gtime));
    BOOST_CHECK_EQUAL (gtime, 2.25);  // the message should show up at the next available time point
    CE (helicsFederateRequestTimeAsync (vFed2, 3.0));
    CE (helicsFederateRequestTimeComplete (vFed1, &gtime));
    BOOST_CHECK_EQUAL (gtime, 2.4);
    CE (helicsFederateFinalize (vFed1));
    CE (helicsFederateFinalize (vFed2));
}

BOOST_AUTO_TEST_CASE (timing_with_input_delay)
{
    SetupTest (helicsCreateMessageFederate, "test", 2);
    auto vFed1 = GetFederateAt (0);
    auto vFed2 = GetFederateAt (1);

    CE (helicsFederateSetPeriod (vFed1, 0.1, 0.0));
    CE (helicsFederateSetPeriod (vFed2, 0.1, 0.0));

	CE (helicsFederateSetInputDelay (vFed2, 0.1));

   auto ept1 = helicsFederateRegisterGlobalEndpoint (vFed1, "e1", "");
    helicsFederateRegisterGlobalEndpoint (vFed2, "e2", "");

    CE (helicsFederateEnterExecutionModeAsync (vFed1));
    CE (helicsFederateEnterExecutionMode (vFed2));
    CE (helicsFederateEnterExecutionModeComplete (vFed1));
    CE (helicsFederateRequestTimeAsync (vFed2, 2.0));
    helics_time_t gtime;
    CE (helicsFederateRequestTime (vFed1, 1.0, &gtime));
    // check that the request is only granted at the appropriate period
    BOOST_CHECK_EQUAL (gtime, 1.0);
    CE (helicsEndpointSendMessageRaw (ept1, "e2", "test1", 5));
    CE (helicsFederateRequestTimeAsync (vFed1, 1.9));
    CE (helicsFederateRequestTimeComplete (vFed2, &gtime));
    BOOST_CHECK_EQUAL (
      gtime, 1.1);  // the message should show up at the next available time point after the impact window
    CE (helicsFederateRequestTimeAsync (vFed2, 2.0));
    CE (helicsFederateRequestTimeComplete (vFed1, &gtime));
    BOOST_CHECK_EQUAL (gtime, 1.9);
    CE (helicsFederateRequestTimeComplete (vFed2, &gtime));
    BOOST_CHECK_EQUAL (gtime, 2.0);
    CE (helicsFederateFinalize (vFed1));
    CE (helicsFederateFinalize (vFed2));
}

BOOST_AUTO_TEST_CASE (timing_with_minDelta_change)
{

	 SetupTest (helicsCreateValueFederate, "test", 1, 1.0);
    auto vFed = GetFederateAt (0);

     CE (helicsFederateEnterExecutionMode (vFed));

    helics_time_t gtime;
     CE (helicsFederateRequestTime (vFed, 1.0, &gtime));
    // check that the request is only granted at the appropriate period

    BOOST_CHECK_EQUAL (gtime, 1.0);

    // purposely requesting 1.0 to test min delta
    CE (helicsFederateRequestTime (vFed, 1.0, &gtime));
    BOOST_CHECK_EQUAL (gtime, 2.0);

	CE (helicsFederateSetTimeDelta (vFed, 0.1));
    CE (helicsFederateRequestTime (vFed, gtime, &gtime));
    BOOST_CHECK_EQUAL (gtime, 2.1);
    CE (helicsFederateFinalize (vFed));
}

BOOST_AUTO_TEST_CASE (timing_with_period_change)
{
   
	 SetupTest (helicsCreateValueFederate, "test",1);
    auto vFed = GetFederateAt (0);

    CE (helicsFederateSetPeriod (vFed, 1.0, 0.0));
    CE (helicsFederateEnterExecutionMode (vFed));

     helics_time_t gtime;
    CE (helicsFederateRequestTime (vFed, 1.0, &gtime));
    // check that the request is only granted at the appropriate period

    BOOST_CHECK_EQUAL (gtime, 1.0);

    // purposely requesting 1.0 to test min delta
    CE (helicsFederateRequestTime (vFed, 1.0, &gtime));
    BOOST_CHECK_EQUAL (gtime, 2.0);

    CE (helicsFederateSetPeriod (vFed, 0.1, 0.0));
    CE (helicsFederateRequestTime (vFed, gtime, &gtime));
    BOOST_CHECK_EQUAL (gtime, 2.1);
    CE (helicsFederateFinalize (vFed));
}

BOOST_AUTO_TEST_SUITE_END ()
