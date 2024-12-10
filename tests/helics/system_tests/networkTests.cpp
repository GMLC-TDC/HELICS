/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../application_api/testFixtures.hpp"
#include "../apps/exeTestHelper.h"
#include "helics/ValueFederates.hpp"
#include "helics/helics-config.h"

#include "gtest/gtest.h"
#include <iostream>
#include <string>
#include <thread>

/** tests for some network options*/

struct network_tests: public FederateTestFixture, public ::testing::Test {};

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
    helics::FederateInfo fedInfo("--coretype=ZMQ --broker=tcp://127.0.0.1:33200");
    helics::ValueFederate fed1("fed1", fedInfo);

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
    helics::FederateInfo fedInfo("--coretype=ZMQ --broker=tcp://127.0.0.1:20200");
    helics::ValueFederate fed1("fed1", fedInfo);

    fed1.enterExecutingMode();
    fed1.finalize();
    EXPECT_TRUE(broker->waitForDisconnect(std::chrono::milliseconds(400)));
    if (broker->isConnected()) {
        broker->disconnect();
    }
}

TEST_F(network_tests, otherport_fail)
{
    const std::string brokerArgs = "--local_interface=tcp://127.0.0.1:33100";
    auto broker = helics::BrokerFactory::create(helics::CoreType::ZMQ, brokerArgs);

    EXPECT_TRUE(broker->isConnected());
    helics::FederateInfo fedInfo("--coretype=ZMQ --broker=tcp://127.0.0.1:33198 --timeout=100ms");
    EXPECT_THROW(helics::ValueFederate fed1("fed1", fedInfo), helics::RegistrationFailure);

    if (broker->isConnected()) {
        broker->disconnect();
    }
}

TEST_F(network_tests, otherport_env)
{
    setEnvironmentVariable("HELICS_CONNECTION_PORT", "33102");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    const std::string brokerArgs = "-f 2";
    auto broker = helics::BrokerFactory::create(helics::CoreType::ZMQ, brokerArgs);

    EXPECT_TRUE(broker->isConnected());
    helics::FederateInfo fedInfo("--core_type=ZMQ --corename=cop1");
    helics::ValueFederate fed1("fed1", fedInfo);

    helics::FederateInfo fi2("--core_type=ZMQ --broker=tcp://127.0.0.1:33102 --corename=cop2");
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
    auto broker = helics::BrokerFactory::create(helics::CoreType::ZMQ, brokerArgs);
    clearEnvironmentVariable("HELICS_LOCAL_PORT");
    EXPECT_TRUE(broker->isConnected());

    setEnvironmentVariable("HELICS_BROKER_PORT", "33104");
    helics::FederateInfo fedInfo("--core_type=ZMQ --corename=c1b");
    helics::ValueFederate fed1("fed1", fedInfo);
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

#ifdef HELICS_ENABLE_TCP_CORE
TEST_F(network_tests, test_core_type_env)
{
    setEnvironmentVariable("HELICS_CORE_TYPE", "TCP");
    const std::string brokerArgs = "-f 2";
    auto broker = helics::BrokerFactory::create(helics::CoreType::TCP, brokerArgs);
    EXPECT_TRUE(broker->isConnected());

    helics::FederateInfo fedInfo("--corename=c1bt");
    helics::ValueFederate fed1("fed1", fedInfo);

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

#ifdef HELICS_ENABLE_ENCRYPTION
#    ifdef HELICS_ENABLE_TCP_CORE
TEST_F(network_tests, test_encrypted_tcp)
{
    const std::string brokerArgs =
        "-f 2 --core_type=tcp --encrypted --encryption_config=" + std::string(TEST_BIN_DIR) +
        "encryption_config/openssl.json";
    exeTestRunner brokerExe(HELICS_BROKER_LOCATION, "helics_broker");
    if (!brokerExe.isActive()) {
        std::cout << " unable to locate helics_broker in " << HELICS_BROKER_LOCATION << std::endl;
    }
    ASSERT_TRUE(brokerExe.isActive());
    auto res = brokerExe.runAsync(brokerArgs);

    // give the helics_broker some time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(40));

    const std::string fedArgs =
        "--core_type=tcp --encrypted --encryption_config=" + std::string(TEST_BIN_DIR) +
        "encryption_config/openssl.json";
    helics::FederateInfo fi_enc(fedArgs);
    helics::ValueFederate fed1("fed1", fi_enc);

    helics::ValueFederate fed2("fed2", fi_enc);

    fed2.enterExecutingModeAsync();
    fed1.enterExecutingMode();
    fed2.enterExecutingModeComplete();

    fed1.finalize();
    fed2.finalize();
    res.get();
}

TEST_F(network_tests, test_encrypted_tcpss)
{
    const std::string brokerArgs =
        "-f 2 --core_type=tcp_ss --encrypted --encryption_config=" + std::string(TEST_BIN_DIR) +
        "encryption_config/openssl.json";
    exeTestRunner brokerExe(HELICS_BROKER_LOCATION, "helics_broker");
    if (!brokerExe.isActive()) {
        std::cout << " unable to locate helics_broker in " << HELICS_BROKER_LOCATION << std::endl;
    }
    ASSERT_TRUE(brokerExe.isActive());
    auto res = brokerExe.runAsync(brokerArgs);

    // give the helics_broker some time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(40));

    const std::string fedArgs =
        "--core_type=tcp_ss --encrypted --encryption_config=" + std::string(TEST_BIN_DIR) +
        "encryption_config/openssl.json";
    helics::FederateInfo fi_enc(fedArgs);
    helics::ValueFederate fed1("fed1", fi_enc);

    helics::ValueFederate fed2("fed2", fi_enc);

    fed2.enterExecutingModeAsync();
    fed1.enterExecutingMode();
    fed2.enterExecutingModeComplete();

    fed1.finalize();
    fed2.finalize();
    res.get();
}

TEST_F(network_tests, test_encrypted_bridge)
{
    const std::string brokerArgs = "-f 2 --core_type=multi --config=" + std::string(TEST_BIN_DIR) +
        "encryption_config/multiBroker_encrypted_bridge.json --encrypted --encryption_config=" +
        std::string(TEST_BIN_DIR) + "encryption_config/openssl.json";
    exeTestRunner brokerExe(HELICS_BROKER_LOCATION, "helics_broker");
    if (!brokerExe.isActive()) {
        std::cout << " unable to locate helics_broker in " << HELICS_BROKER_LOCATION << std::endl;
    }
    ASSERT_TRUE(brokerExe.isActive());
    auto res = brokerExe.runAsync(brokerArgs);

    // give the helics_broker some time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(40));

    const std::string fedArgs_enc =
        "--core_type=tcp_ss --brokerport=30000 --encrypted --encryption_config=" +
        std::string(TEST_BIN_DIR) + "encryption_config/openssl.json";
    helics::FederateInfo fi_enc(fedArgs_enc);
    helics::ValueFederate fed_enc("fed_encrypted", fi_enc);

    const std::string fedArgs_unenc = "--core_type=tcp --brokerport=40000";
    helics::FederateInfo fi_unenc(fedArgs_unenc);
    helics::ValueFederate fed_unenc("fed_unencrypted", fi_unenc);

    fed_unenc.enterExecutingModeAsync();
    fed_enc.enterExecutingMode();
    fed_unenc.enterExecutingModeComplete();

    fed_enc.finalize();
    fed_unenc.finalize();
    res.get();
}
#    endif
#endif
