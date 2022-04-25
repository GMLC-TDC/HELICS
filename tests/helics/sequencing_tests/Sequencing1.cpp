/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/BrokerApp.hpp"
#include "helics/application_api/CombinationFederate.hpp"
#include "helics/application_api/CoreApp.hpp"
#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/MessageFederate.hpp"
#include "helics/core/Core.hpp"
#include "sequencingHelpers.hpp"
#include <gtest/gtest.h>

#include "../application_api/testFixtures.hpp"


#include <future>
#include <iostream>
#include <thread>
/** these test cases test out the message federates
 */


class sequencing1: public ::testing::TestWithParam<int>, public FederateTestFixture {};



TEST_P(sequencing1, send_receive_2fed_multisend)
{
    SetupTest<helics::MessageFederate>("test_3", 2);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    auto epid = mFed1->registerEndpoint("ep1");
    auto epid2 = mFed2->registerGlobalEndpoint("ep2", "random");
    // mFed1->getCorePointer()->setLoggingLevel(0, 5);
    mFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    mFed2->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    epid.setDefaultDestination("ep2");

    auto delay=helics::delayMessages(mFed1.get(), GetParam(), 500);
    
    auto f1finish = std::async(std::launch::async, [&]() { mFed1->enterExecutingMode(); });
    mFed2->enterExecutingMode();
    f1finish.wait();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    helics::SmallBuffer data1(500, 'a');

    epid.send(data1);
    // move the time to 1.0
    auto f1time = std::async(std::launch::async, [&]() { return mFed1->requestTime(1.0); });
    auto gtime = mFed2->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(f1time.get(), 1.0);

    EXPECT_TRUE(!mFed1->hasMessage());

    EXPECT_TRUE(!mFed1->hasMessage(epid));
    auto cnt = mFed2->pendingMessageCount(epid2);
    EXPECT_EQ(cnt, 1);

    auto M1 = mFed2->getMessage(epid2);
    ASSERT_TRUE(M1);
    ASSERT_EQ(M1->data.size(), data1.size());

    EXPECT_EQ(M1->data[245], data1[245]);
    
    EXPECT_EQ(M1->time, 0.0);
    mFed1->finalizeAsync();
    mFed2->finalize();
    mFed1->finalizeComplete();
    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::FINALIZE);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

static const auto testNamer = [](const ::testing::TestParamInfo<int>& parameter) {
    return std::to_string(parameter.param);
};

INSTANTIATE_TEST_SUITE_P(sequencing_tests,
                         sequencing1,
                         ::testing::ValuesIn({5,6, 7, 8,9}),
                         testNamer);

class sequencing2: public ::testing::TestWithParam<int>, public FederateTestFixture {
};

TEST_P(sequencing2, time_interruptions)
{
    SetupTest<helics::MessageFederate>("test_3", 2);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    auto epid = mFed1->registerEndpoint("ep1");
    auto epid2 = mFed2->registerGlobalEndpoint("ep2", "random");
    mFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1);
    mFed2->setProperty(HELICS_PROPERTY_TIME_DELTA, 0.5);

     auto delay = helics::delayMessages(brokers[1].get(), GetParam(), 500);
    auto f1finish = std::async(std::launch::async, [&]() { mFed1->enterExecutingMode(); });
    mFed2->enterExecutingMode();
    f1finish.wait();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    helics::SmallBuffer data(500, 'a');
    helics::SmallBuffer data2(400, 'b');

    epid.sendTo(data, "ep2");
    epid2.sendTo(data2, "fed0/ep1");
    // move the time to 1.0
    auto f1time = std::async(std::launch::async, [&]() { return mFed1->requestTime(1.0); });
    auto gtime = mFed2->requestTime(1.0);

    ASSERT_EQ(gtime, 0.5);

    ASSERT_TRUE(mFed2->hasMessage(epid2));

    auto M2 = mFed2->getMessage(epid2);
    ASSERT_EQ(M2->data.size(), data.size());

    EXPECT_EQ(M2->data[245], data[245]);

    gtime = mFed2->requestTime(1.0);
    EXPECT_EQ(gtime, 1.0);

    EXPECT_EQ(f1time.get(), 1.0);
    auto M1 = mFed1->getMessage(epid);
    EXPECT_TRUE(M1);
    if (M1) {
        EXPECT_EQ(M1->data.size(), data2.size());
        if (M1->data.size() > 245) {
            EXPECT_EQ(M1->data[245], data2[245]);
        }
    }

    EXPECT_TRUE(mFed1->hasMessage() == false);
    mFed1->finalizeAsync();

    gtime = mFed2->requestTime(2.0);
    EXPECT_EQ(gtime, 2.0);
    mFed2->finalize();
    mFed1->finalizeComplete();
    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::FINALIZE);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

INSTANTIATE_TEST_SUITE_P(sequencing_tests,
                         sequencing2,
                         ::testing::Range(12,25),
                         testNamer);
