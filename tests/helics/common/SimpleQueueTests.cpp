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

#include "helics/common/simpleQueue.hpp"

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE (simple_queue_tests, *utf::label("common"))

/** test basic operations */
BOOST_AUTO_TEST_CASE (basic_tests)
{
    SimpleQueue<int> sq;

    sq.push (45);
    sq.push (54);

    BOOST_CHECK (sq.empty () == false);

    BOOST_CHECK_EQUAL (sq.size (), 2);
    auto b = sq.pop ();
    BOOST_CHECK_EQUAL (*b, 45);
    b = sq.pop ();
    BOOST_CHECK_EQUAL (*b, 54);

    b = sq.pop ();
    BOOST_CHECK (!(b));
    BOOST_CHECK (sq.empty ());
}

/** test with a move only element*/
BOOST_AUTO_TEST_CASE (move_only_tests)
{
    SimpleQueue<std::unique_ptr<double>> sq;

    sq.push (std::make_unique<double> (4534.23));

    auto e2 = std::make_unique<double> (34.234);
    sq.push (std::move (e2));

    BOOST_CHECK (sq.empty () == false);

    BOOST_CHECK_EQUAL (sq.size (), 2);
    auto b = sq.pop ();
    BOOST_CHECK_EQUAL (**b, 4534.23);
    b = sq.pop ();
    BOOST_CHECK_EQUAL (**b, 34.234);

    b = sq.pop ();
    BOOST_CHECK (!(b));
    BOOST_CHECK (sq.empty ());
}

/** test the ordering with a larger number of inputs*/

BOOST_AUTO_TEST_CASE (ordering_tests)
{
    SimpleQueue<int> sq;

    for (int ii = 1; ii < 10; ++ii)
    {
        sq.push (ii);
    }

    auto b = sq.pop ();
    BOOST_CHECK_EQUAL (*b, 1);
    for (int ii = 2; ii < 7; ++ii)
    {
        b = sq.pop ();
        BOOST_CHECK_EQUAL (*b, ii);
    }
    for (int ii = 10; ii < 20; ++ii)
    {
        sq.push (ii);
    }
    for (int ii = 7; ii < 20; ++ii)
    {
        b = sq.pop ();
        BOOST_CHECK_EQUAL (*b, ii);
    }

    BOOST_CHECK (sq.empty ());
}

BOOST_AUTO_TEST_CASE (emplace_tests)
{
    SimpleQueue<std::pair<int, double>> sq;

    sq.emplace (10, 45.4);
    sq.emplace (11, 34.1);
    sq.emplace (12, 34.2);

    BOOST_CHECK_EQUAL (sq.size (), 3);
    auto b = sq.pop ();
    BOOST_CHECK_EQUAL (b->first, 10);
    BOOST_CHECK_EQUAL (b->second, 45.4);
    b = sq.pop ();
    BOOST_CHECK_EQUAL (b->first, 11);
    BOOST_CHECK_EQUAL (b->second, 34.1);
}

/** test with single consumer/single producer*/
BOOST_AUTO_TEST_CASE (multithreaded_tests)
{
    SimpleQueue<int64_t> sq (1010000);

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
        auto res = sq.pop ();
        int64_t cnt = 1;
        while ((res))
        {
            auto nres = sq.pop ();
            if (!nres)
            {  // make an additional sleep period so the producer can catch up
                std::this_thread::sleep_for (std::chrono::milliseconds (100));
                nres = sq.pop ();
            }
            if (nres)
            {
                if (*nres > *res)
                {
                    ++cnt;
                }
                else
                {
                    if (*nres > 0)
                    {
                        printf ("%d came before %d\n", static_cast<int> (*nres), static_cast<int> (*res));
                    }
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
    BOOST_CHECK_EQUAL (V, 1'010'000);
}

#ifndef QUICK_TESTS_ONLY
/** test with multiple consumer/single producer*/
BOOST_AUTO_TEST_CASE (multithreaded_tests2)
{
    SimpleQueue<int64_t> sq (1010000);

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
        auto res = sq.pop ();
        int64_t cnt = 0;
        while ((res))
        {
            ++cnt;
            res = sq.pop ();
            if (!res)
            {  // make an additional sleep period so the producer can catch up
                std::this_thread::sleep_for (std::chrono::milliseconds (100));
                res = sq.pop ();
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
    SimpleQueue<int64_t> sq;
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
        auto res = sq.pop ();
        int64_t cnt = 0;
        while ((res))
        {
            ++cnt;
            res = sq.pop ();
            if (!res)
            {  // make an additional sleep period so the producer can catch up
                std::this_thread::sleep_for (std::chrono::milliseconds (100));
                res = sq.pop ();
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

#endif
BOOST_AUTO_TEST_SUITE_END ()
