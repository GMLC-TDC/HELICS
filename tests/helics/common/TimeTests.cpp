/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include <chrono>
#include <gtest/gtest.h>

/** these test cases test data_block and data_view objects
 */

#include "gmlc/utilities/timeStringOps.hpp"
#include "helics/core/helics-time.hpp"
#include "helics/shared_api_library/api-data.h"
using namespace helics;

TEST(time_tests, simple_times)
{
    Time time1(10.0);
    EXPECT_EQ(static_cast<double>(time1), 10.0);

    Time time2(5, time_units::sec);
    Time time3(5000, time_units::ms);
    EXPECT_EQ(time2, time3);

    EXPECT_EQ(time2.toCount(time_units::s), 5);
    EXPECT_EQ(time2.toCount(time_units::ms), 5000);
    EXPECT_EQ(time2.toCount(time_units::us), 5'000'000);
    EXPECT_EQ(time2.toCount(time_units::ns), 5'000'000'000);
    EXPECT_EQ(time2.toCount(time_units::ps), 5'000'000'000'000);
    EXPECT_EQ(time2.toCount(time_units::minutes), 0);

    time3 = 5.01;
    EXPECT_NE(time2, time3);

    EXPECT_EQ(time3.toCount(time_units::s), 5);
    EXPECT_EQ(time3.toCount(time_units::ms), 5010);
    EXPECT_EQ(time3.toCount(time_units::us), 5'010'000);
    EXPECT_EQ(time3.toCount(time_units::ns), 5'010'000'000);
    EXPECT_EQ(time3.toCount(time_units::ps), 5'010'000'000'000);
    EXPECT_EQ(time3.toCount(time_units::minutes), 0);

    time3 = 60.1;
    EXPECT_EQ(time3.toCount(time_units::s), 60);
    EXPECT_EQ(time3.toCount(time_units::ms), 60100);
    EXPECT_EQ(time3.toCount(time_units::minutes), 1);
}

TEST(time_tests, test_baseConversion)
{
    Time time1(49.759632);

    auto cnt = time1.getBaseTimeCode();
    Time time2;
    time2.setBaseTimeCode(cnt);

    EXPECT_EQ(time1, time2);

    Time time3(-3562.28963);

    cnt = time3.getBaseTimeCode();
    Time time4;
    time4.setBaseTimeCode(cnt);

    EXPECT_EQ(time3, time4);
}

TEST(time_tests, test_math)
{
    Time time1(4.3);
    Time time2(2.7);

    EXPECT_EQ(time1 + time2, Time(7.0));
    EXPECT_EQ(time1 + 1.7, Time(6.0));

    EXPECT_EQ(time1 - time2, Time(1.6));

    EXPECT_EQ(-time1, Time(-4.3));

    EXPECT_EQ(Time(2.0) * 5, Time(10.0));
    EXPECT_EQ(Time(10.0) / 4, Time(2.5));
    EXPECT_EQ(Time(10.0) / 2.5, Time(4.0));
    EXPECT_EQ(4 * Time(2.0), Time(8.0));
    EXPECT_EQ(2.5 * Time(2.0), Time(5.0));

    time1 += time2;
    EXPECT_EQ(time1, Time(7.0));
    time1 -= time2;
    EXPECT_EQ(time1, Time(4.3));

    auto time3 = Time(1.0);
    time3 *= 4;
    EXPECT_EQ(time3, Time(4.0));
    time3 *= 2.5;
    EXPECT_EQ(time3, Time(10.0));
    time3 /= 2;
    EXPECT_EQ(time3, Time(5.0));
    time3 /= 2.5;
    EXPECT_EQ(time3, Time(2.0));
}

TEST(time_tests, rounding_tests)
{
    EXPECT_TRUE(Time(1.25e-9) == Time(1, time_units::ns));
    EXPECT_TRUE(Time(0.99e-9) == Time(1, time_units::ns));
    EXPECT_TRUE(Time(1.49e-9) == Time(1, time_units::ns));
    EXPECT_TRUE(Time(1.51e-9) == Time(2, time_units::ns));
}

TEST(time_tests, comparison_tests)
{
    EXPECT_TRUE(Time(1.1) > Time(1.0));
    EXPECT_TRUE(Time(-1.1) < Time(-1.0));
    EXPECT_TRUE(Time(1.0) < Time(1.1));
    EXPECT_TRUE(Time(-1.0) > Time(-1.1));
    EXPECT_TRUE(Time(10, time_units::ms) == Time(10000, time_units::us));

    EXPECT_TRUE(Time(1.1) >= Time(1.0));
    EXPECT_TRUE(Time(-1.1) <= Time(-1.0));
    EXPECT_TRUE(Time(1.0) <= Time(1.1));
    EXPECT_TRUE(Time(-1.0) >= Time(-1.1));

    EXPECT_TRUE(Time(1.0) >= Time(1.0));
    EXPECT_TRUE(Time(-1.0) <= Time(-1.0));

    EXPECT_TRUE(Time(1.0) != Time(1.00001));
    EXPECT_TRUE(!(Time(-1.0) != Time(-1.0)));

    // now with doubles as the second operand
    EXPECT_TRUE(Time(1.1) > 1.0);
    EXPECT_TRUE(Time(-1.1) < -1.0);
    EXPECT_TRUE(Time(1.0) < 1.1);
    EXPECT_TRUE(Time(-1.0) > -1.1);
    EXPECT_TRUE(Time(10, time_units::ms) == 0.01);

    EXPECT_TRUE(Time(1.1) >= 1.0);
    EXPECT_TRUE(Time(-1.1) <= -1.0);
    EXPECT_TRUE(Time(1.0) <= 1.1);
    EXPECT_TRUE(Time(-1.0) >= -1.1);

    EXPECT_TRUE(Time(1.0) >= 1.0);
    EXPECT_TRUE(Time(-1.0) <= -1.0);

    EXPECT_TRUE(Time(1.0) != 1.00001);
    EXPECT_TRUE(!(Time(-1.0) != -1.0));

    // now with doubles as the first operand
    EXPECT_TRUE(1.1 > Time(1.0));
    EXPECT_TRUE(-1.1 < Time(-1.0));
    EXPECT_TRUE(1.0 < Time(1.1));
    EXPECT_TRUE(-1.0 > Time(-1.1));
    EXPECT_TRUE(0.01 == Time(10000, time_units::us));

    EXPECT_TRUE(1.1 >= Time(1.0));
    EXPECT_TRUE(-1.1 <= Time(-1.0));
    EXPECT_TRUE(1.0 <= Time(1.1));
    EXPECT_TRUE(-1.0 >= Time(-1.1));

    EXPECT_TRUE(1.0 >= Time(1.0));
    EXPECT_TRUE(-1.0 <= Time(-1.0));

    EXPECT_TRUE(1.0 != Time(1.00001));
    EXPECT_TRUE(!(-1.0 != Time(-1.0)));
}

TEST(time_tests, test_string_conversions)
{
    using namespace gmlc::utilities;

    EXPECT_EQ(loadTimeFromString<Time>("10"), Time(10));
    EXPECT_EQ(loadTimeFromString<Time>("-10"), Time(-10));

    EXPECT_EQ(loadTimeFromString<Time>("45", time_units::ms), Time(45, time_units::ms));
    EXPECT_EQ(loadTimeFromString<Time>("45000 us", time_units::ms), Time(45, time_units::ms));

    EXPECT_EQ(loadTimeFromString<Time>("0.045   s"), Time(45, time_units::ms));

    EXPECT_EQ(loadTimeFromString<Time>("0.045 seconds"), Time(45, time_units::ms));

    EXPECT_EQ(loadTimeFromString<Time>("4.5 ms"), Time(0.0045));
    EXPECT_EQ(loadTimeFromString<Time>("4.5ms"), Time(0.0045));

    EXPECT_THROW(loadTimeFromString<Time>("happy"), std::invalid_argument);
}

TEST(time_tests, chrono_tests)
{
    using namespace std::chrono;
    milliseconds tm1(100);

    Time b(tm1);
    EXPECT_EQ(b, 0.1);

    nanoseconds tmns(10026523523);
    Time b2(tmns);
    EXPECT_EQ(b2.getBaseTimeCode(), tmns.count());

    EXPECT_TRUE(b2.to_ns() == tmns);
}

TEST(time_tests, max_tests)
{
    auto tm = Time(helics_time_maxtime);
    auto tmax = Time::maxVal();
    EXPECT_EQ(tm, tmax);
    EXPECT_EQ(Time(-helics_time_maxtime), Time::minVal());

    EXPECT_GE(static_cast<double>(Time::maxVal()), helics_time_maxtime);
}
