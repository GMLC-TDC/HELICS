/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include <gtest/gtest.h>
/** these test cases test out the value converters
 */
#include "ctestFixtures.hpp"

TEST(other_tests, global_value_test)
{
    auto err = helicsErrorInitialize();
    auto brk = helicsCreateBroker("test", "gbroker", "-f2 --root",&err);
    std::string globalVal = "this is a string constant that functions as a global";
    std::string globalVal2 = "this is a second string constant that functions as a global";
    helicsBrokerSetGlobal(brk,"testglobal", globalVal.c_str(),&err);
    auto q = helicsCreateQuery("global", "testglobal");
    auto res = helicsQueryBrokerExecute(q,brk,&err);
    EXPECT_EQ(res, globalVal);
    helicsBrokerSetGlobal(brk,"testglobal2", globalVal2.c_str(),&err);
    helicsQueryFree(q);
    q = helicsCreateQuery("global", "testglobal2");
    res = helicsQueryBrokerExecute(q, brk, &err);
    EXPECT_EQ(res, globalVal2);

    res = helicsQueryBrokerExecute(nullptr, brk, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    res = helicsQueryBrokerExecute(q, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    res = helicsQueryBrokerExecute(nullptr, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsBrokerSetGlobal(brk, nullptr, "v2", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsBrokerDisconnect(brk,&err);
    helicsQueryFree(q);
    EXPECT_EQ(helicsBrokerIsConnected(brk),helics_false);
}
