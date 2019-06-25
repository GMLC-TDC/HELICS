/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "../application_api/testFixtures.hpp"
#include "helics/ValueFederates.hpp"
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

namespace bdata = boost::unit_test::data;
namespace utf = boost::unit_test;

/** tests for some network options*/

BOOST_FIXTURE_TEST_SUITE (network_tests, FederateTestFixture, *utf::label ("ci"))

/** test simple creation and destruction*/
BOOST_AUTO_TEST_CASE (test_external_tcp)
{
    extraBrokerArgs = "--external";
    SetupTest<helics::ValueFederate> ("tcp", 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    BOOST_REQUIRE (vFed1);
    // register the publications
    auto &ipt1 = vFed1->registerInput<double> ("ipt1");

    ipt1.setOption (helics::defs::options::connection_optional);
    ipt1.addTarget ("bob");

    vFed1->enterExecutingMode ();
    vFed1->finalize ();
}

BOOST_AUTO_TEST_CASE (test_external_tcp_ipv4)
{
    extraBrokerArgs = "--ipv4";
    SetupTest<helics::ValueFederate> ("tcp", 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    BOOST_REQUIRE (vFed1);
    // register the publications
    auto &ipt1 = vFed1->registerInput<double> ("ipt1");

    ipt1.setOption (helics::defs::options::connection_optional);
    ipt1.addTarget ("bob");

    vFed1->enterExecutingMode ();
    vFed1->finalize ();
}

/** test simple creation and destruction*/
BOOST_AUTO_TEST_CASE (test_external_tcpss)
{
    extraBrokerArgs = "--external";
    SetupTest<helics::ValueFederate> ("tcpss", 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    BOOST_REQUIRE (vFed1);
    // register the publications
    auto &ipt1 = vFed1->registerInput<double> ("ipt1");

    ipt1.setOption (helics::defs::options::connection_optional);
    ipt1.addTarget ("bob");

    vFed1->enterExecutingMode ();
    vFed1->finalize ();
}

BOOST_AUTO_TEST_CASE (test_external_tcpss_ipv4)
{
    extraBrokerArgs = "--ipv4";
    SetupTest<helics::ValueFederate> ("tcpss", 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    BOOST_REQUIRE (vFed1);
    // register the publications
    auto &ipt1 = vFed1->registerInput<double> ("ipt1");

    ipt1.setOption (helics::defs::options::connection_optional);
    ipt1.addTarget ("bob");

    vFed1->enterExecutingMode ();
    vFed1->finalize ();
}

/** test simple creation and destruction*/
BOOST_AUTO_TEST_CASE (test_external_udp)
{
    extraBrokerArgs = "--external";
    SetupTest<helics::ValueFederate> ("udp", 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    BOOST_REQUIRE (vFed1);
    // register the publications
    auto &ipt1 = vFed1->registerInput<double> ("ipt1");

    ipt1.setOption (helics::defs::options::connection_optional);
    ipt1.addTarget ("bob");

    vFed1->enterExecutingMode ();
    vFed1->finalize ();
}

BOOST_AUTO_TEST_CASE (test_external_udp_ipv4)
{
    extraBrokerArgs = "--ipv4";
    SetupTest<helics::ValueFederate> ("udp", 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    BOOST_REQUIRE (vFed1);
    // register the publications
    auto &ipt1 = vFed1->registerInput<double> ("ipt1");

    ipt1.setOption (helics::defs::options::connection_optional);
    ipt1.addTarget ("bob");

    vFed1->enterExecutingMode ();
    vFed1->finalize ();
}
BOOST_AUTO_TEST_SUITE_END ()
