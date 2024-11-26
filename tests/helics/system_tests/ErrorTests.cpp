/*
Copyright (c) 2017-2024,
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
#include <memory>
#include <string>
#include <thread>

#define CORE_TYPE_TO_TEST helics::CoreType::TEST

struct error_tests: public FederateTestFixture, public ::testing::Test {};

TEST_F(error_tests, duplicate_federate_names)
{
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed = std::make_shared<helics::Federate>("test1", fedInfo);

    EXPECT_THROW(auto fed2 = std::make_shared<helics::Federate>("test1", fedInfo),
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
    helics::FederateInfo fedInfo;
    // get the core pointer from fed2 and using the name of fed1 should be an error
    EXPECT_THROW(helics::ValueFederate fed3(fed1->getName(), fed2->getCorePointer(), fedInfo),
                 helics::RegistrationFailure);
    broker->disconnect();
}

TEST_F(error_tests, already_init_broker)
{
    auto broker = AddBroker("test", 1);
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0, "fed");
    auto fed1 = GetFederateAs<helics::ValueFederate>(0);

    fed1->enterInitializingMode();
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreInitString = std::string("--timeout=1s --broker=") + broker->getIdentifier();
    EXPECT_THROW(helics::ValueFederate fed3("fed222", fedInfo), helics::RegistrationFailure);
    broker->disconnect();
}

TEST_F(error_tests, mismatch_broker_key)
{
    auto broker = AddBroker("test", 1);

    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreInitString =
        std::string("--timeout=1s --broker_key=tkey --broker=") + broker->getIdentifier();
    EXPECT_THROW(helics::ValueFederate fed3("fed222", fedInfo), helics::RegistrationFailure);
    broker->disconnect();
    auto core = helics::CoreFactory::findJoinableCoreOfType(helics::CoreType::TEST);
    if (core) {
        core->disconnect();
    }
    helics::cleanupHelicsLibrary();
}

TEST_F(error_tests, mismatch_broker_key2)
{
    auto broker = AddBroker("test", "-f 1 --broker_key=tkey2");

    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreInitString =
        std::string("--timeout=1s --broker_key=tkey --broker=") + broker->getIdentifier();
    EXPECT_THROW(helics::ValueFederate fed3("fed222", fedInfo), helics::RegistrationFailure);
    broker->disconnect();
    auto core = helics::CoreFactory::findJoinableCoreOfType(helics::CoreType::TEST);
    if (core) {
        core->disconnect();
    }
    helics::cleanupHelicsLibrary();
}

TEST_F(error_tests, mismatch_broker_key_success)
{
    auto broker = AddBroker("test", "--broker_key=tkey");

    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreInitString =
        std::string("--timeout=1s --broker_key=tkey --broker=") + broker->getIdentifier();
    helics::ValueFederate fed3("fed2b", fedInfo);
    fed3.enterExecutingMode();
    fed3.finalize();
    broker->waitForDisconnect();
    helics::cleanupHelicsLibrary();
}

TEST_F(error_tests, mismatch_broker_key_success_universal_key)
{
    auto broker = AddBroker("test", "--broker_key=**");

    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreInitString =
        std::string("--timeout=1s --broker_key=tkey --broker=") + broker->getIdentifier();
    helics::ValueFederate fed3("fed2b", fedInfo);
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

TEST_F(error_tests, single_thread_fed)
{
    extraFederateArgs = "--flags=single_thread_federate";
    auto broker = AddBroker("test", 1);
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0, "fed");

    auto fed1 = GetFederateAs<helics::ValueFederate>(0);
    EXPECT_THROW(fed1->enterInitializingModeAsync(), helics::InvalidFunctionCall);
    fed1->enterInitializingMode();
    EXPECT_THROW(fed1->enterExecutingModeAsync(), helics::InvalidFunctionCall);
    fed1->enterExecutingMode();

    EXPECT_THROW(fed1->requestTimeAsync(3.2), helics::InvalidFunctionCall);
    EXPECT_THROW(fed1->requestTimeComplete(), helics::InvalidFunctionCall);
    auto time = fed1->requestTime(2.0);
    EXPECT_EQ(time, 2.0);

    EXPECT_THROW(fed1->requestTimeIterativeAsync(3.2, helics::IterationRequest::FORCE_ITERATION),
                 helics::InvalidFunctionCall);
    EXPECT_THROW(fed1->requestTimeIterativeComplete(), helics::InvalidFunctionCall);

    EXPECT_THROW(fed1->finalizeAsync(), helics::InvalidFunctionCall);

    fed1->finalize();
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
        EXPECT_TRUE(fed2->getCurrentMode() == helics::Federate::Modes::ERROR_STATE);
        // this should do nothing
        EXPECT_THROW(fed2->enterExecutingMode(), helics::InvalidFunctionCall);
        EXPECT_TRUE(fed2->getCurrentMode() == helics::Federate::Modes::ERROR_STATE);
    }

    try {
        fed1->enterInitializingModeComplete();
    }
    catch (const helics::RegistrationFailure&) {
        gotException = true;
        EXPECT_TRUE(fed1->getCurrentMode() == helics::Federate::Modes::ERROR_STATE);
        // this should do nothing
        EXPECT_THROW(fed1->enterExecutingMode(), helics::InvalidFunctionCall);
        EXPECT_TRUE(fed1->getCurrentMode() == helics::Federate::Modes::ERROR_STATE);
    }
    EXPECT_TRUE(gotException);

    fed1->finalize();
    fed2->finalize();
    broker->disconnect();
}

TEST_F(error_tests, duplicate_publication_names2_init_iteration)
{
    auto broker = AddBroker("test", 2);
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0, "fed");
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0, "fed");

    auto fed1 = GetFederateAs<helics::ValueFederate>(0);
    auto fed2 = GetFederateAs<helics::ValueFederate>(1);

    fed1->registerGlobalPublication("testkey", "");
    fed1->enterInitializingModeAsync();

    fed2->enterInitializingModeIterative();

    fed2->registerGlobalPublication("testkey", "");

    bool gotException = false;
    try {
        fed2->enterInitializingMode();
    }
    catch (const helics::RegistrationFailure&) {
        gotException = true;
        EXPECT_TRUE(fed2->getCurrentMode() == helics::Federate::Modes::ERROR_STATE);
        // this should do nothing
        EXPECT_THROW(fed2->enterExecutingMode(), helics::InvalidFunctionCall);
        EXPECT_TRUE(fed2->getCurrentMode() == helics::Federate::Modes::ERROR_STATE);
    }

    try {
        fed1->enterInitializingModeComplete();
    }
    catch (const helics::RegistrationFailure&) {
        gotException = true;
        EXPECT_TRUE(fed1->getCurrentMode() == helics::Federate::Modes::ERROR_STATE);
        // this should do nothing
        EXPECT_THROW(fed1->enterExecutingMode(), helics::InvalidFunctionCall);
        EXPECT_TRUE(fed1->getCurrentMode() == helics::Federate::Modes::ERROR_STATE);
    }
    EXPECT_TRUE(gotException);

    fed1->finalize();
    fed2->finalize();
    broker->disconnect();
}

TEST_F(error_tests, duplicate_publication_names_auto_terminate)
{
    auto broker = AddBroker("test", "-f 2");
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0, "fed");
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0, "fed");

    auto fed1 = GetFederateAs<helics::ValueFederate>(0);
    auto fed2 = GetFederateAs<helics::ValueFederate>(1);

    fed1->setFlagOption(HELICS_FLAG_TERMINATE_ON_ERROR);
    fed2->setFlagOption(HELICS_FLAG_TERMINATE_ON_ERROR);
    fed1->registerGlobalPublication("testkey", "");
    fed1->enterInitializingModeAsync();

    fed2->registerGlobalPublication("testkey", "");

    bool gotException = false;
    try {
        fed2->enterInitializingMode();
    }
    catch (const helics::RegistrationFailure&) {
        gotException = true;
        EXPECT_TRUE(fed2->getCurrentMode() == helics::Federate::Modes::ERROR_STATE);
    }

    try {
        fed1->enterInitializingModeComplete();
    }
    catch (const helics::RegistrationFailure&) {
        gotException = true;
        EXPECT_TRUE(fed1->getCurrentMode() == helics::Federate::Modes::ERROR_STATE);
    }
    EXPECT_TRUE(gotException);

    broker->waitForDisconnect();
}

TEST_F(error_tests, duplicate_publication_names_auto_terminate_core)
{
    auto broker = AddBroker("test", "-f 2");
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0, "fed");
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0, "fed");

    auto fed1 = GetFederateAs<helics::ValueFederate>(0);
    auto fed2 = GetFederateAs<helics::ValueFederate>(1);

    fed1->getCorePointer()->setFlagOption(helics::gLocalCoreId,
                                          HELICS_FLAG_TERMINATE_ON_ERROR,
                                          true);
    fed2->getCorePointer()->setFlagOption(helics::gLocalCoreId,
                                          HELICS_FLAG_TERMINATE_ON_ERROR,
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
        EXPECT_TRUE(fed2->getCurrentMode() == helics::Federate::Modes::ERROR_STATE);
    }

    try {
        fed1->enterInitializingModeComplete();
    }
    catch (const helics::RegistrationFailure&) {
        gotException = true;
        EXPECT_TRUE(fed1->getCurrentMode() == helics::Federate::Modes::ERROR_STATE);
    }
    EXPECT_TRUE(gotException);

    broker->waitForDisconnect();
}

TEST_F(error_tests, duplicate_publication_names_auto_terminate_broker)
{
    auto broker = AddBroker("test", "-f 2 --terminate_on_error");
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
        EXPECT_TRUE(fed2->getCurrentMode() == helics::Federate::Modes::ERROR_STATE);
    }

    try {
        fed1->enterInitializingModeComplete();
    }
    catch (const helics::HelicsException&) {
        gotException = true;
        EXPECT_TRUE(fed1->getCurrentMode() == helics::Federate::Modes::ERROR_STATE);
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

    helics::Publication pub(fed1, "testkey", helics::DataType::HELICS_DOUBLE);
    // copy constructor
    helics::Publication pub2(pubid);

    auto& sub = fed1->registerSubscription(pubid.getName());
    fed1->enterExecutingMode();
    pubid.publish(45.7);
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
    auto& sub1 = fed2->registerSubscription("abcd", "");
    sub1.setOption(helics::defs::Options::CONNECTION_REQUIRED);

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
    fed2->setFlagOption(helics::defs::Flags::CONNECTIONS_REQUIRED, true);
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
    sub.setOption(helics::defs::Options::IGNORE_UNIT_MISMATCH);
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
    auto broker = AddBroker("test", "-f 3 ");
    extraCoreArgs = "--error_timeout=0";
    AddFederates<helics::ValueFederate>("test", 3, broker, 1.0, "fed");

    auto fed1 = GetFederateAs<helics::ValueFederate>(0);
    auto fed2 = GetFederateAs<helics::ValueFederate>(1);
    auto fed3 = GetFederateAs<helics::ValueFederate>(2);

    fed1->registerGlobalPublication("t1", "double", "V");
    fed2->setFlagOption(HELICS_FLAG_TERMINATE_ON_ERROR);
    fed2->registerSubscription("t1", "m");
    auto& sub = fed3->registerSubscription("t1", "m");
    sub.setOption(helics::defs::Options::IGNORE_UNIT_MISMATCH);
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

TEST_F(error_tests, missing_required_ept)
{
    auto broker = AddBroker("test", 2);

    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "fed");
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "fed");

    auto fed1 = GetFederateAs<helics::MessageFederate>(0);
    auto fed2 = GetFederateAs<helics::MessageFederate>(1);

    fed1->registerGlobalTargetedEndpoint("t1", "");
    auto& ept1 = fed2->registerGlobalTargetedEndpoint("abcd", "");
    ept1.setOption(helics::defs::Options::CONNECTION_REQUIRED);

    fed1->enterInitializingModeAsync();
    EXPECT_THROW(fed2->enterInitializingMode(), helics::ConnectionFailure);
    fed1->finalize();
    fed2->finalize();
    broker->disconnect();
}

class error_tests_type: public ::testing::TestWithParam<const char*>, public FederateTestFixture {};

/** test simple creation and destruction*/
TEST_P(error_tests_type, duplicate_broker_name)
{
    std::string bname = std::string("brk_dup_") + GetParam();
    auto broker = AddBroker(GetParam(), std::string("--name=") + bname);
    EXPECT_TRUE(broker->isConnected());
    EXPECT_THROW(AddBroker(GetParam(), std::string("--name=") + bname + " --timeout=500"),
                 helics::RegistrationFailure);
    broker->disconnect();
    helics::cleanupHelicsLibrary();
}

INSTANTIATE_TEST_SUITE_P(error_tests, error_tests_type, ::testing::ValuesIn(CoreTypes_simple));

#if defined(HELICS_ENABLE_ZMQ_CORE) || defined(HELICS_ENABLE_UDP_CORE)

constexpr const char* networkCores[] = {ZMQTEST UDPTEST};

// TCP core is odd for this test and doesn't work on all platforms due to the way TCP handles ports
// duplication
class network_error_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {};

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
