/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"

#include "gtest/gtest.h"
#include <complex>
#include <future>
#include <iostream>

/** these test cases test out the value converters
 */
#include "../application_api/testFixtures.hpp"
#include "helics/helics-config.h"
#include "helics/helics.hpp"

struct dynFed: public FederateTestFixture, public ::testing::Test {
};

/** just a check that in the simple case we do actually get the time back we requested*/
TEST_F(dynFed, simple_observer)
{
    SetupTest<helics::ValueFederate>("test_2", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    vFed1->setProperty(HELICS_PROPERTY_TIME_PERIOD, 0.5);
    vFed2->setProperty(HELICS_PROPERTY_TIME_PERIOD, 0.5);
    auto& pub = vFed1->registerGlobalPublication<double>("pub1");
    vFed2->registerSubscription("pub1");
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();
    pub.publish(0.27);
    auto res = vFed1->requestTime(2.0);
    EXPECT_EQ(res, 2.0);

    res = vFed2->requestTime(2.0);
    EXPECT_EQ(res, 0.5);  // the result should show up at the next available time point
    res = vFed2->requestTime(2.0);
    EXPECT_EQ(res, 2.0);

    helics::FederateInfo fi(helics::CoreType::TEST);
    fi.observer = true;

    auto bname = brokers[0]->getIdentifier();
    auto cdyn = helics::CoreFactory::create(helics::CoreType::TEST,"coredyn",
                                            std::string("--observer --broker=") + bname);

    EXPECT_TRUE(cdyn->connect());
    fi.coreName = "coredyn";

    std::shared_ptr<helics::ValueFederate> fobs;
    EXPECT_NO_THROW(fobs=std::make_shared<helics::ValueFederate>("fedObs", fi));

    
    if (fobs)
    {
        EXPECT_NO_THROW(fobs->enterInitializingMode());

        EXPECT_NO_THROW(fobs->enterExecutingMode());

        EXPECT_EQ(fobs->getCurrentTime(), helics::timeZero);
        fobs->disconnect();

    }
    

    cdyn->disconnect();


    // now we try to join the federation with an observer
    vFed1->finalize();
    vFed2->finalize();
}
