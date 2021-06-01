/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "gtest/gtest.h"
#include <complex>

/** these test cases test out the value converters
 */
#include "../application_api/testFixtures.hpp"
#include "helics/helics.hpp"

#include <future>

struct timing_tests2: public FederateTestFixture, public ::testing::Test {
};
/** just a check that in the simple case we do actually get the time back we requested*/

TEST_F(timing_tests2, small_time_test)
{
    SetupTest<helics::ValueFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    auto pub1_a = helics::make_publication<double>(helics::GLOBAL, vFed1, "pub1_a");
    auto pub1_b = helics::make_publication<double>(helics::GLOBAL, vFed1, "pub1_b");
    auto pub2_a = helics::make_publication<double>(helics::GLOBAL, vFed2, "pub2_a");
    auto pub2_b = helics::make_publication<double>(helics::GLOBAL, vFed2, "pub2_b");

    auto sub1_a = vFed2->registerSubscription("pub1_a");
    auto sub1_b = vFed2->registerSubscription("pub1_b");
    auto sub2_a = vFed2->registerSubscription("pub2_a");
    auto sub2_b = vFed2->registerSubscription("pub2_b");
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();
    auto echoRun = [&]() {
        helics::Time grantedTime = helics::timeZero;
        helics::Time stopTime(100, time_units::ns);
        while (grantedTime < stopTime) {
            grantedTime = vFed2->requestTime(stopTime);
            if (sub1_a.isUpdated()) {
                auto val = sub1_a.getValue<double>();
                pub2_a->publish(val);
            }
            if (sub1_b.isUpdated()) {
                auto val = sub1_b.getValue<double>();
                pub2_b->publish(val);
            }
        }
    };

    auto fut = std::async(echoRun);
    helics::Time grantedTime = helics::timeZero;
    helics::Time requestedTime(10, time_units::ns);
    helics::Time stopTime(100, time_units::ns);
    while (grantedTime < stopTime) {
        grantedTime = vFed1->requestTime(requestedTime);
        if (grantedTime == requestedTime) {
            pub1_a->publish(10.3);
            pub1_b->publish(11.2);
            requestedTime += helics::Time(10, time_units::ns);
        } else {
            EXPECT_TRUE(grantedTime == requestedTime - helics::Time(9, time_units::ns));
            // printf("grantedTime=%e\n", static_cast<double>(grantedTime));
            if (sub2_a.isUpdated()) {
                EXPECT_EQ(sub2_a.getValue<double>(), 10.3);
            }
            if (sub2_b.isUpdated()) {
                EXPECT_EQ(sub2_b.getValue<double>(), 11.2);
            }
        }
    }
    vFed1->finalize();
    fut.get();
    vFed2->finalize();
}

/** based on bug found by Manoj Kumar Cebol Sundarrajan
where a very small period could cause the time to be negative
*/
TEST_F(timing_tests2, small_period_test)
{
    SetupTest<helics::MessageFederate>("test", 3);
    auto rx = GetFederateAs<helics::MessageFederate>(0);
    rx->setProperty(helics::defs::properties::time_delta, 1.0);
    rx->setProperty(helics::defs::properties::period, 0.000001);
    rx->setProperty(helics::defs::properties::offset, 0.0);
    auto send1 = GetFederateAs<helics::MessageFederate>(1);
    auto send2 = GetFederateAs<helics::MessageFederate>(2);

    auto& erx = rx->registerEndpoint("data");
    auto& s1 = send1->registerEndpoint("data");
    auto& s2 = send2->registerEndpoint("data");

    int cnt = 0;
    int cmess = 0;
    auto rxrun = [rx, &erx, &cnt, &cmess]() {
        rx->enterExecutingMode();
        helics::Time maxtime = 1e9;
        helics::Time ctime = -1;
        while (ctime < maxtime) {
            ctime = rx->requestTime(maxtime);
            // std::cout << "receiver: granted time " << static_cast<double> (ctime) << std::endl;
            ++cnt;
            while (erx.hasMessage()) {
                auto m = erx.getMessage();
                // std::cout << "receiver: message from " << m->source << " with data " <<
                // m->data.to_string ()
                //           << std::endl;
                ++cmess;
            }
            if (cnt > 300) {
                break;
            }
        }
        rx->finalize();
    };
    auto send1run = [send1, &s1]() {
        send1->enterExecutingMode();
        helics::Time ctime = helics::timeZero;
        while (ctime <= 10.0) {
            ctime += 1.0;
            ctime = send1->requestTime(ctime);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            //   std::cout << "sender1: sending message at time " << static_cast<double> (ctime) <<
            //   std::endl;
            s1.send("fed0/data", "3.14");
        }
        send1->finalize();
    };

    auto send2run = [send2, &s2]() {
        send2->enterExecutingMode();
        helics::Time ctime = helics::timeZero;
        while (ctime <= 10.0) {
            ctime += 1.0;
            ctime = send2->requestTime(ctime);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            // std::cout << "sender2: sending message at time " << static_cast<double> (ctime) <<
            // std::endl;
            s2.send("fed0/data", "3.14");
        }
        send2->finalize();
    };

    auto futrx = std::async(std::launch::async, rxrun);
    auto futs1 = std::async(std::launch::async, send1run);
    auto futs2 = std::async(std::launch::async, send2run);

    futs1.get();
    futs2.get();
    futrx.get();
    EXPECT_EQ(cnt, 12);
    EXPECT_EQ(cmess, 22);
}

// Tests out the restrictive time policy
TEST_F(timing_tests2, ring_test3)
{
    SetupTest<helics::ValueFederate>("test_2", 3);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    auto vFed3 = GetFederateAs<helics::ValueFederate>(2);
    vFed1->setFlagOption(helics::defs::restrictive_time_policy);
    vFed2->setFlagOption(helics::defs::restrictive_time_policy);
    vFed3->setFlagOption(helics::defs::restrictive_time_policy);

    auto pub1 = helics::make_publication<double>(helics::GLOBAL, vFed1, "pub1");
    auto pub2 = helics::make_publication<double>(helics::GLOBAL, vFed2, "pub2");
    auto pub3 = helics::make_publication<double>(helics::GLOBAL, vFed3, "pub3");
    auto sub1 = vFed1->registerSubscription("pub3");
    auto sub2 = vFed2->registerSubscription("pub1");
    auto sub3 = vFed3->registerSubscription("pub2");
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
    EXPECT_DOUBLE_EQ(val, 45.7);
    vFed2->finalize();
    newTime = vFed3->requestTimeComplete();
    EXPECT_EQ(newTime, 50.0);
    EXPECT_TRUE(!sub3.isUpdated());
    val = sub3.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 45.7);
    vFed3->finalize();
    newTime = vFed1->requestTimeComplete();
    EXPECT_EQ(newTime, 50.0);
    EXPECT_TRUE(!sub1.isUpdated());
    vFed1->finalize();
}

TEST_F(timing_tests2, wait_for_current_time_flag)
{
    SetupTest<helics::ValueFederate>("test_2", 3);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    auto vFed3 = GetFederateAs<helics::ValueFederate>(2);

    vFed3->setFlagOption(helics::defs::wait_for_current_time_update);

    auto pub1 = helics::make_publication<double>(helics::GLOBAL, vFed1, "pub1");

    auto& sub2 = vFed2->registerSubscription("pub1");
    auto& sub3 = vFed3->registerSubscription("pub1");
    sub2.setDefault(2.6);
    sub3.setDefault(1.9);
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingModeAsync();
    vFed3->enterExecutingMode();
    vFed1->enterExecutingModeComplete();
    vFed2->enterExecutingModeComplete();
    // this works since there are no reverse dependencies
    vFed1->requestTime(1.0);
    pub1->publish(3.5);
    auto tm1 = vFed1->requestTime(3.0);
    EXPECT_EQ(tm1, 3.0);
    auto tm2 = vFed2->requestTime(1.0);
    EXPECT_EQ(tm2, 1.0);
    double val2 = sub2.getValue<double>();
    EXPECT_DOUBLE_EQ(val2, 2.6);  // shouldn't have gotten the update

    auto tm3 = vFed3->requestTime(1.0);
    EXPECT_EQ(tm3, 1.0);
    double val3 = sub3.getValue<double>();
    EXPECT_DOUBLE_EQ(val3,
                     3.5);  // should have gotten the update from the wait_for_current_time_flag

    tm2 = vFed2->requestTime(2.0);
    EXPECT_EQ(tm2, helics::Time(1.0) + helics::Time(1, time_units::ns));

    tm3 = vFed3->requestTime(2.0);
    EXPECT_EQ(tm3, 2.0);

    val2 = sub2.getValue<double>();
    val3 = sub3.getValue<double>();
    EXPECT_DOUBLE_EQ(val2, 3.5);
    EXPECT_DOUBLE_EQ(val3, 3.5);

    pub1->publish(9.3);
    vFed1->finalize();

    // Now check that iteration works
    tm3 = vFed2->requestTime(3.0);
    EXPECT_EQ(tm3, 3.0);
    val2 = sub2.getValue<double>();
    EXPECT_DOUBLE_EQ(val2, 3.5);

    auto itTime = vFed2->requestTimeIterative(4.0, helics::iteration_request::iterate_if_needed);
    EXPECT_EQ(itTime.state, helics::iteration_result::iterating);
    EXPECT_EQ(itTime.grantedTime, 3.0);
    val2 = sub2.getValue<double>();
    EXPECT_DOUBLE_EQ(val2, 9.3);

    itTime = vFed2->requestTimeIterative(4.0, helics::iteration_request::iterate_if_needed);
    EXPECT_EQ(itTime.state, helics::iteration_result::next_step);
    EXPECT_EQ(itTime.grantedTime, 4.0);
    val2 = sub2.getValue<double>();
    EXPECT_DOUBLE_EQ(val2, 9.3);

    vFed2->finalize();

    // Now test the wait_for_current_time with iteration enabled
    auto itTime3 = vFed3->requestTimeIterative(3.0, helics::iteration_request::iterate_if_needed);
    EXPECT_EQ(itTime3.state, helics::iteration_result::next_step);
    EXPECT_EQ(itTime3.grantedTime, 3.0);
    val3 = sub3.getValue<double>();
    EXPECT_DOUBLE_EQ(val3, 9.3);

    vFed3->finalize();
}

TEST_F(timing_tests2, wait_for_current_time_flag2)
{
    extraBrokerArgs = "--debugging";
    auto broker = AddBroker("test", 2);
    extraCoreArgs = "--debugging";
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0);
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0);

    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    vFed2->setFlagOption(helics::defs::wait_for_current_time_update);

    auto& pub1_1 = vFed1->registerGlobalPublication<double>("pub1_1");
    auto& pub1_2 = vFed1->registerGlobalPublication<double>("pub1_2");
    vFed1->registerSubscription("pub2_1");

    vFed2->registerGlobalPublication<double>("pub2_1");

    auto& sub2_1 = vFed2->registerSubscription("pub1_1");
    auto& sub2_2 = vFed2->registerSubscription("pub1_2");
    sub2_1.setDefault(9.9);
    sub2_2.setDefault(10.5);

    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();

    pub1_1.publish(3.5);

    vFed2->requestTimeAsync(5.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(vFed2->isAsyncOperationCompleted());

    vFed1->requestTime(1.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(vFed2->isAsyncOperationCompleted());
    pub1_2.publish(8.8);

    vFed1->requestTimeAsync(3.0);
    auto retTime = vFed2->requestTimeComplete();
    EXPECT_EQ(retTime, 1.0);
    EXPECT_EQ(sub2_1.getValue<double>(), 3.5);
    EXPECT_EQ(sub2_2.getValue<double>(), 8.8);

    vFed2->requestTimeAsync(7.0);
    retTime = vFed1->requestTimeComplete();
    EXPECT_EQ(retTime, 3.0);
    pub1_1.publish(5.4);

    broker.reset();
    vFed1->finalize();

    retTime = vFed2->requestTimeComplete();
    EXPECT_EQ(retTime, 3.0);

    retTime = vFed2->requestTime(7.0);
    EXPECT_EQ(retTime, 7.0);
    vFed2->finalize();
}

TEST_F(timing_tests2, offset_timing)
{
    SetupTest<helics::ValueFederate>("test_2", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    vFed1->setFlagOption(helics::defs::ignore_time_mismatch_warnings);
    vFed2->setFlagOption(helics::defs::ignore_time_mismatch_warnings);
    vFed1->setProperty(helics::defs::period, 60.0);
    vFed2->setProperty(helics::defs::period, 60.0);
    vFed1->setProperty(helics::defs::offset, 10.0);

    auto pub1 = helics::make_publication<double>(helics::GLOBAL, vFed1, "pub1");

    auto& sub2 = vFed2->registerSubscription("pub1");
    sub2.setDefault(2.6);

    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();

    // this works since there are no reverse dependencies
    auto tm1 = vFed1->requestTime(70.0);
    EXPECT_EQ(tm1, 70.0);
    pub1->publish(3.5);
    tm1 = vFed1->requestTime(120.0);
    EXPECT_EQ(tm1, 130.0);

    auto tm2 = vFed2->requestTime(60.0);
    EXPECT_EQ(tm2, 60.0);
    tm2 = vFed2->requestTime(70.0);
    EXPECT_EQ(tm2, 120.0);
    double val2 = sub2.getValue<double>();
    EXPECT_DOUBLE_EQ(val2, 3.5);  // shouldn't have gotten the update

    vFed1->finalize();
    vFed2->finalize();
}

// Tests out the time barrier
TEST_F(timing_tests2, time_barrier1)
{
    SetupTest<helics::ValueFederate>("test_2", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    brokers[0]->setTimeBarrier(2.0);
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();
    vFed1->requestTimeAsync(3.0);
    auto rtime = vFed2->requestTime(1.89);
    EXPECT_EQ(rtime, 1.89);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_FALSE(vFed1->isAsyncOperationCompleted());
    brokers[0]->clearTimeBarrier();
    rtime = vFed1->requestTimeComplete();
    EXPECT_EQ(rtime, 3.0);
    vFed1->finalize();
    vFed2->finalize();
}

// Tests out the restrictive time policy
TEST_F(timing_tests2, time_barrier_update)
{
    SetupTest<helics::ValueFederate>("test_2", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    brokers[0]->setTimeBarrier(2.0);
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();
    vFed1->requestTimeAsync(3.0);
    auto rtime = vFed2->requestTime(1.89);
    EXPECT_EQ(rtime, 1.89);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_FALSE(vFed1->isAsyncOperationCompleted());
    brokers[0]->setTimeBarrier(4.0);
    rtime = vFed1->requestTimeComplete();
    EXPECT_EQ(rtime, 3.0);

    vFed1->finalize();
    vFed2->finalize();
}

TEST_F(timing_tests2, time_barrier_clear)
{
    SetupTest<helics::ValueFederate>("test_2", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    brokers[0]->setTimeBarrier(0.0);
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingModeAsync();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(vFed1->isAsyncOperationCompleted());
    EXPECT_FALSE(vFed2->isAsyncOperationCompleted());
    brokers[0]->setTimeBarrier(2.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(vFed1->isAsyncOperationCompleted());
    EXPECT_TRUE(vFed2->isAsyncOperationCompleted());
    vFed1->enterExecutingModeComplete();
    vFed2->enterExecutingModeComplete();
    vFed1->requestTimeAsync(1.0);
    vFed2->requestTimeAsync(1.0);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(vFed1->isAsyncOperationCompleted());
    EXPECT_TRUE(vFed2->isAsyncOperationCompleted());

    brokers[0]->setTimeBarrier(3.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(vFed1->isAsyncOperationCompleted());
    EXPECT_TRUE(vFed2->isAsyncOperationCompleted());
    auto tm = vFed1->requestTimeComplete();
    EXPECT_EQ(tm, 1.0);
    tm = vFed2->requestTimeComplete();
    EXPECT_EQ(tm, 1.0);

    vFed1->requestTimeAsync(3.0);
    auto rtime = vFed2->requestTime(1.89);
    EXPECT_EQ(rtime, 1.89);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_FALSE(vFed1->isAsyncOperationCompleted());
    brokers[0]->clearTimeBarrier();
    rtime = vFed1->requestTimeComplete();
    EXPECT_EQ(rtime, 3.0);

    vFed1->finalize();
    vFed2->finalize();
}

TEST_F(timing_tests2, time_barrier_clear2)
{
    SetupTest<helics::ValueFederate>("test_2", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    /** establish dependencies between the federates*/
    vFed1->registerGlobalPublication<double>("pub1");
    vFed2->registerGlobalPublication<double>("pub2");
    vFed1->registerSubscription("pub2");
    vFed2->registerSubscription("pub1");

    brokers[0]->setTimeBarrier(0.0);
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingModeAsync();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(vFed1->isAsyncOperationCompleted());
    EXPECT_FALSE(vFed2->isAsyncOperationCompleted());
    brokers[0]->setTimeBarrier(2.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(vFed1->isAsyncOperationCompleted());
    EXPECT_TRUE(vFed2->isAsyncOperationCompleted());
    vFed1->enterExecutingModeComplete();
    vFed2->enterExecutingModeComplete();
    vFed1->requestTimeAsync(1.0);
    vFed2->requestTimeAsync(1.0);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(vFed1->isAsyncOperationCompleted());
    EXPECT_TRUE(vFed2->isAsyncOperationCompleted());

    brokers[0]->setTimeBarrier(3.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(vFed1->isAsyncOperationCompleted());
    EXPECT_TRUE(vFed2->isAsyncOperationCompleted());
    auto tm = vFed1->requestTimeComplete();
    EXPECT_EQ(tm, 1.0);
    tm = vFed2->requestTimeComplete();
    EXPECT_EQ(tm, 1.0);

    vFed1->requestTimeAsync(3.0);
    auto rtime = vFed2->requestTime(1.89);
    EXPECT_EQ(rtime, 1.89);
    vFed2->requestTimeAsync(3.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_FALSE(vFed1->isAsyncOperationCompleted());
    EXPECT_FALSE(vFed2->isAsyncOperationCompleted());
    brokers[0]->clearTimeBarrier();
    rtime = vFed1->requestTimeComplete();
    EXPECT_EQ(rtime, 3.0);
    rtime = vFed2->requestTimeComplete();
    EXPECT_EQ(rtime, 3.0);

    vFed1->finalize();
    vFed2->finalize();
}
