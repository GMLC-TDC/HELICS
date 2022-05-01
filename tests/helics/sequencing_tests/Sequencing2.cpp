/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../application_api/testFixtures.hpp"
#include "helics/application_api.hpp"
#include "helics/application_api/BrokerApp.hpp"
#include "helics/application_api/CombinationFederate.hpp"
#include "helics/application_api/CoreApp.hpp"
#include "helics/core/CommonCore.hpp"
#include "helics/core/Core.hpp"
#include "sequencingHelpers.hpp"

#include <future>
#include <gtest/gtest.h>
#include <iostream>
#include <thread>
/** these test cases test out the message federates
 */

class sequencing_interruptions: public ::testing::TestWithParam<int>, public FederateTestFixture {};

TEST_P(sequencing_interruptions, time_interruptions)
{
    SetupTest<helics::MessageFederate>("test_3", 2);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    auto epid = mFed1->registerEndpoint("ep1");
    auto epid2 = mFed2->registerGlobalEndpoint("ep2", "random");
    mFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1);
    mFed2->setProperty(HELICS_PROPERTY_TIME_DELTA, 0.5);

    auto delay = helics::delayMessages(mFed2.get(), GetParam(), 500);
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
    delay.get();
}

static const auto testNamer = [](const ::testing::TestParamInfo<int>& parameter) {
    return std::to_string(parameter.param);
};

INSTANTIATE_TEST_SUITE_P(sequencing_tests,
                         sequencing_interruptions,
                         ::testing::Range(25, 80),
                         testNamer);

class sequencing_reroute: public ::testing::TestWithParam<int>, public FederateTestFixture {};

TEST_P(sequencing_reroute, separate_dest)
{
    static constexpr char* rerouteType = "test_3";
    extraBrokerArgs = " --debugging ";
    auto broker = AddBroker(rerouteType, 3);
    extraCoreArgs = " --debugging ";

    AddFederates<helics::MessageFederate>(rerouteType, 1, broker, 1.0, "sender");
    AddFederates<helics::MessageFederate>(rerouteType, 1, broker, 1.0, "receiver");
    AddFederates<helics::MessageFederate>(rerouteType, 1, broker, 1.0, "filter");

    auto send = GetFederateAs<helics::MessageFederate>(0);
    auto rec = GetFederateAs<helics::MessageFederate>(1);
    auto filt = GetFederateAs<helics::MessageFederate>(2);
    send->setProperty(HELICS_PROPERTY_TIME_GRANT_TIMEOUT, 1.0);
    rec->setProperty(HELICS_PROPERTY_TIME_GRANT_TIMEOUT, 1.0);
    filt->setProperty(HELICS_PROPERTY_TIME_GRANT_TIMEOUT, 1.0);
    auto& p1 = send->registerGlobalEndpoint("send");
    auto& p2 = rec->registerGlobalEndpoint("rec");
    p1.setDefaultDestination("rec");
    auto& p3 = filt->registerGlobalEndpoint("reroute");

    auto& f1 = helics::make_filter(helics::FilterTypes::REROUTE, filt.get(), "rrfilt");

    f1.addDestinationTarget("rec");
    f1.setString("newdestination", "reroute");

    auto act1 = [&p1, &send]() {
        send->enterExecutingMode();
        helics::Time tr = helics::timeZero;
        while (tr < 10.0) {
            p1.send("this is a message");
            tr = send->requestTimeAdvance(1.0);
        }
        send->finalize();
    };
    int cntb{0};
    auto act2 = [&rec, &cntb]() {
        rec->enterExecutingMode();
        helics::Time tr = helics::timeZero;
        while (tr < 10.0) {
            tr = rec->requestTimeAdvance(1.0);
            if (rec->hasMessage()) {
                ++cntb;
            }
        }
        rec->finalize();
    };
    auto delay = helics::delayMessages(send.get(), GetParam(), 500);
    auto t1 = std::thread(act1);
    auto t2 = std::thread(act2);
    int cnt{0};
    filt->enterExecutingMode();
    helics::Time tr = helics::timeZero;
    std::vector<std::pair<helics::Time, int>> tv;
    tv.reserve(12);
    while (tr < 20.0) {
        tr = filt->requestTime(21.0);

        ++cnt;
        tv.emplace_back(tr, p3.pendingMessageCount());
    }
    t1.join();
    t2.join();
    EXPECT_EQ(p2.pendingMessageCount(), 0U);
    EXPECT_EQ(p3.pendingMessageCount(), 10U);
    EXPECT_EQ(cnt, 10);
    EXPECT_EQ(cntb, 0);
    if (cnt != 10) {
        EXPECT_EQ(cnt, 10);
    }
    filt->finalize();
    delay.get();
}

INSTANTIATE_TEST_SUITE_P(sequencing_test, sequencing_reroute, ::testing::Range(15, 40), testNamer);
