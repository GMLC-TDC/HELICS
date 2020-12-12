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
    EXPECT_EQ(helicsGetPropertyIndex("max_iterations"), HELICS_PROPERTY_INT_max_iterations);
    EXPECT_EQ(helicsGetPropertyIndex("MAX_ITERATIONS"), HELICS_PROPERTY_INT_max_iterations);
    EXPECT_EQ(helicsGetPropertyIndex("INT_MAX_ITERATIONS"), HELICS_PROPERTY_INT_max_iterations);

    EXPECT_EQ(helicsGetPropertyIndex("log_level"), HELICS_PROPERTY_INT_log_level);
    EXPECT_EQ(helicsGetPropertyIndex("LOG_LEVEL"), HELICS_PROPERTY_INT_log_level);
    EXPECT_EQ(helicsGetPropertyIndex("INT_LOG_LEVEL"), HELICS_PROPERTY_INT_log_level);

    EXPECT_EQ(helicsGetPropertyIndex("file_log_level"), HELICS_PROPERTY_INT_file_log_level);
    EXPECT_EQ(helicsGetPropertyIndex("FILE_LOG_LEVEL"), HELICS_PROPERTY_INT_file_log_level);
    EXPECT_EQ(helicsGetPropertyIndex("INT_FILE_LOG_LEVEL"), HELICS_PROPERTY_INT_file_log_level);

    EXPECT_EQ(helicsGetPropertyIndex("console_log_level"), HELICS_PROPERTY_INT_console_log_level);
    EXPECT_EQ(helicsGetPropertyIndex("CONSOLE_LOG_LEVEL"), HELICS_PROPERTY_INT_console_log_level);
    EXPECT_EQ(helicsGetPropertyIndex("INT_CONSOLE_LOG_LEVEL"),
              HELICS_PROPERTY_INT_console_log_level);
}

TEST(prop_tests, timeprops)
{
    EXPECT_EQ(helicsGetPropertyIndex("DELTA"), HELICS_PROPERTY_TIME_delta);
    EXPECT_EQ(helicsGetPropertyIndex("TIME_DELTA"), HELICS_PROPERTY_TIME_delta);
    EXPECT_EQ(helicsGetPropertyIndex("timedelta"), HELICS_PROPERTY_TIME_delta);

    EXPECT_EQ(helicsGetPropertyIndex("PERIOD"), HELICS_PROPERTY_TIME_PERIOD);
    EXPECT_EQ(helicsGetPropertyIndex("TIME_PERIOD"), HELICS_PROPERTY_TIME_PERIOD);
    EXPECT_EQ(helicsGetPropertyIndex("timeperiod"), HELICS_PROPERTY_TIME_PERIOD);

    EXPECT_EQ(helicsGetPropertyIndex("OFFSET"), HELICS_PROPERTY_TIME_offset);
    EXPECT_EQ(helicsGetPropertyIndex("TIME_OFFSET"), HELICS_PROPERTY_TIME_offset);
    EXPECT_EQ(helicsGetPropertyIndex("timeoffset"), HELICS_PROPERTY_TIME_offset);

    EXPECT_EQ(helicsGetPropertyIndex("RT_LAG"), HELICS_PROPERTY_TIME_rt_lag);
    EXPECT_EQ(helicsGetPropertyIndex("TIME_RT_LAG"), HELICS_PROPERTY_TIME_rt_lag);
    EXPECT_EQ(helicsGetPropertyIndex("rtlag"), HELICS_PROPERTY_TIME_rt_lag);

    EXPECT_EQ(helicsGetPropertyIndex("RT_LEAD"), HELICS_PROPERTY_TIME_rt_lead);
    EXPECT_EQ(helicsGetPropertyIndex("TIME_RT_LEAD"), HELICS_PROPERTY_TIME_rt_lead);
    EXPECT_EQ(helicsGetPropertyIndex("rtlead"), HELICS_PROPERTY_TIME_rt_lead);

    EXPECT_EQ(helicsGetPropertyIndex("RT_TOLERANCE"), HELICS_PROPERTY_TIME_rt_tolerance);
    EXPECT_EQ(helicsGetPropertyIndex("TIME_RT_TOLERANCE"), HELICS_PROPERTY_TIME_rt_tolerance);
    EXPECT_EQ(helicsGetPropertyIndex("rttolerance"), HELICS_PROPERTY_TIME_rt_tolerance);

    EXPECT_EQ(helicsGetPropertyIndex("input_delay"), HELICS_PROPERTY_TIME_input_delay);
    EXPECT_EQ(helicsGetPropertyIndex("TIME_INPUT_DELAY"), HELICS_PROPERTY_TIME_input_delay);
    EXPECT_EQ(helicsGetPropertyIndex("INPUT_DELAY"), HELICS_PROPERTY_TIME_input_delay);

    EXPECT_EQ(helicsGetPropertyIndex("output_delay"), HELICS_PROPERTY_TIME_output_delay);
    EXPECT_EQ(helicsGetPropertyIndex("TIME_OUTPUT_DELAY"), HELICS_PROPERTY_TIME_output_delay);
    EXPECT_EQ(helicsGetPropertyIndex("OUTPUT_DELAY"), HELICS_PROPERTY_TIME_output_delay);
}

TEST(prop_tests, flagprops)
{
    EXPECT_EQ(helicsGetFlagIndex("OBSERVER"), HELICS_FLAG_observer);
    EXPECT_EQ(helicsGetFlagIndex("UNINTERRUPTIBLE"), HELICS_FLAG_uninterruptible);
    EXPECT_EQ(helicsGetFlagIndex("INTERRUPTIBLE"), HELICS_FLAG_interruptible);
    EXPECT_EQ(helicsGetFlagIndex("DELAY_INIT_ENTRY"), HELICS_FLAG_delay_init_entry);
    EXPECT_EQ(helicsGetFlagIndex("ENABLE_INIT_ENTRY"), HELICS_FLAG_enable_init_entry);
    EXPECT_EQ(helicsGetFlagIndex("IGNORE_TIME_MISMATCH_WARNINGS"),
              HELICS_FLAG_ignore_time_mismatch_warnings);
    EXPECT_EQ(helicsGetFlagIndex("SOURCE_ONLY"), HELICS_FLAG_source_only);
    EXPECT_EQ(helicsGetFlagIndex("ONLY_TRANSMIT_ON_CHANGE"), HELICS_FLAG_only_transmit_on_change);
    EXPECT_EQ(helicsGetFlagIndex("ONLY_UPDATE_ON_CHANGE"), HELICS_FLAG_only_update_on_change);
    EXPECT_EQ(helicsGetFlagIndex("WAIT_FOR_CURRENT_TIME_UPDATE"),
              HELICS_FLAG_wait_for_current_time_update);
    EXPECT_EQ(helicsGetFlagIndex("RESTRICTIVE_TIME_POLICY"), HELICS_FLAG_restrictive_time_policy);
    EXPECT_EQ(helicsGetFlagIndex("ROLLBACK"), HELICS_FLAG_rollback);
    EXPECT_EQ(helicsGetFlagIndex("FORWARD_COMPUTE"), HELICS_FLAG_forward_compute);
    EXPECT_EQ(helicsGetFlagIndex("REALTIME"), HELICS_FLAG_realtime);
    EXPECT_EQ(helicsGetFlagIndex("SINGLE_THREAD_FEDERATE"), HELICS_FLAG_single_thread_federate);
    EXPECT_EQ(helicsGetFlagIndex("SLOW_RESPONDING"), HELICS_FLAG_slow_responding);
    EXPECT_EQ(helicsGetFlagIndex("TERMINATE_ON_ERROR"), HELICS_FLAG_terminate_on_error);
    EXPECT_EQ(helicsGetFlagIndex("STRICT_CONFIG_CHECKING"), HELICS_FLAG_strict_config_checking);
    EXPECT_EQ(helicsGetFlagIndex("FORCE_LOGGING_FLUSH"), HELICS_FLAG_force_logging_flush);
    EXPECT_EQ(helicsGetFlagIndex("DUMPLOG"), HELICS_FLAG_dumplog);
}
