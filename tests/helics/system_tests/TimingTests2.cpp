/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "gtest/gtest.h"

#include <complex>

/** these test cases test out the value converters
 */
#include "../application_api/testFixtures.hpp"
#include "helics/helics.hpp"
#include <future>

struct timing_tests2 : public FederateTestFixture, public ::testing::Test
{
};
/** just a check that in the simple case we do actually get the time back we requested*/

TEST_F (timing_tests2, small_time_test)
{
    SetupTest<helics::ValueFederate> ("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

    auto pub1_a = helics::make_publication<double> (helics::GLOBAL, vFed1, "pub1_a");
    auto pub1_b = helics::make_publication<double> (helics::GLOBAL, vFed1, "pub1_b");
    auto pub2_a = helics::make_publication<double> (helics::GLOBAL, vFed2, "pub2_a");
    auto pub2_b = helics::make_publication<double> (helics::GLOBAL, vFed2, "pub2_b");

    auto sub1_a = vFed2->registerSubscription ("pub1_a");
    auto sub1_b = vFed2->registerSubscription ("pub1_b");
    auto sub2_a = vFed2->registerSubscription ("pub2_a");
    auto sub2_b = vFed2->registerSubscription ("pub2_b");
    vFed1->enterExecutingModeAsync ();
    vFed2->enterExecutingMode ();
    vFed1->enterExecutingModeComplete ();
    auto echoRun = [&] () {
        helics::Time grantedTime = helics::timeZero;
        helics::Time stopTime (100, time_units::ns);
        while (grantedTime < stopTime)
        {
            grantedTime = vFed2->requestTime (stopTime);
            if (sub1_a.isUpdated ())
            {
                auto val = sub1_a.getValue<double> ();
                pub2_a->publish (val);
            }
            if (sub1_b.isUpdated ())
            {
                auto val = sub1_b.getValue<double> ();
                pub2_b->publish (val);
            }
        }
    };

    auto fut = std::async (echoRun);
    helics::Time grantedTime = helics::timeZero;
    helics::Time requestedTime (10, time_units::ns);
    helics::Time stopTime (100, time_units::ns);
    while (grantedTime < stopTime)
    {
        grantedTime = vFed1->requestTime (requestedTime);
        if (grantedTime == requestedTime)
        {
            pub1_a->publish (10.3);
            pub1_b->publish (11.2);
            requestedTime += helics::Time (10, time_units::ns);
        }
        else
        {
            EXPECT_TRUE (grantedTime == requestedTime - helics::Time (9, time_units::ns));
            // printf("grantedTime=%e\n", static_cast<double>(grantedTime));
            if (sub2_a.isUpdated ())
            {
                EXPECT_EQ (sub2_a.getValue<double> (), 10.3);
            }
            if (sub2_b.isUpdated ())
            {
                EXPECT_EQ (sub2_b.getValue<double> (), 11.2);
            }
        }
    }
    vFed1->finalize ();
    fut.get ();
    vFed2->finalize ();
}

/** this test requires a major change to the timing subsystem
TEST_F(timing_tests2,ring_test3)
{
    SetupTest<helics::ValueFederate>("test_2", 3);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    auto vFed3 = GetFederateAs<helics::ValueFederate>(2);


    auto pub1 = helics::make_publication<double>(helics::GLOBAL, vFed1, "pub1");
    auto pub2 = helics::make_publication<double>(helics::GLOBAL, vFed2, "pub2");
    auto pub3 = helics::make_publication<double>(helics::GLOBAL, vFed3, "pub3");
    auto sub1 = vFed1->registerSubscription ("pub3");
    auto sub2 = vFed2->registerSubscription ("pub1");
    auto sub3 = vFed3->registerSubscription ("pub2");
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingModeAsync();
    vFed3->enterExecutingMode();
    vFed1->enterExecutingModeComplete();
    vFed2->enterExecutingModeComplete();
    pub1->publish(45.7);
    vFed1->requestTimeAsync(50.0);
    vFed2->requestTimeAsync(50.0);
    vFed3->requestTimeAsync(50.0);

    auto newTime = vFed2->requestTimeComplete();
    EXPECT_EQ(newTime, 1e-9);
    EXPECT_TRUE(sub2.isUpdated());
    double val = sub2.getValue<double>();
    pub2->publish(val);
    vFed2->requestTimeAsync(50.0);
    newTime = vFed3->requestTimeComplete();
    EXPECT_EQ(newTime, 1e-9);
    EXPECT_TRUE(sub3.isUpdated());
    val = sub3.getValue<double>();
    pub3->publish(val);
    vFed3->requestTimeAsync(50.0);
    newTime = vFed1->requestTimeComplete();
    EXPECT_EQ(newTime, 1e-9);
    EXPECT_TRUE(sub1.isUpdated());
    val = sub1.getValue<double>();
    pub1->publish(val);
    vFed1->requestTimeAsync(50.0);
    // round 2 for time
    newTime = vFed2->requestTimeComplete();
    EXPECT_EQ(newTime, 2e-9);
    EXPECT_TRUE(sub2.isUpdated());
    val = sub2.getValue<double>();
    pub2->publish(val);
    vFed2->requestTimeAsync(50.0);
    newTime = vFed3->requestTimeComplete();
    EXPECT_EQ(newTime, 2e-9);
    EXPECT_TRUE(sub3.isUpdated());
    val = sub3.getValue<double>();
    pub3->publish(val);
    vFed3->requestTimeAsync(50.0);
    newTime = vFed1->requestTimeComplete();
    EXPECT_EQ(newTime, 2e-9);
    EXPECT_TRUE(sub1.isUpdated());
    val = sub1.getValue<double>();
    pub1->publish(val);
    vFed1->requestTimeAsync(50.0);
    // round 3
    newTime = vFed2->requestTimeComplete();
    EXPECT_EQ(newTime, 3e-9);
    EXPECT_TRUE(sub2.isUpdated());
    val = sub2.getValue<double>();
    vFed2->finalize();
    newTime = vFed3->requestTimeComplete();
    EXPECT_EQ(newTime, 50.0);
    EXPECT_TRUE(!sub3.isUpdated());
    val = sub3.getValue<double>();
    vFed3->finalize();
    newTime = vFed1->requestTimeComplete();
    EXPECT_EQ(newTime, 50.0);
    EXPECT_TRUE(!sub1.isUpdated());
    vFed1->finalize();
}

*/
