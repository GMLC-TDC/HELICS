/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "../application_api/testFixtures.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/application_api/queryFunctions.hpp"
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include "helics/common/JsonProcessingFunctions.hpp"

namespace bdata = boost::unit_test::data;
namespace utf = boost::unit_test;

BOOST_FIXTURE_TEST_SUITE (query_tests, FederateTestFixture)

/** test simple creation and destruction*/
BOOST_DATA_TEST_CASE (test_publication_queries, bdata::make (core_types), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 2, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);
    // register the publications
    vFed1->registerGlobalPublication<double> ("pub1");

    vFed2->registerSubscription<double> ("pub1");

    vFed1->registerPublication<double> ("pub2");

    vFed2->registerPublication<double> ("pub3");

    vFed1->enterInitializingModeAsync ();
    vFed2->enterInitializingMode ();
    vFed1->enterInitializingModeComplete ();

    auto core = vFed1->getCorePointer ();
    auto res = core->query ("fed0", "publications");
    BOOST_CHECK_EQUAL (res, "[pub1;fed0/pub2]");
    auto rvec = vectorizeQueryResult (res);

    BOOST_REQUIRE_EQUAL (rvec.size (), 2);
    BOOST_CHECK_EQUAL (rvec[0], "pub1");
    BOOST_CHECK_EQUAL (rvec[1], "fed0/pub2");
    BOOST_CHECK_EQUAL (vFed2->query ("fed0", "publications"), "[pub1;fed0/pub2]");
    BOOST_CHECK_EQUAL (vFed1->query ("fed1", "isinit"), "true");

    BOOST_CHECK_EQUAL (vFed1->query ("fed1", "publications"), "[fed1/pub3]");
    core = nullptr;
    vFed1->finalize ();
    vFed2->finalize ();
    helics::cleanupHelicsLibrary ();
}

BOOST_DATA_TEST_CASE (test_broker_queries, bdata::make (core_types), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);
    auto core = vFed1->getCorePointer ();
    auto res = core->query ("root", "federates");
    std::string str ("[");
    str.append (vFed1->getName ());
    str.push_back (';');
    str.append (vFed2->getName ());
    str.push_back (']');
    BOOST_CHECK_EQUAL (res, "[fed0;fed1]");
    vFed1->enterInitializingModeAsync ();
    vFed2->enterInitializingMode ();
    vFed1->enterInitializingModeComplete ();
    core = nullptr;
    vFed1->finalize ();
    vFed2->finalize ();
    helics::cleanupHelicsLibrary ();
}

BOOST_DATA_TEST_CASE (test_publication_fed_queries, bdata::make (core_types), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 2, 1.0);
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

    BOOST_REQUIRE_EQUAL (rvec.size (), 3);
    BOOST_CHECK_EQUAL (rvec[0], "fed0/pub1");
    BOOST_CHECK_EQUAL (rvec[1], "fed1/pub2");
    BOOST_CHECK_EQUAL (rvec[2], "fed1/pub3");
    vFed1->finalize ();
    vFed2->finalize ();
}

BOOST_AUTO_TEST_CASE (test_federate_map)
{
    SetupTest<helics::ValueFederate> ("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);
    auto core = vFed1->getCorePointer ();
    auto res = core->query ("root", "federate_map");
    vFed1->enterInitializingModeAsync ();
    vFed2->enterInitializingMode ();
    vFed1->enterInitializingModeComplete ();
    auto val = loadJsonStr(res);
    BOOST_CHECK_EQUAL(val["cores"].size(), 1u);
    BOOST_CHECK_EQUAL(val["cores"][0]["federates"].size(), 2u);
    BOOST_CHECK_EQUAL(val["cores"][0]["parent"].asInt(), val["id"].asInt());
    auto v2 = val["cores"][0]["federates"][1];
    BOOST_CHECK_EQUAL(v2["parent"].asInt(), val["cores"][0]["id"].asInt());
    core = nullptr;
    vFed1->finalize ();
    vFed2->finalize ();
    helics::cleanupHelicsLibrary ();
}

BOOST_AUTO_TEST_CASE(test_federate_map2)
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
    BOOST_CHECK_EQUAL(val["cores"].size(), 2u);
    BOOST_CHECK_EQUAL(val["cores"][1]["federates"].size(), 1u);
    BOOST_CHECK_EQUAL(val["cores"][1]["parent"].asInt(), val["id"].asInt());
    auto v2 = val["cores"][1]["federates"][0];
    BOOST_CHECK_EQUAL(v2["parent"].asInt(), val["cores"][1]["id"].asInt());
    core = nullptr;
    vFed1->finalize();
    vFed2->finalize();
    helics::cleanupHelicsLibrary();
}

BOOST_AUTO_TEST_CASE(test_federate_map3)
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
    BOOST_CHECK_EQUAL(val["cores"].size(), 0u);
    BOOST_CHECK_EQUAL(val["brokers"].size(), 1u);
    BOOST_CHECK_EQUAL(val["brokers"][0]["parent"].asInt(), val["id"].asInt());
    auto brk = val["brokers"][0];
    BOOST_CHECK_EQUAL(brk["cores"].size(), 2u);
    BOOST_CHECK_EQUAL(brk["brokers"].size(), 0u);
    BOOST_CHECK_EQUAL(brk["cores"][1]["federates"].size(), 1u);
    BOOST_CHECK_EQUAL(brk["cores"][1]["parent"].asInt(), brk["id"].asInt());
    auto v2 = brk["cores"][1]["federates"][0];
    BOOST_CHECK_EQUAL(v2["parent"].asInt(), brk["cores"][1]["id"].asInt());
    core = nullptr;
    vFed1->finalize();
    vFed2->finalize();
    helics::cleanupHelicsLibrary();
}

BOOST_AUTO_TEST_CASE(test_dependency_graph)
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
    BOOST_CHECK_EQUAL(val["cores"].size(), 1u);
    BOOST_CHECK_EQUAL(val["cores"][0]["federates"].size(), 2u);
    BOOST_CHECK_EQUAL(val["cores"][0]["parent"].asInt(), val["id"].asInt());
    auto v2 = val["cores"][0]["federates"][1];
    BOOST_CHECK_EQUAL(v2["parent"].asInt(), val["cores"][0]["id"].asInt());
    core = nullptr;
    vFed1->finalize();
    vFed2->finalize();
    helics::cleanupHelicsLibrary();
}
BOOST_AUTO_TEST_SUITE_END ()
