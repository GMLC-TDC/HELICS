/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "ctestFixtures.hpp"
#include "helics/helics.h"

#include <gtest/gtest.h>
#include <string>
/**
 tests of evil inputs for all HELICS API function calls*/

TEST(evil_general_test, helicsErrorInitialize)
{
    // HelicsError helicsErrorInitialize(void);
    auto err = helicsErrorInitialize();
    EXPECT_EQ(err.error_code, 0);
    EXPECT_TRUE(std::string(err.message).empty());
}

TEST(evil_general_test, helicsErrorClear)
{
    // void helicsErrorClear(HelicsError* err);
    auto err = helicsErrorInitialize();
    err.error_code = 55;
    err.message = "this is a test";

    helicsErrorClear(&err);
    EXPECT_EQ(err.error_code, 0);
    EXPECT_TRUE(std::string(err.message).empty());
}

TEST(evil_general_test, helicsIsCoreTypeAvailable)
{
    // HelicsBool helicsIsCoreTypeAvailable(const char* type);
    EXPECT_EQ(helicsIsCoreTypeAvailable(nullptr), HELICS_FALSE);
}

TEST(evil_general_test, helicsGetFederateByName)
{
    // HelicsFederate helicsGetFederateByName(const char* fedName, HelicsError* err);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res = helicsGetFederateByName(nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(helicsFederateIsValid(res), HELICS_FALSE);

    res = helicsGetFederateByName("bob", &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(helicsFederateIsValid(res), HELICS_FALSE);
    helicsErrorClear(&err);
    res = helicsGetFederateByName(nullptr, &err);
    EXPECT_EQ(helicsFederateIsValid(res), HELICS_FALSE);

    res = helicsGetFederateByName("bob", &err);
    EXPECT_EQ(helicsFederateIsValid(res), HELICS_FALSE);

    res = helicsGetFederateByName(nullptr, nullptr);
    EXPECT_EQ(helicsFederateIsValid(res), HELICS_FALSE);
}

TEST(evil_general_test, helicsGetPropertyIndex)
{
    // int helicsGetPropertyIndex(const char* val);
    auto res = helicsGetPropertyIndex(nullptr);
    EXPECT_LT(res, 0);
    res = helicsGetPropertyIndex("not_a_property");
    EXPECT_LT(res, 0);
}

TEST(evil_general_test, helicsGetOptionIndex)
{
    // int helicsGetOptionIndex(const char* val);
    auto res = helicsGetOptionIndex(nullptr);
    EXPECT_LT(res, 0);
    res = helicsGetOptionIndex("not_a_property");
    EXPECT_LT(res, 0);
}

TEST(evil_general_test, helicsCloseLibrary)
{
    // void helicsCloseLibrary(void);
    helicsCloseLibrary();
}

TEST(evil_general_test, helicsCleanupLibrary)
{
    // void helicsCleanupLibrary(void);
    helicsCleanupLibrary();
}

// section Creation Function Functions to create the different objects in the library
TEST(evil_creation_test, helicsCreateCore)
{
    // HelicsCore helicsCreateCore(const char* type, const char* name, const char* initString,
    // HelicsError* err);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsCreateCore(nullptr, "name", "", &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_TRUE(helicsCoreIsValid(res1) == HELICS_FALSE);
    helicsErrorClear(&err);
    auto res2 = helicsCreateCore("invalid", "name", "", &err);
    EXPECT_TRUE(helicsCoreIsValid(res2) == HELICS_FALSE);
    auto res3 = helicsCreateCore("invalid", "name", "", nullptr);
    EXPECT_TRUE(helicsCoreIsValid(res3) == HELICS_FALSE);
}

TEST(evil_creation_test, helicsCreateCoreFromArgs)
{
    // HelicsCore helicsCreateCoreFromArgs(const char* type, const char* name, int argc, const
    // char* const* argv, HelicsError* err); HelicsCore helicsCreateCoreFromArgs(const char* type,
    // const char* name, int argc, const char* const* argv, HelicsError* err);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsCreateCoreFromArgs("bob", "bob", 0, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_TRUE(helicsCoreIsValid(res1) == HELICS_FALSE);
    helicsErrorClear(&err);
    auto res2 = helicsCreateCoreFromArgs("bob", "bob", 0, nullptr, &err);
    EXPECT_EQ(helicsCoreIsValid(res2), HELICS_FALSE);
    auto res3 = helicsCreateCoreFromArgs("bob", "bob", 0, nullptr, nullptr);
    EXPECT_EQ(helicsCoreIsValid(res3), HELICS_FALSE);
}

TEST(evil_creation_test, helicsCreateBroker)
{
    // HelicsBroker helicsCreateBroker(const char* type, const char* name, const char* initString,
    // HelicsError* err);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsCreateBroker(nullptr, "name", "", &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_TRUE(helicsBrokerIsValid(res1) == HELICS_FALSE);
    helicsErrorClear(&err);
    auto res2 = helicsCreateBroker("invalid", "name", "", &err);
    EXPECT_TRUE(helicsBrokerIsValid(res2) == HELICS_FALSE);
    auto res3 = helicsCreateBroker("invalid", "name", "", nullptr);
    EXPECT_TRUE(helicsBrokerIsValid(res3) == HELICS_FALSE);
}

TEST(evil_creation_test, helicsCreateBrokerFromArgs)
{
    // HelicsBroker helicsCreateBrokerFromArgs(const char* type, const char* name, int argc, const
    // char* const* argv, HelicsError* err);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsCreateBrokerFromArgs("bob", "bob", 0, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_TRUE(helicsBrokerIsValid(res1) == HELICS_FALSE);
    helicsErrorClear(&err);
    auto res2 = helicsCreateBrokerFromArgs("bob", "bob", 0, nullptr, &err);
    EXPECT_EQ(helicsBrokerIsValid(res2), HELICS_FALSE);
    auto res3 = helicsCreateBrokerFromArgs("bob", "bob", 0, nullptr, nullptr);
    EXPECT_EQ(helicsBrokerIsValid(res3), HELICS_FALSE);
}

TEST(evil_creation_test, helicsCreateValueFederate)
{
    // HelicsFederate helicsCreateValueFederate(const char* fedName, HelicsFederateInfo fedInfo,
    // HelicsError* err); HelicsFederate helicsCreateValueFederate(const char* fedName,
    // HelicsFederateInfo fedInfo, HelicsError* err);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsCreateValueFederate("billy", nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(helicsFederateIsValid(res1), HELICS_FALSE);
    helicsErrorClear(&err);

    auto fedInfo = helicsCreateFederateInfo();
    helicsFederateInfoSetCoreType(fedInfo, HELICS_CORE_TYPE_NNG, nullptr);
    auto res2 = helicsCreateValueFederate("billy", fedInfo, &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(helicsFederateIsValid(res2), HELICS_FALSE);
    helicsErrorClear(&err);
    // auto res2=helicsCreateValueFederate(const char* fedName, HelicsFederateInfo fedInfo,
    // nullptr);

    char rdata[256];
    auto evil_fi = reinterpret_cast<HelicsFederateInfo>(rdata);
    auto res3 = helicsCreateValueFederate("billy", evil_fi, &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(helicsFederateIsValid(res3), HELICS_FALSE);
}

TEST(evil_creation_test, helicsCreateValueFederateFromConfig)
{
    // HelicsFederate helicsCreateValueFederateFromConfig(const char* configFile, HelicsError*
    // err);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsCreateValueFederateFromConfig("unknownfile.json", &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    EXPECT_EQ(helicsFederateIsValid(res1), HELICS_FALSE);
    auto res2 = helicsCreateValueFederateFromConfig("unknownfile.json", &err);
    EXPECT_EQ(helicsFederateIsValid(res2), HELICS_FALSE);
    auto res3 = helicsCreateValueFederateFromConfig(nullptr, nullptr);
    EXPECT_EQ(helicsFederateIsValid(res3), HELICS_FALSE);
}

TEST(evil_creation_test, helicsCreateMessageFederate)
{
    // auto res2=helicsCreateMessageFederate(const char* fedName, HelicsFederateInfo fedInfo,
    // nullptr);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsCreateMessageFederate("billy", nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(helicsFederateIsValid(res1), HELICS_FALSE);
    helicsErrorClear(&err);

    auto fedInfo = helicsCreateFederateInfo();
    helicsFederateInfoSetCoreType(fedInfo, HELICS_CORE_TYPE_NNG, nullptr);
    auto res2 = helicsCreateMessageFederate("billy", fedInfo, &err);
    EXPECT_EQ(helicsFederateIsValid(res2), HELICS_FALSE);
    helicsErrorClear(&err);
    char rdata[256];
    auto evil_fi = reinterpret_cast<HelicsFederateInfo>(rdata);
    auto res3 = helicsCreateMessageFederate("billy", evil_fi, &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(helicsFederateIsValid(res3), HELICS_FALSE);
}

TEST(evil_creation_test, helicsCreateMessageFederateFromConfig)
{
    // HelicsFederate helicsCreateMessageFederateFromConfig(const char* configFile, HelicsError*
    // err);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsCreateMessageFederateFromConfig("unknownfile.json", &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    EXPECT_EQ(helicsFederateIsValid(res1), HELICS_FALSE);
    auto res2 = helicsCreateMessageFederateFromConfig("unknownfile.json", &err);
    EXPECT_EQ(helicsFederateIsValid(res2), HELICS_FALSE);
    auto res3 = helicsCreateMessageFederateFromConfig(nullptr, nullptr);
    EXPECT_EQ(helicsFederateIsValid(res3), HELICS_FALSE);
}

TEST(evil_creation_test, helicsCreateCombinationFederate)
{
    // HelicsFederate helicsCreateCombinationFederate(const char* fedName, HelicsFederateInfo
    // fedInfo, HelicsError* err);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsCreateCombinationFederate("billy", nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(helicsFederateIsValid(res1), HELICS_FALSE);
    helicsErrorClear(&err);

    auto fedInfo = helicsCreateFederateInfo();
    helicsFederateInfoSetCoreType(fedInfo, HELICS_CORE_TYPE_NNG, nullptr);
    auto res2 = helicsCreateCombinationFederate("billy", fedInfo, &err);
    EXPECT_EQ(helicsFederateIsValid(res2), HELICS_FALSE);
    helicsErrorClear(&err);
    char rdata[256];
    auto evil_fi = reinterpret_cast<HelicsFederateInfo>(rdata);
    auto res3 = helicsCreateCombinationFederate("billy", evil_fi, &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(helicsFederateIsValid(res3), HELICS_FALSE);
}

TEST(evil_creation_test, helicsCreateCombinationFederateFromConfig)
{
    // HelicsFederate helicsCreateCombinationFederateFromConfig(const char* configFile,
    // HelicsError* err);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsCreateCombinationFederateFromConfig("unknownfile.json", &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    EXPECT_EQ(helicsFederateIsValid(res1), HELICS_FALSE);
    auto res2 = helicsCreateCombinationFederateFromConfig("unknownfile.json", &err);
    EXPECT_EQ(helicsFederateIsValid(res2), HELICS_FALSE);
    auto res3 = helicsCreateCombinationFederateFromConfig(nullptr, nullptr);
    EXPECT_EQ(helicsFederateIsValid(res3), HELICS_FALSE);
}

TEST(evil_creation_test, helicsCreateFederateInfo)
{
    // HelicsFederateInfo helicsCreateFederateInfo(void);
    auto fedInfo = helicsCreateFederateInfo();
    EXPECT_NE(fedInfo, nullptr);
}

TEST(evil_creation_test, helicsCreateQuery)
{
    // HelicsQuery helicsCreateQuery(const char* target, const char* query);
    auto query = helicsCreateQuery(nullptr, nullptr);
    EXPECT_NE(query, nullptr);
}

// section Core Functions
// functions applying to a \ref HelicsCore object
TEST(evil_core_test, helicsCoreClone)
{
    // HelicsCore helicsCoreClone(HelicsCore core, HelicsError* err);
    char rdata[256];
    auto evil_core = reinterpret_cast<HelicsCore>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsCoreClone(nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_NE(helicsCoreIsValid(res1), HELICS_TRUE);
    helicsErrorClear(&err);
    auto res2 = helicsCoreClone(evil_core, nullptr);
    EXPECT_NE(helicsCoreIsValid(res2), HELICS_TRUE);
    auto res3 = helicsCoreClone(evil_core, &err);
    EXPECT_NE(helicsCoreIsValid(res3), HELICS_TRUE);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_core_test, helicsCoreIsValid)
{
    // HelicsBool helicsCoreIsValid(HelicsCore core);
    char rdata[256];
    auto evil_core = reinterpret_cast<HelicsCore>(rdata);
    EXPECT_NE(helicsCoreIsValid(evil_core), HELICS_TRUE);
    EXPECT_NE(helicsCoreIsValid(nullptr), HELICS_TRUE);
}

TEST(evil_core_test, helicsCoreWaitForDisconnect)
{
    // HelicsBool helicsCoreWaitForDisconnect(HelicsCore core, int msToWait, HelicsError* err);
    char rdata[256];
    auto evil_core = reinterpret_cast<HelicsCore>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsCoreWaitForDisconnect(nullptr, 1, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, HELICS_TRUE);
    helicsErrorClear(&err);
    auto res2 = helicsCoreWaitForDisconnect(nullptr, 1, nullptr);
    EXPECT_EQ(res2, HELICS_TRUE);
    auto res3 = helicsCoreWaitForDisconnect(evil_core, 1, &err);
    EXPECT_EQ(res3, HELICS_TRUE);
}

TEST(evil_core_test, helicsCoreIsConnected)
{
    // HelicsBool helicsCoreIsConnected(HelicsCore core);
    char rdata[256];
    auto evil_core = reinterpret_cast<HelicsCore>(rdata);
    auto res1 = helicsCoreIsConnected(evil_core);
    EXPECT_EQ(res1, HELICS_FALSE);
    auto res2 = helicsCoreIsConnected(nullptr);
    EXPECT_EQ(res2, HELICS_FALSE);
}

TEST(evil_core_test, helicsCoreDataLink)
{
    // void helicsCoreDataLink(HelicsCore core, const char* source, const char* target,
    // HelicsError* err);
    char rdata[256];
    auto evil_core = reinterpret_cast<HelicsCore>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsCoreDataLink(nullptr, nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsCoreDataLink(HelicsCore core, const char* source, const char* target,
    // nullptr);
    helicsCoreDataLink(evil_core, "source", "target", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_core_test, helicsCoreAddSourceFilterToEndpoint)
{
    // void helicsCoreAddSourceFilterToEndpoint(HelicsCore core, const char* filter, const char*
    // endpoint, HelicsError* err);
    char rdata[256];
    auto evil_core = reinterpret_cast<HelicsCore>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsCoreAddSourceFilterToEndpoint(nullptr, "filter", "ept", &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsCoreAddSourceFilterToEndpoint(HelicsCore core, const char* filter, const
    // char* endpoint, nullptr);
    helicsCoreAddSourceFilterToEndpoint(evil_core, "filter", "ept", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_core_test, helicsCoreAddDestinationFilterToEndpoint)
{
    // void helicsCoreAddDestinationFilterToEndpoint(HelicsCore core, const char* filter, const
    // char* endpoint, HelicsError* err);
    char rdata[256];
    auto evil_core = reinterpret_cast<HelicsCore>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsCoreAddDestinationFilterToEndpoint(nullptr, "filter", "ept", &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsCoreAddDestinationFilterToEndpoint(HelicsCore core, const char* filter,
    // const char* endpoint, nullptr);
    helicsCoreAddDestinationFilterToEndpoint(evil_core, "filter", "ept", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_core_test, helicsCoreMakeConnections)
{
    // void helicsCoreMakeConnections(HelicsCore core, const char* file, HelicsError* err);
    char rdata[256];
    auto evil_core = reinterpret_cast<HelicsCore>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsCoreMakeConnections(nullptr, "invalidfile.json", &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsCoreMakeConnections(HelicsCore core, const char* file, nullptr);
    helicsCoreMakeConnections(evil_core, "invalidfile", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_core_test, helicsCoreGetIdentifier)
{
    // const char*  helicsCoreGetIdentifier(HelicsCore core);
    char rdata[256];
    auto evil_core = reinterpret_cast<HelicsCore>(rdata);
    auto res1 = helicsCoreGetIdentifier(evil_core);
    EXPECT_STREQ(res1, "");
}

TEST(evil_core_test, helicsCoreGetAddress)
{
    // const char*  helicsCoreGetAddress(HelicsCore core);
    char rdata[256];
    auto evil_core = reinterpret_cast<HelicsCore>(rdata);
    auto res1 = helicsCoreGetAddress(evil_core);
    EXPECT_STREQ(res1, "");
}

TEST(evil_core_test, helicsCoreSetReadyToInit)
{
    // void helicsCoreSetReadyToInit(HelicsCore core, HelicsError* err);
    char rdata[256];
    auto evil_core = reinterpret_cast<HelicsCore>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsCoreSetReadyToInit(nullptr, &err);
    helicsErrorClear(&err);
    // auto res2=helicsCoreSetReadyToInit(HelicsCore core, nullptr);
    helicsCoreSetReadyToInit(evil_core, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_core_test, helicsCoreConnect)
{
    // void helicsCoreConnect(HelicsCore core, HelicsError* err);
    char rdata[256];
    auto evil_core = reinterpret_cast<HelicsCore>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res = helicsCoreConnect(nullptr, &err);
    helicsErrorClear(&err);
    EXPECT_EQ(res, HELICS_FALSE);
    res = helicsCoreConnect(nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    EXPECT_EQ(res, HELICS_FALSE);
    res = helicsCoreConnect(evil_core, &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(res, HELICS_FALSE);
}

TEST(evil_core_test, helicsCoreDisconnect)
{
    // void helicsCoreDisconnect(HelicsCore core, HelicsError* err);
    char rdata[256];
    auto evil_core = reinterpret_cast<HelicsCore>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsCoreDisconnect(nullptr, &err);
    helicsErrorClear(&err);
    helicsCoreDisconnect(nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    helicsCoreDisconnect(evil_core, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_core_test, helicsCoreDestroy)
{
    // void helicsCoreDestroy(HelicsCore core);
    char rdata[256];
    auto evil_core = reinterpret_cast<HelicsCore>(rdata);
    EXPECT_NO_THROW(helicsCoreDestroy(evil_core));
    EXPECT_NO_THROW(helicsCoreFree(nullptr));
}

TEST(evil_core_test, helicsCoreFree)
{
    // void helicsCoreFree(HelicsCore core);
    char rdata[256];
    auto evil_core = reinterpret_cast<HelicsCore>(rdata);
    EXPECT_NO_THROW(helicsCoreFree(evil_core));
    EXPECT_NO_THROW(helicsCoreFree(nullptr));
}

TEST(evil_core_test, helicsCoreSetGlobal)
{
    // void helicsCoreSetGlobal(HelicsCore core, const char* valueName, const char* value,
    // HelicsError* err);
    char rdata[256];
    auto evil_core = reinterpret_cast<HelicsCore>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsCoreSetGlobal(nullptr, "value", "value", &err);
    helicsErrorClear(&err);
    // auto res2=helicsCoreSetGlobal(HelicsCore core, const char* valueName, const char* value,
    // nullptr);
    helicsCoreSetGlobal(evil_core, "value", "value", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_core_test, helicsCoreSetLogFile)
{
    // void helicsCoreSetLogFile(HelicsCore core, const char* logFileName, HelicsError* err);
    char rdata[256];
    auto evil_core = reinterpret_cast<HelicsCore>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsCoreSetLogFile(nullptr, "unknownfile.log", &err);
    helicsErrorClear(&err);
    // auto res2=helicsCoreSetLogFile(HelicsCore core, const char* logFileName, nullptr);
    helicsCoreSetLogFile(evil_core, "unknownfile.log", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_core_test, helicsCoreSetLoggingCallback)
{
    // void helicsCoreSetLoggingCallback(     HelicsCore core,     void (*logger)(int loglevel,
    // const char* identifier, const char* message, void* userData),     void* userdata,
    // HelicsError* err);
    char rdata[256];
    auto evil_core = reinterpret_cast<HelicsCore>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsCoreSetLoggingCallback(nullptr, nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    EXPECT_EQ(err.error_code, 0);
    // auto res2=helicsCoreSetLoggingCallback(     HelicsCore core,     void (*logger)(int
    // loglevel, const char* identifier, const char* message, void* userData),     void* userdata,
    // nullptr);
    helicsCoreSetLoggingCallback(evil_core, nullptr, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_core_test, helicsCoreRegisterFilter)
{
    // HelicsFilter helicsCoreRegisterFilter(HelicsCore core, HelicsFilterTypes type, const char*
    // name, HelicsError* err);
    char rdata[256];
    auto evil_core = reinterpret_cast<HelicsCore>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsCoreRegisterFilter(nullptr, HELICS_FILTER_TYPE_DELAY, "delay", &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, nullptr);
    helicsErrorClear(&err);
    auto res2 = helicsCoreRegisterFilter(nullptr, HELICS_FILTER_TYPE_DELAY, "delay", nullptr);
    EXPECT_EQ(res2, nullptr);
    auto res3 = helicsCoreRegisterFilter(evil_core, HELICS_FILTER_TYPE_DELAY, "delay", &err);
    EXPECT_EQ(res3, nullptr);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_core_test, helicsCoreRegisterCloningFilter)
{
    // HelicsFilter helicsCoreRegisterCloningFilter(HelicsCore core, const char* deliveryEndpoint,
    // HelicsError* err);
    char rdata[256];
    auto evil_core = reinterpret_cast<HelicsCore>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsCoreRegisterCloningFilter(nullptr, "delivery", &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, nullptr);
    helicsErrorClear(&err);
    auto res2 = helicsCoreRegisterCloningFilter(nullptr, "delivery", nullptr);
    EXPECT_EQ(res2, nullptr);
    auto res3 = helicsCoreRegisterCloningFilter(evil_core, "delivery", &err);
    EXPECT_EQ(res3, nullptr);
    EXPECT_NE(err.error_code, 0);
}

// section Broker Functions
// Functions applying to a \ref HelicsBroker object
TEST(evil_broker_test, helicsBrokerClone)
{
    // HelicsBroker helicsBrokerClone(HelicsBroker broker, HelicsError* err);
    char rdata[256];
    auto evil_broker = reinterpret_cast<HelicsBroker>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsBrokerClone(nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_NE(helicsBrokerIsValid(res1), HELICS_TRUE);
    helicsErrorClear(&err);
    auto res2 = helicsBrokerClone(evil_broker, nullptr);
    EXPECT_NE(helicsBrokerIsValid(res2), HELICS_TRUE);
    auto res3 = helicsBrokerClone(evil_broker, &err);
    EXPECT_NE(helicsBrokerIsValid(res3), HELICS_TRUE);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_broker_test, helicsBrokerIsValid)
{
    // HelicsBool helicsBrokerIsValid(HelicsBroker broker);
    char rdata[256];
    auto evil_broker = reinterpret_cast<HelicsBroker>(rdata);
    EXPECT_NE(helicsBrokerIsValid(evil_broker), HELICS_TRUE);
    EXPECT_NE(helicsBrokerIsValid(nullptr), HELICS_TRUE);
}

TEST(evil_broker_test, helicsBrokerIsConnected)
{
    // HelicsBool helicsBrokerIsConnected(HelicsBroker broker);
    char rdata[256];
    auto evil_broker = reinterpret_cast<HelicsBroker>(rdata);
    auto res1 = helicsBrokerIsConnected(evil_broker);
    EXPECT_EQ(res1, HELICS_FALSE);
    auto res2 = helicsBrokerIsConnected(nullptr);
    EXPECT_EQ(res2, HELICS_FALSE);
}

TEST(evil_broker_test, helicsBrokerDataLink)
{
    // void helicsBrokerDataLink(HelicsBroker broker, const char* source, const char* target,
    // HelicsError* err);
    char rdata[256];
    auto evil_broker = reinterpret_cast<HelicsBroker>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsBrokerDataLink(nullptr, nullptr, nullptr, &err);
    helicsErrorClear(&err);
    // auto res2=helicsBrokerDataLink(HelicsCore core, const char* source, const char* target,
    // nullptr);
    helicsBrokerDataLink(evil_broker, "source", "target", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_broker_test, helicsBrokerAddSourceFilterToEndpoint)
{
    // void helicsBrokerAddSourceFilterToEndpoint(HelicsBroker broker, const char* filter, const
    // char* endpoint, HelicsError* err);
    char rdata[256];
    auto evil_broker = reinterpret_cast<HelicsBroker>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsBrokerAddSourceFilterToEndpoint(nullptr, "filter", "ept", &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsBrokerAddSourceFilterToEndpoint(HelicsCore core, const char* filter, const
    // char* endpoint, nullptr);
    helicsBrokerAddSourceFilterToEndpoint(evil_broker, "filter", "ept", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_broker_test, helicsBrokerAddDestinationFilterToEndpoint)
{
    // void helicsBrokerAddDestinationFilterToEndpoint(HelicsBroker broker, const char* filter,
    // const char* endpoint, HelicsError* err);
    char rdata[256];
    auto evil_broker = reinterpret_cast<HelicsBroker>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsBrokerAddDestinationFilterToEndpoint(nullptr, "filter", "ept", &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsBrokerAddDestinationFilterToEndpoint(HelicsCore core, const char* filter,
    // const char* endpoint, nullptr);
    helicsBrokerAddDestinationFilterToEndpoint(evil_broker, "filter", "ept", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_broker_test, helicsBrokerMakeConnections)
{
    // void helicsBrokerMakeConnections(HelicsBroker broker, const char* file, HelicsError* err);
    char rdata[256];
    auto evil_broker = reinterpret_cast<HelicsBroker>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsBrokerMakeConnections(nullptr, "invalidfile.json", &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsBrokerMakeConnections(HelicsCore core, const char* file, nullptr);
    helicsBrokerMakeConnections(evil_broker, "invalidfile", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_broker_test, helicsBrokerWaitForDisconnect)
{
    // HelicsBool helicsBrokerWaitForDisconnect(HelicsBroker broker, int msToWait, HelicsError*
    // err);
    char rdata[256];
    auto evil_broker = reinterpret_cast<HelicsBroker>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsBrokerWaitForDisconnect(nullptr, 1, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, HELICS_TRUE);
    helicsErrorClear(&err);
    auto res2 = helicsBrokerWaitForDisconnect(nullptr, 1, nullptr);
    EXPECT_EQ(res2, HELICS_TRUE);
    auto res3 = helicsBrokerWaitForDisconnect(evil_broker, 1, &err);
    EXPECT_EQ(res3, HELICS_TRUE);
}

TEST(evil_broker_test, helicsBrokerGetIdentifier)
{
    // const char*  helicsBrokerGetIdentifier(HelicsBroker broker);
    char rdata[256];
    auto evil_broker = reinterpret_cast<HelicsBroker>(rdata);
    auto res1 = helicsBrokerGetIdentifier(evil_broker);
    EXPECT_STREQ(res1, "");
}

TEST(evil_broker_test, helicsBrokerGetAddress)
{
    // const char*  helicsBrokerGetAddress(HelicsBroker broker);
    char rdata[256];
    auto evil_broker = reinterpret_cast<HelicsBroker>(rdata);
    auto res1 = helicsBrokerGetAddress(evil_broker);
    EXPECT_STREQ(res1, "");
}

TEST(evil_broker_test, helicsBrokerDisconnect)
{
    // void helicsBrokerDisconnect(HelicsBroker broker, HelicsError* err);
    char rdata[256];
    auto evil_broker = reinterpret_cast<HelicsBroker>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsBrokerDisconnect(nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    helicsBrokerDisconnect(nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    // auto res2=helicsBrokerDisconnect(HelicsCore core, nullptr);
    helicsBrokerDisconnect(evil_broker, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_broker_test, helicsBrokerDestroy)
{
    // void helicsBrokerDestroy(HelicsBroker broker);
    char rdata[256];
    auto evil_broker = reinterpret_cast<HelicsBroker>(rdata);
    EXPECT_NO_THROW(helicsBrokerDestroy(evil_broker));
    EXPECT_NO_THROW(helicsBrokerFree(nullptr));
}

TEST(evil_broker_test, helicsBrokerFree)
{
    // void helicsBrokerFree(HelicsBroker broker);
    char rdata[256];
    auto evil_broker = reinterpret_cast<HelicsBroker>(rdata);
    EXPECT_NO_THROW(helicsBrokerFree(evil_broker));
    EXPECT_NO_THROW(helicsBrokerFree(nullptr));
}

TEST(evil_broker_test, helicsBrokerSetGlobal)
{
    // void helicsBrokerSetGlobal(HelicsBroker broker, const char* valueName, const char* value,
    // HelicsError* err);
    char rdata[256];
    auto evil_broker = reinterpret_cast<HelicsBroker>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsBrokerSetGlobal(nullptr, "value", "value", &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsBrokerSetGlobal(HelicsCore core, const char* valueName, const char* value,
    // nullptr);
    helicsBrokerSetGlobal(evil_broker, "value", "value", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_broker_test, helicsBrokerSetLogFile)
{
    // void helicsBrokerSetLogFile(HelicsBroker broker, const char* logFileName, HelicsError*
    // err);
    char rdata[256];
    auto evil_broker = reinterpret_cast<HelicsBroker>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsBrokerSetLogFile(nullptr, "unknownfile.log", &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsBrokerSetLogFile(HelicsCore core, const char* logFileName, nullptr);
    helicsBrokerSetLogFile(evil_broker, "unknownfile.log", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_broker_test, helicsBrokerSetLoggingCallback)
{
    // void helicsBrokerSetLoggingCallback(     HelicsBroker broker,     void (*logger)(int
    // loglevel, const char* identifier, const char* message, void* userData),     void* userdata,
    // HelicsError* err);
    char rdata[256];
    auto evil_broker = reinterpret_cast<HelicsBroker>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsBrokerSetLoggingCallback(nullptr, nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    EXPECT_EQ(err.error_code, 0);
    // auto res2=helicsBrokerSetLoggingCallback(     HelicsCore core,     void (*logger)(int
    // loglevel, const char* identifier, const char* message, void* userData),     void* userdata,
    // nullptr);
    helicsBrokerSetLoggingCallback(evil_broker, nullptr, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
}

// section Federate Info Functions
// Functions applying to a \ref HelicsFederateInfo object
TEST(evil_fedInfo_test, helicsFederateInfoClone)
{
    char rdata[256];
    auto evil_fi = reinterpret_cast<HelicsFederateInfo>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res = helicsFederateInfoClone(nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res, nullptr);
    helicsErrorClear(&err);
    auto res2 = helicsFederateInfoClone(evil_fi, &err);
    EXPECT_EQ(res2, nullptr);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_fedInfo_test, helicsFederateInfoLoadFromArgs)
{
    char rdata[256];
    auto evil_fi = reinterpret_cast<HelicsFederateInfo>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateInfoLoadFromArgs(nullptr, 0, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    helicsFederateInfoLoadFromArgs(evil_fi, 0, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_fedInfo_test, helicsFederateInfoFree)
{
    char rdata[256];
    auto evil_fi = reinterpret_cast<HelicsFederateInfo>(rdata);
    EXPECT_NO_THROW(helicsFederateInfoFree(nullptr));
    EXPECT_NO_THROW(helicsFederateInfoFree(evil_fi));
}

TEST(evil_fedInfo_test, helicsFederateInfoSetCoreName)
{
    char rdata[256];
    auto evil_fi = reinterpret_cast<HelicsFederateInfo>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateInfoSetCoreName(nullptr, "core", &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    helicsFederateInfoSetCoreName(evil_fi, "core", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_fedInfo_test, helicsFederateInfoSetCoreInitString)
{
    char rdata[256];
    auto evil_fi = reinterpret_cast<HelicsFederateInfo>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateInfoSetCoreInitString(nullptr, "", &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    helicsFederateInfoSetCoreInitString(evil_fi, "", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_fedInfo_test, helicsFederateInfoSetBrokerInitString)
{
    char rdata[256];
    auto evil_fi = reinterpret_cast<HelicsFederateInfo>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateInfoSetBrokerInitString(nullptr, "", &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    helicsFederateInfoSetBrokerInitString(evil_fi, "", &err);
    EXPECT_NE(err.error_code, 0);
    auto fedInfo = helicsCreateFederateInfo();
    helicsErrorClear(&err);
    helicsFederateInfoSetBrokerInitString(fedInfo, nullptr, &err);
    EXPECT_EQ(err.error_code, 0);
}

TEST(evil_fedInfo_test, helicsFederateInfoSetCoreType)
{
    char rdata[256];
    auto evil_fi = reinterpret_cast<HelicsFederateInfo>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateInfoSetCoreType(nullptr, -97, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    helicsFederateInfoSetCoreType(evil_fi, -97, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_fedInfo_test, helicsFederateInfoSetCoreTypeFromString)
{
    char rdata[256];
    auto evil_fi = reinterpret_cast<HelicsFederateInfo>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateInfoSetCoreTypeFromString(nullptr, "null", &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    helicsFederateInfoSetCoreTypeFromString(evil_fi, "nullcore", &err);
    EXPECT_NE(err.error_code, 0);
    auto fedInfo = helicsCreateFederateInfo();
    helicsErrorClear(&err);
    helicsFederateInfoSetCoreTypeFromString(fedInfo, nullptr, &err);
    EXPECT_EQ(err.error_code, 0);

    helicsFederateInfoSetCoreTypeFromString(fedInfo, "evil_core", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_fedInfo_test, helicsFederateInfoSetBroker)
{
    char rdata[256];
    auto evil_fi = reinterpret_cast<HelicsFederateInfo>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateInfoSetBroker(nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    helicsFederateInfoSetBroker(evil_fi, "10.0.0.1", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_fedInfo_test, helicsFederateInfoSetBrokerKey)
{
    char rdata[256];
    auto evil_fi = reinterpret_cast<HelicsFederateInfo>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateInfoSetBrokerKey(nullptr, "broker_key", &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    helicsFederateInfoSetBrokerKey(evil_fi, "broker_key", &err);
    EXPECT_NE(err.error_code, 0);

    auto fedInfo = helicsCreateFederateInfo();
    helicsErrorClear(&err);
    helicsFederateInfoSetBrokerKey(fedInfo, nullptr, &err);
    EXPECT_EQ(err.error_code, 0);
}

TEST(evil_fedInfo_test, helicsFederateInfoSetBrokerPort)
{
    char rdata[256];
    auto evil_fi = reinterpret_cast<HelicsFederateInfo>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateInfoSetBrokerPort(nullptr, 9999, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    helicsFederateInfoSetBrokerPort(nullptr, 9999, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    helicsFederateInfoSetBrokerPort(evil_fi, 9999, &err);
    EXPECT_NE(err.error_code, 0);

    auto fedInfo = helicsCreateFederateInfo();
    helicsErrorClear(&err);
    helicsFederateInfoSetBrokerPort(fedInfo, 9999, &err);
    EXPECT_EQ(err.error_code, 0);
}

TEST(evil_fedInfo_test, helicsFederateInfoSetLocalPort)
{
    char rdata[256];
    auto evil_fi = reinterpret_cast<HelicsFederateInfo>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateInfoSetLocalPort(nullptr, "9999", &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    helicsFederateInfoSetLocalPort(nullptr, "9999", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    helicsFederateInfoSetLocalPort(evil_fi, "9999", &err);
    EXPECT_NE(err.error_code, 0);

    auto fedInfo = helicsCreateFederateInfo();
    helicsErrorClear(&err);
    helicsFederateInfoSetLocalPort(fedInfo, nullptr, &err);
    EXPECT_EQ(err.error_code, 0);
}

TEST(evil_fedInfo_test, helicsFederateInfoSetFlagOption)
{
    char rdata[256];
    auto evil_fi = reinterpret_cast<HelicsFederateInfo>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateInfoSetFlagOption(nullptr, 9, HELICS_FALSE, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    helicsFederateInfoSetFlagOption(evil_fi, 9, HELICS_FALSE, &err);
    EXPECT_NE(err.error_code, 0);

    auto fedInfo = helicsCreateFederateInfo();
    helicsErrorClear(&err);
    helicsFederateInfoSetFlagOption(fedInfo, 0, HELICS_FALSE, &err);
    EXPECT_EQ(err.error_code, 0);
}

TEST(evil_fedInfo_test, helicsFederateInfoSetSeparator)
{
    char rdata[256];
    auto evil_fi = reinterpret_cast<HelicsFederateInfo>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateInfoSetSeparator(nullptr, '-', &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    helicsFederateInfoSetSeparator(evil_fi, '-', &err);
    EXPECT_NE(err.error_code, 0);

    auto fedInfo = helicsCreateFederateInfo();
    helicsErrorClear(&err);
    helicsFederateInfoSetSeparator(fedInfo, '&', &err);
    EXPECT_EQ(err.error_code, 0);
}

TEST(evil_fedInfo_test, helicsFederateInfoSetTimeProperty)
{
    char rdata[256];
    auto evil_fi = reinterpret_cast<HelicsFederateInfo>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateInfoSetTimeProperty(nullptr, 99, 4.35, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    helicsFederateInfoSetTimeProperty(evil_fi, 99, 4.35, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_fedInfo_test, helicsFederateInfoSetIntegerProperty)
{
    char rdata[256];
    auto evil_fi = reinterpret_cast<HelicsFederateInfo>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateInfoSetIntegerProperty(nullptr, 987, -54, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    helicsFederateInfoSetIntegerProperty(evil_fi, 987, -54, &err);
    EXPECT_NE(err.error_code, 0);
}

// section Federate Functions
// Functions applying to all \ref HelicsFederate objects
TEST(evil_federate_test, helicsFederateDestroy)
{
    // void helicsFederateDestroy(HelicsFederate fed);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    EXPECT_NO_THROW(helicsFederateDestroy(evil_federate));
    EXPECT_NO_THROW(helicsFederateDestroy(nullptr));
}

TEST(evil_federate_test, helicsFederateClone)
{
    // HelicsFederate helicsFederateClone(HelicsFederate fed, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateClone(nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, nullptr);
    helicsErrorClear(&err);
    auto res2 = helicsFederateClone(nullptr, nullptr);
    EXPECT_EQ(res2, nullptr);
    helicsErrorClear(&err);
    auto res3 = helicsFederateClone(evil_federate, &err);
    EXPECT_EQ(res3, nullptr);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateIsValid)
{
    // HelicsBool helicsFederateIsValid(HelicsFederate fed);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    EXPECT_EQ(helicsFederateIsValid(evil_federate), HELICS_FALSE);
    EXPECT_EQ(helicsFederateIsValid(nullptr), HELICS_FALSE);
}

TEST(evil_federate_test, helicsFederateRegisterInterfaces)
{
    // void helicsFederateRegisterInterfaces(HelicsFederate fed, const char* file, HelicsError*
    // err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateRegisterInterfaces(nullptr, "invalidfile.txt", &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFederateRegisterInterfaces(HelicsFederate fed, const char* file, nullptr);
    helicsFederateRegisterInterfaces(evil_federate, "invalid_file.txt", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateLocalError)
{
    // void helicsFederateLocalError(HelicsFederate fed, int errorCode, const char
    // *error_message);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);

    EXPECT_NO_THROW(helicsFederateLocalError(nullptr, 4, nullptr, nullptr));

    EXPECT_NO_THROW(helicsFederateLocalError(evil_federate, -25, "error_message", nullptr));
}

TEST(evil_federate_test, helicsFederateGlobalError)
{
    // void helicsFederateLocalError(HelicsFederate fed, int errorCode, const char
    // *error_message);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);

    EXPECT_NO_THROW(helicsFederateGlobalError(nullptr, 4, nullptr, nullptr));

    EXPECT_NO_THROW(helicsFederateGlobalError(evil_federate, -25, "error_message", nullptr));
}

TEST(evil_federate_test, helicsFederateFinalize)
{
    // void helicsFederateFinalize(HelicsFederate fed, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateFinalize(nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    helicsFederateFinalize(evil_federate, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    helicsFederateFinalize(nullptr, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateFinalizeAsync)
{
    // void helicsFederateFinalizeAsync(HelicsFederate fed, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateFinalizeAsync(nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    helicsFederateFinalizeAsync(evil_federate, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateFinalizeComplete)
{
    // void helicsFederateFinalizeComplete(HelicsFederate fed, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateFinalizeComplete(nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFederateFinalizeComplete(HelicsFederate fed, nullptr);
    helicsFederateFinalizeComplete(evil_federate, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateFree)
{
    // void helicsFederateFree(HelicsFederate fed);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    EXPECT_NO_THROW(helicsFederateFree(evil_federate));
    EXPECT_NO_THROW(helicsFederateFree(nullptr));
}

TEST(evil_federate_test, helicsFederateEnterInitializingMode)
{
    // void helicsFederateEnterInitializingMode(HelicsFederate fed, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateEnterInitializingMode(nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFederateEnterInitializingMode(HelicsFederate fed, nullptr);
    helicsFederateEnterInitializingMode(evil_federate, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateEnterInitializingModeAsync)
{
    // void helicsFederateEnterInitializingModeAsync(HelicsFederate fed, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateEnterInitializingModeAsync(nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFederateEnterInitializingModeAsync(HelicsFederate fed, nullptr);
    helicsFederateEnterInitializingModeAsync(evil_federate, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateIsAsyncOperationCompleted)
{
    // HelicsBool helicsFederateIsAsyncOperationCompleted(HelicsFederate fed, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateIsAsyncOperationCompleted(nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, HELICS_FALSE);
    helicsErrorClear(&err);
    auto res2 = helicsFederateIsAsyncOperationCompleted(nullptr, nullptr);
    EXPECT_EQ(res2, HELICS_FALSE);
    auto res3 = helicsFederateIsAsyncOperationCompleted(evil_federate, &err);
    EXPECT_EQ(res3, HELICS_FALSE);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateEnterInitializingModeComplete)
{
    // void helicsFederateEnterInitializingModeComplete(HelicsFederate fed, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateEnterInitializingModeComplete(nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFederateEnterInitializingModeComplete(HelicsFederate fed, nullptr);
    helicsFederateEnterInitializingModeComplete(evil_federate, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateEnterExecutingMode)
{
    // void helicsFederateEnterExecutingMode(HelicsFederate fed, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateEnterExecutingMode(nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFederateEnterExecutingMode(HelicsFederate fed, nullptr);
    helicsFederateEnterExecutingMode(evil_federate, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateEnterExecutingModeAsync)
{
    // void helicsFederateEnterExecutingModeAsync(HelicsFederate fed, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateEnterExecutingModeAsync(nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFederateEnterExecutingModeAsync(HelicsFederate fed, nullptr);
    helicsFederateEnterExecutingModeAsync(evil_federate, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateEnterExecutingModeComplete)
{
    // void helicsFederateEnterExecutingModeComplete(HelicsFederate fed, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateEnterExecutingModeComplete(nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFederateEnterExecutingModeComplete(HelicsFederate fed, nullptr);
    helicsFederateEnterExecutingModeComplete(evil_federate, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateEnterExecutingModeIterative)
{
    // HelicsIterationResult helicsFederateEnterExecutingModeIterative(HelicsFederate fed,
    // HelicsIterationRequest iterate, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateEnterExecutingModeIterative(nullptr,
                                                          HELICS_ITERATION_REQUEST_NO_ITERATION,
                                                          &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, HELICS_ITERATION_RESULT_ERROR);
    helicsErrorClear(&err);
    auto res2 = helicsFederateEnterExecutingModeIterative(nullptr,
                                                          HELICS_ITERATION_REQUEST_NO_ITERATION,
                                                          nullptr);
    EXPECT_EQ(res2, HELICS_ITERATION_RESULT_ERROR);
    auto res3 = helicsFederateEnterExecutingModeIterative(evil_federate,
                                                          HELICS_ITERATION_REQUEST_NO_ITERATION,
                                                          &err);
    EXPECT_EQ(res3, HELICS_ITERATION_RESULT_ERROR);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateEnterExecutingModeIterativeAsync)
{
    // void helicsFederateEnterExecutingModeIterativeAsync(HelicsFederate fed,
    // HelicsIterationRequest iterate, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateEnterExecutingModeIterativeAsync(nullptr,
                                                   HELICS_ITERATION_REQUEST_NO_ITERATION,
                                                   &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFederateEnterExecutingModeIterativeAsync(HelicsFederate fed,
    // HelicsIterationRequest iterate, nullptr);
    helicsFederateEnterExecutingModeIterativeAsync(evil_federate,
                                                   HELICS_ITERATION_REQUEST_NO_ITERATION,
                                                   &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateEnterExecutingModeIterativeComplete)
{
    // HelicsIterationResult helicsFederateEnterExecutingModeIterativeComplete(HelicsFederate
    // fed, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateEnterExecutingModeIterativeComplete(nullptr, &err);
    EXPECT_EQ(res1, HELICS_ITERATION_RESULT_ERROR);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    auto res2 = helicsFederateEnterExecutingModeIterativeComplete(nullptr, nullptr);
    EXPECT_EQ(res2, HELICS_ITERATION_RESULT_ERROR);
    auto res3 = helicsFederateEnterExecutingModeIterativeComplete(evil_federate, &err);
    EXPECT_EQ(res3, HELICS_ITERATION_RESULT_ERROR);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateGetState)
{
    // HelicsFederateState helicsFederateGetState(HelicsFederate fed, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateGetState(nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, HELICS_STATE_UNKNOWN);
    helicsErrorClear(&err);
    auto res2 = helicsFederateGetState(nullptr, nullptr);
    EXPECT_EQ(res2, HELICS_STATE_UNKNOWN);
    auto res3 = helicsFederateGetState(evil_federate, &err);
    EXPECT_EQ(res3, HELICS_STATE_UNKNOWN);
}

TEST(evil_federate_test, helicsFederateGetCore)
{
    // HelicsCore helicsFederateGetCoreObject(HelicsFederate fed, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateGetCore(nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, nullptr);
    helicsErrorClear(&err);
    auto res2 = helicsFederateGetCore(nullptr, nullptr);
    EXPECT_EQ(res2, nullptr);
    auto res3 = helicsFederateGetCore(evil_federate, &err);
    EXPECT_EQ(res3, nullptr);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateRequestTime)
{
    // HelicsTime helicsFederateRequestTime(HelicsFederate fed, HelicsTime requestTime,
    // HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateRequestTime(nullptr, 1.0, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    EXPECT_DOUBLE_EQ(res1, HELICS_TIME_INVALID);
    auto res2 = helicsFederateRequestTime(nullptr, 1.0, nullptr);
    EXPECT_DOUBLE_EQ(res2, HELICS_TIME_INVALID);
    auto res3 = helicsFederateRequestTime(evil_federate, 1.0, &err);
    EXPECT_DOUBLE_EQ(res3, HELICS_TIME_INVALID);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateRequestTimeAdvance)
{
    // HelicsTime helicsFederateRequestTimeAdvance(HelicsFederate fed, HelicsTime timeDelta,
    // HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateRequestTimeAdvance(nullptr, 1.0, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_DOUBLE_EQ(res1, HELICS_TIME_INVALID);
    helicsErrorClear(&err);
    auto res2 = helicsFederateRequestTimeAdvance(nullptr, 1.0, nullptr);
    EXPECT_DOUBLE_EQ(res2, HELICS_TIME_INVALID);
    auto res3 = helicsFederateRequestTimeAdvance(evil_federate, 1.0, &err);
    EXPECT_DOUBLE_EQ(res3, HELICS_TIME_INVALID);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateRequestNextStep)
{
    // HelicsTime helicsFederateRequestNextStep(HelicsFederate fed, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateRequestNextStep(nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_DOUBLE_EQ(res1, HELICS_TIME_INVALID);
    helicsErrorClear(&err);
    auto res2 = helicsFederateRequestNextStep(nullptr, nullptr);
    EXPECT_DOUBLE_EQ(res2, HELICS_TIME_INVALID);
    auto res3 = helicsFederateRequestNextStep(evil_federate, &err);
    EXPECT_DOUBLE_EQ(res3, HELICS_TIME_INVALID);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateRequestTimeIterative)
{
    // HelicsTime helicsFederateRequestTimeIterative(     HelicsFederate fed,     HelicsTime
    // requestTime,     HelicsIterationRequest iterate,     HelicsIterationResult* outIterate,
    // HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    HelicsIterationResult iteration{HELICS_ITERATION_RESULT_ITERATING};
    auto res1 = helicsFederateRequestTimeIterative(
        nullptr, 1.0, HELICS_ITERATION_REQUEST_FORCE_ITERATION, &iteration, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_DOUBLE_EQ(res1, HELICS_TIME_INVALID);
    EXPECT_EQ(iteration, HELICS_ITERATION_RESULT_ERROR);
    helicsErrorClear(&err);
    iteration = HELICS_ITERATION_RESULT_ITERATING;
    auto res2 = helicsFederateRequestTimeIterative(
        nullptr, 1.0, HELICS_ITERATION_REQUEST_FORCE_ITERATION, &iteration, nullptr);
    EXPECT_DOUBLE_EQ(res2, HELICS_TIME_INVALID);
    EXPECT_EQ(iteration, HELICS_ITERATION_RESULT_ERROR);
    auto res3 = helicsFederateRequestTimeIterative(
        evil_federate, 1.0, HELICS_ITERATION_REQUEST_FORCE_ITERATION, nullptr, &err);
    EXPECT_DOUBLE_EQ(res3, HELICS_TIME_INVALID);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateRequestTimeAsync)
{
    // void helicsFederateRequestTimeAsync(HelicsFederate fed, HelicsTime requestTime,
    // HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateRequestTimeAsync(nullptr, 1.0, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFederateRequestTimeAsync(HelicsFederate fed, HelicsTime requestTime,
    // nullptr);
    helicsFederateRequestTimeAsync(evil_federate, 1.0, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateRequestTimeComplete)
{
    // HelicsTime helicsFederateRequestTimeComplete(HelicsFederate fed, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateRequestTimeComplete(nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_DOUBLE_EQ(res1, HELICS_TIME_INVALID);
    helicsErrorClear(&err);
    auto res2 = helicsFederateRequestTimeComplete(nullptr, nullptr);
    EXPECT_DOUBLE_EQ(res2, HELICS_TIME_INVALID);
    auto res3 = helicsFederateRequestTimeComplete(evil_federate, &err);
    EXPECT_DOUBLE_EQ(res3, HELICS_TIME_INVALID);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateRequestTimeIterativeAsync)
{
    // void helicsFederateRequestTimeIterativeAsync(     HelicsFederate fed,     HelicsTime
    // requestTime,     HelicsIterationRequest iterate,     HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateRequestTimeIterativeAsync(nullptr,
                                            1.0,
                                            HELICS_ITERATION_REQUEST_FORCE_ITERATION,
                                            &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFederateRequestTimeIterativeAsync(     HelicsFederate fed,     HelicsTime
    // requestTime,     HelicsIterationRequest iterate,     nullptr);
    helicsFederateRequestTimeIterativeAsync(evil_federate,
                                            1.0,
                                            HELICS_ITERATION_REQUEST_FORCE_ITERATION,
                                            &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateRequestTimeIterativeComplete)
{
    // HelicsTime helicsFederateRequestTimeIterativeComplete(HelicsFederate fed,
    // HelicsIterationResult* outIterate, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    HelicsIterationResult iteration{HELICS_ITERATION_RESULT_ITERATING};
    auto res1 = helicsFederateRequestTimeIterativeComplete(nullptr, &iteration, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(iteration, HELICS_ITERATION_RESULT_ERROR);
    EXPECT_DOUBLE_EQ(res1, HELICS_TIME_INVALID);
    iteration = HELICS_ITERATION_RESULT_ITERATING;
    helicsErrorClear(&err);
    auto res2 = helicsFederateRequestTimeIterativeComplete(nullptr, nullptr, nullptr);
    EXPECT_DOUBLE_EQ(res2, HELICS_TIME_INVALID);
    auto res3 = helicsFederateRequestTimeIterativeComplete(evil_federate, &iteration, &err);
    EXPECT_DOUBLE_EQ(res3, HELICS_TIME_INVALID);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(iteration, HELICS_ITERATION_RESULT_ERROR);
}

TEST(evil_federate_test, helicsFederateGetName)
{
    // const char*  helicsFederateGetName(HelicsFederate fed);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto res1 = helicsFederateGetName(evil_federate);
    EXPECT_STREQ(res1, "");
    auto res2 = helicsFederateGetName(nullptr);
    EXPECT_STREQ(res2, "");
}

TEST(evil_federate_test, helicsFederateSetTimeProperty)
{
    // void helicsFederateSetTimeProperty(HelicsFederate fed, int timeProperty, HelicsTime time,
    // HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateSetTimeProperty(nullptr, 1, 4.0, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFederateSetTimeProperty(HelicsFederate fed, int timeProperty, HelicsTime
    // time, nullptr);
    helicsFederateSetTimeProperty(evil_federate, 4, 1.0, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateSetFlagOption)
{
    // void helicsFederateSetFlagOption(HelicsFederate fed, int flag, HelicsBool flagValue,
    // HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateSetFlagOption(nullptr, 99, HELICS_FALSE, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFederateSetFlagOption(HelicsFederate fed, int flag, HelicsBool flagValue,
    // nullptr);
    helicsFederateSetFlagOption(evil_federate, 99, HELICS_TRUE, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateSetSeparator)
{
    // void helicsFederateSetSeparator(HelicsFederate fed, char separator, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateSetSeparator(nullptr, '-', &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFederateSetSeparator(HelicsFederate fed, char separator, nullptr);
    helicsFederateSetSeparator(evil_federate, '-', &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateSetIntegerProperty)
{
    // void helicsFederateSetIntegerProperty(HelicsFederate fed, int intProperty, int propertyVal,
    // HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateSetIntegerProperty(nullptr, 0, 99, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFederateSetIntegerProperty(HelicsFederate fed, int intProperty, int
    // propertyVal, nullptr);
    helicsFederateSetIntegerProperty(evil_federate, 99, 99, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateGetTimeProperty)
{
    // HelicsTime helicsFederateGetTimeProperty(HelicsFederate fed, int timeProperty,
    // HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateGetTimeProperty(nullptr, 99, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_DOUBLE_EQ(res1, HELICS_TIME_INVALID);
    helicsErrorClear(&err);
    auto res2 = helicsFederateGetTimeProperty(nullptr, 88, nullptr);
    EXPECT_DOUBLE_EQ(res2, HELICS_TIME_INVALID);
    auto res3 = helicsFederateGetTimeProperty(evil_federate, 77, &err);
    EXPECT_DOUBLE_EQ(res3, HELICS_TIME_INVALID);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateGetFlagOption)
{
    // HelicsBool helicsFederateGetFlagOption(HelicsFederate fed, int flag, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateGetFlagOption(nullptr, 1, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, HELICS_FALSE);
    helicsErrorClear(&err);
    auto res2 = helicsFederateGetFlagOption(nullptr, 1, nullptr);
    EXPECT_EQ(res2, HELICS_FALSE);
    auto res3 = helicsFederateGetFlagOption(evil_federate, 1, &err);
    EXPECT_EQ(res3, HELICS_FALSE);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateGetIntegerProperty)
{
    // int helicsFederateGetIntegerProperty(HelicsFederate fed, int intProperty, HelicsError*
    // err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateGetIntegerProperty(nullptr, 99, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_LT(res1, 0);
    helicsErrorClear(&err);
    auto res2 = helicsFederateGetIntegerProperty(nullptr, 20, nullptr);
    EXPECT_LT(res2, 0);
    auto res3 = helicsFederateGetIntegerProperty(evil_federate, 20, &err);
    EXPECT_LT(res3, 0);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateGetCurrentTime)
{
    // HelicsTime helicsFederateGetCurrentTime(HelicsFederate fed, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateGetCurrentTime(nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_DOUBLE_EQ(res1, HELICS_TIME_INVALID);
    helicsErrorClear(&err);
    auto res2 = helicsFederateGetCurrentTime(nullptr, nullptr);
    EXPECT_DOUBLE_EQ(res2, HELICS_TIME_INVALID);
    auto res3 = helicsFederateGetCurrentTime(evil_federate, &err);
    EXPECT_DOUBLE_EQ(res3, HELICS_TIME_INVALID);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateSetGlobal)
{
    // void helicsFederateSetGlobal(HelicsFederate fed, const char* valueName, const char* value,
    // HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateSetGlobal(nullptr, nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFederateSetGlobal(HelicsFederate fed, const char* valueName, const char*
    // value, nullptr);
    helicsFederateSetGlobal(evil_federate, "global", "glob", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateAddDependency)
{
    // void helicsFederateAddDependency(HelicsFederate fed, const char* fedName, HelicsError*
    // err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateAddDependency(nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFederateSetGlobal(HelicsFederate fed, const char* valueName, const char*
    // value, nullptr);
    helicsFederateAddDependency(evil_federate, "fed", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateSetLogFile)
{
    // void helicsFederateSetLogFile(HelicsFederate fed, const char* logFile, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateSetLogFile(nullptr, "unknownfile.txt", &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFederateSetLogFile(HelicsFederate fed, const char* logFile, nullptr);
    helicsFederateSetLogFile(evil_federate, "unknownfile.txt", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateLogErrorMessage)
{
    // void helicsFederateLogErrorMessage(HelicsFederate fed, const char* logmessage, HelicsError*
    // err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateLogErrorMessage(nullptr, "null log", &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFederateLogErrorMessage(HelicsFederate fed, const char* logmessage,
    // nullptr);
    helicsFederateLogErrorMessage(evil_federate, "null log", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateLogWarningMessage)
{
    // void helicsFederateLogWarningMessage(HelicsFederate fed, const char* logmessage,
    // HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateLogWarningMessage(nullptr, "null log", &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFederateLogWarningMessage(HelicsFederate fed, const char* logmessage,
    // nullptr);
    helicsFederateLogWarningMessage(evil_federate, "null log", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateLogInfoMessage)
{
    // void helicsFederateLogInfoMessage(HelicsFederate fed, const char* logmessage, HelicsError*
    // err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateLogInfoMessage(nullptr, "null log", &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFederateLogInfoMessage(HelicsFederate fed, "null log", nullptr);
    helicsFederateLogInfoMessage(evil_federate, "null log", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateLogDebugMessage)
{
    // void helicsFederateLogDebugMessage(HelicsFederate fed, const char* logmessage, HelicsError*
    // err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateLogDebugMessage(nullptr, "null log", &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFederateLogDebugMessage(HelicsFederate fed, const char* logmessage,
    // nullptr);
    helicsFederateLogDebugMessage(evil_federate, "null log", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateLogLevelMessage)
{
    // void helicsFederateLogLevelMessage(HelicsFederate fed, int loglevel, const char* logmessage,
    // HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateLogLevelMessage(nullptr, 0, "null log", &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFederateLogLevelMessage(HelicsFederate fed, int loglevel, const char*
    // logmessage, nullptr);
    helicsFederateLogLevelMessage(evil_federate, 0, "null log", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_federate_test, helicsFederateSetLoggingCallback)
{
    // void helicsFederateSetLoggingCallback(     HelicsFederate fed,     void (*logger)(int
    // loglevel, const char* identifier, const char* message, void* userData),     void* userdata,
    // HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateSetLoggingCallback(nullptr, nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFederateSetLoggingCallback(     HelicsFederate fed,     void (*logger)(int
    // loglevel, const char* identifier, const char* message, void* userData),     void* userdata,
    // nullptr);
    helicsFederateSetLoggingCallback(evil_federate, nullptr, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
}

// section Value Federate Functions
// functions applying to federates created as a value or combination federate \ref HelicsFederate
// objects
TEST(evil_value_federate_test, helicsFederateRegisterSubscription)
{
    // HelicsInput helicsFederateRegisterSubscription(HelicsFederate fed, const char* key, const
    // char* units, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateRegisterSubscription(nullptr, "key", nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, nullptr);
    helicsErrorClear(&err);
    auto res2 = helicsFederateRegisterSubscription(nullptr, nullptr, nullptr, nullptr);
    EXPECT_EQ(res2, nullptr);
    auto res3 = helicsFederateRegisterSubscription(evil_federate, "key", "pu", &err);
    EXPECT_EQ(res3, nullptr);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_value_federate_test, helicsFederateRegisterPublication)
{
    // HelicsPublication helicsFederateRegisterPublication(HelicsFederate fed, const char* key,
    // HelicsDataTypes type, const char* units, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 =
        helicsFederateRegisterPublication(nullptr, "key", HELICS_DATA_TYPE_COMPLEX, nullptr, &err);
    EXPECT_EQ(res1, nullptr);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    auto res2 = helicsFederateRegisterPublication(
        nullptr, nullptr, HELICS_DATA_TYPE_COMPLEX, nullptr, nullptr);
    EXPECT_EQ(res2, nullptr);
    auto res3 = helicsFederateRegisterPublication(
        evil_federate, "key", HELICS_DATA_TYPE_COMPLEX, nullptr, &err);
    EXPECT_EQ(res3, nullptr);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_value_federate_test, helicsFederateRegisterTypePublication)
{
    // HelicsPublication helicsFederateRegisterTypePublication(HelicsFederate fed, const char*
    // key, const char* type, const char* units, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateRegisterTypePublication(nullptr, "key", "type", "", &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, nullptr);
    helicsErrorClear(&err);
    auto res2 = helicsFederateRegisterTypePublication(nullptr, nullptr, nullptr, nullptr, nullptr);
    EXPECT_EQ(res2, nullptr);
    auto res3 = helicsFederateRegisterTypePublication(evil_federate, "key", "type", "", &err);
    EXPECT_EQ(res3, nullptr);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_value_federate_test, helicsFederateRegisterGlobalPublication)
{
    // HelicsPublication helicsFederateRegisterGlobalPublication(     HelicsFederate fed, const
    // char* key,     HelicsDataTypes type,     const char* units,     HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 =
        helicsFederateRegisterGlobalPublication(nullptr, "key", HELICS_DATA_TYPE_ANY, "", &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, nullptr);
    helicsErrorClear(&err);
    auto res2 = helicsFederateRegisterGlobalPublication(
        nullptr, nullptr, HELICS_DATA_TYPE_ANY, nullptr, nullptr);
    EXPECT_EQ(res2, nullptr);
    auto res3 = helicsFederateRegisterGlobalPublication(
        evil_federate, "key", HELICS_DATA_TYPE_ANY, "", &err);
    EXPECT_EQ(res3, nullptr);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_value_federate_test, helicsFederateRegisterGlobalTypePublication)
{
    // HelicsPublication helicsFederateRegisterGlobalTypePublication(     HelicsFederate fed,
    // const char* key,     const char* type,     const char* units,     HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateRegisterGlobalTypePublication(nullptr, "key", "type", "", &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, nullptr);
    helicsErrorClear(&err);
    auto res2 =
        helicsFederateRegisterGlobalTypePublication(nullptr, nullptr, nullptr, nullptr, nullptr);
    EXPECT_EQ(res2, nullptr);
    auto res3 = helicsFederateRegisterGlobalTypePublication(evil_federate, "key", "type", "", &err);
    EXPECT_EQ(res3, nullptr);
}

TEST(evil_value_federate_test, helicsFederateRegisterInput)
{
    // HelicsInput helicsFederateRegisterInput(HelicsFederate fed, const char* key,
    // HelicsDataTypes type, const char* units, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateRegisterInput(nullptr, "key", HELICS_DATA_TYPE_ANY, "", &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, nullptr);
    helicsErrorClear(&err);
    auto res2 =
        helicsFederateRegisterInput(nullptr, nullptr, HELICS_DATA_TYPE_ANY, nullptr, nullptr);
    EXPECT_EQ(res2, nullptr);
    auto res3 = helicsFederateRegisterInput(evil_federate, "key", HELICS_DATA_TYPE_ANY, "", &err);
    EXPECT_EQ(res3, nullptr);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_value_federate_test, helicsFederateRegisterTypeInput)
{
    // HelicsInput helicsFederateRegisterTypeInput(HelicsFederate fed, const char* key, const
    // char* type, const char* units, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateRegisterTypeInput(nullptr, "key", "type", "", &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, nullptr);
    helicsErrorClear(&err);
    auto res2 = helicsFederateRegisterTypeInput(nullptr, nullptr, nullptr, nullptr, nullptr);
    EXPECT_EQ(res2, nullptr);
    auto res3 = helicsFederateRegisterTypeInput(evil_federate, "key", "type", "", &err);
    EXPECT_EQ(res3, nullptr);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_value_federate_test, helicsFederateRegisterGlobalInput)
{
    // HelicsPublication helicsFederateRegisterGlobalInput(HelicsFederate fed, const char* key,
    // HelicsDataTypes type, const char* units, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateRegisterGlobalInput(nullptr, "key", HELICS_DATA_TYPE_ANY, "", &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, nullptr);
    helicsErrorClear(&err);
    auto res2 =
        helicsFederateRegisterGlobalInput(nullptr, nullptr, HELICS_DATA_TYPE_ANY, nullptr, nullptr);
    EXPECT_EQ(res2, nullptr);
    auto res3 =
        helicsFederateRegisterGlobalInput(evil_federate, "key", HELICS_DATA_TYPE_ANY, "", &err);
    EXPECT_EQ(res3, nullptr);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_value_federate_test, helicsFederateRegisterGlobalTypeInput)
{
    // HelicsPublication helicsFederateRegisterGlobalTypeInput(HelicsFederate fed, const char*
    // key, const char* type, const char* units, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateRegisterGlobalTypeInput(nullptr, "key", "type", "", &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, nullptr);
    helicsErrorClear(&err);
    auto res2 = helicsFederateRegisterGlobalTypeInput(nullptr, nullptr, nullptr, nullptr, nullptr);
    EXPECT_EQ(res2, nullptr);
    auto res3 = helicsFederateRegisterGlobalTypeInput(evil_federate, "key", "type", "", &err);
    EXPECT_EQ(res3, nullptr);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_value_federate_test, helicsFederateGetPublication)
{
    // HelicsPublication helicsFederateGetPublication(HelicsFederate fed, const char* key,
    // HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateGetPublication(nullptr, "key", &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, nullptr);
    helicsErrorClear(&err);
    auto res2 = helicsFederateGetPublication(nullptr, nullptr, nullptr);
    EXPECT_EQ(res2, nullptr);
    auto res3 = helicsFederateGetPublication(evil_federate, "key", &err);
    EXPECT_EQ(res3, nullptr);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_value_federate_test, helicsFederateGetPublicationByIndex)
{
    // HelicsPublication helicsFederateGetPublicationByIndex(HelicsFederate fed, int index,
    // HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateGetPublicationByIndex(nullptr, 0, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, nullptr);
    helicsErrorClear(&err);
    auto res2 = helicsFederateGetPublicationByIndex(nullptr, 0, nullptr);
    EXPECT_EQ(res2, nullptr);
    auto res3 = helicsFederateGetPublicationByIndex(evil_federate, 0, &err);
    EXPECT_EQ(res3, nullptr);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_value_federate_test, helicsFederateGetInput)
{
    // HelicsInput helicsFederateGetInput(HelicsFederate fed, const char* key, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateGetInput(nullptr, "key", &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, nullptr);
    helicsErrorClear(&err);
    auto res2 = helicsFederateGetInput(nullptr, nullptr, nullptr);
    EXPECT_EQ(res2, nullptr);
    auto res3 = helicsFederateGetInput(evil_federate, "key", &err);
    EXPECT_EQ(res3, nullptr);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_value_federate_test, helicsFederateGetInputByIndex)
{
    // HelicsInput helicsFederateGetInputByIndex(HelicsFederate fed, int index, HelicsError*
    // err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateGetInputByIndex(nullptr, 0, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, nullptr);
    helicsErrorClear(&err);
    auto res2 = helicsFederateGetInputByIndex(nullptr, 0, nullptr);
    EXPECT_EQ(res2, nullptr);
    auto res3 = helicsFederateGetInputByIndex(evil_federate, 0, &err);
    EXPECT_EQ(res3, nullptr);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_value_federate_test, helicsFederateGetInputByTarget)
{
    // HelicsInput helicsFederateGetInputByTarget(HelicsFederate fed, const char* key,
    // HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateGetInputByTarget(nullptr, "key", &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, nullptr);
    helicsErrorClear(&err);
    auto res2 = helicsFederateGetInputByTarget(nullptr, nullptr, nullptr);
    EXPECT_EQ(res2, nullptr);
    auto res3 = helicsFederateGetInputByTarget(evil_federate, "key", &err);
    EXPECT_EQ(res3, nullptr);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_value_federate_test, helicsFederateClearUpdates)
{
    // void helicsFederateClearUpdates(HelicsFederate fed);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    EXPECT_NO_THROW(helicsFederateClearUpdates(evil_federate));
}

TEST(evil_value_federate_test, helicsFederateRegisterFromPublicationJSON)
{
    // void helicsFederateRegisterFromPublicationJSON(HelicsFederate fed, const char* json,
    // HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederateRegisterFromPublicationJSON(nullptr, "json.json", &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFederateRegisterFromPublicationJSON(HelicsFederate fed, const char* json,
    // nullptr);
    helicsFederateRegisterFromPublicationJSON(evil_federate, "json.json", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_value_federate_test, helicsFederatePublishJSON)
{
    // void helicsFederatePublishJSON(HelicsFederate fed, const char* json, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFederatePublishJSON(nullptr, "json.json", &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFederatePublishJSON(HelicsFederate fed, const char* json, nullptr);
    helicsFederatePublishJSON(evil_federate, "json.json", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_value_federate_test, helicsFederateGetPublicationCount)
{
    // int helicsFederateGetPublicationCount(HelicsFederate fed);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto res1 = helicsFederateGetPublicationCount(nullptr);
    EXPECT_EQ(res1, 0);
    auto res2 = helicsFederateGetPublicationCount(evil_federate);
    EXPECT_EQ(res2, 0);
}

TEST(evil_value_federate_test, helicsFederateGetInputCount)
{
    // int helicsFederateGetInputCount(HelicsFederate fed);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto res1 = helicsFederateGetInputCount(nullptr);
    EXPECT_EQ(res1, 0);
    auto res2 = helicsFederateGetInputCount(evil_federate);
    EXPECT_EQ(res2, 0);
}

// section Publication interface Functions
// functions applying to a \ref HelicsPublication object

TEST(evil_pub_test, helicsPublicationIsValid)
{
    char rdata[256];
    auto evil_pub = reinterpret_cast<HelicsPublication>(rdata);
    EXPECT_NE(helicsPublicationIsValid(nullptr), HELICS_TRUE);
    EXPECT_NE(helicsPublicationIsValid(evil_pub), HELICS_TRUE);
}

TEST(evil_pub_test, helicsPublicationPublishBytes)
{
    // void helicsPublicationPublishBytes(HelicsPublication pub, const void* data, int
    // inputDataLength, HelicsError* err);
    char rdata[256];
    auto evil_pub = reinterpret_cast<HelicsPublication>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsPublicationPublishBytes(nullptr, nullptr, 85, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsPublicationPublishBytes(HelicsPublication pub, const void* data, int
    // inputDataLength, nullptr);
    helicsPublicationPublishBytes(evil_pub, nullptr, 14654181, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_pub_test, helicsPublicationPublishString)
{
    // void helicsPublicationPublishString(HelicsPublication pub, const char* str, HelicsError*
    // err);
    char rdata[256];
    auto evil_pub = reinterpret_cast<HelicsPublication>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsPublicationPublishString(nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsPublicationPublishString(HelicsPublication pub, const char* str, nullptr);
    helicsPublicationPublishString(evil_pub, "String", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_pub_test, helicsPublicationPublishInteger)
{
    // void helicsPublicationPublishInteger(HelicsPublication pub, int64_t val, HelicsError* err);
    char rdata[256];
    auto evil_pub = reinterpret_cast<HelicsPublication>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsPublicationPublishInteger(nullptr, 1, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsPublicationPublishInteger(HelicsPublication pub, int64_t val, nullptr);
    helicsPublicationPublishInteger(evil_pub, 1, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_pub_test, helicsPublicationPublishBoolean)
{
    // void helicsPublicationPublishBoolean(HelicsPublication pub, HelicsBool val, HelicsError*
    // err);
    char rdata[256];
    auto evil_pub = reinterpret_cast<HelicsPublication>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsPublicationPublishBoolean(nullptr, 0, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsPublicationPublishBoolean(HelicsPublication pub, HelicsBool val, nullptr);
    helicsPublicationPublishBoolean(evil_pub, 99, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_pub_test, helicsPublicationPublishDouble)
{
    // void helicsPublicationPublishDouble(HelicsPublication pub, double val, HelicsError* err);
    char rdata[256];
    auto evil_pub = reinterpret_cast<HelicsPublication>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsPublicationPublishDouble(nullptr, 1.7, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsPublicationPublishDouble(HelicsPublication pub, double val, nullptr);
    helicsPublicationPublishDouble(evil_pub, 2.0, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_pub_test, helicsPublicationPublishTime)
{
    // void helicsPublicationPublishTime(HelicsPublication pub, HelicsTime val, HelicsError*
    // err);
    char rdata[256];
    auto evil_pub = reinterpret_cast<HelicsPublication>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsPublicationPublishTime(nullptr, 4.3, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsPublicationPublishTime(HelicsPublication pub, HelicsTime val, nullptr);
    helicsPublicationPublishTime(evil_pub, 5.0, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_pub_test, helicsPublicationPublishChar)
{
    // void helicsPublicationPublishChar(HelicsPublication pub, char val, HelicsError* err);
    char rdata[256];
    auto evil_pub = reinterpret_cast<HelicsPublication>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsPublicationPublishChar(nullptr, '\0', &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsPublicationPublishChar(HelicsPublication pub, char val, nullptr);
    helicsPublicationPublishChar(evil_pub, 'c', &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_pub_test, helicsPublicationPublishComplex)
{
    // void helicsPublicationPublishComplex(HelicsPublication pub, double real, double imag,
    // HelicsError* err);
    char rdata[256];
    auto evil_pub = reinterpret_cast<HelicsPublication>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsPublicationPublishComplex(nullptr, 2.0, -6.5, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsPublicationPublishComplex(HelicsPublication pub, double real, double imag,
    // nullptr);
    helicsPublicationPublishComplex(evil_pub, 4.507, 11.3, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_pub_test, helicsPublicationPublishVector)
{
    // void helicsPublicationPublishVector(HelicsPublication pub, const double* vectorInput, int
    // vectorLength, HelicsError* err);
    char rdata[256];
    auto evil_pub = reinterpret_cast<HelicsPublication>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsPublicationPublishVector(nullptr, nullptr, 99, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsPublicationPublishVector(HelicsPublication pub, const double* vectorInput,
    // int vectorLength, nullptr);
    helicsPublicationPublishVector(evil_pub, nullptr, 99, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_pub_test, helicsPublicationPublishNamedPoint)
{
    // void helicsPublicationPublishNamedPoint(HelicsPublication pub, const char* str, double val,
    // HelicsError* err);
    char rdata[256];
    auto evil_pub = reinterpret_cast<HelicsPublication>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsPublicationPublishNamedPoint(nullptr, "string", 5.0, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsPublicationPublishNamedPoint(HelicsPublication pub, const char* str, double
    // val, nullptr);
    helicsPublicationPublishNamedPoint(evil_pub, "string", 5.0, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_pub_test, helicsPublicationAddTarget)
{
    // void helicsPublicationAddTarget(HelicsPublication pub, const char* target, HelicsError*
    // err);
    char rdata[256];
    auto evil_pub = reinterpret_cast<HelicsPublication>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsPublicationAddTarget(nullptr, "target", &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsPublicationAddTarget(HelicsPublication pub, const char* target, nullptr);
    helicsPublicationAddTarget(evil_pub, "target", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_pub_test, helicsPublicationGetType)
{
    // const char*  helicsPublicationGetType(HelicsPublication pub);
    char rdata[256];
    auto evil_pub = reinterpret_cast<HelicsPublication>(rdata);
    auto res1 = helicsPublicationGetType(nullptr);
    EXPECT_STREQ(res1, "");
    auto res2 = helicsPublicationGetType(evil_pub);
    EXPECT_STREQ(res2, "");
}

TEST(evil_pub_test, helicsPublicationGetName)
{
    // const char*  helicsPublicationGetName(HelicsPublication pub);
    char rdata[256];
    auto evil_pub = reinterpret_cast<HelicsPublication>(rdata);
    auto res1 = helicsPublicationGetName(nullptr);
    EXPECT_STREQ(res1, "");
    auto res2 = helicsPublicationGetName(evil_pub);
    EXPECT_STREQ(res2, "");
}

TEST(evil_pub_test, helicsPublicationGetUnits)
{
    // const char*  helicsPublicationGetUnits(HelicsPublication pub);
    char rdata[256];
    auto evil_pub = reinterpret_cast<HelicsPublication>(rdata);
    auto res1 = helicsPublicationGetUnits(nullptr);
    EXPECT_STREQ(res1, "");
    auto res2 = helicsPublicationGetUnits(evil_pub);
    EXPECT_STREQ(res2, "");
}

TEST(evil_pub_test, helicsPublicationGetInfo)
{
    // const char*  helicsPublicationGetInfo(HelicsPublication pub);
    char rdata[256];
    auto evil_pub = reinterpret_cast<HelicsPublication>(rdata);
    auto res1 = helicsPublicationGetInfo(nullptr);
    EXPECT_STREQ(res1, "");
    auto res2 = helicsPublicationGetInfo(evil_pub);
    EXPECT_STREQ(res2, "");
}

TEST(evil_pub_test, helicsPublicationSetInfo)
{
    // void helicsPublicationSetInfo(HelicsPublication pub, const char* info, HelicsError* err);
    char rdata[256];
    auto evil_pub = reinterpret_cast<HelicsPublication>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsPublicationSetInfo(nullptr, "info", &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsPublicationSetInfo(HelicsPublication pub, const char* info, nullptr);
    helicsPublicationSetInfo(evil_pub, "", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_pub_test, helicsPublicationSetMinimumChange)
{
    // void helicsPublicationSetMinimumChange(HelicsPublication pub, double tolerance,
    // HelicsError* err);
    char rdata[256];
    auto evil_pub = reinterpret_cast<HelicsPublication>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsPublicationSetMinimumChange(nullptr, 12.0, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsPublicationSetInfo(HelicsPublication pub, const char* info, nullptr);
    helicsPublicationSetMinimumChange(evil_pub, 1.0, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_pub_test, helicsPublicationGetOption)
{
    // HelicsBool helicsPublicationGetOption(HelicsPublication pub, int option);
    char rdata[256];
    auto evil_pub = reinterpret_cast<HelicsPublication>(rdata);
    auto res1 = helicsPublicationGetOption(nullptr, -45);
    EXPECT_EQ(res1, HELICS_FALSE);
    auto res2 = helicsPublicationGetOption(evil_pub, 15);
    EXPECT_EQ(res2, HELICS_FALSE);
}

TEST(evil_pub_test, helicsPublicationSetOption)
{
    // void helicsPublicationSetOption(HelicsPublication pub, int option, HelicsBool val,
    // HelicsError* err);
    char rdata[256];
    auto evil_pub = reinterpret_cast<HelicsPublication>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsPublicationSetOption(nullptr, -10, HELICS_TRUE, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsPublicationSetOption(HelicsPublication pub, int option, HelicsBool val,
    // nullptr);
    helicsPublicationSetOption(evil_pub, 45, HELICS_TRUE, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    helicsPublicationSetOption(nullptr, 45, HELICS_TRUE, &err);
    EXPECT_NE(err.error_code, 0);
}

// section Input interface Functions
// functions applying to a \ref HelicsInput object

TEST(evil_input_test, helicsInputIsValid)
{
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    EXPECT_NE(helicsInputIsValid(nullptr), HELICS_TRUE);
    EXPECT_NE(helicsInputIsValid(evil_input), HELICS_TRUE);
}

TEST(evil_input_test, helicsInputAddTarget)
{
    // void helicsInputAddTarget(HelicsInput ipt, const char* target, HelicsError* err);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsInputAddTarget(nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsInputAddTarget(HelicsInput ipt, const char* target, nullptr);
    helicsInputAddTarget(evil_input, "target", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_input_test, helicsInputGetByteCount)
{
    // int helicsInputGetByteCount(HelicsInput ipt);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto res1 = helicsInputGetByteCount(nullptr);
    EXPECT_EQ(res1, 0);
    auto res2 = helicsInputGetByteCount(evil_input);
    EXPECT_EQ(res2, 0);
}

TEST(evil_input_test, helicsInputGetBytes)
{
    // void helicsInputGetBytes(HelicsInput ipt, void* data, int maxDatalen, int* actualSize,
    // HelicsError* err);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    int actLen = 99;
    helicsInputGetBytes(nullptr, nullptr, 87, &actLen, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(actLen, 0);
    helicsErrorClear(&err);
    actLen = 99;
    // auto res2=helicsInputGetBytes(HelicsInput ipt, void* data, int maxDatalen, int*
    // actualSize, nullptr);
    helicsInputGetBytes(evil_input, rdata, 10, &actLen, &err);
    EXPECT_EQ(actLen, 0);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_input_test, helicsInputGetStringSize)
{
    // int helicsInputGetStringSize(HelicsInput ipt);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto res1 = helicsInputGetStringSize(nullptr);
    EXPECT_EQ(res1, 0);
    auto res2 = helicsInputGetStringSize(evil_input);
    EXPECT_EQ(res2, 0);
}

TEST(evil_input_test, helicsInputGetString)
{
    // void helicsInputGetString(HelicsInput ipt, char* outputString, int maxStringLen, int*
    // actualLength, HelicsError* err);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    int actLen = 99;
    helicsInputGetString(nullptr, nullptr, 67, &actLen, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(actLen, 0);
    helicsErrorClear(&err);
    // auto res2=helicsInputGetString(HelicsInput ipt, char* outputString, int maxStringLen, int*
    // actualLength, nullptr);
    actLen = 99;
    helicsInputGetString(evil_input, nullptr, 45, &actLen, &err);
    EXPECT_EQ(actLen, 0);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_input_test, helicsInputGetInteger)
{
    // int64_t helicsInputGetInteger(HelicsInput ipt, HelicsError* err);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsInputGetInteger(nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, -101);
    helicsErrorClear(&err);
    auto res2 = helicsInputGetInteger(nullptr, nullptr);
    EXPECT_EQ(res2, -101);
    auto res3 = helicsInputGetInteger(evil_input, &err);
    EXPECT_EQ(res3, -101);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_input_test, helicsInputGetBoolean)
{
    // HelicsBool helicsInputGetBoolean(HelicsInput ipt, HelicsError* err);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsInputGetBoolean(nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, HELICS_FALSE);
    helicsErrorClear(&err);
    auto res2 = helicsInputGetBoolean(nullptr, nullptr);
    EXPECT_EQ(res2, HELICS_FALSE);
    auto res3 = helicsInputGetBoolean(evil_input, &err);
    EXPECT_EQ(res3, HELICS_FALSE);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_input_test, helicsInputGetDouble)
{
    // double helicsInputGetDouble(HelicsInput ipt, HelicsError* err);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsInputGetDouble(nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, HELICS_INVALID_DOUBLE);
    helicsErrorClear(&err);
    auto res2 = helicsInputGetDouble(nullptr, nullptr);
    EXPECT_EQ(res2, HELICS_INVALID_DOUBLE);
    auto res3 = helicsInputGetDouble(evil_input, &err);
    EXPECT_EQ(res3, HELICS_INVALID_DOUBLE);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_input_test, helicsInputGetTime)
{
    // HelicsTime helicsInputGetTime(HelicsInput ipt, HelicsError* err);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsInputGetTime(nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, HELICS_TIME_INVALID);
    helicsErrorClear(&err);
    auto res2 = helicsInputGetTime(nullptr, nullptr);
    EXPECT_EQ(res2, HELICS_TIME_INVALID);
    auto res3 = helicsInputGetTime(evil_input, &err);
    EXPECT_EQ(res3, HELICS_TIME_INVALID);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_input_test, helicsInputGetChar)
{
    // char helicsInputGetChar(HelicsInput ipt, HelicsError* err);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    char res1 = helicsInputGetChar(nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    char testChar = '\x15';
    EXPECT_TRUE(res1 == testChar);
    helicsErrorClear(&err);
    char res2 = helicsInputGetChar(nullptr, nullptr);
    EXPECT_TRUE(res2 == testChar);
    char res3 = helicsInputGetChar(evil_input, &err);
    EXPECT_TRUE(res3 == testChar);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_input_test, helicsInputGetComplexObject)
{
    // HelicsComplex helicsInputGetComplexObject(HelicsInput ipt, HelicsError* err);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsInputGetComplexObject(nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1.real, HELICS_TIME_INVALID);
    helicsErrorClear(&err);
    auto res2 = helicsInputGetComplexObject(nullptr, nullptr);
    EXPECT_EQ(res2.real, HELICS_TIME_INVALID);
    auto res3 = helicsInputGetComplexObject(evil_input, &err);
    EXPECT_EQ(res3.real, HELICS_TIME_INVALID);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_input_test, helicsInputGetComplex)
{
    // void helicsInputGetComplex(HelicsInput ipt, double* real, double* imag, HelicsError* err);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsInputGetComplex(nullptr, nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    double value1 = 19.4;
    double value2 = 18.3;
    // auto res2=helicsInputGetComplex(HelicsInput ipt, double* real, double* imag, nullptr);
    helicsInputGetComplex(evil_input, &value1, &value2, &err);
    EXPECT_DOUBLE_EQ(value1, 19.4);
    EXPECT_DOUBLE_EQ(value2, 18.3);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_input_test, helicsInputGetVectorSize)
{
    // int helicsInputGetVectorSize(HelicsInput ipt);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto res1 = helicsInputGetVectorSize(nullptr);
    EXPECT_EQ(res1, 0);
    auto res2 = helicsInputGetVectorSize(evil_input);
    EXPECT_EQ(res2, 0);
}

TEST(evil_input_test, helicsInputGetVector)
{
    // void helicsInputGetVector(HelicsInput ipt, double data[], int maxlen, int* actualSize,
    // HelicsError* err);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    int actLen = -56;
    helicsInputGetVector(nullptr, nullptr, 99, &actLen, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(actLen, 0);
    helicsErrorClear(&err);
    // auto res2=helicsInputGetVector(HelicsInput ipt, double data[], int maxlen, int* actualSize,
    // nullptr);
    helicsInputGetVector(evil_input, nullptr, 107, &actLen, &err);
    EXPECT_EQ(actLen, 0);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_input_test, helicsInputGetNamedPoint)
{
    // void helicsInputGetNamedPoint(HelicsInput ipt, char* outputString, int maxStringLen, int*
    // actualLength, double* val, HelicsError* err);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    int actLen = -56;
    double val = -15.0;
    helicsInputGetNamedPoint(nullptr, nullptr, 55, &actLen, &val, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(actLen, 0);
    EXPECT_EQ(val, -15.0);
    helicsErrorClear(&err);
    helicsInputGetNamedPoint(nullptr, nullptr, 99, nullptr, nullptr, nullptr);
    actLen = -56;
    helicsInputGetNamedPoint(evil_input, rdata, 256, &actLen, &val, &err);
    EXPECT_EQ(actLen, 0);
    EXPECT_EQ(val, -15.0);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_input_test, helicsInputSetDefaultBytes)
{
    // void helicsInputSetDefaultBytes(HelicsInput ipt, const void* data, int inputDataLength,
    // HelicsError* err);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsInputSetDefaultBytes(nullptr, nullptr, -87, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsInputSetDefaultBytes(HelicsInput ipt, const void* data, int inputDataLength,
    // nullptr);
    helicsInputSetDefaultBytes(evil_input, nullptr, 15, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_input_test, helicsInputSetDefaultString)
{
    // void helicsInputSetDefaultString(HelicsInput ipt, const char* str, HelicsError* err);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsInputSetDefaultString(nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsInputSetDefaultString(HelicsInput ipt, const char* str, nullptr);
    helicsInputSetDefaultString(evil_input, "string", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_input_test, helicsInputSetDefaultInteger)
{
    // void helicsInputSetDefaultInteger(HelicsInput ipt, int64_t val, HelicsError* err);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsInputSetDefaultInteger(nullptr, -1798524456525, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsInputSetDefaultInteger(HelicsInput ipt, int64_t val, nullptr);
    helicsInputSetDefaultInteger(evil_input, 99, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_input_test, helicsInputSetDefaultBoolean)
{
    // void helicsInputSetDefaultBoolean(HelicsInput ipt, HelicsBool val, HelicsError* err);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsInputSetDefaultBoolean(nullptr, HELICS_FALSE, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsInputSetDefaultBoolean(HelicsInput ipt, HelicsBool val, nullptr);
    helicsInputSetDefaultBoolean(evil_input, HELICS_TRUE, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_input_test, helicsInputSetDefaultTime)
{
    // void helicsInputSetDefaultTime(HelicsInput ipt, HelicsTime val, HelicsError* err);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsInputSetDefaultTime(nullptr, 5.7, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsInputSetDefaultTime(HelicsInput ipt, HelicsTime val, nullptr);
    helicsInputSetDefaultTime(evil_input, HELICS_TIME_INVALID, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_input_test, helicsInputSetDefaultChar)
{
    // void helicsInputSetDefaultChar(HelicsInput ipt, char val, HelicsError* err);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsInputSetDefaultChar(nullptr, 'b', &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsInputSetDefaultChar(HelicsInput ipt, char val, nullptr);
    helicsInputSetDefaultChar(evil_input, 'a', &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_input_test, helicsInputSetDefaultDouble)
{
    // void helicsInputSetDefaultDouble(HelicsInput ipt, double val, HelicsError* err);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsInputSetDefaultDouble(nullptr, 1.0, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsInputSetDefaultDouble(HelicsInput ipt, double val, nullptr);
    helicsInputSetDefaultDouble(evil_input, 1.0, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_input_test, helicsInputSetDefaultComplex)
{
    // void helicsInputSetDefaultComplex(HelicsInput ipt, double real, double imag, HelicsError*
    // err);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsInputSetDefaultComplex(nullptr, 1.0, 1.0, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsInputSetDefaultComplex(HelicsInput ipt, double real, double imag, nullptr);
    helicsInputSetDefaultComplex(evil_input, 1.0, 1.0, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_input_test, helicsInputSetDefaultVector)
{
    // void helicsInputSetDefaultVector(HelicsInput ipt, const double* vectorInput, int
    // vectorLength, HelicsError* err);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsInputSetDefaultVector(nullptr, nullptr, 28, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsInputSetDefaultVector(HelicsInput ipt, const double* vectorInput, int
    // vectorLength, nullptr);
    helicsInputSetDefaultVector(evil_input, nullptr, 87, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_input_test, helicsInputSetDefaultNamedPoint)
{
    // void helicsInputSetDefaultNamedPoint(HelicsInput ipt, const char* str, double val,
    // HelicsError* err);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsInputSetDefaultNamedPoint(nullptr, nullptr, 0.0, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsInputSetDefaultNamedPoint(HelicsInput ipt, const char* str, double val,
    // nullptr);
    helicsInputSetDefaultNamedPoint(evil_input, "string", 19, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_input_test, helicsInputGetType)
{
    // const char*  helicsInputGetType(HelicsInput ipt);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto res1 = helicsInputGetType(nullptr);
    EXPECT_STREQ(res1, "");
    auto res2 = helicsInputGetType(evil_input);
    EXPECT_STREQ(res2, "");
}

TEST(evil_input_test, helicsInputGetPublicationType)
{
    // const char*  helicsInputGetPublicationType(HelicsInput ipt);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto res1 = helicsInputGetPublicationType(nullptr);
    EXPECT_STREQ(res1, "");
    auto res2 = helicsInputGetPublicationType(evil_input);
    EXPECT_STREQ(res2, "");
}

TEST(evil_input_test, helicsInputGetName)
{
    // const char*  helicsInputGetName(HelicsInput ipt);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto res1 = helicsInputGetName(nullptr);
    EXPECT_STREQ(res1, "");
    auto res2 = helicsInputGetName(evil_input);
    EXPECT_STREQ(res2, "");
}

TEST(evil_input_test, helicsInputGetTarget)
{
    // const char*  helicsInputGetTarget(HelicsInput ipt);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto res1 = helicsInputGetTarget(nullptr);
    EXPECT_STREQ(res1, "");
    auto res2 = helicsInputGetTarget(evil_input);
    EXPECT_STREQ(res2, "");
}

TEST(evil_input_test, helicsInputGetUnits)
{
    // const char*  helicsInputGetUnits(HelicsInput ipt);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto res1 = helicsInputGetUnits(nullptr);
    EXPECT_STREQ(res1, "");
    auto res2 = helicsInputGetUnits(evil_input);
    EXPECT_STREQ(res2, "");
}

TEST(evil_input_test, helicsInputGetInjectionUnits)
{
    // const char*  helicsInputGetInjectionUnits(HelicsInput ipt);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto res1 = helicsInputGetInjectionUnits(nullptr);
    EXPECT_STREQ(res1, "");
    auto res2 = helicsInputGetInjectionUnits(evil_input);
    EXPECT_STREQ(res2, "");
}

TEST(evil_input_test, helicsInputGetExtractionUnits)
{
    // const char*  helicsInputGetExtractionUnits(HelicsInput ipt);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto res1 = helicsInputGetExtractionUnits(nullptr);
    EXPECT_STREQ(res1, "");
    auto res2 = helicsInputGetExtractionUnits(evil_input);
    EXPECT_STREQ(res2, "");
}

TEST(evil_input_test, helicsInputGetInfo)
{
    // const char*  helicsInputGetInfo(HelicsInput inp);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto res1 = helicsInputGetInfo(nullptr);
    EXPECT_STREQ(res1, "");
    auto res2 = helicsInputGetInfo(evil_input);
    EXPECT_STREQ(res2, "");
}

TEST(evil_input_test, helicsInputSetInfo)
{
    // void helicsInputSetInfo(HelicsInput inp, const char* info, HelicsError* err);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsInputSetInfo(nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsInputSetInfo(HelicsInput inp, const char* info, nullptr);
    helicsInputSetInfo(evil_input, "info", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_input_test, helicsInputSetMinimumChange)
{
    // void helicsInputSetMinimumChange(HelicsInput inp, double tolerance, HelicsError* err);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsInputSetMinimumChange(nullptr, 12.0, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsInputSetInfo(HelicsInput inp, const char* info, nullptr);
    helicsInputSetMinimumChange(evil_input, 12.0, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_input_test, helicsInputGetOption)
{
    // HelicsBool helicsInputGetOption(HelicsInput inp, int option);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto res1 = helicsInputGetOption(nullptr, 99);
    EXPECT_EQ(res1, HELICS_FALSE);
    auto res2 = helicsInputGetOption(evil_input, 5);
    EXPECT_EQ(res2, HELICS_FALSE);
}

TEST(evil_input_test, helicsInputSetOption)
{
    // void helicsInputSetOption(HelicsInput inp, int option, HelicsBool value, HelicsError*
    // err);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsInputSetOption(nullptr, 0, HELICS_TRUE, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsInputSetOption(HelicsInput inp, int option, HelicsBool value, nullptr);
    helicsInputSetOption(evil_input, 45, HELICS_TRUE, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    helicsInputSetOption(nullptr, 45, HELICS_TRUE, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_input_test, helicsInputIsUpdated)
{
    // HelicsBool helicsInputIsUpdated(HelicsInput ipt);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto res1 = helicsInputIsUpdated(nullptr);
    EXPECT_EQ(res1, HELICS_FALSE);
    auto res2 = helicsInputIsUpdated(evil_input);
    EXPECT_EQ(res2, HELICS_FALSE);
}

TEST(evil_input_test, helicsInputLastUpdateTime)
{
    // HelicsTime helicsInputLastUpdateTime(HelicsInput ipt);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    auto res1 = helicsInputLastUpdateTime(nullptr);
    EXPECT_DOUBLE_EQ(res1, HELICS_TIME_INVALID);
    auto res2 = helicsInputLastUpdateTime(evil_input);
    EXPECT_DOUBLE_EQ(res2, HELICS_TIME_INVALID);
}

TEST(evil_input_test, helicsInputClearUpdate)
{
    // void helicsInputClearUpdate(HelicsInput ipt);
    char rdata[256];
    auto evil_input = reinterpret_cast<HelicsInput>(rdata);
    EXPECT_NO_THROW(helicsInputClearUpdate(nullptr));
    EXPECT_NO_THROW(helicsInputClearUpdate(evil_input));
}

// section Message Federate Functions
// Functions applying to federates created as a value or combination federate \ref HelicsFederate
// objects
TEST(evil_message_fed_test, helicsFederateRegisterEndpoint)
{
    // HelicsEndpoint helicsFederateRegisterEndpoint(HelicsFederate fed, const char* name, const
    // char* type, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateRegisterEndpoint(nullptr, "name", "type", &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, nullptr);
    helicsErrorClear(&err);
    auto res2 = helicsFederateRegisterEndpoint(nullptr, nullptr, nullptr, nullptr);
    EXPECT_EQ(res2, nullptr);
    auto res3 = helicsFederateRegisterEndpoint(evil_federate, "name", "type", &err);
    EXPECT_EQ(res3, nullptr);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_message_fed_test, helicsFederateRegisterGlobalEndpoint)
{
    // HelicsEndpoint helicsFederateRegisterGlobalEndpoint(HelicsFederate fed, const char* name,
    // const char* type, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateRegisterGlobalEndpoint(nullptr, "name", "type", &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, nullptr);
    helicsErrorClear(&err);
    auto res2 = helicsFederateRegisterGlobalEndpoint(nullptr, nullptr, nullptr, nullptr);
    EXPECT_EQ(res2, nullptr);
    auto res3 = helicsFederateRegisterGlobalEndpoint(evil_federate, "name", "type", &err);
    EXPECT_EQ(res3, nullptr);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_message_fed_test, helicsFederateGetEndpoint)
{
    // HelicsEndpoint helicsFederateGetEndpoint(HelicsFederate fed, const char* name,
    // HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateGetEndpoint(nullptr, "name", &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, nullptr);
    helicsErrorClear(&err);
    auto res2 = helicsFederateGetEndpoint(nullptr, nullptr, nullptr);
    EXPECT_EQ(res2, nullptr);
    auto res3 = helicsFederateGetEndpoint(evil_federate, "name", &err);
    EXPECT_EQ(res3, nullptr);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_message_fed_test, helicsFederateGetEndpointByIndex)
{
    // HelicsEndpoint helicsFederateGetEndpointByIndex(HelicsFederate fed, int index,
    // HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateGetEndpointByIndex(nullptr, 0, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, nullptr);
    helicsErrorClear(&err);
    auto res2 = helicsFederateGetEndpointByIndex(nullptr, 0, nullptr);
    EXPECT_EQ(res2, nullptr);
    auto res3 = helicsFederateGetEndpointByIndex(evil_federate, 0, &err);
    EXPECT_EQ(res3, nullptr);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_message_fed_test, helicsFederateHasMessage)
{
    // HelicsBool helicsFederateHasMessage(HelicsFederate fed);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto res1 = helicsFederateHasMessage(nullptr);
    EXPECT_EQ(res1, HELICS_FALSE);
    auto res2 = helicsFederateHasMessage(evil_federate);
    EXPECT_EQ(res2, HELICS_FALSE);
}

TEST(evil_message_fed_test, helicsFederatePendingMessageCount)
{
    // int helicsFederatePendingMessages(HelicsFederate fed);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto res1 = helicsFederatePendingMessageCount(nullptr);
    EXPECT_EQ(res1, 0);
    auto res2 = helicsFederatePendingMessageCount(evil_federate);
    EXPECT_EQ(res2, 0);
}

TEST(evil_message_fed_test, helicsFederateGetMessageObject)
{
    // HelicsMessage helicsFederateGetMessageObject(HelicsFederate fed);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto res1 = helicsFederateGetMessage(nullptr);
    EXPECT_EQ(res1, nullptr);
    auto res2 = helicsFederateGetMessage(evil_federate);
    EXPECT_EQ(res2, nullptr);
}

TEST(evil_message_fed_test, helicsEndpointGetMessage)
{
    // HelicsMessage helicsEndpointGetMessage(HelicsFederate fed, HelicsError*
    // err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateCreateMessage(nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, nullptr);
    helicsErrorClear(&err);
    auto res2 = helicsFederateCreateMessage(nullptr, nullptr);
    EXPECT_EQ(res2, nullptr);
    auto res3 = helicsEndpointGetMessage(evil_federate);
    EXPECT_EQ(res3, nullptr);
}

TEST(evil_message_fed_test, helicsFederateClearMessages)
{
    // void helicsFederateClearMessages(HelicsFederate fed);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    EXPECT_NO_THROW(helicsFederateClearMessages(nullptr));
    EXPECT_NO_THROW(helicsFederateClearMessages(evil_federate));
}

TEST(evil_message_fed_test, helicsFederateGetEndpointCount)
{
    // int helicsFederateGetEndpointCount(HelicsFederate fed);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto res1 = helicsFederateGetEndpointCount(nullptr);
    EXPECT_EQ(res1, 0);
    auto res2 = helicsFederateGetEndpointCount(evil_federate);
    EXPECT_EQ(res2, 0);
}

// section Message object Functions
// functions applying to a \ref HelicsMessage
TEST(evil_message_object_test, helicsMessageGetSource)
{
    // const char*  helicsMessageGetSource(HelicsMessage message);
    char rdata[256];
    auto evil_mo = reinterpret_cast<HelicsMessage>(rdata);
    auto res1 = helicsMessageGetSource(nullptr);
    EXPECT_STREQ(res1, "");
    auto res2 = helicsMessageGetSource(evil_mo);
    EXPECT_STREQ(res2, "");
}

TEST(evil_message_object_test, helicsMessageGetDestination)
{
    // const char*  helicsMessageGetDestination(HelicsMessage message);
    char rdata[256];
    auto evil_mo = reinterpret_cast<HelicsMessage>(rdata);
    auto res1 = helicsMessageGetDestination(nullptr);
    EXPECT_STREQ(res1, "");
    auto res2 = helicsMessageGetDestination(evil_mo);
    EXPECT_STREQ(res2, "");
}

TEST(evil_message_object_test, helicsMessageGetOriginalSource)
{
    // const char*  helicsMessageGetOriginalSource(HelicsMessage message);
    char rdata[256];
    auto evil_mo = reinterpret_cast<HelicsMessage>(rdata);
    auto res1 = helicsMessageGetOriginalSource(nullptr);
    EXPECT_STREQ(res1, "");
    auto res2 = helicsMessageGetOriginalSource(evil_mo);
    EXPECT_STREQ(res2, "");
}

TEST(evil_message_object_test, helicsMessageGetOriginalDestination)
{
    // const char*  helicsMessageGetOriginalDestination(HelicsMessage message);
    char rdata[256];
    auto evil_mo = reinterpret_cast<HelicsMessage>(rdata);
    auto res1 = helicsMessageGetOriginalDestination(nullptr);
    EXPECT_STREQ(res1, "");
    auto res2 = helicsMessageGetOriginalDestination(evil_mo);
    EXPECT_STREQ(res2, "");
}

TEST(evil_message_object_test, helicsMessageGetTime)
{
    // HelicsTime helicsMessageGetTime(HelicsMessage message);
    char rdata[256];
    auto evil_mo = reinterpret_cast<HelicsMessage>(rdata);
    auto res1 = helicsMessageGetTime(nullptr);
    EXPECT_EQ(res1, HELICS_TIME_INVALID);
    auto res2 = helicsMessageGetTime(evil_mo);
    EXPECT_EQ(res2, HELICS_TIME_INVALID);
}

TEST(evil_message_object_test, helicsMessageGetString)
{
    // const char*  helicsMessageGetString(HelicsMessage message);
    char rdata[256];
    auto evil_mo = reinterpret_cast<HelicsMessage>(rdata);
    auto res1 = helicsMessageGetString(nullptr);
    EXPECT_STREQ(res1, "");
    auto res2 = helicsMessageGetString(evil_mo);
    EXPECT_STREQ(res2, "");
}

TEST(evil_message_object_test, helicsMessageGetMessageID)
{
    // int helicsMessageGetMessageID(HelicsMessage message);
    char rdata[256];
    auto evil_mo = reinterpret_cast<HelicsMessage>(rdata);
    auto res1 = helicsMessageGetMessageID(nullptr);
    EXPECT_EQ(res1, 0);
    auto res2 = helicsMessageGetMessageID(evil_mo);
    EXPECT_EQ(res2, 0);
}

TEST(evil_message_object_test, helicsMessageCheckFlag)
{
    // HelicsBool helicsMessageGetFlagOption(HelicsMessage message, int flag);
    char rdata[256];
    auto evil_mo = reinterpret_cast<HelicsMessage>(rdata);
    auto res1 = helicsMessageGetFlagOption(nullptr, 5);
    EXPECT_EQ(res1, HELICS_FALSE);
    auto res2 = helicsMessageGetFlagOption(evil_mo, 9);
    EXPECT_EQ(res2, HELICS_FALSE);
}

TEST(evil_message_object_test, helicsMessageGetByteCount)
{
    // int helicsMessageGetByteCount(HelicsMessage message);
    char rdata[256];
    auto evil_mo = reinterpret_cast<HelicsMessage>(rdata);
    auto res1 = helicsMessageGetByteCount(nullptr);
    EXPECT_EQ(res1, 0);
    auto res2 = helicsMessageGetByteCount(evil_mo);
    EXPECT_EQ(res2, 0);
}

TEST(evil_message_object_test, helicsMessageGetBytes)
{
    // void helicsMessageGetBytes(HelicsMessage message, void* data, int maxMessagelen,
    // int* actualSize, HelicsError* err);
    char rdata[256];
    auto evil_mo = reinterpret_cast<HelicsMessage>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    int actSize = 98;
    helicsMessageGetBytes(nullptr, nullptr, 55, &actSize, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(actSize, 0);
    helicsErrorClear(&err);
    actSize = 45;
    // auto res2=helicsMessageGetData(nullptr, void* data, int maxMessagelen, int* actualSize,
    // nullptr);
    helicsMessageGetBytes(evil_mo, nullptr, 22, &actSize, &err);
    EXPECT_EQ(actSize, 0);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_message_object_test, helicsMessageIsValid)
{
    // HelicsBool helicsMessageIsValid(HelicsMessage message);
    char rdata[256];
    auto evil_mo = reinterpret_cast<HelicsMessage>(rdata);
    auto res1 = helicsMessageIsValid(nullptr);
    EXPECT_EQ(res1, HELICS_FALSE);
    auto res2 = helicsMessageIsValid(evil_mo);
    EXPECT_EQ(res2, HELICS_FALSE);
}

TEST(evil_message_object_test, helicsMessageSetSource)
{
    // void helicsMessageSetSource(HelicsMessage message, const char* src, HelicsError*
    // err);
    char rdata[256];
    auto evil_mo = reinterpret_cast<HelicsMessage>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsMessageSetSource(nullptr, "src", &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsMessageSetSource(nullptr, const char* src, nullptr);
    helicsMessageSetSource(evil_mo, "src", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_message_object_test, helicsMessageCopy)
{
    // void helicsMessageSetSource(HelicsMessage message, const char* src, HelicsError*
    // err);
    char rdata[256];
    auto evil_mo = reinterpret_cast<HelicsMessage>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsMessageCopy(nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    helicsMessageCopy(nullptr, nullptr, &err);
    EXPECT_NE(err.error_code, 0);

    helicsMessageCopy(evil_mo, evil_mo, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_message_object_test, helicsMessageSetDestination)
{
    // void helicsMessageSetDestination(HelicsMessage message, const char* dest,
    // HelicsError* err);
    char rdata[256];
    auto evil_mo = reinterpret_cast<HelicsMessage>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsMessageSetDestination(nullptr, "dest", &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsMessageSetDestination(nullptr, const char* dest, nullptr);
    helicsMessageSetDestination(evil_mo, "dest", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_message_object_test, helicsMessageSetOriginalSource)
{
    // void helicsMessageSetOriginalSource(HelicsMessage message, const char* src,
    // HelicsError* err);
    char rdata[256];
    auto evil_mo = reinterpret_cast<HelicsMessage>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsMessageSetOriginalSource(nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsMessageSetOriginalSource(nullptr, const char* src, nullptr);
    helicsMessageSetOriginalSource(evil_mo, "osrc", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_message_object_test, helicsMessageSetOriginalDestination)
{
    // void helicsMessageSetOriginalDestination(HelicsMessage message, const char* dest,
    // HelicsError* err);
    char rdata[256];
    auto evil_mo = reinterpret_cast<HelicsMessage>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsMessageSetOriginalDestination(nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsMessageSetOriginalDestination(nullptr, const char* dest, nullptr);
    helicsMessageSetOriginalDestination(evil_mo, "odest", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_message_object_test, helicsMessageSetTime)
{
    // void helicsMessageSetTime(HelicsMessage message, HelicsTime time, HelicsError*
    // err);
    char rdata[256];
    auto evil_mo = reinterpret_cast<HelicsMessage>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsMessageSetTime(nullptr, 1.0, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsMessageSetTime(nullptr, HelicsTime time, nullptr);
    helicsMessageSetTime(evil_mo, HELICS_TIME_INVALID, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_message_object_test, helicsMessageResize)
{
    // void helicsMessageResize(HelicsMessage message, int newSize, HelicsError* err);
    char rdata[256];
    auto evil_mo = reinterpret_cast<HelicsMessage>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsMessageResize(nullptr, 5, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsMessageResize(nullptr, int newSize, nullptr);
    helicsMessageResize(evil_mo, 10, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_message_object_test, helicsMessageReserve)
{
    // void helicsMessageReserve(HelicsMessage message, int reserveSize, HelicsError* err);
    char rdata[256];
    auto evil_mo = reinterpret_cast<HelicsMessage>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsMessageReserve(nullptr, 9999, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsMessageReserve(nullptr, int reserveSize, nullptr);
    helicsMessageReserve(evil_mo, 9999, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_message_object_test, helicsMessageSetMessageID)
{
    // void helicsMessageSetMessageID(HelicsMessage message, int32_t messageID,
    // HelicsError* err);
    char rdata[256];
    auto evil_mo = reinterpret_cast<HelicsMessage>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsMessageSetMessageID(nullptr, 1, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsMessageSetMessageID(nullptr, int32_t messageID, nullptr);
    helicsMessageSetMessageID(evil_mo, 15, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_message_object_test, helicsMessageClearFlags)
{
    // void helicsMessageClearFlags(HelicsMessage message);
    char rdata[256];
    auto evil_mo = reinterpret_cast<HelicsMessage>(rdata);
    EXPECT_NO_THROW(helicsMessageClearFlags(nullptr));
    EXPECT_NO_THROW(helicsMessageClearFlags(evil_mo));
}

TEST(evil_message_object_test, helicsMessageSetFlagOption)
{
    // void helicsMessageSetFlagOption(HelicsMessage message, int flag, HelicsBool
    // flagValue, HelicsError* err);
    char rdata[256];
    auto evil_mo = reinterpret_cast<HelicsMessage>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsMessageSetFlagOption(nullptr, 5, HELICS_FALSE, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsMessageSetFlagOption(nullptr, int flag, HelicsBool flagValue, nullptr);
    helicsMessageSetFlagOption(evil_mo, 7, HELICS_TRUE, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_message_object_test, helicsMessageSetString)
{
    // void helicsMessageSetString(HelicsMessage message, const char* str, HelicsError*
    // err);
    char rdata[256];
    auto evil_mo = reinterpret_cast<HelicsMessage>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsMessageSetString(nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsMessageSetString(nullptr, const char* str, nullptr);
    helicsMessageSetString(evil_mo, "string", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_message_object_test, helicsMessageSetData)
{
    // void helicsMessageSetData(HelicsMessage message, const void* data, int
    // inputDataLength, HelicsError* err);
    char rdata[256];
    auto evil_mo = reinterpret_cast<HelicsMessage>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsMessageSetData(nullptr, nullptr, 99, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsMessageSetData(nullptr, nullptr, int inputDataLength, nullptr);
    helicsMessageSetData(evil_mo, rdata, 99, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_message_object_test, helicsMessageAppendData)
{
    // void helicsMessageAppendData(HelicsMessage message, const void* data, int
    // inputDataLength, HelicsError* err);
    char rdata[256];
    auto evil_mo = reinterpret_cast<HelicsMessage>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsMessageAppendData(nullptr, nullptr, 89, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsMessageAppendData(nullptr, nullptr, int inputDataLength, nullptr);
    helicsMessageAppendData(evil_mo, rdata, 100, &err);
    EXPECT_NE(err.error_code, 0);
}

// section Endpoint interface Functions
// functions applying to a \ref HelicsEndpoint object

TEST(evil_endpoint_test, helicsEndpointIsValid)
{
    // void helicsEndpointSetDefaultDestination(HelicsEndpoint endpoint, const char* dest,
    // HelicsError* err);
    char rdata[256];
    auto evil_ept = reinterpret_cast<HelicsEndpoint>(rdata);
    EXPECT_NE(helicsEndpointIsValid(nullptr), HELICS_TRUE);

    // auto res2=helicsEndpointSetDefaultDestination(nullptr, const char* dest, nullptr);
    EXPECT_NE(helicsEndpointIsValid(evil_ept), HELICS_TRUE);
}

TEST(evil_endpoint_test, helicsEndpointSetDefaultDestination)
{
    // void helicsEndpointSetDefaultDestination(HelicsEndpoint endpoint, const char* dest,
    // HelicsError* err);
    char rdata[256];
    auto evil_ept = reinterpret_cast<HelicsEndpoint>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsEndpointSetDefaultDestination(nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsEndpointSetDefaultDestination(nullptr, const char* dest, nullptr);
    helicsEndpointSetDefaultDestination(evil_ept, "dest", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_endpoint_test, helicsEndpointGetDefaultDestination)
{
    // const char*  helicsEndpointGetDefaultDestination(HelicsEndpoint endpoint);
    char rdata[256];
    auto evil_ept = reinterpret_cast<HelicsEndpoint>(rdata);
    auto res1 = helicsEndpointGetDefaultDestination(nullptr);
    EXPECT_STREQ(res1, "");
    auto res2 = helicsEndpointGetDefaultDestination(evil_ept);
    EXPECT_STREQ(res2, "");
}

TEST(evil_endpoint_test, helicsEndpointSendBytesTo)
{
    // void helicsEndpointSendMessage(HelicsEndpoint endpoint, const char* dest, const void*
    // data, int inputDataLength, HelicsError* err);
    char rdata[256];
    auto evil_ept = reinterpret_cast<HelicsEndpoint>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsEndpointSendBytesTo(nullptr, nullptr, 45, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsEndpointSendMessage(nullptr, nullptr, nullptr, int inputDataLength,
    // nullptr);
    helicsEndpointSendBytesTo(evil_ept, rdata, 200, "dest", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_endpoint_test, helicsEndpointSendBytesToAt)
{
    // void helicsEndpointSendMessage(HelicsEndpoint endpoint, const char* dest, const void*
    // data, int inputDataLength, HelicsError* err);
    char rdata[256];
    auto evil_ept = reinterpret_cast<HelicsEndpoint>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsEndpointSendBytesToAt(nullptr, nullptr, 45, nullptr, 0.0, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    helicsEndpointSendBytesToAt(evil_ept, rdata, 200, "dest", -15.7, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_endpoint_test, helicsEndpointSendBytesAt)
{
    // void helicsEndpointSendBytesToAt(     HelicsEndpoint endpoint,     const char* dest, const
    // void* data,     int inputDataLength,     HelicsTime time,     HelicsError* err);
    char rdata[256];
    auto evil_ept = reinterpret_cast<HelicsEndpoint>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsEndpointSendBytesAt(nullptr, nullptr, 25, 3.5, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsEndpointSendBytesToAt(     nullptr,    nullptr,    nullptr,     int
    // inputDataLength,     HelicsTime time,     nullptr);
    helicsEndpointSendBytesAt(evil_ept, rdata, 56, 3.5, &err);
    EXPECT_NE(err.error_code, 0);
}

// send string

TEST(evil_endpoint_test, helicsEndpointSendStringTo)
{
    // void helicsEndpointSendMessage(HelicsEndpoint endpoint, const char* dest, const void*
    // data, int inputDataLength, HelicsError* err);
    char rdata[256];
    auto evil_ept = reinterpret_cast<HelicsEndpoint>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsEndpointSendStringTo(nullptr, nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsEndpointSendMessage(nullptr, nullptr, nullptr, int inputDataLength,
    // nullptr);
    helicsEndpointSendStringTo(evil_ept, rdata, "dest", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_endpoint_test, helicsEndpointSendStringToAt)
{
    // void helicsEndpointStringToAt(HelicsEndpoint endpoint, const char* dest, const void*
    // data, int inputDataLength, HelicsError* err);
    char rdata[256];
    auto evil_ept = reinterpret_cast<HelicsEndpoint>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsEndpointSendStringToAt(nullptr, nullptr, nullptr, 0.0, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    helicsEndpointSendStringToAt(evil_ept, rdata, "dest", -15.7, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_endpoint_test, helicsEndpointSendStringAt)
{
    // void helicsEndpointSendStringAt(     HelicsEndpoint endpoint,     const char* dest, const
    // void* data,     int inputDataLength,     HelicsTime time,     HelicsError* err);
    char rdata[256];
    auto evil_ept = reinterpret_cast<HelicsEndpoint>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsEndpointSendStringAt(nullptr, nullptr, 3.5, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    helicsEndpointSendStringAt(evil_ept, rdata, 3.5, &err);
    EXPECT_NE(err.error_code, 0);
}
//

TEST(evil_endpoint_test, helicsEndpointSendMessageObject)
{
    // void helicsEndpointSendMessageObject(HelicsEndpoint endpoint, HelicsMessage message,
    // HelicsError* err);
    char rdata[256];
    auto evil_ept = reinterpret_cast<HelicsEndpoint>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsEndpointSendMessage(nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsEndpointSendMessageObject(nullptr, nullptr, nullptr);
    helicsEndpointSendMessage(evil_ept, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_endpoint_test, helicsEndpointSubscribe)
{
    // void helicsEndpointSubscribe(HelicsEndpoint endpoint, const char* key, HelicsError* err);
    char rdata[256];
    auto evil_ept = reinterpret_cast<HelicsEndpoint>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsEndpointSubscribe(nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsEndpointSubscribe(nullptr, nullptr, nullptr);
    helicsEndpointSubscribe(evil_ept, "key", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_endpoint_test, helicsEndpointHasMessage)
{
    // HelicsBool helicsEndpointHasMessage(HelicsEndpoint endpoint);
    char rdata[256];
    auto evil_ept = reinterpret_cast<HelicsEndpoint>(rdata);
    auto res1 = helicsEndpointHasMessage(nullptr);
    EXPECT_EQ(res1, HELICS_FALSE);
    auto res2 = helicsEndpointHasMessage(evil_ept);
    EXPECT_EQ(res2, HELICS_FALSE);
}

TEST(evil_endpoint_test, helicsEndpointPendingMessages)
{
    // int helicsEndpointPendingMessages(HelicsEndpoint endpoint);
    char rdata[256];
    auto evil_ept = reinterpret_cast<HelicsEndpoint>(rdata);
    auto res1 = helicsEndpointPendingMessageCount(nullptr);
    EXPECT_EQ(res1, 0);
    auto res2 = helicsEndpointPendingMessageCount(evil_ept);
    EXPECT_EQ(res2, 0);
}

TEST(evil_endpoint_test, helicsEndpointGetMessageObject)
{
    // HelicsMessage helicsEndpointGetMessageObject(HelicsEndpoint endpoint);
    char rdata[256];
    auto evil_ept = reinterpret_cast<HelicsEndpoint>(rdata);
    auto res1 = helicsEndpointGetMessage(nullptr);
    EXPECT_EQ(res1, nullptr);
    auto res2 = helicsEndpointGetMessage(evil_ept);
    EXPECT_EQ(res2, nullptr);
}

TEST(evil_endpoint_test, helicsEndpointGetType)
{
    // const char*  helicsEndpointGetType(HelicsEndpoint endpoint);
    char rdata[256];
    auto evil_ept = reinterpret_cast<HelicsEndpoint>(rdata);
    auto res1 = helicsEndpointGetType(nullptr);
    EXPECT_STREQ(res1, "");
    auto res2 = helicsEndpointGetType(evil_ept);
    EXPECT_STREQ(res2, "");
}

TEST(evil_endpoint_test, helicsEndpointGetName)
{
    // const char*  helicsEndpointGetName(HelicsEndpoint endpoint);
    char rdata[256];
    auto evil_ept = reinterpret_cast<HelicsEndpoint>(rdata);
    auto res1 = helicsEndpointGetName(nullptr);
    EXPECT_STREQ(res1, "");
    auto res2 = helicsEndpointGetName(evil_ept);
    EXPECT_STREQ(res2, "");
}

TEST(evil_endpoint_test, helicsEndpointGetInfo)
{
    // const char*  helicsEndpointGetInfo(HelicsEndpoint end);
    char rdata[256];
    auto evil_ept = reinterpret_cast<HelicsEndpoint>(rdata);
    auto res1 = helicsEndpointGetInfo(nullptr);
    EXPECT_STREQ(res1, "");
    auto res2 = helicsEndpointGetInfo(evil_ept);
    EXPECT_STREQ(res2, "");
}

TEST(evil_endpoint_test, helicsEndpointSetInfo)
{
    // void helicsEndpointSetInfo(HelicsEndpoint end, const char* info, HelicsError* err);
    char rdata[256];
    auto evil_ept = reinterpret_cast<HelicsEndpoint>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsEndpointSetInfo(nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsEndpointSetInfo(nullptr, nullptr, nullptr);
    helicsEndpointSetInfo(evil_ept, "info", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);

    helicsEndpointSetInfo(nullptr, "info", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_endpoint_test, helicsEndpointSetOption)
{
    // void helicsEndpointSetOption(HelicsEndpoint end, int option, HelicsBool value,
    // HelicsError* err);
    char rdata[256];
    auto evil_ept = reinterpret_cast<HelicsEndpoint>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsEndpointSetOption(nullptr, 5, HELICS_FALSE, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsEndpointSetOption(nullptr, int option, HelicsBool value, nullptr);
    helicsEndpointSetOption(evil_ept, 2, HELICS_TRUE, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_endpoint_test, helicsEndpointGetOption)
{
    // HelicsBool helicsEndpointGetOption(HelicsEndpoint end, int option);
    char rdata[256];
    auto evil_ept = reinterpret_cast<HelicsEndpoint>(rdata);
    auto res1 = helicsEndpointGetOption(nullptr, 5);
    EXPECT_EQ(res1, HELICS_FALSE);
    auto res2 = helicsEndpointGetOption(evil_ept, 0);
    EXPECT_EQ(res2, HELICS_FALSE);
}

// section Filter Federate Functions
// Functions applying to all federates but related to \ref HelicsFilter
TEST(evil_filter_fed_test, helicsFederateRegisterFilter)
{
    // HelicsFilter helicsFederateRegisterFilter(HelicsFederate fed, HelicsFilterTypes type,
    // const char* name, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateRegisterFilter(nullptr, HELICS_FILTER_TYPE_DELAY, "name", &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, nullptr);
    helicsErrorClear(&err);
    auto res2 = helicsFederateRegisterFilter(nullptr, HELICS_FILTER_TYPE_DELAY, nullptr, nullptr);
    EXPECT_EQ(res2, nullptr);
    auto res3 = helicsFederateRegisterFilter(evil_federate, HELICS_FILTER_TYPE_DELAY, "name", &err);
    EXPECT_EQ(res3, nullptr);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_filter_fed_test, helicsFederateRegisterGlobalFilter)
{
    // HelicsFilter helicsFederateRegisterGlobalFilter(HelicsFederate fed, HelicsFilterTypes
    // type, const char* name, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateRegisterGlobalFilter(nullptr, HELICS_FILTER_TYPE_DELAY, "name", &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, nullptr);
    helicsErrorClear(&err);
    auto res2 =
        helicsFederateRegisterGlobalFilter(nullptr, HELICS_FILTER_TYPE_DELAY, nullptr, nullptr);
    EXPECT_EQ(res2, nullptr);
    auto res3 =
        helicsFederateRegisterGlobalFilter(evil_federate, HELICS_FILTER_TYPE_DELAY, "name", &err);
    EXPECT_EQ(res3, nullptr);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_filter_fed_test, helicsFederateRegisterCloningFilter)
{
    // HelicsFilter helicsFederateRegisterCloningFilter(HelicsFederate fed, const char*
    // deliveryEndpoint, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateRegisterCloningFilter(nullptr, "deliver", &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, nullptr);
    helicsErrorClear(&err);
    auto res2 = helicsFederateRegisterCloningFilter(nullptr, nullptr, nullptr);
    EXPECT_EQ(res2, nullptr);
    auto res3 = helicsFederateRegisterCloningFilter(evil_federate, "deliver", &err);
    EXPECT_EQ(res3, nullptr);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_filter_fed_test, helicsFederateRegisterGlobalCloningFilter)
{
    // HelicsFilter helicsFederateRegisterGlobalCloningFilter(HelicsFederate fed, const char*
    // deliveryEndpoint, HelicsError* err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateRegisterGlobalCloningFilter(nullptr, "deliver", &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, nullptr);
    helicsErrorClear(&err);
    auto res2 = helicsFederateRegisterGlobalCloningFilter(nullptr, nullptr, nullptr);
    EXPECT_EQ(res2, nullptr);
    auto res3 = helicsFederateRegisterGlobalCloningFilter(evil_federate, "deliver", &err);
    EXPECT_EQ(res3, nullptr);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_filter_fed_test, helicsFederateGetFilterCount)
{
    // int helicsFederateGetFilterCount(HelicsFederate fed);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto res1 = helicsFederateGetFilterCount(nullptr);
    EXPECT_EQ(res1, 0);
    auto res2 = helicsFederateGetFilterCount(evil_federate);
    EXPECT_EQ(res2, 0);
}

TEST(evil_filter_fed_test, helicsFederateGetFilter)
{
    // HelicsFilter helicsFederateGetFilter(HelicsFederate fed, const char* name, HelicsError*
    // err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateGetFilter(nullptr, "name", &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, nullptr);
    helicsErrorClear(&err);
    auto res2 = helicsFederateGetFilter(nullptr, nullptr, nullptr);
    EXPECT_EQ(res2, nullptr);
    auto res3 = helicsFederateGetFilter(evil_federate, "name", &err);
    EXPECT_EQ(res3, nullptr);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_filter_fed_test, helicsFederateGetFilterByIndex)
{
    // HelicsFilter helicsFederateGetFilterByIndex(HelicsFederate fed, int index, HelicsError*
    // err);
    char rdata[256];
    auto evil_federate = reinterpret_cast<HelicsFederate>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsFederateGetFilterByIndex(nullptr, 0, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_EQ(res1, nullptr);
    helicsErrorClear(&err);
    auto res2 = helicsFederateGetFilterByIndex(nullptr, 0, nullptr);
    EXPECT_EQ(res2, nullptr);
    auto res3 = helicsFederateGetFilterByIndex(evil_federate, 0, &err);
    EXPECT_EQ(res3, nullptr);
    EXPECT_NE(err.error_code, 0);
}

// section Filter interface Functions
// Functions applying to a \ref HelicsFilter object

TEST(evil_filter_test, helicsFilterIsValid)
{
    char rdata[256];
    auto evil_filt = reinterpret_cast<HelicsFilter>(rdata);
    EXPECT_NE(helicsFilterIsValid(nullptr), HELICS_TRUE);
    EXPECT_NE(helicsFilterIsValid(evil_filt), HELICS_TRUE);
}

TEST(evil_filter_test, helicsFilterGetName)
{
    // const char*  helicsFilterGetName(HelicsFilter filt);
    char rdata[256];
    auto evil_filt = reinterpret_cast<HelicsFilter>(rdata);
    auto res1 = helicsFilterGetName(nullptr);
    EXPECT_STREQ(res1, "");
    auto res2 = helicsFilterGetName(evil_filt);
    EXPECT_STREQ(res2, "");
}

TEST(evil_filter_test, helicsFilterSet)
{
    // void helicsFilterSet(HelicsFilter filt, const char* prop, double val, HelicsError* err);
    char rdata[256];
    auto evil_filt = reinterpret_cast<HelicsFilter>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFilterSet(nullptr, nullptr, 5.3, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFilterSet(HelicsFilter filt, const char* prop, double val, nullptr);
    helicsFilterSet(evil_filt, "prop", 5.2, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    helicsFilterSet(nullptr, "prop", 5.2, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_filter_test, helicsFilterSetString)
{
    // void helicsFilterSetString(HelicsFilter filt, const char* prop, const char* val,
    // HelicsError* err);
    char rdata[256];
    auto evil_filt = reinterpret_cast<HelicsFilter>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFilterSetString(nullptr, nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFilterSetString(HelicsFilter filt, const char* prop, const char* val,
    // nullptr);
    helicsFilterSetString(evil_filt, "prop", "val", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_filter_test, helicsFilterAddDestinationTarget)
{
    // void helicsFilterAddDestinationTarget(HelicsFilter filt, const char* dest, HelicsError*
    // err);
    char rdata[256];
    auto evil_filt = reinterpret_cast<HelicsFilter>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFilterAddDestinationTarget(nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFilterAddDestinationTarget(HelicsFilter filt, const char* dest, nullptr);
    helicsFilterAddDestinationTarget(evil_filt, "dest", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_filter_test, helicsFilterAddSourceTarget)
{
    // void helicsFilterAddSourceTarget(HelicsFilter filt, const char* source, HelicsError* err);
    char rdata[256];
    auto evil_filt = reinterpret_cast<HelicsFilter>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFilterAddSourceTarget(nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFilterAddSourceTarget(HelicsFilter filt, const char* source, nullptr);
    helicsFilterAddSourceTarget(evil_filt, "source", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_filter_test, helicsFilterAddDeliveryEndpoint)
{
    // void helicsFilterAddDeliveryEndpoint(HelicsFilter filt, const char* deliveryEndpoint,
    // HelicsError* err);
    char rdata[256];
    auto evil_filt = reinterpret_cast<HelicsFilter>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFilterAddDeliveryEndpoint(nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFilterAddDeliveryEndpoint(HelicsFilter filt, const char* deliveryEndpoint,
    // nullptr);
    helicsFilterAddDeliveryEndpoint(evil_filt, "deliver", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_filter_test, helicsFilterRemoveTarget)
{
    // void helicsFilterRemoveTarget(HelicsFilter filt, const char* target, HelicsError* err);
    char rdata[256];
    auto evil_filt = reinterpret_cast<HelicsFilter>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFilterRemoveTarget(nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFilterRemoveTarget(HelicsFilter filt, const char* target, nullptr);
    helicsFilterRemoveTarget(evil_filt, "target", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_filter_test, helicsFilterRemoveDeliveryEndpoint)
{
    // void helicsFilterRemoveDeliveryEndpoint(HelicsFilter filt, const char* deliveryEndpoint,
    // HelicsError* err);
    char rdata[256];
    auto evil_filt = reinterpret_cast<HelicsFilter>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFilterRemoveDeliveryEndpoint(nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFilterRemoveDeliveryEndpoint(HelicsFilter filt, const char*
    // deliveryEndpoint, nullptr);
    helicsFilterRemoveDeliveryEndpoint(evil_filt, "deliver", &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    helicsFilterRemoveDeliveryEndpoint(nullptr, "deliver", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_filter_test, helicsFilterGetInfo)
{
    // const char*  helicsFilterGetInfo(HelicsFilter filt);
    char rdata[256];
    auto evil_filt = reinterpret_cast<HelicsFilter>(rdata);
    auto res1 = helicsFilterGetInfo(nullptr);
    EXPECT_STREQ(res1, "");
    auto res2 = helicsFilterGetInfo(evil_filt);
    EXPECT_STREQ(res2, "");
}

TEST(evil_filter_test, helicsFilterSetInfo)
{
    // void helicsFilterSetInfo(HelicsFilter filt, const char* info, HelicsError* err);
    char rdata[256];
    auto evil_filt = reinterpret_cast<HelicsFilter>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFilterSetInfo(nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFilterSetInfo(HelicsFilter filt, const char* info, nullptr);
    helicsFilterSetInfo(evil_filt, "info", &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_filter_test, helicsFilterSetOption)
{
    // void helicsFilterSetOption(HelicsFilter filt, int option, HelicsBool value, HelicsError*
    // err);
    char rdata[256];
    auto evil_filt = reinterpret_cast<HelicsFilter>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFilterSetOption(nullptr, 0, HELICS_TRUE, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    // auto res2=helicsFilterSetOption(HelicsFilter filt, int option, HelicsBool value, nullptr);
    helicsFilterSetOption(evil_filt, 5, HELICS_TRUE, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_filter_test, helicsFilterGetOption)
{
    // HelicsBool helicsFilterGetOption(HelicsFilter filt, int option);
    char rdata[256];
    auto evil_filt = reinterpret_cast<HelicsFilter>(rdata);
    auto res1 = helicsFilterGetOption(nullptr, 0);
    EXPECT_EQ(res1, HELICS_FALSE);
    auto res2 = helicsFilterGetOption(evil_filt, 5);
    EXPECT_EQ(res2, HELICS_FALSE);
}

TEST(evil_filter_test, helicsFilterSetCallback)
{
    // void helicsFilterSetCustomCallback(HelicsFilter filt, callback, void *userdata,
    // HelicsError* err);
    char rdata[256];
    auto evil_filt = reinterpret_cast<HelicsFilter>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsFilterSetCustomCallback(nullptr, nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    helicsFilterSetCustomCallback(nullptr, nullptr, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
    helicsErrorClear(&err);
    helicsFilterSetCustomCallback(evil_filt, nullptr, nullptr, &err);
    EXPECT_NE(err.error_code, 0);
}

// section Query Functions
// functions applying to a \ref HelicsQuery object
TEST(evil_query_test, helicsQueryExecute)
{
    // const char*  helicsQueryExecute(HelicsQuery query, HelicsFederate fed, HelicsError* err);
    char rdata[256];
    auto evil_query = reinterpret_cast<HelicsQuery>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsQueryExecute(nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_NE(std::string_view(res1).find("error"), std::string_view::npos);
    helicsErrorClear(&err);
    auto res2 = helicsQueryExecute(nullptr, nullptr, nullptr);
    EXPECT_NE(std::string_view(res2).find("error"), std::string_view::npos);
    auto res3 = helicsQueryExecute(evil_query, evil_query, &err);
    EXPECT_NE(std::string_view(res3).find("error"), std::string_view::npos);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_query_test, helicsQueryCoreExecute)
{
    char rdata[256];
    auto evil_query = reinterpret_cast<HelicsQuery>(rdata);
    // const char*  helicsQueryCoreExecute(HelicsQuery query, HelicsCore core, HelicsError* err);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsQueryCoreExecute(nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_NE(std::string_view(res1).find("error"), std::string_view::npos);
    helicsErrorClear(&err);
    auto res2 = helicsQueryCoreExecute(nullptr, nullptr, nullptr);
    EXPECT_NE(std::string_view(res2).find("error"), std::string_view::npos);
    auto res3 = helicsQueryCoreExecute(evil_query, evil_query, &err);
    EXPECT_NE(std::string_view(res3).find("error"), std::string_view::npos);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_query_test, helicsQueryBrokerExecute)
{
    char rdata[256];
    auto evil_query = reinterpret_cast<HelicsQuery>(rdata);
    // const char*  helicsQueryBrokerExecute(HelicsQuery query, HelicsBroker broker, HelicsError*
    // err);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsQueryBrokerExecute(nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_NE(std::string_view(res1).find("error"), std::string_view::npos);
    helicsErrorClear(&err);
    auto res2 = helicsQueryBrokerExecute(nullptr, nullptr, nullptr);
    EXPECT_NE(std::string_view(res2).find("error"), std::string_view::npos);
    auto res3 = helicsQueryBrokerExecute(evil_query, evil_query, &err);
    EXPECT_NE(std::string_view(res3).find("error"), std::string_view::npos);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_query_test, helicsQueryExecuteAsync)
{
    // void helicsQueryExecuteAsync(HelicsQuery query, HelicsFederate fed, HelicsError* err);
    char rdata[256];
    auto evil_query = reinterpret_cast<HelicsQuery>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    helicsQueryExecuteAsync(nullptr, nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    helicsErrorClear(&err);
    helicsQueryExecuteAsync(evil_query, evil_query, &err);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_query_test, helicsQueryExecuteComplete)
{
    // const char*  helicsQueryExecuteComplete(HelicsQuery query, HelicsError* err);
    char rdata[256];
    auto evil_query = reinterpret_cast<HelicsQuery>(rdata);
    auto err = helicsErrorInitialize();
    err.error_code = 45;
    auto res1 = helicsQueryExecuteComplete(nullptr, &err);
    EXPECT_EQ(err.error_code, 45);
    EXPECT_NE(std::string_view(res1).find("error"), std::string_view::npos);
    helicsErrorClear(&err);
    auto res2 = helicsQueryExecuteComplete(nullptr, nullptr);
    EXPECT_NE(std::string_view(res2).find("error"), std::string_view::npos);
    auto res3 = helicsQueryExecuteComplete(evil_query, &err);
    EXPECT_NE(std::string_view(res3).find("error"), std::string_view::npos);
    EXPECT_NE(err.error_code, 0);
}

TEST(evil_query_test, helicsQueryIsCompleted)
{
    // HelicsBool helicsQueryIsCompleted(HelicsQuery query);
    char rdata[256];
    auto evil_query = reinterpret_cast<HelicsQuery>(rdata);
    auto res1 = helicsQueryIsCompleted(nullptr);
    EXPECT_EQ(res1, HELICS_FALSE);
    auto res2 = helicsQueryIsCompleted(evil_query);
    EXPECT_EQ(res2, HELICS_FALSE);
}

TEST(evil_query_test, helicsQueryFree)
{
    // void helicsQueryFree(HelicsQuery query);
    char rdata[256];
    auto evil_query = reinterpret_cast<HelicsQuery>(rdata);
    EXPECT_NO_THROW(helicsQueryFree(nullptr));
    EXPECT_NO_THROW(helicsQueryFree(evil_query));
}

// end generated code
