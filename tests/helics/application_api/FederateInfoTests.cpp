/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/FederateInfo.hpp"
#include "helics/core/helics_definitions.hpp"

#include "gtest/gtest.h"
#include <chrono>

/** @file these test cases test out the federateInfo object
 */

TEST(federateInfo, constructor1)
{
    helics::FederateInfo f1("--type=test --name fi");
    EXPECT_EQ(f1.coreType, helics::core_type::TEST);
    EXPECT_EQ(f1.defName, "fi");
}

TEST(federateInfo, constructor2)
{
    std::vector<std::string> args{
        "constructor2", "--name", "f2", "--type", "zmq", "--flags", "realtime,source_only"};
    char* argv[7];
    for (size_t ii = 0; ii < args.size(); ++ii) {
        argv[ii] = &(args[ii][0]);
    }

    helics::FederateInfo f1(7, argv);
    EXPECT_EQ(f1.coreType, helics::core_type::ZMQ);
    EXPECT_EQ(f1.defName, "f2");
    EXPECT_EQ(f1.flagProps.size(), 2);
}

TEST(federateInfo, loadArgs1)
{
    std::vector<std::string> args{
        "constructor2", "--name", "f2", "--type", "zmq", "--flags", "realtime,source_only"};
    char* argv[7];
    for (size_t ii = 0; ii < args.size(); ++ii) {
        argv[ii] = &(args[ii][0]);
    }

    helics::FederateInfo f1;
    f1.loadInfoFromArgs(7, argv);
    EXPECT_EQ(f1.coreType, helics::core_type::ZMQ);
    EXPECT_EQ(f1.defName, "f2");
    EXPECT_EQ(f1.flagProps.size(), 2);
}

TEST(federateInfo, constructor3)
{
    helics::FederateInfo f1{
        "--name f3 --type ipc --flags realtime;source_only,-buffer_data --port=5000"};
    EXPECT_EQ(f1.coreType, helics::core_type::INTERPROCESS);
    EXPECT_EQ(f1.defName, "f3");
    EXPECT_EQ(f1.flagProps.size(), 3);
    EXPECT_EQ(f1.brokerPort, 5000);
}

TEST(federateInfo, loadArgs2)
{
    helics::FederateInfo f1;
    f1.loadInfoFromArgs(
        "--name f3 --type ipc --flags realtime;source_only,-buffer_data --port=5000");
    EXPECT_EQ(f1.coreType, helics::core_type::INTERPROCESS);
    EXPECT_EQ(f1.defName, "f3");
    EXPECT_EQ(f1.flagProps.size(), 3);
    EXPECT_EQ(f1.brokerPort, 5000);
}
TEST(federateInfo, constructor4)
{
    helics::FederateInfo f1{"--log_level=no_print --brokerport=5000 --port=5005"};
    EXPECT_EQ(f1.brokerPort, 5000);
    EXPECT_EQ(f1.localport, "5005");
    EXPECT_EQ(f1.intProps.size(), 1);
}

TEST(federateInfo, constructor5)
{
    helics::FederateInfo f1{"--input_delay 50ms --brokerinit='--loglevel 3 --type=zmq'"};
    ASSERT_FALSE(f1.brokerInitString.empty());
    EXPECT_EQ(f1.brokerInitString.front(), ' ');
    EXPECT_EQ(f1.timeProps.size(), 1);
}

TEST(federateInfo, constructor6)
{
    helics::FederateInfo f1{"--outputdelay=2 --separator=/"};
    EXPECT_EQ(f1.timeProps.size(), 1);
    EXPECT_EQ(f1.separator, '/');
}

TEST(federateInfo, constructor_fail)
{
    EXPECT_THROW(helics::FederateInfo f1{"--inputdelay=2 --separator=45"}, std::exception);
}

TEST(federateInfo, property_index)
{
    EXPECT_EQ(helics::getPropertyIndex("unexpected"), -1);
    EXPECT_EQ(helics::getPropertyIndex("rtlag"), helics_property_time_rt_lag);
    EXPECT_EQ(helics::getPropertyIndex("RTlead"), helics_property_time_rt_lead);
    EXPECT_EQ(helics::getPropertyIndex("RT__Tolerance"), helics_property_time_rt_tolerance);
}

TEST(federateInfo, option_index)
{
    EXPECT_EQ(helics::getOptionIndex("unexpected"), -1);
    EXPECT_EQ(
        helics::getOptionIndex("single_connection"), helics_handle_option_single_connection_only);
    EXPECT_EQ(
        helics::getOptionIndex("StrictTypeChecking"), helics_handle_option_strict_type_checking);
    EXPECT_EQ(helics::getOptionIndex("un_interruptible"), helics_handle_option_ignore_interrupts);
}
