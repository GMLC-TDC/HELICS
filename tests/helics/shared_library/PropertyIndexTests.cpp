/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include <gtest/gtest.h>
/** these test cases test out the value converters
 */
#include "ctestFixtures.hpp"

TEST(prop_tests, intprops)
{
    EXPECT_EQ(helicsGetPropertyIndex("max_iterations"), helics_property_int_max_iterations);
    EXPECT_EQ(helicsGetPropertyIndex("MAX_ITERATIONS"), helics_property_int_max_iterations);
    EXPECT_EQ(helicsGetPropertyIndex("INT_MAX_ITERATIONS"), helics_property_int_max_iterations);

    EXPECT_EQ(helicsGetPropertyIndex("log_level"), helics_property_int_log_level);
    EXPECT_EQ(helicsGetPropertyIndex("LOG_LEVEL"), helics_property_int_log_level);
    EXPECT_EQ(helicsGetPropertyIndex("INT_LOG_LEVEL"), helics_property_int_log_level);

    EXPECT_EQ(helicsGetPropertyIndex("file_log_level"), helics_property_int_file_log_level);
    EXPECT_EQ(helicsGetPropertyIndex("FILE_LOG_LEVEL"), helics_property_int_file_log_level);
    EXPECT_EQ(helicsGetPropertyIndex("INT_FILE_LOG_LEVEL"), helics_property_int_file_log_level);

    EXPECT_EQ(helicsGetPropertyIndex("console_log_level"), helics_property_int_console_log_level);
    EXPECT_EQ(helicsGetPropertyIndex("CONSOLE_LOG_LEVEL"), helics_property_int_console_log_level);
    EXPECT_EQ(helicsGetPropertyIndex("INT_CONSOLE_LOG_LEVEL"), helics_property_int_console_log_level);
}


TEST(prop_tests, timeprops)
{
    EXPECT_EQ(helicsGetPropertyIndex("DELTA"), helics_property_time_delta);
    EXPECT_EQ(helicsGetPropertyIndex("TIME_DELTA"), helics_property_time_delta);
    EXPECT_EQ(helicsGetPropertyIndex("timedelta"), helics_property_time_delta);

    EXPECT_EQ(helicsGetPropertyIndex("PERIOD"), helics_property_time_period);
    EXPECT_EQ(helicsGetPropertyIndex("TIME_PERIOD"), helics_property_time_period);
    EXPECT_EQ(helicsGetPropertyIndex("timeperiod"), helics_property_time_period);

    EXPECT_EQ(helicsGetPropertyIndex("OFFSET"), helics_property_time_offset);
    EXPECT_EQ(helicsGetPropertyIndex("TIME_OFFSET"), helics_property_time_offset);
    EXPECT_EQ(helicsGetPropertyIndex("timeoffset"), helics_property_time_offset);

   EXPECT_EQ(helicsGetPropertyIndex("RT_LAG"), helics_property_time_rt_lag);
    EXPECT_EQ(helicsGetPropertyIndex("TIME_RT_LAG"), helics_property_time_rt_lag);
   EXPECT_EQ(helicsGetPropertyIndex("rtlag"), helics_property_time_rt_lag);

    EXPECT_EQ(helicsGetPropertyIndex("RT_LEAD"), helics_property_time_rt_lead);
   EXPECT_EQ(helicsGetPropertyIndex("TIME_RT_LEAD"), helics_property_time_rt_lead);
   EXPECT_EQ(helicsGetPropertyIndex("rtlead"), helics_property_time_rt_lead);

     EXPECT_EQ(helicsGetPropertyIndex("RT_TOLERANCE"), helics_property_time_rt_tolerance);
   EXPECT_EQ(helicsGetPropertyIndex("TIME_RT_TOLERANCE"), helics_property_time_rt_tolerance);
   EXPECT_EQ(helicsGetPropertyIndex("rttolerance"), helics_property_time_rt_tolerance);

    EXPECT_EQ(helicsGetPropertyIndex("input_delay"), helics_property_time_input_delay);
   EXPECT_EQ(helicsGetPropertyIndex("TIME_INPUT_DELAY"), helics_property_time_input_delay);
   EXPECT_EQ(helicsGetPropertyIndex("INPUT_DELAY"), helics_property_time_input_delay);

   EXPECT_EQ(helicsGetPropertyIndex("output_delay"), helics_property_time_output_delay);
   EXPECT_EQ(helicsGetPropertyIndex("TIME_OUTPUT_DELAY"), helics_property_time_output_delay);
   EXPECT_EQ(helicsGetPropertyIndex("OUTPUT_DELAY"), helics_property_time_output_delay);
}
