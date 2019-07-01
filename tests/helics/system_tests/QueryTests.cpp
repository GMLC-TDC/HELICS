/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "../application_api/testFixtures.hpp"
#include "gtest/gtest.h"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/application_api/queryFunctions.hpp"
#include "helics/common/JsonProcessingFunctions.hpp"

struct query_tests : public FederateTestFixture, public ::testing::Test
{
};

class query_type_tests : public ::testing::TestWithParam<const char *>, public FederateTestFixture
{
};
/** test simple creation and destruction*/
TEST_P (query_type_tests, publication_queries)
{
    SetupTest<helics::ValueFederate> (GetParam (), 2, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);
    // register the publications
    vFed1->registerGlobalPublication<double> ("pub1");

    vFed2->registerSubscription ("pub1");

    vFed1->registerPublication<double> ("pub2");

    vFed2->registerPublication<double> ("pub3");

    vFed1->enterInitializingModeAsync ();
    vFed2->enterInitializingMode ();
    vFed1->enterInitializingModeComplete ();

    auto core = vFed1->getCorePointer ();
    auto res = core->query ("fed0", "publications");
    EXPECT_EQ (res, "[pub1;fed0/pub2]");
    auto rvec = vectorizeQueryResult (res);

    ASSERT_EQ (rvec.size (), 2u);
    EXPECT_EQ (rvec[0], "pub1");
    EXPECT_EQ (rvec[1], "fed0/pub2");
    EXPECT_EQ (vFed2->query ("fed0", "publications"), "[pub1;fed0/pub2]");
    EXPECT_EQ (vFed1->query ("fed1", "isinit"), "true");

    EXPECT_EQ (vFed1->query ("fed1", "publications"), "[fed1/pub3]");
    core = nullptr;
    vFed1->finalize ();
    vFed2->finalize ();
    helics::cleanupHelicsLibrary ();
}

TEST_P (query_type_tests, broker_queries)
{
    SetupTest<helics::ValueFederate> (GetParam (), 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);
    auto core = vFed1->getCorePointer ();
    auto res = core->query ("root", "federates");
    std::string str ("[");
    str.append (vFed1->getName ());
    str.push_back (';');
    str.append (vFed2->getName ());
    str.push_back (']');
    EXPECT_EQ (res, "[fed0;fed1]");
    vFed1->enterInitializingModeAsync ();
    vFed2->enterInitializingMode ();
    vFed1->enterInitializingModeComplete ();
    core = nullptr;
    vFed1->finalize ();
    vFed2->finalize ();
    helics::cleanupHelicsLibrary ();
}

TEST_P (query_type_tests, publication_fed_queries)
{
    SetupTest<helics::ValueFederate> (GetParam (), 2, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);
    // register the publications
    vFed1->registerPublication<double> ("pub1");

    vFed2->registerPublication<double> ("pub2");

    vFed2->registerPublication<double> ("pub3");

    vFed1->enterInitializingModeAsync ();
    vFed2->enterInitializingMode ();
    vFed1->enterInitializingModeComplete ();

    auto res = vFed1->query ("federation", "publications");

    auto rvec = vectorizeAndSortQueryResult (res);

    ASSERT_EQ (rvec.size (), 3u);
    EXPECT_EQ (rvec[0], "fed0/pub1");
    EXPECT_EQ (rvec[1], "fed1/pub2");
    EXPECT_EQ (rvec[2], "fed1/pub3");
    vFed1->finalize ();
    vFed2->finalize ();
}

INSTANTIATE_TEST_SUITE_P (query_tests, query_type_tests, ::testing::ValuesIn (core_types));

TEST_F (query_tests, test_federate_map)
{
    SetupTest<helics::ValueFederate> ("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);
    auto core = vFed1->getCorePointer ();
    auto res = core->query ("root", "federate_map");
    vFed1->enterInitializingModeAsync ();
    vFed2->enterInitializingMode ();
    vFed1->enterInitializingModeComplete ();
    auto val = loadJsonStr (res);
    EXPECT_EQ (val["cores"].size (), 1u);
    EXPECT_EQ (val["cores"][0]["federates"].size (), 2u);
    EXPECT_EQ (val["cores"][0]["parent"].asInt (), val["id"].asInt ());
    auto v2 = val["cores"][0]["federates"][1];
    EXPECT_EQ (v2["parent"].asInt (), val["cores"][0]["id"].asInt ());
    core = nullptr;
    vFed1->finalize ();
    vFed2->finalize ();
    helics::cleanupHelicsLibrary ();
}

TEST_F (query_tests, test_federate_map2)
{
    SetupTest<helics::ValueFederate> ("test_2", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);
    auto core = vFed1->getCorePointer ();
    auto res = core->query ("root", "federate_map");
    vFed1->enterInitializingModeAsync ();
    vFed2->enterInitializingMode ();
    vFed1->enterInitializingModeComplete ();
    auto val = loadJsonStr (res);
    EXPECT_EQ (val["cores"].size (), 2u);
    EXPECT_EQ (val["cores"][1]["federates"].size (), 1u);
    EXPECT_EQ (val["cores"][1]["parent"].asInt (), val["id"].asInt ());
    auto v2 = val["cores"][1]["federates"][0];
    EXPECT_EQ (v2["parent"].asInt (), val["cores"][1]["id"].asInt ());
    core = nullptr;
    vFed1->finalize ();
    vFed2->finalize ();
    helics::cleanupHelicsLibrary ();
}

TEST_F (query_tests, test_federate_map3)
{
    SetupTest<helics::ValueFederate> ("test_3", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);
    auto core = vFed1->getCorePointer ();
    auto res = core->query ("root", "federate_map");
    vFed1->enterInitializingModeAsync ();
    vFed2->enterInitializingMode ();
    vFed1->enterInitializingModeComplete ();
    auto val = loadJsonStr (res);
    EXPECT_EQ (val["cores"].size (), 0u);
    EXPECT_EQ (val["brokers"].size (), 1u);
    EXPECT_EQ (val["brokers"][0]["parent"].asInt (), val["id"].asInt ());
    auto brk = val["brokers"][0];
    EXPECT_EQ (brk["cores"].size (), 2u);
    EXPECT_EQ (brk["brokers"].size (), 0u);
    EXPECT_EQ (brk["cores"][1]["federates"].size (), 1u);
    EXPECT_EQ (brk["cores"][1]["parent"].asInt (), brk["id"].asInt ());
    auto v2 = brk["cores"][1]["federates"][0];
    EXPECT_EQ (v2["parent"].asInt (), brk["cores"][1]["id"].asInt ());
    core = nullptr;
    vFed1->finalize ();
    vFed2->finalize ();
    helics::cleanupHelicsLibrary ();
}

TEST_F (query_tests, test_dependency_graph)
{
    SetupTest<helics::ValueFederate> ("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);
    auto core = vFed1->getCorePointer ();
    auto res = core->query ("root", "dependency_graph");
    vFed1->enterInitializingModeAsync ();
    vFed2->enterInitializingMode ();
    vFed1->enterInitializingModeComplete ();
    auto val = loadJsonStr (res);
    EXPECT_EQ (val["cores"].size (), 1u);
    EXPECT_EQ (val["cores"][0]["federates"].size (), 2u);
    EXPECT_EQ (val["cores"][0]["parent"].asInt (), val["id"].asInt ());
    auto v2 = val["cores"][0]["federates"][1];
    EXPECT_EQ (v2["parent"].asInt (), val["cores"][0]["id"].asInt ());
    core = nullptr;
    vFed1->finalize ();
    vFed2->finalize ();
    helics::cleanupHelicsLibrary ();
}
