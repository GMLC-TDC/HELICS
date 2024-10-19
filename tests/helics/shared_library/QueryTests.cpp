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

class QueryTests: public ::testing::TestWithParam<const char*>, public FederateTestFixture {};

class QueryTestSingle: public ::testing::Test, public FederateTestFixture {};
/** test simple creation and destruction*/
TEST_P(QueryTests, publication_queries)
{
    SetupTest(helicsCreateValueFederate, GetParam(), 2);
    auto vFed1 = GetFederateAt(0);
    auto vFed2 = GetFederateAt(1);

    // register the publications

    helicsFederateRegisterGlobalTypePublication(vFed1, "pub1", "double", "", &err);
    helicsFederateRegisterTypePublication(vFed1, "pub2", "double", "", &err);
    helicsFederateRegisterTypePublication(vFed2, "pub3", "double", "", &err);
    CE(helicsFederateEnterInitializingModeAsync(vFed1, &err));
    CE(helicsFederateEnterInitializingMode(vFed2, &err));
    CE(helicsFederateEnterInitializingModeComplete(vFed1, &err));

    auto core = helicsFederateGetCore(vFed1, &err);

    auto query1 = helicsCreateQuery("fed0", "publications");

    CE(std::string res(helicsQueryCoreExecute(query1, core, &err)));

    EXPECT_EQ(res, "[\"pub1\",\"fed0/pub2\"]");
    helicsQueryFree(query1);
    query1 = helicsCreateQuery(nullptr, "publications");
    CE(helicsQuerySetOrdering(query1, 1, &err));
    CE(std::string res2 = helicsQueryExecute(query1, vFed2, &err));
    EXPECT_EQ(res2, "[\"fed1/pub3\"]");

    helicsQueryFree(query1);

    query1 = helicsCreateQuery("fed1", "isinit");

    CE(res = helicsQueryExecute(query1, vFed1, &err));
    EXPECT_EQ(res, "true");
    helicsQueryFree(query1);

    query1 = helicsCreateQuery("fed1", "publications");
    CE(res = helicsQueryExecute(query1, vFed1, &err));
    EXPECT_EQ(res, "[\"fed1/pub3\"]");
    helicsQueryFree(query1);
    helicsCoreFree(core);
    CE(helicsFederateFinalizeAsync(vFed1, &err));
    CE(helicsFederateFinalize(vFed2, &err));
    CE(helicsFederateFinalizeComplete(vFed1, &err));
}

TEST_P(QueryTests, broker_queries)
{
    SetupTest(helicsCreateValueFederate, GetParam(), 2);
    auto vFed1 = GetFederateAt(0);
    auto vFed2 = GetFederateAt(1);

    CE(auto core = helicsFederateGetCore(vFed1, &err));

    auto query1 = helicsCreateQuery("root", "federates");
    std::string res = helicsQueryCoreExecute(query1, core, nullptr);
    std::string str("[\"");
    str.append(helicsFederateGetName(vFed1));
    str.append("\",\"");
    str.append(helicsFederateGetName(vFed2));
    str.append("\"]");

    EXPECT_EQ(res, str);

    CE(std::string res2 = helicsQueryExecute(query1, vFed1, &err));
    EXPECT_EQ(res2, str);
    CE(helicsFederateEnterInitializingModeAsync(vFed1, &err));
    CE(helicsFederateEnterInitializingMode(vFed2, &err));
    CE(helicsFederateEnterInitializingModeComplete(vFed1, &err));
    // expected to be false since it isn't associated with a asynchronous query
    auto qcomplete = helicsQueryIsCompleted(query1);
    EXPECT_EQ(qcomplete, HELICS_FALSE);

    helicsQueryFree(query1);
    helicsCoreFree(core);
    CE(helicsFederateFinalizeAsync(vFed1, &err));
    CE(helicsFederateFinalize(vFed2, &err));
    CE(helicsFederateFinalizeComplete(vFed1, &err));
}

static void queryTest(const char* query, int stringSize, HelicsQueryBuffer buffer, void* /*unused*/)
{
    std::string_view qstring(query, stringSize);
    if (qstring == "abc") {
        static constexpr const char* aret = "AAAA";
        helicsQueryBufferFill(buffer, aret, 4, nullptr);
    } else {
        static constexpr const char* bret = "BBBB";
        helicsQueryBufferFill(buffer, bret, 4, nullptr);
    }
}

TEST_F(QueryTestSingle, queries_callback_test)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);

    helicsFederateSetQueryCallback(vFed1, queryTest, nullptr, nullptr);

    auto query1 = helicsCreateQuery(helicsFederateGetName(vFed1), "abc");

    auto res = helicsQueryExecute(query1, vFed1, nullptr);
    EXPECT_STREQ(res, "AAAA");

    helicsQueryFree(query1);
    query1 = helicsCreateQuery(helicsFederateGetName(vFed1), "bca");
    res = helicsQueryExecute(query1, vFed1, nullptr);
    EXPECT_STREQ(res, "BBBB");
    helicsQueryFree(query1);
    helicsFederateEnterExecutingMode(vFed1, nullptr);
    helicsFederateFinalize(vFed1, nullptr);
}

INSTANTIATE_TEST_SUITE_P(QueryTests, QueryTests, ::testing::ValuesIn(CoreTypes));
