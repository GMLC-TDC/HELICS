/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "ctestFixtures.hpp"
#include "helics/helics.h"

#include <future>
#include <gtest/gtest.h>
#include <string>
#include <thread>
/** these test cases test out the message federates
 */

class translator_simple_type:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {};

/*
class filter_type_tests: public ::testing::TestWithParam<const char*>, public FederateTestFixture {
};
*/

static const auto testNamer = [](const ::testing::TestParamInfo<const char*>& parameter) {
    return std::string(parameter.param);
};

class translator: public FederateTestFixture, public ::testing::Test {};

/** test registration of translators*/

TEST_P(translator_simple_type, registration)
{
    auto broker = AddBroker(GetParam(), 2);
    AddFederates(helicsCreateValueFederate, GetParam(), 1, broker, HELICS_TIME_ZERO, "trans");
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, HELICS_TIME_ZERO, "message");

    auto fFed = GetFederateAt(0);
    auto mFed = GetFederateAt(1);

    CE(auto t1 = helicsFederateRegisterGlobalTranslator(
           fFed, HelicsTranslatorTypes::HELICS_TRANSLATOR_TYPE_JSON, "trans1", &err));
    helicsFederateRegisterGlobalEndpoint(mFed, "port1", "", &err);

    CE(auto t2 = helicsFederateRegisterTranslator(
           fFed, HelicsTranslatorTypes::HELICS_TRANSLATOR_TYPE_JSON, "trans2", &err));

    EXPECT_EQ(err.error_code, HELICS_OK);
    CE(auto p1 =
           helicsFederateRegisterPublication(fFed, "pub1", HELICS_DATA_TYPE_DOUBLE, "", &err));
    CE(helicsPublicationAddTarget(p1, "trans1", &err));
    EXPECT_NE(t1, nullptr);
    EXPECT_NE(t1, t2);

    auto f1_b = helicsFederateGetTranslator(fFed, "trans1", &err);
    const char* tmp;
    tmp = helicsTranslatorGetName(f1_b);
    EXPECT_STREQ(tmp, "trans1");

    CE(helicsTranslatorSetTag(f1_b, "tag1", "tagvalue", &err));
    EXPECT_STREQ(helicsTranslatorGetTag(f1_b, "tag1"), "tagvalue");

    auto f1_c = helicsFederateGetTranslatorByIndex(fFed, 1, &err);
    tmp = helicsTranslatorGetName(f1_c);
    EXPECT_STREQ(tmp, "trans0/trans2");

    auto f1_n = helicsFederateGetTranslatorByIndex(fFed, -2, &err);
    EXPECT_NE(err.error_code, 0);
    EXPECT_EQ(f1_n, nullptr);
    helicsErrorClear(&err);

    CE(helicsFederateEnterExecutingModeAsync(fFed, &err));
    CE(helicsFederateEnterExecutingMode(mFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(fFed, &err));
    CE(helicsFederateFinalizeAsync(mFed, &err));
    CE(helicsFederateFinalize(fFed, &err));
    CE(helicsFederateFinalizeComplete(mFed, &err));

    CE(HelicsFederateState state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}

TEST_F(translator, core_translator_reg)
{
    CE(auto core1 = helicsCreateCore("test", "core1", "--autobroker", &err));

    CE(auto core2 = helicsCoreClone(core1, &err));

    std::string core1IdentifierString = helicsCoreGetIdentifier(core1);

    EXPECT_EQ(core1IdentifierString, "core1");

    CE(auto t1 = helicsCoreRegisterTranslator(
           core1, HelicsTranslatorTypes::HELICS_TRANSLATOR_TYPE_JSON, "core1trans", &err));

    CE(helicsTranslatorAddDestinationEndpoint(t1, "ep1", &err));
    CE(helicsCoreRegisterTranslator(
        core1, HelicsTranslatorTypes::HELICS_TRANSLATOR_TYPE_JSON, "core1trans2", &err));

    helicsTranslatorAddSourceEndpoint(t1, "ep2", &err);

    int core1IsConnected = helicsCoreIsConnected(core1);
    EXPECT_NE(core1IsConnected, HELICS_FALSE);
    helicsCoreSetReadyToInit(core1, &err);
    helicsCoreDisconnect(core1, &err);
    helicsCoreDisconnect(core2, &err);
    helicsCoreFree(core1);
    helicsCoreFree(core2);
    helicsCloseLibrary();
}

/*
TEST_P(translator_simple_type, message_translator_function)
{
    HelicsBroker broker = AddBroker(GetParam(), 2);
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, 1.0, "filter");
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAt(0);
    auto mFed = GetFederateAt(1);

    CE(helicsFederateSetFlagOption(
        mFed, HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS, HELICS_TRUE, &err));
    auto p1 = helicsFederateRegisterGlobalEndpoint(mFed, "port1", nullptr, &err);
    auto p2 = helicsFederateRegisterGlobalEndpoint(mFed, "port2", "", &err);
    EXPECT_EQ(err.error_code, HELICS_OK);

    auto f1 = helicsFederateRegisterFilter(fFed, HELICS_FILTER_TYPE_DELAY, "filter1", &err);
    EXPECT_EQ(err.error_code, HELICS_OK);
    CE(helicsFilterAddSourceTarget(f1, "port1", &err));
    EXPECT_TRUE(f1 != nullptr);
    CE(helicsFilterSet(f1, "delay", 2.5, &err));

    CE(helicsFederateEnterExecutingModeAsync(fFed, &err));
    CE(helicsFederateEnterExecutingMode(mFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(fFed, &err));

    CE(HelicsFederateState state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_EXECUTION);
    std::string data(500, 'a');
    CE(helicsEndpointSendBytesTo(p1, data.c_str(), static_cast<int>(data.size()), "port2", &err));

    CE(helicsFederateRequestTimeAsync(mFed, 1.0, &err));
    CE(helicsFederateRequestTime(fFed, 1.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));

    auto res = helicsFederateHasMessage(mFed);
    EXPECT_TRUE(!res);

    CE(helicsFederateRequestTimeAsync(mFed, 2.0, &err));
    CE(helicsFederateRequestTime(fFed, 2.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));
    ASSERT_TRUE(!helicsEndpointHasMessage(p2));

    CE(helicsFederateRequestTimeAsync(fFed, 3.0, &err));
    CE(helicsFederateRequestTime(mFed, 3.0, &err));

    ASSERT_TRUE(helicsEndpointHasMessage(p2));

    auto m2 = helicsEndpointGetMessage(p2);
    EXPECT_STREQ(helicsMessageGetSource(m2), "port1");
    EXPECT_STREQ(helicsMessageGetOriginalSource(m2), "port1");
    EXPECT_STREQ(helicsMessageGetDestination(m2), "port2");
    EXPECT_EQ(helicsMessageGetByteCount(m2), static_cast<int>(data.size()));
    EXPECT_EQ(helicsMessageGetTime(m2), 2.5);

    CE(helicsFederateRequestTime(mFed, 3.0, &err));
    CE(helicsFederateRequestTimeComplete(fFed, &err));
    CE(helicsFederateFinalizeAsync(mFed, &err));
    CE(helicsFederateFinalize(fFed, &err));
    CE(helicsFederateFinalizeComplete(mFed, &err));
    CE(state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}
*/
/*
TEST_P(translator_simple_type, function_mObj)
{
    HelicsBroker broker = AddBroker(GetParam(), 2);
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, 1.0, "filter");
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAt(0);
    auto mFed = GetFederateAt(1);

    CE(helicsFederateSetFlagOption(
        mFed, HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS, HELICS_TRUE, &err));
    auto p1 = helicsFederateRegisterGlobalEndpoint(mFed, "port1", nullptr, &err);
    auto p2 = helicsFederateRegisterGlobalEndpoint(mFed, "port2", "", &err);
    EXPECT_EQ(err.error_code, HELICS_OK);

    auto f1 = helicsFederateRegisterFilter(fFed, HELICS_FILTER_TYPE_DELAY, "filter1", &err);
    EXPECT_EQ(err.error_code, HELICS_OK);
    CE(helicsFilterAddSourceTarget(f1, "port1", &err));
    EXPECT_TRUE(f1 != nullptr);
    CE(helicsFilterSet(f1, "delay", 2.5, &err));

    CE(helicsFederateEnterExecutingModeAsync(fFed, &err));
    CE(helicsFederateEnterExecutingMode(mFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(fFed, &err));

    CE(HelicsFederateState state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_EXECUTION);
    std::string data(500, 'a');
    CE(helicsEndpointSendBytesTo(p1, data.c_str(), static_cast<int>(data.size()), "port2", &err));

    CE(helicsFederateRequestTimeAsync(mFed, 1.0, &err));
    CE(helicsFederateRequestTime(fFed, 1.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));

    auto res = helicsFederateHasMessage(mFed);
    EXPECT_TRUE(!res);

    CE(helicsFederateRequestTimeAsync(mFed, 2.0, &err));
    CE(helicsFederateRequestTime(fFed, 2.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));
    ASSERT_TRUE(!helicsEndpointHasMessage(p2));

    CE(helicsFederateRequestTimeAsync(fFed, 3.0, &err));
    CE(helicsFederateRequestTime(mFed, 3.0, &err));

    ASSERT_TRUE(helicsEndpointHasMessage(p2));

    auto m2 = helicsEndpointGetMessage(p2);
    EXPECT_STREQ(helicsMessageGetSource(m2), "port1");
    EXPECT_STREQ(helicsMessageGetOriginalSource(m2), "port1");
    EXPECT_STREQ(helicsMessageGetDestination(m2), "port2");
    EXPECT_EQ(helicsMessageGetByteCount(m2), static_cast<int64_t>(data.size()));
    EXPECT_EQ(helicsMessageGetTime(m2), 2.5);

    CE(helicsFederateRequestTime(mFed, 3.0, &err));
    CE(helicsFederateRequestTimeComplete(fFed, &err));
    CE(helicsFederateFinalizeAsync(mFed, &err));
    CE(helicsFederateFinalize(fFed, &err));
    CE(helicsFederateFinalizeComplete(mFed, &err));
    CE(state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}
*/
/** test a translator operator
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the
simulation
*/
/*
TEST_P(translator_simple_type, function_two_stage)
{
    HelicsBroker broker = AddBroker(GetParam(), 3);
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, 1.0, "filter");
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, 1.0, "filter2");
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAt(0);
    auto fFed2 = GetFederateAt(1);
    auto mFed = GetFederateAt(2);

    auto p1 = helicsFederateRegisterGlobalEndpoint(mFed, "port1", "", &err);
    auto p2 = helicsFederateRegisterGlobalEndpoint(mFed, "port2", "", &err);
    EXPECT_EQ(err.error_code, HELICS_OK);

    auto f1 = helicsFederateRegisterFilter(fFed, HELICS_FILTER_TYPE_DELAY, "filter1", &err);
    EXPECT_EQ(err.error_code, HELICS_OK);
    CE(helicsFilterAddSourceTarget(f1, "port1", &err));

    EXPECT_TRUE(f1 != nullptr);
    CE(helicsFilterSet(f1, "delay", 1.25, &err));

    CE(auto f2 = helicsFederateRegisterFilter(fFed2, HELICS_FILTER_TYPE_DELAY, "filter2", &err));
    CE(helicsFilterAddSourceTarget(f2, "port1", &err));
    EXPECT_TRUE(f2 != nullptr);
    CE(helicsFilterSet(f2, "delay", 1.25, &err));

    CE(helicsFederateEnterExecutingModeAsync(fFed, &err));
    CE(helicsFederateEnterExecutingModeAsync(fFed2, &err));
    CE(helicsFederateEnterExecutingMode(mFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(fFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(fFed2, &err));

    CE(HelicsFederateState state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_EXECUTION);
    std::string data(500, 'a');
    CE(helicsEndpointSendBytesTo(p1, data.c_str(), static_cast<int>(data.size()), "port2", &err));

    CE(helicsFederateRequestTimeAsync(mFed, .0, &err));
    CE(helicsFederateRequestTimeAsync(fFed, 1.0, &err));
    CE(helicsFederateRequestTime(fFed2, 1.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));
    CE(helicsFederateRequestTimeComplete(fFed, &err));
    auto res = helicsFederateHasMessage(mFed);
    EXPECT_TRUE(!res);

    CE(helicsFederateRequestTimeAsync(mFed, .0, &err));
    CE(helicsFederateRequestTimeAsync(fFed2, 2.0, &err));
    CE(helicsFederateRequestTime(fFed, 2.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));
    CE(helicsFederateRequestTimeComplete(fFed2, &err));
    ASSERT_TRUE(!helicsEndpointHasMessage(p2));

    CE(helicsFederateRequestTimeAsync(fFed, 3.0, &err));
    CE(helicsFederateRequestTimeAsync(fFed2, 3.0, &err));
    CE(helicsFederateRequestTime(mFed, 3.0, &err));
    if (helicsEndpointHasMessage(p2) == HELICS_FALSE) {
        printf("missing message\n");
    }
    ASSERT_TRUE(helicsEndpointHasMessage(p2));

    auto m2 = helicsEndpointGetMessage(p2);
    EXPECT_STREQ(helicsMessageGetSource(m2), "port1");
    EXPECT_STREQ(helicsMessageGetOriginalSource(m2), "port1");
    EXPECT_STREQ(helicsMessageGetDestination(m2), "port2");
    EXPECT_EQ(helicsMessageGetByteCount(m2), static_cast<int64_t>(data.size()));
    EXPECT_EQ(helicsMessageGetTime(m2), 2.5);

    CE(helicsFederateRequestTimeComplete(fFed, &err));
    CE(helicsFederateRequestTimeComplete(fFed2, &err));
    CE(helicsFederateFinalizeAsync(mFed, &err));
    CE(helicsFederateFinalizeAsync(fFed, &err));
    CE(helicsFederateFinalize(fFed2, &err));
    CE(helicsFederateFinalizeComplete(mFed, &err));
    CE(helicsFederateFinalizeComplete(fFed, &err));
    CE(state = helicsFederateGetState(fFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}
*/

void toMC(HelicsDataBuffer value, HelicsMessage message, void* /*userData*/)
{
    double val = helicsDataBufferToDouble(value);
    helicsMessageSetString(message, std::to_string(val).c_str(), nullptr);
}

void toVC(HelicsMessage message, HelicsDataBuffer value, void* /*userData*/)
{
    const auto* str = helicsMessageGetString(message);
    char* ptr{nullptr};
    double v = strtod(str, &ptr) + 9.0;
    helicsDataBufferFillFromDouble(value, v);
}

TEST_F(translator, custom_translator)
{
    HelicsBroker broker = AddBroker("test_2", 2);
    AddFederates(helicsCreateCombinationFederate, "test_2", 1, broker, 1.0, "value");
    AddFederates(helicsCreateCombinationFederate, "test_2", 1, broker, 1.0, "message");

    auto vFed = GetFederateAt(0);
    auto mFed = GetFederateAt(1);

    auto e1 = helicsFederateRegisterGlobalTargetedEndpoint(mFed, "port1", "", &err);

    EXPECT_EQ(err.error_code, HELICS_OK);

    auto p1 =
        helicsFederateRegisterGlobalPublication(vFed, "pub1", HELICS_DATA_TYPE_DOUBLE, "V", &err);

    auto i1 = helicsFederateRegisterGlobalInput(vFed, "in1", HELICS_DATA_TYPE_DOUBLE, "V", &err);

    EXPECT_EQ(err.error_code, HELICS_OK);

    CE(auto t1 =
           helicsFederateRegisterGlobalTranslator(mFed, HELICS_TRANSLATOR_TYPE_CUSTOM, "t1", &err));
    EXPECT_TRUE(t1 != nullptr);

    helicsTranslatorAddSourceEndpoint(t1, "port1", nullptr);
    helicsTranslatorAddDestinationEndpoint(t1, "port1", nullptr);

    helicsTranslatorAddPublicationTarget(t1, "pub1", nullptr);
    helicsTranslatorAddInputTarget(t1, "in1", nullptr);

    helicsTranslatorSetCustomCallback(t1, toMC, toVC, nullptr, &err);

    EXPECT_EQ(err.error_code, HELICS_OK);

    CE(helicsFederateEnterExecutingModeAsync(vFed, &err));
    CE(helicsFederateEnterExecutingMode(mFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(vFed, &err));

    CE(HelicsFederateState state = helicsFederateGetState(vFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_EXECUTION);
    std::string data = "45.7";
    CE(helicsEndpointSendString(e1, data.c_str(), &err));

    helicsPublicationPublishDouble(p1, 99.23, &err);

    CE(helicsFederateRequestTimeAsync(mFed, 1.0, &err));
    CE(helicsFederateRequestTime(vFed, 1.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));

    auto res = helicsFederateHasMessage(mFed);
    EXPECT_TRUE(res);

    EXPECT_TRUE(helicsInputIsUpdated(i1) == HELICS_TRUE);

    double v3 = helicsInputGetDouble(i1, nullptr);
    EXPECT_DOUBLE_EQ(v3, 54.7);
    auto m2 = helicsEndpointGetMessage(e1);
    EXPECT_EQ(helicsMessageIsValid(m2), HELICS_TRUE);
    EXPECT_STREQ(helicsMessageGetSource(m2), "t1");
    EXPECT_STREQ(helicsMessageGetOriginalSource(m2), "pub1");
    EXPECT_STREQ(helicsMessageGetDestination(m2), "port1");

    EXPECT_EQ(helicsMessageGetTime(m2), 1e-9);
    EXPECT_STREQ(helicsMessageGetString(m2), "99.230000");
    helicsMessageFree(m2);
    CE(helicsFederateFinalize(mFed, &err));
    CE(helicsFederateFinalize(vFed, &err));
    CE(state = helicsFederateGetState(vFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}

TEST_F(translator, custom_translator2)
{
    HelicsBroker broker = AddBroker("test_2", 2);
    AddFederates(helicsCreateCombinationFederate, "test_2", 1, broker, 1.0, "value");
    AddFederates(helicsCreateCombinationFederate, "test_2", 1, broker, 1.0, "message");

    auto vFed = GetFederateAt(0);
    auto mFed = GetFederateAt(1);

    CE(auto t1 =
           helicsFederateRegisterGlobalTranslator(mFed, HELICS_TRANSLATOR_TYPE_CUSTOM, "t1", &err));
    EXPECT_TRUE(t1 != nullptr);

    helicsTranslatorAddSourceEndpoint(t1, "port1", nullptr);
    helicsTranslatorAddDestinationEndpoint(t1, "port1", nullptr);

    helicsTranslatorAddPublicationTarget(t1, "pub1", nullptr);
    helicsTranslatorAddInputTarget(t1, "in1", nullptr);

    helicsTranslatorSetCustomCallback(t1, toMC, toVC, nullptr, &err);

    EXPECT_EQ(err.error_code, HELICS_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    auto e1 = helicsFederateRegisterGlobalTargetedEndpoint(mFed, "port1", "", &err);

    EXPECT_EQ(err.error_code, HELICS_OK);

    auto p1 =
        helicsFederateRegisterGlobalPublication(vFed, "pub1", HELICS_DATA_TYPE_DOUBLE, "V", &err);

    auto i1 = helicsFederateRegisterGlobalInput(vFed, "in1", HELICS_DATA_TYPE_DOUBLE, "V", &err);

    EXPECT_EQ(err.error_code, HELICS_OK);

    CE(helicsFederateEnterExecutingModeAsync(vFed, &err));
    CE(helicsFederateEnterExecutingMode(mFed, &err));
    CE(helicsFederateEnterExecutingModeComplete(vFed, &err));

    CE(HelicsFederateState state = helicsFederateGetState(vFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_EXECUTION);
    std::string data = "45.7";
    CE(helicsEndpointSendBytes(e1, data.c_str(), static_cast<int>(data.size()), &err));

    helicsPublicationPublishDouble(p1, 99.23, &err);

    CE(helicsFederateRequestTimeAsync(mFed, 1.0, &err));
    CE(helicsFederateRequestTime(vFed, 1.0, &err));
    CE(helicsFederateRequestTimeComplete(mFed, &err));

    auto res = helicsFederateHasMessage(mFed);
    EXPECT_TRUE(res);

    EXPECT_TRUE(helicsInputIsUpdated(i1) == HELICS_TRUE);

    double v3 = helicsInputGetDouble(i1, nullptr);
    EXPECT_DOUBLE_EQ(v3, 54.7);
    auto m2 = helicsEndpointGetMessage(e1);
    EXPECT_EQ(helicsMessageIsValid(m2), HELICS_TRUE);
    EXPECT_STREQ(helicsMessageGetSource(m2), "t1");
    EXPECT_STREQ(helicsMessageGetOriginalSource(m2), "pub1");
    EXPECT_STREQ(helicsMessageGetDestination(m2), "port1");

    EXPECT_EQ(helicsMessageGetTime(m2), 1e-9);
    EXPECT_STREQ(helicsMessageGetString(m2), "99.230000");
    helicsMessageFree(m2);
    CE(helicsFederateFinalize(mFed, &err));
    CE(helicsFederateFinalize(vFed, &err));
    CE(state = helicsFederateGetState(vFed, &err));
    EXPECT_TRUE(state == HELICS_STATE_FINALIZE);
}

INSTANTIATE_TEST_SUITE_P(translator,
                         translator_simple_type,
                         ::testing::ValuesIn(CoreTypes_simple),
                         testNamer);
