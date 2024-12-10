/*
Copyright (c) 2017-2024,
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
#include <memory>
#include <string>

/** these test cases test out the value converters
 */
#include "../application_api/testFixtures.hpp"
#include "helics/helics-config.h"
#include "helics/helics.hpp"

struct dynFed: public FederateTestFixture, public ::testing::Test {};

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

    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.observer = true;
    // now we try to join the federation with an observer
    auto bname = brokers[0]->getIdentifier();
    auto cdyn = helics::CoreFactory::create(helics::CoreType::TEST,
                                            "coredyn",
                                            std::string("--observer --broker=") + bname);

    EXPECT_TRUE(cdyn->connect());
    fedInfo.coreName = "coredyn";

    std::shared_ptr<helics::ValueFederate> fobs;
    EXPECT_NO_THROW(fobs = std::make_shared<helics::ValueFederate>("fedObs", fedInfo));

    if (fobs) {
        EXPECT_NO_THROW(fobs->enterInitializingMode());

        EXPECT_NO_THROW(fobs->enterExecutingMode());

        EXPECT_EQ(fobs->getCurrentTime(), helics::timeZero);
        fobs->disconnect();
    }

    cdyn->disconnect();

    vFed1->finalize();
    vFed2->finalize();
}

/** just a check that in the simple case we do actually get the time back we requested*/
TEST_F(dynFed, observer_subscriber)
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

    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.observer = true;

    auto bname = brokers[0]->getIdentifier();
    auto cdyn = helics::CoreFactory::create(helics::CoreType::TEST,
                                            "coredyn",
                                            std::string("--observer --broker=") + bname);

    EXPECT_TRUE(cdyn->connect());
    fedInfo.coreName = "coredyn";

    std::shared_ptr<helics::ValueFederate> fobs;
    EXPECT_NO_THROW(fobs = std::make_shared<helics::ValueFederate>("fedObs", fedInfo));
    // now we try to join the federation with an observer
    if (fobs) {
        fobs->registerSubscription("pub1");
        fobs->query("root", "global_flush");

        EXPECT_NO_THROW(fobs->enterInitializingMode());

        EXPECT_NO_THROW(fobs->enterExecutingMode());
        // should be a 1 time delta(1 ns) before 2.0
        EXPECT_NEAR(static_cast<double>(fobs->getCurrentTime()),
                    static_cast<double>(helics::Time(2.0)),
                    0.000000002);
        fobs->disconnect();
    }

    cdyn->disconnect();

    vFed1->finalize();
    vFed2->finalize();
}

/** now make sure we can get the values properly*/
TEST_F(dynFed, observer_subscriber_value)
{
    SetupTest<helics::ValueFederate>("test_2", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    vFed1->setProperty(HELICS_PROPERTY_TIME_PERIOD, 0.5);
    vFed2->setProperty(HELICS_PROPERTY_TIME_PERIOD, 0.5);
    auto& pub = vFed1->registerGlobalPublication<double>("pub1");
    auto& sub = vFed2->registerSubscription("pub1");
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

    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.observer = true;

    auto bname = brokers[0]->getIdentifier();
    auto cdyn = helics::CoreFactory::create(helics::CoreType::TEST,
                                            "coredyn",
                                            std::string("--observer --broker=") + bname);

    EXPECT_TRUE(cdyn->connect());
    fedInfo.coreName = "coredyn";
    // now we try to join the federation with an observer
    std::shared_ptr<helics::ValueFederate> fobs;
    EXPECT_NO_THROW(fobs = std::make_shared<helics::ValueFederate>("fedObs", fedInfo));

    if (!fobs) {
        cdyn->disconnect();

        vFed1->finalize();
        vFed2->finalize();
        ASSERT_TRUE(fobs);
        return;
    }
    auto& obsSubs = fobs->registerSubscription("pub1");
    fobs->query("root", "global_flush");

    EXPECT_NO_THROW(fobs->enterInitializingMode());

    EXPECT_NO_THROW(fobs->enterExecutingMode());
    res = vFed1->requestTime(3.0);
    EXPECT_EQ(res, 3.0);
    pub.publish(0.8987);
    res = vFed1->requestTime(4.0);
    EXPECT_EQ(res, 4.0);

    res = vFed2->requestTime(4.0);
    EXPECT_EQ(res, 3.0);

    EXPECT_DOUBLE_EQ(sub.getValue<double>(), 0.8987);

    res = fobs->requestTime(4.0);
    EXPECT_EQ(res, 3.0);
    EXPECT_DOUBLE_EQ(obsSubs.getValue<double>(), 0.8987);

    fobs->finalize();
    cdyn->disconnect();

    vFed1->finalize();
    vFed2->finalize();
}

/** now make sure we can get the values properly*/
TEST_F(dynFed, observer_subscriber_value_with_buffer)
{
    SetupTest<helics::ValueFederate>("test_2", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    vFed1->setProperty(HELICS_PROPERTY_TIME_PERIOD, 0.5);
    vFed2->setProperty(HELICS_PROPERTY_TIME_PERIOD, 0.5);
    auto& pub = vFed1->registerGlobalPublication<double>("pub1");
    pub.setOption(HELICS_HANDLE_OPTION_BUFFER_DATA);
    auto& sub = vFed2->registerSubscription("pub1");
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

    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.observer = true;

    auto bname = brokers[0]->getIdentifier();
    auto cdyn = helics::CoreFactory::create(helics::CoreType::TEST,
                                            "coredyn",
                                            std::string("--observer --broker=") + bname);

    EXPECT_TRUE(cdyn->connect());
    fedInfo.coreName = "coredyn";
    // now we try to join the federation with an observer
    std::shared_ptr<helics::ValueFederate> fobs;
    EXPECT_NO_THROW(fobs = std::make_shared<helics::ValueFederate>("fedObs", fedInfo));

    if (!fobs) {
        cdyn->disconnect();

        vFed1->finalize();
        vFed2->finalize();
        ASSERT_TRUE(fobs);
        return;
    }
    auto& obsSubs = fobs->registerSubscription("pub1");
    fobs->query("root", "global_flush");

    EXPECT_NO_THROW(fobs->enterInitializingMode());

    EXPECT_NO_THROW(fobs->enterExecutingMode());
    EXPECT_DOUBLE_EQ(obsSubs.getValue<double>(), 0.27);
    res = vFed1->requestTime(3.0);
    EXPECT_EQ(res, 3.0);
    pub.publish(0.8987);
    res = vFed1->requestTime(4.0);
    EXPECT_EQ(res, 4.0);

    res = vFed2->requestTime(4.0);
    EXPECT_EQ(res, 3.0);

    EXPECT_DOUBLE_EQ(sub.getValue<double>(), 0.8987);

    res = fobs->requestTime(4.0);
    EXPECT_EQ(res, 4.0);
    EXPECT_DOUBLE_EQ(obsSubs.getValue<double>(), 0.8987);

    fobs->finalize();
    cdyn->disconnect();

    vFed1->finalize();
    vFed2->finalize();
}
