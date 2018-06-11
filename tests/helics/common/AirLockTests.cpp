/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <future>
#include <memory>
#include <string>
#include <thread>
#include <utility>
/** these test cases test data_block and data_view objects
 */

#include "helics/common/AirLock.hpp"

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE (airlock_tests, *utf::label("ci") *utf::label("daily") *utf::label("release"))

/** test basic operations */
BOOST_AUTO_TEST_CASE (basic_tests)
{
    AirLock<int> alock;

    BOOST_CHECK (alock.try_load (45));
    BOOST_CHECK (!alock.try_load (54));

    BOOST_CHECK (alock.isLoaded ());
    auto res = alock.try_unload ();
    BOOST_REQUIRE (res);

    BOOST_CHECK_EQUAL (*res, 45);
    BOOST_CHECK (!alock.isLoaded ());
    BOOST_CHECK (alock.try_load (54));
}

/** test with a move only element*/
BOOST_AUTO_TEST_CASE (move_only_tests)
{
    AirLock<std::unique_ptr<double>> alock;

    alock.try_load (std::make_unique<double> (4534.23));

    BOOST_CHECK (alock.isLoaded ());

    auto b = alock.try_unload ();
    BOOST_CHECK_EQUAL (**b, 4534.23);

    b = alock.try_unload ();
    BOOST_CHECK (!(b));
    BOOST_CHECK (!alock.isLoaded ());
}

/** multi-thread test*/
BOOST_AUTO_TEST_CASE (move_mthread_tests)
{
    AirLock<std::string> alock;

    alock.try_load ("load 1");

    BOOST_CHECK (alock.isLoaded ());

    auto fut = std::async (std::launch::async, [&alock]() { alock.load ("load 2"); });
    auto fut2 = std::async (std::launch::async, [&alock]() { alock.load ("load 2"); });
    std::this_thread::yield ();
    auto b = alock.try_unload ();
    BOOST_CHECK_EQUAL (*b, "load 1");
    int chk = 0;
    while (!alock.isLoaded ())
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (100));
        if (chk++ > 10)
        {
            break;
        }
    }
    b = alock.try_unload ();
    BOOST_REQUIRE (b);
    BOOST_CHECK_EQUAL (*b, "load 2");
    fut.get ();
    fut2.get ();
    BOOST_CHECK (alock.isLoaded ());
}

BOOST_AUTO_TEST_SUITE_END ()
