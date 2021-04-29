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

#ifndef DISABLE_SYSTEM_CALL_TESTS
#    include "exeTestHelper.h"
#endif

#include "helics/application_api/Publications.hpp"
#include "helics/apps/BrokerApp.hpp"
#include "helics/apps/Recorder.hpp"

#include <cstdio>
#include <future>

TEST(recorder_tests, simple_recorder_test)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "rcore1";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Recorder rec1("rec1", fi);
    rec1.addSubscription("pub1");

    helics::ValueFederate vfed("block1", fi);
    helics::Publication pub1(helics::GLOBAL, &vfed, "pub1", helics::data_type::helics_double);
    auto fut = std::async(std::launch::async, [&rec1]() { rec1.runTo(4); });
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
    rec1.finalize();
    auto cnt = rec1.pointCount();
    EXPECT_EQ(cnt, 2U);
}

TEST(recorder_tests, simple_recorder_test2)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "rcore1-t2";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Recorder rec1("rec1", fi);
    rec1.addSubscription("pub1");

    helics::ValueFederate vfed("block1", fi);
    helics::Publication pub1(helics::GLOBAL, &vfed, "pub1", helics::data_type::helics_double);
    auto fut = std::async(std::launch::async, [&rec1]() { rec1.runTo(4); });
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
    rec1.finalize();
    auto v1 = rec1.getValue(0);
    EXPECT_EQ(std::get<1>(v1), "pub1");
    EXPECT_EQ(std::get<2>(v1), std::to_string(3.4));

    v1 = rec1.getValue(1);
    EXPECT_EQ(std::get<1>(v1), "pub1");
    EXPECT_EQ(std::get<2>(v1), std::to_string(4.7));

    v1 = rec1.getValue(2);
    EXPECT_EQ(std::get<1>(v1), std::string());
    EXPECT_EQ(std::get<2>(v1), std::string());

    auto m2 = rec1.getMessage(4);
    EXPECT_TRUE(!m2);
}

TEST(recorder_tests, recorder_test_message)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "rcore-tm";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Recorder rec1("rec1", fi);

    helics::MessageFederate mfed("block1", fi);
    helics::Endpoint e1(helics::GLOBAL, &mfed, "d1");

    rec1.addEndpoint("src1");

    auto fut = std::async(std::launch::async, [&rec1]() { rec1.runTo(5.0); });
    mfed.enterExecutingMode();

    auto retTime = mfed.requestTime(1.0);

    e1.send("src1", "this is a test message");
    EXPECT_EQ(retTime, 1.0);
    retTime = mfed.requestTime(2.0);
    e1.send("src1", "this is a test message2");
    EXPECT_EQ(retTime, 2.0);

    mfed.finalize();
    fut.get();
    EXPECT_EQ(rec1.messageCount(), 2U);

    auto m = rec1.getMessage(0);
    EXPECT_EQ(m->data.to_string(), "this is a test message");
}

class recorder_file_tests: public ::testing::TestWithParam<const char*> {
};

TEST_P(recorder_file_tests, test_files)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = std::string("coref") + GetParam();
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Recorder rec1("rec1", fi);

    rec1.loadFile(std::string(TEST_DIR) + GetParam());

    helics::ValueFederate vfed("block1", fi);
    helics::Publication pub1(helics::GLOBAL, &vfed, "pub1", helics::data_type::helics_double);
    helics::Publication pub2(helics::GLOBAL, &vfed, "pub2", helics::data_type::helics_double);

    auto fut = std::async(std::launch::async, [&rec1]() { rec1.runTo(4); });
    vfed.enterExecutingMode();
    auto retTime = vfed.requestTime(1);
    EXPECT_EQ(retTime, 1.0);
    pub1.publish(3.4);

    retTime = vfed.requestTime(1.5);
    EXPECT_EQ(retTime, 1.5);
    pub2.publish(5.7);

    retTime = vfed.requestTime(2.0);
    EXPECT_EQ(retTime, 2.0);
    pub1.publish(4.7);

    retTime = vfed.requestTime(3.0);
    EXPECT_EQ(retTime, 3.0);
    pub2.publish("3.9");

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 5.0);

    vfed.finalize();
    fut.get();
    rec1.finalize();
    EXPECT_EQ(rec1.pointCount(), 4U);
    auto v1 = rec1.getValue(0);
    EXPECT_EQ(std::get<1>(v1), "pub1");
    EXPECT_EQ(std::get<2>(v1), std::to_string(3.4));
    v1 = rec1.getValue(1);
    EXPECT_EQ(std::get<1>(v1), "pub2");
    EXPECT_EQ(std::get<2>(v1), std::to_string(5.7));

    v1 = rec1.getValue(2);
    EXPECT_EQ(std::get<1>(v1), "pub1");
    EXPECT_EQ(std::get<2>(v1), std::to_string(4.7));

    v1 = rec1.getValue(3);
    EXPECT_EQ(std::get<1>(v1), "pub2");
    EXPECT_EQ(std::get<2>(v1), std::to_string(3.9));
}

static constexpr const char* simple_files[] = {"example1.recorder",
                                               "example2.record",
                                               "example3rec.json",
                                               "example4rec.json",
                                               "exampleCapture.txt",
                                               "exampleCapture.json"};

INSTANTIATE_TEST_SUITE_P(recorder_tests, recorder_file_tests, ::testing::ValuesIn(simple_files));

static constexpr const char* simple_message_files[] = {"example4.recorder",
                                                       "example5.record",
                                                       "example6rec.json"};

class recorder_message_file_tests: public ::testing::TestWithParam<const char*> {
};

TEST_P(recorder_message_file_tests, test_message_files)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = std::string("rcoretmf") + GetParam();
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Recorder rec1("rec1", fi);

    rec1.loadFile(std::string(TEST_DIR) + GetParam());

    helics::CombinationFederate cfed("block1", fi);
    helics::Publication pub1(helics::GLOBAL, &cfed, "pub1", helics::data_type::helics_double);
    helics::Publication pub2(helics::GLOBAL, &cfed, "pub2", helics::data_type::helics_double);
    helics::Endpoint e1(helics::GLOBAL, &cfed, "d1");

    auto fut = std::async(std::launch::async, [&rec1]() { rec1.runTo(5); });
    cfed.enterExecutingMode();
    auto retTime = cfed.requestTime(1);
    EXPECT_EQ(retTime, 1.0);
    pub1.publish(3.4);
    e1.send("src1", "this is a test message");

    retTime = cfed.requestTime(1.5);
    EXPECT_EQ(retTime, 1.5);
    pub2.publish(5.7);

    retTime = cfed.requestTime(2.0);
    EXPECT_EQ(retTime, 2.0);
    e1.send("src1", "this is a test message2");
    pub1.publish(4.7);

    retTime = cfed.requestTime(3.0);
    EXPECT_EQ(retTime, 3.0);
    pub2.publish("3.9");

    retTime = cfed.requestTime(5);
    EXPECT_EQ(retTime, 5.0);

    cfed.finalize();
    fut.get();
    rec1.finalize();
    EXPECT_EQ(rec1.pointCount(), 4U);
    EXPECT_EQ(rec1.messageCount(), 2U);

    auto v1 = rec1.getValue(0);
    EXPECT_EQ(std::get<1>(v1), "pub1");
    EXPECT_EQ(std::get<2>(v1), std::to_string(3.4));
    v1 = rec1.getValue(1);
    EXPECT_EQ(std::get<1>(v1), "pub2");
    EXPECT_EQ(std::get<2>(v1), std::to_string(5.7));

    v1 = rec1.getValue(2);
    EXPECT_EQ(std::get<1>(v1), "pub1");
    EXPECT_EQ(std::get<2>(v1), std::to_string(4.7));

    v1 = rec1.getValue(3);
    EXPECT_EQ(std::get<1>(v1), "pub2");
    EXPECT_EQ(std::get<2>(v1), std::to_string(3.9));

    auto m = rec1.getMessage(1);
    ASSERT_TRUE(m);
    EXPECT_EQ(m->data.to_string(), "this is a test message2");
}

#ifdef ENABLE_IPC_CORE
TEST_P(recorder_message_file_tests, test_message_files_cmd)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    helics::apps::BrokerApp brk(helics::core_type::IPC, "ipc_broker", "-f 2");

    std::string exampleFile = std::string(TEST_DIR) + GetParam();

    std::vector<std::string> args{
        "", "--name=rec", "--broker=ipc_broker", "--coretype=ipc", exampleFile};
    char* argv[5];
    argv[0] = &(args[0][0]);
    argv[1] = &(args[1][0]);
    argv[2] = &(args[2][0]);
    argv[3] = &(args[3][0]);
    argv[4] = &(args[4][0]);

    helics::apps::Recorder rec1(5, argv);

    helics::FederateInfo fi(helics::core_type::IPC);
    fi.coreInitString = "-f 1 --broker=ipc_broker";

    helics::CombinationFederate cfed("obj", fi);
    helics::Publication pub1(helics::GLOBAL, &cfed, "pub1", helics::data_type::helics_double);
    helics::Publication pub2(helics::GLOBAL, &cfed, "pub2", helics::data_type::helics_double);
    helics::Endpoint e1(helics::GLOBAL, &cfed, "d1");

    auto fut = std::async(std::launch::async, [&rec1]() { rec1.runTo(5); });
    cfed.enterExecutingMode();
    auto retTime = cfed.requestTime(1);
    EXPECT_EQ(retTime, 1.0);
    pub1.publish(3.4);
    e1.send("src1", "this is a test message");

    retTime = cfed.requestTime(1.5);
    EXPECT_EQ(retTime, 1.5);
    pub2.publish(5.7);

    retTime = cfed.requestTime(2.0);
    EXPECT_EQ(retTime, 2.0);
    e1.send("src1", "this is a test message2");
    pub1.publish(4.7);

    retTime = cfed.requestTime(3.0);
    EXPECT_EQ(retTime, 3.0);
    pub2.publish("3.9");

    retTime = cfed.requestTime(5);
    EXPECT_EQ(retTime, 5.0);

    cfed.finalize();
    fut.get();
    rec1.finalize();
    EXPECT_EQ(rec1.pointCount(), 4U);
    EXPECT_EQ(rec1.messageCount(), 2U);

    auto v1 = rec1.getValue(0);
    EXPECT_EQ(std::get<1>(v1), "pub1");
    EXPECT_EQ(std::get<2>(v1), std::to_string(3.4));
    v1 = rec1.getValue(1);
    EXPECT_EQ(std::get<1>(v1), "pub2");
    EXPECT_EQ(std::get<2>(v1), std::to_string(5.7));

    v1 = rec1.getValue(2);
    EXPECT_EQ(std::get<1>(v1), "pub1");
    EXPECT_EQ(std::get<2>(v1), std::to_string(4.7));

    v1 = rec1.getValue(3);
    EXPECT_EQ(std::get<1>(v1), "pub2");
    EXPECT_EQ(std::get<2>(v1), std::to_string(3.9));

    auto m = rec1.getMessage(1);
    ASSERT_TRUE(m);
    EXPECT_EQ(m->data.to_string(), "this is a test message2");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}
#endif

INSTANTIATE_TEST_SUITE_P(recorder_tests,
                         recorder_message_file_tests,
                         ::testing::ValuesIn(simple_message_files));

TEST(recorder_tests, recorder_test_destendpoint_clone)
{
    helics::FederateInfo fi;
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "rcore-dep";
    fi.coreInitString = "-f 3 --autobroker";
    helics::apps::Recorder rec1("rec1", fi);
    fi.setProperty(helics_property_time_period, 1.0);

    helics::MessageFederate mfed("block1", fi);

    helics::MessageFederate mfed2("block2", fi);
    helics::Endpoint e1(helics::GLOBAL, &mfed, "d1");
    helics::Endpoint e2(helics::GLOBAL, &mfed2, "d2");

    rec1.addDestEndpointClone("d1");
    rec1.addDestEndpointClone("d2");

    auto fut = std::async(std::launch::async, [&rec1]() { rec1.runTo(5.0); });
    mfed2.enterExecutingModeAsync();
    mfed.enterExecutingMode();
    mfed2.enterExecutingModeComplete();

    mfed2.requestTimeAsync(1.0);
    auto retTime = mfed.requestTime(1.0);
    mfed2.requestTimeComplete();

    e1.send("d2", "this is a test message");
    EXPECT_EQ(retTime, 1.0);

    e2.send("d1", "this is a test message2");

    mfed2.requestTimeAsync(2.0);
    retTime = mfed.requestTime(2.0);
    EXPECT_EQ(retTime, 2.0);
    mfed2.requestTimeComplete();

    mfed.finalize();
    mfed2.finalize();
    fut.get();
    EXPECT_GE(rec1.messageCount(), 2U);

    auto m = rec1.getMessage(0);
    EXPECT_TRUE(m);
    if (m) {
        EXPECT_EQ(m->data.to_string(), "this is a test message");
    }
}

TEST(recorder_tests, recorder_test_srcendpoint_clone)
{
    helics::FederateInfo fi;
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "rcore2";
    fi.coreInitString = "-f 3 --autobroker";
    helics::apps::Recorder rec1("rec1", fi);
    fi.setProperty(helics_property_time_period, 1.0);

    helics::MessageFederate mfed("block1", fi);
    helics::MessageFederate mfed2("block2", fi);

    helics::Endpoint e1(helics::GLOBAL, &mfed, "d1");
    helics::Endpoint e2(helics::GLOBAL, &mfed2, "d2");

    rec1.addSourceEndpointClone("d1");
    rec1.addSourceEndpointClone("d2");

    auto fut = std::async(std::launch::async, [&rec1]() { rec1.runTo(5.0); });
    mfed2.enterExecutingModeAsync();
    mfed.enterExecutingMode();
    mfed2.enterExecutingModeComplete();

    mfed2.requestTimeAsync(1.0);
    auto retTime = mfed.requestTime(1.0);
    mfed2.requestTimeComplete();

    e1.send("d2", "this is a test message");
    EXPECT_EQ(retTime, 1.0);

    e2.send("d1", "this is a test message2");

    mfed2.requestTimeAsync(2.0);
    retTime = mfed.requestTime(2.0);
    EXPECT_EQ(retTime, 2.0);
    mfed2.requestTimeComplete();

    mfed.finalize();
    mfed2.finalize();
    fut.get();
    EXPECT_GE(rec1.messageCount(), 2U);

    auto m = rec1.getMessage(0);
    EXPECT_EQ(m->data.to_string(), "this is a test message");
}

TEST(recorder_tests, recorder_test_endpoint_clone)
{
    helics::FederateInfo fi(helics::core_type::TEST);

    fi.coreName = "rcore3";
    fi.coreInitString = "-f 3 --autobroker";
    helics::apps::Recorder rec1("rec1", fi);
    fi.setProperty(helics_property_time_period, 1.0);

    helics::MessageFederate mfed("block1", fi);

    helics::MessageFederate mfed2("block2", fi);
    helics::Endpoint e1(helics::GLOBAL, &mfed, "d1");
    helics::Endpoint e2(helics::GLOBAL, &mfed2, "d2");

    rec1.addDestEndpointClone("d1");
    rec1.addSourceEndpointClone("d1");

    auto fut = std::async(std::launch::async, [&rec1]() { rec1.runTo(5.0); });
    mfed2.enterExecutingModeAsync();
    mfed.enterExecutingMode();
    mfed2.enterExecutingModeComplete();

    mfed2.requestTimeAsync(1.0);
    auto retTime = mfed.requestTime(1.0);
    mfed2.requestTimeComplete();

    e1.send("d2", "this is a test message");
    EXPECT_EQ(retTime, 1.0);

    e2.send("d1", "this is a test message2");

    mfed2.requestTimeAsync(2.0);
    retTime = mfed.requestTime(2.0);
    EXPECT_EQ(retTime, 2.0);
    mfed2.requestTimeComplete();

    mfed.finalize();
    mfed2.finalize();
    fut.get();

    EXPECT_EQ(rec1.messageCount(), 2U);
    rec1.runTo(8.0);
    EXPECT_EQ(rec1.messageCount(), 2U);

    auto m = rec1.getMessage(0);
    rec1.finalize();
    ASSERT_TRUE(m);
    if (m) {
        EXPECT_EQ(m->data.to_string(), "this is a test message");
        EXPECT_EQ(m->time, 1.0);
    }
}

class recorder_clone_file_tests: public ::testing::TestWithParam<const char*> {
};

TEST_P(recorder_clone_file_tests, simple_clone_test_file)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = std::string("rcore4") + GetParam();
    fi.coreInitString = "-f 3 --autobroker";
    helics::apps::Recorder rec1("rec1", fi);
    fi.setProperty(helics_property_time_period, 1.0);

    helics::MessageFederate mfed("block1", fi);

    helics::MessageFederate mfed2("block2", fi);
    helics::Endpoint e1(helics::GLOBAL, &mfed, "d1");
    helics::Endpoint e2(helics::GLOBAL, &mfed2, "d2");

    rec1.loadFile(std::string(TEST_DIR) + GetParam());

    auto fut = std::async(std::launch::async, [&rec1]() { rec1.runTo(5.0); });
    mfed2.enterExecutingModeAsync();
    mfed.enterExecutingMode();
    mfed2.enterExecutingModeComplete();

    mfed2.requestTimeAsync(1.0);
    auto retTime = mfed.requestTime(1.0);
    mfed2.requestTimeComplete();

    e1.send("d2", "this is a test message");
    EXPECT_EQ(retTime, 1.0);

    e2.send("d1", "this is a test message2");

    mfed2.requestTimeAsync(2.0);
    retTime = mfed.requestTime(2.0);
    EXPECT_EQ(retTime, 2.0);
    mfed2.requestTimeComplete();

    mfed.finalize();
    mfed2.finalize();
    fut.get();
    EXPECT_GE(rec1.messageCount(), 2U);

    auto m = rec1.getMessage(0);
    EXPECT_TRUE(m);
    if (m) {
        EXPECT_EQ(m->data.to_string(), "this is a test message");
    }
}

static constexpr const char* simple_clone_test_files[] = {"clone_example1.txt",
                                                          "clone_example2.txt",
                                                          "clone_example3.txt",
                                                          "clone_example4.txt",
                                                          "clone_example5.txt",
                                                          "clone_example6.txt",
                                                          "clone_example7.json",
                                                          "clone_example8.JSON"};

INSTANTIATE_TEST_SUITE_P(recorder_tests,
                         recorder_clone_file_tests,
                         ::testing::ValuesIn(simple_clone_test_files));

TEST(recorder_tests, recorder_test_saveFile1)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "rcore5";
    fi.coreInitString = "-f 3 --autobroker";
    helics::apps::Recorder rec1("rec1", fi);
    fi.setProperty(helics_property_time_period, 1.0);

    helics::MessageFederate mfed("block1", fi);

    helics::MessageFederate mfed2("block2", fi);
    helics::Endpoint e1(helics::GLOBAL, &mfed, "d1");
    helics::Endpoint e2(helics::GLOBAL, &mfed2, "d2");

    rec1.addDestEndpointClone("d1");
    rec1.addSourceEndpointClone("d1");

    auto fut = std::async(std::launch::async, [&rec1]() { rec1.runTo(5.0); });
    mfed2.enterExecutingModeAsync();
    mfed.enterExecutingMode();
    mfed2.enterExecutingModeComplete();

    mfed2.requestTimeAsync(1.0);
    auto retTime = mfed.requestTime(1.0);
    mfed2.requestTimeComplete();

    e1.send("d2", "this is a test message");
    EXPECT_EQ(retTime, 1.0);

    e2.send("d1", "this is a test message2");

    mfed2.requestTimeAsync(2.0);
    retTime = mfed.requestTime(2.0);
    EXPECT_EQ(retTime, 2.0);
    mfed2.requestTimeComplete();

    mfed.finalize();
    mfed2.finalize();
    fut.get();
    EXPECT_EQ(rec1.messageCount(), 2U);

    auto filename = ghc::filesystem::temp_directory_path() / "savefile.txt";
    rec1.saveFile(filename.string());

    EXPECT_TRUE(ghc::filesystem::exists(filename));

    auto filename2 = ghc::filesystem::temp_directory_path() / "savefile.json";
    rec1.saveFile(filename2.string());

    EXPECT_TRUE(ghc::filesystem::exists(filename2));
    ghc::filesystem::remove(filename);
    ghc::filesystem::remove(filename2);
}

TEST(recorder_tests, recorder_test_saveFile2)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "rcore6";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Recorder rec1("rec1", fi);

    rec1.addSubscription("pub1");

    helics::ValueFederate vfed("block1", fi);
    helics::Publication pub1(helics::GLOBAL, &vfed, "pub1", helics::data_type::helics_double);
    auto fut = std::async(std::launch::async, [&rec1]() { rec1.runTo(4); });
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
    rec1.finalize();

    auto m2 = rec1.getMessage(4);
    EXPECT_TRUE(!m2);
    auto filename = ghc::filesystem::temp_directory_path() / "savefile.txt";
    rec1.saveFile(filename.string());

    EXPECT_TRUE(ghc::filesystem::exists(filename));

    auto filename2 = ghc::filesystem::temp_directory_path() / "savefile.json";
    rec1.saveFile(filename2.string());

    EXPECT_TRUE(ghc::filesystem::exists(filename2));
    ghc::filesystem::remove(filename);
    ghc::filesystem::remove(filename2);
}

TEST(recorder_tests, recorder_test_saveFile3)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "rcore7";
    fi.coreInitString = "-f 3 --autobroker";
    helics::apps::Recorder rec1("rec1", fi);
    fi.setProperty(helics_property_time_period, 1);

    helics::CombinationFederate mfed("block1", fi);

    helics::MessageFederate mfed2("block2", fi);
    helics::Endpoint e1(helics::GLOBAL, &mfed, "d1");
    helics::Endpoint e2(helics::GLOBAL, &mfed2, "d2");

    rec1.addDestEndpointClone("d1");
    rec1.addSourceEndpointClone("d1");
    rec1.addSubscription("pub1");

    helics::Publication pub1(helics::GLOBAL, &mfed, "pub1", helics::data_type::helics_double);

    auto fut = std::async(std::launch::async, [&rec1]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        rec1.runTo(5.0);
    });
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

    mfed.finalize();
    mfed2.finalize();
    fut.get();
    EXPECT_EQ(rec1.messageCount(), 2U);
    EXPECT_EQ(rec1.pointCount(), 3U);

    auto filename = ghc::filesystem::temp_directory_path() / "savefile.txt";
    rec1.saveFile(filename.string());

    EXPECT_TRUE(ghc::filesystem::exists(filename));

    auto filename2 = ghc::filesystem::temp_directory_path() / "savefile.json";
    rec1.saveFile(filename2.string());

    EXPECT_TRUE(ghc::filesystem::exists(filename2));
    ghc::filesystem::remove(filename);
    ghc::filesystem::remove(filename2);
}

TEST(recorder_tests, recorder_test_help)
{
    std::vector<std::string> args{"--quiet", "--version"};
    helics::apps::Recorder rec1(args);

    EXPECT_TRUE(!rec1.isActive());

    args.emplace_back("-?");
    helics::apps::Recorder rec2(args);

    EXPECT_TRUE(!rec2.isActive());
}
