/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "testFixtures.h"

#include "helics/application_api/queryFunctions.h"
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

BOOST_FIXTURE_TEST_SUITE (query_tests_value, ValueFederateTestFixture)

namespace bdata = boost::unit_test::data;
#if ENABLE_TEST_TIMEOUTS > 0
namespace utf = boost::unit_test;
#endif
#ifdef QUICK_TESTS_ONLY
const std::string core_types[] = {"test_2", "ipc_2", "zmq", "udp"};
#else
const std::string core_types[] = {"test", "test_2", "ipc", "ipc_2", "zmq", "zmq_2", "udp", "udp_2"};
#endif
/** test simple creation and destruction*/
#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (5))
#endif
BOOST_DATA_TEST_CASE (test_publication_queries, bdata::make (core_types), core_type)
{
    Setup2FederateTest (core_type);
    // register the publications
    vFed1->registerGlobalPublication<double> ("pub1");

    vFed2->registerRequiredSubscription<double> ("pub1");

    vFed1->registerPublication<double> ("pub2");

    vFed2->registerPublication<double> ("pub3");
    vFed1->setTimeDelta (1.0);
    vFed2->setTimeDelta (1.0);

    vFed1->enterInitializationStateAsync ();
    vFed2->enterInitializationState ();
    vFed1->enterInitializationStateFinalize ();

    auto core = vFed1->getCorePointer ();
    auto res = core->query ("test1", "publications");
    BOOST_CHECK_EQUAL (res, "[pub1;test1/pub2]");
    auto rvec = vectorizeQueryResult (res);

    BOOST_REQUIRE_EQUAL (rvec.size (), 2);
    BOOST_CHECK_EQUAL (rvec[0], "pub1");
    BOOST_CHECK_EQUAL (rvec[1], "test1/pub2");
    BOOST_CHECK_EQUAL (vFed2->query ("test1", "publications"), "[pub1;test1/pub2]");
    BOOST_CHECK_EQUAL (vFed1->query ("test2", "isinit"), "true");

    BOOST_CHECK_EQUAL (vFed1->query ("test2", "publications"), "[test2/pub3]");
    vFed1->finalize ();
    vFed2->finalize ();
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (5))
#endif
BOOST_DATA_TEST_CASE (test_broker_queries, bdata::make (core_types), core_type)
{
    Setup2FederateTest (core_type);
    auto core = vFed1->getCorePointer ();
    auto res = core->query ("root", "federates");
    std::string str ("[");
    str.append (vFed1->getName ());
    str.push_back (';');
    str.append (vFed2->getName ());
    str.push_back (']');
    BOOST_CHECK_EQUAL (res, "[test1;test2]");
    vFed1->finalize ();
    vFed2->finalize ();
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (5))
#endif
BOOST_DATA_TEST_CASE (test_publication_fed_queries, bdata::make (core_types), core_type)
{
    Setup2FederateTest (core_type);
    // register the publications
    vFed1->registerPublication<double> ("pub1");

    vFed2->registerPublication<double> ("pub2");

    vFed2->registerPublication<double> ("pub3");

    vFed1->enterInitializationStateAsync ();
    vFed2->enterInitializationState ();
    vFed1->enterInitializationStateFinalize ();

    auto res = vFed1->query ("federation", "publications");

    auto rvec = vectorizeAndSortQueryResult (res);

    BOOST_REQUIRE_EQUAL (rvec.size (), 3);
    BOOST_CHECK_EQUAL (rvec[0], "test1/pub1");
    BOOST_CHECK_EQUAL (rvec[1], "test2/pub2");
    BOOST_CHECK_EQUAL (rvec[2], "test2/pub3");
    vFed1->finalize ();
    vFed2->finalize ();
}
BOOST_AUTO_TEST_SUITE_END ()
