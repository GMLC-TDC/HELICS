/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "exeTestHelper.h"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/apps/Source.hpp"
#include "helics/core/BrokerFactory.hpp"

#include "gtest/gtest.h"
#include <future>

TEST(source_tests, simple_source_test)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "score-source";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Source src1("player1", fi);
    auto index = src1.addSignalGenerator("ramp", "ramp");
    auto gen = src1.getGenerator(index);

    if (!gen) {
        src1.finalize();
        ASSERT_TRUE(gen);
        return;
    }
    gen->set("ramp", 0.3);
    gen->set("level", 1.0);
    src1.addPublication("pub1", helics::data_type::helics_double, 1.0);
    src1.setStartTime("pub1", 1.0);
    helics::ValueFederate vfed("block1", fi);
    auto& sub1 = vfed.registerSubscription("pub1");
    auto fut = std::async(std::launch::async, [&src1]() {
        src1.runTo(5);
        src1.finalize();
    });
    vfed.enterExecutingMode();
    auto retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    auto val = sub1.getValue<double>();
    EXPECT_EQ(val, 1.3);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 2.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 1.6);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 3.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 1.9);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 4.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 2.2);

    retTime = vfed.requestTime(6);
    EXPECT_EQ(retTime, 5.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 2.5);
    vfed.finalize();
    fut.get();
}

TEST(source_tests, simple_source_test2)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "score-simple";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Source src1("player1", fi);

    auto index = src1.addSignalGenerator("ramp", "ramp");
    auto index2 = src1.addSignalGenerator("ramp2", "ramp");
    auto gen = src1.getGenerator(index);
    auto gen2 = src1.getGenerator(index2);
    EXPECT_TRUE(gen);
    EXPECT_TRUE(gen2);
    if (!gen || !gen2) {
        src1.finalize();
        return;
    }
    gen->set("ramp", 0.3);
    gen->set("level", 1.0);
    src1.addPublication("pub1", "ramp", helics::data_type::helics_double, 1.0);
    src1.setStartTime("pub1", 1.0);
    gen2->set("ramp", 0.6);
    gen2->set("level", 2.0);
    src1.addPublication("pub2", "ramp2", helics::data_type::helics_double, 2.0);
    src1.setStartTime("pub2", 3.0);
    helics::ValueFederate vfed("block1", fi);
    auto& sub1 = vfed.registerSubscription("pub1");
    auto& sub2 = vfed.registerSubscription("pub2");
    auto fut = std::async(std::launch::async, [&src1]() {
        src1.runTo(5);
        src1.finalize();
    });
    vfed.enterExecutingMode();
    auto retTime = vfed.requestTime(1.1);
    EXPECT_EQ(retTime, 1.0);
    auto val = sub1.getValue<double>();
    EXPECT_EQ(val, 1.3);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 2.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 1.6);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 3.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 1.9);
    val = sub2.getValue<double>();
    EXPECT_EQ(val, 3.8);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 4.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 2.2);
    val = sub2.getValue<double>();
    EXPECT_EQ(val, 3.8);

    retTime = vfed.requestTime(6);
    EXPECT_EQ(retTime, 5.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 2.5);
    val = sub2.getValue<double>();
    EXPECT_EQ(val, 5.0);
    vfed.finalize();
    fut.get();
}

TEST(source_tests, sine_source_test)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "score-sine";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Source src1("player1", fi);

    auto index = src1.addSignalGenerator("sine", "sine");
    auto gen = src1.getGenerator(index);
    EXPECT_TRUE(gen);
    if (gen) {
        gen->set("freq", 0.5);
        gen->set("amplitude", 1.0);
    }
    src1.addPublication("pub1", helics::data_type::helics_double, 0.5);
    src1.setStartTime("pub1", 1.0);
    helics::ValueFederate vfed("block1", fi);
    auto& sub1 = vfed.registerSubscription("pub1");
    auto fut = std::async(std::launch::async, [&src1]() {
        src1.runTo(5);
        src1.finalize();
    });
    vfed.enterExecutingMode();
    auto retTime = vfed.requestTime(5);

    EXPECT_EQ(retTime, 1.0);
    double val = sub1.getValue<double>();
    EXPECT_NEAR(val, 0.0, 1e-12);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.5);
    val = sub1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, -1.0);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 2.0);
    val = sub1.getValue<double>();
    EXPECT_NEAR(val, 0.0, 1e-12);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 2.5);
    val = sub1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 1.0);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 3.0);
    val = sub1.getValue<double>();
    EXPECT_NEAR(val, 0.0, 1e-12);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 3.5);
    val = sub1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, -1.0);
    vfed.finalize();
    fut.get();
}

TEST(source_tests, simple_source_test_file)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "scorep";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Source src1("source1", fi);
    src1.loadFile(std::string(TEST_DIR) + "/simple_source_test.json");

    helics::ValueFederate vfed("block1", fi);
    auto& sub1 = vfed.registerSubscription("pub1");
    auto fut = std::async(std::launch::async, [&src1]() {
        src1.runTo(5);
        src1.finalize();
    });
    vfed.enterExecutingMode();
    auto retTime = vfed.requestTime(1.1);
    EXPECT_EQ(retTime, 1.0);
    auto val = sub1.getValue<double>();
    EXPECT_EQ(val, 1.3);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 2.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 1.6);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 3.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 1.9);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 4.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 2.2);

    retTime = vfed.requestTime(6);
    EXPECT_EQ(retTime, 5.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 2.5);
    vfed.finalize();
    fut.get();
}

TEST(source_tests, simple_source_test2_file)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "score2";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Source src1("player1", fi);

    src1.loadFile(std::string(TEST_DIR) + "/simple_source_test2.json");
    helics::ValueFederate vfed("block1", fi);
    auto& sub1 = vfed.registerSubscription("pub1");
    auto& sub2 = vfed.registerSubscription("pub2");
    auto fut = std::async(std::launch::async, [&src1]() {
        src1.runTo(5);
        src1.finalize();
    });
    vfed.enterExecutingMode();
    auto retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.0);
    auto val = sub1.getValue<double>();
    EXPECT_EQ(val, 1.3);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 2.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 1.6);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 3.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 1.9);
    val = sub2.getValue<double>();
    EXPECT_EQ(val, 3.8);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 4.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 2.2);
    val = sub2.getValue<double>();
    EXPECT_EQ(val, 3.8);

    retTime = vfed.requestTime(6);
    EXPECT_EQ(retTime, 5.0);
    val = sub1.getValue<double>();
    EXPECT_EQ(val, 2.5);
    val = sub2.getValue<double>();
    EXPECT_EQ(val, 5.0);
    vfed.finalize();
    fut.get();
}

TEST(source_tests, sine_source_test_file)
{
    helics::FederateInfo fi(helics::core_type::TEST);

    fi.coreName = "score1";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Source src1("player1", fi);
    src1.loadFile(std::string(TEST_DIR) + "/simple_sine_source.json");

    helics::ValueFederate vfed("block1", fi);
    auto& sub1 = vfed.registerSubscription("pub1");
    auto fut = std::async(std::launch::async, [&src1]() {
        src1.runTo(5);
        src1.finalize();
    });
    vfed.enterExecutingMode();
    auto retTime = vfed.requestTime(5);

    EXPECT_EQ(retTime, 1.0);
    double val = sub1.getValue<double>();
    EXPECT_NEAR(val, 0.0, 1e-12);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 1.5);
    val = sub1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, -1.0);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 2.0);
    val = sub1.getValue<double>();
    EXPECT_NEAR(val, 0.0, 1e-12);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 2.5);
    val = sub1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 1.0);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 3.0);
    val = sub1.getValue<double>();
    EXPECT_NEAR(val, 0.0, 1e-12);

    retTime = vfed.requestTime(5);
    EXPECT_EQ(retTime, 3.5);
    val = sub1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, -1.0);
    vfed.finalize();
    fut.get();
}
