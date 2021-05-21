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

#ifdef ENABLE_ZMQ_CORE
TEST_F(network_tests, test_otherport)
{
    const std::string brokerArgs = "--interface=tcp://127.0.0.1:33200";
    auto broker = helics::BrokerFactory::create(helics::core_type::ZMQ, brokerArgs);

    EXPECT_TRUE(broker->isConnected());
    helics::FederateInfo fi("--core_type=ZMQ --broker=tcp://127.0.0.1:33200");
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
    const std::string brokerArgs = "--interface=tcp://127.0.0.1:20200";
    auto broker = helics::BrokerFactory::create(helics::core_type::ZMQ, brokerArgs);

    EXPECT_TRUE(broker->isConnected());
    helics::FederateInfo fi("--core_type=ZMQ --broker=tcp://127.0.0.1:20200");
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
    const std::string brokerArgs = "--interface=tcp://127.0.0.1:33100";
    auto broker = helics::BrokerFactory::create(helics::core_type::ZMQ, brokerArgs);

    EXPECT_TRUE(broker->isConnected());
    helics::FederateInfo fi("--core_type=ZMQ --broker=tcp://127.0.0.1:33198 --timeout=100ms");
    EXPECT_THROW(helics::ValueFederate fed1("fed1", fi), helics::RegistrationFailure);

    if (broker->isConnected()) {
        broker->disconnect();
    }
}

TEST_F(network_tests, test_otherport_env)
{
    setEnvironmentVariable("HELICS_CONNECTION_PORT", "33102");
    const std::string brokerArgs = "-f 2";
    auto broker = helics::BrokerFactory::create(helics::core_type::ZMQ, brokerArgs);

    EXPECT_TRUE(broker->isConnected());
    helics::FederateInfo fi("--core_type=ZMQ --corename=c1");
    helics::ValueFederate fed1("fed1", fi);

    helics::FederateInfo fi2("--core_type=ZMQ --broker=tcp://127.0.0.1:33102 --corename=c2");
    helics::ValueFederate fed2("fed2", fi2);

    fed2.enterExecutingModeAsync();
    fed1.enterExecutingMode();
    fed2.enterExecutingModeComplete();
    const auto& address = broker->getAddress();
    EXPECT_NE(address.find("33102"), std::string::npos);
    fed1.finalize();
    fed2.finalize();

    EXPECT_TRUE(broker->waitForDisconnect(std::chrono::milliseconds(400)));
    if (broker->isConnected()) {
        broker->disconnect();
    }

    clearEnvironmentVariable("HELICS_CONNECTION_PORT");
}

TEST_F(network_tests, test_otherport_broker_local_env)
{
    setEnvironmentVariable("HELICS_LOCAL_PORT", "33104");
    const std::string brokerArgs = "-f 2";
    auto broker = helics::BrokerFactory::create(helics::core_type::ZMQ, brokerArgs);
    clearEnvironmentVariable("HELICS_LOCAL_PORT");
    EXPECT_TRUE(broker->isConnected());

    setEnvironmentVariable("HELICS_BROKER_PORT", "33104");
    helics::FederateInfo fi("--core_type=ZMQ --corename=c1b");
    helics::ValueFederate fed1("fed1", fi);
    clearEnvironmentVariable("HELICS_BROKER_PORT");

    helics::FederateInfo fi2("--core_type=ZMQ --broker=tcp://127.0.0.1:33104 --corename=c2b");
    helics::ValueFederate fed2("fed2", fi2);

    fed2.enterExecutingModeAsync();
    fed1.enterExecutingMode();
    fed2.enterExecutingModeComplete();
    const auto& address = broker->getAddress();
    EXPECT_NE(address.find("33104"), std::string::npos);
    fed1.finalize();
    fed2.finalize();
    EXPECT_TRUE(broker->waitForDisconnect(std::chrono::milliseconds(400)));
    if (broker->isConnected()) {
        broker->disconnect();
    }
}

#endif

#ifdef ENABLE_TCP_CORE
TEST_F(network_tests, test_core_type_env)
{
    setEnvironmentVariable("HELICS_CORE_TYPE", "TCP");
    const std::string brokerArgs = "-f 2";
    auto broker = helics::BrokerFactory::create(helics::core_type::TCP, brokerArgs);
    EXPECT_TRUE(broker->isConnected());

    helics::FederateInfo fi("--corename=c1bt");
    helics::ValueFederate fed1("fed1", fi);

    helics::FederateInfo fi2("--core_type=TCP --corename=c2bt");
    helics::ValueFederate fed2("fed2", fi2);

    clearEnvironmentVariable("HELICS_CORE_TYPE");
    fed2.enterExecutingModeAsync();
    fed1.enterExecutingMode();
    fed2.enterExecutingModeComplete();

    fed1.finalize();
    fed2.finalize();
    EXPECT_TRUE(broker->waitForDisconnect(std::chrono::milliseconds(400)));
    if (broker->isConnected()) {
        broker->disconnect();
    }
}
#endif
