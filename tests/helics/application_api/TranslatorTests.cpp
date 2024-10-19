/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/BrokerApp.hpp"
#include "helics/application_api/CombinationFederate.hpp"
#include "helics/application_api/CoreApp.hpp"
#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/Federate.hpp"
#include "helics/application_api/MessageFederate.hpp"
#include "helics/application_api/Translator.hpp"
#include "helics/application_api/TranslatorOperations.hpp"
#include "helics/application_api/ValueFederate.hpp"

#ifndef HELICS_SHARED_LIBRARY
#    include "helics/core/Broker.hpp"
#    include "testFixtures.hpp"
#else
#    include "testFixtures_shared.hpp"
#endif
#include "helics/common/JsonProcessingFunctions.hpp"

#include <future>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <thread>
#include <utility>

/** these test cases test out translator operations
 */

class TranslatorFixture: public ::testing::Test, public FederateTestFixture {};

/** test registration of translators*/
TEST_F(TranslatorFixture, translator_registration)
{
    auto broker = AddBroker("test", 2);

    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "filter");
    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "message");
    // broker->setLoggingLevel (3);
    broker.reset();

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto mFed = GetFederateAs<helics::MessageFederate>(1);

    auto& translator1 = mFed->registerGlobalTranslator("t1");

    auto& translator2 = fFed->registerTranslator("t2");

    EXPECT_EQ(translator1.getName(), "t1");
    EXPECT_EQ(translator2.getName(), "filter0/t2");

    EXPECT_EQ(translator1.getSourceTargets(), "");
    mFed->finalizeAsync();

    fFed->finalize();
    // std::cout << "fFed returned\n";
    mFed->finalizeComplete();
    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::FINALIZE);
    FullDisconnect();
}

TEST_F(TranslatorFixture, translator_queries)
{
    auto broker = AddBroker("test", 2);

    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "A");
    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "B");
    // broker->setLoggingLevel (3);

    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    mFed1->registerGlobalTranslator("t1");

    mFed2->registerTranslator("t2");

    mFed2->enterExecutingModeAsync();
    mFed1->enterExecutingMode();
    mFed2->enterExecutingModeComplete();

    auto str = broker->query("root", "translators");
    EXPECT_EQ(str.find("error"), std::string::npos);
    EXPECT_NE(str.find("t1"), std::string::npos);
    EXPECT_NE(str.find("B1/t2"), std::string::npos);
    broker.reset();

    str = mFed1->query("core", "translators");
    EXPECT_EQ(str.find("error"), std::string::npos);
    EXPECT_NE(str.find("t1"), std::string::npos);
    EXPECT_EQ(str.find("B1/t2"), std::string::npos);

    mFed2->finalizeAsync();

    mFed1->finalize();
    // std::cout << "fFed returned\n";
    mFed2->finalizeComplete();

    FullDisconnect();
}

TEST_F(TranslatorFixture, translator_connections1)
{
    auto broker = AddBroker("test", 2);

    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "A");
    AddFederates<helics::ValueFederate>("test", 1, broker, helics::timeZero, "B");
    // broker->setLoggingLevel (3);

    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::ValueFederate>(1);

    mFed1->registerGlobalTranslator("t1");

    auto& pub2 = mFed2->registerPublication("p2", "any");
    pub2.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    pub2.addDestinationTarget("t1");
    mFed2->enterExecutingModeAsync();
    mFed1->enterExecutingMode();
    EXPECT_NO_THROW(mFed2->enterExecutingModeComplete());

    mFed2->finalizeAsync();

    mFed1->finalize();
    // std::cout << "fFed returned\n";
    mFed2->finalizeComplete();

    FullDisconnect();
}

TEST_F(TranslatorFixture, translator_connections2)
{
    auto broker = AddBroker("test", 2);

    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "A");
    AddFederates<helics::ValueFederate>("test", 1, broker, helics::timeZero, "B");
    // broker->setLoggingLevel (3);

    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::ValueFederate>(1);

    auto& pub2 = mFed2->registerPublication("p2", "any");
    pub2.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    pub2.addDestinationTarget("t1");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    mFed1->registerGlobalTranslator("t1");
    mFed2->enterExecutingModeAsync();
    mFed1->enterExecutingMode();
    EXPECT_NO_THROW(mFed2->enterExecutingModeComplete());

    mFed2->finalizeAsync();

    mFed1->finalize();
    // std::cout << "fFed returned\n";
    mFed2->finalizeComplete();

    FullDisconnect();
}

TEST_F(TranslatorFixture, translator_connections3_nosan)
{
    auto broker = AddBroker("test", 2);

    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "A");
    AddFederates<helics::ValueFederate>("test", 1, broker, helics::timeZero, "B");

    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::ValueFederate>(1);

    // this is to prevent it from printing some expected errors in this test
    broker->setLoggingLevel(HELICS_LOG_LEVEL_NO_PRINT);
    mFed1->setProperty(HELICS_PROPERTY_INT_LOG_LEVEL, HELICS_LOG_LEVEL_NO_PRINT);
    mFed2->setProperty(HELICS_PROPERTY_INT_LOG_LEVEL, HELICS_LOG_LEVEL_NO_PRINT);
    mFed1->getCorePointer()->setIntegerProperty(helics::gLocalCoreId,
                                                HELICS_PROPERTY_INT_LOG_LEVEL,
                                                HELICS_LOG_LEVEL_NO_PRINT);
    mFed2->setProperty(HELICS_PROPERTY_INT_LOG_LEVEL, HELICS_LOG_LEVEL_NO_PRINT);
    mFed2->getCorePointer()->setIntegerProperty(helics::gLocalCoreId,
                                                HELICS_PROPERTY_INT_LOG_LEVEL,
                                                HELICS_LOG_LEVEL_NO_PRINT);
    auto& pub2 = mFed2->registerPublication("p2", "any");
    pub2.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    pub2.addDestinationTarget("t2");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    mFed1->registerGlobalTranslator("t1");
    mFed2->enterExecutingModeAsync();
    EXPECT_THROW(mFed1->enterExecutingMode(), helics::ConnectionFailure);
    EXPECT_THROW(mFed2->enterExecutingModeComplete(), helics::ConnectionFailure);

    mFed2->finalizeAsync();

    mFed1->finalize();
    // std::cout << "fFed returned\n";
    mFed2->finalizeComplete();

    FullDisconnect();
}

TEST_F(TranslatorFixture, translator_connections4)
{
    auto broker = AddBroker("test", 2);

    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "A");
    AddFederates<helics::ValueFederate>("test", 1, broker, helics::timeZero, "B");
    // broker->setLoggingLevel (3);

    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::ValueFederate>(1);

    mFed1->registerGlobalTranslator("t1");

    auto& pub2 = mFed2->registerInput("p2", "any");
    pub2.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    pub2.addSourceTarget("t1");
    mFed2->enterInitializingModeAsync();
    mFed1->enterInitializingMode();
    EXPECT_NO_THROW(mFed2->enterInitializingModeComplete());

    mFed2->finalizeAsync();

    mFed1->finalize();
    // std::cout << "fFed returned\n";
    mFed2->finalizeComplete();

    FullDisconnect();
}

TEST_F(TranslatorFixture, translator_connections4_alias)
{
    auto broker = AddBroker("test", 2);
    broker->addAlias("t1", "trans");
    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "A");
    AddFederates<helics::ValueFederate>("test", 1, broker, helics::timeZero, "B");
    // broker->setLoggingLevel (3);

    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::ValueFederate>(1);

    mFed1->registerGlobalTranslator("t1");

    auto& pub2 = mFed2->registerInput("p2", "any");
    pub2.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    pub2.addSourceTarget("trans");
    mFed2->enterInitializingModeAsync();
    mFed1->enterInitializingMode();
    EXPECT_NO_THROW(mFed2->enterInitializingModeComplete());

    mFed2->finalizeAsync();

    mFed1->finalize();
    mFed2->finalizeComplete();

    FullDisconnect();
}

TEST_F(TranslatorFixture, translator_connections5)
{
    auto broker = AddBroker("test", 2);

    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "A");
    AddFederates<helics::ValueFederate>("test", 1, broker, helics::timeZero, "B");
    // broker->setLoggingLevel (3);

    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::ValueFederate>(1);

    auto& pub2 = mFed2->registerInput("p2", "any");
    pub2.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    pub2.addSourceTarget("t1");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    mFed1->registerGlobalTranslator("t1");
    mFed2->enterInitializingModeAsync();
    mFed1->enterInitializingMode();
    EXPECT_NO_THROW(mFed2->enterInitializingModeComplete());

    mFed2->finalizeAsync();

    mFed1->finalize();
    // std::cout << "fFed returned\n";
    mFed2->finalizeComplete();

    FullDisconnect();
}

TEST_F(TranslatorFixture, translator_connections5_alias)
{
    auto broker = AddBroker("test", 2);

    broker->addAlias("t1", "trans");
    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "A");
    AddFederates<helics::ValueFederate>("test", 1, broker, helics::timeZero, "B");
    // broker->setLoggingLevel (3);

    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::ValueFederate>(1);

    auto& pub2 = mFed2->registerInput("p2", "any");
    pub2.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    pub2.addSourceTarget("trans");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    mFed1->registerGlobalTranslator("t1");
    mFed2->enterInitializingModeAsync();
    mFed1->enterInitializingMode();
    EXPECT_NO_THROW(mFed2->enterInitializingModeComplete());

    mFed2->finalizeAsync();

    mFed1->finalize();
    mFed2->finalizeComplete();

    FullDisconnect();
}

TEST_F(TranslatorFixture, translator_connections6_nosan)
{
    auto broker = AddBroker("test", 2);

    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "A");
    AddFederates<helics::ValueFederate>("test", 1, broker, helics::timeZero, "B");

    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::ValueFederate>(1);

    // this is to prevent it from printing some expected errors in this test
    broker->setLoggingLevel(HELICS_LOG_LEVEL_NO_PRINT);
    mFed1->setProperty(HELICS_PROPERTY_INT_LOG_LEVEL, HELICS_LOG_LEVEL_NO_PRINT);
    mFed2->setProperty(HELICS_PROPERTY_INT_LOG_LEVEL, HELICS_LOG_LEVEL_NO_PRINT);
    mFed1->getCorePointer()->setIntegerProperty(helics::gLocalCoreId,
                                                HELICS_PROPERTY_INT_LOG_LEVEL,
                                                HELICS_LOG_LEVEL_NO_PRINT);
    mFed2->setProperty(HELICS_PROPERTY_INT_LOG_LEVEL, HELICS_LOG_LEVEL_NO_PRINT);
    mFed2->getCorePointer()->setIntegerProperty(helics::gLocalCoreId,
                                                HELICS_PROPERTY_INT_LOG_LEVEL,
                                                HELICS_LOG_LEVEL_NO_PRINT);
    auto& pub2 = mFed2->registerInput("p2", "any");
    pub2.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    pub2.addSourceTarget("t2");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    mFed1->registerGlobalTranslator("t1");
    mFed2->enterInitializingModeAsync();
    EXPECT_THROW(mFed1->enterInitializingMode(), helics::ConnectionFailure);
    EXPECT_THROW(mFed2->enterInitializingModeComplete(), helics::ConnectionFailure);

    mFed2->finalizeAsync();

    mFed1->finalize();
    // std::cout << "fFed returned\n";
    mFed2->finalizeComplete();

    FullDisconnect();
}

TEST_F(TranslatorFixture, translator_connections7)
{
    auto broker = AddBroker("test", 2);

    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "A");
    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "B");
    // broker->setLoggingLevel (3);

    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    mFed1->registerGlobalTranslator("t1");

    auto& pub2 = mFed2->registerTargetedEndpoint("p2", "any");
    pub2.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    pub2.addSourceTarget("t1");
    mFed2->enterInitializingModeAsync();
    mFed1->enterInitializingMode();
    EXPECT_NO_THROW(mFed2->enterInitializingModeComplete());

    mFed2->finalizeAsync();

    mFed1->finalize();
    // std::cout << "fFed returned\n";
    mFed2->finalizeComplete();

    FullDisconnect();
}

TEST_F(TranslatorFixture, translator_connections8)
{
    auto broker = AddBroker("test", 2);

    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "A");
    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "B");
    // broker->setLoggingLevel (3);

    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    auto& pub2 = mFed2->registerTargetedEndpoint("p2", "any");
    pub2.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    pub2.addSourceTarget("t1");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    mFed1->registerGlobalTranslator("t1");
    mFed2->enterInitializingModeAsync();
    mFed1->enterInitializingMode();
    EXPECT_NO_THROW(mFed2->enterInitializingModeComplete());

    mFed2->finalizeAsync();

    mFed1->finalize();
    // std::cout << "fFed returned\n";
    mFed2->finalizeComplete();

    FullDisconnect();
}

TEST_F(TranslatorFixture, translator_connections9_nosan)
{
    auto broker = AddBroker("test", 2);

    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "A");
    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "B");

    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    // this is to prevent it from printing some expected errors in this test
    broker->setLoggingLevel(HELICS_LOG_LEVEL_NO_PRINT);
    mFed1->setProperty(HELICS_PROPERTY_INT_LOG_LEVEL, HELICS_LOG_LEVEL_NO_PRINT);
    mFed2->setProperty(HELICS_PROPERTY_INT_LOG_LEVEL, HELICS_LOG_LEVEL_NO_PRINT);
    mFed1->getCorePointer()->setIntegerProperty(helics::gLocalCoreId,
                                                HELICS_PROPERTY_INT_LOG_LEVEL,
                                                HELICS_LOG_LEVEL_NO_PRINT);
    mFed2->setProperty(HELICS_PROPERTY_INT_LOG_LEVEL, HELICS_LOG_LEVEL_NO_PRINT);
    mFed2->getCorePointer()->setIntegerProperty(helics::gLocalCoreId,
                                                HELICS_PROPERTY_INT_LOG_LEVEL,
                                                HELICS_LOG_LEVEL_NO_PRINT);
    auto& pub2 = mFed2->registerTargetedEndpoint("p2", "any");
    pub2.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    pub2.addSourceTarget("t2");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    mFed1->registerGlobalTranslator("t1");
    mFed2->enterInitializingModeAsync();
    EXPECT_THROW(mFed1->enterInitializingMode(), helics::ConnectionFailure);
    EXPECT_THROW(mFed2->enterInitializingModeComplete(), helics::ConnectionFailure);

    mFed2->finalizeAsync();

    mFed1->finalize();
    // std::cout << "fFed returned\n";
    mFed2->finalizeComplete();

    FullDisconnect();
}

TEST_F(TranslatorFixture, translator_connections10)
{
    auto broker = AddBroker("test", 1);

    AddFederates<helics::CombinationFederate>("test", 1, broker, helics::timeZero, "A");

    auto cFed1 = GetFederateAs<helics::CombinationFederate>(0);

    auto& endpoint1 = cFed1->registerGlobalTargetedEndpoint("e1", "any");
    auto& input1 = cFed1->registerGlobalInput<double>("i1");
    auto& pub1 = cFed1->registerGlobalPublication<double>("p1");
    pub1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    endpoint1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    input1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);

    endpoint1.addSourceTarget("t1");
    pub1.addDestinationTarget("t1");
    input1.addSourceTarget("t1");
    endpoint1.addDestinationTarget("t1");

    cFed1->registerGlobalTranslator("t1");

    EXPECT_NO_THROW(cFed1->enterExecutingMode());

    cFed1->finalize();

    FullDisconnect();
}

TEST_F(TranslatorFixture, translator_time_advance)
{
    auto broker = AddBroker("test", 1);

    AddFederates<helics::CombinationFederate>("test", 1, broker, helics::timeZero, "A");

    auto cFed1 = GetFederateAs<helics::CombinationFederate>(0);

    auto& endpoint1 = cFed1->registerGlobalTargetedEndpoint("e1", "any");
    auto& input1 = cFed1->registerGlobalInput<double>("i1");
    auto& pub1 = cFed1->registerGlobalPublication<double>("p1");
    pub1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    endpoint1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    input1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);

    endpoint1.addSourceTarget("t1");
    pub1.addDestinationTarget("t1");
    input1.addSourceTarget("t1");
    endpoint1.addDestinationTarget("t1");

    cFed1->registerGlobalTranslator("t1");

    EXPECT_NO_THROW(cFed1->enterExecutingMode());

    auto tres = cFed1->requestTime(2.0);
    EXPECT_EQ(tres, 2.0);
    cFed1->finalize();

    FullDisconnect();
}

TEST_F(TranslatorFixture, translator_to_message)
{
    auto broker = AddBroker("test", 1);

    AddFederates<helics::CombinationFederate>("test", 1, broker, helics::timeZero, "A");

    auto cFed1 = GetFederateAs<helics::CombinationFederate>(0);

    auto& endpoint1 = cFed1->registerGlobalTargetedEndpoint("e1", "any");
    auto& input1 = cFed1->registerGlobalInput<double>("i1");
    auto& pub1 = cFed1->registerGlobalPublication<double>("p1");
    pub1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    endpoint1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    input1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);

    endpoint1.addSourceTarget("t1");
    pub1.addDestinationTarget("t1");
    input1.addSourceTarget("t1");
    endpoint1.addDestinationTarget("t1");

    cFed1->registerGlobalTranslator(helics::TranslatorTypes::JSON, "t1");

    EXPECT_NO_THROW(cFed1->enterExecutingMode());

    pub1.publish(20.7);
    auto tres = cFed1->requestTime(2.0);
    EXPECT_LT(tres, 2.0);
    EXPECT_TRUE(endpoint1.hasMessage());
    auto message = endpoint1.getMessage();

    cFed1->finalize();
    ASSERT_TRUE(message);

    auto json = helics::fileops::loadJsonStr(message->data.to_string());

    ASSERT_TRUE(json.contains("value"));
    ASSERT_TRUE(json.contains("type"));
    EXPECT_DOUBLE_EQ(json["value"].get<double>(), 20.7);
    EXPECT_EQ(json["type"].get<std::string>(), "double");
    FullDisconnect();
}

TEST_F(TranslatorFixture, translator_to_message_2fed)
{
    auto broker = AddBroker("test", 2);

    AddFederates<helics::CombinationFederate>("test", 1, broker, 1.0, "A");
    AddFederates<helics::CombinationFederate>("test", 1, broker, 1.0, "B");

    auto cFed1 = GetFederateAs<helics::CombinationFederate>(0);
    auto cFed2 = GetFederateAs<helics::CombinationFederate>(1);

    auto& endpoint1 = cFed2->registerGlobalTargetedEndpoint("e1", "any");
    auto& pub1 = cFed1->registerGlobalPublication<double>("p1");
    pub1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    endpoint1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);

    endpoint1.addSourceEndpoint("t1");
    pub1.addDestinationTarget("t1");

    cFed1->registerGlobalTranslator(helics::TranslatorTypes::JSON, "t1");

    cFed1->enterExecutingModeAsync();
    cFed2->enterExecutingMode();
    cFed1->enterExecutingModeComplete();

    pub1.publish(20.7);
    auto tres = cFed1->requestTime(2.0);
    EXPECT_EQ(tres, 2.0);
    tres = cFed2->requestTime(3.0);
    EXPECT_EQ(tres, 1.0);
    EXPECT_TRUE(endpoint1.hasMessage());
    auto message = endpoint1.getMessage();

    cFed1->finalize();
    cFed2->finalize();
    ASSERT_TRUE(message);

    auto json = helics::fileops::loadJsonStr(message->data.to_string());

    ASSERT_TRUE(json.contains("value"));
    ASSERT_TRUE(json.contains("type"));
    EXPECT_DOUBLE_EQ(json["value"].get<double>(), 20.7);
    EXPECT_EQ(json["type"].get<std::string>(), "double");
    FullDisconnect();
}

TEST_F(TranslatorFixture, translator_to_message_target_from_translator)
{
    auto broker = AddBroker("test", 1);

    AddFederates<helics::CombinationFederate>("test", 1, broker, helics::timeZero, "A");

    auto cFed1 = GetFederateAs<helics::CombinationFederate>(0);

    auto& endpoint1 = cFed1->registerGlobalTargetedEndpoint("e1", "any");
    auto& pub1 = cFed1->registerGlobalPublication<double>("p1");

    auto& translator1 = cFed1->registerGlobalTranslator(helics::TranslatorTypes::JSON, "t1");
    translator1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);

    translator1.addSourceEndpoint("e1");
    translator1.addDestinationEndpoint("e1");
    translator1.addPublication("p1");

    EXPECT_NO_THROW(cFed1->enterExecutingMode());

    pub1.publish(20.7);
    auto tres = cFed1->requestTime(2.0);
    EXPECT_LT(tres, 2.0);
    EXPECT_TRUE(endpoint1.hasMessage());
    auto message = endpoint1.getMessage();

    cFed1->finalize();
    ASSERT_TRUE(message);

    auto json = helics::fileops::loadJsonStr(message->data.to_string());

    ASSERT_TRUE(json.contains("value"));
    ASSERT_TRUE(json.contains("type"));
    EXPECT_DOUBLE_EQ(json["value"].get<double>(), 20.7);
    EXPECT_EQ(json["type"].get<std::string>(), "double");
    FullDisconnect();
}

TEST_F(TranslatorFixture, translator_round_trip)
{
    auto broker = AddBroker("test", 1);

    AddFederates<helics::CombinationFederate>("test", 1, broker, helics::timeZero, "A");

    auto cFed1 = GetFederateAs<helics::CombinationFederate>(0);

    auto& endpoint1 = cFed1->registerGlobalTargetedEndpoint("e1", "any");
    auto& input1 = cFed1->registerGlobalInput<double>("i1");
    auto& pub1 = cFed1->registerGlobalPublication<double>("p1");
    pub1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    endpoint1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    input1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);

    endpoint1.addSourceEndpoint("t1");
    pub1.addInputTarget("t1");
    input1.addPublication("t1");
    endpoint1.addDestinationEndpoint("t1");

    cFed1->registerGlobalTranslator(helics::TranslatorTypes::JSON, "t1");

    EXPECT_NO_THROW(cFed1->enterExecutingMode());

    pub1.publish(20.7);
    auto tres = cFed1->requestTime(2.0);
    EXPECT_LT(tres, 2.0);
    EXPECT_TRUE(endpoint1.hasMessage());
    auto message = endpoint1.getMessage();

    ASSERT_TRUE(message);
    message->dest.clear();
    endpoint1.send(std::move(message));
    auto tres2 = cFed1->requestTime(2.0);
    EXPECT_LT(tres2, 2.0);
    EXPECT_GT(tres2, tres);
    EXPECT_FALSE(endpoint1.hasMessage());
    EXPECT_TRUE(input1.isUpdated());
    EXPECT_DOUBLE_EQ(input1.getDouble(), 20.7);
    cFed1->finalize();
    FullDisconnect();
}

TEST_F(TranslatorFixture, translator_round_trip_target_from_translator)
{
    auto broker = AddBroker("test", 1);

    AddFederates<helics::CombinationFederate>("test", 1, broker, helics::timeZero, "A");

    auto cFed1 = GetFederateAs<helics::CombinationFederate>(0);

    auto& endpoint1 = cFed1->registerGlobalTargetedEndpoint("e1", "any");
    auto& input1 = cFed1->registerGlobalInput<double>("i1");
    auto& pub1 = cFed1->registerGlobalPublication<double>("p1");

    auto& translator1 = cFed1->registerGlobalTranslator(helics::TranslatorTypes::JSON, "t1");
    translator1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    translator1.addSourceEndpoint("e1");
    translator1.addDestinationEndpoint("e1");
    translator1.addPublication("p1");
    translator1.addInputTarget("i1");

    EXPECT_NO_THROW(cFed1->enterExecutingMode());

    pub1.publish(20.7);
    auto tres = cFed1->requestTime(2.0);
    EXPECT_LT(tres, 2.0);
    EXPECT_TRUE(endpoint1.hasMessage());
    auto message = endpoint1.getMessage();

    ASSERT_TRUE(message);
    message->dest.clear();
    endpoint1.send(std::move(message));
    auto tres2 = cFed1->requestTime(2.0);
    EXPECT_LT(tres2, 2.0);
    EXPECT_GT(tres2, tres);
    EXPECT_FALSE(endpoint1.hasMessage());
    EXPECT_TRUE(input1.isUpdated());
    EXPECT_DOUBLE_EQ(input1.getDouble(), 20.7);
    cFed1->finalize();
    FullDisconnect();
}

TEST_F(TranslatorFixture, translator_to_multimessage)
{
    auto broker = AddBroker("test", 1);

    AddFederates<helics::CombinationFederate>("test", 1, broker, helics::timeZero, "A");

    auto cFed1 = GetFederateAs<helics::CombinationFederate>(0);

    auto& endpoint1 = cFed1->registerGlobalTargetedEndpoint("e1", "any");
    auto& endpoint2 = cFed1->registerGlobalTargetedEndpoint("e2", "any");
    auto& input1 = cFed1->registerGlobalInput<double>("i1");
    auto& pub1 = cFed1->registerGlobalPublication<double>("p1");
    pub1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    endpoint1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    input1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);

    endpoint1.addDestinationEndpoint("t1");
    pub1.addInputTarget("t1");
    input1.addPublication("t1");
    endpoint1.addSourceEndpoint("t1");
    endpoint2.addSourceEndpoint("t1");

    cFed1->registerGlobalTranslator(helics::TranslatorTypes::JSON, "t1");

    EXPECT_NO_THROW(cFed1->enterExecutingMode());

    pub1.publish(20.7);
    auto tres = cFed1->requestTime(2.0);
    EXPECT_LT(tres, 2.0);
    EXPECT_TRUE(endpoint1.hasMessage());
    auto message = endpoint1.getMessage();

    ASSERT_TRUE(message);

    auto json = helics::fileops::loadJsonStr(message->data.to_string());

    ASSERT_TRUE(json.contains("value"));
    ASSERT_TRUE(json.contains("type"));
    EXPECT_DOUBLE_EQ(json["value"].get<double>(), 20.7);
    EXPECT_EQ(json["type"].get<std::string>(), "double");

    EXPECT_TRUE(endpoint2.hasMessage());
    auto message2 = endpoint2.getMessage();

    cFed1->finalize();
    ASSERT_TRUE(message2);

    auto json2 = helics::fileops::loadJsonStr(message2->data.to_string());

    ASSERT_TRUE(json2.contains("value"));
    ASSERT_TRUE(json2.contains("type"));
    EXPECT_DOUBLE_EQ(json2["value"].get<double>(), 20.7);
    EXPECT_EQ(json2["type"].get<std::string>(), "double");
    FullDisconnect();
}

TEST_F(TranslatorFixture, translator_from_message)
{
    auto broker = AddBroker("test", 1);

    AddFederates<helics::CombinationFederate>("test", 1, broker, helics::timeZero, "A");

    auto cFed1 = GetFederateAs<helics::CombinationFederate>(0);

    auto& endpoint1 = cFed1->registerGlobalTargetedEndpoint("e1", "any");
    auto& input1 = cFed1->registerGlobalInput<double>("i1");
    auto& pub1 = cFed1->registerGlobalPublication<double>("p1");
    pub1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    endpoint1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    input1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);

    endpoint1.addSourceTarget("t1");
    pub1.addDestinationTarget("t1");
    input1.addSourceTarget("t1");
    endpoint1.addDestinationTarget("t1");

    cFed1->registerGlobalTranslator(helics::TranslatorTypes::JSON, "t1");

    EXPECT_NO_THROW(cFed1->enterExecutingMode());

    auto json = helics::typeConvert(helics::DataType::HELICS_JSON, 20.7);
    std::string val;
    helics::valueExtract(json, helics::DataType::HELICS_JSON, val);
    endpoint1.send(val);
    auto tres = cFed1->requestTime(2.0);
    EXPECT_LT(tres, 2.0);
    EXPECT_TRUE(input1.isUpdated());
    EXPECT_DOUBLE_EQ(input1.getValue<double>(), 20.7);

    cFed1->finalize();

    FullDisconnect();
}

TEST_F(TranslatorFixture, translator_from_message_2fed)
{
    auto broker = AddBroker("test", 2);

    AddFederates<helics::CombinationFederate>("test", 1, broker, 1.0, "A");
    AddFederates<helics::CombinationFederate>("test", 1, broker, 1.0, "B");
    auto cFed1 = GetFederateAs<helics::CombinationFederate>(0);
    auto cFed2 = GetFederateAs<helics::CombinationFederate>(1);

    auto& endpoint1 = cFed1->registerGlobalTargetedEndpoint("e1", "any");
    auto& input1 = cFed2->registerGlobalInput<double>("i1");

    endpoint1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    input1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);

    input1.addSourceTarget("t1");
    endpoint1.addDestinationTarget("t1");

    cFed1->registerGlobalTranslator(helics::TranslatorTypes::JSON, "t1");

    cFed2->enterExecutingModeAsync();
    EXPECT_NO_THROW(cFed1->enterExecutingMode());
    cFed2->enterExecutingModeComplete();

    auto data = helics::typeConvert(helics::DataType::HELICS_JSON, 20.7);
    std::string val{data.to_string()};

    endpoint1.send(val);
    // there should be no dependencies here so it should grant immediately
    auto tres = cFed1->requestTime(3.0);
    EXPECT_EQ(tres, 3.0);

    tres = cFed2->requestTime(3.0);
    EXPECT_EQ(tres, 1.0);
    EXPECT_TRUE(input1.isUpdated());
    EXPECT_DOUBLE_EQ(input1.getValue<double>(), 20.7);

    cFed1->finalize();
    cFed2->finalize();
    FullDisconnect();
}

TEST_F(TranslatorFixture, translator_multiinput)
{
    auto broker = AddBroker("test", 1);

    AddFederates<helics::CombinationFederate>("test", 1, broker, helics::timeZero, "A");

    auto cFed1 = GetFederateAs<helics::CombinationFederate>(0);

    auto& endpoint1 = cFed1->registerGlobalTargetedEndpoint("e1", "any");
    auto& input1 = cFed1->registerGlobalInput<double>("i1");
    auto& input2 = cFed1->registerGlobalInput<double>("i2");
    auto& pub1 = cFed1->registerGlobalPublication<double>("p1");
    pub1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    endpoint1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    input1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);

    endpoint1.addSourceTarget("t1");
    pub1.addDestinationTarget("t1");
    input1.addSourceTarget("t1");
    input2.addSourceTarget("t1");
    endpoint1.addDestinationTarget("t1");

    cFed1->registerGlobalTranslator(helics::TranslatorTypes::JSON, "t1");

    EXPECT_NO_THROW(cFed1->enterExecutingMode());

    auto data = helics::typeConvert(helics::DataType::HELICS_JSON, 20.7);
    std::string val;
    helics::valueExtract(data, helics::DataType::HELICS_JSON, val);
    endpoint1.send(val);
    auto tres = cFed1->requestTime(2.0);
    EXPECT_LT(tres, 2.0);
    EXPECT_TRUE(input1.isUpdated());
    EXPECT_DOUBLE_EQ(input1.getValue<double>(), 20.7);

    EXPECT_TRUE(input2.isUpdated());
    EXPECT_DOUBLE_EQ(input2.getValue<double>(), 20.7);
    cFed1->finalize();

    FullDisconnect();
}

TEST_F(TranslatorFixture, translator_config)
{
    auto broker = AddBroker("test", 1);

    auto cFed1 = std::make_shared<helics::CombinationFederate>(std::string(TEST_DIR) +
                                                               "ControllerConfig.json");

    EXPECT_NO_THROW(cFed1->enterExecutingMode());

    EXPECT_EQ(cFed1->getTranslatorCount(), 5U);
    EXPECT_EQ(cFed1->getInputCount(), 5U);

    auto& trans1 = cFed1->getTranslator(0);

    EXPECT_EQ(trans1.getName(), "EV_1_translator");

    cFed1->query("broker", "global_flush");
    auto dflow = cFed1->query("broker", "data_flow_graph");
    auto fnd = dflow.find("EV_2_translator");
    EXPECT_NE(fnd, std::string::npos);
    cFed1->finalize();

    FullDisconnect();
}

TEST_F(TranslatorFixture, translator_config_json)
{
    auto broker = AddBroker("test", 1);

    auto cFed1 = std::make_shared<helics::CombinationFederate>(std::string(TEST_DIR) +
                                                               "example_translators.json");

    EXPECT_NO_THROW(cFed1->enterExecutingMode());

    EXPECT_EQ(cFed1->getTranslatorCount(), 5U);
    EXPECT_EQ(cFed1->getInputCount(), 5U);

    auto& trans1 = cFed1->getTranslator(0);

    EXPECT_EQ(trans1.getName(), "EV_1_translator");

    cFed1->query("broker", "global_flush");
    auto dflow = cFed1->query("broker", "data_flow_graph");
    auto fnd = dflow.find("EV_2_translator");
    EXPECT_NE(fnd, std::string::npos);
    cFed1->finalize();

    FullDisconnect();
}

TEST_F(TranslatorFixture, translator_config_toml)
{
    auto broker = AddBroker("test", 1);

    auto cFed1 = std::make_shared<helics::CombinationFederate>(std::string(TEST_DIR) +
                                                               "example_translators.toml");

    EXPECT_NO_THROW(cFed1->enterExecutingMode());

    EXPECT_EQ(cFed1->getTranslatorCount(), 5U);
    EXPECT_EQ(cFed1->getInputCount(), 5U);

    auto& trans1 = cFed1->getTranslator(0);

    EXPECT_EQ(trans1.getName(), "EV_1_translator");

    cFed1->query("broker", "global_flush");
    auto dflow = cFed1->query("broker", "data_flow_graph");
    auto fnd = dflow.find("EV_2_translator");
    EXPECT_NE(fnd, std::string::npos);
    cFed1->finalize();

    FullDisconnect();
}
