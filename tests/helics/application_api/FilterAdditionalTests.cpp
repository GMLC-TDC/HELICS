/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/FilterOperations.hpp"
#include "helics/application_api/Filters.hpp"
#include "helics/application_api/MessageFederate.hpp"
#include "helics/application_api/MessageOperators.hpp"
#include "testFixtures.hpp"

#include <future>
#include <gtest/gtest.h>
#include <helics/core/Broker.hpp>
#include <memory>
#include <string>
#include <thread>

/** these test cases test out the message federates
 */

/**
Test rerouter filter
This test case sets reroute filter on a source endpoint. This means message
sent from this endpoint will be rerouted to a new destination endpoint.
*/

static const auto testNamer = [](const ::testing::TestParamInfo<const char*>& parameter) {
    return std::string(parameter.param);
};

class filter_type_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {};

TEST_P(filter_type_tests, message_reroute_filter_object1)
{
    auto broker = AddBroker(GetParam(), 2);

    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto mFed = GetFederateAs<helics::MessageFederate>(1);
    fFed->setProperty(HELICS_PROPERTY_TIME_GRANT_TIMEOUT, 1.0);
    mFed->setProperty(HELICS_PROPERTY_TIME_GRANT_TIMEOUT, 1.0);
    auto& port1 = mFed->registerGlobalEndpoint("port1");
    auto& port2 = mFed->registerGlobalEndpoint("port2");
    auto& port3 = mFed->registerGlobalEndpoint("port3");

    auto& Filt = helics::make_filter(helics::FilterTypes::REROUTE, fFed.get(), "filter1");
    Filt.addSourceTarget("port1");
    Filt.setString("newdestination", "port3");

    fFed->enterExecutingModeAsync();
    mFed->enterExecutingMode();
    fFed->enterExecutingModeComplete();

    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    helics::SmallBuffer data(500, 'a');
    port1.sendTo(data, "port2");

    mFed->requestTimeAsync(1.0);
    fFed->requestTime(1.0);
    // std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    // auto res = broker->query("root", "global_time_debugging");
    // std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    // std::this_thread::yield();
    // auto res2 = broker->query("root", "global_time_debugging");
    mFed->requestTimeComplete();

    EXPECT_TRUE(!mFed->hasMessage(port2));
    EXPECT_TRUE(mFed->hasMessage(port3));

    auto message2 = mFed->getMessage(port3);
    if (message2) {
        EXPECT_EQ(message2->source, "port1");
        EXPECT_EQ(message2->original_dest, "port2");
        EXPECT_EQ(message2->dest, "port3");
        EXPECT_EQ(message2->data.size(), data.size());
    }

    fFed->requestTimeAsync(2.0);
    mFed->requestTime(2.0);
    fFed->requestTimeComplete();

    mFed->finalizeAsync();
    fFed->disconnect();
    mFed->finalizeComplete();
    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

TEST_P(filter_type_tests, message_reroute_filter_object1_close_ci_skip)
{
    auto broker = AddBroker(GetParam(), 2);

    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto mFed = GetFederateAs<helics::MessageFederate>(1);

    auto& port1 = mFed->registerGlobalEndpoint("port1");
    auto& port2 = mFed->registerGlobalEndpoint("port2");
    auto& port3 = mFed->registerGlobalEndpoint("port3");

    auto& Filt = helics::make_filter(helics::FilterTypes::REROUTE, fFed.get(), "filter1");
    Filt.addSourceTarget("port1");
    Filt.setString("newdestination", "port3");

    fFed->enterExecutingModeAsync();
    mFed->enterExecutingMode();
    fFed->enterExecutingModeComplete();

    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    helics::SmallBuffer data(500, 'a');
    port1.sendTo(data, "port2");

    mFed->requestTimeAsync(1.0);
    fFed->requestTime(1.0);
    mFed->requestTimeComplete();

    EXPECT_TRUE(!mFed->hasMessage(port2));
    ASSERT_TRUE(mFed->hasMessage(port3));

    auto message2 = mFed->getMessage(port3);
    EXPECT_EQ(message2->source, "port1");
    EXPECT_EQ(message2->original_dest, "port2");
    EXPECT_EQ(message2->dest, "port3");
    EXPECT_EQ(message2->data.size(), data.size());

    Filt.close();
    port1.sendTo(data, "port2");

    fFed->requestTimeAsync(2.0);
    mFed->requestTime(2.0);
    fFed->requestTimeComplete();

    EXPECT_TRUE(mFed->hasMessage(port2));
    EXPECT_TRUE(!mFed->hasMessage(port3));

    message2 = mFed->getMessage(port2);
    EXPECT_TRUE(message2);
    if (message2) {
        EXPECT_EQ(message2->dest, "port2");
        EXPECT_EQ(message2->data.size(), data.size());
    }

    mFed->finalize();
    fFed->finalize();
    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

/**
Test rerouter filter under condition
This test case sets reroute filter on a source endpoint with a condition parameter.
This means message sent from this endpoint will be rerouted to a new destination
endpoint only if condition matches.
*/
TEST_P(filter_type_tests, message_reroute_filter_condition)
{
    // debugDiagnostic = true;
    auto broker = AddBroker(GetParam(), 2);
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto mFed = GetFederateAs<helics::MessageFederate>(1);
    // for debugging spurious timeouts;
    fFed->setProperty(HELICS_PROPERTY_TIME_GRANT_TIMEOUT, 0.5);
    mFed->setProperty(HELICS_PROPERTY_TIME_GRANT_TIMEOUT, 0.5);
    auto& port1 = mFed->registerGlobalEndpoint("port1");
    auto& port2 = mFed->registerGlobalEndpoint("endpt2");
    auto& port3 = mFed->registerGlobalEndpoint("port3");

    auto& filter1 = fFed->registerFilter("filter1");
    filter1.addSourceTarget("port1");
    auto filter_op = std::make_shared<helics::RerouteFilterOperation>();
    filter_op->setString("newdestination", "port3");
    filter_op->setString("condition",
                         "end");  // match all messages with a destination endpoint stating with
    // "end"

    filter1.setOperator(filter_op->getOperator());

    fFed->enterExecutingModeAsync();
    mFed->enterExecutingMode();
    fFed->enterExecutingModeComplete();

    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    helics::SmallBuffer data(500, 'a');
    port1.sendTo(data, "endpt2");

    mFed->requestTimeAsync(1.0);
    fFed->requestTime(1.0);
    mFed->requestTimeComplete();
    EXPECT_TRUE(!mFed->hasMessage(port2));
    ASSERT_TRUE(mFed->hasMessage(port3));
    auto message2 = mFed->getMessage(port3);

    EXPECT_EQ(message2->source, "port1");
    EXPECT_EQ(message2->dest, "port3");
    EXPECT_EQ(message2->data.size(), data.size());

    fFed->requestTimeAsync(2.0);
    mFed->requestTime(2.0);
    fFed->requestTimeComplete();

    mFed->finalizeAsync();
    fFed->finalize();
    mFed->finalizeComplete();
    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

/**
Test rerouter filter
This test case sets reroute filter on a destination endpoint. This means message
sent to this endpoint will be rerouted to a new destination endpoint.
*/

TEST_P(filter_type_tests, message_reroute_filter_object2_ci_skip)
{
    auto broker = AddBroker(GetParam(), 2);

    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto mFed = GetFederateAs<helics::MessageFederate>(1);

    auto& port1 = mFed->registerGlobalEndpoint("port1");
    auto& port2 = mFed->registerGlobalEndpoint("port2");
    auto& port3 = mFed->registerGlobalEndpoint("port3");

    auto& filter1 = fFed->registerFilter("filter1");
    filter1.addSourceTarget("port1");
    auto filter_op = std::make_shared<helics::RerouteFilterOperation>();
    filter_op->setString("newdestination", "port3");
    filter_op->setString(
        "condition",
        "test");  // match all messages with a destination endpoint starting with "test"

    EXPECT_EQ(filter_op->getString("condition"), "test");
    filter1.setOperator(filter_op->getOperator());
    EXPECT_TRUE(filter1.getString("unknown").empty());
    fFed->enterExecutingModeAsync();
    mFed->enterExecutingMode();
    fFed->enterExecutingModeComplete();

    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    helics::SmallBuffer data(500, 'a');
    port1.sendTo(data, "port2");

    mFed->requestTimeAsync(1.0);
    fFed->requestTime(1.0);
    mFed->requestTimeComplete();
    // this one was delivered to the original destination
    ASSERT_TRUE(mFed->hasMessage(port2));

    // this message should be delivered to the rerouted destination
    port1.sendTo(data, "test324525");

    mFed->requestTimeAsync(2.0);
    fFed->requestTime(2.0);
    mFed->requestTimeComplete();
    if (!mFed->hasMessage(port3)) {
        EXPECT_TRUE(mFed->hasMessage(port3));
    } else {
        auto message2 = mFed->getMessage(port3);
        EXPECT_EQ(message2->source, "port1");
        EXPECT_EQ(message2->dest, "port3");
        EXPECT_EQ(message2->data.size(), data.size());
    }

    mFed->finalizeAsync();
    fFed->finalize();
    mFed->finalizeComplete();
    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

/**
Test random drop filter
This test case sets random drop filter on a source endpoint with a particular
message drop probability. This means messages may be dropped randomly with a
probability of 0.75.
*/
TEST_P(filter_type_tests, message_random_drop_object_ci_skip)
{
    auto broker = AddBroker(GetParam(), 2);
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto mFed = GetFederateAs<helics::MessageFederate>(1);

    auto& port1 = mFed->registerGlobalEndpoint("port1");
    auto& port2 = mFed->registerGlobalEndpoint("port2");

    auto& Filt = helics::make_filter(helics::FilterTypes::RANDOM_DROP, fFed.get(), "filter1");
    Filt.addSourceTarget("port1");
    double drop_prob = 0.75;
    Filt.set("dropprob", drop_prob);
    EXPECT_DOUBLE_EQ(Filt.getProperty("dropprob"), 0.75);
    EXPECT_DOUBLE_EQ(Filt.getProperty("who_cares"), helics::invalidDouble);
    fFed->enterExecutingModeAsync();
    mFed->enterExecutingMode();
    fFed->enterExecutingModeComplete();

    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    helics::SmallBuffer data(100, 'a');

    double timestep = 0.0;  // 1 second
    int max_iterations = 200;
    int dropped = 0;
    for (int i = 0; i < max_iterations; i++) {
        port1.sendTo(data, "port2");
        timestep += 1.0;
        mFed->requestTime(timestep);
        // Check if message is received
        if (!mFed->hasMessage(port2)) {
            dropped++;
        } else {
            mFed->getMessage(port2);
        }
    }
    auto iterations = static_cast<double>(max_iterations);
    double pest = static_cast<double>(dropped) / iterations;
    // this should result in an expected error of 1 in 10K tests
    double ebar = 4.5 * std::sqrt(drop_prob * (1.0 - drop_prob) / iterations);

    EXPECT_GE(pest, drop_prob - ebar);
    EXPECT_LE(pest, drop_prob + ebar);
    mFed->finalizeAsync();
    fFed->finalize();
    mFed->finalizeComplete();
    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

/**
Test random drop filter
This test case sets random drop filter on a source endpoint with a particular
message arrival probability. This means messages may be received randomly with a
probability of 0.9.
*/
TEST_P(filter_type_tests, message_random_drop_object1_ci_skip)
{
    auto broker = AddBroker(GetParam(), 2);
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto mFed = GetFederateAs<helics::MessageFederate>(1);

    auto& port1 = mFed->registerGlobalEndpoint("port1");
    auto& port2 = mFed->registerGlobalEndpoint("port2");

    auto& filter1 = fFed->registerFilter("filter1");
    filter1.addSourceTarget("port1");
    auto randomOp = std::make_shared<helics::RandomDropFilterOperation>();
    double prob = 0.45;
    randomOp->set("prob", prob);
    EXPECT_DOUBLE_EQ(randomOp->getProperty("prob"), prob);
    filter1.setOperator(randomOp->getOperator());
    fFed->enterExecutingModeAsync();
    mFed->enterExecutingMode();
    fFed->enterExecutingModeComplete();

    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    helics::SmallBuffer data(100, 'a');

    double timestep = 0.0;  // 1 second
    int max_iterations = 150;
    int count = 0;
    for (int i = 0; i < max_iterations; i++) {
        port1.sendTo(data, "port2");
        timestep += 1.0;
        mFed->requestTime(timestep);
        // Check if message is received
        if (mFed->hasMessage(port2)) {
            count++;
            mFed->getMessage(port2);
        }
    }
    auto iterations = static_cast<double>(max_iterations);
    double pest = 1.0 - static_cast<double>(count) / iterations;
    // this should result in an expected error of 1 in 10K tests
    double ebar = 4.5 * std::sqrt(prob * (1.0 - prob) / iterations);

    EXPECT_GE(pest, prob - ebar);
    EXPECT_LE(pest, prob + ebar);
    mFed->finalizeAsync();
    fFed->finalize();
    mFed->finalizeComplete();
    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

/**
Test random drop filter
This test case sets random drop filter on a destination endpoint with a particular
message drop probability. This means messages may be dropped randomly with a
probability of 0.75.
*/
TEST_P(filter_type_tests, message_random_drop_dest_object_ci_skip)
{
    auto broker = AddBroker(GetParam(), 2);
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto mFed = GetFederateAs<helics::MessageFederate>(1);

    auto& port1 = mFed->registerGlobalEndpoint("port1");
    auto& port2 = mFed->registerGlobalEndpoint("port2");

    auto& Filt = helics::make_filter(helics::FilterTypes::RANDOM_DROP, fFed.get(), "filter1");
    Filt.addDestinationTarget("port2");
    double drop_prob = 0.25;
    Filt.set("dropprob", drop_prob);

    fFed->enterExecutingModeAsync();
    mFed->enterExecutingMode();
    fFed->enterExecutingModeComplete();

    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    helics::SmallBuffer data(100, 'a');

    double timestep = 0.0;  // 1 second
    int max_iterations = 150;
    int dropped = 0;
    for (int i = 0; i < max_iterations; i++) {
        port1.sendTo(data, "port2");
        timestep += 1.0;
        mFed->requestTime(timestep);
        // Check if message is received
        if (!mFed->hasMessage(port2)) {
            dropped++;
        } else {
            // purposely dropping the messages
            mFed->getMessage(port2);
        }
    }

    auto iterations = static_cast<double>(max_iterations);
    double pest = static_cast<double>(dropped) / iterations;
    // this should result in an expected error of 1 in 10K tests
    double ebar = 4.5 * std::sqrt(drop_prob * (1.0 - drop_prob) / iterations);

    EXPECT_GE(pest, drop_prob - ebar);
    EXPECT_LE(pest, drop_prob + ebar);
    mFed->finalizeAsync();
    fFed->finalize();
    mFed->finalizeComplete();
}

/**
Test random drop filter
This test case sets random drop filter on a destination endpoint with a particular
message arrival probability. This means messages may be received randomly with a
probability of 0.9.
*/
TEST_P(filter_type_tests, message_random_drop_dest_object1_ci_skip)
{
    auto broker = AddBroker(GetParam(), 2);
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto mFed = GetFederateAs<helics::MessageFederate>(1);

    auto& port1 = mFed->registerGlobalEndpoint("port1");
    auto& port2 = mFed->registerGlobalEndpoint("port2");

    auto& filter1 = fFed->registerFilter("filter1");
    filter1.addDestinationTarget("port2");
    auto filterOp = std::make_shared<helics::RandomDropFilterOperation>();
    double prob = 0.1;
    filterOp->set("prob", prob);
    filter1.setOperator(filterOp->getOperator());
    fFed->enterExecutingModeAsync();
    mFed->enterExecutingMode();
    fFed->enterExecutingModeComplete();

    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    helics::SmallBuffer data(500, 'a');

    double timestep = 0.0;  // 1 second
    int max_iterations = 150;
    int count = 0;
    for (int ii = 0; ii < max_iterations; ii++) {
        port1.sendTo(data, "port2");
        timestep++;
        mFed->requestTime(timestep);
        if (mFed->hasMessage(port2)) {
            count++;
            mFed->getMessage(port2);
        }
    }
    auto iterations = static_cast<double>(max_iterations);
    double pest = 1.0 - static_cast<double>(count) / iterations;
    // this should result in an expected error of 1 in 10K tests
    double ebar = 4.5 * std::sqrt(prob * (1.0 - prob) / iterations);

    EXPECT_GE(pest, prob - ebar);
    EXPECT_LE(pest, prob + ebar);
    mFed->finalizeAsync();
    fFed->finalize();
    mFed->finalizeComplete();
}

/**
Test random delay filter
This test case sets random delay filter on a source endpoint.
This means messages may be delayed by random delay based on
binomial distribution.
*/
TEST_P(filter_type_tests, message_random_delay_object_ci_skip)
{
    auto broker = AddBroker(GetParam(), 2);
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto mFed = GetFederateAs<helics::MessageFederate>(1);

    auto& port1 = mFed->registerGlobalEndpoint("port1");
    auto& port2 = mFed->registerGlobalEndpoint("port2");

    auto& Filt = helics::make_filter(helics::FilterTypes::RANDOM_DELAY, fFed.get(), "filter1");
    Filt.addSourceTarget("port1");
    Filt.setString("distribution", "binomial");

    Filt.set("param1", 4);  // max_delay=4
    Filt.set("param2", 0.5);  // prob

    EXPECT_EQ(Filt.getString("param1"), "4.000000");
    EXPECT_DOUBLE_EQ(Filt.getProperty("param2"), 0.5);  // prob

    fFed->enterExecutingModeAsync();
    mFed->enterExecutingMode();
    fFed->enterExecutingModeComplete();

    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    helics::SmallBuffer data(100, 'a');
    port1.sendTo(data, "port2");

    double timestep = 0.0;  // 1 second
    int max_iterations = 6;
    int count = 0;
    double actual_delay = 100.0;

    for (int i = 0; i < max_iterations; i++) {
        timestep += 1.0;
        mFed->requestTime(timestep);
        // Check if message is received
        if (mFed->hasMessage(port2)) {
            auto message2 = mFed->getMessage(port2);
            EXPECT_EQ(message2->source, "port1");
            EXPECT_EQ(message2->dest, "port2");
            EXPECT_EQ(message2->data.size(), data.size());
            actual_delay = message2->time;
            count++;
        }
    }
    EXPECT_EQ(count, 1);
    EXPECT_LE(actual_delay, 4);

    mFed->finalizeAsync();
    fFed->finalize();
    mFed->finalizeComplete();
    EXPECT_TRUE(fFed->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

/**
Test filter info fields
*/
TEST_P(filter_type_tests, test_filter_info_field_ci_skip_nocov)
{
    auto broker = AddBroker(GetParam(), 2);
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(GetParam(), 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto mFed = GetFederateAs<helics::MessageFederate>(1);

    auto& port1 = mFed->registerGlobalEndpoint("port1");
    auto& port2 = mFed->registerGlobalEndpoint("port2");

    port1.setInfo("p1_info");
    port2.setInfo("p2_info");

    auto& filter1 = fFed->registerFilter("filter1");
    filter1.addSourceTarget("port1");
    filter1.setInfo("f1_info");
    auto& filter2 = fFed->registerFilter("filter2");
    filter2.addDestinationTarget("port2");
    filter2.setInfo("f2_info");
    auto& ep1 = fFed->registerEndpoint("fout");
    ep1.setInfo("ep1_info");
    auto& filter3 = fFed->registerFilter();
    filter3.addSourceTarget("filter0/fout");
    filter3.setInfo("f3_info");

    // Test Endpoint info field
    EXPECT_EQ("p1_info", port1.getInfo());
    EXPECT_EQ("p2_info", port2.getInfo());
    EXPECT_EQ("ep1_info", ep1.getInfo());
    EXPECT_EQ("p1_info", port1.getInfo());
    EXPECT_EQ("p2_info", port2.getInfo());
    EXPECT_EQ("ep1_info", ep1.getInfo());

    // Test Filter info field
    EXPECT_EQ("f1_info", filter1.getInfo());
    EXPECT_EQ("f2_info", filter2.getInfo());
    EXPECT_EQ("f3_info", filter3.getInfo());
    EXPECT_EQ("f1_info", filter1.getInfo());
    EXPECT_EQ("f2_info", filter2.getInfo());
    EXPECT_EQ("f3_info", filter3.getInfo());

    mFed->finalizeAsync();
    fFed->finalize();
    mFed->finalizeComplete();
}

INSTANTIATE_TEST_SUITE_P(filter, filter_type_tests, ::testing::ValuesIn(CoreTypes), testNamer);
