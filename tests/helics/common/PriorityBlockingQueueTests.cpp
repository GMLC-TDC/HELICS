/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <future>
#include <memory>
#include <thread>
#include <utility>
/** these test cases test data_block and data_view objects
 */

#include "helics/common/BlockingPriorityQueue.hpp"

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE (blocking_priority_queue_tests, *utf::label("daily") *utf::label("release"))

/** test basic operations */
BOOST_AUTO_TEST_CASE (basic_tests, *utf::label("ci"))
{
    BlockingPriorityQueue<int> sq;

    sq.push (45);
    sq.push (54);

    BOOST_CHECK (!sq.empty ());

    auto b = sq.try_pop ();
    BOOST_CHECK_EQUAL (*b, 45);
    b = sq.try_pop ();
    BOOST_CHECK_EQUAL (*b, 54);

    b = sq.try_pop ();
    BOOST_CHECK (!(b));
    BOOST_CHECK (sq.empty ());

    sq.push (45);
    sq.push (54);

    // check the prioritization is working
    sq.pushPriority (65);

    b = sq.try_pop ();
    BOOST_CHECK_EQUAL (*b, 65);
    b = sq.try_pop ();
    BOOST_CHECK_EQUAL (*b, 45);
}

/** test with a move only element*/
BOOST_AUTO_TEST_CASE (move_only_tests, *utf::label("ci"))
{
    BlockingPriorityQueue<std::unique_ptr<double>> sq;

    sq.push (std::make_unique<double> (4534.23));

    auto e2 = std::make_unique<double> (34.234);
    sq.push (std::move (e2));

    BOOST_CHECK (!sq.empty ());

    auto b = sq.try_pop ();
    BOOST_CHECK_EQUAL (**b, 4534.23);
    b = sq.try_pop ();
    BOOST_CHECK_EQUAL (**b, 34.234);
    e2 = std::make_unique<double> (29.785);
    sq.pushPriority (std::move (e2));

    b = sq.try_pop ();
    BOOST_CHECK_EQUAL (**b, 29.785);
    b = sq.try_pop ();
    BOOST_CHECK (!(b));
    BOOST_CHECK (sq.empty ());
}

/** test the ordering with a larger number of inputs*/

BOOST_AUTO_TEST_CASE (ordering_tests, *utf::label("ci"))
{
    BlockingPriorityQueue<int> sq;

    for (int ii = 1; ii < 10; ++ii)
    {
        sq.push (ii);
    }

    auto b = sq.try_pop ();
    BOOST_CHECK_EQUAL (*b, 1);
    for (int ii = 2; ii < 7; ++ii)
    {
        b = sq.try_pop ();
        BOOST_CHECK_EQUAL (*b, ii);
    }
    for (int ii = 10; ii < 20; ++ii)
    {
        sq.push (ii);
    }
    sq.pushPriority (99);
    b = sq.try_pop ();
    BOOST_CHECK_EQUAL (*b, 99);
    for (int ii = 7; ii < 20; ++ii)
    {
        b = sq.try_pop ();
        BOOST_CHECK_EQUAL (*b, ii);
    }

    BOOST_CHECK (sq.empty ());
}

BOOST_AUTO_TEST_CASE (emplace_tests, *utf::label("ci"))
{
    BlockingPriorityQueue<std::pair<int, double>> sq;

    sq.emplace (10, 45.4);
    sq.emplace (11, 34.1);
    sq.emplace (12, 34.2);
    sq.emplacePriority (14, 19.99);

    auto b = sq.try_pop ();
    BOOST_CHECK_EQUAL (b->first, 14);
    BOOST_CHECK_EQUAL (b->second, 19.99);

    b = sq.try_pop ();
    BOOST_CHECK_EQUAL (b->first, 10);
    BOOST_CHECK_EQUAL (b->second, 45.4);
    b = sq.try_pop ();
    BOOST_CHECK_EQUAL (b->first, 11);
    BOOST_CHECK_EQUAL (b->second, 34.1);
}

/** test with single consumer/single producer*/
BOOST_AUTO_TEST_CASE (multithreaded_tests)
{
    BlockingPriorityQueue<int64_t> sq (1010000);

    for (int64_t ii = 0; ii < 10'000; ++ii)
    {
        sq.push (ii);
    }
    auto prod1 = [&]() {
        for (int64_t ii = 10'000; ii < 1'010'000; ++ii)
        {
            sq.push (ii);
        }
    };

    auto cons = [&]() {
        auto res = sq.try_pop ();
        int64_t cnt = 0;
        while ((res))
        {
            ++cnt;
            res = sq.try_pop ();
            if (!res)
            {  // make an additional sleep period so the producer can catch up
                std::this_thread::sleep_for (std::chrono::milliseconds (100));
                res = sq.try_pop ();
            }
        }
        return cnt;
    };

    auto ret = std::async (std::launch::async, prod1);

    auto res = std::async (std::launch::async, cons);

    ret.wait ();
    auto V = res.get ();
    BOOST_CHECK_EQUAL (V, 1'010'000);
}

/** test with single consumer / single producer */
BOOST_AUTO_TEST_CASE (pop_tests)
{
    BlockingPriorityQueue<int64_t> sq (1010000);

    auto prod1 = [&]() {
        for (int64_t ii = 0; ii < 1'000'000; ++ii)
        {
            sq.push (ii);
        }
        sq.push (-1);
    };

    auto cons = [&]() {
        auto res = sq.pop ();
        int64_t cnt = 1;
        while (res >= 0)
        {
            auto nres = sq.pop ();
            if (nres > res)
            {
                ++cnt;
            }
            else
            {
                if (nres > 0)
                {
                    printf ("%d came before %d\n", static_cast<int> (nres), static_cast<int> (res));
                }
            }
            res = nres;
        }
        return cnt;
    };

    auto ret = std::async (std::launch::async, prod1);

    auto res = std::async (std::launch::async, cons);

    ret.wait ();
    auto V = res.get ();
    BOOST_CHECK_EQUAL (V, 1'000'000);
}

/** test with multiple consumer/single producer*/
BOOST_AUTO_TEST_CASE (multithreaded_tests2)
{
    BlockingPriorityQueue<int64_t> sq (1010000);

    for (int64_t ii = 0; ii < 10'000; ++ii)
    {
        sq.push (ii);
    }
    auto prod1 = [&]() {
        for (int64_t ii = 10'000; ii < 2'010'000; ++ii)
        {
            sq.push (ii);
        }
    };

    auto cons = [&]() {
        auto res = sq.try_pop ();
        int64_t cnt = 0;
        while ((res))
        {
            ++cnt;
            res = sq.try_pop ();
            if (!res)
            {  // make an additional sleep period so the producer can catch up
                std::this_thread::sleep_for (std::chrono::milliseconds (100));
                res = sq.try_pop ();
            }
        }
        return cnt;
    };

    auto ret = std::async (std::launch::async, prod1);

    auto res1 = std::async (std::launch::async, cons);
    auto res2 = std::async (std::launch::async, cons);
    auto res3 = std::async (std::launch::async, cons);
    ret.wait ();
    auto V1 = res1.get ();
    auto V2 = res2.get ();
    auto V3 = res3.get ();

    BOOST_CHECK_EQUAL (V1 + V2 + V3, 2'010'000);
}

/** test with multiple producer/multiple consumer*/
BOOST_AUTO_TEST_CASE (multithreaded_tests3)
{
    BlockingPriorityQueue<int64_t> sq;
    sq.reserve (3'010'000);
    for (int64_t ii = 0; ii < 10'000; ++ii)
    {
        sq.push (ii);
    }
    auto prod1 = [&]() {
        for (int64_t ii = 0; ii < 1'000'000; ++ii)
        {
            sq.push (ii);
        }
    };

    auto cons = [&]() {
        auto res = sq.try_pop ();
        int64_t cnt = 0;
        while ((res))
        {
            ++cnt;
            res = sq.try_pop ();
            if (!res)
            {  // make an additional sleep period so the producer can catch up
                std::this_thread::sleep_for (std::chrono::milliseconds (100));
                res = sq.try_pop ();
            }
        }
        return cnt;
    };

    auto ret1 = std::async (std::launch::async, prod1);
    auto ret2 = std::async (std::launch::async, prod1);
    auto ret3 = std::async (std::launch::async, prod1);

    auto res1 = std::async (std::launch::async, cons);
    auto res2 = std::async (std::launch::async, cons);
    auto res3 = std::async (std::launch::async, cons);
    ret1.wait ();
    ret2.wait ();
    ret3.wait ();
    auto V1 = res1.get ();
    auto V2 = res2.get ();
    auto V3 = res3.get ();

    BOOST_CHECK_EQUAL (V1 + V2 + V3, 3'010'000);
}

/** test with multiple producer/multiple consumer*/
BOOST_AUTO_TEST_CASE (multithreaded_tests3_pop)
{
    BlockingPriorityQueue<int64_t> sq;
    sq.reserve (3'010'000);

    auto prod1 = [&]() {
        for (int64_t ii = 0; ii < 1'000'000; ++ii)
        {
            sq.push (ii);
        }
        sq.push (-1);
    };

    auto cons = [&]() {
        auto res = sq.pop ();
        int64_t cnt = 0;
        while (res >= 0)
        {
            ++cnt;
            res = sq.pop ();
        }
        return cnt;
    };

    auto ret1 = std::async (std::launch::async, prod1);
    auto ret2 = std::async (std::launch::async, prod1);
    auto ret3 = std::async (std::launch::async, prod1);

    auto res1 = std::async (std::launch::async, cons);
    auto res2 = std::async (std::launch::async, cons);
    auto res3 = std::async (std::launch::async, cons);
    ret1.wait ();
    ret2.wait ();
    ret3.wait ();
    auto V1 = res1.get ();
    auto V2 = res2.get ();
    auto V3 = res3.get ();

    BOOST_CHECK_EQUAL (V1 + V2 + V3, 3'000'000);
}

/** test with multiple producer/multiple consumer*/
BOOST_AUTO_TEST_CASE (pop_callback_tests, *utf::label("ci"))
{
    BlockingPriorityQueue<int64_t> sq;
    int pushcnt = 0;
    auto prod1 = [&]() {
        sq.push (7);
        ++pushcnt;
    };

    auto cons = [&](int cnt) {
        for (int ii = 0; ii < cnt; ii++)
        {
            sq.pop (prod1);
        }
        return cnt;
    };

    auto res = cons (25);
    BOOST_CHECK_EQUAL (res, 25);
    BOOST_CHECK_EQUAL (pushcnt, 25);
    auto res2 = cons (127);
    BOOST_CHECK_EQUAL (res2, 127);
    BOOST_CHECK_EQUAL (pushcnt, 127 + 25);
}

BOOST_AUTO_TEST_SUITE_END ()
