/*
Copyright (c) 2017-2024,
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

    auto delay = helics::delayMessages(mFed1.get(), GetParam(), 500);

    mFed1->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    helics::SmallBuffer data1(500, 'a');

    epid.send(data1);
    // move the time to 1.0
    mFed1->requestTimeAsync(1.0);
    auto gtime = mFed2->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(mFed1->requestTimeComplete(), 1.0);

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
    delay.wait();
}

static const auto testNamer = [](const ::testing::TestParamInfo<int>& parameter) {
    return std::to_string(parameter.param);
};

INSTANTIATE_TEST_SUITE_P(sequencing_tests,
                         sequencing1,
                         ::testing::ValuesIn({5, 6, 7, 8, 9}),
                         testNamer);

class sequencing2: public ::testing::TestWithParam<int>, public FederateTestFixture {};

static bool two_stage_filter_test(std::shared_ptr<helics::MessageFederate>& mFed,
                                  std::shared_ptr<helics::MessageFederate>& fFed1,
                                  std::shared_ptr<helics::MessageFederate>& fFed2,
                                  helics::Endpoint& p1,
                                  helics::Endpoint& p2,
                                  helics::Filter& f1,
                                  helics::Filter& f2)
{
    bool correct = true;

    auto timeOperator = std::make_shared<helics::MessageTimeOperator>();
    timeOperator->setTimeFunction([](helics::Time time_in) { return time_in + 1.25; });
    fFed1->setFilterOperator(f1, timeOperator);
    fFed2->setFilterOperator(f2, timeOperator);

    fFed1->enterExecutingModeAsync();
    fFed2->enterExecutingModeAsync();
    mFed->enterExecutingMode();
    fFed1->enterExecutingModeComplete();
    fFed2->enterExecutingModeComplete();

    auto& p2Name = p2.getName();
    EXPECT_TRUE(fFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    helics::SmallBuffer data(500, 'a');
    p1.sendTo(data, p2Name);

    mFed->requestTimeAsync(1.0);
    fFed1->requestTimeAsync(1.0);
    fFed2->requestTime(1.0);
    mFed->requestTimeComplete();
    fFed1->requestTimeComplete();
    auto res = mFed->hasMessage();
    EXPECT_TRUE(!res);
    if (res) {
        correct = false;
    }

    mFed->requestTimeAsync(2.0);
    fFed2->requestTimeAsync(2.0);
    fFed1->requestTime(2.0);
    mFed->requestTimeComplete();
    fFed2->requestTimeComplete();
    if (mFed->hasMessage(p2)) {
        correct = false;
    }

    fFed1->requestTimeAsync(3.0);
    fFed2->requestTimeAsync(3.0);
    /*auto retTime = */ mFed->requestTime(3.0);
    if (!mFed->hasMessage(p2)) {
        printf("missing message\n");
        correct = false;
    }
    if (mFed->hasMessage(p2)) {
        auto m2 = mFed->getMessage(p2);
        const auto& ept1Name = p1.getName();
        if (ept1Name.size() > 1) {
            EXPECT_EQ(m2->source, p1.getName());
            EXPECT_EQ(m2->original_source, p1.getName());
        }

        EXPECT_EQ(m2->dest, p2Name);
        EXPECT_EQ(m2->data.size(), data.size());
        EXPECT_EQ(m2->time, 2.5);
    }

    fFed1->requestTimeComplete();
    fFed2->requestTimeComplete();
    auto filterCore = fFed1->getCorePointer();
    auto mCore = mFed->getCorePointer();
    mFed->finalizeAsync();
    fFed1->finalizeAsync();
    fFed2->finalize();
    mFed->finalizeComplete();
    fFed1->finalizeComplete();
    EXPECT_TRUE(fFed1->getCurrentMode() == helics::Federate::Modes::FINALIZE);
    if (fFed1->getCurrentMode() != helics::Federate::Modes::FINALIZE) {
        correct = false;
    }
    helics::cleanupHelicsLibrary();
    EXPECT_TRUE(!mCore->isConnected());
    if (filterCore->isConnected()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    EXPECT_TRUE(!filterCore->isConnected());

    return correct;
}

TEST_P(sequencing2, message_filter_function_two_stage)
{
    extraBrokerArgs = " --maxcosimduration=15000 ";
    auto broker = AddBroker("test", 3);
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "filter2");
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto fFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto mFed = GetFederateAs<helics::MessageFederate>(2);

    auto& p1 = mFed->registerGlobalEndpoint("port1");
    auto& p2 = mFed->registerGlobalEndpoint("port2");

    auto& f1 = fFed->registerFilter("filter1");
    f1.addSourceTarget("port1");
    EXPECT_TRUE(f1.getHandle().isValid());

    auto& f2 = fFed2->registerFilter("filter2");
    f2.addSourceTarget("port1");
    EXPECT_TRUE(f2.getHandle().isValid());

    fFed->setProperty(HELICS_PROPERTY_TIME_GRANT_TIMEOUT, 1.0);
    fFed2->setProperty(HELICS_PROPERTY_TIME_GRANT_TIMEOUT, 1.0);
    mFed->setProperty(HELICS_PROPERTY_TIME_GRANT_TIMEOUT, 1.0);
    auto delay = helics::delayMessages(fFed2.get(), GetParam(), 500);
    bool res = two_stage_filter_test(mFed, fFed, fFed2, p1, p2, f1, f2);
    EXPECT_TRUE(res);
    delay.wait();
}

INSTANTIATE_TEST_SUITE_P(sequencing_tests, sequencing2, ::testing::Range(5, 15), testNamer);

class sequencing3: public ::testing::TestWithParam<int>, public FederateTestFixture {};

static constexpr auto rerouteType = "test";

TEST_P(sequencing3, reroute_separate2)
{
    extraBrokerArgs = " --globaltime --debugging";
    extraCoreArgs = " --debugging ";
    auto broker = AddBroker(rerouteType, 3);
    AddFederates<helics::MessageFederate>(rerouteType, 1, broker);
    AddFederates<helics::MessageFederate>(rerouteType, 1, broker);
    AddFederates<helics::MessageFederate>(rerouteType, 1, broker);

    auto send = GetFederateAs<helics::MessageFederate>(0);
    auto rec = GetFederateAs<helics::MessageFederate>(1);
    auto filt = GetFederateAs<helics::MessageFederate>(2);

    auto& p1 = send->registerGlobalEndpoint("send");
    auto& p2 = rec->registerGlobalEndpoint("rec");
    p1.setDefaultDestination("rec");
    auto& p3 = filt->registerGlobalEndpoint("reroute");

    auto& f1 = helics::make_filter(helics::FilterTypes::REROUTE, filt.get(), "rrfilt");

    f1.addSourceTarget("send");
    f1.setString("newdestination", "reroute");
    auto delay = helics::delayMessages(filt.get(), GetParam(), 900);
    std::vector<helics::Time> tm;
    auto act1 = [&p1, &send, &tm]() {
        send->enterExecutingMode();
        helics::Time tr = helics::timeZero;
        while (tr < 10.0) {
            p1.send("this is a message");
            tr = send->requestTimeAdvance(1.0);
            tm.emplace_back(tr);
        }
        send->finalize();
    };

    auto act2 = [&rec]() {
        rec->enterExecutingMode();
        helics::Time tr = helics::timeZero;
        while (tr < 10.0) {
            tr = rec->requestTimeAdvance(1.0);
        }
        rec->finalize();
    };

    int cnt{0};
    std::vector<std::pair<helics::Time, int>> res;
    auto act3 = [&filt, &cnt, &res, &p3]() {
        filt->enterExecutingMode();
        helics::Time tr = helics::timeZero;
        while (tr < 20.0) {
            tr = filt->requestTime(helics::Time::maxVal());
            ++cnt;
            res.emplace_back(tr, p3.pendingMessageCount());
        }
    };

    auto t1 = std::thread(act1);
    auto t2 = std::thread(act2);
    auto t3 = std::thread(act3);

    t1.join();
    t2.join();

    // std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    // auto res = broker->query("root", "global_time_debugging");
    t3.join();
    filt->finalize();
    EXPECT_EQ(p2.pendingMessageCount(), 0U);
    EXPECT_EQ(p3.pendingMessageCount(), 10U);
    EXPECT_EQ(cnt, 11);
    // auto res2 = broker->query("root", "global_time_debugging");
    broker->waitForDisconnect();
}

INSTANTIATE_TEST_SUITE_P(sequencing_tests, sequencing3, ::testing::Range(26, 28), testNamer);
