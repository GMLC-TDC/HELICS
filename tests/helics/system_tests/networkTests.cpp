/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../application_api/testFixtures.hpp"
#include "helics/ValueFederates.hpp"
#include "helics/helics-config.h"

#include "gtest/gtest.h"

/** tests for some network options*/

struct network_tests: public FederateTestFixture, public ::testing::Test {
};

#ifdef HELICS_ENABLE_TCP_CORE
/** test simple creation and destruction*/
TEST_F(network_tests, test_external_tcp)
{
    extraBrokerArgs = "--external";
    SetupTest<helics::ValueFederate>("tcp", 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    ASSERT_TRUE(vFed1);
    // register the publications
    auto& ipt1 = vFed1->registerInput<double>("ipt1");

    ipt1.setOption(helics::defs::Options::CONNECTION_OPTIONAL);
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

    ipt1.setOption(helics::defs::Options::CONNECTION_OPTIONAL);
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

    ipt1.setOption(helics::defs::Options::CONNECTION_OPTIONAL);
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

    ipt1.setOption(helics::defs::Options::CONNECTION_OPTIONAL);
    ipt1.addTarget("bob");

    vFed1->enterExecutingMode();
    vFed1->finalize();
}

#endif

#ifdef HELICS_ENABLE_UDP_CORE
/** test simple creation and destruction*/
TEST_F(network_tests, test_external_udp)
{
    extraBrokerArgs = "--external";
    SetupTest<helics::ValueFederate>("udp", 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    ASSERT_TRUE(vFed1);
    // register the publications
    auto& ipt1 = vFed1->registerInput<double>("ipt1");

    ipt1.setOption(helics::defs::Options::CONNECTION_OPTIONAL);
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

    ipt1.setOption(helics::defs::Options::CONNECTION_OPTIONAL);
    ipt1.addTarget("bob");

    vFed1->enterExecutingMode();
    vFed1->finalize();
}
#endif

#ifdef HELICS_ENABLE_ZMQ_CORE
TEST_F(network_tests, test_otherport)
{
    const std::string brokerArgs = "--local_interface=tcp://127.0.0.1:33200";
    auto broker = helics::BrokerFactory::create(helics::CoreType::ZMQ, brokerArgs);

    EXPECT_TRUE(broker->isConnected());
    helics::FederateInfo fi("--coretype=ZMQ --broker=tcp://127.0.0.1:33200");
    helics::ValueFederate fed1("fed1", fi);

    fed1.enterExecutingMode();
    fed1.finalize();
    EXPECT_TRUE(broker->waitForDisconnect(std::chrono::milliseconds(400)));
    if (broker->isConnected()) {
        broker->disconnect();
    }
}

TEST_F(network_tests, test_otherport2)
{
    const std::string brokerArgs = "--local_interface=tcp://127.0.0.1:20200";
    auto broker = helics::BrokerFactory::create(helics::CoreType::ZMQ, brokerArgs);

    EXPECT_TRUE(broker->isConnected());
    helics::FederateInfo fi("--coretype=ZMQ --broker=tcp://127.0.0.1:20200");
    helics::ValueFederate fed1("fed1", fi);

    fed1.enterExecutingMode();
    fed1.finalize();
    EXPECT_TRUE(broker->waitForDisconnect(std::chrono::milliseconds(400)));
    if (broker->isConnected()) {
        broker->disconnect();
    }
}

TEST_F(network_tests, test_otherport_fail)
{
    const std::string brokerArgs = "--local_interface=tcp://127.0.0.1:33100";
    auto broker = helics::BrokerFactory::create(helics::CoreType::ZMQ, brokerArgs);

    EXPECT_TRUE(broker->isConnected());
    helics::FederateInfo fi("--coretype=ZMQ --broker=tcp://127.0.0.1:33198 --timeout=100ms");
    EXPECT_THROW(helics::ValueFederate fed1("fed1", fi), helics::RegistrationFailure);

    if (broker->isConnected()) {
        broker->disconnect();
    }
}

#endif
