/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/BrokerApp.hpp"
#include "helics/application_api/CombinationFederate.hpp"
#include "helics/application_api/CoreApp.hpp"
#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/MessageFederate.hpp"
#include "helics/core/Core.hpp"

#include <gtest/gtest.h>
#ifndef HELICS_SHARED_LIBRARY
#    include "testFixtures.hpp"
#else
#    include "testFixtures_shared.hpp"
#endif

#include <algorithm>
#include <future>
#include <iostream>
#include <numeric>
#include <string>
#include <thread>
#include <vector>

/** these test cases test out the message federates
 */
class mfed_tests: public ::testing::Test, public FederateTestFixture {};
/** test simple creation and destruction*/

TEST_F(mfed_tests, regex1)
{
    SetupTest<helics::MessageFederate>("test", 3, 1.0);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto mFed3 = GetFederateAs<helics::MessageFederate>(2);

    auto ep1 = mFed1->registerGlobalTargetedEndpoint("ep1");
    auto ep2 = mFed2->registerGlobalTargetedEndpoint("ep2");
    auto ep3 = mFed3->registerGlobalTargetedEndpoint("ep3");

    ep1.addDestinationTarget("REGEX:*");

    mFed1->enterExecutingModeAsync();
    mFed3->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();
    mFed3->enterExecutingModeComplete();

    ep1.send("test message");

    mFed1->requestTimeAsync(1.0);
    mFed2->requestTimeAsync(1.0);
    auto gtime = mFed3->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    gtime = mFed1->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);
    gtime = mFed2->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);

    EXPECT_TRUE(ep1.hasMessage());
    EXPECT_TRUE(ep2.hasMessage());
    EXPECT_TRUE(ep3.hasMessage());

    auto message = ep1.getMessage();
    if (message) {
        EXPECT_EQ(message->to_string(), "test message");
    }

    message = ep2.getMessage();
    if (message) {
        EXPECT_EQ(message->to_string(), "test message");
    }

    message = ep3.getMessage();
    if (message) {
        EXPECT_EQ(message->to_string(), "test message");
    }

    mFed1->finalizeAsync();
    mFed2->finalizeAsync();
    mFed3->finalize();
    mFed2->finalizeComplete();
    mFed1->finalizeComplete();
}

TEST_F(mfed_tests, regex2)
{
    SetupTest<helics::MessageFederate>("test", 3, 1.0);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto mFed3 = GetFederateAs<helics::MessageFederate>(2);

    auto ep1 = mFed1->registerGlobalTargetedEndpoint("ep1");
    auto ep2 = mFed2->registerGlobalTargetedEndpoint("ep2");
    auto ep3 = mFed3->registerGlobalTargetedEndpoint("ep3");

    ep1.addDestinationTarget("REGEX:ep.");

    mFed1->enterExecutingModeAsync();
    mFed3->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();
    mFed3->enterExecutingModeComplete();

    ep1.send("test message");

    mFed1->requestTimeAsync(1.0);
    mFed2->requestTimeAsync(1.0);
    auto gtime = mFed3->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    gtime = mFed1->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);
    gtime = mFed2->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);

    EXPECT_TRUE(ep1.hasMessage());
    EXPECT_TRUE(ep2.hasMessage());
    EXPECT_TRUE(ep3.hasMessage());

    auto message = ep1.getMessage();
    if (message) {
        EXPECT_EQ(message->to_string(), "test message");
    }

    message = ep2.getMessage();
    if (message) {
        EXPECT_EQ(message->to_string(), "test message");
    }

    message = ep3.getMessage();
    if (message) {
        EXPECT_EQ(message->to_string(), "test message");
    }

    mFed1->finalizeAsync();
    mFed2->finalizeAsync();
    mFed3->finalize();
    mFed2->finalizeComplete();
    mFed1->finalizeComplete();
}

TEST_F(mfed_tests, regex3)
{
    SetupTest<helics::MessageFederate>("test", 3, 1.0);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto mFed3 = GetFederateAs<helics::MessageFederate>(2);

    auto ep1 = mFed1->registerGlobalTargetedEndpoint("ep1");
    auto ep2 = mFed2->registerGlobalTargetedEndpoint("ep2");
    auto ep3 = mFed3->registerGlobalTargetedEndpoint("ep3");

    ep1.addSourceTarget("REGEX:ep.");

    mFed1->enterExecutingModeAsync();
    mFed3->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();
    mFed3->enterExecutingModeComplete();

    ep1.send("test message1");
    ep2.send("test message2");
    ep3.send("test message3");
    mFed1->requestTimeAsync(1.0);
    mFed2->requestTimeAsync(1.0);
    auto gtime = mFed3->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    gtime = mFed1->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);
    gtime = mFed2->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);

    EXPECT_EQ(ep1.pendingMessageCount(), 3);
    EXPECT_FALSE(ep2.hasMessage());
    EXPECT_FALSE(ep3.hasMessage());

    auto message = ep1.getMessage();
    if (message) {
        EXPECT_EQ(message->to_string(), "test message1");
    }

    message = ep1.getMessage();
    if (message) {
        EXPECT_EQ(message->to_string(), "test message2");
    }

    message = ep3.getMessage();
    if (message) {
        EXPECT_EQ(message->to_string(), "test message3");
    }

    mFed1->finalizeAsync();
    mFed2->finalizeAsync();
    mFed3->finalize();
    mFed2->finalizeComplete();
    mFed1->finalizeComplete();
}

TEST_F(mfed_tests, regex_data_sink)
{
    SetupTest<helics::MessageFederate>("test", 3, 1.0);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto mFed3 = GetFederateAs<helics::MessageFederate>(2);

    auto ep1 = mFed1->registerGlobalTargetedEndpoint("ep1");
    auto ep2 = mFed2->registerGlobalTargetedEndpoint("ep2");
    auto ep3 = mFed3->registerGlobalTargetedEndpoint("ep3");

    auto sink1 = mFed1->registerDataSink("ds1");

    sink1.addSourceTarget("REGEX:.*");

    mFed1->enterExecutingModeAsync();
    mFed3->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();
    mFed3->enterExecutingModeComplete();

    ep3.send("test message3");
    ep1.send("test message1");
    ep2.send("test message2");

    mFed1->requestTimeAsync(1.0);
    mFed2->requestTimeAsync(1.0);
    auto gtime = mFed3->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    gtime = mFed1->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);
    gtime = mFed2->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);

    EXPECT_EQ(sink1.pendingMessageCount(), 3);
    EXPECT_FALSE(ep1.hasMessage());
    EXPECT_FALSE(ep2.hasMessage());
    EXPECT_FALSE(ep3.hasMessage());

    auto message = sink1.getMessage();
    if (message) {
        EXPECT_EQ(message->to_string(), "test message1");
    }

    message = sink1.getMessage();
    if (message) {
        EXPECT_EQ(message->to_string(), "test message2");
    }

    message = sink1.getMessage();
    if (message) {
        EXPECT_EQ(message->to_string(), "test message3");
    }

    mFed1->finalizeAsync();
    mFed2->finalizeAsync();
    mFed3->finalize();
    mFed2->finalizeComplete();
    mFed1->finalizeComplete();
}

// test combinationFederate linking with value Fed
TEST_F(mfed_tests, regex_combo1)
{
    SetupTest<helics::CombinationFederate>("test", 3, 1.0);
    auto mFed1 = GetFederateAs<helics::CombinationFederate>(0);
    auto mFed2 = GetFederateAs<helics::CombinationFederate>(1);
    auto mFed3 = GetFederateAs<helics::CombinationFederate>(2);

    auto ep1 = mFed1->registerGlobalTargetedEndpoint("ep1");
    auto ep2 = mFed2->registerGlobalTargetedEndpoint("ep2");
    auto ep3 = mFed3->registerGlobalTargetedEndpoint("ep3");

    ep1.addDestinationTarget("REGEX:*");

    mFed1->enterExecutingModeAsync();
    mFed3->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();
    mFed3->enterExecutingModeComplete();

    ep1.send("test message");

    mFed1->requestTimeAsync(1.0);
    mFed2->requestTimeAsync(1.0);
    auto gtime = mFed3->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    gtime = mFed1->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);
    gtime = mFed2->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);

    EXPECT_TRUE(ep1.hasMessage());
    EXPECT_TRUE(ep2.hasMessage());
    EXPECT_TRUE(ep3.hasMessage());

    auto message = ep1.getMessage();
    if (message) {
        EXPECT_EQ(message->to_string(), "test message");
    }

    message = ep2.getMessage();
    if (message) {
        EXPECT_EQ(message->to_string(), "test message");
    }

    message = ep3.getMessage();
    if (message) {
        EXPECT_EQ(message->to_string(), "test message");
    }

    mFed1->finalizeAsync();
    mFed2->finalizeAsync();
    mFed3->finalize();
    mFed2->finalizeComplete();
    mFed1->finalizeComplete();
}

TEST_F(mfed_tests, regex_combo2)
{
    SetupTest<helics::CombinationFederate>("test", 3, 1.0);
    auto mFed1 = GetFederateAs<helics::CombinationFederate>(0);
    auto mFed2 = GetFederateAs<helics::CombinationFederate>(1);
    auto mFed3 = GetFederateAs<helics::CombinationFederate>(2);

    auto pub1 = mFed1->registerGlobalPublication<std::string>("ep1");

    auto sub1 = mFed3->registerGlobalInput<std::string>("input1");
    auto sub2 = mFed2->registerGlobalInput<std::string>("input2");
    auto sub3 = mFed3->registerGlobalInput<std::string>("input3");

    pub1.addDestinationTarget("REGEX:input.");

    mFed1->enterExecutingModeAsync();
    mFed3->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();
    mFed3->enterExecutingModeComplete();

    pub1.publish("test message");

    mFed1->requestTimeAsync(1.0);
    mFed2->requestTimeAsync(1.0);
    auto gtime = mFed3->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    gtime = mFed1->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);
    gtime = mFed2->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);

    EXPECT_TRUE(sub1.isUpdated());
    EXPECT_TRUE(sub2.isUpdated());
    EXPECT_TRUE(sub3.isUpdated());

    EXPECT_EQ(sub1.getString(), "test message");
    EXPECT_EQ(sub2.getString(), "test message");
    EXPECT_EQ(sub3.getString(), "test message");

    mFed1->finalizeAsync();
    mFed2->finalizeAsync();
    mFed3->finalize();
    mFed2->finalizeComplete();
    mFed1->finalizeComplete();
}

TEST_F(mfed_tests, regex_combo3)
{
    SetupTest<helics::CombinationFederate>("test", 3, 1.0);
    auto mFed1 = GetFederateAs<helics::CombinationFederate>(0);
    auto mFed2 = GetFederateAs<helics::CombinationFederate>(1);
    auto mFed3 = GetFederateAs<helics::CombinationFederate>(2);

    auto ep1 = mFed1->registerGlobalTargetedEndpoint("ep1");
    auto ep2 = mFed2->registerGlobalTargetedEndpoint("ep2");
    auto ep3 = mFed3->registerGlobalTargetedEndpoint("ep3");

    ep1.addSourceTarget("REGEX:ep.");

    mFed1->enterExecutingModeAsync();
    mFed3->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();
    mFed3->enterExecutingModeComplete();

    ep1.send("test message1");
    ep2.send("test message2");
    ep3.send("test message3");
    mFed1->requestTimeAsync(1.0);
    mFed2->requestTimeAsync(1.0);
    auto gtime = mFed3->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    gtime = mFed1->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);
    gtime = mFed2->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);

    EXPECT_EQ(ep1.pendingMessageCount(), 3);
    EXPECT_FALSE(ep2.hasMessage());
    EXPECT_FALSE(ep3.hasMessage());

    auto message = ep1.getMessage();
    if (message) {
        EXPECT_EQ(message->to_string(), "test message1");
    }

    message = ep1.getMessage();
    if (message) {
        EXPECT_EQ(message->to_string(), "test message2");
    }

    message = ep3.getMessage();
    if (message) {
        EXPECT_EQ(message->to_string(), "test message3");
    }

    mFed1->finalizeAsync();
    mFed2->finalizeAsync();
    mFed3->finalize();
    mFed2->finalizeComplete();
    mFed1->finalizeComplete();
}

TEST_F(mfed_tests, regex_combo_data_sink)
{
    SetupTest<helics::CombinationFederate>("test", 3, 1.0);
    auto mFed1 = GetFederateAs<helics::CombinationFederate>(0);
    auto mFed2 = GetFederateAs<helics::CombinationFederate>(1);
    auto mFed3 = GetFederateAs<helics::CombinationFederate>(2);

    auto ep1 = mFed1->registerGlobalTargetedEndpoint("ep1");
    auto ep2 = mFed2->registerGlobalTargetedEndpoint("ep2");
    auto ep3 = mFed3->registerGlobalTargetedEndpoint("ep3");

    auto sink1 = mFed1->registerDataSink("ds1");

    sink1.addSourceTarget("REGEX:.*");

    mFed1->enterExecutingModeAsync();
    mFed3->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();
    mFed3->enterExecutingModeComplete();

    ep3.send("test message3");
    ep1.send("test message1");
    ep2.send("test message2");

    mFed1->requestTimeAsync(1.0);
    mFed2->requestTimeAsync(1.0);
    auto gtime = mFed3->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    gtime = mFed1->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);
    gtime = mFed2->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);

    EXPECT_EQ(sink1.pendingMessageCount(), 3);
    EXPECT_FALSE(ep1.hasMessage());
    EXPECT_FALSE(ep2.hasMessage());
    EXPECT_FALSE(ep3.hasMessage());

    auto message = sink1.getMessage();
    if (message) {
        EXPECT_EQ(message->to_string(), "test message1");
    }

    message = sink1.getMessage();
    if (message) {
        EXPECT_EQ(message->to_string(), "test message2");
    }

    message = sink1.getMessage();
    if (message) {
        EXPECT_EQ(message->to_string(), "test message3");
    }

    mFed1->finalizeAsync();
    mFed2->finalizeAsync();
    mFed3->finalize();
    mFed2->finalizeComplete();
    mFed1->finalizeComplete();
}

TEST_F(mfed_tests, endpoint_linking)
{
    SetupTest<helics::CombinationFederate>("testA", 1, 1.0);
    auto mFed1 = GetFederateAs<helics::CombinationFederate>(0);

    auto& ept1 = mFed1->registerGlobalTargetedEndpoint("source_endpoint");
    auto& ept2 = mFed1->registerGlobalTargetedEndpoint("dest_endpoint");
    helics::CoreApp core(mFed1->getCorePointer());
    core->linkEndpoints("source_endpoint", "dest_endpoint");
    mFed1->enterExecutingMode();
    ept1.send("test message");
    mFed1->requestNextStep();
    EXPECT_TRUE(ept2.hasMessage());
    auto message = ept2.getMessage();
    EXPECT_EQ(message->to_string(), "test message");
    mFed1->finalize();
}

TEST_F(mfed_tests, endpoint_linking_dest_unknown)
{
    SetupTest<helics::CombinationFederate>("testA", 1, 1.0);
    auto mFed1 = GetFederateAs<helics::CombinationFederate>(0);
    EXPECT_TRUE(mFed1);
    if (!mFed1) {
        return;
    }
    helics::CoreApp core(mFed1->getCorePointer());
    auto& ept1 = mFed1->registerGlobalTargetedEndpoint("source_endpoint");
    core->linkEndpoints("source_endpoint", "dest_endpoint");
    auto& ept2 = mFed1->registerGlobalTargetedEndpoint("dest_endpoint");

    mFed1->enterExecutingMode();
    ept1.send("test message");
    mFed1->requestNextStep();
    EXPECT_TRUE(ept2.hasMessage());
    auto message = ept2.getMessage();
    EXPECT_EQ(message->to_string(), "test message");
    mFed1->finalize();
}

TEST_F(mfed_tests, endpoint_linking_source_unknown)
{
    SetupTest<helics::CombinationFederate>("testA", 1, 1.0);
    auto mFed1 = GetFederateAs<helics::CombinationFederate>(0);
    EXPECT_TRUE(mFed1);
    if (!mFed1) {
        return;
    }
    helics::CoreApp core(mFed1->getCorePointer());
    auto& ept2 = mFed1->registerGlobalTargetedEndpoint("dest_endpoint");
    core->linkEndpoints("source_endpoint", "dest_endpoint");

    auto& ept1 = mFed1->registerGlobalTargetedEndpoint("source_endpoint");
    mFed1->enterExecutingMode();
    ept1.send("test message");
    mFed1->requestNextStep();
    EXPECT_TRUE(ept2.hasMessage());
    auto message = ept2.getMessage();
    EXPECT_EQ(message->to_string(), "test message");
    mFed1->finalize();
}

TEST_F(mfed_tests, endpoint_linking_both_unknown)
{
    SetupTest<helics::CombinationFederate>("testA", 1, 1.0);
    auto mFed1 = GetFederateAs<helics::CombinationFederate>(0);
    EXPECT_TRUE(mFed1);
    if (!mFed1) {
        return;
    }
    helics::CoreApp core(mFed1->getCorePointer());

    core->linkEndpoints("source_endpoint", "dest_endpoint");
    auto& ept2 = mFed1->registerGlobalTargetedEndpoint("dest_endpoint");

    auto& ept1 = mFed1->registerGlobalTargetedEndpoint("source_endpoint");
    mFed1->enterExecutingMode();
    ept1.send("test message");
    mFed1->requestNextStep();
    EXPECT_TRUE(ept2.hasMessage());
    auto message = ept2.getMessage();
    EXPECT_EQ(message->to_string(), "test message");
    mFed1->finalize();
}

TEST_F(mfed_tests, endpoint_linking_source_alias)
{
    SetupTest<helics::CombinationFederate>("testA", 1, 1.0);
    auto mFed1 = GetFederateAs<helics::CombinationFederate>(0);
    EXPECT_TRUE(mFed1);
    if (!mFed1) {
        return;
    }
    helics::CoreApp core(mFed1->getCorePointer());

    core->linkEndpoints("source", "dest_endpoint");
    auto& ept2 = mFed1->registerGlobalTargetedEndpoint("dest_endpoint");

    auto& ept1 = mFed1->registerGlobalTargetedEndpoint("source_endpoint");
    core->addAlias("source_endpoint", "source");
    mFed1->enterExecutingMode();
    ept1.send("test message");
    mFed1->requestNextStep();
    EXPECT_TRUE(ept2.hasMessage());
    auto message = ept2.getMessage();
    EXPECT_EQ(message->to_string(), "test message");
    mFed1->finalize();
}

TEST_F(mfed_tests, endpoint_linking_dest_alias)
{
    SetupTest<helics::CombinationFederate>("testA", 1, 1.0);
    auto mFed1 = GetFederateAs<helics::CombinationFederate>(0);
    EXPECT_TRUE(mFed1);
    if (!mFed1) {
        return;
    }
    helics::CoreApp core(mFed1->getCorePointer());

    core->linkEndpoints("source_endpoint", "dest");
    auto& ept2 = mFed1->registerGlobalTargetedEndpoint("dest_endpoint");

    auto& ept1 = mFed1->registerGlobalTargetedEndpoint("source_endpoint");
    core->addAlias("dest_endpoint", "dest");
    mFed1->enterExecutingMode();
    ept1.send("test message");
    mFed1->requestNextStep();
    EXPECT_TRUE(ept2.hasMessage());
    auto message = ept2.getMessage();
    EXPECT_EQ(message->to_string(), "test message");
    mFed1->finalize();
}

TEST_F(mfed_tests, endpoint_linking_dest_alias_rev)
{
    SetupTest<helics::CombinationFederate>("testA", 1, 1.0);
    auto mFed1 = GetFederateAs<helics::CombinationFederate>(0);
    EXPECT_TRUE(mFed1);
    if (!mFed1) {
        return;
    }
    helics::CoreApp core(mFed1->getCorePointer());

    core->linkEndpoints("source_endpoint", "dest");
    auto& ept2 = mFed1->registerGlobalTargetedEndpoint("dest_endpoint");

    auto& ept1 = mFed1->registerGlobalTargetedEndpoint("source_endpoint");
    core->addAlias("dest", "dest_endpoint");
    mFed1->enterExecutingMode();
    ept1.send("test message");
    mFed1->requestNextStep();
    EXPECT_TRUE(ept2.hasMessage());
    auto message = ept2.getMessage();
    EXPECT_EQ(message->to_string(), "test message");
    mFed1->finalize();
}

class mfed_permutation_tests: public ::testing::TestWithParam<int>, public FederateTestFixture {};

TEST_P(mfed_permutation_tests, endpoint_linking_order_permutations_nosan)
{
    SetupTest<helics::CombinationFederate>("testA", 1, 1.0);
    auto mFed1 = GetFederateAs<helics::CombinationFederate>(0);
    helics::CoreApp core(mFed1->getCorePointer());

    std::vector<std::function<void()>> exList(5);
    std::vector<int> exOrder(5);
    std::iota(exOrder.begin(), exOrder.end(), 0);

    int permutations = GetParam();
    for (int kk = 0; kk < permutations; ++kk) {
        std::next_permutation(exOrder.begin(), exOrder.end());
    }
    exList[0] = [&mFed1]() { mFed1->registerGlobalTargetedEndpoint("dest_endpoint"); };
    exList[1] = [&mFed1]() { mFed1->registerGlobalTargetedEndpoint("source_endpoint"); };
    exList[2] = [&core]() { core->addAlias("dest_endpoint", "dest"); };
    exList[3] = [&core]() { core->addAlias("source_endpoint", "source"); };
    exList[4] = [&core]() { core->linkEndpoints("source", "dest"); };

    for (int ii = 0; ii < 5; ++ii) {
        exList[exOrder[ii]]();
    }
    auto& ept2 = mFed1->getEndpoint("dest_endpoint");
    auto& ept1 = mFed1->getEndpoint("source_endpoint");
    mFed1->enterExecutingMode();
    ept1.send("test message");
    mFed1->requestNextStep();
    EXPECT_TRUE(ept2.hasMessage());
    auto message = ept2.getMessage();
    EXPECT_EQ(message->to_string(), "test message");
    mFed1->finalize();
}

INSTANTIATE_TEST_SUITE_P(OrderPermutations,
                         mfed_permutation_tests,
                         testing::Range(0, 5 * 4 * 3 * 2 * 1));
