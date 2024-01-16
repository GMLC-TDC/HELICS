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
#include "helics/apps/CoreApp.hpp"
#include "helics/apps/Connector.hpp"

#include <future>
#include <thread>

static const std::string testdir=std::string(TEST_DIR)+"/connector/";

TEST(connector_file_tests, simple_connector)
{
    helics::FederateInfo fi(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fi.coreName = "ccore1";
    fi.coreInitString = "-f2 --autobroker";
    fi.setProperty(HELICS_PROPERTY_TIME_PERIOD,1.0);
    helics::apps::Connector conn1("connector1", fi);
    conn1.loadFile(testdir+"simple.txt");
    
    helics::ValueFederate vfed("c1", fi);
    auto& pub1 = vfed.registerGlobalPublication<double>("pub1");
    auto& inp1 = vfed.registerGlobalInput<double>("inp1");

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed.enterExecutingMode();
    const double testValue=3452.562;
    pub1.publish(testValue);
    auto retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    auto val = inp1.getDouble();
    EXPECT_EQ(val, testValue);

    vfed.finalize();
    fut.get();
    EXPECT_EQ(conn1.madeConnections(),1);
}


TEST(connector_file_tests, simple_connector_reverse)
{
    helics::FederateInfo fi(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fi.coreName = "ccore1";
    fi.coreInitString = "-f2 --autobroker";
    fi.setProperty(HELICS_PROPERTY_TIME_PERIOD,1.0);
    helics::apps::Connector conn1("connector1", fi);
    conn1.loadFile(testdir+"simple.json");

    helics::ValueFederate vfed("c1", fi);
    auto& pub1 = vfed.registerGlobalPublication<double>("pub1");
    auto& inp1 = vfed.registerGlobalInput<double>("inp1");

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed.enterExecutingMode();
    const double testValue=3452.562;
    pub1.publish(testValue);
    auto retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    auto val = inp1.getDouble();
    EXPECT_EQ(val, testValue);

    vfed.finalize();
    fut.get();
}


TEST(connector_file_tests, connector_cascade)
{
    helics::FederateInfo fi(helics::CoreType::TEST);
    using helics::apps::InterfaceDirection;

    fi.coreName = "ccore1";
    fi.coreInitString = "-f2 --autobroker";
    fi.setProperty(HELICS_PROPERTY_TIME_PERIOD,1.0);
    helics::apps::Connector conn1("connector1", fi);
    conn1.loadFile(testdir+"cascade.txt");

    helics::ValueFederate vfed("c1", fi);
    auto& pub1 = vfed.registerGlobalPublication<double>("pub1");
    auto& inp1 = vfed.registerGlobalInput<double>("inp1");
    auto& inp2 = vfed.registerGlobalInput<double>("inp2");

    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    vfed.enterExecutingMode();
    const double testValue=3452.562;
    pub1.publish(testValue);
    auto retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    auto val = inp1.getDouble();
    EXPECT_EQ(val, testValue);
    val = inp2.getDouble();
    EXPECT_EQ(val, testValue);
    vfed.finalize();
    fut.get();
}

