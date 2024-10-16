/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../src/helics/cpp98/Broker.hpp"
#include "../src/helics/cpp98/Core.hpp"
#include "../src/helics/cpp98/MessageFederate.hpp"
#include "cpptestFixtures.hpp"

#include <gtest/gtest.h>
#include <string>

struct QueryTests: public FederateTestFixture_cpp, public ::testing::Test {};

TEST_F(QueryTests, exists)
{
    SetupTest<helicscpp::MessageFederate>("test_2", 2, 1.0);
    auto mFed1 = GetFederateAs<helicscpp::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helicscpp::MessageFederate>(1);

    mFed1->registerEndpoint("ept1");
    mFed2->registerEndpoint("ept2");

    mFed1->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();

    mFed1->requestTimeAsync(1.0);
    mFed2->requestTime(1.0);
    mFed1->requestTimeComplete();
    auto res = mFed1->query("exists");
    EXPECT_EQ(res, "true");

    res = mFed1->query(mFed2->getName(), "exists");
    EXPECT_EQ(res, "true");

    auto core1 = helicscpp::Core(mFed1->getCore());

    res = mFed1->query(core1.getIdentifier(), "exists", HELICS_SEQUENCING_MODE_ORDERED);
    EXPECT_EQ(res, "true");

    res = mFed1->query(helicscpp::Core(mFed2->getCore()).getIdentifier(), "exists");
    EXPECT_EQ(res, "true");

    auto& brk = brokers.front();
    res = mFed1->query(brk->getIdentifier(), "exists");
    EXPECT_EQ(res, "true");

    res = brk->query(mFed1->getName(), "exists");
    EXPECT_EQ(res, "true");

    res = brk->query(mFed2->getName(), "exists", HELICS_SEQUENCING_MODE_ORDERED);
    EXPECT_EQ(res, "true");

    res = brk->query("root", "exists");
    EXPECT_EQ(res, "true");

    res = brk->query(core1.getIdentifier(), "exists");
    EXPECT_EQ(res, "true");

    res = brk->query(helicscpp::Core(mFed2->getCore()).getIdentifier(), "exists");
    EXPECT_EQ(res, "true");

    res = core1.query(brk->getIdentifier(), "exists");
    EXPECT_EQ(res, "true");

    res = core1.query(mFed1->getName(), "exists", HELICS_SEQUENCING_MODE_ORDERED);
    EXPECT_EQ(res, "true");

    res = core1.query(mFed2->getName(), "exists");
    EXPECT_EQ(res, "true");

    res = core1.query("root", "exists");
    EXPECT_EQ(res, "true");

    res = core1.query(core1.getIdentifier(), "exists", HELICS_SEQUENCING_MODE_ORDERED);
    EXPECT_EQ(res, "true");

    res = core1.query(helicscpp::Core(mFed2->getCore()).getIdentifier(), "exists");
    EXPECT_EQ(res, "true");

    mFed1->finalize();
    mFed2->finalize();

    core1.waitForDisconnect();
}

TEST_F(QueryTests, callback)
{
    SetupTest<helicscpp::MessageFederate>("test_2", 2, 1.0);
    auto mFed1 = GetFederateAs<helicscpp::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helicscpp::MessageFederate>(1);

    mFed1->registerEndpoint("ept1");
    mFed2->registerEndpoint("ept2");

    mFed1->setQueryCallback([](const std::string& query) -> std::string {
        if (query == "string1") {
            return "abcd";
        }
        if (query == "string2") {
            return "dcba";
        }
        return std::string{};
    });

    mFed1->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();

    auto res = mFed2->query(mFed1->getName(), "string1");
    EXPECT_EQ(res, "abcd");
    res = mFed2->query(mFed1->getName(), "string2");
    EXPECT_EQ(res, "dcba");

    mFed1->finalize();
    mFed2->finalize();
}
