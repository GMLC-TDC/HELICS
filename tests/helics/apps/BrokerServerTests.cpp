/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "exeTestHelper.h"
#include "helics/apps/BrokerServer.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/coreTypeOperations.hpp"
#include "helics/helics.hpp"

#include "gtest/gtest.h"
#include <cstdio>

using namespace helics;

class BrokerServerTests: public ::testing::TestWithParam<std::pair<const char*, core_type>> {
};

TEST_P(BrokerServerTests, startup_tests)
{
    if (!core::isCoreTypeAvailable(GetParam().second)) {
        return;
    }
    apps::BrokerServer brks(std::vector<std::string>{GetParam().first});

    brks.forceTerminate();
    cleanupHelicsLibrary();

    bool active = brks.hasActiveBrokers();
    if (active) {
        cleanupHelicsLibrary();
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        active = brks.hasActiveBrokers();
    }
    EXPECT_TRUE(!active);
    brks.startServers();

    auto cr = helics::CoreFactory::create(GetParam().second, "--brokername=fred");
    EXPECT_TRUE(cr->isConfigured());
    cr->connect();
    EXPECT_TRUE(cr->isConnected());

    auto cr2 = helics::CoreFactory::create(GetParam().second, "--brokername=fred2");
    EXPECT_TRUE(cr2->isConfigured());
    cr2->connect();
    EXPECT_TRUE(cr2->isConnected());

    auto objs = helics::BrokerFactory::getAllBrokers();
    EXPECT_EQ(objs.size(), 2u);

    brks.forceTerminate();
    cr->disconnect();
    cr2->disconnect();
    auto crdisconnect = cr->waitForDisconnect(std::chrono::milliseconds(1000));
    auto cr2disconnect = cr2->waitForDisconnect(std::chrono::milliseconds(1000));
    if (!crdisconnect) {
        crdisconnect = cr->waitForDisconnect(std::chrono::milliseconds(1000));
    }
    if (!cr2) {
        cr2disconnect = cr2->waitForDisconnect(std::chrono::milliseconds(1000));
    }
    EXPECT_TRUE(crdisconnect);
    EXPECT_TRUE(cr2disconnect);
    cleanupHelicsLibrary();
}

TEST_P(BrokerServerTests, execution_tests)
{
    if (!core::isCoreTypeAvailable(GetParam().second)) {
        return;
    }
    apps::BrokerServer brks(std::vector<std::string>{GetParam().first});
    brks.startServers();

    FederateInfo fi(GetParam().second);
    fi.coreName = "c1";
    fi.brokerInitString = "-f 2";
    auto fed1 = ValueFederate("fed1", fi);
    fed1.registerGlobalPublication("key1", "double");
    fi.coreName = "c2";
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    auto fed2 = ValueFederate("fed2", fi);
    auto& sub = fed2.registerSubscription("key1");
    sub.setOption(helics_handle_option_connection_required);
    fed1.enterExecutingModeAsync();
    EXPECT_NO_THROW(fed2.enterExecutingMode());
    fed1.enterExecutingModeComplete();

    fed1.finalize();
    fed2.finalize();
    brks.forceTerminate();
    cleanupHelicsLibrary();
}

TEST_P(BrokerServerTests, execution_tests_duplicate)
{
    if (!core::isCoreTypeAvailable(GetParam().second)) {
        return;
    }
    apps::BrokerServer brks(std::vector<std::string>{GetParam().first});
    brks.startServers();

    FederateInfo fi(GetParam().second);
    fi.coreName = "c1b";

    auto fed1 = ValueFederate("fed1", fi);
    auto& pub1 = fed1.registerGlobalPublication("key1", "double");
    fi.coreName = "c2b";
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    auto fed2 = ValueFederate("fed2", fi);
    auto& sub2 = fed2.registerSubscription("key1");
    sub2.setOption(helics_handle_option_connection_required);
    fed1.enterExecutingModeAsync();
    EXPECT_NO_THROW(fed2.enterExecutingMode());
    fed1.enterExecutingModeComplete();

    fi.coreName = "c3b";
    // this would test two co-sims executing simultaneously
    auto fed3 = ValueFederate("fed3", fi);
    auto& pub3 = fed3.registerGlobalPublication("key1", "double");
    fi.coreName = "c4b";
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    auto fed4 = ValueFederate("fed4", fi);
    auto& sub4 = fed4.registerSubscription("key1");
    sub4.setOption(helics_handle_option_connection_required);
    fed3.enterExecutingModeAsync();
    EXPECT_NO_THROW(fed4.enterExecutingMode());
    fed3.enterExecutingModeComplete();

    pub1.publish(27.5);
    pub3.publish(30.6);

    fed3.requestTimeAsync(1.0);
    fed4.requestTime(1.0);
    fed3.requestTimeComplete();

    fed1.requestTimeAsync(1.0);
    fed2.requestTime(1.0);
    fed1.requestTimeComplete();

    EXPECT_EQ(sub2.getValue<double>(), 27.5);
    EXPECT_EQ(sub4.getValue<double>(), 30.6);
    fed1.finalize();
    fed2.finalize();
    fed3.finalize();
    fed4.finalize();
    brks.forceTerminate();
    cleanupHelicsLibrary();
}

const std::vector<std::pair<const char*, core_type>> tvals{{"--zmq", core_type::ZMQ},
                                                           {"--zmqss", core_type::ZMQ_SS},
                                                           {"--tcp", core_type::TCP},
                                                           {"--udp", core_type::UDP}};

INSTANTIATE_TEST_SUITE_P(broker_server_tests, BrokerServerTests, ::testing::ValuesIn(tvals));
