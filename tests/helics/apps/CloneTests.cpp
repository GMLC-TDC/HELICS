/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
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

#include "helics/application_api/Filters.hpp"
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
    EXPECT_EQ(cnt, 2U);
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
    EXPECT_EQ(cnt, 3U);
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

    EXPECT_EQ(p1.pointCount(), 3U);
    EXPECT_EQ(p1.publicationCount(), 2U);
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
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    auto fut = std::async(std::launch::async, [&c1]() { c1.runTo(4); });
    mfed.enterExecutingMode();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
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
    EXPECT_EQ(cnt, 2U);
    c1.saveFile("eptsave.json");
    // now test the files
    auto fi2 = helics::loadFederateInfo("eptsave.json");
    fi2.coreName = "clone_core5";
    fi2.coreInitString = "--autobroker";
    fi2.coreType = helics::core_type::TEST;
    helics::apps::Player p1("p1", fi2);
    p1.loadFile("eptsave.json");

    p1.initialize();

    EXPECT_EQ(p1.messageCount(), 2U);
    EXPECT_EQ(p1.endpointCount(), 3U);
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

    auto fut = std::async(std::launch::async, [&c1]() { c1.runTo(6); });
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
    auto epts = c1.accessUnderlyingFederate().getEndpointCount();
    EXPECT_EQ(epts, 1);
    auto filts = c1.accessUnderlyingFederate().getFilterCount();
    EXPECT_EQ(filts, 1);

    auto ipts = c1.accessUnderlyingFederate().getInputCount();
    EXPECT_EQ(ipts, 2);
    c1.finalize();

    auto cnt = c1.messageCount();
    EXPECT_EQ(cnt, 2U);

    auto pcnt = c1.pointCount();
    EXPECT_EQ(pcnt, 3U);
    if (pcnt != 3) {
        for (int ii = 0; ii < static_cast<int>(pcnt); ++ii) {
            auto pv = c1.getValue(ii);
            std::cout << "Point " << ii << " source:" << std::get<1>(pv) << " at "
                      << std::get<0>(pv) << ", value: " << std::get<2>(pv) << std::endl;
        }
        EXPECT_EQ(std::get<0>(c1.getValue(static_cast<int>(pcnt))), helics::Time{});
    }

    c1.saveFile("combsave.json");

    auto fi2 = helics::loadFederateInfo("combsave.json");
    fi2.coreName = "clone_core7";
    fi2.coreInitString = "--autobroker";
    fi2.coreType = helics::core_type::TEST;
    helics::apps::Player p1("p1", fi2);
    p1.loadFile("combsave.json");

    p1.initialize();

    EXPECT_EQ(p1.messageCount(), 2U);
    EXPECT_EQ(p1.endpointCount(), 3U);
    EXPECT_EQ(p1.pointCount(), pcnt);
    EXPECT_EQ(p1.publicationCount(), 2U);
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

    auto fut = std::async(std::launch::async, [&c1]() { c1.runTo(6); });
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
    EXPECT_EQ(cnt, 3U);
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

    EXPECT_EQ(p1.pointCount(), 3U);
    EXPECT_EQ(p1.publicationCount(), 2U);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // this depends on some processing occurring
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
