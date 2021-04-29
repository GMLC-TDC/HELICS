/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "ctestFixtures.hpp"

#include <gtest/gtest.h>

struct config_tests: public FederateTestFixture, public ::testing::Test {
};

#ifdef ENABLE_ZMQ_CORE
/** test simple creation and destruction*/
TEST_F(config_tests, control_file_test)
{
    helics_broker broker = AddBroker("zmq", 1);
    EXPECT_TRUE(nullptr != broker);

    std::string testFile(TEST_DIR);
    testFile.append("Control_test.json");

    auto cfed = helicsCreateCombinationFederateFromConfig(testFile.c_str(), &err);

    EXPECT_TRUE(helicsFederateIsValid(cfed));

    EXPECT_EQ(helicsFederateGetEndpointCount(cfed), 6);
    EXPECT_EQ(helicsFederateGetFilterCount(cfed), 6);
    EXPECT_EQ(helicsFederateGetInputCount(cfed), 7);

    auto ept = helicsFederateGetEndpointByIndex(cfed, 0, &err);

    EXPECT_STREQ(helicsEndpointGetName(ept), "EV_Controller/EV6");

    auto filt = helicsFederateGetFilterByIndex(cfed, 3, &err);

    EXPECT_STREQ(helicsFilterGetName(filt), "EV_Controller/filterEV3");

    auto ipt = helicsFederateGetInputByIndex(cfed, 4, &err);
    EXPECT_STREQ(helicsSubscriptionGetKey(ipt), "IEEE_123_feeder_0/charge_EV3");

    helicsFederateDestroy(cfed);
}

#endif
