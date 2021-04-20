/*
Copyright (c) 2017-2021,
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
    EXPECT_EQ(helicsGetPropertyIndex("INT_CONSOLE_LOG_LEVEL"),
              helics_property_int_console_log_level);
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

TEST(prop_tests, flagprops)
{
    EXPECT_EQ(helicsGetFlagIndex("OBSERVER"), helics_flag_observer);
    EXPECT_EQ(helicsGetFlagIndex("UNINTERRUPTIBLE"), helics_flag_uninterruptible);
    EXPECT_EQ(helicsGetFlagIndex("INTERRUPTIBLE"), helics_flag_interruptible);
    EXPECT_EQ(helicsGetFlagIndex("DELAY_INIT_ENTRY"), helics_flag_delay_init_entry);
    EXPECT_EQ(helicsGetFlagIndex("ENABLE_INIT_ENTRY"), helics_flag_enable_init_entry);
    EXPECT_EQ(helicsGetFlagIndex("IGNORE_TIME_MISMATCH_WARNINGS"),
              helics_flag_ignore_time_mismatch_warnings);
    EXPECT_EQ(helicsGetFlagIndex("SOURCE_ONLY"), helics_flag_source_only);
    EXPECT_EQ(helicsGetFlagIndex("ONLY_TRANSMIT_ON_CHANGE"), helics_flag_only_transmit_on_change);
    EXPECT_EQ(helicsGetFlagIndex("ONLY_UPDATE_ON_CHANGE"), helics_flag_only_update_on_change);
    EXPECT_EQ(helicsGetFlagIndex("WAIT_FOR_CURRENT_TIME_UPDATE"),
              helics_flag_wait_for_current_time_update);
    EXPECT_EQ(helicsGetFlagIndex("RESTRICTIVE_TIME_POLICY"), helics_flag_restrictive_time_policy);
    EXPECT_EQ(helicsGetFlagIndex("ROLLBACK"), helics_flag_rollback);
    EXPECT_EQ(helicsGetFlagIndex("FORWARD_COMPUTE"), helics_flag_forward_compute);
    EXPECT_EQ(helicsGetFlagIndex("REALTIME"), helics_flag_realtime);
    EXPECT_EQ(helicsGetFlagIndex("SINGLE_THREAD_FEDERATE"), helics_flag_single_thread_federate);
    EXPECT_EQ(helicsGetFlagIndex("SLOW_RESPONDING"), helics_flag_slow_responding);
    EXPECT_EQ(helicsGetFlagIndex("TERMINATE_ON_ERROR"), helics_flag_terminate_on_error);
    EXPECT_EQ(helicsGetFlagIndex("STRICT_CONFIG_CHECKING"), helics_flag_strict_config_checking);
    EXPECT_EQ(helicsGetFlagIndex("FORCE_LOGGING_FLUSH"), helics_flag_force_logging_flush);
    EXPECT_EQ(helicsGetFlagIndex("DUMPLOG"), helics_flag_dumplog);
}
