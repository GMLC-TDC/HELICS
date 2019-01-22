/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include <chrono>
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

/** these test cases test data_block and data_view objects
 */

#include "helics/core/helics-time.hpp"

BOOST_AUTO_TEST_SUITE (time_tests)

using namespace helics;

BOOST_AUTO_TEST_CASE (simple_times)
{
    Time time1 (10.0);
    BOOST_CHECK_EQUAL (static_cast<double> (time1), 10.0);

    Time time2 (5, time_units::sec);
    Time time3 (5000, time_units::ms);
    BOOST_CHECK_EQUAL (time2, time3);

    BOOST_CHECK_EQUAL (time2.toCount (time_units::s), 5);
    BOOST_CHECK_EQUAL (time2.toCount (time_units::ms), 5000);
    BOOST_CHECK_EQUAL (time2.toCount (time_units::us), 5'000'000);
    BOOST_CHECK_EQUAL (time2.toCount (time_units::ns), 5'000'000'000);
    BOOST_CHECK_EQUAL (time2.toCount (time_units::ps), 5'000'000'000'000);
    BOOST_CHECK_EQUAL (time2.toCount (time_units::minutes), 0);

    time3 = 5.01;
    BOOST_CHECK_NE (time2, time3);

    BOOST_CHECK_EQUAL (time3.toCount (time_units::s), 5);
    BOOST_CHECK_EQUAL (time3.toCount (time_units::ms), 5010);
    BOOST_CHECK_EQUAL (time3.toCount (time_units::us), 5'010'000);
    BOOST_CHECK_EQUAL (time3.toCount (time_units::ns), 5'010'000'000);
    BOOST_CHECK_EQUAL (time3.toCount (time_units::ps), 5'010'000'000'000);
    BOOST_CHECK_EQUAL (time3.toCount (time_units::minutes), 0);

    time3 = 60.1;
    BOOST_CHECK_EQUAL (time3.toCount (time_units::s), 60);
    BOOST_CHECK_EQUAL (time3.toCount (time_units::ms), 60100);
    BOOST_CHECK_EQUAL (time3.toCount (time_units::minutes), 1);
}

BOOST_AUTO_TEST_CASE (test_baseConversion)
{
    Time time1 (49.759632);

    auto cnt = time1.getBaseTimeCode ();
    Time time2;
    time2.setBaseTimeCode (cnt);

    BOOST_CHECK_EQUAL (time1, time2);

    Time time3 (-3562.28963);

    cnt = time3.getBaseTimeCode ();
    Time time4;
    time4.setBaseTimeCode (cnt);

    BOOST_CHECK_EQUAL (time3, time4);
}

BOOST_AUTO_TEST_CASE (test_math)
{
    Time time1 (4.3);
    Time time2 (2.7);

    BOOST_CHECK_EQUAL (time1 + time2, Time (7.0));
    BOOST_CHECK_EQUAL (time1 + 1.7, Time (6.0));

    BOOST_CHECK_EQUAL (time1 - time2, Time (1.6));

    BOOST_CHECK_EQUAL (-time1, Time (-4.3));

    BOOST_CHECK_EQUAL (Time (2.0) * 5, Time (10.0));
    BOOST_CHECK_EQUAL (Time (10.0) / 4, Time (2.5));
    BOOST_CHECK_EQUAL (Time (10.0) / 2.5, Time (4.0));
    BOOST_CHECK_EQUAL (4 * Time (2.0), Time (8.0));
    BOOST_CHECK_EQUAL (2.5 * Time (2.0), Time (5.0));

    time1 += time2;
    BOOST_CHECK_EQUAL (time1, Time (7.0));
    time1 -= time2;
    BOOST_CHECK_EQUAL (time1, Time (4.3));

    auto time3 = Time (1.0);
    time3 *= 4;
    BOOST_CHECK_EQUAL (time3, Time (4.0));
    time3 *= 2.5;
    BOOST_CHECK_EQUAL (time3, Time (10.0));
    time3 /= 2;
    BOOST_CHECK_EQUAL (time3, Time (5.0));
    time3 /= 2.5;
    BOOST_CHECK_EQUAL (time3, Time (2.0));
}

BOOST_AUTO_TEST_CASE (rounding_tests)
{
    BOOST_CHECK (Time (1.25e-9) == Time (1, time_units::ns));
    BOOST_CHECK (Time (0.99e-9) == Time (1, time_units::ns));
    BOOST_CHECK (Time (1.49e-9) == Time (1, time_units::ns));
    BOOST_CHECK (Time (1.51e-9) == Time (2, time_units::ns));
}

BOOST_AUTO_TEST_CASE (comparison_tests)
{
    BOOST_CHECK (Time (1.1) > Time (1.0));
    BOOST_CHECK (Time (-1.1) < Time (-1.0));
    BOOST_CHECK (Time (1.0) < Time (1.1));
    BOOST_CHECK (Time (-1.0) > Time (-1.1));
    BOOST_CHECK (Time (10, time_units::ms) == Time (10000, time_units::us));

    BOOST_CHECK (Time (1.1) >= Time (1.0));
    BOOST_CHECK (Time (-1.1) <= Time (-1.0));
    BOOST_CHECK (Time (1.0) <= Time (1.1));
    BOOST_CHECK (Time (-1.0) >= Time (-1.1));

    BOOST_CHECK (Time (1.0) >= Time (1.0));
    BOOST_CHECK (Time (-1.0) <= Time (-1.0));

    BOOST_CHECK (Time (1.0) != Time (1.00001));
    BOOST_CHECK (!(Time (-1.0) != Time (-1.0)));

    // now with doubles as the second operand
    BOOST_CHECK (Time (1.1) > 1.0);
    BOOST_CHECK (Time (-1.1) < -1.0);
    BOOST_CHECK (Time (1.0) < 1.1);
    BOOST_CHECK (Time (-1.0) > -1.1);
    BOOST_CHECK (Time (10, time_units::ms) == 0.01);

    BOOST_CHECK (Time (1.1) >= 1.0);
    BOOST_CHECK (Time (-1.1) <= -1.0);
    BOOST_CHECK (Time (1.0) <= 1.1);
    BOOST_CHECK (Time (-1.0) >= -1.1);

    BOOST_CHECK (Time (1.0) >= 1.0);
    BOOST_CHECK (Time (-1.0) <= -1.0);

    BOOST_CHECK (Time (1.0) != 1.00001);
    BOOST_CHECK (!(Time (-1.0) != -1.0));

    // now with doubles as the first operand
    BOOST_CHECK (1.1 > Time (1.0));
    BOOST_CHECK (-1.1 < Time (-1.0));
    BOOST_CHECK (1.0 < Time (1.1));
    BOOST_CHECK (-1.0 > Time (-1.1));
    BOOST_CHECK (0.01 == Time (10000, time_units::us));

    BOOST_CHECK (1.1 >= Time (1.0));
    BOOST_CHECK (-1.1 <= Time (-1.0));
    BOOST_CHECK (1.0 <= Time (1.1));
    BOOST_CHECK (-1.0 >= Time (-1.1));

    BOOST_CHECK (1.0 >= Time (1.0));
    BOOST_CHECK (-1.0 <= Time (-1.0));

    BOOST_CHECK (1.0 != Time (1.00001));
    BOOST_CHECK (!(-1.0 != Time (-1.0)));
}

BOOST_AUTO_TEST_CASE (test_string_conversions)
{
    BOOST_CHECK_EQUAL (loadTimeFromString ("10"), Time (10));
    BOOST_CHECK_EQUAL (loadTimeFromString ("-10"), Time (-10));

    BOOST_CHECK_EQUAL (loadTimeFromString ("45", time_units::ms), Time (45, time_units::ms));
    BOOST_CHECK_EQUAL (loadTimeFromString ("45000 us", time_units::ms), Time (45, time_units::ms));

    BOOST_CHECK_EQUAL (loadTimeFromString ("0.045   s"), Time (45, time_units::ms));

    BOOST_CHECK_EQUAL (loadTimeFromString ("0.045 seconds"), Time (45, time_units::ms));

    BOOST_CHECK_EQUAL (loadTimeFromString ("4.5 ms"), Time (0.0045));
    BOOST_CHECK_EQUAL (loadTimeFromString ("4.5ms"), Time (0.0045));

    BOOST_CHECK_THROW (loadTimeFromString ("happy"), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE (chrono_tests)
{
    using namespace std::chrono;
    milliseconds tm1 (100);

    Time b (tm1);
    BOOST_CHECK_EQUAL (b, 0.1);

    nanoseconds tmns (10026523523);
    Time b2 (tmns);
    BOOST_CHECK_EQUAL (b2.getBaseTimeCode (), tmns.count ());

    BOOST_CHECK (b2.to_ns () == tmns);
}

BOOST_AUTO_TEST_SUITE_END ()
