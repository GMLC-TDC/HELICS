/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "../application_api/testFixtures.hpp"
#include "helics/application_api/CombinationFederate.hpp"
#include "helics/application_api/Filters.hpp"
#include "helics/application_api/Publications.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/application_api/queryFunctions.hpp"
#include "helics/common/JsonProcessingFunctions.hpp"

#include "gtest/gtest.h"

struct query_tests: public FederateTestFixture, public ::testing::Test {
};

class query_type_tests: public ::testing::TestWithParam<const char*>, public FederateTestFixture {
};
/** test simple creation and destruction*/
TEST_P(query_type_tests, publication_queries)
{
    SetupTest<helics::ValueFederate>(GetParam(), 2, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    // register the publications
    vFed1->registerGlobalPublication<double>("pub1");

    vFed2->registerSubscription("pub1");

    vFed1->registerPublication<double>("pub2");

    vFed2->registerPublication<double>("pub3");

    vFed1->enterInitializingModeAsync();
    vFed2->enterInitializingMode();
    vFed1->enterInitializingModeComplete();

    auto core = vFed1->getCorePointer();
    auto res = core->query("fed0", "publications");
    EXPECT_EQ(res, "[pub1;fed0/pub2]");
    auto rvec = helics::vectorizeQueryResult(res);

    ASSERT_EQ(rvec.size(), 2u);
    EXPECT_EQ(rvec[0], "pub1");
    EXPECT_EQ(rvec[1], "fed0/pub2");
    EXPECT_EQ(vFed2->query("fed0", "publications"), "[pub1;fed0/pub2]");
    EXPECT_EQ(vFed1->query("fed1", "isinit"), "true");

    EXPECT_EQ(vFed1->query("fed1", "publications"), "[fed1/pub3]");
    core = nullptr;
    vFed1->finalize();
    vFed2->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_P(query_type_tests, broker_queries)
{
    SetupTest<helics::ValueFederate>(GetParam(), 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    auto core = vFed1->getCorePointer();
    auto res = core->query("root", "federates");
    std::string str("[");
    str.append(vFed1->getName());
    str.push_back(';');
    str.append(vFed2->getName());
    str.push_back(']');
    EXPECT_EQ(res, "[fed0;fed1]");
    vFed1->enterInitializingModeAsync();
    vFed2->enterInitializingMode();
    vFed1->enterInitializingModeComplete();
    core = nullptr;
    vFed1->finalize();
    vFed2->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_P(query_type_tests, publication_fed_queries)
{
    SetupTest<helics::ValueFederate>(GetParam(), 2, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    // register the publications
    vFed1->registerPublication<double>("pub1");

    vFed2->registerPublication<double>("pub2");

    vFed2->registerPublication<double>("pub3");

    vFed1->enterInitializingModeAsync();
    vFed2->enterInitializingMode();
    vFed1->enterInitializingModeComplete();

    auto res = vFed1->query("federation", "publications");

    auto rvec = helics::vectorizeAndSortQueryResult(res);

    ASSERT_EQ(rvec.size(), 3u);
    EXPECT_EQ(rvec[0], "fed0/pub1");
    EXPECT_EQ(rvec[1], "fed1/pub2");
    EXPECT_EQ(rvec[2], "fed1/pub3");
    vFed1->finalize();
    vFed2->finalize();
}

INSTANTIATE_TEST_SUITE_P(query_tests, query_type_tests, ::testing::ValuesIn(core_types));

TEST_F(query_tests, test_federate_map)
{
    SetupTest<helics::ValueFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    auto core = vFed1->getCorePointer();
    auto res = core->query("root", "federate_map");
    vFed1->enterInitializingModeAsync();
    vFed2->enterInitializingMode();
    vFed1->enterInitializingModeComplete();
    auto val = loadJsonStr(res);
    EXPECT_EQ(val["cores"].size(), 1u);
    EXPECT_EQ(val["cores"][0]["federates"].size(), 2u);
    EXPECT_EQ(val["cores"][0]["parent"].asInt(), val["id"].asInt());
    auto v2 = val["cores"][0]["federates"][1];
    EXPECT_EQ(v2["parent"].asInt(), val["cores"][0]["id"].asInt());
    core = nullptr;
    vFed1->finalize();
    vFed2->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_F(query_tests, test_federate_map2)
{
    SetupTest<helics::ValueFederate>("test_2", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    auto core = vFed1->getCorePointer();
    auto res = core->query("root", "federate_map");
    vFed1->enterInitializingModeAsync();
    vFed2->enterInitializingMode();
    vFed1->enterInitializingModeComplete();
    auto val = loadJsonStr(res);
    EXPECT_EQ(val["cores"].size(), 2u);
    EXPECT_EQ(val["cores"][1]["federates"].size(), 1u);
    EXPECT_EQ(val["cores"][1]["parent"].asInt(), val["id"].asInt());
    auto v2 = val["cores"][1]["federates"][0];
    EXPECT_EQ(v2["parent"].asInt(), val["cores"][1]["id"].asInt());
    core = nullptr;
    vFed1->finalize();
    vFed2->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_F(query_tests, test_federate_map3)
{
    SetupTest<helics::ValueFederate>("test_3", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    auto core = vFed1->getCorePointer();
    auto res = core->query("root", "federate_map");
    vFed1->enterInitializingModeAsync();
    vFed2->enterInitializingMode();
    vFed1->enterInitializingModeComplete();
    auto val = loadJsonStr(res);
    EXPECT_EQ(val["cores"].size(), 0u);
    EXPECT_EQ(val["brokers"].size(), 1u);
    EXPECT_EQ(val["brokers"][0]["parent"].asInt(), val["id"].asInt());
    auto brk = val["brokers"][0];
    EXPECT_EQ(brk["cores"].size(), 2u);
    EXPECT_EQ(brk["brokers"].size(), 0u);
    EXPECT_EQ(brk["cores"][1]["federates"].size(), 1u);
    EXPECT_EQ(brk["cores"][1]["parent"].asInt(), brk["id"].asInt());
    auto v2 = brk["cores"][1]["federates"][0];
    EXPECT_EQ(v2["parent"].asInt(), brk["cores"][1]["id"].asInt());
    core = nullptr;
    vFed1->finalize();
    vFed2->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_F(query_tests, test_dependency_graph)
{
    SetupTest<helics::ValueFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    auto core = vFed1->getCorePointer();
    auto res = core->query("root", "dependency_graph");
    vFed1->enterInitializingModeAsync();
    vFed2->enterInitializingMode();
    vFed1->enterInitializingModeComplete();
    auto val = loadJsonStr(res);
    EXPECT_EQ(val["cores"].size(), 1u);
    EXPECT_EQ(val["cores"][0]["federates"].size(), 2u);
    EXPECT_EQ(val["cores"][0]["parent"].asInt(), val["id"].asInt());
    auto v2 = val["cores"][0]["federates"][1];
    EXPECT_EQ(v2["parent"].asInt(), val["cores"][0]["id"].asInt());
    core = nullptr;
    vFed1->finalize();
    vFed2->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_F(query_tests, test_updates_indices)
{
    SetupTest<helics::ValueFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto& p1 = vFed1->registerGlobalPublication<double>("pub1");
    auto& p2 = vFed1->registerGlobalPublication<double>("pub2");
    auto& p3 = vFed1->registerGlobalPublication<double>("pub3");

    vFed1->registerSubscription("pub1");
    vFed1->registerSubscription("pub2");
    vFed1->registerSubscription("pub3");

    vFed1->enterExecutingMode();
    p1.publish(45.7);
    p2.publish(23.1);
    p3.publish(19.4);
    vFed1->requestTime(1.0);

    auto qres = vFed1->query("updated_input_indices");
    EXPECT_EQ(qres, "[0;1;2]");
    vFed1->clearUpdates();
    qres = vFed1->query("updated_input_indices");
    EXPECT_EQ(qres, "[]");
    p1.publish(19.7);
    p3.publish(15.1);
    vFed1->requestTime(2.0);
    qres = vFed1->query("updated_input_indices");
    EXPECT_EQ(qres, "[0;2]");
    vFed1->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_F(query_tests, test_updates_names)
{
    SetupTest<helics::ValueFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto& p1 = vFed1->registerGlobalPublication<double>("pub1");
    auto& p2 = vFed1->registerGlobalPublication<double>("pub2");
    auto& p3 = vFed1->registerGlobalPublication<double>("pub3");

    vFed1->registerSubscription("pub1");
    vFed1->registerSubscription("pub2");
    vFed1->registerSubscription("pub3");

    vFed1->enterExecutingMode();
    p1.publish(45.7);
    p2.publish(23.1);
    p3.publish(19.4);
    vFed1->requestTime(1.0);

    auto qres = vFed1->query("updated_input_names");
    EXPECT_EQ(qres, "[pub1;pub2;pub3]");
    vFed1->clearUpdates();
    qres = vFed1->query("updated_input_names");
    EXPECT_EQ(qres, "[]");
    p1.publish(19.7);
    p3.publish(15.1);
    vFed1->requestTime(2.0);
    qres = vFed1->query("updated_input_names");
    EXPECT_EQ(qres, "[pub1;pub3]");
    vFed1->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_F(query_tests, test_update_values)
{
    SetupTest<helics::ValueFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto& p1 = vFed1->registerGlobalPublication<double>("pub1");
    auto& p2 = vFed1->registerGlobalPublication<double>("pub2");
    auto& p3 = vFed1->registerGlobalPublication<double>("pub3");

    vFed1->registerSubscription("pub1");
    vFed1->registerSubscription("pub2");
    vFed1->registerSubscription("pub3");

    vFed1->enterExecutingMode();
    p1.publish(45.7);
    p2.publish(23.1);
    p3.publish(19.4);
    vFed1->requestTime(1.0);

    auto qres = vFed1->query("updates");
    auto val = loadJsonStr(qres);

    EXPECT_EQ(val["pub1"].asDouble(), 45.7);
    EXPECT_EQ(val["pub2"].asDouble(), 23.1);
    EXPECT_EQ(val["pub3"].asDouble(), 19.4);
    vFed1->clearUpdates();
    qres = vFed1->query("updates");
    EXPECT_EQ(qres, "{}");
    p1.publish(19.7);
    p3.publish(15.1);
    vFed1->requestTime(2.0);
    qres = vFed1->query("updates");
    val = loadJsonStr(qres);

    EXPECT_EQ(val["pub1"].asDouble(), 19.7);
    EXPECT_TRUE(val["pub2"].isNull());
    EXPECT_EQ(val["pub3"].asDouble(), 15.1);
    vFed1->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_F(query_tests, test_update_values_local)
{
    SetupTest<helics::ValueFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto& p1 = vFed1->registerGlobalPublication<double>("pub1");
    auto& p2 = vFed1->registerPublication<double>("pub2");
    auto& p3 = vFed1->registerPublication<double>("pub3");

    vFed1->registerSubscription("pub1");
    vFed1->registerSubscription("fed0/pub2");
    vFed1->registerSubscription("fed0/pub3");

    vFed1->enterExecutingMode();
    p1.publish(45.7);
    p2.publish(23.1);
    p3.publish(19.4);
    vFed1->requestTime(1.0);

    auto qres = vFed1->query("updates");
    auto val = loadJsonStr(qres);

    EXPECT_EQ(val["pub1"].asDouble(), 45.7);
    EXPECT_EQ(val["fed0"]["pub2"].asDouble(), 23.1);
    EXPECT_EQ(val["fed0"]["pub3"].asDouble(), 19.4);
    vFed1->clearUpdates();
    qres = vFed1->query("updates");
    EXPECT_EQ(qres, "{}");
    p1.publish(19.7);
    p3.publish(15.1);
    vFed1->requestTime(2.0);
    qres = vFed1->query("updates");
    val = loadJsonStr(qres);

    EXPECT_EQ(val["pub1"].asDouble(), 19.7);
    EXPECT_TRUE(val["fed0"]["pub2"].isNull());
    EXPECT_EQ(val["fed0"]["pub3"].asDouble(), 15.1);
    vFed1->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_F(query_tests, test_update_values_all)
{
    SetupTest<helics::ValueFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto& p1 = vFed1->registerGlobalPublication<double>("pub1");
    auto& p2 = vFed1->registerGlobalPublication<double>("pub2");
    auto& p3 = vFed1->registerGlobalPublication<double>("pub3");

    vFed1->registerSubscription("pub1");
    vFed1->registerSubscription("pub2");
    vFed1->registerSubscription("pub3");

    vFed1->enterExecutingMode();
    p1.publish(45.7);
    p2.publish(23.1);
    p3.publish(19.4);
    vFed1->requestTime(1.0);

    auto qres = vFed1->query("values");
    auto val = loadJsonStr(qres);

    EXPECT_EQ(val["pub1"].asDouble(), 45.7);
    EXPECT_EQ(val["pub2"].asDouble(), 23.1);
    EXPECT_EQ(val["pub3"].asDouble(), 19.4);
    vFed1->clearUpdates();
    auto qres2 = vFed1->query("values");
    EXPECT_EQ(qres, qres2);
    p1.publish(19.7);
    p3.publish(15.1);
    vFed1->requestTime(2.0);
    qres = vFed1->query("values");
    val = loadJsonStr(qres);

    EXPECT_EQ(val["pub1"].asDouble(), 19.7);
    EXPECT_EQ(val["pub2"].asDouble(), 23.1);
    EXPECT_EQ(val["pub3"].asDouble(), 15.1);
    vFed1->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_F(query_tests, test_query_subscriptions)
{
    SetupTest<helics::ValueFederate>("zmq2", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    vFed1->registerGlobalPublication<double>("pub1");
    vFed1->registerGlobalPublication<double>("pub2");
    vFed1->registerGlobalPublication<double>("pub3");

    vFed2->registerSubscription("pub1");
    vFed2->registerSubscription("pub2");
    vFed2->registerSubscription("pub3");
    vFed1->enterInitializingModeAsync();
    vFed2->enterInitializingMode();
    vFed1->enterInitializingModeComplete();

    auto subs = helics::queryFederateSubscriptions(vFed1.get(), "fed1");
    EXPECT_EQ(subs, "[pub1;pub2;pub3]");
    vFed1->finalize();
    vFed2->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_F(query_tests, test_queries_query)
{
    SetupTest<helics::CombinationFederate>("zmq2", 2);
    auto vFed1 = GetFederateAs<helics::CombinationFederate>(0);
    auto vFed2 = GetFederateAs<helics::CombinationFederate>(1);
    vFed1->registerGlobalPublication<double>("pub1");
    vFed1->registerGlobalPublication<double>("pub2");
    vFed1->registerGlobalEndpoint("pub3");

    vFed2->registerSubscription("pub1");
    vFed2->registerSubscription("pub2");
    auto& f1 = vFed2->registerFilter("f1");
    f1.addSourceTarget("pub3");
    f1.addDestinationTarget("pub3");
    vFed1->enterInitializingModeAsync();
    vFed2->enterInitializingMode();
    vFed1->enterInitializingModeComplete();

    // federate queries
    auto res = vFed1->query("queries");
    auto vec = helics::vectorizeQueryResult(res);
    for (auto& qstr : vec) {
        auto qres = vFed1->query(qstr);
        EXPECT_NE(qres, "#invalid") << qstr << " produced #invalid";
    }

    res = vFed1->query("core", "queries");
    vec = helics::vectorizeQueryResult(res);
    for (auto& qstr : vec) {
        auto qres = vFed1->query("core", qstr);
        EXPECT_NE(qres, "#invalid") << qstr << " produced #invalid in core";
    }

    res = vFed1->query("root", "queries");
    vec = helics::vectorizeQueryResult(res);
    for (auto& qstr : vec) {
        auto qres = vFed1->query("root", qstr);
        EXPECT_NE(qres, "#invalid") << qstr << " produced #invalid in core";
    }

    vFed1->finalize();
    vFed2->finalize();
    helics::cleanupHelicsLibrary();
}
