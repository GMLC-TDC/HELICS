/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

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
#include <future>
#include <gtest/gtest.h>
/** these test cases test out the message federates
 */

class filter_single_type_test:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {
};

class filter_all_type_test:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {
};

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

    auto& f1 = fFed->registerFilter("filter1");
    fFed->addSourceTarget(f1, "port1");
    EXPECT_TRUE(f1.getHandle().isValid());
    auto& f2 = fFed->registerFilter("filter2");
    fFed->addDestinationTarget(f2, "port2");
    EXPECT_TRUE(f2.getHandle().isValid());
    auto& ep1 = fFed->registerEndpoint("fout");
    EXPECT_TRUE(ep1.getHandle().isValid());

    mFed->finalizeAsync();
    // std::this_thread::sleep_for (std::chrono::milliseconds (50));
    auto& f3 = fFed->registerFilter();
    fFed->addSourceTarget(f3, "filter0/fout");
    EXPECT_TRUE(f3.getHandle() != f2.getHandle());
    fFed->finalize();
    // std::cout << "fFed returned\n";
    mFed->finalizeComplete();
    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::modes::finalize);
    FullDisconnect();
}

/** test a filter operator
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the simulation
*/
TEST_P(filter_single_type_test, message_filter_function)
{
    auto broker = AddBroker(GetParam(), 2);
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto mFed = GetFederateAs<helics::MessageFederate>(1);

    auto& p1 = mFed->registerGlobalEndpoint("port1");
    auto& p2 = mFed->registerGlobalEndpoint("port2");

    auto& f1 = fFed->registerFilter("filter1");
    fFed->addSourceTarget(f1, "port1");
    EXPECT_TRUE(f1.getHandle().isValid());
    auto timeOperator = std::make_shared<helics::MessageTimeOperator>();
    timeOperator->setTimeFunction([](helics::Time time_in) { return time_in + 2.5; });
    fFed->setFilterOperator(f1, timeOperator);

    fFed->enterExecutingModeAsync();
    mFed->enterExecutingMode();
    fFed->enterExecutingModeComplete();

    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::modes::executing);
    helics::data_block data(500, 'a');
    mFed->sendMessage(p1, "port2", data);

    mFed->requestTimeAsync(1.0);
    fFed->requestTime(1.0);
    mFed->requestTimeComplete();

    auto res = mFed->hasMessage();
    EXPECT_TRUE(!res);
    if (res) {
        auto m3 = mFed->getMessage(p2);
    }
    mFed->requestTimeAsync(2.0);
    fFed->requestTime(2.0);
    mFed->requestTimeComplete();
    EXPECT_TRUE(!mFed->hasMessage(p2));
    if (mFed->hasMessage(p2)) {
        auto m3 = mFed->getMessage(p2);
    }
    fFed->requestTimeAsync(3.0);
    auto retTime = mFed->requestTime(3.0);
    EXPECT_EQ(retTime, 3.0);
    ASSERT_TRUE(mFed->hasMessage(p2));

    auto m2 = mFed->getMessage(p2);
    EXPECT_EQ(m2->source, "port1");
    EXPECT_EQ(m2->original_source, "port1");
    EXPECT_EQ(m2->dest, "port2");
    EXPECT_EQ(m2->data.size(), data.size());
    EXPECT_EQ(m2->time, 2.5);

    mFed->requestTime(3.0);
    fFed->requestTimeComplete();
    mFed->finalizeAsync();
    fFed->finalize();
    mFed->finalizeComplete();
    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::modes::finalize);
}

/** test a filter operator
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the simulation
*/

TEST_P(filter_single_type_test, message_filter_object)
{
    auto broker = AddBroker(GetParam(), 2);
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto mFed = GetFederateAs<helics::MessageFederate>(1);

    auto& p1 = mFed->registerGlobalEndpoint("port1");
    auto& p2 = mFed->registerGlobalEndpoint("port2");

    auto& Filt = helics::make_filter(helics::filter_types::delay, fFed.get());
    Filt.addSourceTarget("port1");
    Filt.set("delay", 2.5);

    fFed->enterExecutingModeAsync();
    mFed->enterExecutingMode();
    fFed->enterExecutingModeComplete();

    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::modes::executing);
    helics::data_block data(500, 'a');
    mFed->sendMessage(p1, "port2", data);

    mFed->requestTimeAsync(1.0);
    fFed->requestTime(1.0);
    mFed->requestTimeComplete();

    auto res = mFed->hasMessage();
    EXPECT_TRUE(!res);

    mFed->requestTimeAsync(2.0);
    fFed->requestTime(2.0);
    mFed->requestTimeComplete();
    ASSERT_TRUE(!mFed->hasMessage(p2));

    fFed->requestTimeAsync(3.0);
    /*auto retTime = */ mFed->requestTime(3.0);

    ASSERT_TRUE(mFed->hasMessage(p2));

    auto m2 = mFed->getMessage(p2);
    EXPECT_EQ(m2->source, "port1");
    EXPECT_EQ(m2->original_source, "port1");
    EXPECT_EQ(m2->dest, "port2");
    EXPECT_EQ(m2->data.size(), data.size());
    EXPECT_EQ(m2->time, 2.5);

    mFed->requestTime(3.0);
    fFed->requestTimeComplete();
    mFed->finalizeAsync();
    fFed->finalize();
    mFed->finalizeComplete();
    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::modes::finalize);
}

/** test a remove dest filter operator
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the simulation
*/

TEST_P(filter_single_type_test, message_dest_filter_function)
{
    auto broker = AddBroker(GetParam(), 2);
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto mFed = GetFederateAs<helics::MessageFederate>(1);

    auto& p1 = mFed->registerGlobalEndpoint("port1");
    auto& p2 = mFed->registerGlobalEndpoint("port2");

    auto& f1 = fFed->registerFilter("filter1");
    fFed->addDestinationTarget(f1, "port2");
    EXPECT_TRUE(f1.getHandle().isValid());
    auto timeOperator = std::make_shared<helics::MessageTimeOperator>();
    timeOperator->setTimeFunction([](helics::Time time_in) { return time_in + 2.5; });
    fFed->setFilterOperator(f1, timeOperator);

    fFed->enterExecutingModeAsync();
    mFed->enterExecutingMode();
    fFed->enterExecutingModeComplete();

    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::modes::executing);
    helics::data_block data(500, 'a');
    mFed->sendMessage(p1, "port2", data);

    mFed->requestTime(1.0);

    auto res = mFed->hasMessage();
    if (res) {
        auto m = mFed->getMessage();
        EXPECT_TRUE(!res);
    }

    mFed->requestTime(2.0);
    ASSERT_TRUE(!mFed->hasMessage(p2));

    fFed->requestTimeAsync(3.0);
    /*auto retTime = */ mFed->requestTime(3.0);

    ASSERT_TRUE(mFed->hasMessage(p2));

    auto m2 = mFed->getMessage(p2);
    EXPECT_EQ(m2->source, "port1");
    EXPECT_EQ(m2->original_source, "port1");
    EXPECT_EQ(m2->dest, "port2");
    EXPECT_EQ(m2->data.size(), data.size());
    EXPECT_EQ(m2->time, 2.5);

    mFed->requestTime(3.0);
    fFed->requestTimeComplete();
    mFed->finalizeAsync();
    fFed->finalize();
    mFed->finalizeComplete();
    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::modes::finalize);
}

/** test a remote dest filter operator
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the simulation
*/

TEST_P(filter_all_type_test, message_dest_filter_function_t2)
{
    auto broker = AddBroker(GetParam(), 2);
    AddFederates<helics::MessageFederate>(GetParam(), 2, broker, 0.5, "message");

    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    auto& p1 = mFed1->registerGlobalEndpoint("port1");
    auto& p2 = mFed2->registerGlobalEndpoint("port2");

    auto& f1 = mFed2->registerFilter("filter1");
    mFed2->addSourceTarget(f1, "port1");
    EXPECT_TRUE(f1.getHandle().isValid());
    auto timeOperator = std::make_shared<helics::MessageTimeOperator>();
    timeOperator->setTimeFunction([](helics::Time time_in) { return time_in + 2.5; });
    mFed2->setFilterOperator(f1, timeOperator);

    mFed1->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();

    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::modes::executing);
    helics::data_block data(500, 'a');
    mFed1->sendMessage(p1, "port2", data);

    mFed1->requestTimeAsync(1.0);
    mFed2->requestTime(1.0);
    mFed1->requestTimeComplete();

    auto res = mFed2->hasMessage();
    EXPECT_TRUE(!res);

    mFed1->requestTimeAsync(2.0);
    mFed2->requestTime(2.0);
    mFed1->requestTimeComplete();
    ASSERT_TRUE(!mFed2->hasMessage(p2));

    mFed1->requestTimeAsync(3.0);
    auto retTime = mFed2->requestTime(3.0);

    EXPECT_TRUE(retTime == 2.5);
    ASSERT_TRUE(mFed2->hasMessage(p2));

    auto m2 = mFed2->getMessage(p2);

    mFed2->requestTime(3.0);
    mFed1->requestTimeComplete();
    mFed1->finalizeAsync();
    mFed2->finalize();
    mFed1->finalizeComplete();
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::modes::finalize);
}

/** test a remove dest filter operator
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the simulation
*/

TEST_P(filter_single_type_test, message_dest_filter_object)
{
    auto broker = AddBroker(GetParam(), 2);
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto mFed = GetFederateAs<helics::MessageFederate>(1);

    auto& p1 = mFed->registerGlobalEndpoint("port1");
    auto& p2 = mFed->registerGlobalEndpoint("port2");

    auto f1 =
        helics::make_filter(helics::filter_types::delay, fFed->getCorePointer().get(), "filter1");
    f1->addDestinationTarget("port2");
    f1->set("delay", 2.5);

    fFed->enterExecutingModeAsync();
    mFed->enterExecutingMode();
    fFed->enterExecutingModeComplete();

    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::modes::executing);
    helics::data_block data(500, 'a');
    mFed->sendMessage(p1, "port2", data);

    mFed->requestTimeAsync(1.0);
    fFed->requestTime(1.0);
    mFed->requestTimeComplete();

    auto res = mFed->hasMessage();
    EXPECT_TRUE(!res);

    mFed->requestTimeAsync(2.0);
    fFed->requestTime(2.0);
    mFed->requestTimeComplete();
    ASSERT_TRUE(!mFed->hasMessage(p2));

    fFed->requestTimeAsync(3.0);
    /*auto retTime = */ mFed->requestTime(3.0);

    ASSERT_TRUE(mFed->hasMessage(p2));

    auto m2 = mFed->getMessage(p2);
    EXPECT_EQ(m2->source, "port1");
    EXPECT_EQ(m2->original_source, "port1");
    EXPECT_EQ(m2->dest, "port2");
    EXPECT_EQ(m2->data.size(), data.size());
    EXPECT_EQ(m2->time, 2.5);

    mFed->requestTime(3.0);
    fFed->requestTimeComplete();
    auto filterCore = fFed->getCorePointer();
    auto mCore = mFed->getCorePointer();
    mFed->finalizeAsync();
    fFed->finalize();
    mFed->finalizeComplete();
    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::modes::finalize);
    helics::cleanupHelicsLibrary();
    EXPECT_TRUE(!filterCore->isConnected());
    EXPECT_TRUE(!mCore->isConnected());
}

static bool two_stage_filter_test(
    std::shared_ptr<helics::MessageFederate>& mFed,
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

    auto& p2Name = mFed->getInterfaceName(p2);
    EXPECT_TRUE(fFed1->getCurrentMode() == helics::Federate::modes::executing);
    helics::data_block data(500, 'a');
    mFed->sendMessage(p1, p2Name, data);

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
        auto ept1Name = mFed->getInterfaceName(p1);
        if (ept1Name.size() > 1) {
            EXPECT_EQ(m2->source, mFed->getInterfaceName(p1));
            EXPECT_EQ(m2->original_source, mFed->getInterfaceName(p1));
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
    EXPECT_TRUE(fFed1->getCurrentMode() == helics::Federate::modes::finalize);
    if (fFed1->getCurrentMode() != helics::Federate::modes::finalize) {
        correct = false;
    }
    helics::cleanupHelicsLibrary();
    EXPECT_TRUE(!filterCore->isConnected());
    EXPECT_TRUE(!mCore->isConnected());
    return correct;
}
/** test a filter operator
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the simulation
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

    auto& p1 = mFed->registerGlobalEndpoint("port1");
    auto& p2 = mFed->registerGlobalEndpoint("port2");

    auto& f1 = fFed->registerFilter("filter1");
    fFed->addSourceTarget(f1, "port1");
    EXPECT_TRUE(f1.getHandle().isValid());

    auto& f2 = fFed2->registerFilter("filter2");
    fFed2->addSourceTarget(f2, "port1");
    EXPECT_TRUE(f2.getHandle().isValid());

    bool res = two_stage_filter_test(mFed, fFed, fFed2, p1, p2, f1, f2);
    EXPECT_TRUE(res);
}

TEST_P(filter_single_type_test, message_filter_function_two_stage_endpoint_target)
{
    auto broker = AddBroker(GetParam(), 3);
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter2");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto fFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto mFed = GetFederateAs<helics::MessageFederate>(2);

    auto& p1 = mFed->registerEndpoint();
    auto& p2 = mFed->registerGlobalEndpoint("port2");
    mFed->addSourceFilter(p1, "filter1");
    mFed->addSourceFilter(p1, "filter2");

    auto& f1 = fFed->registerGlobalFilter("filter1");

    EXPECT_TRUE(f1.getHandle().isValid());

    auto& f2 = fFed2->registerGlobalFilter("filter2");

    EXPECT_TRUE(f2.getHandle().isValid());

    bool res = two_stage_filter_test(mFed, fFed, fFed2, p1, p2, f1, f2);
    EXPECT_TRUE(res);
}

TEST_P(filter_single_type_test, message_filter_function_two_stage_endpoint_target_dest)
{
    auto broker = AddBroker(GetParam(), 3);
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter2");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto fFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto mFed = GetFederateAs<helics::MessageFederate>(2);

    // nameless endpoint
    auto& p1 = mFed->registerEndpoint();
    auto& p2 = mFed->registerGlobalEndpoint("port2");
    mFed->addSourceFilter(p1, "filter1");
    mFed->addDestinationFilter(p2, "filter2");

    auto& f1 = fFed->registerGlobalFilter("filter1");

    EXPECT_TRUE(f1.getHandle().isValid());

    auto& f2 = fFed2->registerGlobalFilter("filter2");

    EXPECT_TRUE(f2.getHandle().isValid());

    bool res = two_stage_filter_test(mFed, fFed, fFed2, p1, p2, f1, f2);
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

    auto& p1 = mFed->registerGlobalEndpoint("port1");
    auto& p2 = mFed->registerGlobalEndpoint("port2");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    broker->addSourceFilterToEndpoint("filter1", "port1");
    broker->addDestinationFilterToEndpoint("filter2", "port2");

    auto& f1 = fFed->registerGlobalFilter("filter1");

    EXPECT_TRUE(f1.getHandle().isValid());

    auto& f2 = fFed2->registerGlobalFilter("filter2");

    EXPECT_TRUE(f2.getHandle().isValid());

    bool res = two_stage_filter_test(mFed, fFed, fFed2, p1, p2, f1, f2);
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
    auto& f1 = fFed->registerGlobalFilter("filter1");
    auto& f2 = fFed2->registerGlobalFilter("filter2");

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    broker->addSourceFilterToEndpoint("filter1", "port1");
    broker->addDestinationFilterToEndpoint("filter2", "port2");
    auto& p1 = mFed->registerGlobalEndpoint("port1");
    auto& p2 = mFed->registerGlobalEndpoint("port2");

    bool res = two_stage_filter_test(mFed, fFed, fFed2, p1, p2, f1, f2);
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

    auto& p1 = mFed->registerGlobalEndpoint("port1");
    auto& p2 = mFed->registerGlobalEndpoint("port2");

    auto& f1 = fFed->registerGlobalFilter("filter1");
    auto& f2 = fFed2->registerGlobalFilter("filter2");

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    broker->addSourceFilterToEndpoint("filter1", "port1");
    broker->addDestinationFilterToEndpoint("filter2", "port2");
    bool res = two_stage_filter_test(mFed, fFed, fFed2, p1, p2, f1, f2);
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

    auto& p1 = mFed->registerGlobalEndpoint("port1");
    auto& p2 = mFed->registerGlobalEndpoint("port2");

    auto& f1 = fFed->registerGlobalFilter("filter1");
    auto& f2 = fFed2->registerGlobalFilter("filter2");

    bool res = two_stage_filter_test(mFed, fFed, fFed2, p1, p2, f1, f2);
    EXPECT_TRUE(res);
}

/** test a filter operator
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the simulation
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
    auto& p1 = mFed->registerGlobalEndpoint("port1");
    auto& p2 = mFed->registerGlobalEndpoint("port2");

    auto& f1 = helics::make_filter(helics::filter_types::delay, fFed.get(), "filter1");
    f1.addSourceTarget("port1");
    f1.set("delay", 1.25);

    auto& f2 = helics::make_filter(helics::filter_types::delay, fFed.get(), "filter2");
    f2.addSourceTarget("port1");
    f2.set("delay", 1.25);

    fFed->enterExecutingModeAsync();
    fFed2->enterExecutingModeAsync();
    mFed->enterExecutingMode();
    fFed->enterExecutingModeComplete();
    fFed2->enterExecutingModeComplete();

    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::modes::executing);
    helics::data_block data(500, 'a');
    mFed->sendMessage(p1, "port2", data);

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
    ASSERT_TRUE(!mFed->hasMessage(p2));

    fFed->requestTimeAsync(3.0);
    fFed2->requestTimeAsync(3.0);
    /*auto retTime = */ mFed->requestTime(3.0);
    if (!mFed->hasMessage(p2)) {
        printf("missing message\n");
    }
    ASSERT_TRUE(mFed->hasMessage(p2));

    auto m2 = mFed->getMessage(p2);
    EXPECT_EQ(m2->source, "port1");
    EXPECT_EQ(m2->original_source, "port1");
    EXPECT_EQ(m2->dest, "port2");
    EXPECT_EQ(m2->data.size(), data.size());
    EXPECT_EQ(m2->time, 2.5);

    fFed->requestTimeComplete();
    fFed2->requestTimeComplete();
    auto filterCore = fFed->getCorePointer();
    auto mCore = mFed->getCorePointer();
    mFed->finalizeAsync();
    fFed->finalizeAsync();
    fFed2->finalize();
    mFed->finalizeComplete();
    fFed->finalizeComplete();
    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::modes::finalize);
    helics::cleanupHelicsLibrary();
    EXPECT_TRUE(!filterCore->isConnected());
    EXPECT_TRUE(!mCore->isConnected());
}
/** test two filter operators
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the simulation
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

    auto& p1 = mFed->registerGlobalEndpoint("port1");
    auto& p2 = mFed->registerGlobalEndpoint("port2");

    auto& f1 = fFed->registerFilter("filter1");
    fFed->addSourceTarget(f1, "port1");
    fFed->addSourceTarget(f1, "port2");
    EXPECT_TRUE(f1.getHandle().isValid());
    auto timeOperator = std::make_shared<helics::MessageTimeOperator>();
    timeOperator->setTimeFunction([](helics::Time time_in) { return time_in + 2.5; });
    fFed->setFilterOperator(f1, timeOperator);

    fFed->enterExecutingModeAsync();
    mFed->enterExecutingMode();
    fFed->enterExecutingModeComplete();

    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::modes::executing);
    helics::data_block data(500, 'a');
    mFed->sendMessage(p1, "port2", data);

    mFed->requestTimeAsync(1.0);
    fFed->requestTime(1.0);
    mFed->requestTimeComplete();

    auto res = mFed->hasMessage();
    EXPECT_TRUE(!res);
    mFed->sendMessage(p2, "port1", data);
    mFed->requestTimeAsync(2.0);
    fFed->requestTime(2.0);
    mFed->requestTimeComplete();
    ASSERT_TRUE(!mFed->hasMessage(p2));

    std::this_thread::yield();
    mFed->requestTime(3.0);

    ASSERT_TRUE(mFed->hasMessage(p2));

    auto m2 = mFed->getMessage(p2);
    EXPECT_EQ(m2->source, "port1");
    EXPECT_EQ(m2->original_source, "port1");
    EXPECT_EQ(m2->dest, "port2");
    EXPECT_EQ(m2->data.size(), data.size());
    EXPECT_EQ(m2->time, 2.5);

    EXPECT_TRUE(!mFed->hasMessage(p1));
    mFed->requestTime(4.0);
    EXPECT_TRUE(mFed->hasMessage(p1));
    mFed->finalizeAsync();
    fFed->finalize();
    mFed->finalizeComplete();
    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::modes::finalize);
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

    auto& p1 = mFed->registerGlobalEndpoint("port1");
    auto& p2 = mFed->registerGlobalEndpoint("port2");

    auto& f1 = fFed->registerFilter("filter1");
    fFed->addSourceTarget(f1, "port1");
    fFed->addSourceTarget(f1, "port2");
    EXPECT_TRUE(f1.getHandle().isValid());
    auto timeOperator = std::make_shared<helics::MessageTimeOperator>();
    timeOperator->setTimeFunction([](helics::Time time_in) { return time_in + 2.5; });
    fFed->setFilterOperator(f1, timeOperator);

    fFed->enterExecutingModeAsync();
    mFed->enterExecutingMode();
    fFed->enterExecutingModeComplete();

    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::modes::executing);
    helics::data_block data(500, 'a');
    mFed->sendMessage(p1, "port2", data);

    mFed->requestTimeAsync(1.0);
    fFed->requestTime(1.0);
    mFed->requestTimeComplete();

    auto res = mFed->hasMessage();
    EXPECT_TRUE(!res);
    mFed->sendMessage(p2, "port1", data);
    mFed->requestTimeAsync(2.0);
    fFed->requestTime(2.0);
    mFed->requestTimeComplete();
    ASSERT_TRUE(!mFed->hasMessage(p2));

    std::this_thread::yield();
    mFed->requestTime(3.0);

    ASSERT_TRUE(mFed->hasMessage(p2));

    auto m2 = mFed->getMessage(p2);
    EXPECT_EQ(m2->source, "port1");
    EXPECT_EQ(m2->original_source, "port1");
    EXPECT_EQ(m2->dest, "port2");
    EXPECT_EQ(m2->data.size(), data.size());
    EXPECT_EQ(m2->time, 2.5);

    EXPECT_TRUE(!mFed->hasMessage(p1));
    mFed->requestTime(4.0);
    EXPECT_TRUE(mFed->hasMessage(p1));
    f1.removeTarget("port1");
    mFed->requestTimeAsync(5.0);
    fFed->requestTime(5.0);
    mFed->requestTimeComplete();
    mFed->sendMessage(p1, "port2", data);

    mFed->requestTimeAsync(6.0);
    fFed->requestTime(6.0);
    mFed->requestTimeComplete();
    // now the message hasn't been delayed
    EXPECT_TRUE(mFed->hasMessage(p2));

    mFed->finalize();
    fFed->finalize();
    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::modes::finalize);
}

class filter_test: public ::testing::Test, public FederateTestFixture {
};

TEST_F(filter_test, message_clone_test)
{
    auto broker = AddBroker("test", 3);
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "source");
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "dest");
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "dest_clone");

    auto sFed = GetFederateAs<helics::MessageFederate>(0);
    auto dFed = GetFederateAs<helics::MessageFederate>(1);
    auto dcFed = GetFederateAs<helics::MessageFederate>(2);

    auto& p1 = sFed->registerGlobalEndpoint("src");
    auto& p2 = dFed->registerGlobalEndpoint("dest");
    auto& p3 = dcFed->registerGlobalEndpoint("cm");

    helics::CloningFilter cFilt(dcFed.get());
    cFilt.addSourceTarget("src");
    cFilt.addDeliveryEndpoint("cm");

    sFed->enterExecutingModeAsync();
    dcFed->enterExecutingModeAsync();
    dFed->enterExecutingMode();
    sFed->enterExecutingModeComplete();
    dcFed->enterExecutingModeComplete();

    EXPECT_TRUE(sFed->getCurrentMode() == helics::Federate::modes::executing);
    helics::data_block data(500, 'a');
    sFed->sendMessage(p1, "dest", data);

    sFed->requestTimeAsync(1.0);
    dcFed->requestTimeAsync(1.0);
    dFed->requestTime(1.0);
    sFed->requestTimeComplete();
    dcFed->requestTimeComplete();

    auto res = dFed->hasMessage();
    EXPECT_TRUE(res);

    if (res) {
        auto m2 = dFed->getMessage(p2);
        EXPECT_EQ(m2->source, "src");
        EXPECT_EQ(m2->original_source, "src");
        EXPECT_EQ(m2->dest, "dest");
        EXPECT_EQ(m2->data.size(), data.size());
    }

    // now check the message clone
    res = dcFed->hasMessage();
    EXPECT_TRUE(res);

    if (res) {
        auto m2 = dcFed->getMessage(p3);
        EXPECT_EQ(m2->source, "src");
        EXPECT_EQ(m2->original_source, "src");
        EXPECT_EQ(m2->dest, "cm");
        EXPECT_EQ(m2->original_dest, "dest");
        EXPECT_EQ(m2->data.size(), data.size());
    }

    sFed->finalizeAsync();
    dFed->finalizeAsync();
    dcFed->finalize();
    sFed->finalizeComplete();
    dFed->finalizeComplete();
    EXPECT_TRUE(sFed->getCurrentMode() == helics::Federate::modes::finalize);
}

TEST_F(filter_test, message_multi_clone_test)
{
    auto broker = AddBroker("test", 4);
    AddFederates<helics::MessageFederate>("test", 2, broker, 1.0, "source");
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "dest");
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0, "dest_clone");

    auto sFed = GetFederateAs<helics::MessageFederate>(0);
    auto sFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto dFed = GetFederateAs<helics::MessageFederate>(2);
    auto dcFed = GetFederateAs<helics::MessageFederate>(3);

    auto& p1 = sFed->registerGlobalEndpoint("src");
    auto& p2 = sFed2->registerGlobalEndpoint("src2");
    auto& p3 = dFed->registerGlobalEndpoint("dest");
    auto& p4 = dcFed->registerGlobalEndpoint("cm");

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

    EXPECT_TRUE(sFed->getCurrentMode() == helics::Federate::modes::executing);
    helics::data_block data(500, 'a');
    helics::data_block data2(400, 'b');
    sFed->sendMessage(p1, "dest", data);
    sFed2->sendMessage(p2, "dest", data2);
    sFed->requestTimeAsync(1.0);
    sFed2->requestTimeAsync(1.0);
    dcFed->requestTimeAsync(1.0);
    dFed->requestTime(1.0);
    sFed->requestTimeComplete();
    sFed2->requestTimeComplete();
    dcFed->requestTimeComplete();

    auto mcnt = dFed->pendingMessages(p3);
    EXPECT_EQ(mcnt, 2);
    auto res = dFed->hasMessage();
    EXPECT_TRUE(res);

    if (res) {
        auto m2 = dFed->getMessage(p3);
        EXPECT_EQ(m2->source, "src");
        EXPECT_EQ(m2->original_source, "src");
        EXPECT_EQ(m2->dest, "dest");
        EXPECT_EQ(m2->data.size(), data.size());
        res = dFed->hasMessage();
        EXPECT_TRUE(res);

        if (res) {
            m2 = dFed->getMessage(p3);
            EXPECT_EQ(m2->source, "src2");
            EXPECT_EQ(m2->original_source, "src2");
            EXPECT_EQ(m2->dest, "dest");
            EXPECT_EQ(m2->data.size(), data2.size());
        }
    }

    // now check the message clone
    mcnt = dcFed->pendingMessages(p4);
    EXPECT_EQ(mcnt, 2);
    res = dcFed->hasMessage();
    EXPECT_TRUE(res);

    if (res) {
        auto m2 = dcFed->getMessage(p4);
        EXPECT_EQ(m2->source, "src");
        EXPECT_EQ(m2->original_source, "src");
        EXPECT_EQ(m2->dest, "cm");
        EXPECT_EQ(m2->original_dest, "dest");
        EXPECT_EQ(m2->data.size(), data.size());
        res = dcFed->hasMessage();
        EXPECT_TRUE(res);

        if (res) {
            m2 = dcFed->getMessage(p4);
            EXPECT_EQ(m2->source, "src2");
            EXPECT_EQ(m2->original_source, "src2");
            EXPECT_EQ(m2->dest, "cm");
            EXPECT_EQ(m2->original_dest, "dest");
            EXPECT_EQ(m2->data.size(), data2.size());
        }
    }

    sFed->finalizeAsync();
    sFed2->finalizeAsync();
    dFed->finalizeAsync();
    dcFed->finalize();
    sFed->finalizeComplete();
    sFed2->finalizeComplete();
    dFed->finalizeComplete();
    EXPECT_TRUE(sFed->getCurrentMode() == helics::Federate::modes::finalize);
}

/** test whether a core termination when it should
 */

TEST_P(filter_single_type_test, test_filter_core_termination)
{
    auto broker = AddBroker(GetParam(), 2);
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto mFed = GetFederateAs<helics::MessageFederate>(1);

    auto& p1 = mFed->registerGlobalEndpoint("port1");
    auto& p2 = mFed->registerGlobalEndpoint("port2");

    auto c2 = fFed->getCorePointer();
    auto f1 = c2->registerFilter("filter1", std::string(), std::string());
    c2->addSourceTarget(f1, "port1");
    auto timeOperator = std::make_shared<helics::MessageTimeOperator>();
    timeOperator->setTimeFunction([](helics::Time time_in) { return time_in + 2.5; });
    c2->setFilterOperator(f1, timeOperator);

    fFed->enterExecutingModeAsync();
    mFed->enterExecutingMode();
    fFed->enterExecutingModeComplete();

    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::modes::executing);
    helics::data_block data(500, 'a');
    mFed->sendMessage(p1, "port2", data);

    mFed->requestTimeAsync(1.0);
    fFed->requestTime(1.0);
    mFed->requestTimeComplete();
    fFed->finalizeAsync();
    auto res = mFed->hasMessage();
    EXPECT_TRUE(!res);

    mFed->requestTime(2.0);
    ASSERT_TRUE(!mFed->hasMessage(p2));
    EXPECT_TRUE(c2->isConnected());
    mFed->requestTime(3.0);
    mFed->sendMessage(p1, "port2", data);
    ASSERT_TRUE(mFed->hasMessage(p2));

    auto m2 = mFed->getMessage(p2);
    EXPECT_EQ(m2->source, "port1");
    EXPECT_EQ(m2->original_source, "port1");
    EXPECT_EQ(m2->dest, "port2");
    EXPECT_EQ(m2->data.size(), data.size());
    EXPECT_EQ(m2->time, 2.5);

    mFed->requestTime(4.0);
    EXPECT_TRUE(!mFed->hasMessage(p2));
    mFed->requestTime(6.0);
    EXPECT_TRUE(mFed->hasMessage(p2));
    mFed->finalize();
    fFed->finalizeComplete();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    if (c2->isConnected()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }
    EXPECT_TRUE(!c2->isConnected());
    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::modes::finalize);
}

INSTANTIATE_TEST_SUITE_P(
    filter_tests,
    filter_single_type_test,
    ::testing::ValuesIn(core_types_simple));
INSTANTIATE_TEST_SUITE_P(filter_tests, filter_all_type_test, ::testing::ValuesIn(core_types_all));
