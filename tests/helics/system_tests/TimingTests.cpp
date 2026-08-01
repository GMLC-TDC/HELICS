/*
Copyright (c) 2017-2026,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Energy
Innovation LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"

#include "gtest/gtest.h"
#include <chrono>
#include <complex>
#include <future>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

/** these test cases test out the value converters
 */
#include "../application_api/testFixtures.hpp"
#include "helics/helics-config.h"
#include "helics/helics.hpp"

struct timing: public FederateTestFixture, public ::testing::Test {};

/** just a check that in the simple case we do actually get the time back we requested*/
TEST_F(timing, simple_timing)
{
    SetupTest<helics::ValueFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    vFed1->setProperty(HELICS_PROPERTY_TIME_PERIOD, 0.5);
    vFed2->setProperty(HELICS_PROPERTY_TIME_PERIOD, 0.5);
    auto& pub = vFed1->registerGlobalPublication<double>("pub1");
    vFed2->registerSubscription("pub1");
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();
    pub.publish(0.27);
    auto res = vFed1->requestTime(2.0);
    EXPECT_EQ(res, 2.0);

    res = vFed2->requestTime(2.0);
    EXPECT_EQ(res, 0.5);  // the result should show up at the next available time point
    res = vFed2->requestTime(2.0);
    EXPECT_EQ(res, 2.0);

    vFed1->finalize();
    vFed2->finalize();
}

TEST_F(timing, simple_timing2)
{
    SetupTest<helics::ValueFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    vFed1->setFlagOption(HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS);
    vFed2->setFlagOption(HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS);
    vFed1->setProperty(HELICS_PROPERTY_TIME_PERIOD, 0.5);
    vFed2->setProperty(HELICS_PROPERTY_TIME_PERIOD, 0.5);

    auto& pub = vFed1->registerGlobalPublication<double>("pub1");
    vFed2->registerSubscription("pub1");
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();

    auto res = vFed1->requestTime(0.32);
    // check that the request is only granted at the appropriate period
    EXPECT_EQ(res, 0.5);
    pub.publish(0.27);
    res = vFed1->requestTime(1.85);
    EXPECT_EQ(res, 2.0);
    res = vFed2->requestTime(1.79);
    EXPECT_EQ(res, 0.5);  // the result should show up at the next available time point
    res = vFed2->requestTime(2.0);
    EXPECT_EQ(res, 2.0);

    vFed1->finalize();
    vFed2->finalize();
}

TEST_F(timing, simple_timing_message)
{
    SetupTest<helics::MessageFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto vFed2 = GetFederateAs<helics::MessageFederate>(1);

    vFed1->setProperty(HELICS_PROPERTY_TIME_PERIOD, 0.6);
    vFed2->setProperty(HELICS_PROPERTY_TIME_PERIOD, 0.45);
    vFed1->setFlagOption(HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS);
    vFed2->setFlagOption(HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS);
    auto& ept1 = vFed1->registerGlobalEndpoint("e1");
    vFed2->registerGlobalEndpoint("e2");
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();
    vFed2->requestTimeAsync(3.5);
    auto res = vFed1->requestTime(0.32);
    // check that the request is only granted at the appropriate period
    EXPECT_EQ(res, 0.6);
    ept1.sendTo("test1", "e2");
    vFed1->requestTimeAsync(1.85);
    res = vFed2->requestTimeComplete();
    EXPECT_EQ(res, 0.9);  // the message should show up at the next available time point
    vFed2->requestTimeAsync(2.0);
    res = vFed2->requestTimeComplete();
    EXPECT_EQ(res, 2.25);  // the message should show up at the next available time point
    vFed2->requestTimeAsync(3.0);
    res = vFed1->requestTimeComplete();
    EXPECT_EQ(res, 2.4);
    vFed1->finalize();
    vFed2->finalize();  // this will also test finalizing while a time request is ongoing otherwise
                        // it will time out.
}

TEST_F(timing, simple_timing2_single_thread)
{
    extraFederateArgs = "--flags=single_thread_federate";

    SetupTest<helics::ValueFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    vFed1->setFlagOption(HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS);
    vFed2->setFlagOption(HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS);
    vFed1->setProperty(HELICS_PROPERTY_TIME_PERIOD, 0.5);
    vFed2->setProperty(HELICS_PROPERTY_TIME_PERIOD, 0.5);

    auto& pub = vFed1->registerGlobalPublication<double>("pub1");
    vFed2->registerSubscription("pub1");
    std::vector<helics::Time> times;

    auto fedThread = std::thread([&vFed1, &times, &pub]() {
        vFed1->enterExecutingMode();
        auto res = vFed1->requestTime(0.32);
        times.push_back(res);
        pub.publish(0.27);
        res = vFed1->requestTime(1.85);
        times.push_back(res);
    });

    vFed2->enterExecutingMode();

    auto res = vFed2->requestTime(1.79);
    EXPECT_EQ(res, 0.5);  // the result should show up at the next available time point
    res = vFed2->requestTime(2.0);
    EXPECT_EQ(res, 2.0);

    fedThread.join();
    // check that the request is only granted at the appropriate period
    EXPECT_EQ(times[0], 0.5);

    EXPECT_EQ(times[1], 2.0);
    vFed1->finalize();
    vFed2->finalize();
}

TEST_F(timing, simple_timing_message_single_thread)
{
    extraFederateArgs = "--flags=single_thread_federate";

    SetupTest<helics::MessageFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto vFed2 = GetFederateAs<helics::MessageFederate>(1);

    vFed1->setProperty(HELICS_PROPERTY_TIME_PERIOD, 0.6);
    vFed2->setProperty(HELICS_PROPERTY_TIME_PERIOD, 0.45);
    vFed1->setFlagOption(HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS);
    vFed2->setFlagOption(HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS);
    auto& ept1 = vFed1->registerGlobalEndpoint("e1");
    vFed2->registerGlobalEndpoint("e2");

    std::vector<helics::Time> times;
    auto fedThread = std::thread([&vFed1, &times, &ept1]() {
        vFed1->enterExecutingMode();
        auto res = vFed1->requestTime(0.32);
        times.push_back(res);
        ept1.sendTo("test1", "e2");
        res = vFed1->requestTime(1.85);
        times.push_back(res);
    });

    vFed2->enterExecutingMode();
    auto res = vFed2->requestTime(3.5);

    EXPECT_THROW(vFed2->requestTimeComplete(), helics::InvalidFunctionCall);
    EXPECT_EQ(res, 0.9);  // the message should show up at the next available time point
    res = vFed2->requestTime(2.0);
    EXPECT_EQ(res, 2.25);  // the message should show up at the next available time point
    vFed2->finalize();

    fedThread.join();
    EXPECT_EQ(times[0], 0.6);

    EXPECT_EQ(times[1], 2.4);
    vFed1->finalize();
}

TEST_F(timing, simple_global_timing_message)
{
    extraBrokerArgs = " --globaltime ";
    SetupTest<helics::MessageFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto vFed2 = GetFederateAs<helics::MessageFederate>(1);

    vFed1->setProperty(HELICS_PROPERTY_TIME_PERIOD, 0.6);
    vFed2->setProperty(HELICS_PROPERTY_TIME_PERIOD, 0.45);
    vFed1->setFlagOption(HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS);
    vFed2->setFlagOption(HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS);
    auto& ept1 = vFed1->registerGlobalEndpoint("e1");
    vFed2->registerGlobalEndpoint("e2");
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();
    vFed2->requestTimeAsync(3.5);
    auto res = vFed1->requestTime(0.32);
    // check that the request is only granted at the appropriate period
    EXPECT_EQ(res, 0.6);
    ept1.sendTo("test1", "e2");
    vFed1->requestTimeAsync(1.85);
    res = vFed2->requestTimeComplete();
    EXPECT_EQ(res, 0.9);  // the message should show up at the next available time point
    vFed2->requestTimeAsync(2.0);
    res = vFed2->requestTimeComplete();
    EXPECT_EQ(res, 2.25);  // the message should show up at the next available time point
    vFed2->requestTimeAsync(3.0);
    res = vFed1->requestTimeComplete();
    EXPECT_EQ(res, 2.4);
    vFed1->finalize();
    vFed2->finalize();  // this will also test finalizing while a time request is ongoing otherwise
                        // it will time out.
}

TEST_F(timing, test_uninteruptible_flag)
{
    SetupTest<helics::ValueFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    vFed2->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    vFed2->setFlagOption(HELICS_FLAG_UNINTERRUPTIBLE);

    auto& pub = vFed1->registerGlobalPublication<double>("pub1");
    vFed2->registerSubscription("pub1");

    auto rfed1 = [&]() {
        vFed1->enterExecutingMode();
        for (helics::Time requestTime = 1.0; requestTime <= 100.0; requestTime += 1.0) {
            pub.publish(requestTime);
            vFed1->requestTime(requestTime);
        }
    };

    auto rfed2 = [&]() {
        vFed2->enterExecutingMode();
        std::vector<helics::Time> res;
        for (helics::Time requestTime = 5.0; requestTime <= 100.0; requestTime += 5.0) {
            auto grantedTime = vFed2->requestTime(requestTime);
            res.push_back(grantedTime);
        }
        return res;
    };

    auto fed2res = std::async(std::launch::async, rfed2);
    auto fed1res = std::async(std::launch::async, rfed1);

    fed1res.get();
    vFed1->finalize();
    auto rvec = fed2res.get();
    EXPECT_EQ(rvec.front(), 5.0);
    EXPECT_EQ(rvec.size(), 20U);
    EXPECT_EQ(rvec[1], 10.0);
    EXPECT_EQ(rvec.back(), 100.0);

    vFed2->finalize();
}

TEST_F(timing, uninteruptible_flag_option)
{
    SetupTest<helics::ValueFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    vFed2->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);

    auto& pub = vFed1->registerGlobalPublication<double>("pub1");
    auto& IP2 = vFed2->registerSubscription("pub1");
    // test with the handle option vs the federate option
    IP2.setOption(helics::defs::Options::IGNORE_INTERRUPTS);
    auto rfed1 = [&]() {
        vFed1->enterExecutingMode();
        helics::Time requestTime{1.0};
        while (requestTime <= 100.0) {
            pub.publish(requestTime);
            auto grantedTime = vFed1->requestTime(requestTime);
            if (grantedTime == helics::Time::maxVal()) {
                break;
            }
            requestTime += 1.0;
        }
    };

    auto rfed2 = [&]() {
        vFed2->enterExecutingMode();
        std::vector<helics::Time> res;
        double time{5.0};
        while (time <= 100.0) {
            auto grantedTime = vFed2->requestTime(time);
            res.push_back(grantedTime);
            time += 5.0;
        }
        return res;
    };

    auto fed2res = std::async(std::launch::async, rfed2);
    auto fed1res = std::async(std::launch::async, rfed1);

    fed1res.get();
    vFed1->finalize();
    auto rvec = fed2res.get();
    EXPECT_EQ(rvec.front(), 5.0);
    EXPECT_EQ(rvec.size(), 20U);
    EXPECT_EQ(rvec[1], 10.0);
    EXPECT_EQ(rvec.back(), 100.0);

    vFed2->finalize();
}

TEST_F(timing, uninterruptible_flag_two_way_comm)
{
    SetupTest<helics::ValueFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    vFed1->setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    vFed2->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    vFed2->setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    vFed2->setFlagOption(HELICS_FLAG_UNINTERRUPTIBLE);

    auto& pub1 = vFed1->registerGlobalPublication<double>("pub1");
    auto& pub2 = vFed2->registerGlobalPublication<double>("pub2");
    vFed1->registerSubscription("pub2");
    vFed2->registerSubscription("pub1");

    auto rfed1 = [&]() {
        vFed1->enterExecutingMode();
        double requestTime{1.0};
        while (requestTime <= 100.0) {
            try {
                pub1.publish(requestTime);
            }
            catch (const helics::HelicsException&) {
                std::cerr << "error in fed 1 publication at time " << requestTime << '\n';
                break;
            }
            auto grantedTime = vFed1->requestTime(requestTime);
            if (grantedTime == helics::Time::maxVal()) {
                break;
            }
            requestTime += 1.0;
        }
    };

    auto rfed2 = [&]() {
        vFed2->enterExecutingMode();
        std::vector<helics::Time> res;
        double requestTime{5.0};
        while (requestTime <= 100.0) {
            try {
                pub2.publish(requestTime);
            }
            catch (const helics::HelicsException&) {
                std::cerr << "error in fed 2 publication at time " << requestTime << '\n';
                break;
            }
            auto grantedTime = vFed2->requestTime(requestTime);
            res.push_back(grantedTime);
            requestTime += 5.0;
            if (grantedTime == helics::Time::maxVal()) {
                break;
            }
        }
        return res;
    };

    auto fed2res = std::async(std::launch::async, rfed2);
    auto fed1res = std::async(std::launch::async, rfed1);

    fed1res.get();
    vFed1->finalize();
    auto rvec = fed2res.get();
    EXPECT_EQ(rvec.front(), 5.0);
    EXPECT_EQ(rvec.size(), 20U);
    EXPECT_EQ(rvec[1], 10.0);
    EXPECT_EQ(rvec.back(), 100.0);

    vFed2->finalize();
}

TEST_F(timing, uninterruptible_iterations)
{
    SetupTest<helics::ValueFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    vFed1->setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    vFed2->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    vFed2->setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    vFed2->setFlagOption(HELICS_FLAG_UNINTERRUPTIBLE);

    auto& pub1 = vFed1->registerGlobalPublication<double>("pub1");
    auto& pub2 = vFed2->registerGlobalPublication<double>("pub2");
    vFed1->registerSubscription("pub2");
    vFed2->registerSubscription("pub1");
    int iterationCount1{0};
    int iterationCount2{0};
    auto rfed1 = [&]() {
        vFed1->enterExecutingMode();
        double requestTime{1.0};
        double previousTime{0.0};
        while (requestTime <= 100.0) {
            try {
                if (requestTime > previousTime) {
                    pub1.publish(requestTime);
                }
            }
            catch (const helics::HelicsException&) {
                std::cerr << "error in fed 1 publication at time " << requestTime << '\n';
                break;
            }
            auto grantedTime = vFed1->requestTimeIterative(
                requestTime, helics::IterationRequest::ITERATE_IF_NEEDED);
            if (grantedTime.grantedTime == helics::Time::maxVal()) {
                break;
            }
            previousTime = requestTime;
            if (grantedTime.state == helics::IterationResult::NEXT_STEP) {
                requestTime += 1.0;
            } else {
                ++iterationCount1;
            }
        }
    };

    auto rfed2 = [&]() {
        vFed2->enterExecutingMode();
        std::vector<helics::Time> res;
        double requestTime{5.0};
        double previousTime{0.0};
        while (requestTime <= 100.0) {
            try {
                if (requestTime > previousTime) {
                    pub2.publish(requestTime);
                }
            }
            catch (const helics::HelicsException&) {
                std::cerr << "error in fed 2 publication at time " << requestTime << '\n';
                break;
            }
            auto grantedTime = vFed2->requestTimeIterative(
                requestTime, helics::IterationRequest::ITERATE_IF_NEEDED);
            res.push_back(grantedTime.grantedTime);
            previousTime = requestTime;
            if (grantedTime.state == helics::IterationResult::NEXT_STEP) {
                requestTime += 5.0;
            } else {
                ++iterationCount2;
            }

            if (grantedTime.grantedTime == helics::Time::maxVal()) {
                break;
            }
        }
        return res;
    };

    auto fed2res = std::async(std::launch::async, rfed2);
    auto fed1res = std::async(std::launch::async, rfed1);

    fed1res.get();
    vFed1->finalize();
    auto rvec = fed2res.get();
    EXPECT_EQ(rvec.front(), 0.0);
    EXPECT_EQ(rvec.size(), 40U);
    EXPECT_EQ(rvec[1], 5.0);
    EXPECT_EQ(rvec.back(), 100.0);
    EXPECT_EQ(iterationCount1, 20);
    EXPECT_EQ(iterationCount2, 20);
    vFed2->finalize();
}

TEST_F(timing, timing_with_input_delay)
{
    SetupTest<helics::MessageFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto vFed2 = GetFederateAs<helics::MessageFederate>(1);

    vFed1->setProperty(HELICS_PROPERTY_TIME_PERIOD, 0.1);
    vFed2->setProperty(HELICS_PROPERTY_TIME_PERIOD, 0.1);
    vFed2->setProperty(HELICS_PROPERTY_TIME_INPUT_DELAY, 0.1);

    auto& ept1 = vFed1->registerGlobalEndpoint("e1");
    vFed2->registerGlobalEndpoint("e2");
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();
    vFed2->requestTimeAsync(2.0);
    auto res = vFed1->requestTime(1.0);
    // check that the request is only granted at the appropriate period
    EXPECT_EQ(res, 1.0);
    ept1.sendTo("test1", "e2");
    vFed1->requestTimeAsync(1.9);
    res = vFed2->requestTimeComplete();
    EXPECT_EQ(res,
              1.1);  // the message should show up at the next available time point after the impact
                     // window
    vFed2->requestTimeAsync(2.0);
    res = vFed1->requestTimeComplete();
    EXPECT_EQ(res, 1.9);
    res = vFed2->requestTimeComplete();
    EXPECT_EQ(res, 2.0);
    vFed1->finalize();
    vFed2->finalize();
}

TEST_F(timing, timing_with_minDelta_change)
{
    SetupTest<helics::ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    vFed1->enterExecutingMode();

    auto res = vFed1->requestTime(1.0);
    // check that the request is only granted at the appropriate period

    EXPECT_EQ(res, 1.0);

    // purposely requesting 1.0 to test min delta
    res = vFed1->requestTime(1.0);
    EXPECT_EQ(res, 2.0);

    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 0.1);
    res = vFed1->requestTime(res);
    EXPECT_EQ(res, 2.1);
    vFed1->finalize();
}

TEST_F(timing, timing_with_period_change)
{
    SetupTest<helics::ValueFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    vFed1->setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    vFed1->enterExecutingMode();

    auto res = vFed1->requestTime(1.0);
    // check that the request is only granted at the appropriate period

    EXPECT_EQ(res, 1.0);

    // purposely requesting 1.0 to test min delta
    res = vFed1->requestTime(1.0);
    EXPECT_EQ(res, 2.0);

    vFed1->setProperty(HELICS_PROPERTY_TIME_PERIOD, 0.1);
    res = vFed1->requestTime(res);
    EXPECT_EQ(res, 2.1);
    vFed1->finalize();
}

TEST_F(timing, sender_finalize_timing_result)
{
    SetupTest<helics::ValueFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    helics::Publication sender(helics::InterfaceVisibility::GLOBAL,
                               vFed1,
                               "pub",
                               helics::DataType::HELICS_DOUBLE);
    auto& receiver = vFed2->registerSubscription("pub");
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();
    auto granted1 = vFed1->requestTime(1.0);
    EXPECT_EQ(granted1, 1.0);
    sender.publish(1.0);

    granted1 = vFed1->requestTime(2.0);
    EXPECT_EQ(granted1, 2.0);
    // now check that the receiver got the data at time 1.0
    auto granted2 = vFed2->requestTime(400.0);  // request a big time
    EXPECT_EQ(granted2, 1.0);
    EXPECT_TRUE(receiver.isUpdated());
    EXPECT_EQ(receiver.getValue<double>(), 1.0);

    // now do 2 publish cycles in a row
    sender.publish(2.0);
    granted1 = vFed1->requestTime(3.0);
    EXPECT_EQ(granted1, 3.0);

    sender.publish(3.0);
    granted1 = vFed1->requestTime(4.0);
    EXPECT_EQ(granted1, 4.0);
    sender.publish(4.0);

    // check the grant at time 2.0
    granted2 = vFed2->requestTime(400.0);  // request a big time
    EXPECT_EQ(granted2, 2.0);
    EXPECT_TRUE(receiver.isUpdated());
    EXPECT_EQ(receiver.getValue<double>(), 2.0);

    granted1 = vFed1->requestTime(6.0);
    EXPECT_EQ(granted1, 6.0);
    sender.publish(6.0);

    vFed1->finalize();
    // check the grant at time 2.0
    granted2 = vFed2->requestTime(400.0);  // request a big time
    EXPECT_EQ(granted2, 3.0);
    EXPECT_TRUE(receiver.isUpdated());
    EXPECT_EQ(receiver.getValue<double>(), 3.0);
    // check the grant at time 2.0
    granted2 = vFed2->requestTime(400.0);  // request a big time
    EXPECT_EQ(granted2, 4.0);
    EXPECT_TRUE(receiver.isUpdated());
    EXPECT_EQ(receiver.getValue<double>(), 4.0);
    // check the grant at time 2.0
    granted2 = vFed2->requestTime(400.0);  // request a big time
    EXPECT_EQ(granted2, 6.0);
    EXPECT_TRUE(receiver.isUpdated());
    EXPECT_EQ(receiver.getValue<double>(), 6.0);
    vFed2->finalize();
}

TEST_F(timing, sender_finalize_timing_result2)
{
    SetupTest<helics::ValueFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    helics::Publication sender(helics::InterfaceVisibility::GLOBAL,
                               vFed1,
                               "pub",
                               helics::DataType::HELICS_DOUBLE);
    auto& receiver = vFed2->registerSubscription("pub");
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();
    auto granted1 = vFed1->requestTime(1.0);
    EXPECT_EQ(granted1, 1.0);
    sender.publish(1.0);

    granted1 = vFed1->requestTime(2.0);
    EXPECT_EQ(granted1, 2.0);
    // now check that the receiver got the data at time 1.0
    auto granted2 = vFed2->requestTime(400.0);  // request a big time
    EXPECT_EQ(granted2, 1.0);
    EXPECT_TRUE(receiver.isUpdated());
    EXPECT_EQ(receiver.getValue<double>(), 1.0);

    // now do 2 publish cycles in a row
    sender.publish(2.0);
    granted1 = vFed1->requestTime(3.0);
    EXPECT_EQ(granted1, 3.0);

    sender.publish(3.0);
    granted1 = vFed1->requestTime(4.0);
    EXPECT_EQ(granted1, 4.0);
    sender.publish(4.0);

    // check the grant at time 2.0
    granted2 = vFed2->requestTime(400.0);  // request a big time
    EXPECT_EQ(granted2, 2.0);
    EXPECT_TRUE(receiver.isUpdated());
    EXPECT_EQ(receiver.getValue<double>(), 2.0);

    granted1 = vFed1->requestTime(6.0);
    EXPECT_EQ(granted1, 6.0);
    sender.publish(6.0);

    vFed1->finalize();
    // check the grant at time 2.0
    granted2 = vFed2->requestTime(400.0);  // request a big time
    EXPECT_EQ(granted2, 3.0);
    EXPECT_TRUE(receiver.isUpdated());
    EXPECT_EQ(receiver.getValue<double>(), 3.0);
    // check the grant at time 2.0
    granted2 = vFed2->requestTime(400.0);  // request a big time
    EXPECT_EQ(granted2, 4.0);
    EXPECT_TRUE(receiver.isUpdated());
    EXPECT_EQ(receiver.getValue<double>(), 4.0);
    // check the grant at time 2.0
    granted2 = vFed2->requestTime(5.0);  // request time of 5
    EXPECT_EQ(granted2, 5.0);
    EXPECT_TRUE(!receiver.isUpdated());  // should not have an update
    EXPECT_EQ(receiver.getValue<double>(), 4.0);  // the get value should be the previous value

    granted2 = vFed2->requestTime(400.0);  // request a big time
    EXPECT_EQ(granted2, 6.0);
    EXPECT_TRUE(receiver.isUpdated());
    EXPECT_EQ(receiver.getValue<double>(), 6.0);
    vFed2->finalize();
}

#ifdef HELICS_ENABLE_ZMQ_CORE
TEST_F(timing, fast_sender_tests_ci_skip)  // ci_skip
{
    SetupTest<helics::ValueFederate>("zmq_2", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    helics::Publication sender(helics::InterfaceVisibility::GLOBAL,
                               vFed1,
                               "pub",
                               helics::DataType::HELICS_DOUBLE);
    auto& receiver = vFed2->registerSubscription("pub");
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();
    const helics::Time endTime(8000.0);
    helics::Time currentTime = 0.0;
    while (currentTime <= endTime) {
        currentTime += 1.0;
        currentTime = vFed1->requestTime(currentTime);
        sender.publish(static_cast<double>(currentTime));
    }
    vFed1->finalize();
    currentTime = 0.0;
    while (currentTime <= endTime) {
        currentTime = vFed2->requestTime(endTime + 2000.0);
        if (receiver.isUpdated()) {
            double val = receiver.getValue<double>();
            EXPECT_EQ(val, static_cast<double>(currentTime));
        }
    }
    vFed2->finalize();
}

TEST_F(timing, dual_fast_sender_tests_ci_skip)  // ci_skip
{
    SetupTest<helics::ValueFederate>("zmq_2", 3);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    auto vFed3 = GetFederateAs<helics::ValueFederate>(2);
    helics::Publication sender1(helics::InterfaceVisibility::GLOBAL,
                                vFed1,
                                "pub1",
                                helics::DataType::HELICS_DOUBLE);
    auto& receiver1 = vFed2->registerSubscription("pub1");
    helics::Publication sender2(helics::InterfaceVisibility::GLOBAL,
                                vFed3,
                                "pub2",
                                helics::DataType::HELICS_DOUBLE);
    auto& receiver2 = vFed2->registerSubscription("pub2");
    vFed1->enterExecutingModeAsync();
    vFed3->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();
    vFed3->enterExecutingModeComplete();
    const helics::Time endTime(500.0);
    helics::Time currentTime = helics::timeZero;
    while (currentTime <= endTime) {
        currentTime += 1.0;
        currentTime = vFed1->requestTime(currentTime);
        sender1.publish(static_cast<double>(currentTime));
    }
    vFed1->finalize();
    currentTime = helics::timeZero;
    while (currentTime <= endTime) {
        currentTime += 1.0;
        currentTime = vFed3->requestTime(currentTime);
        sender2.publish(static_cast<double>(currentTime));
    }
    vFed3->finalize();
    currentTime = helics::timeZero;
    while (currentTime <= endTime) {
        currentTime = vFed2->requestTime(endTime + 2000.0);
        if (receiver1.isUpdated()) {
            double val = receiver1.getValue<double>();
            EXPECT_EQ(val, static_cast<double>(currentTime));
        }
        if (receiver2.isUpdated()) {
            double val = receiver2.getValue<double>();
            EXPECT_EQ(val, static_cast<double>(currentTime));
        }
    }
    vFed2->finalize();
}

#endif

TEST_F(timing, async_timing)
{
    extraBrokerArgs = "--asynctime";
    SetupTest<helics::ValueFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    vFed1->setProperty(HELICS_PROPERTY_TIME_PERIOD, 0.5);
    vFed2->setProperty(HELICS_PROPERTY_TIME_PERIOD, 0.5);

    auto& pub = vFed1->registerGlobalPublication<double>("pub1");
    vFed2->registerSubscription("pub1");
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();

    auto res = vFed1->requestTime(1.0);
    EXPECT_EQ(res, 1.0);
    pub.publish(0.27);
    res = vFed2->requestTime(0.5);
    EXPECT_EQ(res, 0.5);
    res = vFed2->requestTime(2.0);
    EXPECT_GE(res, 1.0);  // the result should show up at the next available time point
    // vFed2 should be able to grant in advance of fed1 using the async timing
    res = vFed2->requestTime(2.0);
    EXPECT_GE(res, 2.0);
    res = vFed1->requestTime(1.5);
    EXPECT_EQ(res, 1.5);
    pub.publish(0.44);
    vFed1->query("root", "global_flush");
    vFed1->requestTime(2.0);
    res = vFed2->requestTime(5.0);
    EXPECT_LT(res, 4.0);

    vFed1->finalize();
    vFed2->finalize();
}

TEST_F(timing, async_timing_message)
{
    extraBrokerArgs = "--asynctime";
    SetupTest<helics::MessageFederate>("test", 2);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);
    mFed1->setProperty(HELICS_PROPERTY_TIME_PERIOD, 0.5);
    mFed2->setProperty(HELICS_PROPERTY_TIME_PERIOD, 0.5);

    auto& pub = mFed1->registerGlobalTargetedEndpoint("pub1");
    auto& sub = mFed2->registerTargetedEndpoint("pubr");
    sub.addSourceEndpoint("pub1");
    mFed1->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();

    auto res = mFed1->requestTime(1.0);
    EXPECT_EQ(res, 1.0);
    pub.send("test message");
    res = mFed2->requestTime(0.5);
    EXPECT_EQ(res, 0.5);
    res = mFed2->requestTime(2.0);
    EXPECT_GE(res, 1.0);  // the result should show up at the next available time point
    // vFed2 should be able to grant in advance of fed1 using the async timing
    res = mFed2->requestTime(2.0);
    EXPECT_GE(res, 2.0);
    res = mFed1->requestTime(1.5);
    EXPECT_EQ(res, 1.5);
    pub.send("test 2");
    mFed1->query("root", "global_flush");
    mFed1->requestTime(2.0);
    res = mFed2->requestTime(5.0);
    EXPECT_LT(res, 4.0);

    mFed1->finalize();
    mFed2->finalize();
}

TEST_F(timing, dual_max_time)
{
    SetupTest<helics::ValueFederate>("test_2", 2);
    auto controller = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    vFed2->setFlagOption(HELICS_FLAG_UNINTERRUPTIBLE, true);

    auto& pubc = controller->registerGlobalPublication<double>("control");
    auto& subc = controller->registerSubscription("value");
    auto& pubv = vFed2->registerGlobalPublication<double>("value");
    auto& subv = vFed2->registerSubscription("control");

    auto cfed = [&]() {
        controller->enterExecutingMode();
        helics::Time grantedTime = helics::timeZero;
        helics::Time maxTime = cHelicsBigNumber;
        grantedTime = controller->requestTime(maxTime);
        while (grantedTime < cHelicsTerminateTime) {
            double value = subc.getDouble();
            pubc.publish(2.0 * value);
            grantedTime = controller->requestTime(maxTime);
        }
        controller->disconnect();
    };

    double finalValue{0.0};
    auto vfed = [&]() {
        vFed2->enterExecutingMode();
        helics::Time grantedTime = helics::timeZero;
        helics::Time maxTime = 12.0;
        pubv.publish(1.0);
        while (grantedTime < maxTime) {
            grantedTime = vFed2->requestTime(grantedTime + 2.0);
            double value = subv.getDouble();
            pubv.publish(value + 0.7);
        }
        vFed2->requestTime(cHelicsBigNumber);
        finalValue = subv.getDouble();
    };

    auto fed2res = std::async(std::launch::async, cfed);
    auto fed1res = std::async(std::launch::async, vfed);

    fed1res.get();

    fed2res.get();
    double testVal{2.0};
    // compute the expected value
    auto updateValue = [](double val) { return 2.0 * (val + 0.7); };
    for (int ii = 0; ii < 6; ++ii) {
        testVal = updateValue(testVal);
    }
    EXPECT_DOUBLE_EQ(finalValue, testVal);
}

TEST_F(timing, dual_max_time_endpoint)
{
    SetupTest<helics::MessageFederate>("test_2", 2);
    auto controller = GetFederateAs<helics::MessageFederate>(0);
    auto vFed2 = GetFederateAs<helics::MessageFederate>(1);

    vFed2->setFlagOption(HELICS_FLAG_UNINTERRUPTIBLE, true);

    auto& controlEndpoint = controller->registerGlobalEndpoint("control");
    auto& valueEndpoint = vFed2->registerGlobalEndpoint("value");

    auto cfed = [&]() {
        controller->enterExecutingMode();
        helics::Time grantedTime = helics::timeZero;
        helics::Time maxTime = cHelicsBigNumber;

        grantedTime = controller->requestTime(maxTime);
        while (grantedTime < cHelicsTerminateTime) {
            auto message = controlEndpoint.getMessage();
            message->data.append("a", 1);
            message->dest = "value";
            controlEndpoint.send(std::move(message));
            grantedTime = controller->requestTime(maxTime);
        }
        controller->disconnect();
    };

    std::string finalValue;
    auto vfed = [&]() {
        vFed2->enterExecutingMode();
        helics::Time grantedTime = helics::timeZero;
        helics::Time maxTime = 12.0;
        valueEndpoint.sendTo("b", "control");
        while (grantedTime < maxTime) {
            grantedTime = vFed2->requestTime(grantedTime + 2.0);
            auto message = valueEndpoint.getMessage();
            if (message) {
                message->data.append("b", 1);
                message->dest = "control";
                valueEndpoint.send(std::move(message));
            }
        }
        vFed2->requestTime(cHelicsBigNumber);
        auto message = valueEndpoint.getMessage();
        if (message) {
            finalValue = message->data.to_string();
        }
    };

    auto fed2res = std::async(std::launch::async, cfed);
    auto fed1res = std::async(std::launch::async, vfed);

    fed1res.get();

    fed2res.get();
    std::string test{"bababababababa"};
    // compute the expected value
    EXPECT_EQ(finalValue, test);
}

static void maxTimeDrainAfterSenderFinalizeEndpoint(FederateTestFixture& fixture,
                                                    std::string_view coreType,
                                                    bool waitForCurrentTimeUpdate = false)
{
    fixture.SetupTest<helics::MessageFederate>(coreType, 2);
    auto sender = fixture.GetFederateAs<helics::MessageFederate>(0);
    auto receiver = fixture.GetFederateAs<helics::MessageFederate>(1);

    if (waitForCurrentTimeUpdate) {
        receiver->setFlagOption(helics::defs::WAIT_FOR_CURRENT_TIME_UPDATE);
    }

    auto& sendEndpoint = sender->registerGlobalEndpoint("source");
    auto& receiveEndpoint = receiver->registerGlobalEndpoint("sink");
    sendEndpoint.setDefaultDestination("sink");

    auto senderTask = [&]() {
        sender->enterExecutingMode();
        for (int ii = 0; ii < 5; ++ii) {
            const auto payload = std::string("message") + std::to_string(ii);
            sendEndpoint.send(payload.data(), payload.size());
            sender->requestTime(static_cast<double>(ii + 1));
        }
        sender->finalize();
    };

    auto receiverTask = [&]() {
        receiver->enterExecutingMode();
        std::vector<std::string> messages;
        helics::Time grantedTime = helics::timeZero;
        while (grantedTime < cHelicsTerminateTime) {
            grantedTime = receiver->requestTime(helics::Time::maxVal());
            while (receiveEndpoint.hasMessage()) {
                auto msg = receiveEndpoint.getMessage();
                messages.emplace_back(msg->data.to_string());
            }
        }
        receiver->finalize();
        return messages;
    };

    auto receiverFuture = std::async(std::launch::async, receiverTask);
    auto senderFuture = std::async(std::launch::async, senderTask);

    ASSERT_EQ(senderFuture.wait_for(std::chrono::seconds(5)), std::future_status::ready);
    senderFuture.get();

    ASSERT_EQ(receiverFuture.wait_for(std::chrono::seconds(5)), std::future_status::ready);
    const auto messages = receiverFuture.get();

    ASSERT_EQ(messages.size(), 5U);
    EXPECT_EQ(messages.front(), "message0");
    EXPECT_EQ(messages.back(), "message4");
}

TEST_F(timing, max_time_drain_after_sender_finalize_endpoint)
{
    maxTimeDrainAfterSenderFinalizeEndpoint(*this, "test_2");
}

TEST_F(timing, max_time_drain_after_sender_finalize_endpoint_zmq)
{
    maxTimeDrainAfterSenderFinalizeEndpoint(*this, "zmq_2");
}

TEST_F(timing, max_time_drain_after_sender_finalize_endpoint_global_time)
{
    extraBrokerArgs = "--globaltime";
    maxTimeDrainAfterSenderFinalizeEndpoint(*this, "test_2");
}

TEST_F(timing, max_time_drain_after_sender_finalize_endpoint_wait_for_current_time)
{
    maxTimeDrainAfterSenderFinalizeEndpoint(*this, "test_2", true);
}

TEST_F(timing, max_time_drain_after_sender_finalize_endpoint_wait_for_current_time_zmq)
{
    maxTimeDrainAfterSenderFinalizeEndpoint(*this, "zmq_2", true);
}

TEST_F(timing, max_time_drain_after_sender_finalize_mixed_value_endpoint_zmq)
{
    SetupTest<helics::CombinationFederate>("zmq_2", 2);
    auto grid = GetFederateAs<helics::CombinationFederate>(0);
    auto storage = GetFederateAs<helics::CombinationFederate>(1);

    grid->setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);

    constexpr int storageDeviceCount = 4;
    auto& totalPower = grid->registerGlobalPublication<std::complex<double>>("total_power", "kW");
    std::vector<helics::Input*> powerSubscriptions;
    std::vector<helics::Publication*> voltagePublications;
    std::vector<helics::Publication*> socPublications;
    std::vector<helics::Publication*> gridPowerPublications;
    std::vector<helics::Publication*> storagePowerPublications;

    powerSubscriptions.reserve(storageDeviceCount);
    voltagePublications.reserve(storageDeviceCount);
    socPublications.reserve(storageDeviceCount);
    gridPowerPublications.reserve(storageDeviceCount);
    storagePowerPublications.reserve(storageDeviceCount);

    for (int ii = 0; ii < storageDeviceCount; ++ii) {
        const auto device = std::string("storage") + std::to_string(ii);
        auto& powerSub = grid->registerSubscription(device + "/power", "kW");
        powerSub.setDefault(std::complex<double>{0.0, 0.0});
        powerSubscriptions.push_back(&powerSub);

        voltagePublications.push_back(
            &grid->registerGlobalPublication<double>("storage." + device + ".voltage", "pu"));
        socPublications.push_back(
            &grid->registerGlobalPublication<double>("storage." + device + ".soc"));
        gridPowerPublications.push_back(&grid->registerGlobalPublication<std::complex<double>>(
            "storage." + device + ".power", "kW"));

        storagePowerPublications.push_back(
            &storage->registerGlobalPublication<std::complex<double>>(device + "/power", "kW"));
    }

    auto& reliabilityEndpoint = grid->registerGlobalEndpoint("reliability");
    auto& metricsEndpoint = grid->registerGlobalEndpoint("metrics");
    auto& storageEndpoint = storage->registerGlobalEndpoint("storage_status");

    auto storageTask = [&]() {
        storage->enterExecutingMode();
        for (int step = 0; step < 12; ++step) {
            for (int ii = 0; ii < storageDeviceCount; ++ii) {
                storagePowerPublications[ii]->publish(
                    std::complex<double>{static_cast<double>(step + ii), -0.25 * ii});
            }
            storageEndpoint.sendTo(std::string("reliability") + std::to_string(step),
                                   "reliability");
            storageEndpoint.sendTo(std::string("metrics") + std::to_string(step), "metrics");
            storage->requestTime(static_cast<double>(step + 1));
        }
        storage->finalize();
    };

    auto gridTask = [&]() {
        grid->enterExecutingMode();
        std::vector<std::string> endpointMessages;
        helics::Time grantedTime = helics::timeZero;
        for (int step = 0; step < 12; ++step) {
            grantedTime = grid->requestTime(static_cast<double>(step + 1));
            std::complex<double> aggregate{0.0, 0.0};
            for (int ii = 0; ii < storageDeviceCount; ++ii) {
                aggregate += powerSubscriptions[ii]->getValue<std::complex<double>>();
                voltagePublications[ii]->publish(1.0 + 0.01 * ii);
                socPublications[ii]->publish(0.5 + 0.01 * step);
                gridPowerPublications[ii]->publish(aggregate);
            }
            totalPower.publish(aggregate);

            while (reliabilityEndpoint.hasMessage()) {
                endpointMessages.emplace_back(reliabilityEndpoint.getMessage()->data.to_string());
            }
            while (metricsEndpoint.hasMessage()) {
                endpointMessages.emplace_back(metricsEndpoint.getMessage()->data.to_string());
            }
        }

        while (grantedTime < cHelicsTerminateTime) {
            grantedTime = grid->requestTime(helics::Time::maxVal());
            while (reliabilityEndpoint.hasMessage()) {
                endpointMessages.emplace_back(reliabilityEndpoint.getMessage()->data.to_string());
            }
            while (metricsEndpoint.hasMessage()) {
                endpointMessages.emplace_back(metricsEndpoint.getMessage()->data.to_string());
            }
        }
        grid->finalize();
        return endpointMessages.size();
    };

    auto gridFuture = std::async(std::launch::async, gridTask);
    auto storageFuture = std::async(std::launch::async, storageTask);

    ASSERT_EQ(storageFuture.wait_for(std::chrono::seconds(10)), std::future_status::ready);
    storageFuture.get();

    ASSERT_EQ(gridFuture.wait_for(std::chrono::seconds(10)), std::future_status::ready);
    EXPECT_EQ(gridFuture.get(), 24U);
}
