/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "testFixtures.h"
#include <boost/test/unit_test.hpp>

#include "helics/core/MessageTimer.hpp"

BOOST_AUTO_TEST_SUITE (messageTimer_tests)

using namespace helics;
namespace utf = boost::unit_test;

BOOST_AUTO_TEST_CASE (basic_test, *utf::label("ci"))
{
    std::mutex mlock;
    ActionMessage M;
    auto cback = [&](ActionMessage &&m) {
        std::lock_guard<std::mutex> locker (mlock);
        M = std::move (m);
    };
    std::cout << "making timer" << std::endl;
    auto mtimer = std::make_shared<MessageTimer> (cback,true);
    std::cout << "time made" << std::endl;
    std::unique_lock<std::mutex> localLock (mlock);
    std::cout << "adding timeout" << std::endl;
    mtimer->addTimerFromNow (std::chrono::milliseconds (200), CMD_PROTOCOL);
    BOOST_CHECK (M.action () == CMD_IGNORE);
    localLock.unlock ();
    std::cout << "sleeping" << std::endl;
    std::this_thread::sleep_for (std::chrono::milliseconds (300));
    std::cout << "checking" << std::endl;
    localLock.lock();
    std::cout << "locked final check" << std::endl;
    BOOST_CHECK (M.action () == CMD_PROTOCOL);
}

BOOST_AUTO_TEST_CASE (basic_test_update)
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
    BOOST_CHECK (M.action () == CMD_IGNORE);
    localLock.unlock ();
    std::this_thread::sleep_for (std::chrono::milliseconds (100));

    auto res = mtimer->addTimeToTimer (index, std::chrono::milliseconds (300));
    BOOST_CHECK (res);
    std::this_thread::sleep_for (std::chrono::milliseconds (500));
    localLock.lock();
    BOOST_CHECK (M.action () == CMD_IGNORE);
    localLock.unlock();
    std::this_thread::sleep_for (std::chrono::milliseconds (300));
    localLock.lock();
    BOOST_CHECK (M.action () == CMD_PROTOCOL);
}

BOOST_AUTO_TEST_CASE (basic_test_multiple)
{
    std::atomic<int> counter{0};
    auto cback = [&counter](helics::ActionMessage &&) { ++counter; };
    auto mtimer = std::make_shared<helics::MessageTimer> (cback);

    mtimer->addTimerFromNow (std::chrono::milliseconds (200), helics::CMD_PROTOCOL);
    mtimer->addTimerFromNow (std::chrono::milliseconds (400), helics::CMD_PROTOCOL);
    mtimer->addTimerFromNow (std::chrono::milliseconds (600), helics::CMD_PROTOCOL);
    mtimer->addTimerFromNow (std::chrono::milliseconds (800), helics::CMD_PROTOCOL);
    std::this_thread::sleep_for (std::chrono::milliseconds (50));
    BOOST_CHECK_EQUAL (counter.load (), 0);
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    BOOST_CHECK_EQUAL (counter.load (), 1);
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    BOOST_CHECK_EQUAL (counter.load (), 2);
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    BOOST_CHECK_EQUAL (counter.load (), 3);
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    BOOST_CHECK_EQUAL (counter.load (), 4);
}

BOOST_AUTO_TEST_CASE (basic_test_multiple_alt)
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
    BOOST_CHECK_EQUAL (counter.load (), 0);
    std::this_thread::sleep_until (ctime + std::chrono::milliseconds (300));
    BOOST_CHECK_EQUAL (counter.load (), 1);
    std::this_thread::sleep_until (ctime + std::chrono::milliseconds (500));
    BOOST_CHECK_EQUAL (counter.load (), 2);
    std::this_thread::sleep_until (ctime + std::chrono::milliseconds (700));
    BOOST_CHECK_EQUAL (counter.load (), 3);
    std::this_thread::sleep_until (ctime + std::chrono::milliseconds (900));
    BOOST_CHECK_EQUAL (counter.load (), 4);
}

BOOST_AUTO_TEST_CASE (basic_test_multiple_cancel)
{
    std::atomic<int> counter{0};
    auto cback = [&counter](helics::ActionMessage &&) { ++counter; };
    auto mtimer = std::make_shared<helics::MessageTimer> (cback);

    mtimer->addTimerFromNow (std::chrono::milliseconds (200), helics::CMD_PROTOCOL);
    mtimer->addTimerFromNow (std::chrono::milliseconds (400), helics::CMD_PROTOCOL);
    mtimer->addTimerFromNow (std::chrono::milliseconds (600), helics::CMD_PROTOCOL);
    mtimer->addTimerFromNow (std::chrono::milliseconds (800), helics::CMD_PROTOCOL);
    std::this_thread::sleep_for (std::chrono::milliseconds (50));
    BOOST_CHECK_EQUAL (counter.load (), 0);
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    BOOST_CHECK_EQUAL (counter.load (), 1);
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    BOOST_CHECK_EQUAL (counter.load (), 2);
    mtimer->cancelAll ();
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    BOOST_CHECK_EQUAL (counter.load (), 2);
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    BOOST_CHECK_EQUAL (counter.load (), 2);
}

BOOST_AUTO_TEST_CASE (basic_test_multiple_change_time)
{
    std::atomic<int> counter{0};
    auto cback = [&counter](helics::ActionMessage &&) { ++counter; };
    auto mtimer = std::make_shared<helics::MessageTimer> (cback);

    mtimer->addTimerFromNow (std::chrono::milliseconds (200), helics::CMD_PROTOCOL);
    auto t2 = mtimer->addTimerFromNow (std::chrono::milliseconds (400), helics::CMD_PROTOCOL);
    auto t3 = mtimer->addTimerFromNow (std::chrono::milliseconds (600), helics::CMD_PROTOCOL);
    mtimer->addTimerFromNow (std::chrono::milliseconds (800), helics::CMD_PROTOCOL);
    std::this_thread::sleep_for (std::chrono::milliseconds (50));
    BOOST_CHECK_EQUAL (counter.load (), 0);
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    BOOST_CHECK_EQUAL (counter.load (), 1);
    mtimer->addTimeToTimer (t2, std::chrono::milliseconds (400));
    mtimer->addTimeToTimer (t3, std::chrono::milliseconds (200));
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    BOOST_CHECK_EQUAL (counter.load (), 1);
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    BOOST_CHECK_EQUAL (counter.load (), 1);
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    BOOST_CHECK_EQUAL (counter.load (), 4);
}

BOOST_AUTO_TEST_CASE (basic_test_multiple_change_time2)
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
    BOOST_CHECK_EQUAL (counter.load (), 0);
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    BOOST_CHECK_EQUAL (counter.load (), 1);
    mtimer->updateTimer (t2, ctime + std::chrono::milliseconds (800));
    mtimer->updateTimer (t3, ctime + std::chrono::milliseconds (800));
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    BOOST_CHECK_EQUAL (counter.load (), 1);
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    mtimer->cancelTimer (t4);
    BOOST_CHECK_EQUAL (counter.load (), 1);
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    BOOST_CHECK_EQUAL (counter.load (), 3);
}

BOOST_AUTO_TEST_CASE (basic_test_updatemessage)
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
    BOOST_CHECK (M.action () == CMD_IGNORE);
    localLock.unlock ();
    std::this_thread::sleep_for (std::chrono::milliseconds (100));

    mtimer->updateTimer (index, ctime + std::chrono::milliseconds (700), CMD_BROKER_ACK);
    std::this_thread::sleep_for (std::chrono::milliseconds (400));
    BOOST_CHECK (M.action () == CMD_IGNORE);
    std::this_thread::sleep_for (std::chrono::milliseconds (300));
    BOOST_CHECK (M.action () == CMD_BROKER_ACK);
}

BOOST_AUTO_TEST_CASE (basic_test_updatemessage2)
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
    BOOST_CHECK (M.action () == CMD_IGNORE);
    localLock.unlock ();
    std::this_thread::sleep_for (std::chrono::milliseconds (100));

    mtimer->updateMessage (index, CMD_BROKER_ACK);
    std::this_thread::sleep_for (std::chrono::milliseconds (400));
    BOOST_CHECK (M.action () == CMD_BROKER_ACK);
}

BOOST_AUTO_TEST_SUITE_END ()
