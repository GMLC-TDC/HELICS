/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include <complex>
#include <gtest/gtest.h>
/** these test cases test out the value converters
 */
#include "ctestFixtures.hpp"

struct timing_tests: public FederateTestFixture, public ::testing::Test {};
/** just a check that in the simple case we do actually get the time back we requested*/
TEST_F(timing_tests, simple_timing_test)
{
    SetupTest(helicsCreateValueFederate, "test", 2);
    auto vFed1 = GetFederateAt(0);
    auto vFed2 = GetFederateAt(1);

    CE(helicsFederateSetTimeProperty(vFed1, HELICS_PROPERTY_TIME_PERIOD, 0.5, &err));
    CE(helicsFederateSetTimeProperty(vFed2, HELICS_PROPERTY_TIME_PERIOD, 0.5, &err));

    CE(helicsFederateSetFlagOption(
        vFed1, HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS, HELICS_TRUE, &err));
    CE(helicsFederateSetFlagOption(
        vFed2, HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS, HELICS_TRUE, &err));

    auto pub = helicsFederateRegisterGlobalTypePublication(vFed1, "pub1", "double", "", &err);
    CE(helicsFederateRegisterSubscription(vFed2, "pub1", "", &err));
    CE(helicsFederateEnterExecutingModeAsync(vFed1, &err));
    CE(helicsFederateEnterExecutingMode(vFed2, &err));
    CE(helicsFederateEnterExecutingModeComplete(vFed1, &err));
    CE(helicsPublicationPublishDouble(pub, 0.27, &err));
    HelicsTime gtime;
    CE(gtime = helicsFederateRequestTime(vFed1, 2.0, &err));
    EXPECT_EQ(gtime, 2.0);

    CE(gtime = helicsFederateRequestTime(vFed2, 2.0, &err));
    EXPECT_EQ(gtime, 0.5);

    CE(gtime = helicsFederateRequestTime(vFed2, 2.0, &err));
    EXPECT_EQ(gtime, 2.0);

    CE(helicsFederateFinalize(vFed1, &err));
    CE(helicsFederateFinalize(vFed2, &err));
}

TEST_F(timing_tests, simple_timing_test2)
{
    SetupTest(helicsCreateValueFederate, "test", 2);
    auto vFed1 = GetFederateAt(0);
    auto vFed2 = GetFederateAt(1);

    CE(helicsFederateSetFlagOption(
        vFed1, HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS, HELICS_TRUE, &err));
    CE(helicsFederateSetFlagOption(
        vFed2, HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS, HELICS_TRUE, &err));

    CE(helicsFederateSetTimeProperty(vFed1, HELICS_PROPERTY_TIME_PERIOD, 0.5, &err));
    CE(helicsFederateSetTimeProperty(vFed2, HELICS_PROPERTY_TIME_PERIOD, 0.5, &err));

    auto pub = helicsFederateRegisterGlobalPublication(
        vFed1, "pub1", HELICS_DATA_TYPE_DOUBLE, "", nullptr);
    helicsFederateRegisterSubscription(vFed2, "pub1", "", nullptr);

    CE(helicsFederateEnterExecutingModeAsync(vFed1, &err));
    CE(helicsFederateEnterExecutingMode(vFed2, &err));
    CE(helicsFederateEnterExecutingModeComplete(vFed1, &err));

    HelicsTime gtime;
    CE(gtime = helicsFederateRequestTime(vFed1, 0.32, &err));

    // check that the request is only granted at the appropriate period
    EXPECT_EQ(gtime, 0.5);
    CE(helicsPublicationPublishDouble(pub, 0.27, &err));
    CE(gtime = helicsFederateRequestTime(vFed1, 1.85, &err));
    EXPECT_EQ(gtime, 2.0);
    CE(gtime = helicsFederateRequestTime(vFed2, 1.79, &err));
    EXPECT_EQ(gtime, 0.5);  // the result should show up at the next available time point
    CE(gtime = helicsFederateRequestTime(vFed2, 2.0, &err));
    EXPECT_EQ(gtime, 2.0);
    CE(helicsFederateFinalize(vFed1, &err));
    // just test the next step function with no others
    CE(gtime = helicsFederateRequestNextStep(vFed2, &err));
    EXPECT_EQ(gtime, 2.5);

    CE(helicsFederateFinalize(vFed2, &err));
}

TEST_F(timing_tests, simple_timing_test_message)
{
    SetupTest(helicsCreateMessageFederate, "test", 2);
    auto vFed1 = GetFederateAt(0);
    auto vFed2 = GetFederateAt(1);

    CE(helicsFederateSetTimeProperty(vFed1, HELICS_PROPERTY_TIME_PERIOD, 0.6, &err));
    CE(helicsFederateSetTimeProperty(vFed2, HELICS_PROPERTY_TIME_PERIOD, 0.45, &err));

    CE(helicsFederateSetFlagOption(
        vFed1, HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS, HELICS_TRUE, &err));
    CE(helicsFederateSetFlagOption(
        vFed2, HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS, HELICS_TRUE, &err));

    auto ept1 = helicsFederateRegisterGlobalEndpoint(vFed1, "e1", "", &err);
    helicsFederateRegisterGlobalEndpoint(vFed2, "e2", "", &err);
    ASSERT_EQ(err.error_code, 0);
    CE(helicsFederateEnterExecutingModeAsync(vFed1, &err));
    CE(helicsFederateEnterExecutingMode(vFed2, &err));
    CE(helicsFederateEnterExecutingModeComplete(vFed1, &err));
    CE(helicsFederateRequestTimeAsync(vFed2, 3.5, &err));

    HelicsTime gtime;
    CE(gtime = helicsFederateRequestTime(vFed1, 0.32, &err));

    // check that the request is only granted at the appropriate period
    EXPECT_NEAR(gtime, 0.6, 0.000000001);
    CE(helicsEndpointSendBytesTo(ept1, "test1", 5, "e2", &err));

    CE(helicsFederateRequestTimeAsync(vFed1, 1.85, &err));

    CE(gtime = helicsFederateRequestTimeComplete(vFed2, &err));

    EXPECT_EQ(gtime, 0.9);  // the message should show up at the next available time point
    CE(helicsFederateRequestTimeAsync(vFed2, 2.0, &err));
    CE(gtime = helicsFederateRequestTimeComplete(vFed2, &err));
    EXPECT_EQ(gtime, 2.25);  // the message should show up at the next available time point
    CE(helicsFederateRequestTimeAsync(vFed2, 3.0, &err));
    CE(gtime = helicsFederateRequestTimeComplete(vFed1, &err));
    EXPECT_EQ(gtime, 2.4);
    CE(helicsFederateFinalize(vFed1, &err));
    CE(helicsFederateFinalize(vFed2, &err));
}

TEST_F(timing_tests, timing_with_input_delay)
{
    SetupTest(helicsCreateMessageFederate, "test", 2);
    auto vFed1 = GetFederateAt(0);
    auto vFed2 = GetFederateAt(1);

    CE(helicsFederateSetTimeProperty(vFed1, HELICS_PROPERTY_TIME_PERIOD, 0.1, &err));
    CE(helicsFederateSetTimeProperty(vFed2, HELICS_PROPERTY_TIME_PERIOD, 0.1, &err));

    CE(helicsFederateSetTimeProperty(vFed2, HELICS_PROPERTY_TIME_INPUT_DELAY, 0.1, &err));

    CE(helicsFederateSetFlagOption(
        vFed1, HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS, HELICS_TRUE, &err));
    CE(helicsFederateSetFlagOption(
        vFed2, HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS, HELICS_TRUE, &err));

    auto ept1 = helicsFederateRegisterGlobalEndpoint(vFed1, "e1", "", nullptr);
    helicsFederateRegisterGlobalEndpoint(vFed2, "e2", "", nullptr);

    CE(helicsFederateEnterExecutingModeAsync(vFed1, &err));
    CE(helicsFederateEnterExecutingMode(vFed2, &err));
    CE(helicsFederateEnterExecutingModeComplete(vFed1, &err));
    CE(helicsFederateRequestTimeAsync(vFed2, 2.0, &err));
    HelicsTime gtime;
    CE(gtime = helicsFederateRequestTime(vFed1, 1.0, &err));
    // check that the request is only granted at the appropriate period
    EXPECT_EQ(gtime, 1.0);
    CE(helicsEndpointSendBytesTo(ept1, "test1", 5, "e2", &err));
    CE(helicsFederateRequestTimeAsync(vFed1, 1.9, &err));
    CE(gtime = helicsFederateRequestTimeComplete(vFed2, &err));
    EXPECT_DOUBLE_EQ(gtime,
                     1.1);  // the message should show up at the next available time point after the
                            // impact window
    CE(helicsFederateRequestTimeAsync(vFed2, 2.0, &err));
    CE(gtime = helicsFederateRequestTimeComplete(vFed1, &err));
    EXPECT_DOUBLE_EQ(gtime, 1.9);

    CE(auto tres = helicsFederateGetTimeProperty(vFed1, HELICS_PROPERTY_TIME_PERIOD, &err));
    EXPECT_DOUBLE_EQ(tres, 0.1);

    CE(gtime = helicsFederateRequestTimeComplete(vFed2, &err));
    EXPECT_DOUBLE_EQ(gtime, 2.0);
    CE(helicsFederateFinalize(vFed1, &err));
    CE(helicsFederateFinalize(vFed2, &err));
}

TEST_F(timing_tests, timing_with_minDelta_change)
{
    SetupTest(helicsCreateValueFederate, "test", 1, 1.0);
    auto vFed = GetFederateAt(0);

    CE(helicsFederateEnterExecutingMode(vFed, &err));

    HelicsTime gtime;
    CE(gtime = helicsFederateRequestTime(vFed, 1.0, &err));
    // check that the request is only granted at the appropriate period

    EXPECT_EQ(gtime, 1.0);

    // purposely requesting 1.0 to test min delta
    CE(gtime = helicsFederateRequestTime(vFed, 1.0, &err));
    EXPECT_EQ(gtime, 2.0);

    CE(helicsFederateSetTimeProperty(vFed, HELICS_PROPERTY_TIME_DELTA, 0.1, &err));
    CE(gtime = helicsFederateRequestTime(vFed, gtime, &err));
    EXPECT_EQ(gtime, 2.1);
    CE(helicsFederateFinalize(vFed, &err));
}

TEST_F(timing_tests, timing_with_period_change)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed = GetFederateAt(0);

    CE(helicsFederateSetFlagOption(
        vFed, HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS, HELICS_TRUE, &err));
    CE(helicsFederateSetTimeProperty(vFed, HELICS_PROPERTY_TIME_PERIOD, 1.0, &err));
    CE(helicsFederateEnterExecutingMode(vFed, &err));

    HelicsTime gtime;
    CE(gtime = helicsFederateRequestTime(vFed, 1.0, &err));
    // check that the request is only granted at the appropriate period

    EXPECT_EQ(gtime, 1.0);
    CE(auto val =
           helicsFederateGetFlagOption(vFed, HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS, &err));

    EXPECT_EQ(val, HELICS_TRUE);

    // purposely requesting 1.0 to test min delta
    CE(gtime = helicsFederateRequestTime(vFed, 1.0, &err));
    EXPECT_EQ(gtime, 2.0);

    CE(helicsFederateSetTimeProperty(vFed, HELICS_PROPERTY_TIME_PERIOD, 0.1, &err));
    CE(gtime = helicsFederateRequestTime(vFed, gtime, &err));
    EXPECT_EQ(gtime, 2.1);
    CE(helicsFederateFinalize(vFed, &err));
}

TEST_F(timing_tests, max_time_consistency)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed = GetFederateAt(0);
    CE(helicsFederateEnterExecutingMode(vFed, &err));
    HelicsTime gtime;
    CE(gtime = helicsFederateRequestTime(vFed, HELICS_TIME_MAXTIME, &err));
    EXPECT_GE(gtime, HELICS_TIME_MAXTIME);
    CE(helicsFederateFinalize(vFed, &err));
    CE(gtime = helicsFederateGetCurrentTime(vFed, &err));
    EXPECT_GE(gtime, HELICS_TIME_MAXTIME);
}
