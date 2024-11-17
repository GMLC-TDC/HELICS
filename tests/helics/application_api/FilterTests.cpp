/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/BrokerApp.hpp"
#include "helics/application_api/CoreApp.hpp"
#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/Federate.hpp"
#include "helics/application_api/Filters.hpp"
#include "helics/application_api/MessageFederate.hpp"
#include "helics/application_api/MessageOperators.hpp"

#ifndef HELICS_SHARED_LIBRARY
#    include "helics/core/Broker.hpp"
#    include "testFixtures.hpp"
#else
#    include "testFixtures_shared.hpp"
#endif
#include <cstdio>
#include <future>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

/** these test cases test out the message federates
 */

class filter_single_type_test:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {};

static const auto testNamer = [](const ::testing::TestParamInfo<const char*>& parameter) {
    return std::string(parameter.param);
};
/*
class filter_all_type_test:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {
};
*/

class filter: public ::testing::Test, public FederateTestFixture {};

/** test registration of filters*/
TEST_P(filter_single_type_test, message_filter_registration)
{
    auto broker = AddBroker(GetParam(), 2);

    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, helics::timeZero, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, helics::timeZero, "message");
    // broker->setLoggingLevel (3);
    broker.reset();

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto mFed = GetFederateAs<helics::MessageFederate>(1);

    mFed->registerGlobalEndpoint("port1");
    mFed->registerGlobalEndpoint("port2");

    auto& filt1 = fFed->registerFilter("filter1");
    filt1.addSourceTarget("port1");
    EXPECT_TRUE(filt1.getHandle().isValid());
    auto& filt2 = fFed->registerFilter("filter2");
    filt2.addDestinationTarget("port2");
    EXPECT_TRUE(filt2.getHandle().isValid());
    auto& ept1 = fFed->registerEndpoint("fout");
    EXPECT_TRUE(ept1.getHandle().isValid());

    mFed->finalizeAsync();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    auto& filt3 = fFed->registerCloningFilter();
    filt3.addSourceTarget("filter0/fout");
    filt3.addDestinationTarget("port2");
    EXPECT_TRUE(filt3.getHandle() != filt2.getHandle());

    auto& filt4 = fFed->registerFilter();
    filt4.addSourceTarget("filter0/fout");
    EXPECT_TRUE(filt4.getHandle() != filt3.getHandle());
    fFed->finalize();
    // std::cout << "fFed returned\n";
    mFed->finalizeComplete();
    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::FINALIZE);
    FullDisconnect();
}

/** test a filter operator
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the
simulation
*/
TEST_P(filter_single_type_test, message_filter_function)
{
    auto broker = AddBroker(GetParam(), 2);
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto mFed = GetFederateAs<helics::MessageFederate>(1);

    auto& ept1 = mFed->registerGlobalEndpoint("port1");
    auto& ept2 = mFed->registerGlobalEndpoint("port2");

    auto& filt1 = fFed->registerFilter("filter1");
    filt1.addSourceTarget("port1");
    EXPECT_TRUE(filt1.getHandle().isValid());
    auto timeOperator = std::make_shared<helics::MessageTimeOperator>();
    timeOperator->setTimeFunction([](helics::Time time_in) { return time_in + 2.5; });
    filt1.setOperator(timeOperator);
    fFed->enterExecutingModeAsync();
    mFed->enterExecutingMode();
    fFed->enterExecutingModeComplete();

    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    helics::SmallBuffer data(500, 'a');
    ept1.sendTo(data, "port2");

    mFed->requestTimeAsync(1.0);
    fFed->requestTime(1.0);
    mFed->requestTimeComplete();

    auto res = mFed->hasMessage();
    EXPECT_TRUE(!res);
    if (res) {
        auto message3 = mFed->getMessage(ept2);
        (void)(message3);
    }
    mFed->requestTimeAsync(2.0);
    fFed->requestTime(2.0);
    mFed->requestTimeComplete();
    EXPECT_TRUE(!mFed->hasMessage(ept2));
    if (mFed->hasMessage(ept2)) {
        auto message3 = mFed->getMessage(ept2);
        (void)(message3);
    }
    fFed->requestTimeAsync(3.0);
    auto retTime = mFed->requestTime(3.0);
    EXPECT_EQ(retTime, 3.0);
    ASSERT_TRUE(mFed->hasMessage(ept2));

    auto message2 = mFed->getMessage(ept2);
    EXPECT_EQ(message2->source, "port1");
    EXPECT_EQ(message2->original_source, "port1");
    EXPECT_EQ(message2->dest, "port2");
    EXPECT_EQ(message2->data.size(), data.size());
    EXPECT_EQ(message2->time, 2.5);

    mFed->requestTime(3.0);
    fFed->requestTimeComplete();
    mFed->finalizeAsync();
    fFed->finalize();
    mFed->finalizeComplete();
    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

/** test a filter operator
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the
simulation
*/

TEST_P(filter_single_type_test, message_filter_object)
{
    auto broker = AddBroker(GetParam(), 2);
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto mFed = GetFederateAs<helics::MessageFederate>(1);

    auto& ept1 = mFed->registerGlobalEndpoint("port1");
    auto& ept2 = mFed->registerGlobalEndpoint("port2");

    auto& Filt = helics::make_filter(helics::FilterTypes::DELAY, fFed.get());
    Filt.addSourceTarget("port1");
    Filt.set("delay", 2.5);

    fFed->enterExecutingModeAsync();
    mFed->enterExecutingMode();
    fFed->enterExecutingModeComplete();

    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    helics::SmallBuffer data(500, 'a');
    ept1.sendTo(data, "port2");

    mFed->requestTimeAsync(1.0);
    fFed->requestTime(1.0);
    mFed->requestTimeComplete();

    auto res = mFed->hasMessage();
    EXPECT_TRUE(!res);

    mFed->requestTimeAsync(2.0);
    fFed->requestTime(2.0);
    mFed->requestTimeComplete();
    ASSERT_TRUE(!mFed->hasMessage(ept2));

    fFed->requestTimeAsync(3.0);
    /*auto retTime = */ mFed->requestTime(3.0);

    ASSERT_TRUE(mFed->hasMessage(ept2));

    auto message2 = mFed->getMessage(ept2);
    EXPECT_EQ(message2->source, "port1");
    EXPECT_EQ(message2->original_source, "port1");
    EXPECT_EQ(message2->dest, "port2");
    EXPECT_EQ(message2->data.size(), data.size());
    EXPECT_EQ(message2->time, 2.5);

    mFed->requestTime(3.0);
    fFed->requestTimeComplete();
    mFed->finalizeAsync();
    fFed->finalize();
    mFed->finalizeComplete();
    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

/** test a remove dest filter operator
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the
simulation
*/

TEST_P(filter_single_type_test, message_dest_filter_function)
{
    auto broker = AddBroker(GetParam(), 2);
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto mFed = GetFederateAs<helics::MessageFederate>(1);

    auto& ept1 = mFed->registerGlobalEndpoint("port1");
    auto& ept2 = mFed->registerGlobalEndpoint("port2");

    auto& filt1 = fFed->registerFilter("filter1");
    filt1.addDestinationTarget("port2");
    EXPECT_TRUE(filt1.getHandle().isValid());
    auto timeOperator = std::make_shared<helics::MessageTimeOperator>();
    timeOperator->setTimeFunction([](helics::Time time_in) { return time_in + 2.5; });
    filt1.setOperator(timeOperator);
    fFed->enterExecutingModeAsync();
    mFed->enterExecutingMode();
    fFed->enterExecutingModeComplete();

    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    helics::SmallBuffer data(500, 'a');
    ept1.sendTo(data, "port2");

    mFed->requestTime(1.0);

    auto res = mFed->hasMessage();
    if (res) {
        auto message = mFed->getMessage();
        EXPECT_TRUE(!res);
    }

    mFed->requestTime(2.0);
    ASSERT_TRUE(!mFed->hasMessage(ept2));

    fFed->requestTimeAsync(3.0);
    /*auto retTime = */ mFed->requestTime(3.0);

    ASSERT_TRUE(mFed->hasMessage(ept2));

    auto message2 = mFed->getMessage(ept2);
    EXPECT_EQ(message2->source, "port1");
    EXPECT_EQ(message2->original_source, "port1");
    EXPECT_EQ(message2->dest, "port2");
    EXPECT_EQ(message2->data.size(), data.size());
    EXPECT_EQ(message2->time, 2.5);

    mFed->requestTime(3.0);
    fFed->requestTimeComplete();
    mFed->finalizeAsync();
    fFed->finalize();
    mFed->finalizeComplete();
    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

/** test a remote dest filter operator
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the
simulation
*/

TEST_P(filter_single_type_test, message_source_filter_function)
{
    auto typeParam = GetParam();
    auto broker = AddBroker(typeParam, 2);
    AddFederates<helics::MessageFederate>(typeParam, 2, broker, 0.5, "message");

    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    auto& ept1 = mFed1->registerGlobalEndpoint("port1");
    auto& ept2 = mFed2->registerGlobalEndpoint("port2");

    auto& filt1 = mFed2->registerFilter("filter1");
    filt1.addSourceTarget("port1");
    EXPECT_TRUE(filt1.getHandle().isValid());
    auto timeOperator = std::make_shared<helics::MessageTimeOperator>();
    timeOperator->setTimeFunction([](helics::Time time_in) { return time_in + 2.5; });
    filt1.setOperator(timeOperator);
    mFed1->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();

    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    helics::SmallBuffer data(500, 'a');
    ept1.sendTo(data, "port2");

    mFed1->requestTimeAsync(1.0);
    mFed2->requestTimeAsync(1.0);
    // std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    // auto res1 = broker->query("root", "global_time_debugging");
    // std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    // std::this_thread::yield();
    // auto res2 = broker->query("root", "global_time_debugging");

    mFed1->requestTimeComplete();
    mFed2->requestTimeComplete();
    auto res = mFed2->hasMessage();
    EXPECT_TRUE(!res);

    mFed1->requestTimeAsync(2.0);
    mFed2->requestTime(2.0);
    mFed1->requestTimeComplete();
    ASSERT_TRUE(!mFed2->hasMessage(ept2));

    mFed1->requestTimeAsync(3.0);
    auto retTime = mFed2->requestTime(3.0);

    EXPECT_TRUE(retTime == 2.5);
    ASSERT_TRUE(mFed2->hasMessage(ept2));

    auto message2 = mFed2->getMessage(ept2);

    mFed2->requestTime(3.0);
    mFed1->requestTimeComplete();
    mFed1->finalizeAsync();
    mFed2->finalize();
    mFed1->finalizeComplete();
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

/** test a remove dest filter operator
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the
simulation
*/

TEST_P(filter_single_type_test, message_dest_filter_object)
{
    // debugDiagnostic = true;
    auto broker = AddBroker(GetParam(), 2);
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto mFed = GetFederateAs<helics::MessageFederate>(1);

    auto& ept1 = mFed->registerGlobalEndpoint("port1");
    auto& ept2 = mFed->registerGlobalEndpoint("port2");

    auto filt1 =
        helics::make_filter(helics::FilterTypes::DELAY, fFed->getCorePointer().get(), "filter1");
    filt1->addDestinationTarget("port2");
    filt1->set("delay", 2.5);

    fFed->enterExecutingModeAsync();
    mFed->enterExecutingMode();
    // std::this_thread::sleep_for(std::chrono::milliseconds(300));
    // auto qres = fFed->query("root", "global_time_debugging");
    fFed->enterExecutingModeComplete();

    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    helics::SmallBuffer data(500, 'a');
    ept1.sendTo(data, "port2");

    mFed->requestTimeAsync(1.0);
    fFed->requestTime(1.0);

    mFed->requestTimeComplete();

    auto res = mFed->hasMessage();
    EXPECT_TRUE(!res);

    mFed->requestTimeAsync(2.0);
    fFed->requestTime(2.0);
    mFed->requestTimeComplete();
    ASSERT_TRUE(!mFed->hasMessage(ept2));

    fFed->requestTimeAsync(3.0);
    /*auto retTime = */ mFed->requestTime(3.0);

    ASSERT_TRUE(mFed->hasMessage(ept2));

    auto message2 = mFed->getMessage(ept2);
    EXPECT_EQ(message2->source, "port1");
    EXPECT_EQ(message2->original_source, "port1");
    EXPECT_EQ(message2->dest, "port2");
    EXPECT_EQ(message2->data.size(), data.size());
    EXPECT_EQ(message2->time, 2.5);

    mFed->requestTime(3.0);
    fFed->requestTimeComplete();
    auto filterCore = fFed->getCorePointer();
    auto mCore = mFed->getCorePointer();
    mFed->finalizeAsync();
    fFed->finalize();
    mFed->finalizeComplete();
    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::FINALIZE);
    helics::cleanupHelicsLibrary();
    EXPECT_TRUE(!filterCore->isConnected());
    EXPECT_TRUE(!mCore->isConnected());
}

static bool twoStageFilterTest(std::shared_ptr<helics::MessageFederate>& mFed,
                               std::shared_ptr<helics::MessageFederate>& fFed1,
                               std::shared_ptr<helics::MessageFederate>& fFed2,
                               helics::Endpoint& ept1,
                               helics::Endpoint& ept2,
                               helics::Filter& filt1,
                               helics::Filter& filt2)
{
    bool correct = true;

    auto timeOperator = std::make_shared<helics::MessageTimeOperator>();
    timeOperator->setTimeFunction([](helics::Time time_in) { return time_in + 1.25; });
    filt1.setOperator(timeOperator);
    filt2.setOperator(timeOperator);
    fFed1->enterExecutingModeAsync();
    fFed2->enterExecutingModeAsync();
    mFed->enterExecutingMode();
    fFed1->enterExecutingModeComplete();
    fFed2->enterExecutingModeComplete();

    auto& p2Name = ept2.getName();
    EXPECT_TRUE(fFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    helics::SmallBuffer data(500, 'a');
    ept1.sendTo(data, p2Name);

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
    if (mFed->hasMessage(ept2)) {
        correct = false;
    }

    fFed1->requestTimeAsync(3.0);
    fFed2->requestTimeAsync(3.0);
    /*auto retTime = */ mFed->requestTime(3.0);
    if (!mFed->hasMessage(ept2)) {
        printf("missing message\n");
        correct = false;
    }
    if (mFed->hasMessage(ept2)) {
        auto message2 = mFed->getMessage(ept2);
        const auto& ept1Name = ept1.getName();
        if (ept1Name.size() > 1) {
            EXPECT_EQ(message2->source, ept1.getName());
            EXPECT_EQ(message2->original_source, ept1.getName());
        }

        EXPECT_EQ(message2->dest, p2Name);
        EXPECT_EQ(message2->data.size(), data.size());
        EXPECT_EQ(message2->time, 2.5);
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
/** test a filter operator
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the
simulation
*/

TEST_P(filter_single_type_test, message_filter_function_two_stage)
{
    auto broker = AddBroker(GetParam(), 3);
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter2");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto fFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto mFed = GetFederateAs<helics::MessageFederate>(2);

    auto& ept1 = mFed->registerGlobalEndpoint("port1");
    auto& ept2 = mFed->registerGlobalEndpoint("port2");

    auto& filt1 = fFed->registerFilter("filter1");
    filt1.addSourceTarget("port1");
    EXPECT_TRUE(filt1.getHandle().isValid());

    auto& filt2 = fFed2->registerFilter("filter2");
    filt2.addSourceTarget("port1");
    EXPECT_TRUE(filt2.getHandle().isValid());

    bool res = twoStageFilterTest(mFed, fFed, fFed2, ept1, ept2, filt1, filt2);
    EXPECT_TRUE(res);
}

TEST_P(filter_single_type_test, message_filter_function_two_stage_endpoint_target)
{
    // debugDiagnostic = true;
    auto broker = AddBroker(GetParam(), 3);
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter2");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto fFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto mFed = GetFederateAs<helics::MessageFederate>(2);

    auto& ept1 = mFed->registerEndpoint();
    auto& ept2 = mFed->registerGlobalEndpoint("port2");
    ept1.addSourceFilter("filter1");
    ept1.addSourceFilter("filter2");

    auto& filt1 = fFed->registerGlobalFilter("filter1");

    EXPECT_TRUE(filt1.getHandle().isValid());

    auto& filt2 = fFed2->registerGlobalFilter("filter2");

    EXPECT_TRUE(filt2.getHandle().isValid());

    bool res = twoStageFilterTest(mFed, fFed, fFed2, ept1, ept2, filt1, filt2);
    EXPECT_TRUE(res);
}

TEST_F(filter, message_filter_function_two_stage_endpoint_target_alias)
{
    // debugDiagnostic = true;
    auto broker = AddBroker("test", 3);
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "filter2");
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto fFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto mFed = GetFederateAs<helics::MessageFederate>(2);

    fFed2->addAlias("filter1", "filterA");
    broker->addAlias("filter2", "filterB");
    auto& ept1 = mFed->registerEndpoint();
    auto& ept2 = mFed->registerGlobalEndpoint("port2");
    ept1.addSourceFilter("filterA");
    ept1.addSourceFilter("filterB");

    auto& filt1 = fFed->registerGlobalFilter("filter1");

    EXPECT_TRUE(filt1.getHandle().isValid());

    auto& filt2 = fFed2->registerGlobalFilter("filter2");

    EXPECT_TRUE(filt2.getHandle().isValid());

    bool res = twoStageFilterTest(mFed, fFed, fFed2, ept1, ept2, filt1, filt2);
    EXPECT_TRUE(res);
}

TEST_F(filter, message_filter_function_two_stage_endpoint_target_alias_regex)
{
    // debugDiagnostic = true;
    auto broker = AddBroker("test", 3);
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "filter2");
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto fFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto mFed = GetFederateAs<helics::MessageFederate>(2);

    fFed2->addAlias("ffilt11", "fBBBEA");
    broker->addAlias("ffilt22", "fBBBEB");
    auto& ept1 = mFed->registerEndpoint();
    auto& ept2 = mFed->registerGlobalEndpoint("port2");
    ept1.addSourceFilter("REGEX:fBBBE.");

    auto& filt1 = fFed->registerGlobalFilter("ffilt11");

    EXPECT_TRUE(filt1.getHandle().isValid());

    auto& filt2 = fFed2->registerGlobalFilter("ffilt22");

    EXPECT_TRUE(filt2.getHandle().isValid());

    bool res = twoStageFilterTest(mFed, fFed, fFed2, ept1, ept2, filt1, filt2);
    EXPECT_TRUE(res);
}

TEST_P(filter_single_type_test, message_filter_function_two_stage_endpoint_target_dest)
{
    // debugDiagnostic = true;
    auto broker = AddBroker(GetParam(), 3);
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter2");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto fFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto mFed = GetFederateAs<helics::MessageFederate>(2);

    // nameless endpoint
    auto& ept1 = mFed->registerEndpoint();
    auto& ept2 = mFed->registerGlobalEndpoint("port2");
    ept1.addSourceFilter("filter1");
    ept2.addDestinationFilter("filter2");

    auto& filt1 = fFed->registerGlobalFilter("filter1");

    EXPECT_TRUE(filt1.getHandle().isValid());

    auto& filt2 = fFed2->registerGlobalFilter("filter2");

    EXPECT_TRUE(filt2.getHandle().isValid());

    bool res = twoStageFilterTest(mFed, fFed, fFed2, ept1, ept2, filt1, filt2);
    EXPECT_TRUE(res);
}

TEST_P(filter_single_type_test, message_filter_function_two_stage_broker_filter_link)
{
    auto broker = AddBroker(GetParam(), 3);
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter2");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto fFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto mFed = GetFederateAs<helics::MessageFederate>(2);

    auto& ept1 = mFed->registerGlobalEndpoint("port1");
    auto& ept2 = mFed->registerGlobalEndpoint("port2");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    broker->addSourceFilterToEndpoint("filter1", "port1");
    broker->addDestinationFilterToEndpoint("filter2", "port2");

    auto& filt1 = fFed->registerGlobalFilter("filter1");

    EXPECT_TRUE(filt1.getHandle().isValid());

    auto& filt2 = fFed2->registerGlobalFilter("filter2");

    EXPECT_TRUE(filt2.getHandle().isValid());

    bool res = twoStageFilterTest(mFed, fFed, fFed2, ept1, ept2, filt1, filt2);
    EXPECT_TRUE(res);
}

TEST_F(filter, message_filter_function_two_stage_brokerApp_filter_link)
{
    auto broker = AddBroker("test", 3);
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "filter2");
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto fFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto mFed = GetFederateAs<helics::MessageFederate>(2);

    auto& ept1 = mFed->registerGlobalEndpoint("port1");
    auto& ept2 = mFed->registerGlobalEndpoint("port2");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    helics::BrokerApp brk(broker);
    brk.addSourceFilterToEndpoint("filter1", "port1");
    brk.addDestinationFilterToEndpoint("filter2", "port2");

    auto& filt1 = fFed->registerGlobalFilter("filter1");

    EXPECT_TRUE(filt1.getHandle().isValid());

    auto& filt2 = fFed2->registerGlobalFilter("filter2");

    EXPECT_TRUE(filt2.getHandle().isValid());

    bool res = twoStageFilterTest(mFed, fFed, fFed2, ept1, ept2, filt1, filt2);
    EXPECT_TRUE(res);
}
#ifdef HELICS_ENABLE_ZMQ_CORE
static constexpr std::string_view rerouteType("zmq");
#else
static constexpr std::string_view rerouteType("test");
#endif

TEST_F(filter, reroute_separate)
{
    extraBrokerArgs = " --globaltime";
    auto broker = AddBroker(rerouteType, 3);
    AddFederates<helics::MessageFederate>(rerouteType, 1, broker, 1.0, "sender");
    AddFederates<helics::MessageFederate>(rerouteType, 1, broker, 1.0, "receiver");
    AddFederates<helics::MessageFederate>(rerouteType, 1, broker, 1.0, "filter");

    auto send = GetFederateAs<helics::MessageFederate>(0);
    auto rec = GetFederateAs<helics::MessageFederate>(1);
    auto filt = GetFederateAs<helics::MessageFederate>(2);

    auto& ept1 = send->registerGlobalEndpoint("send");
    auto& ept2 = rec->registerGlobalEndpoint("rec");
    ept1.setDefaultDestination("rec");
    auto& reroute1 = filt->registerGlobalEndpoint("reroute");

    auto& filt1 = helics::make_filter(helics::FilterTypes::REROUTE, filt.get(), "rrfilt");

    filt1.addSourceTarget("send");
    filt1.setString("newdestination", "reroute");

    auto act1 = [&ept1, &send]() {
        send->enterExecutingMode();
        helics::Time requestTime = helics::timeZero;
        while (requestTime < 10.0) {
            ept1.send("this is a message");
            requestTime = send->requestTimeAdvance(1.0);
        }
        send->finalize();
    };
    int cntb{0};
    auto act2 = [&rec, &cntb]() {
        rec->enterExecutingMode();
        helics::Time requestTime = helics::timeZero;
        while (requestTime < 10.0) {
            requestTime = rec->requestTimeAdvance(1.0);
            if (rec->hasMessage()) {
                ++cntb;
            }
        }
        rec->finalize();
    };

    auto thread1 = std::thread(act1);
    auto thread2 = std::thread(act2);
    int cnt{0};
    filt->enterExecutingMode();
    helics::Time requestTime = helics::timeZero;
    helics::Time ptr = helics::timeZero;
    while (requestTime < 20.0) {
        requestTime = filt->requestTime(21.0);
        if (requestTime < 20.0) {
            EXPECT_EQ(requestTime - ptr, 1.0);
            ptr = requestTime;
        }
        ++cnt;
    }
    thread1.join();
    thread2.join();
    EXPECT_EQ(ept2.pendingMessageCount(), 0U);
    EXPECT_EQ(reroute1.pendingMessageCount(), 10U);
    EXPECT_EQ(cnt, 11);
    EXPECT_EQ(cntb, 0);
    filt->finalize();
}

TEST_F(filter, many_filters)
{
    extraBrokerArgs = "--globaltime";
    auto broker = AddBroker("test", 20);
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "sender");
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "receiver");
    AddFederates<helics::MessageFederate>("test_2", 18, broker, 1.0, "filter");

    auto send = GetFederateAs<helics::MessageFederate>(0);
    auto rec = GetFederateAs<helics::MessageFederate>(1);

    auto& ept1 = send->registerGlobalEndpoint("send");
    auto& ept2 = rec->registerGlobalEndpoint("rec");
    ept1.setDefaultDestination("rec");

    std::vector<std::shared_ptr<helics::MessageFederate>> filterFeds;
    for (int ii = 0; ii < 18; ++ii) {
        auto filt = GetFederateAs<helics::MessageFederate>(2 + ii);
        auto& filt1 = filt->registerFilter("filt1");
        auto dataOp = std::make_shared<helics::MessageDataOperator>();
        dataOp->setDataFunction([ii](helics::SmallBuffer& buffer) { buffer.push_back('a' + ii); });
        filt1.setOperator(dataOp);
        filt1.addSourceTarget("send");
        filterFeds.push_back(std::move(filt));
    }

    auto act1 = [&ept1, &send]() {
        send->enterExecutingMode();
        helics::Time requestTime = helics::timeZero;
        while (requestTime < 10.0) {
            ept1.send("this is a message");
            requestTime = send->requestTimeAdvance(1.0);
        }
        send->finalize();
    };
    int cntb{0};
    auto act2 = [&rec, &cntb, &ept2]() {
        rec->enterExecutingMode();
        helics::Time requestTime = helics::timeZero;
        while (requestTime < 10.0) {
            requestTime = rec->requestTimeAdvance(1.0);
            if (ept2.hasMessage()) {
                ++cntb;
                auto message = ept2.getMessage();
                EXPECT_EQ(message->data.size(), 17U + 18U);
            }
        }
        rec->finalize();
    };

    auto thread1 = std::thread(act1);
    auto thread2 = std::thread(act2);

    for (auto& ffed : filterFeds) {
        ffed->enterExecutingModeAsync();
    }
    for (auto& ffed : filterFeds) {
        ffed->enterExecutingModeComplete();
        ffed->requestTimeAsync(50);
    }

    thread1.join();
    thread2.join();
    EXPECT_EQ(ept2.pendingMessageCount(), 0U);

    EXPECT_EQ(cntb, 10);
    for (auto& ffed : filterFeds) {
        ffed->requestTimeComplete();
        ffed->finalize();
    }
}

TEST_F(filter, many_filters_multi)
{
    auto broker = AddBroker("test", 10);
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "sender");
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "receiver");
    AddFederates<helics::MessageFederate>("test_2", 8, broker, 1.0, "filter");

    auto send = GetFederateAs<helics::MessageFederate>(0);
    auto rec = GetFederateAs<helics::MessageFederate>(1);

    auto& ept1 = send->registerGlobalEndpoint("send");
    auto& ept2 = rec->registerGlobalEndpoint("rec");
    ept1.setDefaultDestination("rec");

    std::vector<std::shared_ptr<helics::MessageFederate>> filterFeds;
    for (int ii = 0; ii < 8; ++ii) {
        auto filt = GetFederateAs<helics::MessageFederate>(2 + ii);
        auto& filt1 = filt->registerFilter("filt1");
        auto dataOp = std::make_shared<helics::MessageDataOperator>();
        dataOp->setDataFunction([ii](helics::SmallBuffer& buffer) { buffer.push_back('a' + ii); });
        filt1.setOperator(dataOp);
        filt1.addSourceTarget("send");
        filterFeds.push_back(std::move(filt));
    }

    auto act1 = [&ept1, &send]() {
        send->enterExecutingMode();
        helics::Time requestTime = helics::timeZero;
        while (requestTime < 10.0) {
            ept1.send("this is a message1");
            ept1.send("this is a message2");
            ept1.send("this is a message3");
            ept1.send("this is a message4");
            requestTime = send->requestTimeAdvance(1.0);
        }
        send->finalize();
    };
    int cntb{0};
    auto act2 = [&rec, &cntb, &ept2]() {
        rec->enterExecutingMode();
        helics::Time requestTime = helics::timeZero;
        while (requestTime < 10.0) {
            requestTime = rec->requestTimeAdvance(1.0);
            while (ept2.hasMessage()) {
                ++cntb;
                auto message = ept2.getMessage();
                EXPECT_EQ(message->data.size(), 18U + 8U);
            }
        }
        rec->finalize();
    };

    auto thread1 = std::thread(act1);
    auto thread2 = std::thread(act2);

    for (auto& ffed : filterFeds) {
        ffed->enterExecutingModeAsync();
    }
    for (auto& ffed : filterFeds) {
        ffed->enterExecutingModeComplete();
        ffed->requestTimeAsync(50);
    }

    thread1.join();
    thread2.join();
    EXPECT_EQ(ept2.pendingMessageCount(), 0U);

    EXPECT_EQ(cntb, 40);
    for (auto& ffed : filterFeds) {
        ffed->requestTimeComplete();
        ffed->finalize();
    }
}

TEST_F(filter, reroute_cascade)
{
    extraBrokerArgs = "--globaltime";
    auto broker = AddBroker("test", 10);
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "sender");
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "receiver");
    AddFederates<helics::MessageFederate>("test_2", 8, broker, 1.0, "filter");

    auto send = GetFederateAs<helics::MessageFederate>(0);
    auto rec = GetFederateAs<helics::MessageFederate>(1);

    auto& send1 = send->registerGlobalEndpoint("send");
    auto& rec1 = rec->registerGlobalEndpoint("rec1");
    auto& rec2 = rec->registerGlobalEndpoint("rec2");
    auto& rec3 = rec->registerGlobalEndpoint("rec3");
    auto& rec4 = rec->registerGlobalEndpoint("rec4");
    auto& rec5 = rec->registerGlobalEndpoint("rec5");
    auto& rec6 = rec->registerGlobalEndpoint("rec6");
    auto& rec7 = rec->registerGlobalEndpoint("rec7");
    auto& rec8 = rec->registerGlobalEndpoint("rec8");
    auto& rec9 = rec->registerGlobalEndpoint("rec9");
    send1.setDefaultDestination("rec1");

    std::vector<std::shared_ptr<helics::MessageFederate>> filterFeds;
    std::vector<helics::Filter> filters;
    for (int ii = 0; ii < 8; ++ii) {
        auto filt = GetFederateAs<helics::MessageFederate>(2 + ii);
        auto& filt1 = helics::make_filter(helics::FilterTypes::REROUTE,
                                          filt.get(),
                                          std::string("rrfilt") + std::to_string(ii));
        filt1.addDestinationTarget(std::string("rec") + std::to_string(ii + 1));
        filt1.setString("newdestination", std::string("rec") + std::to_string(ii + 2));
        filters.push_back(filt1);
        filterFeds.push_back(std::move(filt));
    }

    auto act1 = [&send1, &send]() {
        send->enterExecutingMode();
        helics::Time requestTime = helics::timeZero;
        while (requestTime < 10.0) {
            send1.send("this is a message1");
            send1.send("this is a message2");
            send1.send("this is a message3");
            send1.send("this is a message4");
            requestTime = send->requestTimeAdvance(1.0);
        }
        send->finalize();
    };
    int cntb{0};
    auto act2 = [&rec, &cntb, &rec9]() {
        rec->enterExecutingMode();
        helics::Time requestTime = helics::timeZero;
        while (requestTime < 10.0) {
            requestTime = rec->requestTimeAdvance(1.0);
            while (rec9.hasMessage()) {
                ++cntb;
                auto message = rec9.getMessage();
                EXPECT_EQ(message->data.size(), 18U);
            }
        }
        rec->finalize();
    };

    auto thread1 = std::thread(act1);
    auto thread2 = std::thread(act2);

    for (auto& ffed : filterFeds) {
        ffed->enterExecutingModeAsync();
    }
    for (auto& ffed : filterFeds) {
        ffed->enterExecutingModeComplete();
        ffed->requestTimeAsync(50);
    }

    thread1.join();
    thread2.join();
    EXPECT_EQ(rec1.pendingMessageCount(), 0U);
    EXPECT_EQ(rec2.pendingMessageCount(), 0U);
    EXPECT_EQ(rec3.pendingMessageCount(), 0U);
    EXPECT_EQ(rec4.pendingMessageCount(), 0U);
    EXPECT_EQ(rec5.pendingMessageCount(), 0U);
    EXPECT_EQ(rec6.pendingMessageCount(), 0U);
    EXPECT_EQ(rec7.pendingMessageCount(), 0U);
    EXPECT_EQ(rec8.pendingMessageCount(), 0U);

    EXPECT_EQ(cntb, 40);
    for (auto& ffed : filterFeds) {
        ffed->requestTimeComplete();
        ffed->finalize();
    }
}

class RecFilterCheck {
  private:
    std::thread thread;
    std::shared_ptr<helics::MessageFederate> mFed;
    int mIndex{0};

  public:
    int mCnt{0};

    RecFilterCheck() = default;
    RecFilterCheck(std::shared_ptr<helics::MessageFederate> fed, int index):
        mFed(std::move(fed)), mIndex(index)
    {
    }

    void run()
    {
        auto& rec1 = mFed->registerGlobalEndpoint(std::string("rec") + std::to_string(mIndex));
        auto act1 = [this, &rec1]() {
            mFed->enterExecutingMode();
            helics::Time requestTime = helics::timeZero;
            while (requestTime < 10.0) {
                requestTime = mFed->requestTimeAdvance(1.0);
                while (rec1.hasMessage()) {
                    ++mCnt;
                    auto message = rec1.getMessage();
                    EXPECT_EQ(message->data.size(), 18U);
                }
            }
            mFed->finalize();
        };
        thread = std::thread(act1);
    }
    void join() { thread.join(); }
};

/** this test case fails as of yet with no good path to resolving it yet*/
/*
TEST_F(filter, reroute_cascade_2_ci_skip)
{
    extraBrokerArgs = " --global_time --logfile=logs.txt";
    auto broker = AddBroker("test", 18);
    broker->setLoggingLevel(HELICS_LOG_LEVEL_TRACE);
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "sender");
    AddFederates<helics::MessageFederate>("test_2", 9, broker, 1.0, "receiver");
    AddFederates<helics::MessageFederate>("test_2", 8, broker, 1.0, "filter");

    auto send = GetFederateAs<helics::MessageFederate>(0);
    std::vector<rfcheck> recFeds;
    for (int ii = 1; ii < 10; ++ii) {
        recFeds.emplace_back(GetFederateAs<helics::MessageFederate>(ii), ii);
    }

    auto& s1 = send->registerGlobalEndpoint("send");
    s1.setDefaultDestination("rec1");

    std::vector<std::shared_ptr<helics::MessageFederate>> filterFeds;
    std::vector<helics::Filter> filters;
    for (int ii = 0; ii < 8; ++ii) {
        auto filt = GetFederateAs<helics::MessageFederate>(10 + ii);
        auto& filt1 = helics::make_filter(helics::FilterTypes::REROUTE,
                                       filt.get(),
                                       std::string("rrfilt") + std::to_string(ii));
        filt1.addDestinationTarget(std::string("rec") + std::to_string(ii + 1));
        filt1.setString("newdestination", std::string("rec") + std::to_string(ii + 2));
        filters.push_back(filt1);
        filterFeds.push_back(std::move(filt));
    }

    auto acts = [&s1, &send]() {
        send->enterExecutingMode();
        helics::Time requestTime = helics::timeZero;
        while (requestTime < 10.0) {
            s1.send("this is a message1");
            s1.send("this is a message2");
            s1.send("this is a message3");
            s1.send("this is a message4");
            requestTime = send->requestTimeAdvance(1.0);
        }
        send->finalize();
    };
    auto thread1 = std::thread(acts);

    for (auto& rfed : recFeds) {
        rfed.run();
    }
    for (auto& ffed : filterFeds) {
        ffed->enterExecutingModeAsync();
    }
    for (auto& ffed : filterFeds) {
        ffed->enterExecutingModeComplete();
        ffed->requestTimeAsync(50);
    }

    helics::Time requestTime = helics::timeZero;
    helics::Time ptr = helics::timeZero;

    thread1.join();

    for (auto& rfed : recFeds) {
        rfed.join();
    }

    EXPECT_EQ(recFeds[0].mCnt, 0);
    EXPECT_EQ(recFeds[1].mCnt, 0);
    EXPECT_EQ(recFeds[2].mCnt, 0);
    EXPECT_EQ(recFeds[3].mCnt, 0);
    EXPECT_EQ(recFeds[4].mCnt, 0);
    EXPECT_EQ(recFeds[5].mCnt, 0);
    EXPECT_EQ(recFeds[6].mCnt, 0);
    EXPECT_EQ(recFeds[7].mCnt, 0);
    EXPECT_EQ(recFeds[8].mCnt, 40);

    for (auto& ffed : filterFeds) {
        ffed->requestTimeComplete();
        ffed->finalize();
    }
}
*/
TEST_F(filter, reroute_separate2)
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

    auto& ept1 = send->registerGlobalEndpoint("send");
    auto& ept2 = rec->registerGlobalEndpoint("rec");
    ept1.setDefaultDestination("rec");
    auto& reroute1 = filt->registerGlobalEndpoint("reroute");

    auto& filt1 = helics::make_filter(helics::FilterTypes::REROUTE, filt.get(), "rrfilt");

    filt1.addSourceTarget("send");
    filt1.setString("newdestination", "reroute");

    auto act1 = [&ept1, &send]() {
        send->enterExecutingMode();
        helics::Time requestTime = helics::timeZero;
        while (requestTime < 10.0) {
            ept1.send("this is a message");
            requestTime = send->requestTimeAdvance(1.0);
        }
        send->finalize();
    };

    auto act2 = [&rec]() {
        rec->enterExecutingMode();
        helics::Time requestTime = helics::timeZero;
        while (requestTime < 10.0) {
            requestTime = rec->requestTimeAdvance(1.0);
        }
        rec->finalize();
    };

    int cnt{0};
    auto act3 = [&filt, &cnt]() {
        filt->enterExecutingMode();
        helics::Time requestTime = helics::timeZero;
        while (requestTime < 20.0) {
            requestTime = filt->requestTime(helics::Time::maxVal());
            ++cnt;
        }
    };

    auto thread1 = std::thread(act1);
    auto thread2 = std::thread(act2);
    auto thread3 = std::thread(act3);

    thread1.join();
    thread2.join();

    // std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    // auto res = broker->query("root", "global_time_debugging");
    thread3.join();
    filt->finalize();
    EXPECT_EQ(ept2.pendingMessageCount(), 0U);
    EXPECT_EQ(reroute1.pendingMessageCount(), 10U);
    EXPECT_EQ(cnt, 11);
    // auto res2 = broker->query("root", "global_time_debugging");
    broker->waitForDisconnect();
}

TEST_F(filter, reroute_separate3)
{
    extraBrokerArgs = " --globaltime";
    auto broker = AddBroker(rerouteType, 3);
    AddFederates<helics::MessageFederate>(rerouteType, 1, broker);
    AddFederates<helics::MessageFederate>(rerouteType, 1, broker);
    AddFederates<helics::MessageFederate>(rerouteType, 1, broker);

    auto send = GetFederateAs<helics::MessageFederate>(0);
    auto rec = GetFederateAs<helics::MessageFederate>(1);
    auto filt = GetFederateAs<helics::MessageFederate>(2);

    auto& ept1 = send->registerGlobalEndpoint("send");
    auto& ept2 = rec->registerGlobalEndpoint("rec");
    ept1.setDefaultDestination("rec");
    auto& reroute1 = filt->registerGlobalEndpoint("reroute");

    auto& filt1 = helics::make_filter(helics::FilterTypes::REROUTE, filt.get(), "rrfilt");

    filt1.addSourceTarget("send");
    filt1.setString("newdestination", "reroute");

    auto act1 = [&ept1, &send]() {
        send->enterExecutingMode();
        helics::Time requestTime = helics::timeZero;
        while (requestTime < 10.0) {
            ept1.send("this is a message");
            requestTime = send->requestTimeAdvance(1.0);
        }
        send->finalize();
    };

    auto act2 = [&rec]() {
        rec->enterExecutingMode();
        helics::Time requestTime = helics::timeZero;
        while (requestTime < 10.0) {
            requestTime = rec->requestTimeAdvance(1.0);
        }
        rec->finalize();
    };

    auto thread1 = std::thread(act1);
    auto thread2 = std::thread(act2);
    int cnt{0};
    filt->enterExecutingMode();
    helics::Time requestTime = helics::timeZero;
    while (requestTime < 20.0) {
        requestTime = filt->requestTime(helics::Time::maxVal());
        ++cnt;
    }
    thread1.join();
    thread2.join();
    EXPECT_EQ(ept2.pendingMessageCount(), 0U);
    EXPECT_EQ(reroute1.pendingMessageCount(), 10U);
    EXPECT_EQ(cnt, 11);
    filt->finalize();
}

TEST_F(filter, reroute_separate_dest_target)
{
    auto broker = AddBroker(rerouteType, 3);
    AddFederates<helics::MessageFederate>(rerouteType, 1, broker, 1.0, "sender");
    AddFederates<helics::MessageFederate>(rerouteType, 1, broker, 1.0, "receiver");
    AddFederates<helics::MessageFederate>(rerouteType, 1, broker, 1.0, "filter");

    auto send = GetFederateAs<helics::MessageFederate>(0);
    auto rec = GetFederateAs<helics::MessageFederate>(1);
    auto filt = GetFederateAs<helics::MessageFederate>(2);
    send->setProperty(HELICS_PROPERTY_TIME_GRANT_TIMEOUT, 1.0);
    rec->setProperty(HELICS_PROPERTY_TIME_GRANT_TIMEOUT, 1.0);
    filt->setProperty(HELICS_PROPERTY_TIME_GRANT_TIMEOUT, 1.0);
    auto& ept1 = send->registerGlobalEndpoint("send");
    auto& ept2 = rec->registerGlobalEndpoint("rec");
    ept1.setDefaultDestination("rec");
    auto& reroute1 = filt->registerGlobalEndpoint("reroute");

    auto& filt1 = helics::make_filter(helics::FilterTypes::REROUTE, filt.get(), "rrfilt");

    filt1.addDestinationTarget("rec");
    filt1.setString("newdestination", "reroute");

    auto act1 = [&ept1, &send]() {
        send->enterExecutingMode();
        helics::Time requestTime = helics::timeZero;
        while (requestTime < 10.0) {
            ept1.send("this is a message");
            requestTime = send->requestTimeAdvance(1.0);
        }
        send->finalize();
    };
    int cntb{0};
    auto act2 = [&rec, &cntb]() {
        rec->enterExecutingMode();
        helics::Time requestTime = helics::timeZero;
        while (requestTime < 10.0) {
            requestTime = rec->requestTimeAdvance(1.0);
            if (rec->hasMessage()) {
                ++cntb;
            }
        }
        rec->finalize();
    };

    auto thread1 = std::thread(act1);
    auto thread2 = std::thread(act2);
    int cnt{0};
    filt->enterExecutingMode();
    std::vector<helics::Time> timeVector;
    timeVector.reserve(12);
    helics::Time requestTime = helics::timeZero;
    while (requestTime < 20.0) {
        requestTime = filt->requestTime(21.0);
        ++cnt;
        timeVector.emplace_back(requestTime);
    }
    thread1.join();
    thread2.join();
    EXPECT_EQ(ept2.pendingMessageCount(), 0U);
    EXPECT_EQ(reroute1.pendingMessageCount(), 10U);
    EXPECT_EQ(cnt, 10);
    if (cnt == 11) {
        EXPECT_EQ(cnt, 10);
    }
    EXPECT_EQ(cntb, 0);
    filt->finalize();
}

TEST_F(filter, separate_slow_filter_ci_skip)
{
    auto broker = AddBroker(rerouteType, 3);
    AddFederates<helics::MessageFederate>(rerouteType, 1, broker, 1.0, "sender");
    AddFederates<helics::MessageFederate>(rerouteType, 1, broker, 1.0, "receiver");
    AddFederates<helics::MessageFederate>(rerouteType, 1, broker, 1.0, "filter");

    auto send = GetFederateAs<helics::MessageFederate>(0);
    auto rec = GetFederateAs<helics::MessageFederate>(1);
    auto filt = GetFederateAs<helics::MessageFederate>(2);

    auto& ept1 = send->registerGlobalEndpoint("send");
    rec->registerGlobalEndpoint("rec");
    ept1.setDefaultDestination("rec");

    auto& filt1 = helics::make_filter(helics::FilterTypes::CUSTOM, filt.get(), "rrfilt");

    auto customOp = std::make_shared<helics::CustomMessageOperator>();
    auto mop = [](std::unique_ptr<helics::Message> message) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        message->data.append("bb");
        return message;
    };

    customOp->setMessageFunction(mop);
    filt1.setOperator(customOp);
    filt1.addSourceTarget("send");
    auto act1 = [&ept1, &send]() {
        send->enterExecutingMode();
        helics::Time requestTime = helics::timeZero;
        while (requestTime < 10.0) {
            ept1.send("this is a message");
            requestTime = send->requestTimeAdvance(1.0);
        }
        send->finalize();
    };
    int cntb{0};
    int mcnt{0};
    auto act2 = [&rec, &cntb, &mcnt]() {
        rec->enterExecutingMode();
        helics::Time requestTime = helics::timeZero;
        while (requestTime < 10.0) {
            requestTime = rec->requestTimeAdvance(1.0);
            ++cntb;
            while (rec->hasMessage()) {
                ++mcnt;
                auto message = rec->getMessage();
                EXPECT_EQ(message->data.to_string().back(), 'b');
            }
        }
        rec->finalize();
    };

    auto thread1 = std::thread(act1);
    auto thread2 = std::thread(act2);
    filt->enterExecutingMode();
    helics::Time requestTime = helics::timeZero;
    while (requestTime < 20.0) {
        requestTime = filt->requestTime(21.0);
    }
    thread1.join();
    thread2.join();
    EXPECT_EQ(mcnt, 10);
    EXPECT_EQ(cntb, 10);
    filt->finalize();
}

TEST_F(filter, separate_slow_dest_filter_ci_skip)
{
    auto broker = AddBroker(rerouteType, 3);
    AddFederates<helics::MessageFederate>(rerouteType, 1, broker, 1.0, "sender");
    AddFederates<helics::MessageFederate>(rerouteType, 1, broker, 1.0, "receiver");
    AddFederates<helics::MessageFederate>(rerouteType, 1, broker, 1.0, "filter");

    auto send = GetFederateAs<helics::MessageFederate>(0);
    auto rec = GetFederateAs<helics::MessageFederate>(1);
    auto filt = GetFederateAs<helics::MessageFederate>(2);

    auto& ept1 = send->registerGlobalEndpoint("send");
    rec->registerGlobalEndpoint("rec");
    ept1.setDefaultDestination("rec");

    auto& filt1 = helics::make_filter(helics::FilterTypes::CUSTOM, filt.get(), "rrfilt");

    auto customOp = std::make_shared<helics::CustomMessageOperator>();
    auto mop = [](std::unique_ptr<helics::Message> message) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        message->data.append("bb");
        return message;
    };

    customOp->setMessageFunction(mop);
    filt1.setOperator(customOp);
    filt1.addDestinationTarget("rec");
    auto act1 = [&ept1, &send]() {
        send->enterExecutingMode();
        helics::Time requestTime = helics::timeZero;
        while (requestTime < 10.0) {
            ept1.send("this is a message");
            requestTime = send->requestTimeAdvance(1.0);
        }
        send->finalize();
    };
    int cntb{0};
    int mcnt{0};
    auto act2 = [&rec, &cntb, &mcnt]() {
        rec->enterExecutingMode();
        helics::Time requestTime = helics::timeZero;
        while (requestTime < 10.0) {
            requestTime = rec->requestTimeAdvance(1.0);
            ++cntb;
            while (rec->hasMessage()) {
                ++mcnt;
                auto message = rec->getMessage();
                EXPECT_EQ(message->data.to_string().back(), 'b');
            }
        }
        rec->finalize();
    };

    auto thread1 = std::thread(act1);
    auto thread2 = std::thread(act2);
    filt->enterExecutingMode();
    helics::Time requestTime = helics::timeZero;
    while (requestTime < 20.0) {
        requestTime = filt->requestTime(21.0);
    }
    thread1.join();
    thread2.join();
    EXPECT_EQ(mcnt, 10);
    EXPECT_EQ(cntb, 10);
    filt->finalize();
}

TEST_F(filter, reroute_separate2_5message_nocov)
{
    extraBrokerArgs = " --globaltime";
    auto broker = AddBroker(rerouteType, 3);
    AddFederates<helics::MessageFederate>(rerouteType, 1, broker, helics::timeZero, "send");
    AddFederates<helics::MessageFederate>(rerouteType, 1, broker, helics::timeZero, "rec");
    AddFederates<helics::MessageFederate>(rerouteType, 1, broker, helics::timeZero, "filt");

    auto send = GetFederateAs<helics::MessageFederate>(0);
    auto rec = GetFederateAs<helics::MessageFederate>(1);
    auto filt = GetFederateAs<helics::MessageFederate>(2);

    auto& send1 = send->registerGlobalEndpoint("send1");
    auto& send2 = send->registerGlobalEndpoint("send2");
    auto& send3 = send->registerGlobalEndpoint("send3");
    auto& send4 = send->registerGlobalEndpoint("send4");
    auto& send5 = send->registerGlobalEndpoint("send5");

    auto& rec1 = rec->registerGlobalEndpoint("rec1");
    auto& rec2 = rec->registerGlobalEndpoint("rec2");
    auto& rec3 = rec->registerGlobalEndpoint("rec3");
    auto& rec4 = rec->registerGlobalEndpoint("rec4");
    auto& rec5 = rec->registerGlobalEndpoint("rec5");

    send1.setDefaultDestination("rec1");
    send2.setDefaultDestination("rec2");
    send3.setDefaultDestination("rec3");
    send4.setDefaultDestination("rec4");
    send5.setDefaultDestination("rec5");

    auto& reroute1 = filt->registerGlobalEndpoint("reroute");

    auto& filt1 = helics::make_filter(helics::FilterTypes::REROUTE, filt.get(), "rrfilt");

    filt1.addSourceTarget("send1");
    filt1.addSourceTarget("send2");
    filt1.addSourceTarget("send3");
    filt1.addSourceTarget("send4");
    filt1.addSourceTarget("send5");

    filt1.setString("newdestination", "reroute");

    auto act1 = [&]() {
        send->enterExecutingMode();
        helics::Time requestTime = helics::timeZero;
        while (requestTime < 10.0) {
            send1.send("this is a message1");

            send2.send("this is a message2");
            // std::this_thread::sleep_for(std::chrono::milliseconds(200));
            send3.send("this is a message3");
            send4.send("this is a message4");
            send5.send("this is a message5");
            // std::this_thread::sleep_for(std::chrono::milliseconds(200));
            requestTime = send->requestTimeAdvance(1.0);
        }
        send->finalize();
    };

    auto act2 = [&rec]() {
        rec->enterExecutingMode();
        helics::Time requestTime = helics::timeZero;
        while (requestTime < 10.0) {
            requestTime = rec->requestTimeAdvance(1.0);
        }
        rec->finalize();
    };

    int cnt{0};
    std::vector<int> mcount;
    auto act3 = [&]() {
        filt->enterExecutingMode();
        helics::Time requestTime = helics::timeZero;
        while (requestTime < 20.0) {
            requestTime = filt->requestTime(helics::Time::maxVal());
            mcount.push_back(0);
            while (reroute1.hasMessage()) {
                auto message = reroute1.getMessage();
                (void)message;
                ++mcount[cnt];
            }
            ++cnt;
        }
    };

    auto thread1 = std::thread(act1);
    auto thread2 = std::thread(act2);
    auto thread3 = std::thread(act3);

    thread1.join();
    thread2.join();

    // std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    // auto res = broker->query("root", "global_time_debugging");
    thread3.join();
    filt->finalize();
    EXPECT_EQ(rec1.pendingMessageCount(), 0U);
    EXPECT_EQ(rec2.pendingMessageCount(), 0U);
    EXPECT_EQ(rec3.pendingMessageCount(), 0U);
    EXPECT_EQ(rec4.pendingMessageCount(), 0U);
    EXPECT_EQ(rec5.pendingMessageCount(), 0U);

    EXPECT_EQ(reroute1.pendingMessageCount(), 0U);
    EXPECT_EQ(cnt, 11);
    int totalMessageCount{0};
    int index = 0;
    for (auto& count : mcount) {
        totalMessageCount += count;
        EXPECT_TRUE(count == 5 || count == 0) << "incorrect # of messages in interval [" << index
                                              << "], (" << count << ") messages instead of 5 ";
        ++index;
    }
    EXPECT_EQ(totalMessageCount, 50);
    // auto res2 = broker->query("root", "global_time_debugging");
    broker->waitForDisconnect();
}

TEST_F(filter, reroute_separate2_5000message_ci_skip_nocov)
{
    extraBrokerArgs = " --globaltime";
    auto broker = AddBroker(rerouteType, 3);
    AddFederates<helics::MessageFederate>(rerouteType, 1, broker, helics::timeZero, "send");
    AddFederates<helics::MessageFederate>(rerouteType, 1, broker, helics::timeZero, "rec");
    AddFederates<helics::MessageFederate>(rerouteType, 1, broker, helics::timeZero, "filt");

    auto send = GetFederateAs<helics::MessageFederate>(0);
    auto rec = GetFederateAs<helics::MessageFederate>(1);
    auto filt = GetFederateAs<helics::MessageFederate>(2);

    auto& send1 = send->registerGlobalEndpoint("send1");
    auto& send2 = send->registerGlobalEndpoint("send2");
    auto& send3 = send->registerGlobalEndpoint("send3");
    auto& send4 = send->registerGlobalEndpoint("send4");
    auto& send5 = send->registerGlobalEndpoint("send5");

    auto& rec1 = rec->registerGlobalEndpoint("rec1");
    auto& rec2 = rec->registerGlobalEndpoint("rec2");
    auto& rec3 = rec->registerGlobalEndpoint("rec3");
    auto& rec4 = rec->registerGlobalEndpoint("rec4");
    auto& rec5 = rec->registerGlobalEndpoint("rec5");

    send1.setDefaultDestination("rec1");
    send2.setDefaultDestination("rec2");
    send3.setDefaultDestination("rec3");
    send4.setDefaultDestination("rec4");
    send5.setDefaultDestination("rec5");

    auto& reroute1 = filt->registerGlobalEndpoint("reroute");

    auto& filt1 = helics::make_filter(helics::FilterTypes::REROUTE, filt.get(), "rrfilt");

    filt1.addSourceTarget("send1");
    filt1.addSourceTarget("send2");
    filt1.addSourceTarget("send3");
    filt1.addSourceTarget("send4");
    filt1.addSourceTarget("send5");

    filt1.setString("newdestination", "reroute");

    auto act1 = [&]() {
        send->enterExecutingMode();
        helics::Time requestTime = helics::timeZero;
        while (requestTime < 10.0) {
            for (int kk = 0; kk < 100; ++kk) {
                send1.send("this is a message1");

                send2.send("this is a message2");
                // std::this_thread::sleep_for(std::chrono::milliseconds(200));
                send3.send("this is a message3");
                send4.send("this is a message4");
                send5.send("this is a message5");
            }
            // std::this_thread::sleep_for(std::chrono::milliseconds(200));
            requestTime = send->requestTimeAdvance(1.0);
        }
        send->finalize();
    };

    auto act2 = [&rec]() {
        rec->enterExecutingMode();
        helics::Time requestTime = helics::timeZero;
        while (requestTime < 10.0) {
            requestTime = rec->requestTimeAdvance(1.0);
        }
        rec->finalize();
    };

    int cnt{0};
    std::vector<int> mcount;
    auto act3 = [&]() {
        filt->enterExecutingMode();
        helics::Time requestTime = helics::timeZero;
        while (requestTime < 20.0) {
            requestTime = filt->requestTime(helics::Time::maxVal());
            mcount.push_back(0);
            while (reroute1.hasMessage()) {
                auto message = reroute1.getMessage();
                (void)message;
                ++mcount[cnt];
            }
            ++cnt;
        }
    };

    auto thread1 = std::thread(act1);
    auto thread2 = std::thread(act2);
    auto thread3 = std::thread(act3);

    thread1.join();
    thread2.join();

    // std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    // auto res = broker->query("root", "global_time_debugging");
    thread3.join();
    filt->finalize();
    EXPECT_EQ(rec1.pendingMessageCount(), 0U);
    EXPECT_EQ(rec2.pendingMessageCount(), 0U);
    EXPECT_EQ(rec3.pendingMessageCount(), 0U);
    EXPECT_EQ(rec4.pendingMessageCount(), 0U);
    EXPECT_EQ(rec5.pendingMessageCount(), 0U);

    EXPECT_EQ(reroute1.pendingMessageCount(), 0U);
    EXPECT_EQ(cnt, 11);
    int totalMessageCount{0};
    int index = 0;
    for (auto& count : mcount) {
        totalMessageCount += count;
        EXPECT_TRUE(count == 500 || count == 0) << "incorrect # of messages in interval [" << index
                                                << "], (" << count << ") messages instead of 50 ";
        ++index;
    }
    EXPECT_EQ(totalMessageCount, 5000);
    // auto res2 = broker->query("root", "global_time_debugging");
    broker->waitForDisconnect();
}

TEST_F(filter, reroute_separate2_5message_b_nocov)
{
    extraBrokerArgs = " --globaltime";
    auto broker = AddBroker(rerouteType, 3);
    AddFederates<helics::MessageFederate>(rerouteType, 1, broker, helics::timeZero, "send");
    AddFederates<helics::MessageFederate>(rerouteType, 1, broker, helics::timeZero, "rec");
    AddFederates<helics::MessageFederate>(rerouteType, 1, broker, helics::timeZero, "filt");

    auto send = GetFederateAs<helics::MessageFederate>(0);
    auto rec = GetFederateAs<helics::MessageFederate>(1);
    auto filt = GetFederateAs<helics::MessageFederate>(2);

    auto& send1 = send->registerGlobalEndpoint("send1");
    auto& send2 = send->registerGlobalEndpoint("send2");
    auto& send3 = send->registerGlobalEndpoint("send3");
    auto& send4 = send->registerGlobalEndpoint("send4");
    auto& send5 = send->registerGlobalEndpoint("send5");

    auto& rec1 = rec->registerGlobalEndpoint("rec1");
    auto& rec2 = rec->registerGlobalEndpoint("rec2");
    auto& rec3 = rec->registerGlobalEndpoint("rec3");
    auto& rec4 = rec->registerGlobalEndpoint("rec4");
    auto& rec5 = rec->registerGlobalEndpoint("rec5");

    send1.setDefaultDestination("rec1");
    send2.setDefaultDestination("rec2");
    send3.setDefaultDestination("rec3");
    send4.setDefaultDestination("rec4");
    send5.setDefaultDestination("rec5");

    auto& ept3 = filt->registerGlobalEndpoint("reroute");

    auto& filt1 = helics::make_filter(helics::FilterTypes::REROUTE, filt.get(), "rrfilt1");
    auto& filt2 = helics::make_filter(helics::FilterTypes::REROUTE, filt.get(), "rrfilt2");
    auto& filt3 = helics::make_filter(helics::FilterTypes::REROUTE, filt.get(), "rrfilt3");
    auto& filt4 = helics::make_filter(helics::FilterTypes::REROUTE, filt.get(), "rrfilt4");
    auto& filt5 = helics::make_filter(helics::FilterTypes::REROUTE, filt.get(), "rrfilt5");

    filt1.addSourceTarget("send1");
    filt2.addSourceTarget("send2");
    filt3.addSourceTarget("send3");
    filt4.addSourceTarget("send4");
    filt5.addSourceTarget("send5");

    filt1.setString("newdestination", "reroute");
    filt2.setString("newdestination", "reroute");
    filt3.setString("newdestination", "reroute");
    filt4.setString("newdestination", "reroute");
    filt5.setString("newdestination", "reroute");

    auto act1 = [&]() {
        send->enterExecutingMode();
        helics::Time requestTime = helics::timeZero;
        while (requestTime < 10.0) {
            send1.send("this is a message1");

            send2.send("this is a message2");
            // std::this_thread::sleep_for(std::chrono::milliseconds(200));
            send3.send("this is a message3");
            send4.send("this is a message4");
            send5.send("this is a message5");
            // std::this_thread::sleep_for(std::chrono::milliseconds(200));
            requestTime = send->requestTimeAdvance(1.0);
        }
        send->finalize();
    };

    auto act2 = [&rec]() {
        rec->enterExecutingMode();
        helics::Time currentTime = helics::timeZero;
        while (currentTime < 10.0) {
            currentTime = rec->requestTimeAdvance(1.0);
        }
        rec->finalize();
    };

    int cnt{0};
    std::vector<int> mcount;
    auto act3 = [&]() {
        filt->enterExecutingMode();
        helics::Time currentTime = helics::timeZero;
        while (currentTime < 20.0) {
            currentTime = filt->requestTime(helics::Time::maxVal());
            mcount.push_back(0);
            while (ept3.hasMessage()) {
                auto message = ept3.getMessage();
                ++mcount[cnt];
            }
            ++cnt;
        }
    };

    auto thread1 = std::thread(act1);
    auto thread2 = std::thread(act2);
    auto thread3 = std::thread(act3);

    thread1.join();
    thread2.join();

    // std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    // auto res = broker->query("root", "global_time_debugging");
    thread3.join();
    filt->finalize();
    EXPECT_EQ(rec1.pendingMessageCount(), 0U);
    EXPECT_EQ(rec2.pendingMessageCount(), 0U);
    EXPECT_EQ(rec3.pendingMessageCount(), 0U);
    EXPECT_EQ(rec4.pendingMessageCount(), 0U);
    EXPECT_EQ(rec5.pendingMessageCount(), 0U);

    EXPECT_EQ(ept3.pendingMessageCount(), 0U);
    EXPECT_EQ(cnt, 11);
    int totalMessageCount{0};
    int index{0};
    for (auto& count : mcount) {
        totalMessageCount += count;
        EXPECT_TRUE(count == 5 || count == 0) << "incorrect # of messages in interval [" << index
                                              << "], (" << count << ") messages instead of 5 ";
        ++index;
    }
    EXPECT_EQ(totalMessageCount, 50);
    // auto res2 = broker->query("root", "global_time_debugging");
    broker->waitForDisconnect();
}

TEST_F(filter, message_filter_function_two_stage_coreApp_filter_link)
{
    auto broker = AddBroker("test", 3);
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "filter2");
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto fFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto mFed = GetFederateAs<helics::MessageFederate>(2);

    auto& ept1 = mFed->registerGlobalEndpoint("port1");
    auto& ept2 = mFed->registerGlobalEndpoint("port2");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    helics::CoreApp cApp(mFed->getCorePointer());
    cApp.addSourceFilterToEndpoint("filter1", "port1");
    cApp.addDestinationFilterToEndpoint("filter2", "port2");

    auto& filt1 = fFed->registerGlobalFilter("filter1");

    EXPECT_TRUE(filt1.getHandle().isValid());

    auto& filt2 = fFed2->registerGlobalFilter("filter2");

    EXPECT_TRUE(filt2.getHandle().isValid());

    bool res = twoStageFilterTest(mFed, fFed, fFed2, ept1, ept2, filt1, filt2);
    EXPECT_TRUE(res);
}

TEST_P(filter_single_type_test, message_filter_function_two_stage_broker_filter_link_switch_order)
{
    auto broker = AddBroker(GetParam(), 3);
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter2");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto fFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto mFed = GetFederateAs<helics::MessageFederate>(2);
    auto& filt1 = fFed->registerGlobalFilter("filter1");
    auto& filt2 = fFed2->registerGlobalFilter("filter2");

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    broker->addSourceFilterToEndpoint("filter1", "port1");
    broker->addDestinationFilterToEndpoint("filter2", "port2");
    auto& ept1 = mFed->registerGlobalEndpoint("port1");
    auto& ept2 = mFed->registerGlobalEndpoint("port2");

    bool res = twoStageFilterTest(mFed, fFed, fFed2, ept1, ept2, filt1, filt2);
    EXPECT_TRUE(res);
}

TEST_P(filter_single_type_test, message_filter_function_two_stage_broker_filter_link_late)
{
    auto broker = AddBroker(GetParam(), 3);
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter2");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto fFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto mFed = GetFederateAs<helics::MessageFederate>(2);

    auto& ept1 = mFed->registerGlobalEndpoint("port1");
    auto& ept2 = mFed->registerGlobalEndpoint("port2");

    auto& filt1 = fFed->registerGlobalFilter("filter1");
    auto& filt2 = fFed2->registerGlobalFilter("filter2");

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    broker->addSourceFilterToEndpoint("filter1", "port1");
    broker->addDestinationFilterToEndpoint("filter2", "port2");
    bool res = twoStageFilterTest(mFed, fFed, fFed2, ept1, ept2, filt1, filt2);
    EXPECT_TRUE(res);
}

TEST_P(filter_single_type_test, message_filter_function_two_stage_broker_filter_link_early)
{
    auto broker = AddBroker(GetParam(), 3);

    broker->addSourceFilterToEndpoint("filter1", "port1");
    broker->addDestinationFilterToEndpoint("filter2", "port2");

    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter2");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto fFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto mFed = GetFederateAs<helics::MessageFederate>(2);

    auto& ept1 = mFed->registerGlobalEndpoint("port1");
    auto& ept2 = mFed->registerGlobalEndpoint("port2");

    auto& filt1 = fFed->registerGlobalFilter("filter1");
    auto& filt2 = fFed2->registerGlobalFilter("filter2");

    bool res = twoStageFilterTest(mFed, fFed, fFed2, ept1, ept2, filt1, filt2);
    EXPECT_TRUE(res);
}

/** test a filter operator
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the
simulation
*/

TEST_P(filter_single_type_test, message_filter_function_two_stage_object)
{
    auto broker = AddBroker(GetParam(), 3);
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter2");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto fFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto mFed = GetFederateAs<helics::MessageFederate>(2);

    ASSERT_TRUE(fFed);
    ASSERT_TRUE(fFed2);
    ASSERT_TRUE(mFed);
    auto& ept1 = mFed->registerGlobalEndpoint("port1");
    auto& ept2 = mFed->registerGlobalEndpoint("port2");

    auto& filt1 = helics::make_filter(helics::FilterTypes::DELAY, fFed.get(), "filter1");
    filt1.addSourceTarget("port1");
    filt1.set("delay", 1.25);

    auto& filt2 = helics::make_filter(helics::FilterTypes::DELAY, fFed.get(), "filter2");
    filt2.addSourceTarget("port1");
    filt2.set("delay", 1.25);

    fFed->enterExecutingModeAsync();
    fFed2->enterExecutingModeAsync();
    mFed->enterExecutingMode();
    fFed->enterExecutingModeComplete();
    fFed2->enterExecutingModeComplete();

    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    helics::SmallBuffer data(500, 'a');
    ept1.sendTo(data, "port2");

    mFed->requestTimeAsync(1.0);
    fFed->requestTimeAsync(1.0);
    fFed2->requestTime(1.0);
    mFed->requestTimeComplete();
    fFed->requestTimeComplete();
    auto res = mFed->hasMessage();
    EXPECT_TRUE(!res);

    mFed->requestTimeAsync(2.0);
    fFed2->requestTimeAsync(2.0);
    fFed->requestTime(2.0);
    mFed->requestTimeComplete();
    fFed2->requestTimeComplete();
    ASSERT_TRUE(!mFed->hasMessage(ept2));

    fFed->requestTimeAsync(3.0);
    fFed2->requestTimeAsync(3.0);
    /*auto retTime = */ mFed->requestTime(3.0);
    if (!mFed->hasMessage(ept2)) {
        printf("missing message\n");
    }
    ASSERT_TRUE(mFed->hasMessage(ept2));

    auto message2 = mFed->getMessage(ept2);
    EXPECT_EQ(message2->source, "port1");
    EXPECT_EQ(message2->original_source, "port1");
    EXPECT_EQ(message2->dest, "port2");
    EXPECT_EQ(message2->data.size(), data.size());
    EXPECT_EQ(message2->time, 2.5);

    fFed->requestTimeComplete();
    fFed2->requestTimeComplete();
    auto filterCore = fFed->getCorePointer();
    auto mCore = mFed->getCorePointer();
    mFed->finalizeAsync();
    fFed->finalizeAsync();
    fFed2->finalize();
    mFed->finalizeComplete();
    fFed->finalizeComplete();
    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::FINALIZE);
    helics::cleanupHelicsLibrary();
    EXPECT_TRUE(!filterCore->isConnected());
    EXPECT_TRUE(!mCore->isConnected());
}
/** test two filter operators
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the
simulation
*/

TEST_P(filter_single_type_test, message_filter_function2)
{
    auto broker = AddBroker(GetParam(), 2);
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto mFed = GetFederateAs<helics::MessageFederate>(1);
    ASSERT_TRUE(fFed);
    ASSERT_TRUE(mFed);

    auto& ept1 = mFed->registerGlobalEndpoint("port1");
    auto& ept2 = mFed->registerGlobalEndpoint("port2");

    auto& filt1 = fFed->registerFilter("filter1");
    filt1.addSourceTarget("port1");
    filt1.addSourceTarget("port2");
    EXPECT_TRUE(filt1.getHandle().isValid());
    auto timeOperator = std::make_shared<helics::MessageTimeOperator>();
    timeOperator->setTimeFunction([](helics::Time time_in) { return time_in + 2.5; });
    filt1.setOperator(timeOperator);
    fFed->enterExecutingModeAsync();
    mFed->enterExecutingMode();
    fFed->enterExecutingModeComplete();

    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    helics::SmallBuffer data(500, 'a');
    ept1.sendTo(data, "port2");

    mFed->requestTimeAsync(1.0);
    fFed->requestTime(1.0);
    mFed->requestTimeComplete();

    auto res = mFed->hasMessage();
    EXPECT_TRUE(!res);
    ept2.sendTo(data, "port1");
    mFed->requestTimeAsync(2.0);
    fFed->requestTime(2.0);
    mFed->requestTimeComplete();
    ASSERT_TRUE(!mFed->hasMessage(ept2));

    std::this_thread::yield();
    mFed->requestTime(3.0);

    ASSERT_TRUE(mFed->hasMessage(ept2));

    auto message2 = mFed->getMessage(ept2);
    EXPECT_EQ(message2->source, "port1");
    EXPECT_EQ(message2->original_source, "port1");
    EXPECT_EQ(message2->dest, "port2");
    EXPECT_EQ(message2->data.size(), data.size());
    EXPECT_EQ(message2->time, 2.5);

    EXPECT_TRUE(!mFed->hasMessage(ept1));
    mFed->requestTime(4.0);
    EXPECT_TRUE(mFed->hasMessage(ept1));
    mFed->finalizeAsync();
    fFed->finalize();
    mFed->finalizeComplete();
    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

TEST_P(filter_single_type_test, message_filter_function2_rem_target)
{
    auto broker = AddBroker(GetParam(), 2);
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto mFed = GetFederateAs<helics::MessageFederate>(1);
    ASSERT_TRUE(fFed);
    ASSERT_TRUE(mFed);

    auto& ept1 = mFed->registerGlobalEndpoint("port1");
    auto& ept2 = mFed->registerGlobalEndpoint("port2");

    auto& filt1 = fFed->registerFilter("filter1");
    filt1.addSourceTarget("port1");
    filt1.addSourceTarget("port2");
    EXPECT_TRUE(filt1.getHandle().isValid());
    auto timeOperator = std::make_shared<helics::MessageTimeOperator>();
    timeOperator->setTimeFunction([](helics::Time time_in) { return time_in + 2.5; });
    filt1.setOperator(timeOperator);
    fFed->enterExecutingModeAsync();
    mFed->enterExecutingMode();
    fFed->enterExecutingModeComplete();

    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    helics::SmallBuffer data(500, 'a');
    ept1.sendTo(data, "port2");

    mFed->requestTimeAsync(1.0);
    fFed->requestTime(1.0);
    mFed->requestTimeComplete();

    auto res = mFed->hasMessage();
    EXPECT_TRUE(!res);
    ept2.sendTo(data, "port1");
    mFed->requestTimeAsync(2.0);
    fFed->requestTime(2.0);
    mFed->requestTimeComplete();
    ASSERT_TRUE(!mFed->hasMessage(ept2));

    std::this_thread::yield();
    mFed->requestTime(3.0);

    ASSERT_TRUE(mFed->hasMessage(ept2));

    auto message2 = mFed->getMessage(ept2);
    EXPECT_EQ(message2->source, "port1");
    EXPECT_EQ(message2->original_source, "port1");
    EXPECT_EQ(message2->dest, "port2");
    EXPECT_EQ(message2->data.size(), data.size());
    EXPECT_EQ(message2->time, 2.5);

    EXPECT_TRUE(!mFed->hasMessage(ept1));
    mFed->requestTime(4.0);
    EXPECT_TRUE(mFed->hasMessage(ept1));
    filt1.removeTarget("port1");
    mFed->requestTimeAsync(5.0);
    fFed->requestTime(5.0);
    mFed->requestTimeComplete();
    ept1.sendTo(data, "port2");

    mFed->requestTimeAsync(6.0);
    fFed->requestTime(6.0);
    mFed->requestTimeComplete();
    // now the message hasn't been delayed
    EXPECT_TRUE(mFed->hasMessage(ept2));

    mFed->finalize();
    fFed->finalize();
    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

class filter_test: public ::testing::Test, public FederateTestFixture {};

TEST_F(filter_test, message_clone_test)
{
    auto broker = AddBroker("test", 3);
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "source");
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "dest");
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "dest_clone");

    auto sFed = GetFederateAs<helics::MessageFederate>(0);
    auto dFed = GetFederateAs<helics::MessageFederate>(1);
    auto dcFed = GetFederateAs<helics::MessageFederate>(2);

    auto& ept1 = sFed->registerGlobalEndpoint("src");
    auto& ept2 = dFed->registerGlobalEndpoint("dest");
    auto& ept3 = dcFed->registerGlobalEndpoint("cm");

    helics::CloningFilter cFilt(dcFed.get());
    cFilt.addSourceTarget("src");
    cFilt.addDeliveryEndpoint("cm");

    sFed->enterExecutingModeAsync();
    dcFed->enterExecutingModeAsync();
    dFed->enterExecutingMode();
    sFed->enterExecutingModeComplete();
    dcFed->enterExecutingModeComplete();

    EXPECT_TRUE(sFed->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    helics::SmallBuffer data(500, 'a');
    ept1.sendTo(data, "dest");

    sFed->requestTimeAsync(1.0);
    dcFed->requestTimeAsync(1.0);
    dFed->requestTime(1.0);
    sFed->requestTimeComplete();
    dcFed->requestTimeComplete();

    auto res = dFed->hasMessage();
    EXPECT_TRUE(res);

    if (res) {
        auto message2 = dFed->getMessage(ept2);
        EXPECT_EQ(message2->source, "src");
        EXPECT_EQ(message2->original_source, "src");
        EXPECT_EQ(message2->dest, "dest");
        EXPECT_EQ(message2->data.size(), data.size());
    }

    // now check the message clone
    res = dcFed->hasMessage();
    EXPECT_TRUE(res);

    if (res) {
        auto message2 = dcFed->getMessage(ept3);
        EXPECT_EQ(message2->source, "src");
        EXPECT_EQ(message2->original_source, "src");
        EXPECT_EQ(message2->dest, "cm");
        EXPECT_EQ(message2->original_dest, "dest");
        EXPECT_EQ(message2->data.size(), data.size());
    }

    sFed->finalizeAsync();
    dFed->finalizeAsync();
    dcFed->finalize();
    sFed->finalizeComplete();
    dFed->finalizeComplete();
    EXPECT_TRUE(sFed->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

TEST_F(filter_test, message_multi_clone_test)
{
    extraBrokerArgs = " --globaltime";
    auto broker = AddBroker("test", 4);
    AddFederates<helics::MessageFederate>("test", 2, broker, 1.0, "source");
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "dest");
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "dest_clone");

    auto sFed = GetFederateAs<helics::MessageFederate>(0);
    auto sFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto dFed = GetFederateAs<helics::MessageFederate>(2);
    auto dcFed = GetFederateAs<helics::MessageFederate>(3);

    auto& ept1 = sFed->registerGlobalEndpoint("src");
    auto& ept2 = sFed2->registerGlobalEndpoint("src2");
    auto& destEpt = dFed->registerGlobalEndpoint("dest");
    auto& cmEpt = dcFed->registerGlobalEndpoint("cm");

    helics::CloningFilter cFilt(dcFed.get());
    cFilt.addSourceTarget("src");
    cFilt.addSourceTarget("src2");
    cFilt.addDeliveryEndpoint("cm");

    sFed->enterExecutingModeAsync();
    sFed2->enterExecutingModeAsync();
    dcFed->enterExecutingModeAsync();
    dFed->enterExecutingMode();
    sFed->enterExecutingModeComplete();
    sFed2->enterExecutingModeComplete();
    dcFed->enterExecutingModeComplete();

    EXPECT_TRUE(sFed->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    helics::SmallBuffer data(500, 'a');
    helics::SmallBuffer data2(400, 'b');
    ept1.sendTo(data, "dest");
    ept2.sendTo(data2, "dest");
    sFed->requestTimeAsync(1.0);
    sFed2->requestTimeAsync(1.0);
    dcFed->requestTimeAsync(1.0);
    dFed->requestTime(1.0);
    sFed->requestTimeComplete();
    sFed2->requestTimeComplete();
    dcFed->requestTimeComplete();

    auto mcnt = dFed->pendingMessageCount(destEpt);
    EXPECT_EQ(mcnt, 2);
    auto res = dFed->hasMessage();
    EXPECT_TRUE(res);

    if (res) {
        auto message2 = dFed->getMessage(destEpt);
        EXPECT_EQ(message2->source, "src");
        EXPECT_EQ(message2->original_source, "src");
        EXPECT_EQ(message2->dest, "dest");
        EXPECT_EQ(message2->data.size(), data.size());
        res = dFed->hasMessage();
        EXPECT_TRUE(res);

        if (res) {
            message2 = dFed->getMessage(destEpt);
            EXPECT_EQ(message2->source, "src2");
            EXPECT_EQ(message2->original_source, "src2");
            EXPECT_EQ(message2->dest, "dest");
            EXPECT_EQ(message2->data.size(), data2.size());
        }
    }

    // now check the message clone
    mcnt = dcFed->pendingMessageCount(cmEpt);
    EXPECT_EQ(mcnt, 2);
    res = dcFed->hasMessage();
    EXPECT_TRUE(res);

    if (res) {
        auto message2 = dcFed->getMessage(cmEpt);
        EXPECT_EQ(message2->source, "src");
        EXPECT_EQ(message2->original_source, "src");
        EXPECT_EQ(message2->dest, "cm");
        EXPECT_EQ(message2->original_dest, "dest");
        EXPECT_EQ(message2->data.size(), data.size());
        res = dcFed->hasMessage();
        EXPECT_TRUE(res);

        if (res) {
            message2 = dcFed->getMessage(cmEpt);
            EXPECT_EQ(message2->source, "src2");
            EXPECT_EQ(message2->original_source, "src2");
            EXPECT_EQ(message2->dest, "cm");
            EXPECT_EQ(message2->original_dest, "dest");
            EXPECT_EQ(message2->data.size(), data2.size());
        }
    }

    sFed->finalizeAsync();
    sFed2->finalizeAsync();
    dFed->finalizeAsync();
    dcFed->finalize();
    sFed->finalizeComplete();
    sFed2->finalizeComplete();
    dFed->finalizeComplete();
    EXPECT_TRUE(sFed->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

/** test whether a core termination when it should
 */

TEST_P(filter_single_type_test, filter_core_termination)
{
    auto broker = AddBroker(GetParam(), 2);
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto mFed = GetFederateAs<helics::MessageFederate>(1);

    auto& ept1 = mFed->registerGlobalEndpoint("port1");
    auto& ept2 = mFed->registerGlobalEndpoint("port2");

    auto coreP = fFed->getCorePointer();
    auto filt1 = coreP->registerFilter("filter1", std::string(), std::string());
    coreP->addSourceTarget(filt1, "port1");
    auto timeOperator = std::make_shared<helics::MessageTimeOperator>();
    timeOperator->setTimeFunction([](helics::Time time_in) { return time_in + 2.5; });
    coreP->setFilterOperator(filt1, timeOperator);

    fFed->enterExecutingModeAsync();
    mFed->enterExecutingMode();
    fFed->enterExecutingModeComplete();

    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    helics::SmallBuffer data(500, 'a');
    ept1.sendTo(data, "port2");

    mFed->requestTimeAsync(1.0);
    fFed->requestTime(1.0);
    mFed->requestTimeComplete();
    fFed->finalizeAsync();
    auto res = mFed->hasMessage();
    EXPECT_TRUE(!res);

    mFed->requestTime(2.0);
    ASSERT_TRUE(!mFed->hasMessage(ept2));
    EXPECT_TRUE(coreP->isConnected());
    mFed->requestTime(3.0);
    ept1.sendTo(data, "port2");
    ASSERT_TRUE(mFed->hasMessage(ept2));

    auto message2 = mFed->getMessage(ept2);
    EXPECT_EQ(message2->source, "port1");
    EXPECT_EQ(message2->original_source, "port1");
    EXPECT_EQ(message2->dest, "port2");
    EXPECT_EQ(message2->data.size(), data.size());
    EXPECT_EQ(message2->time, 2.5);

    auto requestTime = mFed->requestTime(4.0);
    EXPECT_TRUE(!mFed->hasMessage(ept2));
    requestTime = mFed->requestTime(6.0);
    EXPECT_TRUE(mFed->hasMessage(ept2));
    mFed->finalize();
    fFed->finalizeComplete();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    if (coreP->isConnected()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }
    EXPECT_TRUE(!coreP->isConnected());
    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

// test for filter configuration of different flavors
TEST_F(filter, filter_config_test1)
{
    auto fed1 =
        std::make_shared<helics::MessageFederate>(std::string(TEST_DIR) + "filter_test1.json");

    EXPECT_NO_THROW(fed1->enterExecutingMode());

    EXPECT_EQ(fed1->getFilterCount(), 1U);

    EXPECT_EQ(fed1->getEndpointCount(), 3U);
    auto& ept1 = fed1->getEndpoint(0);
    auto& ept2 = fed1->getEndpoint(1);

    ept1.send("1");
    fed1->requestNextStep();
    EXPECT_FALSE(ept2.hasMessage());
    fed1->requestNextStep();
    EXPECT_TRUE(ept2.hasMessage());

    fed1->finalize();

    FullDisconnect();
}

// test for filter configuration of different flavors
TEST_F(filter, filter_config_test2)
{
    auto fed1 =
        std::make_shared<helics::MessageFederate>(std::string(TEST_DIR) + "filter_test2.json");

    EXPECT_NO_THROW(fed1->enterExecutingMode());

    EXPECT_EQ(fed1->getFilterCount(), 1U);

    EXPECT_EQ(fed1->getEndpointCount(), 3U);
    auto& ept1 = fed1->getEndpoint(0);
    auto& ept2 = fed1->getEndpoint(1);

    ept1.send("1");
    fed1->requestNextStep();
    EXPECT_FALSE(ept2.hasMessage());
    fed1->requestNextStep();
    EXPECT_TRUE(ept2.hasMessage());

    fed1->finalize();

    FullDisconnect();
}

// test for filter configuration of different flavors
TEST_F(filter, filter_config_test3)
{
    auto fed1 =
        std::make_shared<helics::MessageFederate>(std::string(TEST_DIR) + "filter_test3.json");

    EXPECT_NO_THROW(fed1->enterExecutingMode());

    EXPECT_EQ(fed1->getFilterCount(), 1U);

    EXPECT_EQ(fed1->getEndpointCount(), 3U);
    auto& ept1 = fed1->getEndpoint(0);
    auto& ept2 = fed1->getEndpoint(1);
    auto& ept3 = fed1->getEndpoint(2);

    ept1.send("1");
    fed1->requestNextStep();
    EXPECT_FALSE(ept2.hasMessage());
    EXPECT_TRUE(ept3.hasMessage());

    fed1->finalize();

    FullDisconnect();
}

// test for filter configuration of different flavors
TEST_F(filter, filter_config_test4)
{
    auto fed1 =
        std::make_shared<helics::MessageFederate>(std::string(TEST_DIR) + "filter_test4.json");

    EXPECT_NO_THROW(fed1->enterExecutingMode());

    EXPECT_EQ(fed1->getFilterCount(), 1U);

    EXPECT_EQ(fed1->getEndpointCount(), 3U);
    auto& ept1 = fed1->getEndpoint(0);
    auto& ept2 = fed1->getEndpoint(1);
    auto& ept3 = fed1->getEndpoint(2);

    ept1.send("1");
    fed1->requestNextStep();
    EXPECT_TRUE(ept2.hasMessage());
    EXPECT_TRUE(ept3.hasMessage());

    fed1->finalize();

    FullDisconnect();
}

// test for filter configuration of different flavors
TEST_F(filter, filter_config_test5)
{
    auto fed1 =
        std::make_shared<helics::MessageFederate>(std::string(TEST_DIR) + "filter_test5.json");

    EXPECT_NO_THROW(fed1->enterExecutingMode());

    EXPECT_EQ(fed1->getFilterCount(), 1U);

    EXPECT_EQ(fed1->getEndpointCount(), 3U);
    auto& ept1 = fed1->getEndpoint(0);
    auto& ept2 = fed1->getEndpoint(1);
    auto& ept3 = fed1->getEndpoint(2);

    ept1.send("1");
    fed1->requestNextStep();
    EXPECT_TRUE(ept2.hasMessage());
    EXPECT_TRUE(ept3.hasMessage());

    fed1->finalize();

    FullDisconnect();
}

INSTANTIATE_TEST_SUITE_P(filter,
                         filter_single_type_test,
                         ::testing::ValuesIn(CoreTypes_simple),
                         testNamer);
/*
INSTANTIATE_TEST_SUITE_P(filter, filter_all_type_test, ::testing::ValuesIn(CoreTypes_all));
*/
