/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/FederateInfo.hpp"
#include "helics/core/core-exceptions.hpp"
#include "helics/core/helics_definitions.hpp"

#include "gtest/gtest.h"
#include <chrono>
#include <string>
#include <vector>

/** @file these test cases test out the federateInfo object
 */

TEST(federateInfo, constructor1)
{
    helics::FederateInfo fedInfo("--coretype=test --name fedInfo");
    EXPECT_EQ(fedInfo.coreType, helics::CoreType::TEST);
    EXPECT_EQ(fedInfo.defName, "fedInfo");
}

TEST(federateInfo, constructor2)
{
    std::vector<std::string> args{"constructor2",
                                  "--name",
                                  "f2",
                                  "--coretype",
                                  "inproc",
                                  "--flags",
                                  "realtime,,source_only;autobroker"};
    char* argv[7];
    for (size_t ii = 0; ii < args.size(); ++ii) {
        argv[ii] = args[ii].data();
    }

    helics::FederateInfo fedInfo(7, argv);
    EXPECT_EQ(fedInfo.coreType, helics::CoreType::INPROC);
    EXPECT_EQ(fedInfo.defName, "f2");
    EXPECT_TRUE(fedInfo.autobroker);
    EXPECT_EQ(fedInfo.flagProps.size(), 2U);
}

TEST(federateInfo, constructor_error)
{
    std::vector<std::string> args{
        "constructor2", "--name", "f2", "--coretype", "inproc", "--brokerport=hippity_hopity"};
    char* argv[6];
    for (size_t ii = 0; ii < args.size(); ++ii) {
        argv[ii] = args[ii].data();
    }

    EXPECT_THROW(helics::FederateInfo fedInfo(6, argv), helics::InvalidParameter);
}

TEST(federateInfo, loadArgs1)
{
    std::vector<std::string> args{
        "constructor2", "--name", "f2", "--coretype", "zmq", "--flags", "realtime,source_only,17"};
    char* argv[7];
    for (size_t ii = 0; ii < args.size(); ++ii) {
        argv[ii] = args[ii].data();
    }

    helics::FederateInfo fedInfo;
    fedInfo.loadInfoFromArgs(7, argv);
    EXPECT_EQ(fedInfo.coreType, helics::CoreType::ZMQ);
    EXPECT_EQ(fedInfo.defName, "f2");
    EXPECT_EQ(fedInfo.flagProps.size(), 3U);
}

TEST(federateInfo, constructor3)
{
    helics::FederateInfo fedInfo{
        "--name f3 --coretype ipc --flags realtime;source_only,-buffer_data --port=5000"};
    EXPECT_EQ(fedInfo.coreType, helics::CoreType::INTERPROCESS);
    EXPECT_EQ(fedInfo.defName, "f3");
    EXPECT_EQ(fedInfo.flagProps.size(), 3U);
    EXPECT_EQ(fedInfo.brokerPort, 5000);
}

TEST(federateInfo, loadArgs2)
{
    helics::FederateInfo fedInfo;
    fedInfo.loadInfoFromArgs(
        "--name f3 --coretype ipc --flags realtime;source_only,-buffer_data --port=5000 --RT_tolerance 200ms");
    EXPECT_EQ(fedInfo.coreType, helics::CoreType::INTERPROCESS);
    EXPECT_EQ(fedInfo.defName, "f3");
    EXPECT_EQ(fedInfo.flagProps.size(), 3U);
    EXPECT_EQ(fedInfo.brokerPort, 5000);
    EXPECT_EQ(fedInfo.timeProps.size(), 1U);
}

TEST(federateInfo, loadArgs_error)
{
    helics::FederateInfo fedInfo;
    EXPECT_NO_THROW(
        fedInfo.loadInfoFromArgs("--name f3 --coretype ipc --flags unrecognized --port=5000"));

    EXPECT_THROW(fedInfo.loadInfoFromArgs("--name f3 --coretype ipc --brokerport=hippity_hopity"),
                 helics::InvalidParameter);
}

TEST(federateInfo, loadArgs_error2)
{
    std::vector<std::string> args{
        "constructor2", "--name", "f2", "--coretype", "zmq", "--brokerport=hippity_hopity"};
    char* argv[6];
    for (size_t ii = 0; ii < args.size(); ++ii) {
        argv[ii] = args[ii].data();
    }

    helics::FederateInfo fedInfo;
    EXPECT_THROW(fedInfo.loadInfoFromArgs(6, argv), helics::InvalidParameter);
}

TEST(federateInfo, constructor4)
{
    helics::FederateInfo fedInfo{
        "--log_level=no_print --brokerport=5000 --port=5005 --offset=5 --time_delta=45ms --max_iterations 10"};
    EXPECT_EQ(fedInfo.brokerPort, 5000);
    EXPECT_EQ(fedInfo.localport, "5005");
    EXPECT_EQ(fedInfo.intProps.size(), 2U);
    EXPECT_EQ(fedInfo.timeProps.size(), 2U);
}

TEST(federateInfo, constructor5)
{
    helics::FederateInfo fedInfo{
        "--input_delay 50ms --broker_init_string='--loglevel 3 --coretype=zmq'"};
    ASSERT_FALSE(fedInfo.brokerInitString.empty());
    EXPECT_EQ(fedInfo.brokerInitString.front(), ' ');
    EXPECT_EQ(fedInfo.timeProps.size(), 1U);
}

TEST(federateInfo, constructor5b)
{
    helics::FederateInfo fedInfo{
        "--input_delay 50ms broker --initstring='--loglevel 3 --coretype=zmq' --port=765"};
    ASSERT_FALSE(fedInfo.brokerInitString.empty());
    EXPECT_EQ(fedInfo.brokerInitString.front(), ' ');
    EXPECT_EQ(fedInfo.timeProps.size(), 1U);
    EXPECT_EQ(fedInfo.brokerPort, 765);
}

TEST(federateInfo, constructor6)
{
    helics::FederateInfo fedInfo{"--outputdelay=2 --separator=/ --rtlead=100ms --rtlag=50ms"};
    EXPECT_EQ(fedInfo.timeProps.size(), 3U);
    EXPECT_EQ(fedInfo.separator, '/');
}

TEST(federateInfo, constructor7)
{
    auto fedInfo = helics::loadFederateInfo("fedname7");
    EXPECT_EQ(fedInfo.defName, "fedname7");
}

TEST(federateInfo, constructor_fail)
{
    EXPECT_THROW(helics::FederateInfo fedInfo{"--inputdelay=2 --separator=cbc"}, std::exception);
}

TEST(federateInfo, property_index)
{
    EXPECT_EQ(helics::getPropertyIndex("unexpected"), HELICS_INVALID_OPTION_INDEX);
    EXPECT_EQ(helics::getPropertyIndex("rtlag"), HELICS_PROPERTY_TIME_RT_LAG);
    EXPECT_EQ(helics::getPropertyIndex("RTlead"), HELICS_PROPERTY_TIME_RT_LEAD);
    EXPECT_EQ(helics::getPropertyIndex("RT__Tolerance"), HELICS_PROPERTY_TIME_RT_TOLERANCE);
}

TEST(federateInfo, option_index)
{
    EXPECT_EQ(helics::getOptionIndex("unexpected"), HELICS_INVALID_OPTION_INDEX);
    EXPECT_EQ(helics::getOptionIndex("single_connection_only"),
              HELICS_HANDLE_OPTION_SINGLE_CONNECTION_ONLY);
    EXPECT_EQ(helics::getOptionIndex("strictInputTypeChecking"),
              HELICS_HANDLE_OPTION_STRICT_TYPE_CHECKING);
    EXPECT_EQ(helics::getOptionIndex("un_interruptible"), HELICS_HANDLE_OPTION_IGNORE_INTERRUPTS);
}

TEST(federateInfo, flag_index)
{
    EXPECT_EQ(helics::getFlagIndex("StrictConfigChecking"), HELICS_FLAG_STRICT_CONFIG_CHECKING);
    EXPECT_EQ(helics::getFlagIndex("un_interruptible"), HELICS_FLAG_UNINTERRUPTIBLE);
    EXPECT_EQ(helics::getFlagIndex("strict_config_checking"), HELICS_FLAG_STRICT_CONFIG_CHECKING);
}

TEST(federateInfo, loadinfoError)
{
    EXPECT_THROW(helics::loadFederateInfo("{\"log_level\":\"whatever\"}"),
                 helics::InvalidIdentifier);
}

TEST(federateInfo, loadinfoPropsJson)
{
    auto fedInfo = helics::loadFederateInfo(R"({"separator":":"})");
    EXPECT_EQ(fedInfo.separator, ':');
    fedInfo = helics::loadFederateInfo(R"({"core":"zmq"})");
    EXPECT_EQ(fedInfo.coreType, helics::CoreType::ZMQ);
    fedInfo = helics::loadFederateInfo(R"({"core":"fred"})");
    EXPECT_EQ(fedInfo.coreName, "fred");
    EXPECT_THROW(helics::loadFederateInfo("{\"coreType\":\"fred\"}"), helics::InvalidIdentifier);
    EXPECT_THROW(helics::loadFederateInfo("{\"coretype\":\"fred\"}"), helics::InvalidIdentifier);
    EXPECT_THROW(helics::loadFederateInfo("{\"coretype\":\"fred\"}"), helics::InvalidIdentifier);

    fedInfo = helics::loadFederateInfo(R"({"flags":"autobroker,source_only"})");
    EXPECT_EQ(fedInfo.flagProps.size(), 1U);
    EXPECT_TRUE(fedInfo.autobroker);

    fedInfo = helics::loadFederateInfo("{\"port\":5000}");
    EXPECT_EQ(fedInfo.brokerPort, 5000);
    fedInfo = helics::loadFederateInfo(R"({"brokerport":5005,"port":5000})");
    EXPECT_EQ(fedInfo.brokerPort, 5005);
    EXPECT_EQ(fedInfo.localport, "5000");

    fedInfo = helics::loadFederateInfo(R"({"localport":5005,"port":5000})");
    EXPECT_EQ(fedInfo.brokerPort, 5000);
    EXPECT_EQ(fedInfo.localport, "5005");

    fedInfo = helics::loadFederateInfo(R"({"loglevel":"timing"})");
    EXPECT_EQ(fedInfo.intProps.size(), 1U);
    EXPECT_EQ(fedInfo.intProps[0].second, HELICS_LOG_LEVEL_TIMING);

    fedInfo = helics::loadFederateInfo(R"({"loglevel":"summary"})");
    EXPECT_EQ(fedInfo.intProps.size(), 1U);
    EXPECT_EQ(fedInfo.intProps[0].second, HELICS_LOG_LEVEL_SUMMARY);

    EXPECT_EQ(fedInfo.checkIntProperty(HELICS_PROPERTY_INT_LOG_LEVEL, HELICS_LOG_LEVEL_NO_PRINT),
              HELICS_LOG_LEVEL_SUMMARY);

    EXPECT_THROW(helics::loadFederateInfo("{\"loglevel\":\"unknown\"}"), helics::InvalidIdentifier);
}

TEST(federateInfo, loadinfoPropsToml)
{
    auto fedInfo = helics::loadFederateInfo(R"("separator"=":")");
    EXPECT_EQ(fedInfo.separator, ':');
    fedInfo = helics::loadFederateInfo(R"("core"="zmq")");
    EXPECT_EQ(fedInfo.coreType, helics::CoreType::ZMQ);
    fedInfo = helics::loadFederateInfo(R"("core"="fred")");
    EXPECT_EQ(fedInfo.coreName, "fred");
    EXPECT_THROW(helics::loadFederateInfo("\"coreType\"=\"fred\""), helics::InvalidIdentifier);
    EXPECT_THROW(helics::loadFederateInfo("\"coretype\"=\"fred\""), helics::InvalidIdentifier);
    EXPECT_THROW(helics::loadFederateInfo("\"coretype\"=\"fred\""), helics::InvalidIdentifier);

    fedInfo = helics::loadFederateInfo(R"("coreType"="web")");
    EXPECT_EQ(fedInfo.coreType, helics::CoreType::WEBSOCKET);

    fedInfo = helics::loadFederateInfo(R"("coretype"="UDP")");
    EXPECT_EQ(fedInfo.coreType, helics::CoreType::UDP);

    fedInfo = helics::loadFederateInfo(R"("flags"="autobroker,source_only")");
    EXPECT_EQ(fedInfo.flagProps.size(), 1U);
    EXPECT_TRUE(fedInfo.autobroker);

    fedInfo = helics::loadFederateInfo("\"port\"=5000");
    EXPECT_EQ(fedInfo.brokerPort, 5000);
    fedInfo = helics::loadFederateInfo("\"brokerport\"=5005\n\"port\"=5000");
    EXPECT_EQ(fedInfo.brokerPort, 5005);
    EXPECT_EQ(fedInfo.localport, "5000");

    fedInfo = helics::loadFederateInfo("\"localport\"=5005\n\"port\"=5000");
    EXPECT_EQ(fedInfo.brokerPort, 5000);
    EXPECT_EQ(fedInfo.localport, "5005");

    fedInfo = helics::loadFederateInfo(R"("loglevel"="timing")");
    EXPECT_EQ(fedInfo.intProps.size(), 1U);
    EXPECT_EQ(fedInfo.intProps[0].second, HELICS_LOG_LEVEL_TIMING);

    fedInfo = helics::loadFederateInfo(R"("loglevel"="summary")");
    EXPECT_EQ(fedInfo.intProps.size(), 1U);
    EXPECT_EQ(fedInfo.intProps[0].second, HELICS_LOG_LEVEL_SUMMARY);
    EXPECT_THROW(helics::loadFederateInfo("\"loglevel\"=\"unknown\""), helics::InvalidIdentifier);
}

TEST(federateInfo, initString)
{
    helics::FederateInfo fedInfo;
    fedInfo.brokerPort = 6700;
    fedInfo.localport = "5000";
    fedInfo.key = "broker_key";
    fedInfo.broker = "broker2";
    fedInfo.brokerInitString = "-f3";
    helics::FederateInfo fedInfo3(helics::generateFullCoreInitString(fedInfo));

    EXPECT_EQ(fedInfo.brokerPort, fedInfo3.brokerPort);
    EXPECT_EQ(fedInfo.localport, fedInfo3.localport);
    EXPECT_EQ(fedInfo.key, fedInfo3.key);
    EXPECT_EQ(fedInfo.autobroker, fedInfo3.autobroker);
    EXPECT_EQ(fedInfo.broker, fedInfo3.broker);

    EXPECT_TRUE(fedInfo3.brokerInitString.find(fedInfo.brokerInitString) != std::string::npos);
}
