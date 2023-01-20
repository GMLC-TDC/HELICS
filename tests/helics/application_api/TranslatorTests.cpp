/*
Copyright (c) 2017-2023,
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
#include <thread>

/** these test cases test out translator operations
 */

class translator: public ::testing::Test, public FederateTestFixture {};

/** test registration of translators*/
TEST_F(translator, translator_registration)
{
    auto broker = AddBroker("test", 2);

    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "filter");
    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "message");
    // broker->setLoggingLevel (3);
    broker.reset();

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto mFed = GetFederateAs<helics::MessageFederate>(1);

    auto& t1 = mFed->registerGlobalTranslator("t1");

    auto& t2 = fFed->registerTranslator("t2");

    EXPECT_EQ(t1.getName(), "t1");
    EXPECT_EQ(t2.getName(), "filter0/t2");

    EXPECT_EQ(t1.getSourceTargets(), "");
    mFed->finalizeAsync();

    fFed->finalize();
    // std::cout << "fFed returned\n";
    mFed->finalizeComplete();
    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::FINALIZE);
    FullDisconnect();
}

TEST_F(translator, translator_queries)
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

TEST_F(translator, translator_connections1)
{
    auto broker = AddBroker("test", 2);

    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "A");
    AddFederates<helics::ValueFederate>("test", 1, broker, helics::timeZero, "B");
    // broker->setLoggingLevel (3);

    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::ValueFederate>(1);

    mFed1->registerGlobalTranslator("t1");

    auto& p2 = mFed2->registerPublication("p2", "any");
    p2.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    p2.addDestinationTarget("t1");
    mFed2->enterExecutingModeAsync();
    mFed1->enterExecutingMode();
    EXPECT_NO_THROW(mFed2->enterExecutingModeComplete());

    mFed2->finalizeAsync();

    mFed1->finalize();
    // std::cout << "fFed returned\n";
    mFed2->finalizeComplete();

    FullDisconnect();
}

TEST_F(translator, translator_connections2)
{
    auto broker = AddBroker("test", 2);

    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "A");
    AddFederates<helics::ValueFederate>("test", 1, broker, helics::timeZero, "B");
    // broker->setLoggingLevel (3);

    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::ValueFederate>(1);

    auto& p2 = mFed2->registerPublication("p2", "any");
    p2.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    p2.addDestinationTarget("t1");
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

TEST_F(translator, translator_connections3_nosan)
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
    auto& p2 = mFed2->registerPublication("p2", "any");
    p2.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    p2.addDestinationTarget("t2");
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

TEST_F(translator, translator_connections4)
{
    auto broker = AddBroker("test", 2);

    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "A");
    AddFederates<helics::ValueFederate>("test", 1, broker, helics::timeZero, "B");
    // broker->setLoggingLevel (3);

    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::ValueFederate>(1);

    mFed1->registerGlobalTranslator("t1");

    auto& p2 = mFed2->registerInput("p2", "any");
    p2.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    p2.addSourceTarget("t1");
    mFed2->enterInitializingModeAsync();
    mFed1->enterInitializingMode();
    EXPECT_NO_THROW(mFed2->enterInitializingModeComplete());

    mFed2->finalizeAsync();

    mFed1->finalize();
    // std::cout << "fFed returned\n";
    mFed2->finalizeComplete();

    FullDisconnect();
}

TEST_F(translator, translator_connections4_alias)
{
    auto broker = AddBroker("test", 2);
    broker->addAlias("t1", "trans");
    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "A");
    AddFederates<helics::ValueFederate>("test", 1, broker, helics::timeZero, "B");
    // broker->setLoggingLevel (3);

    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::ValueFederate>(1);

    mFed1->registerGlobalTranslator("t1");

    auto& p2 = mFed2->registerInput("p2", "any");
    p2.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    p2.addSourceTarget("trans");
    mFed2->enterInitializingModeAsync();
    mFed1->enterInitializingMode();
    EXPECT_NO_THROW(mFed2->enterInitializingModeComplete());

    mFed2->finalizeAsync();

    mFed1->finalize();
    mFed2->finalizeComplete();

    FullDisconnect();
}

TEST_F(translator, translator_connections5)
{
    auto broker = AddBroker("test", 2);

    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "A");
    AddFederates<helics::ValueFederate>("test", 1, broker, helics::timeZero, "B");
    // broker->setLoggingLevel (3);

    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::ValueFederate>(1);

    auto& p2 = mFed2->registerInput("p2", "any");
    p2.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    p2.addSourceTarget("t1");
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

TEST_F(translator, translator_connections5_alias)
{
    auto broker = AddBroker("test", 2);

    broker->addAlias("t1", "trans");
    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "A");
    AddFederates<helics::ValueFederate>("test", 1, broker, helics::timeZero, "B");
    // broker->setLoggingLevel (3);

    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::ValueFederate>(1);

    auto& p2 = mFed2->registerInput("p2", "any");
    p2.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    p2.addSourceTarget("trans");
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

TEST_F(translator, translator_connections6_nosan)
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
    auto& p2 = mFed2->registerInput("p2", "any");
    p2.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    p2.addSourceTarget("t2");
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

TEST_F(translator, translator_connections7)
{
    auto broker = AddBroker("test", 2);

    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "A");
    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "B");
    // broker->setLoggingLevel (3);

    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    mFed1->registerGlobalTranslator("t1");

    auto& p2 = mFed2->registerTargetedEndpoint("p2", "any");
    p2.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    p2.addSourceTarget("t1");
    mFed2->enterInitializingModeAsync();
    mFed1->enterInitializingMode();
    EXPECT_NO_THROW(mFed2->enterInitializingModeComplete());

    mFed2->finalizeAsync();

    mFed1->finalize();
    // std::cout << "fFed returned\n";
    mFed2->finalizeComplete();

    FullDisconnect();
}

TEST_F(translator, translator_connections8)
{
    auto broker = AddBroker("test", 2);

    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "A");
    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "B");
    // broker->setLoggingLevel (3);

    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    auto& p2 = mFed2->registerTargetedEndpoint("p2", "any");
    p2.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    p2.addSourceTarget("t1");
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

TEST_F(translator, translator_connections9_nosan)
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
    auto& p2 = mFed2->registerTargetedEndpoint("p2", "any");
    p2.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    p2.addSourceTarget("t2");
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

TEST_F(translator, translator_connections10)
{
    auto broker = AddBroker("test", 1);

    AddFederates<helics::CombinationFederate>("test", 1, broker, helics::timeZero, "A");

    auto cFed1 = GetFederateAs<helics::CombinationFederate>(0);

    auto& e1 = cFed1->registerGlobalTargetedEndpoint("e1", "any");
    auto& i1 = cFed1->registerGlobalInput<double>("i1");
    auto& p1 = cFed1->registerGlobalPublication<double>("p1");
    p1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    e1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    i1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);

    e1.addSourceTarget("t1");
    p1.addDestinationTarget("t1");
    i1.addSourceTarget("t1");
    e1.addDestinationTarget("t1");

    cFed1->registerGlobalTranslator("t1");

    EXPECT_NO_THROW(cFed1->enterExecutingMode());

    cFed1->finalize();

    FullDisconnect();
}

TEST_F(translator, translator_time_advance)
{
    auto broker = AddBroker("test", 1);

    AddFederates<helics::CombinationFederate>("test", 1, broker, helics::timeZero, "A");

    auto cFed1 = GetFederateAs<helics::CombinationFederate>(0);

    auto& e1 = cFed1->registerGlobalTargetedEndpoint("e1", "any");
    auto& i1 = cFed1->registerGlobalInput<double>("i1");
    auto& p1 = cFed1->registerGlobalPublication<double>("p1");
    p1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    e1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    i1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);

    e1.addSourceTarget("t1");
    p1.addDestinationTarget("t1");
    i1.addSourceTarget("t1");
    e1.addDestinationTarget("t1");

    cFed1->registerGlobalTranslator("t1");

    EXPECT_NO_THROW(cFed1->enterExecutingMode());

    auto tres = cFed1->requestTime(2.0);
    EXPECT_EQ(tres, 2.0);
    cFed1->finalize();

    FullDisconnect();
}

TEST_F(translator, translator_to_message)
{
    auto broker = AddBroker("test", 1);

    AddFederates<helics::CombinationFederate>("test", 1, broker, helics::timeZero, "A");

    auto cFed1 = GetFederateAs<helics::CombinationFederate>(0);

    auto& e1 = cFed1->registerGlobalTargetedEndpoint("e1", "any");
    auto& i1 = cFed1->registerGlobalInput<double>("i1");
    auto& p1 = cFed1->registerGlobalPublication<double>("p1");
    p1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    e1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    i1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);

    e1.addSourceTarget("t1");
    p1.addDestinationTarget("t1");
    i1.addSourceTarget("t1");
    e1.addDestinationTarget("t1");

    cFed1->registerGlobalTranslator(helics::TranslatorTypes::JSON, "t1");

    EXPECT_NO_THROW(cFed1->enterExecutingMode());

    p1.publish(20.7);
    auto tres = cFed1->requestTime(2.0);
    EXPECT_LT(tres, 2.0);
    EXPECT_TRUE(e1.hasMessage());
    auto m = e1.getMessage();

    cFed1->finalize();
    ASSERT_TRUE(m);

    auto b = helics::fileops::loadJsonStr(m->data.to_string());

    ASSERT_TRUE(b.isMember("value"));
    ASSERT_TRUE(b.isMember("type"));
    EXPECT_DOUBLE_EQ(b["value"].asDouble(), 20.7);
    EXPECT_STREQ(b["type"].asCString(), "double");
    FullDisconnect();
}

TEST_F(translator, translator_to_message_2fed)
{
    auto broker = AddBroker("test", 2);

    AddFederates<helics::CombinationFederate>("test", 1, broker, 1.0, "A");
    AddFederates<helics::CombinationFederate>("test", 1, broker, 1.0, "B");

    auto cFed1 = GetFederateAs<helics::CombinationFederate>(0);
    auto cFed2 = GetFederateAs<helics::CombinationFederate>(1);

    auto& e1 = cFed2->registerGlobalTargetedEndpoint("e1", "any");
    auto& p1 = cFed1->registerGlobalPublication<double>("p1");
    p1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    e1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);

    e1.addSourceEndpoint("t1");
    p1.addDestinationTarget("t1");

    cFed1->registerGlobalTranslator(helics::TranslatorTypes::JSON, "t1");

    cFed1->enterExecutingModeAsync();
    cFed2->enterExecutingMode();
    cFed1->enterExecutingModeComplete();

    p1.publish(20.7);
    auto tres = cFed1->requestTime(2.0);
    EXPECT_EQ(tres, 2.0);
    tres = cFed2->requestTime(3.0);
    EXPECT_EQ(tres, 1.0);
    EXPECT_TRUE(e1.hasMessage());
    auto m = e1.getMessage();

    cFed1->finalize();
    cFed2->finalize();
    ASSERT_TRUE(m);

    auto b = helics::fileops::loadJsonStr(m->data.to_string());

    ASSERT_TRUE(b.isMember("value"));
    ASSERT_TRUE(b.isMember("type"));
    EXPECT_DOUBLE_EQ(b["value"].asDouble(), 20.7);
    EXPECT_STREQ(b["type"].asCString(), "double");
    FullDisconnect();
}

TEST_F(translator, translator_to_message_target_from_translator)
{
    auto broker = AddBroker("test", 1);

    AddFederates<helics::CombinationFederate>("test", 1, broker, helics::timeZero, "A");

    auto cFed1 = GetFederateAs<helics::CombinationFederate>(0);

    auto& e1 = cFed1->registerGlobalTargetedEndpoint("e1", "any");
    auto& p1 = cFed1->registerGlobalPublication<double>("p1");

    auto& t1 = cFed1->registerGlobalTranslator(helics::TranslatorTypes::JSON, "t1");
    t1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);

    t1.addSourceEndpoint("e1");
    t1.addDestinationEndpoint("e1");
    t1.addPublication("p1");

    EXPECT_NO_THROW(cFed1->enterExecutingMode());

    p1.publish(20.7);
    auto tres = cFed1->requestTime(2.0);
    EXPECT_LT(tres, 2.0);
    EXPECT_TRUE(e1.hasMessage());
    auto m = e1.getMessage();

    cFed1->finalize();
    ASSERT_TRUE(m);

    auto b = helics::fileops::loadJsonStr(m->data.to_string());

    ASSERT_TRUE(b.isMember("value"));
    ASSERT_TRUE(b.isMember("type"));
    EXPECT_DOUBLE_EQ(b["value"].asDouble(), 20.7);
    EXPECT_STREQ(b["type"].asCString(), "double");
    FullDisconnect();
}

TEST_F(translator, translator_round_trip)
{
    auto broker = AddBroker("test", 1);

    AddFederates<helics::CombinationFederate>("test", 1, broker, helics::timeZero, "A");

    auto cFed1 = GetFederateAs<helics::CombinationFederate>(0);

    auto& e1 = cFed1->registerGlobalTargetedEndpoint("e1", "any");
    auto& i1 = cFed1->registerGlobalInput<double>("i1");
    auto& p1 = cFed1->registerGlobalPublication<double>("p1");
    p1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    e1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    i1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);

    e1.addSourceEndpoint("t1");
    p1.addInputTarget("t1");
    i1.addPublication("t1");
    e1.addDestinationEndpoint("t1");

    cFed1->registerGlobalTranslator(helics::TranslatorTypes::JSON, "t1");

    EXPECT_NO_THROW(cFed1->enterExecutingMode());

    p1.publish(20.7);
    auto tres = cFed1->requestTime(2.0);
    EXPECT_LT(tres, 2.0);
    EXPECT_TRUE(e1.hasMessage());
    auto m = e1.getMessage();

    ASSERT_TRUE(m);
    m->dest.clear();
    e1.send(std::move(m));
    auto tres2 = cFed1->requestTime(2.0);
    EXPECT_LT(tres2, 2.0);
    EXPECT_GT(tres2, tres);
    EXPECT_FALSE(e1.hasMessage());
    EXPECT_TRUE(i1.isUpdated());
    EXPECT_DOUBLE_EQ(i1.getDouble(), 20.7);
    cFed1->finalize();
    FullDisconnect();
}

TEST_F(translator, translator_round_trip_target_from_translator)
{
    auto broker = AddBroker("test", 1);

    AddFederates<helics::CombinationFederate>("test", 1, broker, helics::timeZero, "A");

    auto cFed1 = GetFederateAs<helics::CombinationFederate>(0);

    auto& e1 = cFed1->registerGlobalTargetedEndpoint("e1", "any");
    auto& i1 = cFed1->registerGlobalInput<double>("i1");
    auto& p1 = cFed1->registerGlobalPublication<double>("p1");

    auto& t1 = cFed1->registerGlobalTranslator(helics::TranslatorTypes::JSON, "t1");
    t1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    t1.addSourceEndpoint("e1");
    t1.addDestinationEndpoint("e1");
    t1.addPublication("p1");
    t1.addInputTarget("i1");

    EXPECT_NO_THROW(cFed1->enterExecutingMode());

    p1.publish(20.7);
    auto tres = cFed1->requestTime(2.0);
    EXPECT_LT(tres, 2.0);
    EXPECT_TRUE(e1.hasMessage());
    auto m = e1.getMessage();

    ASSERT_TRUE(m);
    m->dest.clear();
    e1.send(std::move(m));
    auto tres2 = cFed1->requestTime(2.0);
    EXPECT_LT(tres2, 2.0);
    EXPECT_GT(tres2, tres);
    EXPECT_FALSE(e1.hasMessage());
    EXPECT_TRUE(i1.isUpdated());
    EXPECT_DOUBLE_EQ(i1.getDouble(), 20.7);
    cFed1->finalize();
    FullDisconnect();
}

TEST_F(translator, translator_to_multimessage)
{
    auto broker = AddBroker("test", 1);

    AddFederates<helics::CombinationFederate>("test", 1, broker, helics::timeZero, "A");

    auto cFed1 = GetFederateAs<helics::CombinationFederate>(0);

    auto& e1 = cFed1->registerGlobalTargetedEndpoint("e1", "any");
    auto& e2 = cFed1->registerGlobalTargetedEndpoint("e2", "any");
    auto& i1 = cFed1->registerGlobalInput<double>("i1");
    auto& p1 = cFed1->registerGlobalPublication<double>("p1");
    p1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    e1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    i1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);

    e1.addDestinationEndpoint("t1");
    p1.addInputTarget("t1");
    i1.addPublication("t1");
    e1.addSourceEndpoint("t1");
    e2.addSourceEndpoint("t1");

    cFed1->registerGlobalTranslator(helics::TranslatorTypes::JSON, "t1");

    EXPECT_NO_THROW(cFed1->enterExecutingMode());

    p1.publish(20.7);
    auto tres = cFed1->requestTime(2.0);
    EXPECT_LT(tres, 2.0);
    EXPECT_TRUE(e1.hasMessage());
    auto m = e1.getMessage();

    ASSERT_TRUE(m);

    auto b = helics::fileops::loadJsonStr(m->data.to_string());

    ASSERT_TRUE(b.isMember("value"));
    ASSERT_TRUE(b.isMember("type"));
    EXPECT_DOUBLE_EQ(b["value"].asDouble(), 20.7);
    EXPECT_STREQ(b["type"].asCString(), "double");

    EXPECT_TRUE(e2.hasMessage());
    auto m2 = e2.getMessage();

    cFed1->finalize();
    ASSERT_TRUE(m2);

    auto b2 = helics::fileops::loadJsonStr(m2->data.to_string());

    ASSERT_TRUE(b2.isMember("value"));
    ASSERT_TRUE(b2.isMember("type"));
    EXPECT_DOUBLE_EQ(b2["value"].asDouble(), 20.7);
    EXPECT_STREQ(b2["type"].asCString(), "double");
    FullDisconnect();
}

TEST_F(translator, translator_from_message)
{
    auto broker = AddBroker("test", 1);

    AddFederates<helics::CombinationFederate>("test", 1, broker, helics::timeZero, "A");

    auto cFed1 = GetFederateAs<helics::CombinationFederate>(0);

    auto& e1 = cFed1->registerGlobalTargetedEndpoint("e1", "any");
    auto& i1 = cFed1->registerGlobalInput<double>("i1");
    auto& p1 = cFed1->registerGlobalPublication<double>("p1");
    p1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    e1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    i1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);

    e1.addSourceTarget("t1");
    p1.addDestinationTarget("t1");
    i1.addSourceTarget("t1");
    e1.addDestinationTarget("t1");

    cFed1->registerGlobalTranslator(helics::TranslatorTypes::JSON, "t1");

    EXPECT_NO_THROW(cFed1->enterExecutingMode());

    auto sm = helics::typeConvert(helics::DataType::HELICS_JSON, 20.7);
    std::string val;
    helics::valueExtract(sm, helics::DataType::HELICS_JSON, val);
    e1.send(val);
    auto tres = cFed1->requestTime(2.0);
    EXPECT_LT(tres, 2.0);
    EXPECT_TRUE(i1.isUpdated());
    EXPECT_DOUBLE_EQ(i1.getValue<double>(), 20.7);

    cFed1->finalize();

    FullDisconnect();
}

TEST_F(translator, translator_from_message_2fed)
{
    auto broker = AddBroker("test", 2);

    AddFederates<helics::CombinationFederate>("test", 1, broker, 1.0, "A");
    AddFederates<helics::CombinationFederate>("test", 1, broker, 1.0, "B");
    auto cFed1 = GetFederateAs<helics::CombinationFederate>(0);
    auto cFed2 = GetFederateAs<helics::CombinationFederate>(1);

    auto& e1 = cFed1->registerGlobalTargetedEndpoint("e1", "any");
    auto& i1 = cFed2->registerGlobalInput<double>("i1");

    e1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    i1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);

    i1.addSourceTarget("t1");
    e1.addDestinationTarget("t1");

    cFed1->registerGlobalTranslator(helics::TranslatorTypes::JSON, "t1");

    cFed2->enterExecutingModeAsync();
    EXPECT_NO_THROW(cFed1->enterExecutingMode());
    cFed2->enterExecutingModeComplete();

    auto sm = helics::typeConvert(helics::DataType::HELICS_JSON, 20.7);
    std::string val{sm.to_string()};

    e1.send(val);
    // there should be no dependencies here so it should grant immediately
    auto tres = cFed1->requestTime(3.0);
    EXPECT_EQ(tres, 3.0);

    tres = cFed2->requestTime(3.0);
    EXPECT_EQ(tres, 1.0);
    EXPECT_TRUE(i1.isUpdated());
    EXPECT_DOUBLE_EQ(i1.getValue<double>(), 20.7);

    cFed1->finalize();
    cFed2->finalize();
    FullDisconnect();
}

TEST_F(translator, translator_multiinput)
{
    auto broker = AddBroker("test", 1);

    AddFederates<helics::CombinationFederate>("test", 1, broker, helics::timeZero, "A");

    auto cFed1 = GetFederateAs<helics::CombinationFederate>(0);

    auto& e1 = cFed1->registerGlobalTargetedEndpoint("e1", "any");
    auto& i1 = cFed1->registerGlobalInput<double>("i1");
    auto& i2 = cFed1->registerGlobalInput<double>("i2");
    auto& p1 = cFed1->registerGlobalPublication<double>("p1");
    p1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    e1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);
    i1.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);

    e1.addSourceTarget("t1");
    p1.addDestinationTarget("t1");
    i1.addSourceTarget("t1");
    i2.addSourceTarget("t1");
    e1.addDestinationTarget("t1");

    cFed1->registerGlobalTranslator(helics::TranslatorTypes::JSON, "t1");

    EXPECT_NO_THROW(cFed1->enterExecutingMode());

    auto sm = helics::typeConvert(helics::DataType::HELICS_JSON, 20.7);
    std::string val;
    helics::valueExtract(sm, helics::DataType::HELICS_JSON, val);
    e1.send(val);
    auto tres = cFed1->requestTime(2.0);
    EXPECT_LT(tres, 2.0);
    EXPECT_TRUE(i1.isUpdated());
    EXPECT_DOUBLE_EQ(i1.getValue<double>(), 20.7);

    EXPECT_TRUE(i2.isUpdated());
    EXPECT_DOUBLE_EQ(i2.getValue<double>(), 20.7);
    cFed1->finalize();

    FullDisconnect();
}
