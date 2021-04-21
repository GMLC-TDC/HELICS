/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/FederateInfo.hpp"
#include "helics/core/core-exceptions.hpp"
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
    std::vector<std::string> args{"constructor2",
                                  "--name",
                                  "f2",
                                  "--type",
                                  "inproc",
                                  "--flags",
                                  "realtime,,source_only;autobroker"};
    char* argv[7];
    for (size_t ii = 0; ii < args.size(); ++ii) {
        argv[ii] = &(args[ii][0]);
    }

    helics::FederateInfo f1(7, argv);
    EXPECT_EQ(f1.coreType, helics::core_type::INPROC);
    EXPECT_EQ(f1.defName, "f2");
    EXPECT_TRUE(f1.autobroker);
    EXPECT_EQ(f1.flagProps.size(), 2U);
}

TEST(federateInfo, constructor_error)
{
    std::vector<std::string> args{
        "constructor2", "--name", "f2", "--type", "inproc", "--brokerport=hippity_hopity"};
    char* argv[6];
    for (size_t ii = 0; ii < args.size(); ++ii) {
        argv[ii] = &(args[ii][0]);
    }

    EXPECT_THROW(helics::FederateInfo f1(6, argv), helics::InvalidParameter);
}

TEST(federateInfo, loadArgs1)
{
    std::vector<std::string> args{
        "constructor2", "--name", "f2", "--type", "zmq", "--flags", "realtime,source_only,17"};
    char* argv[7];
    for (size_t ii = 0; ii < args.size(); ++ii) {
        argv[ii] = &(args[ii][0]);
    }

    helics::FederateInfo f1;
    f1.loadInfoFromArgs(7, argv);
    EXPECT_EQ(f1.coreType, helics::core_type::ZMQ);
    EXPECT_EQ(f1.defName, "f2");
    EXPECT_EQ(f1.flagProps.size(), 3U);
}

TEST(federateInfo, constructor3)
{
    helics::FederateInfo f1{
        "--name f3 --type ipc --flags realtime;source_only,-buffer_data --port=5000"};
    EXPECT_EQ(f1.coreType, helics::core_type::INTERPROCESS);
    EXPECT_EQ(f1.defName, "f3");
    EXPECT_EQ(f1.flagProps.size(), 3U);
    EXPECT_EQ(f1.brokerPort, 5000);
}

TEST(federateInfo, loadArgs2)
{
    helics::FederateInfo f1;
    f1.loadInfoFromArgs(
        "--name f3 --type ipc --flags realtime;source_only,-buffer_data --port=5000 --RT_tolerance 200ms");
    EXPECT_EQ(f1.coreType, helics::core_type::INTERPROCESS);
    EXPECT_EQ(f1.defName, "f3");
    EXPECT_EQ(f1.flagProps.size(), 3U);
    EXPECT_EQ(f1.brokerPort, 5000);
    EXPECT_EQ(f1.timeProps.size(), 1U);
}

TEST(federateInfo, loadArgs_error)
{
    helics::FederateInfo f1;
    EXPECT_NO_THROW(f1.loadInfoFromArgs("--name f3 --type ipc --flags unrecognized --port=5000"));

    EXPECT_THROW(f1.loadInfoFromArgs("--name f3 --type ipc --brokerport=hippity_hopity"),
                 helics::InvalidParameter);
}

TEST(federateInfo, loadArgs_error2)
{
    std::vector<std::string> args{
        "constructor2", "--name", "f2", "--type", "zmq", "--brokerport=hippity_hopity"};
    char* argv[6];
    for (size_t ii = 0; ii < args.size(); ++ii) {
        argv[ii] = &(args[ii][0]);
    }

    helics::FederateInfo f1;
    EXPECT_THROW(f1.loadInfoFromArgs(6, argv), helics::InvalidParameter);
}

TEST(federateInfo, constructor4)
{
    helics::FederateInfo f1{
        "--log_level=no_print --brokerport=5000 --port=5005 --offset=5 --time_delta=45ms --max_iterations 10"};
    EXPECT_EQ(f1.brokerPort, 5000);
    EXPECT_EQ(f1.localport, "5005");
    EXPECT_EQ(f1.intProps.size(), 2U);
    EXPECT_EQ(f1.timeProps.size(), 2U);
}

TEST(federateInfo, constructor5)
{
    helics::FederateInfo f1{"--input_delay 50ms --brokerinit='--loglevel 3 --type=zmq'"};
    ASSERT_FALSE(f1.brokerInitString.empty());
    EXPECT_EQ(f1.brokerInitString.front(), ' ');
    EXPECT_EQ(f1.timeProps.size(), 1U);
}

TEST(federateInfo, constructor6)
{
    helics::FederateInfo f1{"--outputdelay=2 --separator=/ --rtlead=100ms --rtlag=50ms"};
    EXPECT_EQ(f1.timeProps.size(), 3U);
    EXPECT_EQ(f1.separator, '/');
}

TEST(federateInfo, constructor7)
{
    auto f1 = helics::loadFederateInfo("fedname7");
    EXPECT_EQ(f1.defName, "fedname7");
}

TEST(federateInfo, constructor_fail)
{
    EXPECT_THROW(helics::FederateInfo f1{"--inputdelay=2 --separator=cbc"}, std::exception);
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
    EXPECT_EQ(helics::getOptionIndex("single_connection"),
              helics_handle_option_single_connection_only);
    EXPECT_EQ(helics::getOptionIndex("StrictTypeChecking"),
              helics_handle_option_strict_type_checking);
    EXPECT_EQ(helics::getOptionIndex("un_interruptible"), helics_handle_option_ignore_interrupts);
}

TEST(federateInfo, flag_index)
{
    EXPECT_EQ(helics::getFlagIndex("StrictConfigChecking"), helics_flag_strict_config_checking);
    EXPECT_EQ(helics::getFlagIndex("un_interruptible"), helics_flag_uninterruptible);
    EXPECT_EQ(helics::getFlagIndex("strict_config_checking"), helics_flag_strict_config_checking);
}

TEST(federateInfo, loadinfoError)
{
    EXPECT_THROW(helics::loadFederateInfo("{\"log_level\":\"whatever\"}"),
                 helics::InvalidIdentifier);
}

TEST(federateInfo, loadinfoPropsJson)
{
    auto f1 = helics::loadFederateInfo(R"({"separator":":"})");
    EXPECT_EQ(f1.separator, ':');
    f1 = helics::loadFederateInfo(R"({"core":"zmq"})");
    EXPECT_EQ(f1.coreType, helics::core_type::ZMQ);
    f1 = helics::loadFederateInfo(R"({"core":"fred"})");
    EXPECT_EQ(f1.coreName, "fred");
    EXPECT_THROW(helics::loadFederateInfo("{\"coreType\":\"fred\"}"), helics::InvalidIdentifier);
    EXPECT_THROW(helics::loadFederateInfo("{\"coretype\":\"fred\"}"), helics::InvalidIdentifier);
    EXPECT_THROW(helics::loadFederateInfo("{\"type\":\"fred\"}"), helics::InvalidIdentifier);

    f1 = helics::loadFederateInfo(R"({"flags":"autobroker,source_only"})");
    EXPECT_EQ(f1.flagProps.size(), 1U);
    EXPECT_TRUE(f1.autobroker);

    f1 = helics::loadFederateInfo("{\"port\":5000}");
    EXPECT_EQ(f1.brokerPort, 5000);
    f1 = helics::loadFederateInfo(R"({"brokerport":5005,"port":5000})");
    EXPECT_EQ(f1.brokerPort, 5005);
    EXPECT_EQ(f1.localport, "5000");

    f1 = helics::loadFederateInfo(R"({"localport":5005,"port":5000})");
    EXPECT_EQ(f1.brokerPort, 5000);
    EXPECT_EQ(f1.localport, "5005");

    f1 = helics::loadFederateInfo("{\"loglevel\":5}");
    EXPECT_EQ(f1.intProps.size(), 1U);
    EXPECT_EQ(f1.intProps[0].second, 5);

    f1 = helics::loadFederateInfo(R"({"loglevel":"summary"})");
    EXPECT_EQ(f1.intProps.size(), 1U);
    EXPECT_EQ(f1.intProps[0].second, 2);

    EXPECT_EQ(f1.checkIntProperty(helics_property_int_log_level, -1), helics_log_level_summary);

    EXPECT_THROW(helics::loadFederateInfo("{\"loglevel\":\"unknown\"}"), helics::InvalidIdentifier);
}

TEST(federateInfo, loadinfoPropsToml)
{
    auto f1 = helics::loadFederateInfo(R"("separator"=":")");
    EXPECT_EQ(f1.separator, ':');
    f1 = helics::loadFederateInfo(R"("core"="zmq")");
    EXPECT_EQ(f1.coreType, helics::core_type::ZMQ);
    f1 = helics::loadFederateInfo(R"("core"="fred")");
    EXPECT_EQ(f1.coreName, "fred");
    EXPECT_THROW(helics::loadFederateInfo("\"coreType\"=\"fred\""), helics::InvalidIdentifier);
    EXPECT_THROW(helics::loadFederateInfo("\"coretype\"=\"fred\""), helics::InvalidIdentifier);
    EXPECT_THROW(helics::loadFederateInfo("\"type\"=\"fred\""), helics::InvalidIdentifier);

    f1 = helics::loadFederateInfo(R"("coreType"="web")");
    EXPECT_EQ(f1.coreType, helics::core_type::WEBSOCKET);

    f1 = helics::loadFederateInfo(R"("type"="UDP")");
    EXPECT_EQ(f1.coreType, helics::core_type::UDP);

    f1 = helics::loadFederateInfo(R"("flags"="autobroker,source_only")");
    EXPECT_EQ(f1.flagProps.size(), 1U);
    EXPECT_TRUE(f1.autobroker);

    f1 = helics::loadFederateInfo("\"port\"=5000");
    EXPECT_EQ(f1.brokerPort, 5000);
    f1 = helics::loadFederateInfo("\"brokerport\"=5005\n\"port\"=5000");
    EXPECT_EQ(f1.brokerPort, 5005);
    EXPECT_EQ(f1.localport, "5000");

    f1 = helics::loadFederateInfo("\"localport\"=5005\n\"port\"=5000");
    EXPECT_EQ(f1.brokerPort, 5000);
    EXPECT_EQ(f1.localport, "5005");

    f1 = helics::loadFederateInfo("\"loglevel\"=5");
    EXPECT_EQ(f1.intProps.size(), 1U);
    EXPECT_EQ(f1.intProps[0].second, 5);

    f1 = helics::loadFederateInfo(R"("loglevel"="summary")");
    EXPECT_EQ(f1.intProps.size(), 1U);
    EXPECT_EQ(f1.intProps[0].second, 2);
    EXPECT_THROW(helics::loadFederateInfo("\"loglevel\"=\"unknown\""), helics::InvalidIdentifier);
}

TEST(federateInfo, initString)
{
    helics::FederateInfo fi;
    fi.brokerPort = 6700;
    fi.localport = "5000";
    fi.key = "key";
    fi.broker = "broker2";
    fi.brokerInitString = "-f3";
    helics::FederateInfo f3(helics::generateFullCoreInitString(fi));

    EXPECT_EQ(fi.brokerPort, f3.brokerPort);
    EXPECT_EQ(fi.localport, f3.localport);
    EXPECT_EQ(fi.key, f3.key);
    EXPECT_EQ(fi.autobroker, f3.autobroker);
    EXPECT_EQ(fi.broker, f3.broker);

    EXPECT_TRUE(f3.brokerInitString.find(fi.brokerInitString) != std::string::npos);
}
