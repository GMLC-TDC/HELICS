/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include <gtest/gtest.h>
/** these test cases test out the value converters
 */
#include "ctestFixtures.hpp"
#include "helics/helics.h"

#include <atomic>
#include <csignal>
#include <future>
#include <string>
#include <thread>

// select an incorrect app
TEST(app_tests, load_error)
{
    auto err = helicsErrorInitialize();

    helicsCreateApp("testApp", "whatever", NULL, NULL, &err);
    EXPECT_NE(err.error_code, 0);
}

/** this is the same test as in the player tests
just meant to test the methods in C not the player itself
*/
TEST(app_tests, simple_player)
{
    if (helicsAppEnabled() != HELICS_TRUE) {
        EXPECT_TRUE(true);
        return;
    }
    auto err = helicsErrorInitialize();
    HelicsFederateInfo fedInfo = helicsCreateFederateInfo();
    helicsFederateInfoSetCoreType(fedInfo, HELICS_CORE_TYPE_TEST, &err);
    helicsFederateInfoSetCoreName(fedInfo, "pscore1", &err);
    helicsFederateInfoSetCoreInitString(fedInfo,
                                        "-f2 --brokername=player_broker --autobroker",
                                        &err);
    auto play1 = helicsCreateApp("playerc1", "player", NULL, fedInfo, &err);
    EXPECT_TRUE(helicsAppIsActive(play1) == HELICS_TRUE);
    auto play1Fed = helicsAppGetFederate(play1, &err);
    EXPECT_TRUE(helicsFederateIsValid(play1Fed));

    helicsAppLoadFile(play1, (std::string(APP_TEST_DIR) + "example1.player").c_str(), &err);
    EXPECT_EQ(err.error_code, 0);
    helicsFederateInfoSetCoreInitString(fedInfo, "", &err);

    auto vFed = helicsCreateValueFederate("block1", fedInfo, &err);
    auto sub1 = helicsFederateRegisterSubscription(vFed, "pub1", nullptr, &err);
    auto sub2 = helicsFederateRegisterSubscription(vFed, "pub2", nullptr, &err);
    auto err2 = helicsErrorInitialize();

    auto thread1 = std::thread([&play1, &err2]() { helicsAppRun(play1, &err2); });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    helicsFederateEnterExecutingMode(vFed, &err);
    auto val = helicsInputGetDouble(sub1, &err);
    EXPECT_EQ(val, 0.3);
    auto retTime = helicsFederateRequestTime(vFed, 5, &err);
    EXPECT_EQ(retTime, 1.0);
    val = helicsInputGetDouble(sub1, &err);
    EXPECT_EQ(val, 0.5);
    val = helicsInputGetDouble(sub2, &err);
    EXPECT_DOUBLE_EQ(val, 0.4);

    retTime = helicsFederateRequestTime(vFed, 5, &err);
    EXPECT_EQ(retTime, 2.0);
    val = helicsInputGetDouble(sub1, &err);
    EXPECT_EQ(val, 0.7);
    val = helicsInputGetDouble(sub2, &err);
    EXPECT_EQ(val, 0.6);
    retTime = helicsFederateRequestTime(vFed, 5, &err);
    EXPECT_EQ(retTime, 3.0);
    val = helicsInputGetDouble(sub1, &err);
    EXPECT_EQ(val, 0.8);
    val = helicsInputGetDouble(sub2, &err);
    EXPECT_EQ(val, 0.9);

    retTime = helicsFederateRequestTime(vFed, 5, &err);
    EXPECT_EQ(retTime, 5.0);
    helicsFederateDestroy(vFed);
    thread1.join();
    EXPECT_EQ(err2.error_code, 0);
    EXPECT_EQ(err.error_code, 0);
}

TEST(app_tests, recorder)
{
    if (helicsAppEnabled() != HELICS_TRUE) {
        EXPECT_TRUE(true);
        return;
    }
    auto err = helicsErrorInitialize();
    HelicsFederateInfo fedInfo = helicsCreateFederateInfo();
    helicsFederateInfoSetCoreType(fedInfo, HELICS_CORE_TYPE_TEST, &err);
    helicsFederateInfoSetCoreName(fedInfo, "rcore1", &err);
    helicsFederateInfoSetCoreInitString(fedInfo, "-f2 --brokername=rec_broker --autobroker", &err);

    auto rec1 = helicsCreateApp("recc1", "recorder", NULL, fedInfo, &err);
    EXPECT_TRUE(helicsAppIsActive(rec1) == HELICS_TRUE);

    helicsAppLoadFile(rec1, (std::string(APP_TEST_DIR) + "example3rec.json").c_str(), &err);
    EXPECT_EQ(err.error_code, 0);

    auto vFed = helicsCreateValueFederate("block1", fedInfo, &err);
    auto pub1 =
        helicsFederateRegisterGlobalPublication(vFed, "pub1", HELICS_DATA_TYPE_DOUBLE, NULL, &err);
    auto pub2 =
        helicsFederateRegisterGlobalPublication(vFed, "pub2", HELICS_DATA_TYPE_DOUBLE, NULL, &err);

    auto err2 = helicsErrorInitialize();

    auto thread1 = std::thread([&rec1, &err2]() { helicsAppRunTo(rec1, 4.0, &err2); });
    helicsFederateEnterExecutingMode(vFed, &err);
    auto retTime = helicsFederateRequestTime(vFed, 1, &err);
    EXPECT_EQ(retTime, 1.0);
    helicsPublicationPublishDouble(pub1, 3.4, &err);

    retTime = helicsFederateRequestTime(vFed, 1.5, &err);
    EXPECT_EQ(retTime, 1.5);
    helicsPublicationPublishDouble(pub2, 5.7, &err);

    retTime = helicsFederateRequestTime(vFed, 2, &err);
    EXPECT_EQ(retTime, 2.0);
    helicsPublicationPublishDouble(pub1, 4.7, &err);

    retTime = helicsFederateRequestTime(vFed, 3.0, &err);
    EXPECT_EQ(retTime, 3.0);
    helicsPublicationPublishString(pub2, "3.9", &err);

    retTime = helicsFederateRequestTime(vFed, 5.0, &err);
    EXPECT_EQ(retTime, 5.0);

    helicsFederateDestroy(vFed);

    thread1.join();

    helicsAppDestroy(rec1);
}

TEST(app_tests, recorder_object_nosan_ci_skip)
{
    // this test would trigger address sanitizer errors
    if (helicsAppEnabled() != HELICS_TRUE) {
        EXPECT_TRUE(true);
        return;
    }
    auto err = helicsErrorInitialize();
    HelicsFederateInfo fedInfo = helicsCreateFederateInfo();
    helicsFederateInfoSetCoreType(fedInfo, HELICS_CORE_TYPE_TEST, &err);
    helicsFederateInfoSetCoreName(fedInfo, "rcoret2", &err);
    helicsFederateInfoSetCoreInitString(fedInfo, "-f1 --brokername=rec_brokerb --autobroker", &err);

    auto recc1 = helicsCreateApp("recc1", "recorder", NULL, fedInfo, &err);
    EXPECT_TRUE(helicsAppIsActive(recc1) == HELICS_TRUE);

    helicsAppDestroy(recc1);

    EXPECT_EQ(helicsAppIsActive(recc1), HELICS_FALSE);
}

TEST(app_tests, connector)
{
    if (helicsAppEnabled() != HELICS_TRUE) {
        EXPECT_TRUE(true);
        return;
    }
    auto err = helicsErrorInitialize();
    HelicsFederateInfo fedInfo = helicsCreateFederateInfo();
    helicsFederateInfoSetCoreType(fedInfo, HELICS_CORE_TYPE_TEST, &err);
    helicsFederateInfoSetTimeProperty(fedInfo, HELICS_PROPERTY_TIME_PERIOD, 1.0, &err);
    helicsFederateInfoSetCoreName(fedInfo, "ccoref5", &err);
    helicsFederateInfoSetCoreInitString(fedInfo, "-f2 --brokername=conn_broker --autobroker", &err);

    auto conn1 = helicsCreateApp("connectorc1", "connector", NULL, fedInfo, &err);
    EXPECT_TRUE(helicsAppIsActive(conn1) == HELICS_TRUE);

    helicsAppLoadFile(conn1,
                      (std::string(APP_TEST_DIR) + "connector/simple_tags.txt").c_str(),
                      &err);
    EXPECT_EQ(err.error_code, 0);

    auto vFed = helicsCreateValueFederate("c1", fedInfo, &err);

    auto pub1 = helicsFederateRegisterGlobalPublication(
        vFed, "pub1", HELICS_DATA_TYPE_DOUBLE, nullptr, &err);

    auto inp1 =
        helicsFederateRegisterGlobalInput(vFed, "inp1", HELICS_DATA_TYPE_DOUBLE, nullptr, &err);
    auto inp2 =
        helicsFederateRegisterGlobalInput(vFed, "inp2", HELICS_DATA_TYPE_DOUBLE, nullptr, &err);
    helicsFederateSetGlobal(vFed, "tag2", "true", &err);
    auto err2 = helicsErrorInitialize();
    auto thread1 = std::thread([&conn1, &err2]() { helicsAppRun(conn1, &err2); });
    helicsFederateEnterExecutingMode(vFed, &err);

    const double testValue = 3452.562;
    helicsPublicationPublishDouble(pub1, testValue, &err);

    auto retTime = helicsFederateRequestTime(vFed, 5.0, &err);
    EXPECT_EQ(retTime, 1.0);
    auto val = helicsInputGetDouble(inp1, &err);
    EXPECT_EQ(val, HELICS_INVALID_DOUBLE);

    val = helicsInputGetDouble(inp2, &err);
    EXPECT_EQ(val, testValue);
    helicsFederateDestroy(vFed);
    thread1.join();
    EXPECT_EQ(err2.error_code, 0);

    helicsAppDestroy(conn1);
}

TEST(app_tests, echo)
{
    if (helicsAppEnabled() != HELICS_TRUE) {
        EXPECT_TRUE(true);
        return;
    }
    auto err = helicsErrorInitialize();
    HelicsFederateInfo fedInfo = helicsCreateFederateInfo();
    helicsFederateInfoSetCoreType(fedInfo, HELICS_CORE_TYPE_TEST, &err);
    helicsFederateInfoSetCoreName(fedInfo, "ecore4-file", &err);
    helicsFederateInfoSetCoreInitString(fedInfo, "-f2 --brokername=echo_broker --autobroker", &err);

    auto echo1 = helicsCreateApp("echoc1", "echo", NULL, fedInfo, &err);
    EXPECT_TRUE(helicsAppIsActive(echo1) == HELICS_TRUE);

    helicsAppLoadFile(echo1, (std::string(APP_TEST_DIR) + "echo_example.json").c_str(), &err);
    EXPECT_EQ(err.error_code, 0);

    auto mFed = helicsCreateMessageFederate("source", fedInfo, &err);

    auto ep1 = helicsFederateRegisterEndpoint(mFed, "src", "", &err);
    auto err2 = helicsErrorInitialize();
    auto thread1 = std::thread([&echo1, &err2]() { helicsAppRunTo(echo1, 5.0, &err2); });
    helicsFederateEnterExecutingMode(mFed, &err);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    helicsEndpointSendStringTo(ep1, "hello world", "test", &err);
    helicsFederateRequestTime(mFed, 1.0, &err);
    helicsEndpointSendStringTo(ep1, "hello again", "test2", &err);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(helicsEndpointHasMessage(ep1));
    auto ntime = helicsFederateRequestTime(mFed, 2.0, &err);
    EXPECT_DOUBLE_EQ(ntime, HELICS_TIME_EPSILON + 1.2);
    EXPECT_TRUE(helicsEndpointHasMessage(ep1));
    auto message = helicsEndpointGetMessage(ep1);
    ASSERT_TRUE(message);
    EXPECT_STREQ(helicsMessageGetString(message), "hello world");
    EXPECT_STREQ(helicsMessageGetSource(message), "test");

    ntime = helicsFederateRequestTime(mFed, 3.0, &err);
    EXPECT_EQ(ntime, 2.2);
    EXPECT_TRUE(helicsEndpointHasMessage(ep1));
    helicsMessageFree(message);
    message = helicsEndpointGetMessage(ep1);
    ASSERT_TRUE(message);
    EXPECT_STREQ(helicsMessageGetString(message), "hello again");
    EXPECT_STREQ(helicsMessageGetSource(message), "test2");
    helicsMessageFree(message);
    helicsFederateDestroy(mFed);
    thread1.join();
    EXPECT_EQ(err2.error_code, 0);
}
