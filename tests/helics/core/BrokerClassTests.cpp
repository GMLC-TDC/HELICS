/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreBroker.hpp"
#include "helics/helics-config.h"

#include "gtest/gtest.h"

/** test the assignment and retrieval of global value from a broker object*/
TEST(broker_tests, global_value_test)
{
    auto brk = helics::BrokerFactory::create(helics::core_type::TEST, "gbroker", "-f2 --root");
    std::string globalVal = "this is a string constant that functions as a global";
    std::string globalVal2 = "this is a second string constant that functions as a global";
    brk->setGlobal("testglobal", globalVal);
    auto res = brk->query("global", "testglobal");
    EXPECT_EQ(res, globalVal);
    brk->setGlobal("testglobal2", globalVal2);

    res = brk->query("global", "testglobal");
    EXPECT_EQ(res, globalVal);
    res = brk->query("global", "testglobal2");
    EXPECT_EQ(res, globalVal2);
    brk->disconnect();
    EXPECT_FALSE(brk->isConnected());
}
