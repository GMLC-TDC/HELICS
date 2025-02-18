/*
Copyright (c) 2017-2025,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helics/application_api/Filters.hpp"
#include "helics/application_api/Publications.hpp"
#include "helics/apps/Clone.hpp"
#include "helics/apps/Player.hpp"

#include "gtest/gtest.h"
#include <cstdio>
#include <filesystem>
#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

TEST(clone_tests, simple_clone_test_pub)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "clone_core1";
    fedInfo.coreInitString = "-f 2 --autobroker";
    helics::apps::Clone cloner("c1", fedInfo);
    cloner.setFederateToClone("block1");

    helics::ValueFederate vfed("block1", fedInfo);
    helics::Publication pub1(helics::InterfaceVisibility::GLOBAL,
                             &vfed,
                             "pub1",
                             helics::DataType::HELICS_DOUBLE);
    auto fut = std::async(std::launch::async, [&cloner]() { cloner.runTo(4); });
    vfed.enterExecutingMode();
    auto retTime = vfed.requestTime(1);
    EXPECT_EQ(retTime, 1.0);
    pub1.publish(3.4);

    retTime = vfed.requestTime(2.0);
    EXPECT_EQ(retTime, 2.0);
    pub1.publish(4.7);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 5.0);

    vfed.finalize();
    fut.get();
    cloner.finalize();
    auto cnt = cloner.pointCount();
    EXPECT_EQ(cnt, 2U);
}

TEST(clone_tests, simple_clone_test_pub_arg)
{
    std::string coreInitString = "-f 2 --autobroker block1 --coretype=test --corename=clone_core1";
    helics::apps::Clone cloner("c1", coreInitString);

    helics::ValueFederate vfed("block1", "--coretype=test --corename=clone_core1");
    helics::Publication pub1(helics::InterfaceVisibility::GLOBAL,
                             &vfed,
                             "pub1",
                             helics::DataType::HELICS_DOUBLE);
    auto fut = std::async(std::launch::async, [&cloner]() { cloner.runTo(4); });
    vfed.enterExecutingMode();
    auto retTime = vfed.requestTime(1);
    EXPECT_EQ(retTime, 1.0);
    pub1.publish(3.4);

    retTime = vfed.requestTime(2.0);
    EXPECT_EQ(retTime, 2.0);
    pub1.publish(4.7);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 5.0);

    vfed.finalize();
    fut.get();
    cloner.finalize();
    auto cnt = cloner.pointCount();
    EXPECT_EQ(cnt, 2U);
}

TEST(clone_tests, simple_clone_test_pub_arg2)
{
    std::string coreInitString =
        "-f 2 --autobroker --capture=block1 --coretype=test --corename=clone_core1";
    helics::apps::Clone cloner("c1", coreInitString);

    helics::ValueFederate vfed("block1", "--coretype=test --corename=clone_core1");
    helics::Publication pub1(helics::InterfaceVisibility::GLOBAL,
                             &vfed,
                             "pub1",
                             helics::DataType::HELICS_DOUBLE);
    auto fut = std::async(std::launch::async, [&cloner]() { cloner.runTo(4); });
    vfed.enterExecutingMode();
    auto retTime = vfed.requestTime(1);
    EXPECT_EQ(retTime, 1.0);
    pub1.publish(3.4);

    retTime = vfed.requestTime(2.0);
    EXPECT_EQ(retTime, 2.0);
    pub1.publish(4.7);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 5.0);

    vfed.finalize();
    fut.get();
    cloner.finalize();
    auto cnt = cloner.pointCount();
    EXPECT_EQ(cnt, 2U);
}

TEST(clone_tests, simple_clone_test_pub2)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "clone_core2";
    fedInfo.coreInitString = "-f 2 --autobroker";
    helics::apps::Clone cloner("c1", fedInfo);

    cloner.setFederateToClone("block1");

    helics::ValueFederate vfed("block1", fedInfo);
    helics::Publication pub1(helics::InterfaceVisibility::GLOBAL,
                             &vfed,
                             "pub1",
                             helics::DataType::HELICS_DOUBLE);

    auto& pub2 = vfed.registerPublication("pub2", "double", "m");

    auto fut = std::async(std::launch::async, [&cloner]() { cloner.runTo(4); });
    vfed.enterExecutingMode();
    auto retTime = vfed.requestTime(1);
    EXPECT_EQ(retTime, 1.0);
    pub1.publish(3.4);

    retTime = vfed.requestTime(2.0);
    EXPECT_EQ(retTime, 2.0);
    pub1.publish(4.7);
    pub2.publish(3.3);
    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 5.0);

    vfed.finalize();
    fut.get();
    cloner.finalize();
    auto cnt = cloner.pointCount();
    EXPECT_EQ(cnt, 3U);
    auto icnt = cloner.accessUnderlyingFederate().getInputCount();
    EXPECT_EQ(icnt, 2);
    cloner.saveFile("pubtest2.json");

    ASSERT_TRUE(std::filesystem::exists("pubtest2.json"));

    auto fi2 = helics::loadFederateInfo("pubtest2.json");
    fi2.coreName = "clone_core3";
    fi2.coreInitString = "--autobroker";
    fi2.coreType = helics::CoreType::TEST;
    helics::apps::Player player("p1", fi2);
    player.initialize();

    EXPECT_EQ(player.pointCount(), 3U);
    EXPECT_EQ(player.publicationCount(), 2U);
    player.finalize();
    std::filesystem::remove("pubtest2.json");
}

TEST(clone_tests, simple_clone_test_message)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "clone_core4";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    fedInfo.coreInitString = "-f 2 --autobroker";
    helics::apps::Clone cloner("c1", fedInfo);
    cloner.setFederateToClone("block1");

    helics::MessageFederate mfed("block1", fedInfo);
    auto& ept = mfed.registerGlobalEndpoint("ept1", "etype");
    auto& ept2 = mfed.registerGlobalEndpoint("ept3");
    mfed.registerEndpoint("e3");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    auto fut = std::async(std::launch::async, [&cloner]() { cloner.runTo(4); });
    mfed.enterExecutingMode();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    auto retTime = mfed.requestTime(1);
    EXPECT_EQ(retTime, 1.0);
    ept.sendTo("message", "ept3");

    retTime = mfed.requestTime(2.0);
    EXPECT_EQ(retTime, 2.0);
    ept2.sendTo("reply", "ept1");

    retTime = mfed.requestTime(5);
    EXPECT_EQ(retTime, 3.0);

    mfed.finalize();
    fut.get();
    cloner.finalize();
    auto cnt = cloner.messageCount();
    EXPECT_EQ(cnt, 2U);
    cloner.saveFile("eptsave.json");
    // now test the files
    auto fi2 = helics::loadFederateInfo("eptsave.json");
    fi2.coreName = "clone_core5";
    fi2.coreInitString = "--autobroker";
    fi2.coreType = helics::CoreType::TEST;
    helics::apps::Player player("p1", fi2);

    player.initialize();

    EXPECT_EQ(player.messageCount(), 2U);
    EXPECT_EQ(player.endpointCount(), 3U);
    player.finalize();
    std::filesystem::remove("eptsave.json");
}

TEST(clone_tests, simple_clone_test_combo)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "clone_core6";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    fedInfo.coreInitString = "-f 2 --autobroker";
    helics::apps::Clone cloner("c1", fedInfo);
    cloner.setFederateToClone("block1");

    helics::CombinationFederate mfed("block1", fedInfo);
    auto& ept = mfed.registerGlobalEndpoint("ept1", "etype");
    auto& ept2 = mfed.registerGlobalEndpoint("ept3");
    mfed.registerEndpoint("e3");

    helics::Publication pub1(helics::InterfaceVisibility::GLOBAL,
                             &mfed,
                             "pub1",
                             helics::DataType::HELICS_DOUBLE);

    auto& pub2 = mfed.registerPublication("pub2", "double", "m");

    auto fut = std::async(std::launch::async, [&cloner]() { cloner.runTo(6); });
    mfed.enterExecutingMode();
    auto retTime = mfed.requestTime(1);
    EXPECT_EQ(retTime, 1.0);
    ept.sendTo("message", "ept3");
    pub1.publish(3.4);
    retTime = mfed.requestTime(2.0);
    EXPECT_EQ(retTime, 2.0);
    ept2.sendTo("reply", "ept1");
    pub1.publish(4.7);
    pub2.publish(3.3);
    retTime = mfed.requestTime(5);
    EXPECT_EQ(retTime, 3.0);
    retTime = mfed.requestTime(5);
    EXPECT_EQ(retTime, 5.0);
    mfed.finalize();
    fut.get();
    auto epts = cloner.accessUnderlyingFederate().getEndpointCount();
    EXPECT_EQ(epts, 1);
    auto filts = cloner.accessUnderlyingFederate().getFilterCount();
    EXPECT_EQ(filts, 1);

    auto ipts = cloner.accessUnderlyingFederate().getInputCount();
    EXPECT_EQ(ipts, 2);
    cloner.finalize();

    auto cnt = cloner.messageCount();
    EXPECT_EQ(cnt, 2U);

    int pcnt = static_cast<int>(cloner.pointCount());
    EXPECT_EQ(pcnt, 3);
    if (pcnt != 3) {
        for (int ii = 0; ii < static_cast<int>(pcnt); ++ii) {
            auto pointValue = cloner.getValue(ii);
            std::cout << "Point " << ii << " source:" << std::get<1>(pointValue) << " at "
                      << std::get<0>(pointValue) << ", value: " << std::get<2>(pointValue) << '\n';
        }
        EXPECT_EQ(std::get<0>(cloner.getValue(static_cast<int>(pcnt))), helics::Time{});
    }

    cloner.saveFile("combsave.json");

    auto fi2 = helics::loadFederateInfo("combsave.json");
    fi2.coreName = "clone_core7";
    fi2.coreInitString = "--autobroker";
    fi2.coreType = helics::CoreType::TEST;
    helics::apps::Player player("p1", fi2);

    player.initialize();

    EXPECT_EQ(player.messageCount(), 2U);
    EXPECT_EQ(player.endpointCount(), 3U);
    EXPECT_EQ(static_cast<int>(player.pointCount()), pcnt);
    EXPECT_EQ(player.publicationCount(), 2U);
    player.finalize();
    std::filesystem::remove("combsave.json");
}

TEST(clone_tests, simple_clone_test_sub)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "clone_core8";
    fedInfo.coreInitString = "-f 3 --autobroker";
    helics::apps::Clone cloner("c1", fedInfo);

    cloner.setFederateToClone("block1");

    helics::ValueFederate vfed("block1", fedInfo);
    helics::ValueFederate vfed2("block2", fedInfo);
    helics::Publication pub1(helics::InterfaceVisibility::GLOBAL,
                             &vfed,
                             "pub1",
                             helics::DataType::HELICS_DOUBLE);

    auto& pub2 = vfed.registerPublication("pub2", "double", "m");

    vfed2.registerPublication<double>("pub");

    vfed2.registerPublication<std::string>("pub2");

    vfed.registerSubscription("block2/pub2");

    vfed.registerSubscription("block2/pub");

    auto fut = std::async(std::launch::async, [&cloner]() { cloner.runTo(6); });
    vfed2.enterExecutingModeAsync();
    vfed.enterExecutingMode();
    vfed2.enterExecutingModeComplete();
    vfed2.requestTimeAsync(5);
    auto retTime = vfed.requestTime(1);
    vfed2.requestTimeComplete();
    EXPECT_EQ(retTime, 1.0);
    pub1.publish(3.4);

    retTime = vfed.requestTime(2.0);
    EXPECT_EQ(retTime, 2.0);
    pub1.publish(4.7);
    pub2.publish(3.3);
    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 5.0);
    vfed2.finalize();
    vfed.finalize();
    fut.get();

    auto cnt = cloner.pointCount();
    EXPECT_EQ(cnt, 3U);
    auto icnt = cloner.accessUnderlyingFederate().getInputCount();
    EXPECT_EQ(icnt, 2);
    cloner.finalize();
    cloner.saveFile("subtest.json");

    ASSERT_TRUE(std::filesystem::exists("subtest.json"));

    // now test the file loading
    auto fi2 = helics::loadFederateInfo("subtest.json");
    fi2.coreName = "clone_core9";
    fi2.coreInitString = "--autobroker";
    fi2.coreType = helics::CoreType::TEST;
    helics::apps::Player player1("p1", fi2);

    player1.initialize();

    EXPECT_EQ(player1.pointCount(), 3U);
    EXPECT_EQ(player1.publicationCount(), 2U);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // this depends on some processing occurring
    EXPECT_EQ(player1.accessUnderlyingFederate().getInputCount(), 2);
    player1.finalize();
    std::filesystem::remove("subtest.json");
}

TEST(clone_tests, clone_test_help)
{
    std::vector<std::string> args{"--quiet", "--version"};
    helics::apps::Clone cloner(args);

    EXPECT_TRUE(!cloner.isActive());

    args.emplace_back("-?");
    helics::apps::Clone cloner2(args);

    EXPECT_TRUE(!cloner2.isActive());
}
