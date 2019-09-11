/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include <boost/test/unit_test.hpp>

#include <complex>

/** these test cases test out the value converters
 */
#include "ctestFixtures.hpp"
#include "helics/helics.hpp"
namespace utf = boost::unit_test;

BOOST_FIXTURE_TEST_SUITE (timing_tests, FederateTestFixture, *utf::label ("ci"))

/** just a check that in the simple case we do actually get the time back we requested*/
BOOST_AUTO_TEST_CASE (simple_timing_test)
{
    SetupTest (helicsCreateValueFederate, "test", 2);
    auto vFed1 = GetFederateAt (0);
    auto vFed2 = GetFederateAt (1);

    CE (helicsFederateSetTimeProperty (vFed1, helics_property_time_period, 0.5, &err));
    CE (helicsFederateSetTimeProperty (vFed2, helics_property_time_period, 0.5, &err));

    CE (helicsFederateSetFlagOption (vFed1, helics_flag_ignore_time_mismatch_warnings, helics_true, &err));
    CE (helicsFederateSetFlagOption (vFed2, helics_flag_ignore_time_mismatch_warnings, helics_true, &err));

    auto pub = helicsFederateRegisterGlobalTypePublication (vFed1, "pub1", "double", "", &err);
    CE (helicsFederateRegisterSubscription (vFed2, "pub1", "", &err));
    CE (helicsFederateEnterExecutingModeAsync (vFed1, &err));
    CE (helicsFederateEnterExecutingMode (vFed2, &err));
    CE (helicsFederateEnterExecutingModeComplete (vFed1, &err));
    CE (helicsPublicationPublishDouble (pub, 0.27, &err));
    helics_time gtime;
    CE (gtime = helicsFederateRequestTime (vFed1, 2.0, &err));
    BOOST_CHECK_EQUAL (gtime, 2.0);

    CE (gtime = helicsFederateRequestTime (vFed2, 2.0, &err));
    BOOST_CHECK_EQUAL (gtime, 0.5);

    CE (gtime = helicsFederateRequestTime (vFed2, 2.0, &err));
    BOOST_CHECK_EQUAL (gtime, 2.0);

    CE (helicsFederateFinalize (vFed1, &err));
    CE (helicsFederateFinalize (vFed2, &err));
}

BOOST_AUTO_TEST_CASE (simple_timing_test2)
{
    SetupTest (helicsCreateValueFederate, "test", 2);
    auto vFed1 = GetFederateAt (0);
    auto vFed2 = GetFederateAt (1);

    CE (helicsFederateSetFlagOption (vFed1, helics_flag_ignore_time_mismatch_warnings, helics_true, &err));
    CE (helicsFederateSetFlagOption (vFed2, helics_flag_ignore_time_mismatch_warnings, helics_true, &err));

    CE (helicsFederateSetTimeProperty (vFed1, helics_property_time_period, 0.5, &err));
    CE (helicsFederateSetTimeProperty (vFed2, helics_property_time_period, 0.5, &err));

    auto pub = helicsFederateRegisterGlobalPublication (vFed1, "pub1", helics_data_type_double, "", nullptr);
    helicsFederateRegisterSubscription (vFed2, "pub1", "", nullptr);

    CE (helicsFederateEnterExecutingModeAsync (vFed1, &err));
    CE (helicsFederateEnterExecutingMode (vFed2, &err));
    CE (helicsFederateEnterExecutingModeComplete (vFed1, &err));

    helics_time gtime;
    CE (gtime = helicsFederateRequestTime (vFed1, 0.32, &err));

    // check that the request is only granted at the appropriate period
    BOOST_CHECK_EQUAL (gtime, 0.5);
    CE (helicsPublicationPublishDouble (pub, 0.27, &err));
    CE (gtime = helicsFederateRequestTime (vFed1, 1.85, &err));
    BOOST_CHECK_EQUAL (gtime, 2.0);
    CE (gtime = helicsFederateRequestTime (vFed2, 1.79, &err));
    BOOST_CHECK_EQUAL (gtime, 0.5);  // the result should show up at the next available time point
    CE (gtime = helicsFederateRequestTime (vFed2, 2.0, &err));
    BOOST_CHECK_EQUAL (gtime, 2.0);
    CE (helicsFederateFinalize (vFed1, &err));
    // just test the next step function with no others
    CE (gtime = helicsFederateRequestNextStep (vFed2, &err));
    BOOST_CHECK_EQUAL (gtime, 2.5);

    CE (helicsFederateFinalize (vFed2, &err));
}

BOOST_AUTO_TEST_CASE (simple_timing_test_message)
{
    SetupTest (helicsCreateMessageFederate, "test", 2);
    auto vFed1 = GetFederateAt (0);
    auto vFed2 = GetFederateAt (1);

    CE (helicsFederateSetTimeProperty (vFed1, helics_property_time_period, 0.6, &err));
    CE (helicsFederateSetTimeProperty (vFed2, helics_property_time_period, 0.45, &err));

    CE (helicsFederateSetFlagOption (vFed1, helics_flag_ignore_time_mismatch_warnings, helics_true, &err));
    CE (helicsFederateSetFlagOption (vFed2, helics_flag_ignore_time_mismatch_warnings, helics_true, &err));

    auto ept1 = helicsFederateRegisterGlobalEndpoint (vFed1, "e1", "", &err);
    helicsFederateRegisterGlobalEndpoint (vFed2, "e2", "", &err);
    BOOST_REQUIRE_EQUAL (err.error_code, 0);
    CE (helicsFederateEnterExecutingModeAsync (vFed1, &err));
    CE (helicsFederateEnterExecutingMode (vFed2, &err));
    CE (helicsFederateEnterExecutingModeComplete (vFed1, &err));
    CE (helicsFederateRequestTimeAsync (vFed2, 3.5, &err));

    helics_time gtime;
    CE (gtime = helicsFederateRequestTime (vFed1, 0.32, &err));

    // check that the request is only granted at the appropriate period
    BOOST_CHECK_CLOSE (gtime, 0.6, 0.000000001);
    CE (helicsEndpointSendMessageRaw (ept1, "e2", "test1", 5, &err));

    CE (helicsFederateRequestTimeAsync (vFed1, 1.85, &err));

    CE (gtime = helicsFederateRequestTimeComplete (vFed2, &err));

    BOOST_CHECK_EQUAL (gtime, 0.9);  // the message should show up at the next available time point
    CE (helicsFederateRequestTimeAsync (vFed2, 2.0, &err));
    CE (gtime = helicsFederateRequestTimeComplete (vFed2, &err));
    BOOST_CHECK_EQUAL (gtime, 2.25);  // the message should show up at the next available time point
    CE (helicsFederateRequestTimeAsync (vFed2, 3.0, &err));
    CE (gtime = helicsFederateRequestTimeComplete (vFed1, &err));
    BOOST_CHECK_EQUAL (gtime, 2.4);
    CE (helicsFederateFinalize (vFed1, &err));
    CE (helicsFederateFinalize (vFed2, &err));
}

BOOST_AUTO_TEST_CASE (timing_with_input_delay)
{
    SetupTest (helicsCreateMessageFederate, "test", 2);
    auto vFed1 = GetFederateAt (0);
    auto vFed2 = GetFederateAt (1);

    CE (helicsFederateSetTimeProperty (vFed1, helics_property_time_period, 0.1, &err));
    CE (helicsFederateSetTimeProperty (vFed2, helics_property_time_period, 0.1, &err));

    CE (helicsFederateSetTimeProperty (vFed2, helics_property_time_input_delay, 0.1, &err));

    CE (helicsFederateSetFlagOption (vFed1, helics_flag_ignore_time_mismatch_warnings, helics_true, &err));
    CE (helicsFederateSetFlagOption (vFed2, helics_flag_ignore_time_mismatch_warnings, helics_true, &err));

    auto ept1 = helicsFederateRegisterGlobalEndpoint (vFed1, "e1", "", nullptr);
    helicsFederateRegisterGlobalEndpoint (vFed2, "e2", "", nullptr);

    CE (helicsFederateEnterExecutingModeAsync (vFed1, &err));
    CE (helicsFederateEnterExecutingMode (vFed2, &err));
    CE (helicsFederateEnterExecutingModeComplete (vFed1, &err));
    CE (helicsFederateRequestTimeAsync (vFed2, 2.0, &err));
    helics_time gtime;
    CE (gtime = helicsFederateRequestTime (vFed1, 1.0, &err));
    // check that the request is only granted at the appropriate period
    BOOST_CHECK_EQUAL (gtime, 1.0);
    CE (helicsEndpointSendMessageRaw (ept1, "e2", "test1", 5, &err));
    CE (helicsFederateRequestTimeAsync (vFed1, 1.9, &err));
    CE (gtime = helicsFederateRequestTimeComplete (vFed2, &err));
    BOOST_CHECK_EQUAL (
      gtime, 1.1);  // the message should show up at the next available time point after the impact window
    CE (helicsFederateRequestTimeAsync (vFed2, 2.0, &err));
    CE (gtime = helicsFederateRequestTimeComplete (vFed1, &err));
    BOOST_CHECK_EQUAL (gtime, 1.9);
    CE (gtime = helicsFederateRequestTimeComplete (vFed2, &err));
    BOOST_CHECK_EQUAL (gtime, 2.0);
    CE (helicsFederateFinalize (vFed1, &err));
    CE (helicsFederateFinalize (vFed2, &err));
}

BOOST_AUTO_TEST_CASE (timing_with_minDelta_change)
{
    SetupTest (helicsCreateValueFederate, "test", 1, 1.0);
    auto vFed = GetFederateAt (0);

    CE (helicsFederateEnterExecutingMode (vFed, &err));

    helics_time gtime;
    CE (gtime = helicsFederateRequestTime (vFed, 1.0, &err));
    // check that the request is only granted at the appropriate period

    BOOST_CHECK_EQUAL (gtime, 1.0);

    // purposely requesting 1.0 to test min delta
    CE (gtime = helicsFederateRequestTime (vFed, 1.0, &err));
    BOOST_CHECK_EQUAL (gtime, 2.0);

    CE (helicsFederateSetTimeProperty (vFed, helics_property_time_delta, 0.1, &err));
    CE (gtime = helicsFederateRequestTime (vFed, gtime, &err));
    BOOST_CHECK_EQUAL (gtime, 2.1);
    CE (helicsFederateFinalize (vFed, &err));
}

BOOST_AUTO_TEST_CASE (timing_with_period_change)
{
    SetupTest (helicsCreateValueFederate, "test", 1);
    auto vFed = GetFederateAt (0);

    CE (helicsFederateSetFlagOption (vFed, helics_flag_ignore_time_mismatch_warnings, helics_true, &err));
    CE (helicsFederateSetTimeProperty (vFed, helics_property_time_period, 1.0, &err));
    CE (helicsFederateEnterExecutingMode (vFed, &err));

    helics_time gtime;
    CE (gtime = helicsFederateRequestTime (vFed, 1.0, &err));
    // check that the request is only granted at the appropriate period

    BOOST_CHECK_EQUAL (gtime, 1.0);

    // purposely requesting 1.0 to test min delta
    CE (gtime = helicsFederateRequestTime (vFed, 1.0, &err));
    BOOST_CHECK_EQUAL (gtime, 2.0);

    CE (helicsFederateSetTimeProperty (vFed, helics_property_time_period, 0.1, &err));
    CE (gtime = helicsFederateRequestTime (vFed, gtime, &err));
    BOOST_CHECK_EQUAL (gtime, 2.1);
    CE (helicsFederateFinalize (vFed, &err));
}

BOOST_AUTO_TEST_CASE (max_time_consistency)
{
    SetupTest (helicsCreateValueFederate, "test", 1);
    auto vFed = GetFederateAt (0);
    CE (helicsFederateEnterExecutingMode (vFed, &err));
    helics_time gtime;
    CE (gtime = helicsFederateRequestTime (vFed, helics_time_maxtime, &err));
    BOOST_CHECK_GE (gtime, helics_time_maxtime);
    CE (helicsFederateFinalize (vFed, &err));
    CE (gtime = helicsFederateGetCurrentTime (vFed, &err));
    BOOST_CHECK_GE (gtime, helics_time_maxtime);
}
BOOST_AUTO_TEST_SUITE_END ()
