/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "ctestFixtures.hpp"

#include <complex>
#include <gtest/gtest.h>
#include <string>
#include <thread>
#include <vector>
// misc test copying some of the JAVA/Python tests

#ifdef HELICS_ENABLE_ZMQ_CORE
/** test simple creation and destruction*/
TEST(misc_tests, misc_tests)
{
    auto fedInfo1 = helicsCreateFederateInfo();
    helicsFederateInfoSetCoreInitString(fedInfo1, "-f 1", nullptr);
    helicsFederateInfoSetCoreName(fedInfo1, "core3", nullptr);
    helicsFederateInfoSetCoreType(fedInfo1, 3, nullptr);
    helicsFederateInfoSetCoreTypeFromString(fedInfo1, "zmq", nullptr);
    helicsFederateInfoSetFlagOption(fedInfo1, 1, HELICS_TRUE, nullptr);
    helicsFederateInfoSetTimeProperty(fedInfo1, HELICS_PROPERTY_TIME_INPUT_DELAY, 1.0, nullptr);
    helicsFederateInfoSetIntegerProperty(fedInfo1,
                                         HELICS_PROPERTY_INT_LOG_LEVEL,
                                         HELICS_LOG_LEVEL_WARNING,
                                         nullptr);
    helicsFederateInfoSetIntegerProperty(fedInfo1,
                                         HELICS_PROPERTY_INT_MAX_ITERATIONS,
                                         100,
                                         nullptr);
    helicsFederateInfoSetTimeProperty(fedInfo1, HELICS_PROPERTY_TIME_OUTPUT_DELAY, 1.0, nullptr);
    helicsFederateInfoSetTimeProperty(fedInfo1, HELICS_PROPERTY_TIME_PERIOD, 1.0, nullptr);
    helicsFederateInfoSetTimeProperty(fedInfo1, HELICS_PROPERTY_TIME_DELTA, 1.0, nullptr);
    helicsFederateInfoSetTimeProperty(fedInfo1, HELICS_PROPERTY_TIME_OFFSET, 0.1, nullptr);
    helicsFederateInfoFree(fedInfo1);

    auto broker3 =
        helicsCreateBroker("zmq", "broker3", "--federates 1 --loglevel warning", nullptr);
    EXPECT_STREQ(helicsBrokerGetIdentifier(broker3), "broker3");
    auto fedInfo2 = helicsCreateFederateInfo();
    const char* coreInitString = "--federates 1";
    helicsFederateInfoSetCoreInitString(fedInfo2, coreInitString, nullptr);
    helicsFederateInfoSetCoreTypeFromString(fedInfo2, "zmq", nullptr);
    helicsFederateInfoSetIntegerProperty(fedInfo2,
                                         HELICS_PROPERTY_INT_LOG_LEVEL,
                                         HELICS_LOG_LEVEL_WARNING,
                                         nullptr);
    helicsFederateInfoSetTimeProperty(fedInfo2, HELICS_PROPERTY_TIME_DELTA, 1.0, nullptr);
    auto fed1 = helicsCreateCombinationFederate("fed1", fedInfo2, nullptr);
    auto fed2 = helicsFederateClone(fed1, nullptr);
    helicsFederateInfoFree(fedInfo2);
    helicsGetFederateByName("fed1", nullptr);
    helicsFederateSetFlagOption(fed2, 1, HELICS_FALSE, nullptr);

    helicsFederateSetTimeProperty(fed2, HELICS_PROPERTY_TIME_INPUT_DELAY, 1.0, nullptr);
    helicsFederateSetIntegerProperty(fed1,
                                     HELICS_PROPERTY_INT_LOG_LEVEL,
                                     HELICS_LOG_LEVEL_WARNING,
                                     nullptr);
    helicsFederateSetIntegerProperty(fed2, HELICS_PROPERTY_INT_MAX_ITERATIONS, 100, nullptr);
    helicsFederateSetTimeProperty(fed2, HELICS_PROPERTY_TIME_OUTPUT_DELAY, 1.0, nullptr);
    helicsFederateSetTimeProperty(fed2, HELICS_PROPERTY_TIME_PERIOD, 0.0, nullptr);
    helicsFederateSetTimeProperty(fed2, HELICS_PROPERTY_TIME_DELTA, 1.0, nullptr);

    helicsFederateRegisterCloningFilter(fed1, "fed1/Ep1", nullptr);
    auto fed1DestinationFilter = helicsFederateRegisterFilter(fed1,
                                                              HELICS_FILTER_TYPE_DELAY,
                                                              "fed1DestinationFilter",
                                                              nullptr);
    helicsFilterAddDestinationTarget(fed1DestinationFilter, "Ep2", nullptr);

    auto ep1 = helicsFederateRegisterEndpoint(fed1, "Ep1", "string", nullptr);
    auto ep2 = helicsFederateRegisterGlobalEndpoint(fed1, "Ep2", "string", nullptr);
    auto pub1 =
        helicsFederateRegisterGlobalPublication(fed1, "pub1", HELICS_DATA_TYPE_DOUBLE, "", nullptr);
    auto pub2 = helicsFederateRegisterGlobalTypePublication(fed1, "pub2", "complex", "", nullptr);

    auto sub1 = helicsFederateRegisterSubscription(fed1, "pub1", nullptr, nullptr);
    auto sub2 = helicsFederateRegisterSubscription(fed1, "pub2", nullptr, nullptr);
    helicsInputAddTarget(sub2, "Ep2", nullptr);
    auto pub3 =
        helicsFederateRegisterPublication(fed1, "pub3", HELICS_DATA_TYPE_STRING, "", nullptr);

    auto pub1KeyString = helicsPublicationGetName(pub1);
    auto pub1TypeString = helicsPublicationGetType(pub1);
    auto pub1UnitsString = helicsPublicationGetUnits(pub1);
    auto sub1KeyString = helicsInputGetTarget(sub1);
    auto sub1UnitsString = helicsInputGetUnits(sub1);
    EXPECT_STREQ("pub1", pub1KeyString);
    EXPECT_STREQ("double", pub1TypeString);
    EXPECT_STREQ("", pub1UnitsString);
    EXPECT_STREQ("pub1", sub1KeyString);
    EXPECT_STREQ("", sub1UnitsString);

    auto fed1SourceFilter =
        helicsFederateRegisterFilter(fed1, HELICS_FILTER_TYPE_DELAY, "fed1SourceFilter", nullptr);
    helicsFilterAddSourceTarget(fed1SourceFilter, "Ep2", nullptr);
    helicsFilterAddDestinationTarget(fed1SourceFilter, "fed1/Ep1", nullptr);
    helicsFilterRemoveTarget(fed1SourceFilter, "fed1/Ep1", nullptr);
    helicsFilterAddSourceTarget(fed1SourceFilter, "Ep2", nullptr);
    helicsFilterRemoveTarget(fed1SourceFilter, "Ep2", nullptr);

    auto fed1SourceFilterNameString = helicsFilterGetName(fed1SourceFilter);
    EXPECT_STREQ(fed1SourceFilterNameString, "fed1/fed1SourceFilter");

    auto sub3 = helicsFederateRegisterSubscription(fed1, "fed1/pub3", "", nullptr);
    auto pub4 = helicsFederateRegisterTypePublication(fed1, "pub4", "int", "", nullptr);

    auto sub4 = helicsFederateRegisterSubscription(fed1, "fed1/pub4", "", nullptr);
    auto pub5 = helicsFederateRegisterGlobalTypePublication(fed1, "pub5", "boolean", "", nullptr);

    auto sub5 = helicsFederateRegisterSubscription(fed1, "pub5", "", nullptr);
    auto pub6 =
        helicsFederateRegisterGlobalPublication(fed1, "pub6", HELICS_DATA_TYPE_VECTOR, "", nullptr);
    auto sub6 = helicsFederateRegisterSubscription(fed1, "pub6", "", nullptr);
    auto pub7 = helicsFederateRegisterGlobalPublication(
        fed1, "pub7", HELICS_DATA_TYPE_NAMED_POINT, "", nullptr);
    auto sub7 = helicsFederateRegisterSubscription(fed1, "pub7", "", nullptr);

    helicsInputSetDefaultBoolean(sub5, HELICS_FALSE, nullptr);
    helicsInputSetDefaultComplex(sub2, -9.9, 2.5, nullptr);
    helicsInputSetDefaultDouble(sub1, 3.4, nullptr);
    helicsInputSetDefaultInteger(sub4, 6, nullptr);
    helicsInputSetDefaultNamedPoint(sub7, "hollow", 20.0, nullptr);
    helicsInputSetDefaultString(sub3, "default", nullptr);
    double sub6Default[] = {3.4, 90.9, 4.5};
    helicsInputSetDefaultVector(sub6, sub6Default, 3, nullptr);
    helicsEndpointSubscribe(ep2, "fed1/pub3", nullptr);
    helicsFederateEnterInitializingModeAsync(fed1, nullptr);
    auto rs = helicsFederateIsAsyncOperationCompleted(fed1, nullptr);
    if (rs == HELICS_FALSE) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        rs = helicsFederateIsAsyncOperationCompleted(fed1, nullptr);
        if (rs == HELICS_FALSE) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            rs = helicsFederateIsAsyncOperationCompleted(fed1, nullptr);
            ASSERT_EQ(rs, HELICS_TRUE);
        }
    }
    helicsFederateEnterInitializingModeComplete(fed1, nullptr);
    helicsFederateEnterExecutingModeAsync(fed1, nullptr);
    helicsFederateEnterExecutingModeComplete(fed1, nullptr);

    auto mesg1 = helicsFederateCreateMessage(fed1, nullptr);
    helicsMessageSetString(mesg1, "Hello", nullptr);
    helicsMessageSetSource(mesg1, "fed1/Ep1", nullptr);
    helicsMessageSetOriginalSource(mesg1, "fed1/Ep1", nullptr);
    helicsMessageSetDestination(mesg1, "Ep2", nullptr);
    helicsMessageSetOriginalDestination(mesg1, "Ep2", nullptr);

    helicsEndpointSendMessage(ep1, mesg1, nullptr);
    mesg1 = helicsFederateCreateMessage(fed1, nullptr);
    helicsMessageSetString(mesg1, "There", nullptr);
    helicsMessageSetSource(mesg1, "fed1/Ep1", nullptr);
    helicsMessageSetOriginalSource(mesg1, "fed1/Ep1", nullptr);
    helicsMessageSetDestination(mesg1, "Ep2", nullptr);
    helicsMessageSetOriginalDestination(mesg1, "Ep2", nullptr);
    helicsEndpointSendMessage(ep1, mesg1, nullptr);
    helicsEndpointSetDefaultDestination(ep2, "fed1/Ep1", nullptr);

    auto ep1NameString = helicsEndpointGetName(ep1);
    auto ep1TypeString = helicsEndpointGetType(ep1);

    EXPECT_STREQ(ep1NameString, "fed1/Ep1");
    EXPECT_STREQ(ep1TypeString, "string");

    helicsFederateGetCore(fed1, nullptr);

    auto fed1Time = helicsFederateGetCurrentTime(fed1, nullptr);
    EXPECT_EQ(fed1Time, 0.0);
    auto fed1EndpointCount = helicsFederateGetEndpointCount(fed1);
    EXPECT_EQ(fed1EndpointCount, 2);

    auto fed1NameString = helicsFederateGetName(fed1);
    EXPECT_STREQ(fed1NameString, "fed1");

    auto fed1State = helicsFederateGetState(fed1, nullptr);
    EXPECT_EQ(fed1State, 2);
    auto fed1PubCount = helicsFederateGetPublicationCount(fed1);
    EXPECT_EQ(fed1PubCount, 7);
    auto fed1SubCount = helicsFederateGetInputCount(fed1);
    EXPECT_EQ(fed1SubCount, 7);

    helicsPublicationPublishBoolean(pub5, HELICS_TRUE, nullptr);
    helicsPublicationPublishComplex(pub2, 5.6, -0.67, nullptr);
    helicsPublicationPublishDouble(pub1, 457.234, nullptr);
    helicsPublicationPublishInteger(pub4, 1, nullptr);
    helicsPublicationPublishNamedPoint(pub7, "Blah Blah", 20.0, nullptr);
    helicsPublicationPublishString(pub3, "Mayhem", nullptr);
    double pub6Vector[] = {4.5, 56.5};
    helicsPublicationPublishVector(pub6, pub6Vector, 2, nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    helicsFederateRequestTimeAsync(fed1, 1.0, nullptr);

    auto returnTime = helicsFederateRequestTimeComplete(fed1, nullptr);
    EXPECT_EQ(returnTime, 1.0);
    auto ep2MsgCount = helicsEndpointPendingMessageCount(ep2);
    EXPECT_EQ(ep2MsgCount, 0);

    returnTime = helicsFederateRequestTime(fed1, 3.0, nullptr);
    EXPECT_EQ(returnTime, 3.0);
    ep2MsgCount = helicsEndpointPendingMessageCount(ep2);
    EXPECT_EQ(ep2MsgCount, 3);

    helicsFederateDestroy(fed1);
    helicsBrokerDestroy(broker3);
}

#endif

TEST(type_conversion, namedType)
{
    EXPECT_TRUE(helicsGetDataType("int") == HELICS_DATA_TYPE_INT);
    EXPECT_TRUE(helicsGetDataType("INT") == HELICS_DATA_TYPE_INT);
    EXPECT_TRUE(helicsGetDataType("char") == HELICS_DATA_TYPE_CHAR);
    EXPECT_TRUE(helicsGetDataType(typeid(int).name()) == HELICS_DATA_TYPE_INT);
    EXPECT_TRUE(helicsGetDataType(typeid(float).name()) == HELICS_DATA_TYPE_DOUBLE);
    EXPECT_TRUE(helicsGetDataType(typeid(std::string).name()) == HELICS_DATA_TYPE_STRING);
    EXPECT_TRUE(helicsGetDataType(typeid(char*).name()) == HELICS_DATA_TYPE_STRING);
    EXPECT_TRUE(helicsGetDataType(typeid(const char*).name()) == HELICS_DATA_TYPE_STRING);
    EXPECT_TRUE(helicsGetDataType(typeid(double).name()) == HELICS_DATA_TYPE_DOUBLE);
    EXPECT_TRUE(helicsGetDataType(typeid(bool).name()) == HELICS_DATA_TYPE_BOOLEAN);
    EXPECT_TRUE(helicsGetDataType(typeid(int64_t).name()) == HELICS_DATA_TYPE_INT);
    EXPECT_TRUE(helicsGetDataType(typeid(char).name()) == HELICS_DATA_TYPE_CHAR);
    EXPECT_TRUE(helicsGetDataType(typeid(std::complex<double>).name()) == HELICS_DATA_TYPE_COMPLEX);
    EXPECT_TRUE(helicsGetDataType("COMPLEX") == HELICS_DATA_TYPE_COMPLEX);
    EXPECT_TRUE(helicsGetDataType("map") == HELICS_DATA_TYPE_RAW);
    EXPECT_TRUE(helicsGetDataType("any") == HELICS_DATA_TYPE_ANY);
    EXPECT_TRUE(helicsGetDataType("json") == HELICS_DATA_TYPE_JSON);
    EXPECT_TRUE(helicsGetDataType("JSON") == HELICS_DATA_TYPE_JSON);
    EXPECT_TRUE(helicsGetDataType("namedpoint") == HELICS_DATA_TYPE_NAMED_POINT);
    EXPECT_TRUE(helicsGetDataType("") == HELICS_DATA_TYPE_ANY);
    EXPECT_TRUE(helicsGetDataType(typeid(std::vector<std::complex<double>>).name()) ==
                HELICS_DATA_TYPE_COMPLEX_VECTOR);
}
