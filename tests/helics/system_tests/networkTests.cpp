/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "../application_api/testFixtures.hpp"
#include "helics/ValueFederates.hpp"
#include "helics/helics-config.h"

#include "gtest/gtest.h"

/** tests for some network options*/

struct network_tests: public FederateTestFixture, public ::testing::Test {
};

#ifdef ENABLE_TCP_CORE
/** test simple creation and destruction*/
TEST_F(network_tests, test_external_tcp)
{
    extraBrokerArgs = "--external";
    SetupTest<helics::ValueFederate>("tcp", 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    ASSERT_TRUE(vFed1);
    // register the publications
    auto& ipt1 = vFed1->registerInput<double>("ipt1");

    ipt1.setOption(helics::defs::options::connection_optional);
    ipt1.addTarget("bob");

    vFed1->enterExecutingMode();
    vFed1->finalize();
}

TEST_F(network_tests, test_external_tcp_ipv4)
{
    extraBrokerArgs = "--ipv4";
    SetupTest<helics::ValueFederate>("tcp", 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    ASSERT_TRUE(vFed1);
    // register the publications
    auto& ipt1 = vFed1->registerInput<double>("ipt1");

    ipt1.setOption(helics::defs::options::connection_optional);
    ipt1.addTarget("bob");

    vFed1->enterExecutingMode();
    vFed1->finalize();
}

/** test simple creation and destruction*/
TEST_F(network_tests, test_external_tcpss)
{
    extraBrokerArgs = "--external";
    SetupTest<helics::ValueFederate>("tcpss", 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    ASSERT_TRUE(vFed1);
    // register the publications
    auto& ipt1 = vFed1->registerInput<double>("ipt1");

    ipt1.setOption(helics::defs::options::connection_optional);
    ipt1.addTarget("bob");

    vFed1->enterExecutingMode();
    vFed1->finalize();
}

TEST_F(network_tests, test_external_tcpss_ipv4)
{
    extraBrokerArgs = "--ipv4";
    SetupTest<helics::ValueFederate>("tcpss", 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    ASSERT_TRUE(vFed1);
    // register the publications
    auto& ipt1 = vFed1->registerInput<double>("ipt1");

    ipt1.setOption(helics::defs::options::connection_optional);
    ipt1.addTarget("bob");

    vFed1->enterExecutingMode();
    vFed1->finalize();
}

#endif

#ifdef ENABLE_UDP_CORE
/** test simple creation and destruction*/
TEST_F(network_tests, test_external_udp)
{
    extraBrokerArgs = "--external";
    SetupTest<helics::ValueFederate>("udp", 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    ASSERT_TRUE(vFed1);
    // register the publications
    auto& ipt1 = vFed1->registerInput<double>("ipt1");

    ipt1.setOption(helics::defs::options::connection_optional);
    ipt1.addTarget("bob");

    vFed1->enterExecutingMode();
    vFed1->finalize();
}

TEST_F(network_tests, test_external_udp_ipv4)
{
    extraBrokerArgs = "--ipv4";
    SetupTest<helics::ValueFederate>("udp", 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    ASSERT_TRUE(vFed1);
    // register the publications
    auto& ipt1 = vFed1->registerInput<double>("ipt1");

    ipt1.setOption(helics::defs::options::connection_optional);
    ipt1.addTarget("bob");

    vFed1->enterExecutingMode();
    vFed1->finalize();
}

#endif
