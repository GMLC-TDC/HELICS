/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "gmlc/libguarded/atomic_guarded.hpp"
#include "helics/core/MessageTimer.hpp"

#include "gtest/gtest.h"
using namespace helics;

using namespace std::literals::chrono_literals;

TEST(messageTimer_tests, basic_test)
{
    auto contextPtr = AsioContextManager::getContextPointer();
    auto loopHandle = contextPtr->startContextLoop();
    if (!contextPtr->isRunning()) {
        std::this_thread::sleep_for(600ms);
    }
    auto loopHandle2 = contextPtr->startContextLoop();
    if (!contextPtr->isRunning()) {
        std::this_thread::sleep_for(600ms);
    }
    // if this fails there is a separate error path that this doesn't account for
    EXPECT_TRUE(contextPtr->isRunning());
    gmlc::libguarded::atomic_guarded<ActionMessage> M;
    auto cback = [&M](ActionMessage&& m) { M = std::move(m); };
    auto mtimer = std::make_shared<MessageTimer>(cback);
    std::this_thread::yield();  // just get the loop started
    auto index = mtimer->addTimerFromNow(200ms, CMD_PROTOCOL);
    EXPECT_EQ(index, 0);
    EXPECT_TRUE(M.load().action() == CMD_IGNORE);
    std::this_thread::sleep_for(300ms);
    if (M.load().action() != CMD_PROTOCOL) {
        std::this_thread::sleep_for(300ms);
    }
    if (M.load().action() != CMD_PROTOCOL) {
        std::cout << "waiting Again" << std::endl;
        std::this_thread::sleep_for(300ms);
    }
    if (M.load().action() != CMD_PROTOCOL) {
        std::this_thread::sleep_for(300ms);
    }
    auto tm = M.load();
    EXPECT_TRUE(tm.action() == CMD_PROTOCOL) << "current = " << prettyPrintString(tm);
    if (tm.action() != CMD_PROTOCOL) {
        mtimer->cancelAll();
    }
}
TEST(messageTimer_tests, shorttime_test)
{
    gmlc::libguarded::atomic_guarded<ActionMessage> M;
    auto cback = [&](ActionMessage&& m) { M = std::move(m); };
    auto mtimer = std::make_shared<MessageTimer>(cback);
    auto index = mtimer->addTimerFromNow(0ms, CMD_PROTOCOL);
    EXPECT_EQ(index, 0);

    std::this_thread::sleep_for(5ms);
    auto tm = M.load();
    EXPECT_TRUE(tm.action() == CMD_PROTOCOL) << "current = " << prettyPrintString(tm);
    if (tm.action() != CMD_PROTOCOL) {
        mtimer->cancelAll();
    }
}

TEST(messageTimer_tests_ci_skip, basic_test_update)
{
    std::mutex mlock;
    helics::ActionMessage M;
    auto cback = [&](ActionMessage&& m) {
        std::lock_guard<std::mutex> locker(mlock);
        M = std::move(m);
    };
    auto mtimer = std::make_shared<MessageTimer>(cback);
    std::unique_lock<std::mutex> localLock(mlock);

    auto index = mtimer->addTimerFromNow(500ms, CMD_PROTOCOL);
    EXPECT_TRUE(M.action() == CMD_IGNORE);
    localLock.unlock();
    std::this_thread::sleep_for(100ms);

    auto res = mtimer->addTimeToTimer(index, 300ms);
    EXPECT_TRUE(res);
    std::this_thread::sleep_for(500ms);
    localLock.lock();
    EXPECT_TRUE(M.action() == CMD_IGNORE);
    localLock.unlock();
    std::this_thread::sleep_for(300ms);
    localLock.lock();
    EXPECT_TRUE(M.action() == CMD_PROTOCOL);
}

TEST(messageTimer_tests_ci_skip, basic_test_multiple)
{
    std::atomic<int> counter{0};
    auto cback = [&counter](helics::ActionMessage&&) { ++counter; };
    auto mtimer = std::make_shared<helics::MessageTimer>(cback);

    mtimer->addTimerFromNow(200ms, helics::CMD_PROTOCOL);
    mtimer->addTimerFromNow(400ms, helics::CMD_PROTOCOL);
    mtimer->addTimerFromNow(600ms, helics::CMD_PROTOCOL);
    mtimer->addTimerFromNow(800ms, helics::CMD_PROTOCOL);
    std::this_thread::sleep_for(50ms);
    EXPECT_EQ(counter.load(), 0);
    std::this_thread::sleep_for(200ms);
    EXPECT_EQ(counter.load(), 1);
    std::this_thread::sleep_for(200ms);
    EXPECT_EQ(counter.load(), 2);
    std::this_thread::sleep_for(200ms);
    EXPECT_EQ(counter.load(), 3);
    std::this_thread::sleep_for(200ms);
    EXPECT_EQ(counter.load(), 4);
}

TEST(messageTimer_tests_ci_skip, basic_test_multiple_alt)
{
    std::atomic<int> counter{0};
    auto cback = [&counter](helics::ActionMessage&&) { ++counter; };
    auto mtimer = std::make_shared<helics::MessageTimer>(cback);

    auto ctime = std::chrono::steady_clock::now();
    mtimer->addTimer(ctime + 200ms, helics::CMD_PROTOCOL);
    mtimer->addTimer(ctime + 400ms, helics::CMD_PROTOCOL);
    mtimer->addTimer(ctime + 600ms, helics::CMD_PROTOCOL);
    mtimer->addTimer(ctime + 800ms, helics::CMD_PROTOCOL);
    std::this_thread::sleep_until(ctime + 100ms);
    EXPECT_EQ(counter.load(), 0);
    std::this_thread::sleep_until(ctime + 300ms);
    EXPECT_EQ(counter.load(), 1);
    std::this_thread::sleep_until(ctime + 500ms);
    EXPECT_EQ(counter.load(), 2);
    std::this_thread::sleep_until(ctime + 700ms);
    EXPECT_EQ(counter.load(), 3);
    std::this_thread::sleep_until(ctime + 900ms);
    EXPECT_EQ(counter.load(), 4);
}

TEST(messageTimer_tests_ci_skip, basic_test_multiple_cancel)
{
    std::atomic<int> counter{0};
    auto cback = [&counter](helics::ActionMessage&&) { ++counter; };
    auto mtimer = std::make_shared<helics::MessageTimer>(cback);

    mtimer->addTimerFromNow(200ms, helics::CMD_PROTOCOL);
    mtimer->addTimerFromNow(400ms, helics::CMD_PROTOCOL);
    mtimer->addTimerFromNow(600ms, helics::CMD_PROTOCOL);
    mtimer->addTimerFromNow(800ms, helics::CMD_PROTOCOL);
    std::this_thread::sleep_for(50ms);
    EXPECT_EQ(counter.load(), 0);
    std::this_thread::sleep_for(200ms);
    EXPECT_EQ(counter.load(), 1);
    std::this_thread::sleep_for(200ms);
    EXPECT_EQ(counter.load(), 2);
    mtimer->cancelAll();
    std::this_thread::sleep_for(200ms);
    EXPECT_EQ(counter.load(), 2);
    std::this_thread::sleep_for(200ms);
    EXPECT_EQ(counter.load(), 2);
}

TEST(messageTimer_tests_ci_skip, basic_test_multiple_change_time)
{
    std::atomic<int> counter{0};
    auto cback = [&counter](helics::ActionMessage&&) { ++counter; };
    auto mtimer = std::make_shared<helics::MessageTimer>(cback);

    mtimer->addTimerFromNow(200ms, helics::CMD_PROTOCOL);
    auto t2 = mtimer->addTimerFromNow(400ms, helics::CMD_PROTOCOL);
    auto t3 = mtimer->addTimerFromNow(600ms, helics::CMD_PROTOCOL);
    mtimer->addTimerFromNow(800ms, helics::CMD_PROTOCOL);
    std::this_thread::sleep_for(50ms);
    EXPECT_EQ(counter.load(), 0);
    std::this_thread::sleep_for(200ms);
    EXPECT_EQ(counter.load(), 1);
    mtimer->addTimeToTimer(t2, 400ms);
    mtimer->addTimeToTimer(t3, 200ms);
    std::this_thread::sleep_for(200ms);
    EXPECT_EQ(counter.load(), 1);
    std::this_thread::sleep_for(200ms);
    EXPECT_EQ(counter.load(), 1);
    std::this_thread::sleep_for(200ms);
    EXPECT_EQ(counter.load(), 4);
}

TEST(messageTimer_tests_ci_skip, basic_test_multiple_change_time2)
{
    std::atomic<int> counter{0};
    auto cback = [&counter](helics::ActionMessage&&) { ++counter; };
    auto mtimer = std::make_shared<helics::MessageTimer>(cback);
    auto ctime = std::chrono::steady_clock::now();
    mtimer->addTimerFromNow(200ms, helics::CMD_PROTOCOL);
    auto t2 = mtimer->addTimerFromNow(400ms, helics::CMD_PROTOCOL);
    auto t3 = mtimer->addTimerFromNow(600ms, helics::CMD_PROTOCOL);
    auto t4 = mtimer->addTimerFromNow(800ms, helics::CMD_PROTOCOL);
    std::this_thread::sleep_for(50ms);
    EXPECT_EQ(counter.load(), 0);
    std::this_thread::sleep_for(200ms);
    EXPECT_EQ(counter.load(), 1);
    mtimer->updateTimer(t2, ctime + 800ms);
    mtimer->updateTimer(t3, ctime + 800ms);
    std::this_thread::sleep_for(200ms);
    EXPECT_EQ(counter.load(), 1);
    std::this_thread::sleep_for(200ms);
    mtimer->cancelTimer(t4);
    EXPECT_EQ(counter.load(), 1);
    std::this_thread::sleep_for(200ms);
    EXPECT_EQ(counter.load(), 3);
}

TEST(messageTimer_tests_ci_skip, basic_test_updatemessage)
{
    std::mutex mlock;
    helics::ActionMessage M;
    auto cback = [&](ActionMessage&& m) {
        std::lock_guard<std::mutex> locker(mlock);
        M = std::move(m);
    };
    auto mtimer = std::make_shared<MessageTimer>(cback);
    auto ctime = std::chrono::steady_clock::now();
    std::unique_lock<std::mutex> localLock(mlock);

    auto index = mtimer->addTimer(ctime + 400ms, CMD_PROTOCOL);
    EXPECT_TRUE(M.action() == CMD_IGNORE);
    localLock.unlock();
    std::this_thread::sleep_for(100ms);

    mtimer->updateTimer(index, ctime + 700ms, CMD_BROKER_ACK);
    std::this_thread::sleep_for(400ms);
    localLock.lock();
    EXPECT_TRUE(M.action() == CMD_IGNORE);
    localLock.unlock();
    std::this_thread::sleep_for(300ms);
    localLock.lock();
    EXPECT_TRUE(M.action() == CMD_BROKER_ACK);
}

TEST(messageTimer_tests_ci_skip, basic_test_updatemessage2)
{
    std::mutex mlock;
    helics::ActionMessage M;
    auto cback = [&](ActionMessage&& m) {
        std::lock_guard<std::mutex> locker(mlock);
        M = std::move(m);
    };
    auto mtimer = std::make_shared<MessageTimer>(cback);
    auto ctime = std::chrono::steady_clock::now();
    std::unique_lock<std::mutex> localLock(mlock);

    auto index = mtimer->addTimer(ctime + 400ms, CMD_PROTOCOL);
    EXPECT_TRUE(M.action() == CMD_IGNORE);
    localLock.unlock();
    std::this_thread::sleep_for(100ms);

    mtimer->updateMessage(index, CMD_BROKER_ACK);
    std::this_thread::sleep_for(400ms);
    localLock.lock();
    EXPECT_TRUE(M.action() == CMD_BROKER_ACK);
}
