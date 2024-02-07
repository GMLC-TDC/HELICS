/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../application_api/testFixtures.hpp"
#include "gmlc/utilities/stringOps.h"
#include "helics/application_api/CombinationFederate.hpp"
#include "helics/application_api/CoreApp.hpp"
#include "helics/application_api/Filters.hpp"
#include "helics/application_api/Publications.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/application_api/queryFunctions.hpp"
#include "helics/common/JsonProcessingFunctions.hpp"
#include "helics/core/helicsVersion.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <thread>

struct query: public FederateTestFixture, public ::testing::Test {};

using gmlc::utilities::stringOps::removeQuotes;
class query_type: public ::testing::TestWithParam<const char*>, public FederateTestFixture {};
/** test simple creation and destruction*/
TEST_P(query_type, publication_queries)
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
    auto res = core->query("fed0", "publications", HELICS_SEQUENCING_MODE_FAST);
    EXPECT_EQ(res, R"(["pub1","fed0/pub2"])");
    auto rvec = helics::vectorizeQueryResult(res);

    ASSERT_EQ(rvec.size(), 2U);
    EXPECT_EQ(rvec[0], "pub1");
    EXPECT_EQ(rvec[1], "fed0/pub2");
    EXPECT_EQ(vFed2->query("fed0", "publications"), "[\"pub1\",\"fed0/pub2\"]");
    EXPECT_EQ(vFed1->query("fed1", "isinit"), "true");

    EXPECT_EQ(vFed1->query("fed1", "publications"), "[\"fed1/pub3\"]");
    core = nullptr;
    vFed1->finalize();
    vFed2->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_P(query_type, broker_queries)
{
    SetupTest<helics::ValueFederate>(GetParam(), 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    auto core = vFed1->getCorePointer();
    auto res = core->query("root", "federates", HELICS_SEQUENCING_MODE_FAST);
    std::string str("[");
    str.append(vFed1->getName());
    str.push_back(';');
    str.append(vFed2->getName());
    str.push_back(']');
    EXPECT_EQ(res, "[\"fed0\",\"fed1\"]");
    vFed1->enterInitializingModeAsync();
    vFed2->enterInitializingMode();
    vFed1->enterInitializingModeComplete();
    core = nullptr;
    vFed1->finalize();
    vFed2->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_P(query_type, publication_fed_queries)
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

    ASSERT_EQ(rvec.size(), 3U);
    EXPECT_EQ(rvec[0], "fed0/pub1");
    EXPECT_EQ(rvec[1], "fed1/pub2");
    EXPECT_EQ(rvec[2], "fed1/pub3");
    vFed1->finalize();
    vFed2->finalize();
}

using helics::fileops::loadJsonStr;

INSTANTIATE_TEST_SUITE_P(query, query_type, ::testing::ValuesIn(CoreTypes));

TEST_F(query, federate_map)
{
    SetupTest<helics::ValueFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    auto core = vFed1->getCorePointer();
    auto res = core->query("root", "federate_map", HELICS_SEQUENCING_MODE_FAST);
    vFed1->enterInitializingModeAsync();
    vFed2->enterInitializingMode();
    vFed1->enterInitializingModeComplete();
    auto val = loadJsonStr(res);
    EXPECT_EQ(val["cores"].size(), 1U);
    EXPECT_EQ(val["cores"][0]["federates"].size(), 2U);
    EXPECT_EQ(val["cores"][0]["attributes"]["parent"].asInt(), val["attributes"]["id"].asInt());
    auto v2 = val["cores"][0]["federates"][1];
    EXPECT_EQ(v2["attributes"]["parent"].asInt(), val["cores"][0]["attributes"]["id"].asInt());
    core = nullptr;
    vFed1->finalize();
    vFed2->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_F(query, federate_map2)
{
    SetupTest<helics::ValueFederate>("test_2", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    auto core = vFed1->getCorePointer();
    auto res = core->query("root", "federate_map", HELICS_SEQUENCING_MODE_FAST);
    vFed1->enterInitializingModeAsync();
    vFed2->enterInitializingMode();
    vFed1->enterInitializingModeComplete();
    auto val = loadJsonStr(res);
    EXPECT_EQ(val["cores"].size(), 2U);
    EXPECT_EQ(val["cores"][1]["federates"].size(), 1U);
    EXPECT_EQ(val["cores"][1]["attributes"]["parent"].asInt(), val["attributes"]["id"].asInt());
    auto v2 = val["cores"][1]["federates"][0];
    EXPECT_EQ(v2["attributes"]["parent"].asInt(), val["cores"][1]["attributes"]["id"].asInt());
    core = nullptr;
    vFed1->finalize();
    vFed2->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_F(query, federate_map3)
{
    SetupTest<helics::ValueFederate>("test_3", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    auto core = vFed1->getCorePointer();
    auto res = core->query("root", "federate_map", HELICS_SEQUENCING_MODE_FAST);
    vFed1->enterInitializingModeAsync();
    vFed2->enterInitializingMode();
    vFed1->enterInitializingModeComplete();
    auto val = loadJsonStr(res);
    EXPECT_EQ(val["cores"].size(), 0U);
    EXPECT_EQ(val["brokers"].size(), 1U);
    EXPECT_EQ(val["brokers"][0]["attributes"]["parent"].asInt(), val["attributes"]["id"].asInt());
    auto brk = val["brokers"][0];
    EXPECT_EQ(brk["cores"].size(), 2U);
    EXPECT_EQ(brk["brokers"].size(), 0U);
    EXPECT_EQ(brk["cores"][1]["federates"].size(), 1U);
    EXPECT_EQ(brk["cores"][1]["attributes"]["parent"].asInt(), brk["attributes"]["id"].asInt());
    auto v2 = brk["cores"][1]["federates"][0];
    EXPECT_EQ(v2["attributes"]["parent"].asInt(), brk["cores"][1]["attributes"]["id"].asInt());
    core = nullptr;
    vFed1->finalize();
    vFed2->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_F(query, dependency_graph)
{
    SetupTest<helics::ValueFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    auto core = vFed1->getCorePointer();
    auto res = core->query("root", "dependency_graph", HELICS_SEQUENCING_MODE_FAST);
    vFed1->enterInitializingModeAsync();
    vFed2->enterInitializingMode();
    vFed1->enterInitializingModeComplete();
    auto val = loadJsonStr(res);
    EXPECT_EQ(val["cores"].size(), 1U);
    EXPECT_EQ(val["cores"][0]["federates"].size(), 2U);
    EXPECT_EQ(val["cores"][0]["attributes"]["parent"].asInt(), val["attributes"]["id"].asInt());
    auto v2 = val["cores"][0]["federates"][1];
    EXPECT_EQ(v2["attributes"]["parent"].asInt(), val["cores"][0]["attributes"]["id"].asInt());
    core = nullptr;
    vFed1->finalize();
    vFed2->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_F(query, dependency_graph_reset)
{
    SetupTest<helics::ValueFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    vFed1->registerGlobalPublication<double>("test1");
    auto core = vFed1->getCorePointer();
    auto res1 = core->query("root", "dependency_graph", HELICS_SEQUENCING_MODE_FAST);
    vFed2->registerSubscription("test1");
    vFed1->enterInitializingModeAsync();
    vFed2->enterInitializingMode();
    vFed1->enterInitializingModeComplete();
    auto res2 = core->query("root", "dependency_graph", HELICS_SEQUENCING_MODE_FAST);
    EXPECT_NE(res1, res2);
    vFed1->finalize();
    vFed2->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_F(query, global_time)
{
    SetupTest<helics::ValueFederate>("test_3", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    auto core = vFed1->getCorePointer();

    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();

    auto res = core->query("root", "global_time", HELICS_SEQUENCING_MODE_FAST);

    auto val = loadJsonStr(res);
    EXPECT_EQ(val["cores"].size(), 0U);
    EXPECT_EQ(val["brokers"].size(), 1U);
    ASSERT_EQ(val["brokers"][0]["cores"].size(), 2U);
    EXPECT_EQ(val["brokers"][0]["cores"][0]["federates"].size(), 1U);
    EXPECT_EQ(val["brokers"][0]["cores"][1]["federates"].size(), 1U);
    EXPECT_EQ(val["brokers"][0]["cores"][0]["federates"][0]["send_time"].asDouble(), 0.0);
    EXPECT_EQ(val["brokers"][0]["cores"][0]["federates"][0]["granted_time"].asDouble(), 0.0);

    vFed2->requestTimeAsync(1.0);
    vFed1->requestTime(1.0);
    vFed2->requestTimeComplete();

    res = core->query("root", "global_time", HELICS_SEQUENCING_MODE_FAST);

    val = loadJsonStr(res);
    EXPECT_EQ(val["cores"].size(), 0U);
    EXPECT_EQ(val["brokers"].size(), 1U);
    ASSERT_EQ(val["brokers"][0]["cores"].size(), 2U);
    EXPECT_EQ(val["brokers"][0]["cores"][0]["federates"].size(), 1U);
    EXPECT_EQ(val["brokers"][0]["cores"][1]["federates"].size(), 1U);
    EXPECT_EQ(val["brokers"][0]["cores"][0]["federates"][0]["send_time"].asDouble(), 1.0);
    EXPECT_EQ(val["brokers"][0]["cores"][0]["federates"][0]["granted_time"].asDouble(), 1.0);

    core = nullptr;
    vFed1->finalize();
    vFed2->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_F(query, current_time)
{
    SetupTest<helics::MessageFederate>("test_3", 2);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    mFed1->registerEndpoint("ept1");
    mFed2->registerEndpoint("ept2");

    mFed1->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();

    mFed1->requestTimeAsync(1.0);
    mFed2->requestTime(1.0);
    mFed1->requestTimeComplete();
    auto res = mFed1->query("time");
    EXPECT_EQ(std::stod(res), 1.0);
    res = mFed1->query("current_time");
    auto val = loadJsonStr(res);
    EXPECT_EQ(val["granted_time"].asDouble(), 1.0);
    EXPECT_EQ(val["requested_time"].asDouble(), 1.0);

    mFed1->requestTimeAsync(3.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    res = mFed1->query("current_time");
    val = loadJsonStr(res);
    EXPECT_EQ(val["granted_time"].asDouble(), 1.0);
    EXPECT_EQ(val["requested_time"].asDouble(), 3.0);
    mFed2->requestTime(3.0);
    mFed1->requestTimeComplete();
    mFed1->query("broker", "flush");
    res = mFed1->query("broker", "current_time");
    val = loadJsonStr(res);
    EXPECT_EQ(val["time_next"].asDouble(), 3.0);

    res = mFed1->query("root", "current_time");
    EXPECT_EQ(res, "{}");
    mFed1->finalize();
    mFed2->finalize();
}

TEST_F(query, version)
{
    SetupTest<helics::MessageFederate>("test", 2);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    mFed1->registerEndpoint("ept1");
    mFed2->registerEndpoint("ept2");

    mFed1->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();

    mFed1->requestTimeAsync(1.0);
    mFed2->requestTime(1.0);
    mFed1->requestTimeComplete();
    auto res = removeQuotes(mFed1->query("version"));
    EXPECT_EQ(res, helics::versionString);
    res = removeQuotes(mFed1->query("broker", "version"));
    EXPECT_EQ(res, helics::versionString);
    res = removeQuotes(mFed1->query("core", "version"));
    EXPECT_EQ(res, helics::versionString);
    res = removeQuotes(mFed1->query("root", "version"));
    EXPECT_EQ(res, helics::versionString);
    res = mFed1->query("root", "version_all");
    auto val = loadJsonStr(res);
    EXPECT_EQ(val["version"].asString(), helics::versionString);
    mFed1->finalize();
    mFed2->finalize();
}

TEST_F(query, exists)
{
    SetupTest<helics::MessageFederate>("test_3", 2);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

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

    res = mFed1->query(mFed1->getCorePointer()->getIdentifier(), "exists");
    EXPECT_EQ(res, "true");

    res = mFed1->query(mFed2->getCorePointer()->getIdentifier(), "exists");
    EXPECT_EQ(res, "true");

    auto brk = brokers.front();
    res = mFed1->query(brk->getIdentifier(), "exists");
    EXPECT_EQ(res, "true");

    res = brk->query(mFed1->getName(), "exists");
    EXPECT_EQ(res, "true");

    res = brk->query(mFed2->getName(), "exists");
    EXPECT_EQ(res, "true");

    res = brk->query("root", "exists");
    EXPECT_EQ(res, "true");

    res = brk->query(mFed1->getCorePointer()->getIdentifier(), "exists");
    EXPECT_EQ(res, "true");

    res = brk->query(mFed2->getCorePointer()->getIdentifier(), "exists");
    EXPECT_EQ(res, "true");

    res = brk->query("unknown_fed", "exists");
    EXPECT_EQ(res, "false");

    res = mFed1->getCorePointer()->query("unknown_fed", "exists", HELICS_SEQUENCING_MODE_FAST);

    mFed1->finalize();
    mFed2->finalize();
}

TEST_F(query, current_state)
{
    SetupTest<helics::ValueFederate>("test_2", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    auto core = vFed1->getCorePointer();

    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();

    auto res = core->query("root", "current_state", HELICS_SEQUENCING_MODE_FAST);

    auto val = loadJsonStr(res);
    EXPECT_EQ(val["federates"].size(), 2U);
    EXPECT_EQ(val["cores"].size(), 2U);
    EXPECT_STREQ(val["federates"][0]["state"].asCString(), "connected");

    vFed1->localError(-3, "test error");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    res = core->query("root", "current_state", HELICS_SEQUENCING_MODE_FAST);

    val = loadJsonStr(res);
    EXPECT_EQ(val["federates"].size(), 2U);
    EXPECT_EQ(val["cores"].size(), 2U);
    EXPECT_EQ(val["brokers"].size(), 0U);
    EXPECT_STREQ(val["federates"][0]["state"].asCString(), "error");

    vFed2->finalize();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    res = core->query("root", "current_state", HELICS_SEQUENCING_MODE_FAST);

    val = loadJsonStr(res);
    EXPECT_EQ(val["federates"].size(), 2U);
    EXPECT_STREQ(val["federates"][1]["state"].asCString(), "disconnected");
    EXPECT_STREQ(val["cores"][1]["state"].asCString(), "disconnected");
    core = nullptr;

    vFed1->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_F(query, global_state)
{
    SetupTest<helics::ValueFederate>("test_2", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    auto core = vFed1->getCorePointer();

    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();

    auto res = core->query("root", "global_state", HELICS_SEQUENCING_MODE_FAST);

    auto val = loadJsonStr(res);
    EXPECT_EQ(val["cores"].size(), 2U);
    EXPECT_EQ(val["cores"][0]["federates"].size(), 1U);
    EXPECT_STREQ(val["cores"][0]["federates"][0]["state"].asCString(), "executing");

    vFed1->localError(-3, "test error");

    EXPECT_THROW(vFed1->requestTime(2.0), helics::HelicsException);
    res = core->query("root", "global_state", HELICS_SEQUENCING_MODE_FAST);

    val = loadJsonStr(res);
    EXPECT_EQ(val["cores"].size(), 2U);
    EXPECT_EQ(val["cores"][0]["federates"].size(), 1U);
    if (val["cores"][0]["federates"][0]["attributes"]["name"].asString() == "fed0") {
        EXPECT_STREQ(val["cores"][0]["federates"][0]["state"].asCString(), "error");
    } else {
        EXPECT_STREQ(val["cores"][1]["federates"][0]["state"].asCString(), "error");
    }

    vFed2->finalize();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    res = core->query("root", "global_state", HELICS_SEQUENCING_MODE_FAST);

    val = loadJsonStr(res);
    EXPECT_EQ(val["cores"].size(), 2U);
    EXPECT_EQ(val["cores"][1]["federates"].size(), 1U);
    EXPECT_STREQ(val["cores"][1]["federates"][0]["state"].asCString(), "error");
    EXPECT_STREQ(val["cores"][0]["state"].asCString(), "disconnected");
    core = nullptr;

    vFed1->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_F(query, current_state_core)
{
    SetupTest<helics::ValueFederate>("test_2", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();

    auto res = vFed1->query("core", "current_state");

    auto val = loadJsonStr(res);
    EXPECT_EQ(val["federates"].size(), 1U);
    EXPECT_STREQ(val["federates"][0]["state"].asCString(), "connected");

    vFed1->localError(-3, "test error");
    EXPECT_THROW(vFed1->requestTime(1.0), helics::HelicsException);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    res = vFed1->query("core", "current_state");

    val = loadJsonStr(res);
    EXPECT_EQ(val["federates"].size(), 1U);
    EXPECT_STREQ(val["federates"][0]["state"].asCString(), "error");

    vFed2->finalize();

    vFed1->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_F(query, data_flow_graph)
{
    SetupTest<helics::ValueFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    vFed1->registerGlobalInput<double>("ipt1");
    auto& p1 = vFed2->registerGlobalPublication<double>("pub1");
    p1.addTarget("ipt1");
    vFed1->enterInitializingModeAsync();
    vFed2->enterInitializingMode();
    vFed1->enterInitializingModeComplete();
    auto core = vFed1->getCorePointer();
    auto res = core->query("root", "data_flow_graph", HELICS_SEQUENCING_MODE_FAST);
    auto val = loadJsonStr(res);
    EXPECT_EQ(val["cores"].size(), 1U);
    EXPECT_EQ(val["cores"][0]["federates"].size(), 2U);
    EXPECT_EQ(val["cores"][0]["parent"].asInt(), val["id"].asInt());
    auto v2 = val["cores"][0]["federates"][1];
    auto v1 = val["cores"][0]["federates"][0];
    EXPECT_EQ(v2["parent"].asInt(), val["cores"][0]["id"].asInt());
    EXPECT_EQ(v2["publications"].size(), 1U);
    EXPECT_EQ(v1["inputs"].size(), 1U);
    EXPECT_EQ(v1["inputs"][0]["key"], "ipt1");
    EXPECT_EQ(v2["publications"][0]["key"], "pub1");
    EXPECT_EQ(v1["inputs"][0]["sources"].size(), 1U);
    EXPECT_EQ(v2["publications"][0]["targets"].size(), 1U);
    core = nullptr;
    vFed1->finalize();
    vFed2->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_F(query, interfaces)
{
    SetupTest<helics::CombinationFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::CombinationFederate>(0);

    auto& i1 = vFed1->registerGlobalInput<double>("ipt1", "kV");
    auto& p1 = vFed1->registerGlobalPublication<double>("pub1", "V");
    auto& e1 = vFed1->registerGlobalEndpoint("ept1", "type1");
    p1.addTarget("ipt1");
    p1.setTag("tag1", "val1");

    i1.setTag("tag2", "val2");
    e1.setTag("tag3", "val3");
    vFed1->enterInitializingMode();
    auto core = vFed1->getCorePointer();
    auto res = core->query("fed0", "interfaces", HELICS_SEQUENCING_MODE_FAST);
    core = nullptr;
    vFed1->finalize();
    auto val = loadJsonStr(res);
    EXPECT_STREQ(val["inputs"][0]["name"].asCString(), "ipt1");
    EXPECT_STREQ(val["publications"][0]["name"].asCString(), "pub1");
    EXPECT_STREQ(val["endpoints"][0]["name"].asCString(), "ept1");
    EXPECT_STREQ(val["inputs"][0]["type"].asCString(), "double");
    EXPECT_STREQ(val["publications"][0]["type"].asCString(), "double");
    EXPECT_STREQ(val["endpoints"][0]["type"].asCString(), "type1");

    EXPECT_STREQ(val["inputs"][0]["units"].asCString(), "kV");
    EXPECT_STREQ(val["publications"][0]["units"].asCString(), "V");

    ASSERT_TRUE(val["inputs"][0]["tags"].isArray());
    ASSERT_TRUE(val["publications"][0]["tags"].isArray());
    ASSERT_TRUE(val["endpoints"][0]["tags"].isArray());
    EXPECT_STREQ(val["publications"][0]["tags"][0]["value"].asCString(), "val1");
    EXPECT_STREQ(val["inputs"][0]["tags"][0]["value"].asCString(), "val2");
    EXPECT_STREQ(val["endpoints"][0]["tags"][0]["value"].asCString(), "val3");
    EXPECT_STREQ(val["publications"][0]["tags"][0]["name"].asCString(), "tag1");
    EXPECT_STREQ(val["inputs"][0]["tags"][0]["name"].asCString(), "tag2");
    EXPECT_STREQ(val["endpoints"][0]["tags"][0]["name"].asCString(), "tag3");
    helics::cleanupHelicsLibrary();
}

#ifdef HELICS_ENABLE_ZMQ_CORE
TEST_F(query, interfaces_json_serialization)
{
    extraCoreArgs = "--json";
    SetupTest<helics::CombinationFederate>("zmq4", 1);
    auto vFed1 = GetFederateAs<helics::CombinationFederate>(0);

    auto& i1 = vFed1->registerGlobalInput<double>("ipt1", "kV");
    auto& p1 = vFed1->registerGlobalPublication<double>("pub1", "V");
    auto& e1 = vFed1->registerGlobalEndpoint("ept1", "type1");
    p1.addTarget("ipt1");
    p1.setTag("tag1", "val1");

    i1.setTag("tag2", "val2");
    e1.setTag("tag3", "val3");
    vFed1->enterInitializingMode();
    auto core = vFed1->getCorePointer();
    auto res = core->query("fed0", "interfaces", HELICS_SEQUENCING_MODE_FAST);
    core = nullptr;
    vFed1->finalize();
    auto val = loadJsonStr(res);
    EXPECT_STREQ(val["inputs"][0]["name"].asCString(), "ipt1");
    EXPECT_STREQ(val["publications"][0]["name"].asCString(), "pub1");
    EXPECT_STREQ(val["endpoints"][0]["name"].asCString(), "ept1");
    EXPECT_STREQ(val["inputs"][0]["type"].asCString(), "double");
    EXPECT_STREQ(val["publications"][0]["type"].asCString(), "double");
    EXPECT_STREQ(val["endpoints"][0]["type"].asCString(), "type1");

    EXPECT_STREQ(val["inputs"][0]["units"].asCString(), "kV");
    EXPECT_STREQ(val["publications"][0]["units"].asCString(), "V");

    ASSERT_TRUE(val["inputs"][0]["tags"].isArray());
    ASSERT_TRUE(val["publications"][0]["tags"].isArray());
    ASSERT_TRUE(val["endpoints"][0]["tags"].isArray());
    EXPECT_STREQ(val["publications"][0]["tags"][0]["value"].asCString(), "val1");
    EXPECT_STREQ(val["inputs"][0]["tags"][0]["value"].asCString(), "val2");
    EXPECT_STREQ(val["endpoints"][0]["tags"][0]["value"].asCString(), "val3");
    EXPECT_STREQ(val["publications"][0]["tags"][0]["name"].asCString(), "tag1");
    EXPECT_STREQ(val["inputs"][0]["tags"][0]["name"].asCString(), "tag2");
    EXPECT_STREQ(val["endpoints"][0]["tags"][0]["name"].asCString(), "tag3");
    helics::cleanupHelicsLibrary();
}
#endif

TEST_F(query, federate_tags)
{
    SetupTest<helics::CombinationFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::CombinationFederate>(0);

    vFed1->setTag("description", "a federate description");
    vFed1->setTag("version", "1.4.5");

    EXPECT_EQ(vFed1->getTag("version"), "1.4.5");
    EXPECT_EQ(vFed1->getTag("description"), "a federate description");
    // test an unknown tag
    EXPECT_TRUE(vFed1->getTag("nonatag").empty());

    vFed1->enterInitializingMode();
    auto core = vFed1->getCorePointer();
    auto res = core->query("fed0", "tags", HELICS_SEQUENCING_MODE_FAST);
    auto desc = core->query("fed0", "description", HELICS_SEQUENCING_MODE_FAST);

    EXPECT_EQ(desc, "\"a federate description\"");
    core = nullptr;
    vFed1->finalize();
    auto val = loadJsonStr(res);
    EXPECT_STREQ(val["version"].asCString(), "1.4.5");
    EXPECT_STREQ(val["description"].asCString(), "a federate description");

    helics::cleanupHelicsLibrary();
}

TEST_F(query, core_tags)
{
    SetupTest<helics::CombinationFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::CombinationFederate>(0);
    ASSERT_TRUE(vFed1);
    auto cr = helics::CoreApp(vFed1->getCorePointer());
    cr.setTag("description", "a core description");
    cr.setTag("version", "1.4.6");

    EXPECT_EQ(cr.getTag("version"), "1.4.6");
    EXPECT_EQ(cr.getTag("description"), "a core description");

    // test an unknown tag
    EXPECT_TRUE(cr.getTag("nonatag").empty());

    vFed1->enterInitializingMode();

    auto res = vFed1->query("core", "tags", HELICS_SEQUENCING_MODE_FAST);

    auto desc = vFed1->query("core", "description");
    EXPECT_EQ(desc, "\"a core description\"");
    cr.reset();
    vFed1->finalize();
    auto val = loadJsonStr(res);
    EXPECT_STREQ(val["version"].asCString(), "1.4.6");
    EXPECT_STREQ(val["description"].asCString(), "a core description");

    helics::cleanupHelicsLibrary();
}

TEST_F(query, data_flow_graph_ordered)
{
    SetupTest<helics::ValueFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    vFed1->registerGlobalInput<double>("ipt1");
    auto& p1 = vFed2->registerGlobalPublication<double>("pub1");
    p1.addTarget("ipt1");
    vFed1->enterInitializingModeAsync();
    vFed2->enterInitializingMode();
    vFed1->enterInitializingModeComplete();
    auto core = vFed1->getCorePointer();
    auto res = core->query("root", "data_flow_graph", HELICS_SEQUENCING_MODE_ORDERED);
    auto val = loadJsonStr(res);
    EXPECT_EQ(val["cores"].size(), 1U);
    EXPECT_EQ(val["cores"][0]["federates"].size(), 2U);
    EXPECT_EQ(val["cores"][0]["attributes"]["parent"].asInt(), val["attributes"]["id"].asInt());
    auto v2 = val["cores"][0]["federates"][1];
    auto v1 = val["cores"][0]["federates"][0];
    EXPECT_EQ(v2["attributes"]["parent"].asInt(), val["cores"][0]["attributes"]["id"].asInt());
    EXPECT_EQ(v2["publications"].size(), 1U);
    EXPECT_EQ(v1["inputs"].size(), 1U);
    EXPECT_EQ(v1["inputs"][0]["key"], "ipt1");
    EXPECT_EQ(v2["publications"][0]["key"], "pub1");
    EXPECT_EQ(v1["inputs"][0]["sources"].size(), 1U);
    EXPECT_EQ(v2["publications"][0]["targets"].size(), 1U);
    core = nullptr;
    vFed1->finalize();
    vFed2->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_F(query, data_flow_graph_concurrent)
{
    SetupTest<helics::ValueFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    vFed1->registerGlobalInput<double>("ipt1");
    auto& p1 = vFed2->registerGlobalPublication<double>("pub1");
    p1.addTarget("ipt1");
    vFed1->enterInitializingModeAsync();
    vFed2->enterInitializingMode();
    vFed1->enterInitializingModeComplete();

    vFed1->enterExecutingModeAsync();
    auto core = vFed1->getCorePointer();
    auto res = core->query("root", "data_flow_graph", HELICS_SEQUENCING_MODE_FAST);
    auto val = loadJsonStr(res);
    EXPECT_EQ(val["cores"].size(), 1U);
    EXPECT_EQ(val["cores"][0]["federates"].size(), 2U);
    EXPECT_EQ(val["cores"][0]["attributes"]["parent"].asInt(), val["attributes"]["id"].asInt());
    auto v2 = val["cores"][0]["federates"][1];
    auto v1 = val["cores"][0]["federates"][0];
    if (v1["attributes"]["id"].asInt() > v2["attributes"]["id"].asInt()) {
        std::swap(v1, v2);
    }
    EXPECT_EQ(v2["attributes"]["parent"].asInt(), val["cores"][0]["attributes"]["id"].asInt());
    EXPECT_EQ(v2["publications"].size(), 1U);
    EXPECT_EQ(v1["inputs"].size(), 1U);
    EXPECT_EQ(v1["inputs"][0]["key"], "ipt1");
    EXPECT_EQ(v2["publications"][0]["key"], "pub1");
    EXPECT_EQ(v1["inputs"][0]["sources"].size(), 1U);
    EXPECT_EQ(v2["publications"][0]["targets"].size(), 1U);
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();
    core = nullptr;
    vFed1->finalize();
    vFed2->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_F(query, updates_indices)
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
    EXPECT_EQ(qres, "[0,1,2]");
    vFed1->clearUpdates();
    qres = vFed1->query("updated_input_indices");
    EXPECT_EQ(qres, "[]");
    p1.publish(19.7);
    p3.publish(15.1);
    vFed1->requestTime(2.0);
    qres = vFed1->query("updated_input_indices");
    EXPECT_EQ(qres, "[0,2]");
    vFed1->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_F(query, updates_names)
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
    auto res = helics::vectorizeQueryResult(qres);
    EXPECT_EQ(res.size(), 3U);
    vFed1->clearUpdates();
    qres = vFed1->query("updated_input_names");
    EXPECT_EQ(qres, "[]");
    p1.publish(19.7);
    p3.publish(15.1);
    vFed1->requestTime(2.0);
    qres = vFed1->query("updated_input_names");
    res = helics::vectorizeQueryResult(qres);
    EXPECT_EQ(res.size(), 2U);
    vFed1->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_F(query, update_values)
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

TEST_F(query, update_values_local)
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

TEST_F(query, update_values_all)
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

TEST_F(query, global_flush)
{
    SetupTest<helics::ValueFederate>("test_4", 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto& p1 = vFed1->registerGlobalPublication<double>("pub1");

    vFed1->registerSubscription("pub1");

    vFed1->enterExecutingMode();
    p1.publish(45.7);
    vFed1->requestTime(1.0);

    auto qres = vFed1->query("root", "global_flush");
    vFed1->finalize();
    helics::cleanupHelicsLibrary();
    auto val = loadJsonStr(qres);
    ASSERT_TRUE(val["status"].isBool());
    EXPECT_TRUE(val["status"].asBool());
}

#ifdef HELICS_ENABLE_ZMQ_CORE
TEST_F(query, query_subscriptions)
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
    EXPECT_EQ(subs, "[\"pub1\",\"pub2\",\"pub3\"]");
    vFed1->finalize();
    vFed2->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_F(query, queries_query)
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
    bool skipversion = std::string(helics::versionString).find("error") != std::string::npos;

    for (auto& qstr : vec) {
        if (skipversion) {
            if (qstr == "version" || qstr == "version_all") {
                continue;
            }
        }
        auto qres = vFed1->query(qstr);
        EXPECT_EQ(qres.find("error"), std::string::npos) << qstr << " produced an error";
        try {
            auto v = loadJsonStr(qres);
        }
        catch (...) {
            EXPECT_TRUE(false) << "Unable to load JSON string " << qstr;
        }
    }

    res = vFed1->query("core", "queries");
    vec = helics::vectorizeQueryResult(res);
    for (auto& qstr : vec) {
        if (skipversion) {
            if (qstr == "version" || qstr == "version_all") {
                continue;
            }
        }
        auto qres = vFed1->query("core", qstr);
        EXPECT_EQ(qres.find("error"), std::string::npos) << qstr << " produced an error in core";
        try {
            auto v = loadJsonStr(qres);
        }
        catch (...) {
            EXPECT_TRUE(false) << "Unable to load JSON string " << qstr;
        }
    }

    res = vFed1->query("root", "queries");
    vec = helics::vectorizeQueryResult(res);
    for (auto& qstr : vec) {
        if (skipversion) {
            if (qstr == "version" || qstr == "version_all") {
                continue;
            }
        }
        auto qres = vFed1->query("root", qstr);
        EXPECT_EQ(qres.find("error"), std::string::npos) << qstr << " produced an error in root";
        try {
            auto v = loadJsonStr(qres);
        }
        catch (...) {
            EXPECT_TRUE(false) << "Unable to load JSON string " << qstr;
        }
    }

    vFed1->finalize();
    vFed2->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_F(query, queries_callback_test)
{
    SetupTest<helics::ValueFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    vFed1->setQueryCallback([](std::string_view queryStr) {
        return (queryStr == "abc") ? std::string("AAAA") : std::string("BBBB");
    });

    auto res = vFed1->query("abc");
    EXPECT_EQ(res, "AAAA");
    res = vFed1->query("bca");
    EXPECT_EQ(res, "BBBB");
    vFed1->enterInitializingMode();
    vFed1->finalize();
}

TEST_F(query, concurrent_callback)
{
    SetupTest<helics::ValueFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    vFed1->registerGlobalInput<double>("ipt1");
    auto& p1 = vFed2->registerGlobalPublication<double>("pub1");
    p1.addTarget("ipt1");

    vFed1->setQueryCallback([](std::string_view queryStr) {
        return (queryStr == "abc") ? std::string("AAAA") : std::string("BBBB");
    });

    vFed1->enterInitializingModeAsync();
    vFed2->enterInitializingMode();
    vFed1->enterInitializingModeComplete();

    vFed1->enterExecutingModeAsync();
    auto core = vFed1->getCorePointer();
    auto res = core->query(vFed1->getName(), "abc", HELICS_SEQUENCING_MODE_FAST);
    EXPECT_EQ(res, "AAAA");
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();
    res = core->query(vFed1->getName(), "bca", HELICS_SEQUENCING_MODE_FAST);
    EXPECT_EQ(res, "BBBB");
    core = nullptr;
    vFed1->finalize();
    vFed2->finalize();
    helics::cleanupHelicsLibrary();
}

TEST_F(query, queries_disconnected)
{
    SetupTest<helics::ValueFederate>("test_4", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();

    auto res = vFed1->query(vFed2->getName(), "dependency_graph");
    EXPECT_NE(res[0], '#');
    vFed2->finalize();
    vFed1->requestTime(3.0);
    res = vFed1->query(vFed2->getName(), "state");
    int ii{0};
    while (res.find("disconnected") == std::string::npos) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        res = vFed1->query(vFed2->getName(), "state");
        if (++ii > 10) {
            break;
        }
    }
    EXPECT_THAT(res, ::testing::HasSubstr("disconnected"));
    res = vFed1->query(vFed2->getName(), "dependency_graph");
    EXPECT_THAT(res, ::testing::HasSubstr("disconnected"));
    EXPECT_THAT(res, ::testing::HasSubstr("error"));
    vFed1->finalize();
}

// the purpose of this test is to make sure the calls don't block when a federate disconnects
TEST_F(query, queries_disconnected_global)
{
    SetupTest<helics::ValueFederate>("test_2", 3);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    auto vFed3 = GetFederateAs<helics::ValueFederate>(2);
    vFed1->enterExecutingModeAsync();
    vFed3->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();
    vFed3->enterExecutingModeComplete();

    auto res = brokers[0]->query("root", "global_time");
    try {
        auto v = loadJsonStr(res);
        (void)(v);
    }
    catch (...) {
        EXPECT_TRUE(false) << "Unable to load JSON string " << res;
    }
    vFed2->finalize();
    vFed1->requestTime(3.0);
    res = brokers[0]->query("root", "global_time");
    try {
        auto v = loadJsonStr(res);
        (void)(v);
    }
    catch (...) {
        EXPECT_TRUE(false) << "Unable to load JSON string " << res;
    }
    vFed2->finalize();
    res = brokers[0]->query("root", "global_time");
    try {
        auto v = loadJsonStr(res);
        (void)(v);
    }
    catch (...) {
        EXPECT_TRUE(false) << "Unable to load JSON string " << res;
    }
    vFed3->finalize();
    res = brokers[0]->query("root", "global_time");
    try {
        auto v = loadJsonStr(res);
        (void)(v);
    }
    catch (...) {
        EXPECT_TRUE(false) << "Unable to load JSON string " << res;
    }
}

TEST_F(query, queries_timeout_ci_skip)
{
    extraCoreArgs = "--querytimeout=100ms --tick=100ms";
    SetupTest<helics::ValueFederate>("test_2", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    vFed1->setQueryCallback([](std::string_view queryStr) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        return (queryStr == "abc") ? std::string("AAAA") : std::string("BBBB");
    });

    auto res = vFed2->query(vFed1->getName(), "abc");
    EXPECT_THAT(res, ::testing::HasSubstr("timeout"));

    vFed2->enterExecutingModeAsync();
    vFed1->enterExecutingMode();
    vFed2->enterExecutingModeComplete();
    vFed1->finalize();
    vFed2->finalize();
}

TEST_F(query, broker_queries_timeout_ci_skip)
{
    extraBrokerArgs = "--querytimeout=100ms --tick=100ms";
    SetupTest<helics::ValueFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    vFed1->setQueryCallback([](std::string_view queryStr) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        return (queryStr == "abc") ? std::string("AAAA") : std::string("BBBB");
    });

    auto res = brokers[0]->query(vFed1->getName(), "abc");
    EXPECT_THAT(res, ::testing::HasSubstr("timeout"));

    vFed1->enterExecutingMode();
    vFed1->finalize();
}
#endif
