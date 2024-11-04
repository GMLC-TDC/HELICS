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

    helicsCreateApp("testApp","whatever",NULL,NULL,&err);
    EXPECT_NE(err.error_code,0);
}

/** this the same test as in the player tests
just meant to test the methods in C not the player itself
*/
TEST(app_tests, simple_player)
{
    auto err = helicsErrorInitialize();
    HelicsFederateInfo fedInfo=helicsCreateFederateInfo();
    helicsFederateInfoSetCoreType(fedInfo,HELICS_CORE_TYPE_TEST,&err);
    helicsFederateInfoSetCoreName(fedInfo,"pcore1",&err);
    helicsFederateInfoSetCoreInitString(fedInfo,"-f2 --autobroker",&err);


    auto play1=helicsCreateApp("player1","player",NULL,fedInfo,&err);
    EXPECT_TRUE(helicsAppIsActive(play1)==HELICS_TRUE);

    auto play1Fed=helicsAppGetFederate(play1,&err);
    EXPECT_TRUE(helicsFederateIsValid(play1Fed));

    helicsAppLoadFile(play1,(std::string(APP_TEST_DIR) + "example1.player").c_str(),&err);
    EXPECT_EQ(err.error_code,0);
    auto vFed=helicsCreateValueFederate("block1",fedInfo,&err);

    auto sub1 = helicsFederateRegisterSubscription(vFed,"pub1",nullptr,&err);
    auto sub2 = helicsFederateRegisterSubscription(vFed,"pub2",nullptr,&err);
    auto err2 = helicsErrorInitialize();

    auto fut = std::async(std::launch::async, [&play1,&err2]() { helicsAppRun(play1, &err2);});
    helicsFederateEnterExecutingMode(vFed,&err);
    auto val = helicsInputGetDouble(sub1,&err);
    EXPECT_EQ(val, 0.3);

    auto retTime = helicsFederateRequestTime(vFed,5,&err);
    EXPECT_EQ(retTime, 1.0);
    val = helicsInputGetDouble(sub1,&err);
    EXPECT_EQ(val, 0.5);
    val = helicsInputGetDouble(sub2,&err);
    EXPECT_DOUBLE_EQ(val, 0.4);

    retTime = helicsFederateRequestTime(vFed,5,&err);
    EXPECT_EQ(retTime, 2.0);
    val = helicsInputGetDouble(sub1,&err);
    EXPECT_EQ(val, 0.7);
    val = helicsInputGetDouble(sub2,&err);
    EXPECT_EQ(val, 0.6);

    retTime = helicsFederateRequestTime(vFed,5,&err);
    EXPECT_EQ(retTime, 3.0);
    val = helicsInputGetDouble(sub1,&err);
    EXPECT_EQ(val, 0.8);
    val = helicsInputGetDouble(sub2,&err);
    EXPECT_EQ(val, 0.9);

    retTime = helicsFederateRequestTime(vFed,5,&err);
    EXPECT_EQ(retTime, 5.0);
    helicsFederateFinalize(vFed,&err);
    fut.get();
    EXPECT_EQ(err2.error_code,0);
    EXPECT_EQ(err.error_code,0);
}

