/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "ctestFixtures.hpp"

#include <gtest/gtest.h>

class query_tests: public ::testing::TestWithParam<const char*>, public FederateTestFixture {
};
/** test simple creation and destruction*/
TEST_P(query_tests, publication_queries)
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

    auto core = helicsFederateGetCoreObject(vFed1, &err);

    auto q1 = helicsCreateQuery("fed0", "publications");

    CE(std::string res(helicsQueryCoreExecute(q1, core, &err)));

    EXPECT_EQ(res, "[pub1;fed0/pub2]");
    helicsQueryFree(q1);
    q1 = helicsCreateQuery(nullptr, "publications");
    CE(std::string res2 = helicsQueryExecute(q1, vFed2, &err));
    EXPECT_EQ(res2, "[fed1/pub3]");

    helicsQueryFree(q1);

    q1 = helicsCreateQuery("fed1", "isinit");

    CE(res = helicsQueryExecute(q1, vFed1, &err));
    EXPECT_EQ(res, "true");
    helicsQueryFree(q1);

    q1 = helicsCreateQuery("fed1", "publications");
    CE(res = helicsQueryExecute(q1, vFed1, &err));
    EXPECT_EQ(res, "[fed1/pub3]");
    helicsQueryFree(q1);
    helicsCoreFree(core);
    CE(helicsFederateFinalizeAsync(vFed1, &err));
    CE(helicsFederateFinalize(vFed2, &err));
    CE(helicsFederateFinalizeComplete(vFed1, &err));
}

TEST_P(query_tests, broker_queries)
{
    SetupTest(helicsCreateValueFederate, GetParam(), 2);
    auto vFed1 = GetFederateAt(0);
    auto vFed2 = GetFederateAt(1);

    CE(auto core = helicsFederateGetCoreObject(vFed1, &err));

    auto q1 = helicsCreateQuery("root", "federates");
    std::string res = helicsQueryCoreExecute(q1, core, nullptr);
    std::string str("[");
    str.append(helicsFederateGetName(vFed1));
    str.push_back(';');
    str.append(helicsFederateGetName(vFed2));
    str.push_back(']');

    EXPECT_EQ(res, str);

    CE(std::string res2 = helicsQueryExecute(q1, vFed1, &err));
    EXPECT_EQ(res2, str);
    CE(helicsFederateEnterInitializingModeAsync(vFed1, &err));
    CE(helicsFederateEnterInitializingMode(vFed2, &err));
    CE(helicsFederateEnterInitializingModeComplete(vFed1, &err));
    //expected to be false since it isn't associated with a asynchronous query
    auto qcomplete = helicsQueryIsCompleted(q1);
    EXPECT_EQ(qcomplete, helics_false);

    helicsQueryFree(q1);
    helicsCoreFree(core);
    CE(helicsFederateFinalizeAsync(vFed1, &err));
    CE(helicsFederateFinalize(vFed2, &err));
    CE(helicsFederateFinalizeComplete(vFed1, &err));
}

INSTANTIATE_TEST_SUITE_P(query_tests, query_tests, ::testing::ValuesIn(core_types));
