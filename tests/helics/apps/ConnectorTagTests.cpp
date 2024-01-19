/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "gtest/gtest.h"
#include <cstdio>
#ifndef DISABLE_SYSTEM_CALL_TESTS
#    include "exeTestHelper.h"
#endif

#include "helics/apps/BrokerApp.hpp"
#include "helics/apps/Connector.hpp"
#include "helics/apps/CoreApp.hpp"

#include <future>
#include <thread>

static constexpr std::string_view testdir = TEST_DIR "/connector/";

TEST(connector_tags, no_match_tag)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fedInfo.coreName = "ccore1";
    fedInfo.coreInitString = "-f2 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::apps::Connector conn1("connector1", fedInfo);
    conn1.addConnection("inp1", "pub1", InterfaceDirection::FROM_TO,{"tag1"});

    helics::ValueFederate vfed("c1", fedInfo);
    auto& pub1 = vfed.registerGlobalPublication<double>("pub1");
    auto& inp1 = vfed.registerGlobalInput<double>("inp1");

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed.enterExecutingMode();
    const double testValue = 3452.562;
    pub1.publish(testValue);
    auto retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    auto val = inp1.getDouble();
    EXPECT_EQ(val, testValue);

    vfed.finalize();
    fut.get();
    EXPECT_EQ(conn1.madeConnections(), 0);
}
