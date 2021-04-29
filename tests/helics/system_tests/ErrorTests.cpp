/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../application_api/testFixtures.hpp"
#include "helics/application_api.hpp"
#include "helics/core/core-exceptions.hpp"

#include "gtest/gtest.h"
#include <complex>
#include <future>

#define CORE_TYPE_TO_TEST helics::core_type::TEST

struct error_tests: public FederateTestFixture, public ::testing::Test {
};

TEST_F(error_tests, duplicate_federate_names)
{
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed = std::make_shared<helics::Federate>("test1", fi);

    EXPECT_THROW(auto fed2 = std::make_shared<helics::Federate>("test1", fi),
                 helics::RegistrationFailure);
    Fed->finalize();
}

TEST_F(error_tests, duplicate_federate_names2)
{
    auto broker = AddBroker("test", 2);
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0, "fed");
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0, "fed");

    auto fed1 = GetFederateAs<helics::ValueFederate>(0);
    auto fed2 = GetFederateAs<helics::ValueFederate>(1);
    helics::FederateInfo fi;
    // get the core pointer from fed2 and using the name of fed1 should be an error
    EXPECT_THROW(helics::ValueFederate fed3(fed1->getName(), fed2->getCorePointer(), fi),
                 helics::RegistrationFailure);
    broker->disconnect();
}

TEST_F(error_tests, already_init_broker)
{
    auto broker = AddBroker("test", 1);
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0, "fed");
    auto fed1 = GetFederateAs<helics::ValueFederate>(0);

    fed1->enterInitializingMode();
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreInitString = std::string("--timeout=1s --broker=") + broker->getIdentifier();
    EXPECT_THROW(helics::ValueFederate fed3("fed222", fi), helics::RegistrationFailure);
    broker->disconnect();
}

TEST_F(error_tests, mismatch_broker_key)
{
    auto broker = AddBroker("test", 1);

    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreInitString = std::string("--timeout=1s --key=tkey --broker=") + broker->getIdentifier();
    EXPECT_THROW(helics::ValueFederate fed3("fed222", fi), helics::RegistrationFailure);
    broker->disconnect();
    auto core = helics::CoreFactory::findJoinableCoreOfType(helics::core_type::TEST);
    if (core) {
        core->disconnect();
    }
    helics::cleanupHelicsLibrary();
}

TEST_F(error_tests, mismatch_broker_key2)
{
    auto broker = AddBroker("test", "-f 1 --key=tkey2");

    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreInitString = std::string("--timeout=1s --key=tkey --broker=") + broker->getIdentifier();
    EXPECT_THROW(helics::ValueFederate fed3("fed222", fi), helics::RegistrationFailure);
    broker->disconnect();
    auto core = helics::CoreFactory::findJoinableCoreOfType(helics::core_type::TEST);
    if (core) {
        core->disconnect();
    }
    helics::cleanupHelicsLibrary();
}

TEST_F(error_tests, mismatch_broker_key_success)
{
    auto broker = AddBroker("test", "--key=tkey");

    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreInitString = std::string("--timeout=1s --key=tkey --broker=") + broker->getIdentifier();
    helics::ValueFederate fed3("fed2b", fi);
    fed3.enterExecutingMode();
    fed3.finalize();
    broker->waitForDisconnect();
    helics::cleanupHelicsLibrary();
}

TEST_F(error_tests, mismatch_broker_key_success_universal_key)
{
    auto broker = AddBroker("test", "--key=**");

    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreInitString = std::string("--timeout=1s --key=tkey --broker=") + broker->getIdentifier();
    helics::ValueFederate fed3("fed2b", fi);
    fed3.enterExecutingMode();
    fed3.finalize();
    broker->waitForDisconnect();
    helics::cleanupHelicsLibrary();
}

TEST_F(error_tests, already_init_core)
{
    auto broker = AddBroker("test", 1);
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0, "fed");

    auto fed1 = GetFederateAs<helics::ValueFederate>(0);
    fed1->enterExecutingMode();
    // get the core pointer from fed2 and using the name of fed1 should be an error
    EXPECT_THROW(helics::ValueFederate fed2("fed2", fed1->getCorePointer(), helics::FederateInfo()),
                 helics::RegistrationFailure);
    broker->disconnect();
}

TEST_F(error_tests, duplicate_publication_names)
{
    auto broker = AddBroker("test", 1);
    AddFederates<helics::ValueFederate>("test", 2, broker, 1.0, "fed");
    auto fed1 = GetFederateAs<helics::ValueFederate>(0);
    auto fed2 = GetFederateAs<helics::ValueFederate>(1);

    fed1->registerGlobalPublication("testkey", "");
    EXPECT_THROW(fed2->registerGlobalPublication("testkey", ""), helics::RegistrationFailure);
    fed1->finalize();
    fed2->finalize();
    broker->disconnect();
}

TEST_F(error_tests, duplicate_publication_names2)
{
    auto broker = AddBroker("test", 2);
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0, "fed");
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0, "fed");

    auto fed1 = GetFederateAs<helics::ValueFederate>(0);
    auto fed2 = GetFederateAs<helics::ValueFederate>(1);

    fed1->registerGlobalPublication("testkey", "");
    fed1->enterInitializingModeAsync();

    fed2->registerGlobalPublication("testkey", "");

    bool gotException = false;
    try {
        fed2->enterInitializingMode();
    }
    catch (const helics::RegistrationFailure&) {
        gotException = true;
        EXPECT_TRUE(fed2->getCurrentMode() == helics::Federate::modes::error);
        // this should do nothing
        EXPECT_THROW(fed2->enterExecutingMode(), helics::InvalidFunctionCall);
        EXPECT_TRUE(fed2->getCurrentMode() == helics::Federate::modes::error);
    }

    try {
        fed1->enterInitializingModeComplete();
    }
    catch (const helics::RegistrationFailure&) {
        gotException = true;
        EXPECT_TRUE(fed1->getCurrentMode() == helics::Federate::modes::error);
        // this should do nothing
        EXPECT_THROW(fed1->enterExecutingMode(), helics::InvalidFunctionCall);
        EXPECT_TRUE(fed1->getCurrentMode() == helics::Federate::modes::error);
    }
    EXPECT_TRUE(gotException);

    fed1->finalize();
    fed2->finalize();
    broker->disconnect();
}

TEST_F(error_tests, duplicate_publication_names_auto_terminate)
{
    auto broker = AddBroker("test", "-f 2 --error_timeout=0");
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0, "fed");
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0, "fed");

    auto fed1 = GetFederateAs<helics::ValueFederate>(0);
    auto fed2 = GetFederateAs<helics::ValueFederate>(1);

    fed1->setFlagOption(helics_flag_terminate_on_error);
    fed2->setFlagOption(helics_flag_terminate_on_error);
    fed1->registerGlobalPublication("testkey", "");
    fed1->enterInitializingModeAsync();

    fed2->registerGlobalPublication("testkey", "");

    bool gotException = false;
    try {
        fed2->enterInitializingMode();
    }
    catch (const helics::RegistrationFailure&) {
        gotException = true;
        EXPECT_TRUE(fed2->getCurrentMode() == helics::Federate::modes::error);
    }

    try {
        fed1->enterInitializingModeComplete();
    }
    catch (const helics::RegistrationFailure&) {
        gotException = true;
        EXPECT_TRUE(fed1->getCurrentMode() == helics::Federate::modes::error);
    }
    EXPECT_TRUE(gotException);

    broker->waitForDisconnect();
}

TEST_F(error_tests, duplicate_publication_names_auto_terminate_core)
{
    auto broker = AddBroker("test", "-f 2 --error_timeout=0");
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0, "fed");
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0, "fed");

    auto fed1 = GetFederateAs<helics::ValueFederate>(0);
    auto fed2 = GetFederateAs<helics::ValueFederate>(1);

    fed1->getCorePointer()->setFlagOption(helics::local_core_id,
                                          helics_flag_terminate_on_error,
                                          true);
    fed2->getCorePointer()->setFlagOption(helics::local_core_id,
                                          helics_flag_terminate_on_error,
                                          true);

    fed1->registerGlobalPublication("testkey", "");
    fed1->enterInitializingModeAsync();

    fed2->registerGlobalPublication("testkey", "");

    bool gotException = false;
    try {
        fed2->enterInitializingMode();
    }
    catch (const helics::RegistrationFailure&) {
        gotException = true;
        EXPECT_TRUE(fed2->getCurrentMode() == helics::Federate::modes::error);
    }

    try {
        fed1->enterInitializingModeComplete();
    }
    catch (const helics::RegistrationFailure&) {
        gotException = true;
        EXPECT_TRUE(fed1->getCurrentMode() == helics::Federate::modes::error);
    }
    EXPECT_TRUE(gotException);

    broker->waitForDisconnect();
}

TEST_F(error_tests, duplicate_publication_names_auto_terminate_broker)
{
    auto broker = AddBroker("test", "-f 2 --error_timeout=0 --terminate_on_error");
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0, "fed");
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0, "fed");

    auto fed1 = GetFederateAs<helics::ValueFederate>(0);
    auto fed2 = GetFederateAs<helics::ValueFederate>(1);

    fed1->registerGlobalPublication("testkey", "");
    fed1->enterInitializingModeAsync();

    fed2->registerGlobalPublication("testkey", "");

    bool gotException = false;
    try {
        fed2->enterInitializingMode();
    }
    catch (const helics::HelicsException&) {
        gotException = true;
        EXPECT_TRUE(fed2->getCurrentMode() == helics::Federate::modes::error);
    }

    try {
        fed1->enterInitializingModeComplete();
    }
    catch (const helics::HelicsException&) {
        gotException = true;
        EXPECT_TRUE(fed1->getCurrentMode() == helics::Federate::modes::error);
    }
    EXPECT_TRUE(gotException);

    broker->waitForDisconnect();
}

TEST_F(error_tests, duplicate_publication_names3)
{
    auto broker = AddBroker("test", 1);
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0, "fed");

    auto fed1 = GetFederateAs<helics::ValueFederate>(0);

    fed1->registerPublication("testkey", "");

    EXPECT_THROW(fed1->registerPublication("testkey", ""), helics::RegistrationFailure);
    fed1->finalize();
    broker->disconnect();
}

TEST_F(error_tests, duplicate_publication_names4)
{
    auto broker = AddBroker("test", 1);
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0, "fed");

    auto fed1 = GetFederateAs<helics::ValueFederate>(0);

    // all 3 of these should publish to the same thing
    auto& pubid = fed1->registerPublication("testkey", "");

    helics::Publication pub(fed1, "testkey", helics::data_type::helics_double);
    // copy constructor
    helics::Publication pub2(pubid);

    auto& sub = fed1->registerSubscription(fed1->getInterfaceName(pubid));
    fed1->enterExecutingMode();
    fed1->publish(pubid, 45.7);
    fed1->requestTime(1.0);
    auto res = sub.getValue<double>();
    EXPECT_EQ(res, 45.7);
    pub.publish(99.2);

    fed1->requestTime(2.0);
    res = sub.getValue<double>();
    EXPECT_EQ(res, 99.2);
    pub2.publish(103.8);

    fed1->requestTime(3.0);
    res = sub.getValue<double>();
    EXPECT_EQ(res, 103.8);
    fed1->finalize();
    broker->disconnect();
}

TEST_F(error_tests, duplicate_endpoint_names)
{
    auto broker = AddBroker("test", 1);
    AddFederates<helics::MessageFederate>("test", 2, broker, 1.0, "fed");
    auto fed1 = GetFederateAs<helics::MessageFederate>(0);
    auto fed2 = GetFederateAs<helics::MessageFederate>(1);

    fed1->registerGlobalEndpoint("testEpt");
    EXPECT_THROW(fed2->registerGlobalEndpoint("testEpt"), helics::RegistrationFailure);
    fed1->finalize();
    fed2->finalize();
    broker->disconnect();
}

TEST_F(error_tests, duplicate_endpoint_names2)
{
    auto broker = AddBroker("test", 2);
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "fed");
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "fed");

    auto fed1 = GetFederateAs<helics::MessageFederate>(0);
    auto fed2 = GetFederateAs<helics::MessageFederate>(1);

    fed1->registerGlobalEndpoint("testEpt");
    fed2->registerGlobalEndpoint("testEpt");

    fed1->enterInitializingModeAsync();

    bool gotException = false;
    try {
        fed2->enterInitializingMode();
    }
    catch (const helics::RegistrationFailure&) {
        gotException = true;
    }

    try {
        fed1->enterInitializingModeComplete();
    }
    catch (const helics::RegistrationFailure&) {
        gotException = true;
    }
    EXPECT_TRUE(gotException);
    fed1->finalize();
    fed2->finalize();
    broker->disconnect();
}

TEST_F(error_tests, missing_required_pub)
{
    auto broker = AddBroker("test", 2);

    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0, "fed");
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0, "fed");

    auto fed1 = GetFederateAs<helics::ValueFederate>(0);
    auto fed2 = GetFederateAs<helics::ValueFederate>(1);

    fed1->registerGlobalPublication("t1", "");
    auto& i2 = fed2->registerSubscription("abcd", "");
    i2.setOption(helics::defs::options::connection_required, 1);

    fed1->enterInitializingModeAsync();
    EXPECT_THROW(fed2->enterInitializingMode(), helics::ConnectionFailure);
    fed1->finalize();
    fed2->finalize();
    broker->disconnect();
}

TEST_F(error_tests, missing_required_pub_with_default)
{
    auto broker = AddBroker("test", 2);

    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0, "fed");
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0, "fed");

    auto fed1 = GetFederateAs<helics::ValueFederate>(0);
    auto fed2 = GetFederateAs<helics::ValueFederate>(1);

    fed1->registerGlobalPublication("t1", "");
    fed2->setFlagOption(helics::defs::flags::connections_required, true);
    fed2->registerSubscription("abcd", "");

    fed1->enterInitializingModeAsync();
    EXPECT_THROW(fed2->enterInitializingMode(), helics::ConnectionFailure);
    // this is definitely not how you would normally do this,
    // we are calling finalize while an async call is active, this should result in finalize
    // throwing since it was a global connection failure,  depending on how things go this will be a
    // registration failure or a connection failure
    EXPECT_THROW(fed1->finalize(), helics::HelicsException);
    fed2->finalize();
    broker->disconnect();
}

TEST_F(error_tests, mismatched_units)
{
    auto broker = AddBroker("test", 3);

    AddFederates<helics::ValueFederate>("test", 3, broker, 1.0, "fed");

    auto fed1 = GetFederateAs<helics::ValueFederate>(0);
    auto fed2 = GetFederateAs<helics::ValueFederate>(1);
    auto fed3 = GetFederateAs<helics::ValueFederate>(2);

    fed1->registerGlobalPublication("t1", "double", "V");
    fed2->registerSubscription("t1", "m");
    auto& sub = fed3->registerSubscription("t1", "m");
    sub.setOption(helics::defs::options::ignore_unit_mismatch);
    fed1->enterExecutingModeAsync();
    fed2->enterExecutingModeAsync();
    EXPECT_NO_THROW(fed3->enterExecutingMode());
    fed1->enterExecutingModeComplete();
    EXPECT_THROW(fed2->enterExecutingModeComplete(), helics::ConnectionFailure);

    fed1->finalize();
    fed2->finalize();
    fed3->finalize();
    broker->disconnect();
}

TEST_F(error_tests, mismatched_units_terminate_on_error)
{
    auto broker = AddBroker("test", "-f 3 --error_timeout=0");

    AddFederates<helics::ValueFederate>("test", 3, broker, 1.0, "fed");

    auto fed1 = GetFederateAs<helics::ValueFederate>(0);
    auto fed2 = GetFederateAs<helics::ValueFederate>(1);
    auto fed3 = GetFederateAs<helics::ValueFederate>(2);

    fed1->registerGlobalPublication("t1", "double", "V");
    fed2->setFlagOption(helics_flag_terminate_on_error);
    fed2->registerSubscription("t1", "m");
    auto& sub = fed3->registerSubscription("t1", "m");
    sub.setOption(helics::defs::options::ignore_unit_mismatch);
    fed1->enterExecutingModeAsync();
    fed2->enterExecutingModeAsync();
    try {
        fed3->enterExecutingMode();
        fed1->enterExecutingModeComplete();
    }
    catch (const helics::HelicsException&) {
        ;
    }
    EXPECT_THROW(fed2->enterExecutingModeComplete(), helics::ConnectionFailure);

    EXPECT_TRUE(broker->waitForDisconnect(std::chrono::milliseconds(500)));
}

TEST_F(error_tests, too_many_connections)
{
    auto broker = AddBroker("test", 2);

    AddFederates<helics::ValueFederate>("test", 2, broker, 1.0, "fed");

    auto fed1 = GetFederateAs<helics::ValueFederate>(0);
    auto fed2 = GetFederateAs<helics::ValueFederate>(1);

    fed1->registerGlobalPublication("t1", "double");
    fed1->registerGlobalPublication("t2", "double");
    fed1->registerGlobalPublication("t3", "double");
    auto& inp1 = fed2->registerGlobalInput("c1", "double");
    inp1.setOption(helics::defs::connections, 2);
    inp1.addTarget("t1");
    inp1.addTarget("t2");
    inp1.addTarget("t3");
    fed1->enterExecutingModeAsync();

    EXPECT_THROW(fed2->enterExecutingMode(), helics::ConnectionFailure);
    fed1->enterExecutingModeComplete();
    fed1->finalize();
    fed2->finalize();
    broker->disconnect();
}

TEST_F(error_tests, not_enough_connections)
{
    auto broker = AddBroker("test", 2);

    AddFederates<helics::ValueFederate>("test", 2, broker, 1.0, "fed");

    auto fed1 = GetFederateAs<helics::ValueFederate>(0);
    auto fed2 = GetFederateAs<helics::ValueFederate>(1);

    fed1->registerGlobalPublication("t1", "double");
    fed1->registerGlobalPublication("t2", "double");
    fed1->registerGlobalPublication("t3", "double");
    auto& inp1 = fed2->registerGlobalInput("c1", "double");
    inp1.setOption(helics::defs::connections, 3);
    inp1.addTarget("t1");
    inp1.addTarget("t2");
    fed1->enterExecutingModeAsync();

    EXPECT_THROW(fed2->enterExecutingMode(), helics::ConnectionFailure);
    fed1->enterExecutingModeComplete();
    fed1->finalize();
    fed2->finalize();
    broker->disconnect();
}

class error_tests_type: public ::testing::TestWithParam<const char*>, public FederateTestFixture {
};

/** test simple creation and destruction*/
TEST_P(error_tests_type, test_duplicate_broker_name)
{
    auto broker = AddBroker(GetParam(), "--name=brk1");
    EXPECT_TRUE(broker->isConnected());
    EXPECT_THROW(AddBroker(GetParam(), "--name=brk1 --timeout=500"), helics::RegistrationFailure);
    broker->disconnect();
    helics::cleanupHelicsLibrary();
}

INSTANTIATE_TEST_SUITE_P(error_tests, error_tests_type, ::testing::ValuesIn(core_types_simple));

#if defined(ENABLE_ZMQ_CORE) || defined(ENABLE_UDP_CORE)

constexpr const char* networkCores[] = {ZMQTEST UDPTEST};

// TCP core is odd for this test and doesn't work on all platforms due to the way TCP handles ports
// duplication
class network_error_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {
};

/** test simple creation and destruction*/
TEST_P(network_error_tests, test_duplicate_default_brokers)
{
    auto broker = AddBroker(GetParam(), "");
    auto broker2 = AddBroker(GetParam(), "--timeout=500");
    EXPECT_TRUE(!broker2->isConnected());
    broker->disconnect();
    helics::cleanupHelicsLibrary();
}

/** test broker recovery*/
TEST_P(network_error_tests, test_broker_recovery)
{
    auto broker = AddBroker(GetParam(), "");
    ASSERT_TRUE(broker->isConnected());
    auto res = std::async(std::launch::async, [&broker]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(1400));
        broker->disconnect();
    });
    auto broker2 = AddBroker(GetParam(), " --timeout=2500");
    EXPECT_TRUE(!broker->isConnected());
    EXPECT_TRUE(broker2->isConnected());
    broker2->disconnect();
    helics::cleanupHelicsLibrary();
}

INSTANTIATE_TEST_SUITE_P(error_tests, network_error_tests, ::testing::ValuesIn(networkCores));

#endif
