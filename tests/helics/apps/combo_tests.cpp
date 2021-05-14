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

#include "exeTestHelper.h"
#include "helics/application_api/CombinationFederate.hpp"
#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/apps/Player.hpp"
#include "helics/apps/Recorder.hpp"
#include "helics/core/BrokerFactory.hpp"

#include <cstdio>
#include <future>

static void generateFiles(const ghc::filesystem::path& f1, const ghc::filesystem::path& f2)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "ccore2";
    fi.coreInitString = "-f 3 --autobroker";
    helics::apps::Recorder rec1("rec1", fi);
    fi.setProperty(helics_property_time_period, 1.0);

    helics::CombinationFederate mfed("block1", fi);

    helics::MessageFederate mfed2("block2", fi);
    helics::Endpoint e1(helics::GLOBAL, &mfed, "d1");
    helics::Endpoint e2(helics::GLOBAL, &mfed2, "d2");

    rec1.addDestEndpointClone("d1");
    rec1.addSourceEndpointClone("d1");
    rec1.addSubscription("pub1");

    helics::Publication pub1(helics::GLOBAL, &mfed, "pub1", helics::data_type::helics_double);

    auto fut = std::async(std::launch::async, [&rec1]() { rec1.runTo(5.0); });
    mfed2.enterExecutingModeAsync();
    mfed.enterExecutingMode();
    mfed2.enterExecutingModeComplete();
    pub1.publish(3.4);

    mfed2.requestTimeAsync(1.0);
    auto retTime = mfed.requestTime(1.0);
    mfed2.requestTimeComplete();

    e1.send("d2", "this is a test message");
    pub1.publish(4.7);
    EXPECT_EQ(retTime, 1.0);

    e2.send("d1", "this is a test message2");

    mfed2.requestTimeAsync(2.0);
    retTime = mfed.requestTime(2.0);
    EXPECT_EQ(retTime, 2.0);

    mfed2.requestTimeComplete();
    pub1.publish(4.7);

    mfed2.requestTimeAsync(4.0);
    retTime = mfed.requestTime(4.0);
    EXPECT_EQ(retTime, 4.0);
    mfed2.requestTimeComplete();

    mfed2.requestTimeAsync(6.0);
    retTime = mfed.requestTime(6.0);
    EXPECT_EQ(retTime, 6.0);
    mfed2.requestTimeComplete();

    fut.get();

    mfed.finalize();
    mfed2.finalize();

    EXPECT_EQ(rec1.messageCount(), 2U);
    EXPECT_EQ(rec1.pointCount(), 3U);

    EXPECT_LT(std::get<0>(rec1.getValue(0)), 0.0001);
    EXPECT_GE(std::get<0>(rec1.getValue(0)), 0.0);
    EXPECT_EQ(std::get<0>(rec1.getValue(1)), 1.0);
    EXPECT_EQ(std::get<0>(rec1.getValue(2)), 2.0);
    rec1.saveFile(f1.string());

    EXPECT_TRUE(ghc::filesystem::exists(f1));

    rec1.saveFile(f2.string());

    EXPECT_TRUE(ghc::filesystem::exists(f2));
}

static void useFile(const std::string& corename, const std::string& file)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = corename;
    fi.coreInitString = "-f 1 --autobroker";
    fi.setProperty(helics_property_time_period, 1.0);

    helics::apps::Player play1("play1", fi);
    play1.loadFile(file);

    play1.initialize();
    EXPECT_EQ(play1.pointCount(), 3U);
    EXPECT_EQ(play1.publicationCount(), 1U);
    EXPECT_EQ(play1.messageCount(), 2U);
    EXPECT_EQ(play1.endpointCount(), 2U);

    play1.finalize();
    ghc::filesystem::remove(file);
}

static const std::string Message1("this is a test message\n and a \"secondMessage\"");
static const std::string Message2(55, 17);

static void generateFiles2(const ghc::filesystem::path& f1, const ghc::filesystem::path& f2)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "ccore2b";
    fi.coreInitString = "-f 3 --autobroker";
    helics::apps::Recorder rec1("rec1", fi);
    fi.setProperty(helics_property_time_period, 1.0);

    helics::CombinationFederate mfed("block1", fi);

    helics::MessageFederate mfed2("block2", fi);
    helics::Endpoint e1(helics::GLOBAL, &mfed, "d1");
    helics::Endpoint e2(helics::GLOBAL, &mfed2, "d2");

    rec1.addDestEndpointClone("d1");
    rec1.addSourceEndpointClone("d1");
    rec1.addSubscription("pub1");

    helics::Publication pub1(helics::GLOBAL, &mfed, "pub1", helics::data_type::helics_double);

    auto fut = std::async(std::launch::async, [&rec1]() { rec1.runTo(5.0); });
    mfed2.enterExecutingModeAsync();
    mfed.enterExecutingMode();
    mfed2.enterExecutingModeComplete();
    pub1.publish(3.4);

    mfed2.requestTimeAsync(1.0);
    auto retTime = mfed.requestTime(1.0);
    mfed2.requestTimeComplete();

    e1.send("d2", Message1);
    pub1.publish(4.7);
    EXPECT_EQ(retTime, 1.0);

    e2.send("d1", Message2);

    mfed2.requestTimeAsync(2.0);
    retTime = mfed.requestTime(2.0);
    EXPECT_EQ(retTime, 2.0);

    mfed2.requestTimeComplete();
    pub1.publish(4.7);

    mfed2.requestTimeAsync(4.0);
    retTime = mfed.requestTime(4.0);
    EXPECT_EQ(retTime, 4.0);
    mfed2.requestTimeComplete();

    mfed2.requestTimeAsync(6.0);
    retTime = mfed.requestTime(6.0);
    EXPECT_EQ(retTime, 6.0);
    mfed2.requestTimeComplete();

    fut.get();

    mfed.finalize();
    mfed2.finalize();

    EXPECT_EQ(rec1.messageCount(), 2U);
    EXPECT_EQ(rec1.pointCount(), 3U);

    EXPECT_LT(std::get<0>(rec1.getValue(0)), 0.0001);
    EXPECT_GE(std::get<0>(rec1.getValue(0)), 0.0);
    EXPECT_EQ(std::get<0>(rec1.getValue(1)), 1.0);
    EXPECT_EQ(std::get<0>(rec1.getValue(2)), 2.0);
    rec1.saveFile(f1.string());

    EXPECT_TRUE(ghc::filesystem::exists(f1));

    rec1.saveFile(f2.string());

    EXPECT_TRUE(ghc::filesystem::exists(f2));
}

static void useFile2(const std::string& corename, const std::string& file)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = corename;
    fi.coreInitString = "-f 1 --autobroker";
    fi.setProperty(helics_property_time_period, 1.0);

    helics::apps::Player play1("play1", fi);
    play1.loadFile(file);

    play1.initialize();
    EXPECT_EQ(play1.pointCount(), 3U);
    EXPECT_EQ(play1.publicationCount(), 1U);
    EXPECT_EQ(play1.messageCount(), 2U);
    EXPECT_EQ(play1.endpointCount(), 2U);
    if (play1.messageCount() == 2U) {
        EXPECT_EQ(play1.getMessage(0).mess.data.to_string(), Message1);
        EXPECT_EQ(play1.getMessage(1).mess.data.to_string(), Message2);
    }

    play1.finalize();
    ghc::filesystem::remove(file);
}

// this is the same as another test in test recorders
TEST(combo_tests, save_load_file1)
{
    auto filename1 = ghc::filesystem::temp_directory_path() / "savefile.txt";

    auto filename2 = ghc::filesystem::temp_directory_path() / "savefile.json";

    generateFiles(filename1, filename2);
    ASSERT_TRUE(ghc::filesystem::exists(filename1));
    ASSERT_TRUE(ghc::filesystem::exists(filename2));

    useFile("ccore4", filename1.string());
    useFile("ccore5", filename2.string());
}

// this is the same as another test in test recorders
TEST(combo_tests, save_load_file2)
{
    auto filename1 = ghc::filesystem::temp_directory_path() / "savefile2.txt";

    auto filename2 = ghc::filesystem::temp_directory_path() / "savefile2.json";

    generateFiles2(filename1, filename2);
    ASSERT_TRUE(ghc::filesystem::exists(filename1));
    ASSERT_TRUE(ghc::filesystem::exists(filename2));

    useFile2("ccore4b", filename1.string());
    useFile2("ccore5b", filename2.string());
}

static void generateFiles_binary(const ghc::filesystem::path& f1, const ghc::filesystem::path& f2)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "ccore3";
    fi.coreInitString = "-f 3 --autobroker";
    helics::apps::Recorder rec1("rec1", fi);
    fi.setProperty(helics_property_time_period, 1.0);

    helics::CombinationFederate mfed("block1", fi);

    helics::MessageFederate mfed2("block2", fi);
    helics::Endpoint e1(helics::GLOBAL, &mfed, "d1");
    helics::Endpoint e2(helics::GLOBAL, &mfed2, "d2");

    rec1.addDestEndpointClone("d1");
    rec1.addSourceEndpointClone("d1");
    rec1.addSubscription("pub1");

    helics::Publication pub1(helics::GLOBAL, &mfed, "pub1", helics::data_type::helics_double);

    auto fut = std::async(std::launch::async, [&rec1]() { rec1.runTo(5.0); });
    mfed2.enterExecutingModeAsync();
    mfed.enterExecutingMode();
    mfed2.enterExecutingModeComplete();
    pub1.publish(3.4);

    mfed2.requestTimeAsync(1.0);
    auto retTime = mfed.requestTime(1.0);
    mfed2.requestTimeComplete();
    helics::data_block n5(256);
    for (int ii = 0; ii < 256; ++ii) {
        n5[ii] = ii;
    }
    e1.send("d2", n5);
    pub1.publish(4.7);
    EXPECT_EQ(retTime, 1.0);
    helics::data_block n6(256);
    for (int ii = 0; ii < 256; ++ii) {
        n6[ii] = 255 - ii;
    }
    e2.send("d1", n6);

    mfed2.requestTimeAsync(2.0);
    retTime = mfed.requestTime(2.0);
    EXPECT_EQ(retTime, 2.0);

    mfed2.requestTimeComplete();
    pub1.publish(4.7);

    mfed2.requestTimeAsync(3.0);
    retTime = mfed.requestTime(3.0);
    EXPECT_EQ(retTime, 3.0);

    mfed2.requestTimeComplete();

    mfed.finalize();
    mfed2.finalize();
    fut.get();
    EXPECT_EQ(rec1.messageCount(), 2U);
    EXPECT_EQ(rec1.pointCount(), 3U);

    rec1.saveFile(f1.string());

    EXPECT_TRUE(ghc::filesystem::exists(f1));

    rec1.saveFile(f2.string());

    EXPECT_TRUE(ghc::filesystem::exists(f2));
}

static void useFileBinary(const std::string& corename, const std::string& file)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreType = helics::core_type::TEST;
    fi.coreName = corename;
    fi.coreInitString = "-f 1 --autobroker";
    fi.setProperty(helics_property_time_period, 1.0);

    helics::apps::Player play1("play1", fi);
    play1.loadFile(file);

    play1.initialize();
    EXPECT_EQ(play1.pointCount(), 3U);
    EXPECT_EQ(play1.publicationCount(), 1U);
    EXPECT_EQ(play1.messageCount(), 2U);
    EXPECT_EQ(play1.endpointCount(), 2U);

    auto& b1 = play1.getMessage(0);
    helics::data_block n5(256);
    for (int ii = 0; ii < 256; ++ii) {
        n5[ii] = ii;
    }
    EXPECT_EQ(b1.mess.data.to_string(), n5.to_string());

    auto& b2 = play1.getMessage(1);
    helics::data_block n6(256);
    for (int ii = 0; ii < 256; ++ii) {
        n6[ii] = 255 - ii;
    }
    EXPECT_EQ(b2.mess.data.to_string(), n6.to_string());
    play1.finalize();
    ghc::filesystem::remove(file);
}

TEST(combo_tests, save_load_file_binary)
{
    auto tpath = ghc::filesystem::temp_directory_path();

    auto filename1 = ghc::filesystem::temp_directory_path() / "savefile_binary.txt";
    auto filename2 = ghc::filesystem::temp_directory_path() / "savefile_binary.json";

    generateFiles_binary(filename1, filename2);
    ASSERT_TRUE(ghc::filesystem::exists(filename1));

    ASSERT_TRUE(ghc::filesystem::exists(filename2));

    useFileBinary("ccore6", filename1.string());
    useFileBinary("ccore7", filename2.string());
}

TEST(combo_tests, check_combination_file_load)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "ccore_combo";
    fi.coreInitString = "-f 3 --autobroker";

    helics::apps::Player play1("play1", fi);
    play1.loadFile(TEST_DIR "test_HELICS_player.json");

    helics::apps::Recorder rec1("rec1", fi);
    rec1.loadFile(TEST_DIR "test_HELICS_recorder.json");

    helics::CombinationFederate fed("", TEST_DIR "federate_config.json");
    auto fut_rec = std::async(std::launch::async, [&rec1]() { rec1.run(); });
    auto fut_play = std::async(std::launch::async, [&play1]() { play1.run(); });
    fed.enterExecutingMode();
    auto tm = fed.getCurrentTime();
    while (tm < 1300.0) {
        tm = fed.requestNextStep();
        if (tm == 120.0) {
            fed.getEndpoint(0).send("73.4");
        }
        if (tm == 730.0) {
            fed.getEndpoint(1).send("on");
        }
        if (tm == 230.0) {
            fed.getPublication(0).publish(34.6);
        }
        if (tm == 640.0) {
            fed.getPublication(1).publish(1);
        }
    }
    EXPECT_EQ(fed.pendingMessages(), 3U);
    fed.finalize();
    fut_play.get();
    fut_rec.get();
    EXPECT_EQ(rec1.messageCount(), 2U);
    EXPECT_EQ(rec1.pointCount(), 2U);
}
