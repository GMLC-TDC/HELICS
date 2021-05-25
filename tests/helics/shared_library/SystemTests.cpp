/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include <gtest/gtest.h>
/** these test cases test out the value converters
 */
#include "ctestFixtures.hpp"

#include <atomic>
#include <csignal>
#include <thread>

// test generating a global from a broker and some of its error pathways
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
    EXPECT_STREQ("#invalid", res);

    res = helicsQueryBrokerExecute(q, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_STREQ("#invalid", res);

    res = helicsQueryBrokerExecute(nullptr, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_STREQ("#invalid", res);

    helicsBrokerSetGlobal(brk, nullptr, "v2", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsBrokerDisconnect(brk, &err);
    helicsQueryFree(q);
    EXPECT_EQ(helicsBrokerIsConnected(brk), helics_false);
    helicsBrokerFree(brk);
}

// test global value creation from a core and its error pathways
TEST(other_tests, core_global_value)
{
    helicsCloseLibrary();
    auto err = helicsErrorInitialize();
    auto brk = helicsCreateBroker("test", "gbrokerc", "--root", &err);

    auto cr = helicsCreateCore("test", "gcore", "--broker=gbrokerc", &err);
    EXPECT_EQ(err.error_code, 0);
    auto connected = helicsCoreConnect(cr, &err);
    EXPECT_EQ(connected, helics_true);
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
    EXPECT_STREQ("#invalid", res);
    res = helicsQueryCoreExecute(q, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_STREQ("#invalid", res);
    res = helicsQueryCoreExecute(nullptr, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_STREQ("#invalid", res);
    helicsCoreSetGlobal(cr, nullptr, "v2", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    helicsBrokerDisconnect(brk, &err);
    helicsCoreDisconnect(cr, &err);

    helicsQueryFree(q);
    EXPECT_EQ(helicsBrokerIsConnected(brk), helics_false);
}

// test global value creation from a federate and some error pathways for queries and global
// creation
TEST(other_tests, federate_global_value)
{
    auto err = helicsErrorInitialize();
    auto brk = helicsCreateBroker("test", "gbrokerc", "--root", &err);

    auto cr = helicsCreateCore("test", "gcore", "--broker=gbrokerc", &err);

    // test creation of federateInfo from command line arguments
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

    argv[3] = "--period=frogs";  // this is meant to generate an error in command line processing

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

    auto q2 = helicsCreateQuery(nullptr, "isinit");
    helicsQueryExecuteAsync(q2, fed, &err);
    while (helicsQueryIsCompleted(q2) == helics_false) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    res = helicsQueryExecuteComplete(q2, &err);
    EXPECT_STREQ(res, "false");

    // a series of invalid query calls
    res = helicsQueryExecute(nullptr, fed, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_STREQ("#invalid", res);

    res = helicsQueryExecute(q, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_STREQ("#invalid", res);

    res = helicsQueryExecute(nullptr, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_STREQ("#invalid", res);

    helicsFederateSetGlobal(fed, nullptr, "v2", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsQueryExecuteAsync(q, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsQueryExecuteAsync(nullptr, fed, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsFederateFinalize(fed, &err);

    helicsCoreDisconnect(cr, &err);
    helicsBrokerDisconnect(brk, &err);

    helicsQueryFree(q);
    helicsQueryFree(q2);
    EXPECT_EQ(helicsBrokerIsConnected(brk), helics_false);
}

// test global value creation from a federate and some error pathways for queries and global
// creation
TEST(other_tests, federate_add_dependency)
{
    auto err = helicsErrorInitialize();
    auto brk = helicsCreateBroker("test", "gbrokerd", "--root", &err);

    auto cr = helicsCreateCore("test", "dcore", "--broker=gbrokerd", &err);

    // test creation of federateInfo from command line arguments
    const char* argv[4];
    argv[0] = "";
    argv[1] = "--corename=dcore";
    argv[2] = "--type=test";
    argv[3] = "--period=1.0";

    auto fi = helicsCreateFederateInfo();
    helicsFederateInfoLoadFromArgs(fi, 4, argv, &err);
    helicsFederateInfoSetFlagOption(fi, helics_flag_source_only, helics_true, &err);

    auto fed1 = helicsCreateMessageFederate("fed1", fi, &err);
    EXPECT_EQ(err.error_code, 0);

    auto fi2 = helicsCreateFederateInfo();
    helicsFederateInfoLoadFromArgs(fi2, 4, argv, &err);
    auto fed2 = helicsCreateMessageFederate("fed2", fi2, &err);
    helicsFederateRegisterGlobalEndpoint(fed2, "ept2", nullptr, &err);

    helicsFederateRegisterGlobalEndpoint(fed1, "ept1", nullptr, &err);

    helicsFederateAddDependency(fed1, "fed2", &err);
    EXPECT_EQ(err.error_code, 0);

    helicsFederateEnterExecutingModeAsync(fed1, &err);
    helicsFederateEnterExecutingMode(fed2, &err);
    helicsFederateEnterExecutingModeComplete(fed1, &err);

    helicsFederateInfoFree(fi);
    helicsFederateInfoFree(fi2);
    helicsFederateFinalize(fed1, &err);
    helicsFederateFinalize(fed2, &err);
    helicsBrokerFree(brk);
    helicsCoreFree(cr);
}

// test core creation from command line arguments
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

// test broker creation from command line arguments
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

TEST(federate_tests, federateGeneratedLocalError)
{
    auto fi = helicsCreateFederateInfo();
    helicsFederateInfoSetCoreType(fi, helics_core_type_test, nullptr);
    helicsFederateInfoSetCoreName(fi, "core_full_le", nullptr);
    helicsFederateInfoSetCoreInitString(fi, "-f 1 --autobroker --error_timeout=0", nullptr);

    auto fed1 = helicsCreateValueFederate("fed1", fi, nullptr);
    helicsFederateInfoFree(fi);
    helicsFederateEnterExecutingMode(fed1, nullptr);

    helicsFederateRequestTime(fed1, 2.0, nullptr);
    helicsFederateLocalError(fed1, 9827, "user generated error");

    auto err = helicsErrorInitialize();
    helicsFederateRequestTime(fed1, 3.0, &err);
    EXPECT_NE(err.error_code, 0);

    auto cr = helicsFederateGetCoreObject(fed1, nullptr);
    helicsCoreDisconnect(cr, nullptr);
    helicsCoreFree(cr);
    helicsFederateDestroy(fed1);
}

TEST(federate_tests, federateGeneratedGlobalError)
{
    auto fi = helicsCreateFederateInfo();
    helicsFederateInfoSetCoreType(fi, helics_core_type_test, nullptr);
    helicsFederateInfoSetCoreName(fi, "core_full_ge", nullptr);
    helicsFederateInfoSetCoreInitString(fi, "-f 1 --autobroker --error_timeout=0", nullptr);

    auto fed1 = helicsCreateValueFederate("fed1", fi, nullptr);
    helicsFederateInfoFree(fi);
    helicsFederateEnterExecutingMode(fed1, nullptr);

    helicsFederateRequestTime(fed1, 2.0, nullptr);
    helicsFederateGlobalError(fed1, 9827, "user generated global error");

    auto err = helicsErrorInitialize();
    helicsFederateRequestTime(fed1, 3.0, &err);
    EXPECT_NE(err.error_code, 0);

    helicsFederateDestroy(fed1);
}

// test generating a global from a broker and some of its error pathways
TEST(other_tests, broker_after_close)
{
    auto err = helicsErrorInitialize();
    auto brk = helicsCreateBroker("test", "gbroker_test", "--root", &err);
    std::string globalVal = "this is a string constant that functions as a global";
    std::string globalVal2 = "this is a second string constant that functions as a global";
    helicsBrokerSetGlobal(brk, "testglobal", globalVal.c_str(), &err);
    auto q = helicsCreateQuery("global", "testglobal");
    auto res = helicsQueryBrokerExecute(q, brk, &err);
    EXPECT_EQ(res, globalVal);
    helicsBrokerSetGlobal(brk, "testglobal2", globalVal2.c_str(), &err);
    helicsQueryFree(q);
    helicsCloseLibrary();

    EXPECT_EQ(helicsBrokerIsConnected(brk), helics_false);
    helicsBrokerDestroy(brk);
    EXPECT_EQ(helicsBrokerIsConnected(brk), helics_false);
    EXPECT_EQ(helicsBrokerIsValid(brk), helics_false);
    helicsBrokerFree(brk);
    EXPECT_EQ(helicsBrokerIsValid(brk), helics_false);
}

static std::atomic<int> handlerCount{0};

static helics_bool testHandler(int /*unused*/)
{
    ++handlerCount;
    return helics_false;
}

TEST(other_tests, signal_handler_callback)
{
    helicsLoadSignalHandlerCallback(testHandler);
    raise(SIGINT);
    EXPECT_EQ(handlerCount.load(), 1);
    helicsClearSignalHandler();
}

/** test the default signal handler*/
TEST(other_tests, signal_handler_death_ci_skip)
{
    helicsLoadSignalHandler();
    EXPECT_EXIT(raise(SIGINT), testing::ExitedWithCode(helics_error_user_abort), "");
    helicsClearSignalHandler();
}
