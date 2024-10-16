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

#include <atomic>
#include <csignal>
#include <future>
#include <string>
#include <thread>

// test generating a global from a broker and some of its error pathways
TEST(other_tests, broker_global_value)
{
    auto err = helicsErrorInitialize();
    auto brk = helicsCreateBroker("test", "gbroker", "--root", &err);
    std::string globalVal = "this is a string constant that functions as a global";
    std::string globalVal2 = "this is a second string constant that functions as a global";
    helicsBrokerSetGlobal(brk, "testglobal", globalVal.c_str(), &err);
    auto q = helicsCreateQuery("global_value", "testglobal");
    auto res = helicsQueryBrokerExecute(q, brk, &err);
    EXPECT_EQ(res, globalVal);
    helicsBrokerSetGlobal(brk, "testglobal2", globalVal2.c_str(), &err);
    helicsQueryFree(q);
    q = helicsCreateQuery("global_value", "testglobal2");
    res = helicsQueryBrokerExecute(q, brk, &err);
    EXPECT_EQ(res, globalVal2);

    helicsBrokerDisconnect(brk, &err);
    helicsQueryFree(q);
    EXPECT_EQ(helicsBrokerIsConnected(brk), HELICS_FALSE);
    helicsBrokerFree(brk);
}

TEST(other_tests, broker_global_value_errors_nosan_ci_skip)
{
    auto err = helicsErrorInitialize();
    auto brk = helicsCreateBroker("test", "gbroker2", "--root", &err);
    auto q = helicsCreateQuery("global_value", "testglobal");
    auto res = helicsQueryBrokerExecute(nullptr, brk, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_NE(std::string_view(res).find("error"), std::string_view::npos);

    res = helicsQueryBrokerExecute(q, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_NE(std::string_view(res).find("error"), std::string_view::npos);

    res = helicsQueryBrokerExecute(nullptr, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_NE(std::string_view(res).find("error"), std::string_view::npos);

    helicsBrokerSetGlobal(brk, nullptr, "v2", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsBrokerDisconnect(brk, &err);
    helicsQueryFree(q);
    EXPECT_EQ(helicsBrokerIsConnected(brk), HELICS_FALSE);
    helicsBrokerFree(brk);
}

// test global value creation from a core and its error pathways
TEST(other_tests, core_global_value)
{
    helicsCleanupLibrary();
    auto err = helicsErrorInitialize();
    auto brk = helicsCreateBroker("test", "gbrokerc", "--root", &err);

    auto cr = helicsCreateCore("test", "gcore", "--broker=gbrokerc", &err);
    EXPECT_EQ(err.error_code, 0);
    auto connected = helicsCoreConnect(cr, &err);
    EXPECT_EQ(connected, HELICS_TRUE);
    EXPECT_EQ(err.error_code, 0);
    EXPECT_EQ(helicsCoreIsConnected(cr), HELICS_TRUE);
    std::string globalVal = "this is a string constant that functions as a global";
    std::string globalVal2 = "this is a second string constant that functions as a global";
    helicsCoreSetGlobal(cr, "testglobal", globalVal.c_str(), &err);
    auto q = helicsCreateQuery("global_value", "testglobal");
    auto res = helicsQueryCoreExecute(q, cr, &err);
    EXPECT_EQ(res, globalVal);
    helicsCoreSetGlobal(cr, "testglobal2", globalVal2.c_str(), &err);
    helicsQueryFree(q);
    q = helicsCreateQuery("global_value", "testglobal2");
    res = helicsQueryCoreExecute(q, cr, &err);
    EXPECT_EQ(res, globalVal2);
    helicsBrokerDisconnect(brk, &err);
    helicsCoreDisconnect(cr, &err);

    helicsQueryFree(q);
    EXPECT_EQ(helicsBrokerIsConnected(brk), HELICS_FALSE);
}

// test global value creation from a core and its error pathways
TEST(other_tests, core_global_value_errors_nosan_ci_skip)
{
    helicsCleanupLibrary();
    auto err = helicsErrorInitialize();
    auto brk = helicsCreateBroker("test", "gbrokerce", "--root", &err);

    auto cr = helicsCreateCore("test", "gcore", "--broker=gbrokerce", &err);
    EXPECT_EQ(err.error_code, 0);
    auto connected = helicsCoreConnect(cr, &err);
    EXPECT_EQ(connected, HELICS_TRUE);
    EXPECT_EQ(err.error_code, 0);
    EXPECT_EQ(helicsCoreIsConnected(cr), HELICS_TRUE);
    auto q = helicsCreateQuery("global_value", "testglobal");

    auto res = helicsQueryCoreExecute(nullptr, cr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_NE(std::string_view(res).find("error"), std::string_view::npos);
    res = helicsQueryCoreExecute(q, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_NE(std::string_view(res).find("error"), std::string_view::npos);
    res = helicsQueryCoreExecute(nullptr, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_NE(std::string_view(res).find("error"), std::string_view::npos);
    helicsCoreSetGlobal(cr, nullptr, "v2", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    helicsBrokerDisconnect(brk, &err);
    helicsCoreDisconnect(cr, &err);

    helicsQueryFree(q);
    EXPECT_EQ(helicsBrokerIsConnected(brk), HELICS_FALSE);
}

// test global value creation from a federate and some error pathways for queries and global
// creation
TEST(other_tests, federate_global_value)
{
    auto err = helicsErrorInitialize();
    auto brk = helicsCreateBroker("test", "gbrokerf", "--root", &err);

    auto cr = helicsCreateCore("test", "gcore", "--broker=gbrokerf", &err);

    // test creation of federateInfo from command line arguments
    const char* argv[4];
    argv[0] = "";
    argv[1] = "--corename=gcore";
    argv[2] = "--coretype=test";
    argv[3] = "--period=1.0";

    auto fedInfo = helicsCreateFederateInfo();
    helicsFederateInfoLoadFromArgs(fedInfo, 4, argv, &err);
    EXPECT_EQ(err.error_code, 0);

    auto fed = helicsCreateValueFederate("fed0", fedInfo, &err);
    EXPECT_EQ(err.error_code, 0);

    helicsFederateInfoFree(fedInfo);

    std::string globalVal = "this is a string constant that functions as a global";
    std::string globalVal2 = "this is a second string constant that functions as a global";
    helicsFederateSetGlobal(fed, "testglobal", globalVal.c_str(), &err);
    auto q = helicsCreateQuery("global_value", "testglobal");
    auto res = helicsQueryExecute(q, fed, &err);
    EXPECT_EQ(res, globalVal);
    helicsFederateSetGlobal(fed, "testglobal2", globalVal2.c_str(), &err);
    helicsQueryFree(q);
    q = helicsCreateQuery("global_value", "testglobal2");
    helicsQueryExecuteAsync(q, fed, &err);
    while (helicsQueryIsCompleted(q) == HELICS_FALSE) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    res = helicsQueryExecuteComplete(q, &err);
    EXPECT_EQ(res, globalVal2);

    auto q2 = helicsCreateQuery(nullptr, "isinit");
    helicsQueryExecuteAsync(q2, fed, &err);
    while (helicsQueryIsCompleted(q2) == HELICS_FALSE) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    res = helicsQueryExecuteComplete(q2, &err);
    EXPECT_STREQ(res, "false");

    helicsFederateFinalize(fed, &err);

    helicsCoreDisconnect(cr, &err);
    helicsBrokerDisconnect(brk, &err);

    helicsQueryFree(q);
    helicsQueryFree(q2);
    EXPECT_EQ(helicsBrokerIsConnected(brk), HELICS_FALSE);
}

TEST(other_tests, federate_global_value_errors_nosan_ci_skip)
{
    auto err = helicsErrorInitialize();
    auto brk = helicsCreateBroker("test", "gbrokerfe", "--root", &err);

    auto cr = helicsCreateCore("test", "gcore", "--broker=gbrokerfe", &err);

    // test creation of federateInfo from command line arguments
    const char* argv[4];
    argv[0] = "";
    argv[1] = "--corename=gcore";
    argv[2] = "--coretype=test";
    argv[3] = "--period=1.0";

    auto fedInfo = helicsCreateFederateInfo();
    helicsFederateInfoLoadFromArgs(fedInfo, 4, argv, &err);
    EXPECT_EQ(err.error_code, 0);

    auto fed = helicsCreateValueFederate("fed0", fedInfo, &err);
    EXPECT_EQ(err.error_code, 0);
    argv[3] = "--period=frogs";  // this is meant to generate an error in command line processing

    auto fi2 = helicsFederateInfoClone(fedInfo, &err);
    EXPECT_NE(fi2, nullptr);
    helicsFederateInfoLoadFromArgs(fi2, 4, argv, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    helicsFederateInfoFree(fi2);
    helicsFederateInfoFree(fedInfo);

    auto q = helicsCreateQuery("global_value", "testglobal2");

    // a series of invalid query calls
    auto res = helicsQueryExecute(nullptr, fed, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_NE(std::string_view(res).find("error"), std::string_view::npos);

    res = helicsQueryExecute(q, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_NE(std::string_view(res).find("error"), std::string_view::npos);

    res = helicsQueryExecute(nullptr, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_NE(std::string_view(res).find("error"), std::string_view::npos);

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
    EXPECT_EQ(helicsBrokerIsConnected(brk), HELICS_FALSE);
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
    argv[2] = "--coretype=test";
    argv[3] = "--period=1.0";

    auto fedInfo = helicsCreateFederateInfo();
    helicsFederateInfoLoadFromArgs(fedInfo, 4, argv, &err);
    helicsFederateInfoSetFlagOption(fedInfo, HELICS_FLAG_SOURCE_ONLY, HELICS_TRUE, &err);

    auto fed1 = helicsCreateMessageFederate("fed1", fedInfo, &err);
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

    helicsFederateInfoFree(fedInfo);
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
    auto brk = helicsCreateBroker("test", "gbrokercn", "--root", &err);

    const char* argv[4];
    argv[0] = "";
    argv[1] = "--name=gcore";
    argv[2] = "--timeout=2000";
    argv[3] = "--broker=gbrokercn";

    auto cr = helicsCreateCoreFromArgs("test", nullptr, 4, argv, &err);
    EXPECT_EQ(err.error_code, 0);
    EXPECT_STREQ(helicsCoreGetIdentifier(cr), "gcore");

    helicsBrokerDisconnect(brk, &err);
    helicsCoreDisconnect(cr, &err);

    EXPECT_EQ(helicsBrokerIsConnected(brk), HELICS_FALSE);
}

// test core creation from command line arguments
TEST(other_tests, core_creation_error_nosan)
{
    auto err = helicsErrorInitialize();

    const char* argv[4];
    argv[0] = "";
    argv[1] = "--name=gcore2";
    argv[2] = "--log_level=what_logs?";
    argv[3] = "--broker=gbrokercm2";

    auto cr2 = helicsCreateCoreFromArgs("test", nullptr, 4, argv, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_EQ(cr2, nullptr);
}

// test broker creation from command line arguments
TEST(other_tests, broker_creation)
{
    auto err = helicsErrorInitialize();

    const char* argv[4];
    argv[0] = "";
    argv[1] = "--name=gbrokercm3";
    argv[2] = "--timeout=2000";
    argv[3] = "--root";

    auto brk = helicsCreateBrokerFromArgs("test", nullptr, 4, argv, &err);
    EXPECT_EQ(err.error_code, 0);
    EXPECT_STREQ(helicsBrokerGetIdentifier(brk), "gbrokercm3");

    helicsBrokerDisconnect(brk, &err);
}

// test broker creation error pathway
TEST(other_tests, broker_creation_nosan)
{
    auto err = helicsErrorInitialize();

    const char* argv[4];
    argv[0] = "";
    argv[1] = "--name=gbrokerAC1";
    argv[2] = "--log_level=what_logs?";
    argv[3] = "--root";

    auto brk2 = helicsCreateBrokerFromArgs("test", nullptr, 4, argv, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_EQ(brk2, nullptr);
}

TEST(federate_tests, federateGeneratedLocalError_nosan)
{
    auto fedInfo = helicsCreateFederateInfo();
    helicsFederateInfoSetCoreType(fedInfo, HELICS_CORE_TYPE_TEST, nullptr);
    helicsFederateInfoSetCoreName(fedInfo, "core_full_le", nullptr);
    helicsFederateInfoSetCoreInitString(fedInfo,
                                        "-f 1 --autobroker --broker=flebroker1 --error_timeout=0",
                                        nullptr);

    auto fed1 = helicsCreateValueFederate("fed1", fedInfo, nullptr);
    helicsFederateInfoFree(fedInfo);
    helicsFederateEnterExecutingMode(fed1, nullptr);

    helicsFederateRequestTime(fed1, 2.0, nullptr);
    helicsFederateLocalError(fed1, 9827, "user generated error", nullptr);

    auto err = helicsErrorInitialize();
    helicsFederateRequestTime(fed1, 3.0, &err);
    EXPECT_NE(err.error_code, 0);

    auto cr = helicsFederateGetCore(fed1, nullptr);
    helicsCoreDisconnect(cr, nullptr);
    helicsCoreFree(cr);
    helicsFederateDestroy(fed1);
}

TEST(federate, federateGeneratedGlobalError_nosan)
{
    auto fedInfo = helicsCreateFederateInfo();
    helicsFederateInfoSetCoreType(fedInfo, HELICS_CORE_TYPE_TEST, nullptr);
    helicsFederateInfoSetCoreName(fedInfo, "core_full_ge", nullptr);
    helicsFederateInfoSetCoreInitString(fedInfo,
                                        "-f 1 --autobroker --broker=fgebroker2 --error_timeout=0",
                                        nullptr);

    auto fed1 = helicsCreateValueFederate("fed1", fedInfo, nullptr);
    helicsFederateInfoFree(fedInfo);
    helicsFederateEnterExecutingMode(fed1, nullptr);

    helicsFederateRequestTime(fed1, 2.0, nullptr);
    helicsFederateGlobalError(fed1, 9827, "user generated global error", nullptr);

    auto err = helicsErrorInitialize();
    helicsFederateRequestTime(fed1, 3.0, &err);
    EXPECT_NE(err.error_code, 0);

    helicsFederateDestroy(fed1);
}

// test generating a global from a broker and some of its error pathways
TEST(other_tests, broker_after_close_nosan)
{
    auto err = helicsErrorInitialize();
    auto brk = helicsCreateBroker("test", "gbroker_test", "--root", &err);
    std::string globalVal = "this is a string constant that functions as a global";
    std::string globalVal2 = "this is a second string constant that functions as a global";
    helicsBrokerSetGlobal(brk, "testglobal", globalVal.c_str(), &err);
    auto q = helicsCreateQuery("global_value", "testglobal");
    auto res = helicsQueryBrokerExecute(q, brk, &err);
    EXPECT_EQ(res, globalVal);
    helicsBrokerSetGlobal(brk, "testglobal2", globalVal2.c_str(), &err);
    helicsQueryFree(q);
    helicsCloseLibrary();

    EXPECT_EQ(helicsBrokerIsConnected(brk), HELICS_FALSE);
    helicsBrokerDestroy(brk);
    EXPECT_EQ(helicsBrokerIsConnected(brk), HELICS_FALSE);
    EXPECT_EQ(helicsBrokerIsValid(brk), HELICS_FALSE);
    helicsBrokerFree(brk);
    EXPECT_EQ(helicsBrokerIsValid(brk), HELICS_FALSE);
}

static std::atomic<int> handlerCount{0};

static HelicsBool testHandlerFalse(int /*unused*/)
{
    ++handlerCount;
    return HELICS_FALSE;
}

static HelicsBool testHandlerTrue(int /*unused*/)
{
    ++handlerCount;
    return HELICS_TRUE;
}

TEST(other_tests, signal_handler_callback)
{
    handlerCount.store(0);
    helicsLoadSignalHandlerCallback(testHandlerFalse, HELICS_FALSE);
    raise(SIGINT);
    EXPECT_EQ(handlerCount.load(), 1);
    helicsClearSignalHandler();
    helicsCleanupLibrary();
    helicsCloseLibrary();
}

/** test the default signal handler*/
TEST(other_tests, signal_handler_death_ci_skip)
{
    helicsLoadSignalHandler();
    EXPECT_EXIT(raise(SIGINT), testing::ExitedWithCode(HELICS_ERROR_USER_ABORT), "");
    helicsClearSignalHandler();
    helicsCleanupLibrary();
    helicsCloseLibrary();
}

/** test the default signal handler*/
TEST(other_tests, signal_handler_threaded_death_ci_skip)
{
    helicsLoadSignalHandlerCallbackNoExit(nullptr, HELICS_TRUE);
    auto hb = helicsCreateBroker("TEST", "zbroker1", nullptr, nullptr);
    EXPECT_TRUE(helicsBrokerIsConnected(hb));
    raise(SIGINT);
    auto res = helicsBrokerWaitForDisconnect(hb, 1000, nullptr);
    if (res == HELICS_FALSE) {
        res = helicsBrokerWaitForDisconnect(hb, 1000, nullptr);
    }
    EXPECT_TRUE(res);
    helicsClearSignalHandler();
    helicsCleanupLibrary();
    helicsCloseLibrary();
}

/** test the threaded signal handler*/
TEST(other_tests, signal_handler_callback_threaded_death_ci_skip)
{
    handlerCount.store(0);
    helicsLoadSignalHandlerCallbackNoExit(testHandlerTrue, HELICS_TRUE);

    auto hb = helicsCreateBroker("TEST", "zbroker2", nullptr, nullptr);
    EXPECT_TRUE(helicsBrokerIsConnected(hb));
    raise(SIGINT);
    auto res = helicsBrokerWaitForDisconnect(hb, 1000, nullptr);
    if (res == HELICS_FALSE) {
        res = helicsBrokerWaitForDisconnect(hb, 1000, nullptr);
    }
    EXPECT_TRUE(res);
    helicsClearSignalHandler();
    EXPECT_EQ(handlerCount.load(), 1);
    helicsCleanupLibrary();
    helicsCloseLibrary();
}

/** test the threaded signal handler during disconnected fed construction*/
TEST(other_tests, signal_handler_fed_construction_death_ci_skip)
{
    handlerCount.store(0);
    helicsLoadSignalHandlerCallbackNoExit(nullptr, HELICS_TRUE);
    HelicsError err = helicsErrorInitialize();
    auto fedGen = [&err]() {
        auto fedInfo = helicsCreateFederateInfo();
        helicsFederateInfoSetCoreInitString(fedInfo, "--networktimeout=300ms", nullptr);
        auto federate = helicsCreateCombinationFederate("test", fedInfo, &err);
        helicsFederateInfoFree(fedInfo);
        helicsFederateDestroy(federate);
    };

    auto t1 = std::async(std::launch::async, fedGen);

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    helicsAbort(-44, "zippity_zoo_za");
    auto status = t1.wait_for(std::chrono::milliseconds(3000));
    EXPECT_EQ(status, std::future_status::ready);
    t1.wait();
    EXPECT_NE(err.error_code, HELICS_OK);
    EXPECT_TRUE(std::string(err.message).find("zippity_zoo_za") != std::string::npos);
    helicsClearSignalHandler();
    helicsCleanupLibrary();
    helicsCloseLibrary();
}

TEST(federate, federateNoProtection)
{
    auto fedInfo = helicsCreateFederateInfo();
    helicsFederateInfoSetCoreType(fedInfo, HELICS_CORE_TYPE_TEST, nullptr);
    helicsFederateInfoSetCoreName(fedInfo, "core_protect", nullptr);
    helicsFederateInfoSetCoreInitString(fedInfo,
                                        "-f 1 --autobroker --broker=npbroker1 --error_timeout=0",
                                        nullptr);

    auto fed1 = helicsCreateValueFederate("fed1", fedInfo, nullptr);

    helicsFederateEnterExecutingMode(fed1, nullptr);

    std::string fed1Nm = helicsFederateGetName(fed1);
    HelicsError err = helicsErrorInitialize();

    EXPECT_FALSE(helicsFederateIsProtected(fed1Nm.c_str(), &err));
    helicsFederateFree(fed1);

    auto fedFind = helicsGetFederateByName(fed1Nm.c_str(), &err);

    EXPECT_FALSE(helicsFederateIsValid(fedFind));
    EXPECT_NE(err.error_code, 0);
    helicsFederateInfoFree(fedInfo);
}

TEST(federate, federateProtection)
{
    auto fedInfo = helicsCreateFederateInfo();
    helicsFederateInfoSetCoreType(fedInfo, HELICS_CORE_TYPE_TEST, nullptr);
    helicsFederateInfoSetCoreName(fedInfo, "core_protect", nullptr);
    helicsFederateInfoSetCoreInitString(fedInfo,
                                        "-f 1 --autobroker --broker=npbroker2 --error_timeout=0",
                                        nullptr);

    auto fed1 = helicsCreateValueFederate("fed1", fedInfo, nullptr);
    helicsFederateInfoFree(fedInfo);
    helicsFederateEnterExecutingMode(fed1, nullptr);

    std::string fed1Nm = helicsFederateGetName(fed1);
    HelicsError err = helicsErrorInitialize();
    helicsFederateProtect(fed1Nm.c_str(), &err);
    helicsFederateFree(fed1);

    EXPECT_TRUE(helicsFederateIsProtected(fed1Nm.c_str(), &err));
    auto fedFind = helicsGetFederateByName(fed1Nm.c_str(), &err);

    EXPECT_TRUE(helicsFederateIsValid(fedFind));
    EXPECT_EQ(err.error_code, 0);

    helicsFederateUnProtect(fed1Nm.c_str(), &err);
    EXPECT_FALSE(helicsFederateIsProtected(fed1Nm.c_str(), &err));
    helicsFederateFinalize(fedFind, &err);
    helicsFederateFree(fedFind);
    EXPECT_EQ(err.error_code, 0);

    EXPECT_FALSE(helicsFederateIsProtected(fed1Nm.c_str(), &err));
    EXPECT_NE(err.error_code, 0);
}
