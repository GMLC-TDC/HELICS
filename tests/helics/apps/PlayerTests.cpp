/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "gtest/gtest.h"
#include <cstdio>
#ifndef DISABLE_SYSTEM_CALL_TESTS
#    include "exeTestHelper.h"
#endif
#include "helics/application_api/Subscriptions.hpp"
#include "helics/apps/BrokerApp.hpp"
#include "helics/apps/Player.hpp"

#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

TEST(player_tests, simple_player)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);

    fedInfo.coreName = "pcore1";
    fedInfo.coreInitString = "-f2 --autobroker";
    helics::apps::Player play1("player1", fedInfo);

    play1.addPublication("pub1", helics::DataType::HELICS_DOUBLE);
    play1.addPoint(1.0, "pub1", 0.5);
    play1.addPoint(2.0, "pub1", 0.7);
    play1.addPoint(3.0, "pub1", 0.8);

    helics::ValueFederate vfed("block1", fedInfo);
    auto& sub1 = vfed.registerSubscription("pub1");
    auto fut = std::async(std::launch::async, [&play1]() { play1.run(); });
    vfed.enterExecutingMode();
    auto retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    auto val = sub1.getValue<double>();
    EXPECT_EQ(val, 0.5);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 2.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 0.7);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 3.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 0.8);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 5.0);
    vfed.finalize();
    fut.get();
}

TEST(player_tests, simple_player_diff_inputs)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "pcore2";
    fedInfo.coreInitString = "-f 2 --autobroker";
    helics::apps::Player play1("player1", fedInfo);

    play1.addPublication("pub1", helics::DataType::HELICS_DOUBLE);
    play1.addPoint(1.0, "pub1", "v[3.0,4.0]");
    play1.addPoint(2.0, "pub1", "0.7");
    play1.addPoint(3.0, "pub1", std::complex<double>(0.0, 0.8));
    play1.addPoint(4.0, "pub1", "c[3.0+0j, 0.0-4.0j]");
    helics::ValueFederate vfed("block1", fedInfo);
    auto& sub1 = vfed.registerSubscription("pub1");
    auto fut = std::async(std::launch::async, [&play1]() { play1.run(); });
    vfed.enterExecutingMode();
    auto retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    auto val = sub1.getValue<double>();
    EXPECT_EQ(val, 5.0);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 2.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 0.7);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 3.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 0.8);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 4.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 5.0);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 5.0);
    vfed.finalize();
    fut.get();
}

TEST(player_tests, simple_player_iterative)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "pcore3";
    fedInfo.coreInitString = "-f 2 --autobroker";
    helics::apps::Player play1("player1", fedInfo);

    play1.addPublication("pub1", helics::DataType::HELICS_DOUBLE);
    play1.addPoint(1.0, 0, "pub1", 0.5);
    play1.addPoint(1.0, 1, "pub1", 0.7);
    play1.addPoint(1.0, 2, "pub1", 0.8);

    helics::ValueFederate vfed("block1", fedInfo);
    auto& sub1 = vfed.registerSubscription("pub1");
    auto fut = std::async(std::launch::async, [&play1]() { play1.run(); });
    vfed.enterExecutingMode();
    auto retTime = vfed.requestTimeIterative(5, helics::IterationRequest::ITERATE_IF_NEEDED);
    EXPECT_EQ(retTime.grantedTime, 1.0);
    EXPECT_TRUE(retTime.state == helics::IterationResult::NEXT_STEP);
    auto val = sub1.getValue<double>();
    EXPECT_EQ(val, 0.5);

    retTime = vfed.requestTimeIterative(5, helics::IterationRequest::ITERATE_IF_NEEDED);
    EXPECT_EQ(retTime.grantedTime, 1.0);
    EXPECT_TRUE(retTime.state == helics::IterationResult::ITERATING);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 0.7);

    retTime = vfed.requestTimeIterative(5, helics::IterationRequest::ITERATE_IF_NEEDED);
    EXPECT_EQ(retTime.grantedTime, 1.0);
    EXPECT_TRUE(retTime.state == helics::IterationResult::ITERATING);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 0.8);

    retTime = vfed.requestTimeIterative(5, helics::IterationRequest::ITERATE_IF_NEEDED);
    EXPECT_EQ(retTime.grantedTime, 5.0);
    EXPECT_TRUE(retTime.state == helics::IterationResult::NEXT_STEP);
    vfed.finalize();
    fut.get();
}

TEST(player_tests, simple_player2)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "pcore4";
    fedInfo.coreInitString = "-f 2 --autobroker";
    helics::apps::Player play1("player1", fedInfo);

    play1.addPublication<double>("pub1");
    play1.addPoint(1.0, "pub1", 0.5);
    play1.addPoint(2.0, "pub1", 0.7);
    play1.addPoint(3.0, "pub1", 0.8);

    play1.addPublication<double>("pub2");
    play1.addPoint(1.0, "pub2", 0.4);
    play1.addPoint(2.0, "pub2", 0.6);
    play1.addPoint(3.0, "pub2", 0.9);

    helics::ValueFederate vfed("block1", fedInfo);
    auto& sub1 = vfed.registerSubscription("pub1");
    auto& sub2 = vfed.registerSubscription("pub2");
    auto fut = std::async(std::launch::async, [&play1]() { play1.run(); });
    vfed.enterExecutingMode();

    auto retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    auto val = sub1.getValue<double>();
    EXPECT_EQ(val, 0.5);
    val = sub2.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 0.4);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 2.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 0.7);
    val = sub2.getValue<double>();
    EXPECT_EQ(val, 0.6);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 3.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 0.8);
    val = sub2.getValue<double>();
    EXPECT_EQ(val, 0.9);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 5.0);
    vfed.finalize();
    fut.get();
}

static constexpr const char* simple_files[] = {"example1.player",
                                               "example2.player",
                                               "example3.player",
                                               "example4.player",
                                               "example5.json",
                                               "example5.player"};

class player_file_tests: public ::testing::TestWithParam<const char*> {};

TEST_P(player_file_tests, files)
{
    static char indx = 'a';
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = std::string("pcore5") + GetParam();
    fedInfo.coreName.push_back(indx++);
    fedInfo.coreInitString = "-f 2 --autobroker";
    helics::apps::Player play1("player1", fedInfo);

    play1.loadFile(std::string(TEST_DIR) + GetParam());

    helics::ValueFederate vfed("block1", fedInfo);
    auto& sub1 = vfed.registerSubscription("pub1");
    auto& sub2 = vfed.registerSubscription("pub2");
    auto fut = std::async(std::launch::async, [&play1]() { play1.run(); });
    vfed.enterExecutingMode();
    auto val = sub1.getValue<double>();
    EXPECT_EQ(val, 0.3);

    auto retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 0.5);
    val = sub2.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 0.4);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 2.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 0.7);
    val = sub2.getValue<double>();
    EXPECT_EQ(val, 0.6);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 3.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 0.8);
    val = sub2.getValue<double>();
    EXPECT_EQ(val, 0.9);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 5.0);
    vfed.finalize();
    fut.get();
}

TEST(player_tests, bigfile)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "pcorebig";
    fedInfo.coreInitString = "-f 2 --autobroker";
    helics::apps::Player play1("player1", fedInfo);

    play1.loadFile(std::string(TEST_DIR) + "bigtest.txt");
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 60.0);
    helics::ValueFederate vfed("charger", fedInfo);
    auto& sub1 = vfed.registerSubscription("Battery/EV1_current");
    auto& sub2 = vfed.registerSubscription("Battery/EV2_current");
    auto& sub3 = vfed.registerSubscription("Battery/EV3_current");
    auto& sub4 = vfed.registerSubscription("Battery/EV4_current");
    auto& sub5 = vfed.registerSubscription("Battery/EV5_current");

    sub1.setDefault(1.0);
    sub2.setDefault(1.0);
    sub3.setDefault(1.0);
    sub4.setDefault(1.0);
    sub5.setDefault(1.0);

    auto fut = std::async(std::launch::async, [&play1]() { play1.runTo(500.0); });
    vfed.enterExecutingMode();
    // helics::Time maxTime = 60.0 * 60.0 * 24.0 * 7.0;
    helics::Time period = 60.0;
    auto retTime = vfed.requestTime(60.0);

    while (retTime <= 200.0) {
        double val1{0.0};
        double val2{0.0};
        double val3{0.0};
        double val4{0.0};
        double val5{0.0};
        EXPECT_NO_THROW(val1 = sub1.getValue<double>());
        EXPECT_NO_THROW(val2 = sub2.getValue<double>());
        EXPECT_NO_THROW(val3 = sub3.getValue<double>());
        EXPECT_NO_THROW(val4 = sub4.getValue<double>());
        EXPECT_NO_THROW(val5 = sub5.getValue<double>());
        EXPECT_GE(val1, 0.0001);
        EXPECT_LT(val1, 1000.0);
        EXPECT_GE(val2, 0.0001);
        EXPECT_LT(val2, 1000.0);
        EXPECT_GE(val3, 0.0001);
        EXPECT_LT(val3, 1000.0);
        EXPECT_GE(val4, 0.0001);
        EXPECT_LT(val4, 1000.0);
        EXPECT_GE(val5, 0.0001);
        EXPECT_LT(val5, 1000.0);
        retTime = vfed.requestTimeAdvance(period);
    }

    vfed.finalize();
    fut.get();
}

TEST(player_tests, testfile)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "pcoretest";
    fedInfo.coreInitString = "-f 2 --autobroker";
    helics::apps::Player play1("player1", fedInfo);

    play1.loadFile(std::string(TEST_DIR) + "oorder.txt");
    helics::ValueFederate vfed("receiver", fedInfo);
    vfed.registerSubscription("player1/G");
    vfed.registerSubscription("player1/T");
    vfed.registerSubscription("player1/Fc");
    vfed.registerSubscription("player1/Ud");
    vfed.registerSubscription("player1/Rg");
    vfed.registerSubscription("player1/ctl");

    std::vector<std::vector<double>> results;
    results.resize(6);

    auto fut = std::async(std::launch::async, [&play1]() { play1.runTo(8.0); });
    vfed.enterExecutingMode();
    // helics::Time maxTime = 60.0 * 60.0 * 24.0 * 7.0;
    auto retTime = vfed.requestTime(8.0);

    while (retTime <= 7.0) {
        for (int ii = 0; ii < 6; ++ii) {
            auto& inp = vfed.getInput(ii);
            if (inp.isUpdated()) {
                results[ii].push_back(inp.getDouble());
            }
        }
        retTime = vfed.requestTime(7.0);
    }

    vfed.finalize();
    fut.get();
    EXPECT_GE(results[0].size(), 120U);
    EXPECT_EQ(results[1].size(), 2U);
    EXPECT_EQ(results[2].size(), 2U);
    EXPECT_EQ(results[3].size(), 2U);
    EXPECT_EQ(results[4].size(), 2U);
    EXPECT_EQ(results[5].size(), 2U);
    EXPECT_EQ(results[2][1], 63.0);
}

TEST(player_tests, simple_player_mlinecomment)
{
    static char indx = 'a';
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "pcore6-mline";
    fedInfo.coreName.push_back(indx++);
    fedInfo.coreInitString = " -f 2 --autobroker";
    helics::apps::Player play1("player1", fedInfo);
    play1.loadFile(std::string(TEST_DIR) + "/example_comments.player");

    EXPECT_EQ(play1.pointCount(), 7U);
    helics::ValueFederate vfed("block1", fedInfo);
    auto& sub1 = vfed.registerSubscription("pub1");
    auto& sub2 = vfed.registerSubscription("pub2");
    auto fut = std::async(std::launch::async, [&play1]() { play1.run(); });
    vfed.enterExecutingMode();
    auto val = sub1.getValue<double>();
    EXPECT_EQ(val, 0.3);

    auto retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 0.5);
    val = sub2.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 0.4);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 2.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 0.7);
    val = sub2.getValue<double>();
    EXPECT_EQ(val, 0.6);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 3.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 0.8);
    val = sub2.getValue<double>();
    EXPECT_EQ(val, 0.9);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 5.0);
    vfed.finalize();
    fut.get();
    EXPECT_EQ(play1.publicationCount(), 2U);
}

#ifdef HELICS_ENABLE_IPC_CORE
TEST_P(player_file_tests, test_files_cmd)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    helics::apps::BrokerApp brk(helics::CoreType::IPC, "ipc_broker", "-f 2");

    std::string exampleFile = std::string(TEST_DIR) + GetParam();

    std::vector<std::string> args{
        "", "--name=player", "--broker=ipc_broker", "--coretype=ipc", "--input=" + exampleFile};
    char* argv[5];
    argv[0] = &(args[0][0]);
    argv[1] = &(args[1][0]);
    argv[2] = &(args[2][0]);
    argv[3] = &(args[3][0]);
    argv[4] = &(args[4][0]);

    helics::apps::Player play1(5, argv);

    helics::FederateInfo fedInfo(helics::CoreType::IPC);
    fedInfo.coreInitString = "--broker=ipc_broker";

    helics::ValueFederate vfed("obj", fedInfo);
    auto& sub1 = vfed.registerSubscription("pub1");
    auto& sub2 = vfed.registerSubscription("pub2");
    auto fut = std::async(std::launch::async, [&play1]() { play1.run(); });
    vfed.enterExecutingMode();
    auto val = sub1.getValue<double>();
    EXPECT_EQ(val, 0.3);

    auto retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 0.5);
    val = sub2.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 0.4);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 2.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 0.7);
    val = sub2.getValue<double>();
    EXPECT_EQ(val, 0.6);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 3.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 0.8);
    val = sub2.getValue<double>();
    EXPECT_EQ(val, 0.9);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 5.0);
    vfed.finalize();
    fut.get();
    brk.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
}
#endif

#ifdef HELICS_ENABLE_ZMQ_CORE
#    ifndef DISABLE_SYSTEM_CALL_TESTS
TEST_P(player_file_tests, test_files_exe)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    exeTestRunner playerExe(HELICS_INSTALL_LOC, HELICS_BUILD_LOC, "helics_player");

    exeTestRunner brokerExe(HELICS_INSTALL_LOC, HELICS_BUILD_LOC, "helics_broker");

    ASSERT_TRUE(playerExe.isActive());
    if (!brokerExe.isActive()) {
        std::cout << " unable to locate helics_broker in " << HELICS_INSTALL_LOC << " or "
                  << HELICS_BUILD_LOC << std::endl;
    }
    ASSERT_TRUE(brokerExe.isActive());
    auto res = brokerExe.runAsync("-f 2 --coretype=zmq --name=zmq_broker");
    std::string exampleFile = std::string(TEST_DIR) + GetParam();
    auto res2 =
        playerExe.runCaptureOutputAsync("--name=player --coretype=zmq --input=" + exampleFile);

    helics::FederateInfo fedInfo(helics::CoreType::ZMQ);
    fedInfo.coreInitString = "";

    helics::ValueFederate vfed("fed", fedInfo);
    auto& sub1 = vfed.registerSubscription("pub1");
    auto& sub2 = vfed.registerSubscription("pub2");
    vfed.enterExecutingMode();
    auto val = sub1.getValue<double>();
    EXPECT_EQ(val, 0.3);

    auto retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 0.5);
    val = sub2.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 0.4);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 2.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 0.7);
    val = sub2.getValue<double>();
    EXPECT_EQ(val, 0.6);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 3.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 0.8);
    val = sub2.getValue<double>();
    EXPECT_EQ(val, 0.9);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 5.0);
    vfed.finalize();
    res2.get();
    res.get();
    // out = 0;
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
}
#    endif
#endif

INSTANTIATE_TEST_SUITE_P(player_tests, player_file_tests, ::testing::ValuesIn(simple_files));

TEST(player_tests, simple_player_testjson)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "pcore7";
    fedInfo.coreInitString = "-f 2 --autobroker";
    helics::apps::Player play1("player1", fedInfo);
    play1.loadFile(std::string(TEST_DIR) + "/example6.json");

    helics::ValueFederate vfed("block1", fedInfo);
    auto& sub1 = vfed.registerSubscription("pub1");
    auto& sub2 = vfed.registerSubscription("pub2");
    auto fut = std::async(std::launch::async, [&play1]() { play1.run(); });
    vfed.enterExecutingMode();

    auto retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    auto val = sub1.getValue<double>();
    EXPECT_EQ(val, 0.5);
    val = sub2.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 0.4);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 2.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 0.7);
    val = sub2.getValue<double>();
    EXPECT_EQ(val, 0.6);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 3.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 0.8);
    val = sub2.getValue<double>();
    EXPECT_EQ(val, 0.9);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 5.0);
    vfed.finalize();
    fut.get();
}

TEST(player_tests, player_test_message)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "pcore8";
    fedInfo.coreInitString = "-f 2 --autobroker";
    helics::apps::Player play1("player1", fedInfo);

    helics::MessageFederate mfed("block1", fedInfo);
    helics::Endpoint e1(helics::InterfaceVisibility::GLOBAL, &mfed, "dest");

    play1.addMessage(1.0, "src", "dest", "this is a message");
    auto fut = std::async(std::launch::async, [&play1]() { play1.run(); });
    mfed.enterExecutingMode();

    auto retTime = mfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    auto mess = e1.getMessage();
    EXPECT_TRUE(mess);
    if (mess) {
        EXPECT_EQ(mess->source, "src");
        EXPECT_EQ(mess->dest, "dest");
        EXPECT_EQ(mess->data.to_string(), "this is a message");
    }

    mfed.finalize();
    fut.get();
}

TEST(player_tests, player_test_message2)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "pcore9";
    fedInfo.coreInitString = "-f 2 --autobroker";
    helics::apps::Player play1("player1", fedInfo);

    helics::MessageFederate mfed("block1", fedInfo);
    helics::Endpoint e1(helics::InterfaceVisibility::GLOBAL, &mfed, "dest");

    play1.addMessage(1.0, "src", "dest", "this is a test message");
    play1.addMessage(2.0, "src", "dest", "this is test message2");

    play1.addMessage(3.0, "src", "dest", "this is message 3");
    auto fut = std::async(std::launch::async, [&play1]() { play1.run(); });
    mfed.enterExecutingMode();

    auto retTime = mfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    auto mess = e1.getMessage();
    EXPECT_TRUE(mess);
    if (mess) {
        EXPECT_EQ(mess->source, "src");
        EXPECT_EQ(mess->dest, "dest");
        EXPECT_EQ(mess->data.to_string(), "this is a test message");
    }

    retTime = mfed.requestTime(5);
    EXPECT_EQ(retTime, 2.0);
    mess = e1.getMessage();
    EXPECT_TRUE(mess);
    if (mess) {
        EXPECT_EQ(mess->source, "src");
        EXPECT_EQ(mess->dest, "dest");
        EXPECT_EQ(mess->data.to_string(), "this is test message2");
    }

    retTime = mfed.requestTime(5);
    EXPECT_EQ(retTime, 3.0);
    mess = e1.getMessage();
    EXPECT_TRUE(mess);
    if (mess) {
        EXPECT_EQ(mess->source, "src");
        EXPECT_EQ(mess->dest, "dest");
        EXPECT_EQ(mess->data.to_string(), "this is message 3");
    }
    mfed.finalize();
    fut.get();
}

TEST(player_tests, player_test_message3)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "pcore10";
    fedInfo.coreInitString = "-f 2 --autobroker";
    helics::apps::Player play1("player1", fedInfo);

    helics::MessageFederate mfed("block1", fedInfo);
    helics::Endpoint e1(helics::InterfaceVisibility::GLOBAL, &mfed, "dest");

    play1.addMessage(1.0, "src", "dest", "this is a test message");
    play1.addMessage(1.0, 2.0, "src", "dest", "this is test message2");

    play1.addMessage(2.0, 3.0, "src", "dest", "this is message 3");
    // mfed.getCorePointer()->setLoggingLevel(helics::invalid_fed_id, 5);
    auto fut = std::async(std::launch::async, [&play1]() { play1.run(); });
    mfed.enterExecutingMode();

    auto retTime = mfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    auto mess = e1.getMessage();
    EXPECT_TRUE(mess);
    if (mess) {
        EXPECT_EQ(mess->source, "src");
        EXPECT_EQ(mess->dest, "dest");
        EXPECT_EQ(mess->data.to_string(), "this is a test message");
    }

    retTime = mfed.requestTime(5);
    EXPECT_EQ(retTime, 2.0);
    mess = e1.getMessage();
    EXPECT_TRUE(mess);
    if (mess) {
        EXPECT_EQ(mess->source, "src");
        EXPECT_EQ(mess->dest, "dest");
        EXPECT_EQ(mess->data.to_string(), "this is test message2");
    }

    retTime = mfed.requestTime(5);
    EXPECT_EQ(retTime, 3.0);
    mess = e1.getMessage();
    EXPECT_TRUE(mess);
    if (mess) {
        EXPECT_EQ(mess->source, "src");
        EXPECT_EQ(mess->dest, "dest");
        EXPECT_EQ(mess->data.to_string(), "this is message 3");
    }
    mfed.finalize();
    fut.get();
}

static constexpr const char* simple_message_files[] = {"example_message1.player",
                                                       "example_message2.player",
                                                       "example_message3.json"};

class player_message_file_tests: public ::testing::TestWithParam<const char*> {};

TEST_P(player_message_file_tests, message_test_files)
{
    static char indx = 'a';
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = std::string("pcore11") + GetParam();
    fedInfo.coreName.push_back(indx++);
    fedInfo.coreInitString = "-f 2 --autobroker";
    helics::apps::Player play1("player1", fedInfo);

    helics::MessageFederate mfed("block1", fedInfo);
    helics::Endpoint e1(helics::InterfaceVisibility::GLOBAL, &mfed, "dest");
    play1.loadFile(std::string(TEST_DIR) + GetParam());
    auto fut = std::async(std::launch::async, [&play1]() { play1.run(); });
    mfed.enterExecutingMode();

    auto retTime = mfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    auto mess = e1.getMessage();
    EXPECT_TRUE(mess);
    if (mess) {
        EXPECT_EQ(mess->source, "src");
        EXPECT_EQ(mess->dest, "dest");
        EXPECT_EQ(mess->data.to_string(), "this is a test message");
    }

    retTime = mfed.requestTime(5);
    EXPECT_EQ(retTime, 2.0);
    mess = e1.getMessage();
    EXPECT_TRUE(mess);
    if (mess) {
        EXPECT_EQ(mess->source, "src");
        EXPECT_EQ(mess->dest, "dest");
        EXPECT_EQ(mess->data.to_string(), "this is test message2");
    }

    retTime = mfed.requestTime(5);
    EXPECT_EQ(retTime, 3.0);
    mess = e1.getMessage();
    EXPECT_TRUE(mess);
    if (mess) {
        EXPECT_EQ(mess->source, "src");
        EXPECT_EQ(mess->dest, "dest");
        EXPECT_EQ(mess->data.to_string(), "this is message 3");
    }
    mfed.finalize();
    fut.get();
}

INSTANTIATE_TEST_SUITE_P(player_tests,
                         player_message_file_tests,
                         ::testing::ValuesIn(simple_message_files));

TEST(player_tests, player_test_help)
{
    std::vector<std::string> args{"--quiet", "--version"};
    helics::apps::Player play1(args);

    EXPECT_TRUE(!play1.isActive());

    args.emplace_back("-?");
    helics::apps::Player play2(args);

    EXPECT_TRUE(!play2.isActive());
}
#ifndef DISABLE_SYSTEM_CALL_TESTS
/*
TEST( player_tests,simple_player_test_exe)
{
    static exeTestRunner playerExe (HELICS_BIN_LOC "apps/", "helics_player");

    static exeTestRunner brokerExe (HELICS_BIN_LOC "apps/", "helics_broker");

    auto res = brokerExe.runAsync ("1 --coretype=ipc --name=ipc_broker");
    std::string exampleFile = std::string (TEST_DIR) + "/example1.Player";
    auto res2 = playerExe.runCaptureOutputAsync ("--name=Player --broker=ipc_broker --coretype=ipc "
+ exampleFile);

    auto val = res2.get ();
    auto val2 = res.get ();
    EXPECT_EQ (val2, 0);
    std::string compareString = "read file 24 points for 1 tags";
    EXPECT_TRUE (val.compare (0, compareString.size (), compareString) == 0);
}
*/
#endif
