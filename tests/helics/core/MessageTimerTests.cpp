/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "gtest/gtest.h"

#include "helics/core/MessageTimer.hpp"
#include "libguarded/atomic_guarded.hpp"
using namespace helics;

TEST (messageTimer_tests, basic_test)
{
    libguarded::atomic_guarded<ActionMessage> M;
    auto cback = [&](ActionMessage &&m) { M = std::move (m); };
    auto mtimer = std::make_shared<MessageTimer> (cback);
    std::this_thread::yield ();  // just get the loop started
    mtimer->addTimerFromNow (std::chrono::milliseconds (200), CMD_PROTOCOL);
    EXPECT_TRUE (M.load ().action () == CMD_IGNORE);
    std::this_thread::sleep_for (std::chrono::milliseconds (300));
    if (M.load ().action () != CMD_PROTOCOL)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (500));
    }
    auto tm = M.load ();
    EXPECT_TRUE (tm.action () == CMD_PROTOCOL) << tm;
}

TEST (messageTimer_tests_skip_ci, basic_test_update)
{
    std::mutex mlock;
    helics::ActionMessage M;
    auto cback = [&](ActionMessage &&m) {
        std::lock_guard<std::mutex> locker (mlock);
        M = std::move (m);
    };
    auto mtimer = std::make_shared<MessageTimer> (cback);
    std::unique_lock<std::mutex> localLock (mlock);

    auto index = mtimer->addTimerFromNow (std::chrono::milliseconds (500), CMD_PROTOCOL);
    EXPECT_TRUE (M.action () == CMD_IGNORE);
    localLock.unlock ();
    std::this_thread::sleep_for (std::chrono::milliseconds (100));

    auto res = mtimer->addTimeToTimer (index, std::chrono::milliseconds (300));
    EXPECT_TRUE (res);
    std::this_thread::sleep_for (std::chrono::milliseconds (500));
    localLock.lock ();
    EXPECT_TRUE (M.action () == CMD_IGNORE);
    localLock.unlock ();
    std::this_thread::sleep_for (std::chrono::milliseconds (300));
    localLock.lock ();
    EXPECT_TRUE (M.action () == CMD_PROTOCOL);
}

TEST (messageTimer_tests_skip_ci, basic_test_multiple)
{
    std::atomic<int> counter{0};
    auto cback = [&counter](helics::ActionMessage &&) { ++counter; };
    auto mtimer = std::make_shared<helics::MessageTimer> (cback);

    mtimer->addTimerFromNow (std::chrono::milliseconds (200), helics::CMD_PROTOCOL);
    mtimer->addTimerFromNow (std::chrono::milliseconds (400), helics::CMD_PROTOCOL);
    mtimer->addTimerFromNow (std::chrono::milliseconds (600), helics::CMD_PROTOCOL);
    mtimer->addTimerFromNow (std::chrono::milliseconds (800), helics::CMD_PROTOCOL);
    std::this_thread::sleep_for (std::chrono::milliseconds (50));
    EXPECT_EQ (counter.load (), 0);
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    EXPECT_EQ (counter.load (), 1);
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    EXPECT_EQ (counter.load (), 2);
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    EXPECT_EQ (counter.load (), 3);
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    EXPECT_EQ (counter.load (), 4);
}

TEST (messageTimer_tests_skip_ci, basic_test_multiple_alt)
{
    std::atomic<int> counter{0};
    auto cback = [&counter](helics::ActionMessage &&) { ++counter; };
    auto mtimer = std::make_shared<helics::MessageTimer> (cback);

    auto ctime = std::chrono::steady_clock::now ();
    mtimer->addTimer (ctime + std::chrono::milliseconds (200), helics::CMD_PROTOCOL);
    mtimer->addTimer (ctime + std::chrono::milliseconds (400), helics::CMD_PROTOCOL);
    mtimer->addTimer (ctime + std::chrono::milliseconds (600), helics::CMD_PROTOCOL);
    mtimer->addTimer (ctime + std::chrono::milliseconds (800), helics::CMD_PROTOCOL);
    std::this_thread::sleep_until (ctime + std::chrono::milliseconds (100));
    EXPECT_EQ (counter.load (), 0);
    std::this_thread::sleep_until (ctime + std::chrono::milliseconds (300));
    EXPECT_EQ (counter.load (), 1);
    std::this_thread::sleep_until (ctime + std::chrono::milliseconds (500));
    EXPECT_EQ (counter.load (), 2);
    std::this_thread::sleep_until (ctime + std::chrono::milliseconds (700));
    EXPECT_EQ (counter.load (), 3);
    std::this_thread::sleep_until (ctime + std::chrono::milliseconds (900));
    EXPECT_EQ (counter.load (), 4);
}

TEST (messageTimer_tests_skip_ci, basic_test_multiple_cancel)
{
    std::atomic<int> counter{0};
    auto cback = [&counter](helics::ActionMessage &&) { ++counter; };
    auto mtimer = std::make_shared<helics::MessageTimer> (cback);

    mtimer->addTimerFromNow (std::chrono::milliseconds (200), helics::CMD_PROTOCOL);
    mtimer->addTimerFromNow (std::chrono::milliseconds (400), helics::CMD_PROTOCOL);
    mtimer->addTimerFromNow (std::chrono::milliseconds (600), helics::CMD_PROTOCOL);
    mtimer->addTimerFromNow (std::chrono::milliseconds (800), helics::CMD_PROTOCOL);
    std::this_thread::sleep_for (std::chrono::milliseconds (50));
    EXPECT_EQ (counter.load (), 0);
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    EXPECT_EQ (counter.load (), 1);
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    EXPECT_EQ (counter.load (), 2);
    mtimer->cancelAll ();
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    EXPECT_EQ (counter.load (), 2);
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    EXPECT_EQ (counter.load (), 2);
}

TEST (messageTimer_tests_skip_ci, basic_test_multiple_change_time)
{
    std::atomic<int> counter{0};
    auto cback = [&counter](helics::ActionMessage &&) { ++counter; };
    auto mtimer = std::make_shared<helics::MessageTimer> (cback);

    mtimer->addTimerFromNow (std::chrono::milliseconds (200), helics::CMD_PROTOCOL);
    auto t2 = mtimer->addTimerFromNow (std::chrono::milliseconds (400), helics::CMD_PROTOCOL);
    auto t3 = mtimer->addTimerFromNow (std::chrono::milliseconds (600), helics::CMD_PROTOCOL);
    mtimer->addTimerFromNow (std::chrono::milliseconds (800), helics::CMD_PROTOCOL);
    std::this_thread::sleep_for (std::chrono::milliseconds (50));
    EXPECT_EQ (counter.load (), 0);
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    EXPECT_EQ (counter.load (), 1);
    mtimer->addTimeToTimer (t2, std::chrono::milliseconds (400));
    mtimer->addTimeToTimer (t3, std::chrono::milliseconds (200));
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    EXPECT_EQ (counter.load (), 1);
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    EXPECT_EQ (counter.load (), 1);
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    EXPECT_EQ (counter.load (), 4);
}

TEST (messageTimer_tests_skip_ci, basic_test_multiple_change_time2)
{
    std::atomic<int> counter{0};
    auto cback = [&counter](helics::ActionMessage &&) { ++counter; };
    auto mtimer = std::make_shared<helics::MessageTimer> (cback);
    auto ctime = std::chrono::steady_clock::now ();
    mtimer->addTimerFromNow (std::chrono::milliseconds (200), helics::CMD_PROTOCOL);
    auto t2 = mtimer->addTimerFromNow (std::chrono::milliseconds (400), helics::CMD_PROTOCOL);
    auto t3 = mtimer->addTimerFromNow (std::chrono::milliseconds (600), helics::CMD_PROTOCOL);
    auto t4 = mtimer->addTimerFromNow (std::chrono::milliseconds (800), helics::CMD_PROTOCOL);
    std::this_thread::sleep_for (std::chrono::milliseconds (50));
    EXPECT_EQ (counter.load (), 0);
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    EXPECT_EQ (counter.load (), 1);
    mtimer->updateTimer (t2, ctime + std::chrono::milliseconds (800));
    mtimer->updateTimer (t3, ctime + std::chrono::milliseconds (800));
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    EXPECT_EQ (counter.load (), 1);
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    mtimer->cancelTimer (t4);
    EXPECT_EQ (counter.load (), 1);
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    EXPECT_EQ (counter.load (), 3);
}

TEST (messageTimer_tests_skip_ci, basic_test_updatemessage)
{
    std::mutex mlock;
    helics::ActionMessage M;
    auto cback = [&](ActionMessage &&m) {
        std::lock_guard<std::mutex> locker (mlock);
        M = std::move (m);
    };
    auto mtimer = std::make_shared<MessageTimer> (cback);
    auto ctime = std::chrono::steady_clock::now ();
    std::unique_lock<std::mutex> localLock (mlock);

    auto index = mtimer->addTimer (ctime + std::chrono::milliseconds (400), CMD_PROTOCOL);
    EXPECT_TRUE (M.action () == CMD_IGNORE);
    localLock.unlock ();
    std::this_thread::sleep_for (std::chrono::milliseconds (100));

    mtimer->updateTimer (index, ctime + std::chrono::milliseconds (700), CMD_BROKER_ACK);
    std::this_thread::sleep_for (std::chrono::milliseconds (400));
    localLock.lock ();
    EXPECT_TRUE (M.action () == CMD_IGNORE);
    localLock.unlock ();
    std::this_thread::sleep_for (std::chrono::milliseconds (300));
    localLock.lock ();
    EXPECT_TRUE (M.action () == CMD_BROKER_ACK);
}

TEST (messageTimer_tests_skip_ci, basic_test_updatemessage2)
{
    std::mutex mlock;
    helics::ActionMessage M;
    auto cback = [&](ActionMessage &&m) {
        std::lock_guard<std::mutex> locker (mlock);
        M = std::move (m);
    };
    auto mtimer = std::make_shared<MessageTimer> (cback);
    auto ctime = std::chrono::steady_clock::now ();
    std::unique_lock<std::mutex> localLock (mlock);

    auto index = mtimer->addTimer (ctime + std::chrono::milliseconds (400), CMD_PROTOCOL);
    EXPECT_TRUE (M.action () == CMD_IGNORE);
    localLock.unlock ();
    std::this_thread::sleep_for (std::chrono::milliseconds (100));

    mtimer->updateMessage (index, CMD_BROKER_ACK);
    std::this_thread::sleep_for (std::chrono::milliseconds (400));
    localLock.lock ();
    EXPECT_TRUE (M.action () == CMD_BROKER_ACK);
}
