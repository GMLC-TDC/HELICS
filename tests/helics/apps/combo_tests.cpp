/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "exeTestHelper.h"
#include "helics/application_api/CombinationFederate.hpp"
#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/apps/Player.hpp"
#include "helics/apps/Recorder.hpp"
#include "helics/core/BrokerFactory.hpp"

#include "gtest/gtest.h"
#include <cstdio>
#include <filesystem>
#include <future>
#include <iostream>
#include <string>

static void generateFiles(const std::filesystem::path& f1, const std::filesystem::path& f2)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "ccore2";
    fedInfo.coreInitString = "-f 3 --autobroker";
    helics::apps::Recorder rec1("rec1", fedInfo);
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);

    helics::CombinationFederate mfed("block1", fedInfo);

    helics::MessageFederate mfed2("block2", fedInfo);
    helics::Endpoint e1(helics::InterfaceVisibility::GLOBAL, &mfed, "d1");
    helics::Endpoint e2(helics::InterfaceVisibility::GLOBAL, &mfed2, "d2");

    rec1.addDestEndpointClone("d1");
    rec1.addSourceEndpointClone("d1");
    rec1.addSubscription("pub1");

    helics::Publication pub1(helics::InterfaceVisibility::GLOBAL,
                             &mfed,
                             "pub1",
                             helics::DataType::HELICS_DOUBLE);

    auto fut = std::async(std::launch::async, [&rec1]() { rec1.runTo(5.0); });
    mfed2.enterExecutingModeAsync();
    mfed.enterExecutingMode();
    mfed2.enterExecutingModeComplete();
    pub1.publish(3.4);

    mfed2.requestTimeAsync(1.0);
    auto retTime = mfed.requestTime(1.0);
    mfed2.requestTimeComplete();

    e1.sendTo("this is a test message", "d2");
    pub1.publish(4.7);
    EXPECT_EQ(retTime, 1.0);

    e2.sendTo("this is a test message2", "d1");

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

    EXPECT_TRUE(std::filesystem::exists(f1));

    rec1.saveFile(f2.string());

    EXPECT_TRUE(std::filesystem::exists(f2));
}

static void useFile(const std::string& corename, const std::string& file)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = corename;
    fedInfo.coreInitString = "-f 1 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);

    helics::apps::Player play1("play1", fedInfo);
    play1.loadFile(file);

    play1.initialize();
    EXPECT_EQ(play1.pointCount(), 3U);
    EXPECT_EQ(play1.publicationCount(), 1U);
    EXPECT_EQ(play1.messageCount(), 2U);
    EXPECT_EQ(play1.endpointCount(), 2U);

    play1.finalize();
    std::filesystem::remove(file);
}

static constexpr std::string_view Message1("this is a test message\n and a \"secondMessage\"");
static const std::string Message2(55, 17);

static void generateFiles2(const std::filesystem::path& f1, const std::filesystem::path& f2)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "ccore2b";
    fedInfo.coreInitString = "-f 3 --autobroker";
    helics::apps::Recorder rec1("rec1", fedInfo);
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);

    helics::CombinationFederate mfed("block1", fedInfo);

    helics::MessageFederate mfed2("block2", fedInfo);
    helics::Endpoint e1(helics::InterfaceVisibility::GLOBAL, &mfed, "d1");
    helics::Endpoint e2(helics::InterfaceVisibility::GLOBAL, &mfed2, "d2");

    rec1.addDestEndpointClone("d1");
    rec1.addSourceEndpointClone("d1");
    rec1.addSubscription("pub1");

    helics::Publication pub1(helics::InterfaceVisibility::GLOBAL,
                             &mfed,
                             "pub1",
                             helics::DataType::HELICS_DOUBLE);

    auto fut = std::async(std::launch::async, [&rec1]() { rec1.runTo(5.0); });
    mfed2.enterExecutingModeAsync();
    mfed.enterExecutingMode();
    mfed2.enterExecutingModeComplete();
    pub1.publish(3.4);

    mfed2.requestTimeAsync(1.0);
    auto retTime = mfed.requestTime(1.0);
    mfed2.requestTimeComplete();

    e1.sendTo(Message1, "d2");
    pub1.publish(4.7);
    EXPECT_EQ(retTime, 1.0);

    e2.sendTo(Message2, "d1");

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

    EXPECT_TRUE(std::filesystem::exists(f1));

    rec1.saveFile(f2.string());

    EXPECT_TRUE(std::filesystem::exists(f2));
}

static void useFile2(const std::string& corename, const std::string& file)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = corename;
    fedInfo.coreInitString = "-f 1 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);

    helics::apps::Player play1("play1", fedInfo);
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
    std::filesystem::remove(file);
}

// this is the same as another test in test recorders
TEST(combo, save_load_file1)
{
    auto filename1 = std::filesystem::temp_directory_path() / "savefile.txt";

    auto filename2 = std::filesystem::temp_directory_path() / "savefile.json";

    generateFiles(filename1, filename2);
    ASSERT_TRUE(std::filesystem::exists(filename1));
    ASSERT_TRUE(std::filesystem::exists(filename2));

    useFile("ccore4", filename1.string());
    useFile("ccore5", filename2.string());
}

// this is the same as another test in test recorders
TEST(combo, save_load_file2)
{
    auto filename1 = std::filesystem::temp_directory_path() / "savefile2.txt";

    auto filename2 = std::filesystem::temp_directory_path() / "savefile2.json";

    generateFiles2(filename1, filename2);
    ASSERT_TRUE(std::filesystem::exists(filename1));
    ASSERT_TRUE(std::filesystem::exists(filename2));

    useFile2("ccore4b", filename1.string());
    useFile2("ccore5b", filename2.string());
}

static void generateFiles_binary(const std::filesystem::path& f1, const std::filesystem::path& f2)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "ccore3";
    fedInfo.coreInitString = "-f 3 --autobroker";
    helics::apps::Recorder rec1("rec1", fedInfo);
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);

    helics::CombinationFederate mfed("block1", fedInfo);

    helics::MessageFederate mfed2("block2", fedInfo);
    helics::Endpoint e1(helics::InterfaceVisibility::GLOBAL, &mfed, "d1");
    helics::Endpoint e2(helics::InterfaceVisibility::GLOBAL, &mfed2, "d2");

    rec1.addDestEndpointClone("d1");
    rec1.addSourceEndpointClone("d1");
    rec1.addSubscription("pub1");

    helics::Publication pub1(helics::InterfaceVisibility::GLOBAL,
                             &mfed,
                             "pub1",
                             helics::DataType::HELICS_DOUBLE);

    auto fut = std::async(std::launch::async, [&rec1]() { rec1.runTo(5.0); });
    mfed2.enterExecutingModeAsync();
    mfed.enterExecutingMode();
    mfed2.enterExecutingModeComplete();
    pub1.publish(3.4);

    mfed2.requestTimeAsync(1.0);
    auto retTime = mfed.requestTime(1.0);
    mfed2.requestTimeComplete();
    helics::SmallBuffer n5(256);
    for (int ii = 0; ii < 256; ++ii) {
        n5[ii] = std::byte(ii);
    }
    e1.sendTo(n5, "d2");
    pub1.publish(4.7);
    EXPECT_EQ(retTime, 1.0);
    helics::SmallBuffer n6(256);
    for (int ii = 0; ii < 256; ++ii) {
        n6[ii] = std::byte(255 - ii);
    }
    e2.sendTo(n6, "d1");

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
    if (rec1.pointCount() < 3U) {
        for (size_t ii = 0; ii < rec1.pointCount(); ++ii) {
            std::cout << "point " << ii << " time=" << std::get<0>(rec1.getValue(ii))
                      << " value= " << std::get<2>(rec1.getValue(ii)) << std::endl;
        }
    }
    rec1.saveFile(f1.string());

    EXPECT_TRUE(std::filesystem::exists(f1));

    rec1.saveFile(f2.string());

    EXPECT_TRUE(std::filesystem::exists(f2));
}

static void useFileBinary(const std::string& corename, const std::string& file)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreType = helics::CoreType::TEST;
    fedInfo.coreName = corename;
    fedInfo.coreInitString = "-f 1 --autobroker";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);

    helics::apps::Player play1("play1", fedInfo);
    play1.loadFile(file);

    play1.initialize();
    EXPECT_EQ(play1.pointCount(), 3U);
    EXPECT_EQ(play1.publicationCount(), 1U);
    EXPECT_EQ(play1.messageCount(), 2U);
    EXPECT_EQ(play1.endpointCount(), 2U);

    auto& b1 = play1.getMessage(0);
    helics::SmallBuffer n5(256);
    for (int ii = 0; ii < 256; ++ii) {
        n5[ii] = std::byte(ii);
    }
    EXPECT_EQ(b1.mess.data.to_string(), n5.to_string());

    auto& b2 = play1.getMessage(1);
    helics::SmallBuffer n6(256);
    for (int ii = 0; ii < 256; ++ii) {
        n6[ii] = std::byte(255 - ii);
    }
    EXPECT_EQ(b2.mess.data.to_string(), n6.to_string());
    play1.finalize();
    std::filesystem::remove(file);
}

TEST(combo, save_load_file_binary)
{
    auto tpath = std::filesystem::temp_directory_path();

    auto filename1 = tpath / "savefile_binary.txt";
    auto filename2 = tpath / "savefile_binary.json";

    generateFiles_binary(filename1, filename2);
    ASSERT_TRUE(std::filesystem::exists(filename1));

    ASSERT_TRUE(std::filesystem::exists(filename2));

    useFileBinary("ccore6", filename1.string());
    useFileBinary("ccore7", filename2.string());
}

TEST(combo, check_combination_file_load)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "ccore_combo";
    fedInfo.coreInitString = "-f 3 --autobroker";

    helics::apps::Player play1("play1", fedInfo);
    play1.loadFile(TEST_DIR "test_HELICS_player.json");

    helics::apps::Recorder rec1("rec1", fedInfo);
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
    EXPECT_EQ(fed.pendingMessageCount(), 3U);
    fed.finalize();
    fut_play.get();
    fut_rec.get();
    EXPECT_EQ(rec1.messageCount(), 2U);
    EXPECT_EQ(rec1.pointCount(), 2U);
}
