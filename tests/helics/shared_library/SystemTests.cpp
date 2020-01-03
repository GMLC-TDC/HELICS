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

#include <thread>

TEST(other_tests, broker_global_value)
{
    auto err = helicsErrorInitialize();
    auto brk = helicsCreateBroker("test", "gbroker", "--root", &err);
    std::string globalVal = "this is a string constant that functions as a global";
    std::string globalVal2 = "this is a second string constant that functions as a global";
    helicsBrokerSetGlobal(brk, "testglobal", globalVal.c_str(), &err);
    auto q = helicsCreateQuery("global", "testglobal");
    auto res = helicsQueryBrokerExecute(q, brk, &err);
    EXPECT_EQ(res, globalVal);
    helicsBrokerSetGlobal(brk, "testglobal2", globalVal2.c_str(), &err);
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

    helicsBrokerDisconnect(brk, &err);
    helicsQueryFree(q);
    EXPECT_EQ(helicsBrokerIsConnected(brk), helics_false);
}

/*

TEST(other_tests, core_global_value)
{
    auto err = helicsErrorInitialize();
    auto brk = helicsCreateBroker("test", "gbrokerc", "--root", &err);

    auto cr = helicsCreateCore("test", "gcore", "--broker=gbrokerc", &err);
    EXPECT_EQ(err.error_code, 0);
    EXPECT_EQ(helicsCoreIsConnected(cr), helics_true);
    std::string globalVal = "this is a string constant that functions as a global";
    std::string globalVal2 = "this is a second string constant that functions as a global";
    helicsCoreSetGlobal(cr, "testglobal", globalVal.c_str(), &err);
    auto q = helicsCreateQuery("global", "testglobal");
    auto res = helicsQueryCoreExecute(q, cr, &err);
    EXPECT_EQ(res, globalVal);
    helicsCoreSetGlobal(cr, "testglobal2", globalVal2.c_str(), &err);
    helicsQueryFree(q);
    q = helicsCreateQuery("global", "testglobal2");
    res = helicsQueryCoreExecute(q, cr, &err);
    EXPECT_EQ(res, globalVal2);

    res = helicsQueryCoreExecute(nullptr, cr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    res = helicsQueryCoreExecute(q, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    res = helicsQueryCoreExecute(nullptr, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsCoreSetGlobal(cr, nullptr, "v2", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsCoreDisconnect(cr, &err);
    helicsBrokerDisconnect(brk, &err);

    helicsQueryFree(q);
    EXPECT_EQ(helicsBrokerIsConnected(brk), helics_false);
}

TEST(other_tests, federate_global_value)
{
    auto err = helicsErrorInitialize();
    auto brk = helicsCreateBroker("test", "gbrokerc", "--root", &err);

    auto cr = helicsCreateCore("test", "gcore", "--broker=gbrokerc", &err);

    const char* argv[4];
    argv[0] = "";
    argv[1] = "--corename=gcore";
    argv[2] = "--type=test";
    argv[3] = "--period=1.0";

    auto fi = helicsCreateFederateInfo();
    helicsFederateInfoLoadFromArgs(fi, 4, argv, &err);
    EXPECT_EQ(err.error_code, 0);

    auto fed = helicsCreateValueFederate("fed0", fi, &err);
    EXPECT_EQ(err.error_code, 0);

    argv[3] = "--period=frogs"; //this is meant to generate an error

    auto fi2 = helicsFederateInfoClone(fi, &err);
    EXPECT_NE(fi2, nullptr);
    helicsFederateInfoLoadFromArgs(fi2, 4, argv, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsFederateInfoFree(fi2);
    helicsFederateInfoFree(fi);

    std::string globalVal = "this is a string constant that functions as a global";
    std::string globalVal2 = "this is a second string constant that functions as a global";
    helicsFederateSetGlobal(fed, "testglobal", globalVal.c_str(), &err);
    auto q = helicsCreateQuery("global", "testglobal");
    auto res = helicsQueryExecute(q, fed, &err);
    EXPECT_EQ(res, globalVal);
    helicsFederateSetGlobal(fed, "testglobal2", globalVal2.c_str(), &err);
    helicsQueryFree(q);
    q = helicsCreateQuery("global", "testglobal2");
    helicsQueryExecuteAsync(q, fed, &err);
    while (helicsQueryIsCompleted(q) == helics_false) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    res = helicsQueryExecuteComplete(q, &err);
    EXPECT_EQ(res, globalVal2);

    res = helicsQueryExecute(nullptr, fed, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    res = helicsQueryExecute(q, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    res = helicsQueryExecute(nullptr, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsFederateSetGlobal(fed, nullptr, "v2", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsQueryExecuteAsync(q, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsFederateFinalize(fed, &err);

    helicsCoreDisconnect(cr, &err);
    helicsBrokerDisconnect(brk, &err);

    helicsQueryFree(q);
    EXPECT_EQ(helicsBrokerIsConnected(brk), helics_false);
}

TEST(other_tests, core_creation)
{
    auto err = helicsErrorInitialize();
    auto brk = helicsCreateBroker("test", "gbrokerc", "--root", &err);

    const char* argv[4];
    argv[0] = "";
    argv[1] = "--name=gcore";
    argv[2] = "--timeout=2000";
    argv[3] = "--broker=gbrokerc";

    auto cr = helicsCreateCoreFromArgs("test", nullptr, 4, argv, &err);
    EXPECT_EQ(err.error_code, 0);
    EXPECT_STREQ(helicsCoreGetIdentifier(cr), "gcore");

    argv[1] = "--name=gcore2";
    argv[2] = "--log-level=what_logs?";

    auto cr2 = helicsCreateCoreFromArgs("test", nullptr, 4, argv, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_EQ(cr2, nullptr);

    
    helicsBrokerDisconnect(brk, &err);
    helicsCoreDisconnect(cr, &err);
    
    EXPECT_EQ(helicsBrokerIsConnected(brk), helics_false);
}

TEST(other_tests, broker_creation)
{
    auto err = helicsErrorInitialize();

    const char* argv[4];
    argv[0] = "";
    argv[1] = "--name=gbrokerc";
    argv[2] = "--timeout=2000";
    argv[3] = "--root";

    auto brk = helicsCreateBrokerFromArgs("test", nullptr, 4, argv, &err);
    EXPECT_EQ(err.error_code, 0);
    EXPECT_STREQ(helicsBrokerGetIdentifier(brk), "gbrokerc");

    argv[1] = "--name=gbrokerc2";
    argv[2] = "--log-level=what_logs?";

    auto brk2 = helicsCreateBrokerFromArgs("test", nullptr, 4, argv, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_EQ(brk2, nullptr);

    helicsBrokerDisconnect(brk, &err);

}

*/
