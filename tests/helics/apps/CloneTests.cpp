/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "gtest/gtest.h"

#ifdef _MSC_VER
#    pragma warning(push, 0)
#    include "helics/external/filesystem.hpp"
#    pragma warning(pop)
#else
#    include "helics/external/filesystem.hpp"
#endif

#include "helics/application_api/Publications.hpp"
#include "helics/apps/Clone.hpp"
#include "helics/apps/Player.hpp"

#include <cstdio>
#include <future>

TEST(clone_tests, simple_clone_test_pub)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "clone_core1";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Clone c1("c1", fi);
    c1.setFederateToClone("block1");

    helics::ValueFederate vfed("block1", fi);
    helics::Publication pub1(helics::GLOBAL, &vfed, "pub1", helics::data_type::helics_double);
    auto fut = std::async(std::launch::async, [&c1]() { c1.runTo(4); });
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
    c1.finalize();
    auto cnt = c1.pointCount();
    EXPECT_EQ(cnt, 2u);
}

TEST(clone_tests, simple_clone_test_pub2)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "clone_core2";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Clone c1("c1", fi);

    c1.setFederateToClone("block1");

    helics::ValueFederate vfed("block1", fi);
    helics::Publication pub1(helics::GLOBAL, &vfed, "pub1", helics::data_type::helics_double);

    auto& pub2 = vfed.registerPublication("pub2", "double", "m");

    auto fut = std::async(std::launch::async, [&c1]() { c1.runTo(4); });
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
    c1.finalize();
    auto cnt = c1.pointCount();
    EXPECT_EQ(cnt, 3u);
    auto icnt = c1.accessUnderlyingFederate().getInputCount();
    EXPECT_EQ(icnt, 2);
    c1.saveFile("pubtest2.json");

    ASSERT_TRUE(ghc::filesystem::exists("pubtest2.json"));

    auto fi2 = helics::loadFederateInfo("pubtest2.json");
    fi2.coreName = "clone_core3";
    fi2.coreInitString = "--autobroker";
    fi2.coreType = helics::core_type::TEST;
    helics::apps::Player p1("p1", fi2);
    p1.loadFile("pubtest2.json");

    p1.initialize();

    EXPECT_EQ(p1.pointCount(), 3u);
    EXPECT_EQ(p1.publicationCount(), 2u);
    p1.finalize();
    ghc::filesystem::remove("pubtest2.json");
}

TEST(clone_tests, simple_clone_test_message)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "clone_core4";
    fi.setProperty(helics_property_time_period, 1.0);
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Clone c1("c1", fi);
    c1.setFederateToClone("block1");

    helics::MessageFederate mfed("block1", fi);
    auto& ept = mfed.registerGlobalEndpoint("ept1", "etype");
    auto& ept2 = mfed.registerGlobalEndpoint("ept3");
    mfed.registerEndpoint("e3");

    auto fut = std::async(std::launch::async, [&c1]() { c1.runTo(4); });
    mfed.enterExecutingMode();
    auto retTime = mfed.requestTime(1);
    EXPECT_EQ(retTime, 1.0);
    ept.send("ept3", "message");

    retTime = mfed.requestTime(2.0);
    EXPECT_EQ(retTime, 2.0);
    ept2.send("ept1", "reply");

    retTime = mfed.requestTime(5);
    EXPECT_EQ(retTime, 3.0);

    mfed.finalize();
    fut.get();
    c1.finalize();
    auto cnt = c1.messageCount();
    EXPECT_EQ(cnt, 2u);
    c1.saveFile("eptsave.json");
    // now test the files
    auto fi2 = helics::loadFederateInfo("eptsave.json");
    fi2.coreName = "clone_core5";
    fi2.coreInitString = "--autobroker";
    fi2.coreType = helics::core_type::TEST;
    helics::apps::Player p1("p1", fi2);
    p1.loadFile("eptsave.json");

    p1.initialize();

    EXPECT_EQ(p1.messageCount(), 2u);
    EXPECT_EQ(p1.endpointCount(), 3u);
    p1.finalize();
    ghc::filesystem::remove("eptsave.json");
}

TEST(clone_tests, simple_clone_test_combo)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "clone_core6";
    fi.setProperty(helics_property_time_period, 1.0);
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Clone c1("c1", fi);
    c1.setFederateToClone("block1");

    helics::CombinationFederate mfed("block1", fi);
    auto& ept = mfed.registerGlobalEndpoint("ept1", "etype");
    auto& ept2 = mfed.registerGlobalEndpoint("ept3");
    mfed.registerEndpoint("e3");

    helics::Publication pub1(helics::GLOBAL, &mfed, "pub1", helics::data_type::helics_double);

    auto& pub2 = mfed.registerPublication("pub2", "double", "m");

    auto fut = std::async(std::launch::async, [&c1]() { c1.runTo(4); });
    mfed.enterExecutingMode();
    auto retTime = mfed.requestTime(1);
    EXPECT_EQ(retTime, 1.0);
    ept.send("ept3", "message");
    pub1.publish(3.4);
    retTime = mfed.requestTime(2.0);
    EXPECT_EQ(retTime, 2.0);
    ept2.send("ept1", "reply");
    pub1.publish(4.7);
    pub2.publish(3.3);
    retTime = mfed.requestTime(5);
    EXPECT_EQ(retTime, 3.0);

    mfed.finalize();
    fut.get();
    c1.finalize();
    auto cnt = c1.messageCount();
    EXPECT_EQ(cnt, 2u);

    cnt = c1.pointCount();
    EXPECT_EQ(cnt, 3u);

    c1.saveFile("combsave.json");

    auto fi2 = helics::loadFederateInfo("combsave.json");
    fi2.coreName = "clone_core7";
    fi2.coreInitString = "--autobroker";
    fi2.coreType = helics::core_type::TEST;
    helics::apps::Player p1("p1", fi2);
    p1.loadFile("combsave.json");

    p1.initialize();

    EXPECT_EQ(p1.messageCount(), 2u);
    EXPECT_EQ(p1.endpointCount(), 3u);
    EXPECT_EQ(p1.pointCount(), 3u);
    EXPECT_EQ(p1.publicationCount(), 2u);
    p1.finalize();
    ghc::filesystem::remove("combsave.json");
}

TEST(clone_tests, simple_clone_test_sub)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "clone_core8";
    fi.coreInitString = "-f 3 --autobroker";
    helics::apps::Clone c1("c1", fi);

    c1.setFederateToClone("block1");

    helics::ValueFederate vfed("block1", fi);
    helics::ValueFederate vfed2("block2", fi);
    helics::Publication pub1(helics::GLOBAL, &vfed, "pub1", helics::data_type::helics_double);

    auto& pub2 = vfed.registerPublication("pub2", "double", "m");

    vfed2.registerPublication<double>("pub");

    vfed2.registerPublication<std::string>("pub2");

    vfed.registerSubscription("block2/pub2");

    vfed.registerSubscription("block2/pub");

    auto fut = std::async(std::launch::async, [&c1]() { c1.runTo(4); });
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

    auto cnt = c1.pointCount();
    EXPECT_EQ(cnt, 3u);
    auto icnt = c1.accessUnderlyingFederate().getInputCount();
    EXPECT_EQ(icnt, 2);
    c1.finalize();
    c1.saveFile("subtest.json");

    ASSERT_TRUE(ghc::filesystem::exists("subtest.json"));

    // now test the file loading
    auto fi2 = helics::loadFederateInfo("subtest.json");
    fi2.coreName = "clone_core9";
    fi2.coreInitString = "--autobroker";
    fi2.coreType = helics::core_type::TEST;
    helics::apps::Player p1("p1", fi2);
    p1.loadFile("subtest.json");

    p1.initialize();

    EXPECT_EQ(p1.pointCount(), 3u);
    EXPECT_EQ(p1.publicationCount(), 2u);
    EXPECT_EQ(p1.accessUnderlyingFederate().getInputCount(), 2);
    p1.finalize();
    ghc::filesystem::remove("subtest.json");
}

TEST(clone_tests, clone_test_help)
{
    std::vector<std::string> args{"--quiet", "--version"};
    helics::apps::Clone c1(args);

    EXPECT_TRUE(!c1.isActive());

    args.emplace_back("-?");
    helics::apps::Clone c2(args);

    EXPECT_TRUE(!c2.isActive());
}
