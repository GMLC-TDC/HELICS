/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "exeTestHelper.h"
#include "helics/application_api/Endpoints.hpp"
#include "helics/apps/Probe.hpp"
#include "helics/core/BrokerFactory.hpp"

#include "gtest/gtest.h"
#include <cstdio>
#include <future>
#include <thread>

// this test will test basic probe functionality
TEST(probe, probe2)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);

    fedInfo.coreName = "ecore1";
    fedInfo.coreInitString = "--autobroker";
    fedInfo.brokerInitString = "-f 2";
    fedInfo.setProperty(HELICS_PROPERTY_INT_LOG_LEVEL, HELICS_LOG_LEVEL_SUMMARY);
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Probe probe1("probe1", fedInfo);
    fedInfo.coreInitString.clear();
    helics::apps::Probe probe2("probe2", fedInfo);

    auto fut1 = std::async(std::launch::async, [&probe1]() { probe1.runTo(3.0); });
    auto fut2 = std::async(std::launch::async, [&probe2]() { probe2.runTo(3.0); });

    fut1.get();
    fut2.get();
    EXPECT_EQ(probe1.getConnections(), 1);
    EXPECT_EQ(probe2.getConnections(), 1);

    EXPECT_EQ(probe1.getMessageCount(), 3);
    EXPECT_EQ(probe2.getMessageCount(), 3);
    probe1.finalize();
    probe2.finalize();
}

TEST(probe, probe4)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);

    fedInfo.coreInitString = "--autobroker";
    fedInfo.brokerInitString = "-f 4";
    fedInfo.setProperty(HELICS_PROPERTY_INT_LOG_LEVEL, HELICS_LOG_LEVEL_SUMMARY);
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Probe probe1("probe1", fedInfo);
    fedInfo.coreInitString.clear();
    helics::apps::Probe probe2("probe2", fedInfo);
    helics::apps::Probe probe3("probe3", fedInfo);
    helics::apps::Probe probe4("probe4", fedInfo);

    auto fut1 = std::async(std::launch::async, [&probe1]() { probe1.runTo(4.0); });
    auto fut2 = std::async(std::launch::async, [&probe2]() { probe2.runTo(4.0); });
    auto fut3 = std::async(std::launch::async, [&probe3]() { probe3.runTo(4.0); });
    auto fut4 = std::async(std::launch::async, [&probe4]() { probe4.runTo(4.0); });

    fut1.get();
    fut2.get();
    fut3.get();
    fut4.get();
    EXPECT_EQ(probe1.getConnections(), 3);
    EXPECT_EQ(probe2.getConnections(), 3);
    EXPECT_EQ(probe3.getConnections(), 3);
    EXPECT_EQ(probe4.getConnections(), 3);

    EXPECT_EQ(probe1.getMessageCount(), 12);
    EXPECT_EQ(probe2.getMessageCount(), 12);
    EXPECT_EQ(probe3.getMessageCount(), 12);
    EXPECT_EQ(probe4.getMessageCount(), 12);
    probe1.finalize();
    probe2.finalize();
    probe3.finalize();
    probe4.finalize();
}
