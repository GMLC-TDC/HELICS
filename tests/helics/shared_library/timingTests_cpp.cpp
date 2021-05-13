/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../src/helics/cpp98/Broker.hpp"
#include "../src/helics/cpp98/Core.hpp"
#include "../src/helics/cpp98/ValueFederate.hpp"
#include "cpptestFixtures.hpp"

#include <gtest/gtest.h>
#include <thread>

struct timing_tests: public FederateTestFixture_cpp, public ::testing::Test {
};

TEST_F(timing_tests, barrier1)
{
    SetupTest<helicscpp::ValueFederate>("test_2", 2);
    auto vFed1 = GetFederateAs<helicscpp::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helicscpp::ValueFederate>(1);

    brokers[0]->setTimeBarrier(2.0);
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();
    vFed1->requestTimeAsync(3.0);
    auto rtime = vFed2->requestTime(1.89);
    EXPECT_DOUBLE_EQ(rtime, 1.89);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_FALSE(vFed1->isAsyncOperationCompleted());
    brokers[0]->clearTimeBarrier();
    rtime = vFed1->requestTimeComplete();
    EXPECT_EQ(rtime, 3.0);
    vFed1->finalize();
    vFed2->finalize();
}

// Tests out the restrictive time policy
TEST_F(timing_tests, time_barrier_update)
{
    SetupTest<helicscpp::ValueFederate>("test_2", 2);
    auto vFed1 = GetFederateAs<helicscpp::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helicscpp::ValueFederate>(1);

    brokers[0]->setTimeBarrier(2.0);
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();
    vFed1->requestTimeAsync(3.0);
    auto rtime = vFed2->requestTime(1.89);
    EXPECT_DOUBLE_EQ(rtime, 1.89);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_FALSE(vFed1->isAsyncOperationCompleted());
    brokers[0]->setTimeBarrier(4.0);
    rtime = vFed1->requestTimeComplete();
    EXPECT_EQ(rtime, 3.0);

    vFed1->finalize();
    vFed2->finalize();
}
