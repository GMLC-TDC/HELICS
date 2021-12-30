/*
Copyright (c) 2017-2021,
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
#include "helics/application_api/MessageOperators.hpp"

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

/** test registration of filters*/
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

