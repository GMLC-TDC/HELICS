/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../src/helics/cpp98/Broker.hpp"
#include "../src/helics/cpp98/Core.hpp"
#include "../src/helics/cpp98/ValueFederate.hpp"
#include "cpptestFixtures.hpp"

#include <array>
#include <gtest/gtest.h>
#include <thread>

struct timing_tests: public FederateTestFixture_cpp, public ::testing::Test {};

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

// Tests out time barrier update
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

// Tests out time barrier update
TEST_F(timing_tests, time_barrier_update_command)
{
    SetupTest<helicscpp::ValueFederate>("test_2", 2);
    auto vFed1 = GetFederateAs<helicscpp::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helicscpp::ValueFederate>(1);

    vFed1->sendCommand("root", "set barrier 2.0");
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();
    vFed1->requestTimeAsync(3.0);
    auto rtime = vFed2->requestTime(1.89);
    EXPECT_DOUBLE_EQ(rtime, 1.89);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_FALSE(vFed1->isAsyncOperationCompleted());
    vFed1->sendCommand("root", "set barrier 4.0");
    rtime = vFed1->requestTimeComplete();
    EXPECT_EQ(rtime, 3.0);

    vFed1->finalize();
    vFed2->finalize();
}

#if defined(HELICS_HAS_FUNCTIONAL) && HELICS_HAS_FUNCTIONAL != 0
// Tests out the time update callback in cpp98
TEST_F(timing_tests, timeUpdateCallback)
{
    SetupTest<helicscpp::ValueFederate>("test_2", 2);
    auto vFed1 = GetFederateAs<helicscpp::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helicscpp::ValueFederate>(1);
    int updCall{0};
    auto tUpdate = [&](HelicsTime /*time*/, bool /*iterating*/) { ++updCall; };
    vFed1->setTimeUpdateCallback(tUpdate);
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();
    EXPECT_EQ(updCall, 1);
    vFed1->requestTimeAsync(3.0);
    auto rtime = vFed2->requestTime(1.89);
    EXPECT_DOUBLE_EQ(rtime, 1.89);
    rtime = vFed1->requestTimeComplete();
    EXPECT_EQ(rtime, 3.0);
    EXPECT_EQ(updCall, 2);
    vFed1->finalize();
    vFed2->finalize();
}

// Tests out all the callback sequencing in cpp98
TEST_F(timing_tests, callbackSequence)
{
    int sequence{1};
    std::array<int, 4> seqVal{{0, 0, 0, 0}};

    SetupTest<helicscpp::ValueFederate>("test_2", 1);
    auto vFed1 = GetFederateAs<helicscpp::ValueFederate>(0);

    auto timeRequestEntry = [&](auto /*v1*/, auto /*v2*/, auto /*v2*/) { seqVal[0] = sequence++; };
    auto stateUpdate = [&](auto /*v1*/, auto /*v2*/) { seqVal[1] = sequence++; };
    auto timeUpdate = [&](auto /*v1*/, auto /*v2*/) { seqVal[2] = sequence++; };
    auto timeRequestReturn = [&](auto /*v1*/, auto /*v2*/) { seqVal[3] = sequence++; };

    ASSERT_TRUE(vFed1);
    vFed1->setTimeUpdateCallback(timeUpdate);
    vFed1->setStateChangeCallback(stateUpdate);
    vFed1->setTimeRequestEntryCallback(timeRequestEntry);
    vFed1->setTimeRequestReturnCallback(timeRequestReturn);

    vFed1->enterExecutingMode();
    // state change callback would have been called twice
    EXPECT_EQ(seqVal[0], 0);  // should not have been called
    EXPECT_EQ(seqVal[1], 2);
    EXPECT_EQ(seqVal[2], 3);
    EXPECT_EQ(seqVal[3], 4);

    // reset the sequence counter
    sequence = 1;
    seqVal[0] = 99;
    seqVal[1] = 99;
    seqVal[2] = 99;
    seqVal[3] = 99;
    vFed1->requestTime(4.0);

    EXPECT_EQ(seqVal[0], 1);
    EXPECT_EQ(seqVal[1], 99);
    EXPECT_EQ(seqVal[2], 2);
    EXPECT_EQ(seqVal[3], 3);

    vFed1->finalize();
    EXPECT_EQ(seqVal[1], 4);
}

#endif
