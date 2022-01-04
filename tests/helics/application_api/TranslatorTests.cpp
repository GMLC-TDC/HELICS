/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/BrokerApp.hpp"
#include "helics/application_api/CoreApp.hpp"
#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/Federate.hpp"
#include "helics/application_api/Translator.hpp"
#include "helics/application_api/MessageFederate.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/application_api/TranslatorOperations.hpp"

#ifndef HELICS_SHARED_LIBRARY
#    include "helics/core/Broker.hpp"
#    include "testFixtures.hpp"
#else
#    include "testFixtures_shared.hpp"
#endif
#include <future>
#include <gtest/gtest.h>
#include <thread>
/** these test cases test out translator operations
 */

class translator_tests: public ::testing::Test, public FederateTestFixture {
};

/** test registration of translators*/
TEST_F(translator_tests, translator_registration)
{
    auto broker = AddBroker("test", 2);

    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "filter");
    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "message");
    // broker->setLoggingLevel (3);
    broker.reset();

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto mFed = GetFederateAs<helics::MessageFederate>(1);

     auto & t1=mFed->registerGlobalTranslator("t1");
    

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

TEST_F(translator_tests, translator_queries)
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


TEST_F(translator_tests, translator_connections1)
{
    auto broker = AddBroker("test", 2);

    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "A");
    AddFederates<helics::ValueFederate>("test", 1, broker, helics::timeZero, "B");
    // broker->setLoggingLevel (3);

    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::ValueFederate>(1);

    mFed1->registerGlobalTranslator("t1");

    auto & p2=mFed2->registerPublication("p2", "any");
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

TEST_F(translator_tests, translator_connections2)
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

TEST_F(translator_tests, translator_connections3)
{
    auto broker = AddBroker("test", 2);

    AddFederates<helics::MessageFederate>("test", 1, broker, helics::timeZero, "A");
    AddFederates<helics::ValueFederate>("test", 1, broker, helics::timeZero, "B");
    
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::ValueFederate>(1);

    //this is to prevent it from printing some expected errors in this test
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
    EXPECT_THROW(mFed2->enterExecutingModeComplete(),helics::ConnectionFailure);

    mFed2->finalizeAsync();

    mFed1->finalize();
    // std::cout << "fFed returned\n";
    mFed2->finalizeComplete();

    FullDisconnect();
}


TEST_F(translator_tests, translator_connections4)
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

TEST_F(translator_tests, translator_connections5)
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

TEST_F(translator_tests, translator_connections6)
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

TEST_F(translator_tests, translator_connections7)
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

TEST_F(translator_tests, translator_connections8)
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

TEST_F(translator_tests, translator_connections9)
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
