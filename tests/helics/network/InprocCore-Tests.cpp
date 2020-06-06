/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/CoreFederateInfo.hpp"
#include "helics/core/core-exceptions.hpp"
#include "helics/core/core-types.hpp"
#include "helics/core/inproc/InprocCore.h"

#include "gtest/gtest.h"

using helics::Core;
using namespace helics::CoreFactory;

TEST(InprocCore_tests, initialization_test)
{
    auto broker = helics::BrokerFactory::create(helics::core_type::INPROC, std::string{});
    ASSERT_TRUE(broker);
    EXPECT_TRUE(broker->isConnected());
    std::string configureString = std::string("-f 4") + " --broker=" + broker->getIdentifier();
    auto core = create(helics::core_type::INPROC, configureString);

    auto Tcore = std::dynamic_pointer_cast<helics::inproc::InprocCore>(core);

    ASSERT_TRUE(core);
    ASSERT_TRUE(Tcore);
    EXPECT_TRUE(core->isConfigured());

    core->connect();

    EXPECT_TRUE(core->isConnected());
    core->disconnect();
    broker->disconnect();
    EXPECT_EQ(core->isConnected(), false);
    EXPECT_EQ(broker->isConnected(), false);
    core = nullptr;
    broker = nullptr;
}

TEST(InprocCore_tests, initialization_test_with_test_broker)
{
    auto broker = helics::BrokerFactory::create(helics::core_type::TEST, std::string{});
    ASSERT_TRUE(broker);
    EXPECT_TRUE(broker->isConnected());
    std::string configureString = std::string("-f 4") + " --broker=" + broker->getIdentifier();
    auto core = create(helics::core_type::INPROC, configureString);

    auto Tcore = std::dynamic_pointer_cast<helics::inproc::InprocCore>(core);

    ASSERT_TRUE(core);
    ASSERT_TRUE(Tcore);
    EXPECT_TRUE(core->isConfigured());

    core->connect();

    EXPECT_TRUE(core->isConnected());
    core->disconnect();
    broker->disconnect();
    EXPECT_EQ(core->isConnected(), false);
    EXPECT_EQ(broker->isConnected(), false);
    core = nullptr;
    broker = nullptr;
}

TEST(InprocCore_tests, pubsub_value_test)
{
    const char* configureString = "-f 1 --autobroker";
    auto core = create(helics::core_type::INPROC, configureString);

    ASSERT_TRUE(core != nullptr);
    EXPECT_TRUE(core->isConfigured());
    EXPECT_EQ(core->getFederationSize(), 0);
    core->connect();
    ASSERT_TRUE(core->isConnected());

    auto id = core->registerFederate("sim1", helics::CoreFederateInfo());

    EXPECT_EQ(core->getFederationSize(), 1);
    EXPECT_EQ(core->getFederateName(id), "sim1");
    EXPECT_TRUE(core->getFederateId("sim1") == id);

    core->setTimeProperty(id, helics_property_time_delta, 1.0);

    auto sub1 = core->registerInput(id, "", "type", "units");
    core->addSourceTarget(sub1, "sim1_pub");
    EXPECT_EQ(core->getExtractionType(sub1), "type");
    EXPECT_EQ(core->getExtractionUnits(sub1), "units");

    auto pub1 = core->registerPublication(id, "sim1_pub", "type", "units");
    EXPECT_TRUE(core->getPublication(id, "sim1_pub") == pub1);
    EXPECT_EQ(core->getExtractionType(pub1), "type");
    EXPECT_EQ(core->getExtractionUnits(pub1), "units");

    core->enterInitializingMode(id);

    core->enterExecutingMode(id);

    core->timeRequest(id, 50.0);
    std::string str1 = "hello world";
    core->setValue(pub1, str1.data(), str1.size());
    auto valueUpdates = core->getValueUpdates(id);
    EXPECT_TRUE(valueUpdates.empty());
    auto data = core->getValue(sub1);
    EXPECT_TRUE(data == nullptr);

    core->timeRequest(id, 100.0);
    valueUpdates = core->getValueUpdates(id);
    ASSERT_EQ(valueUpdates.size(), 1u);
    EXPECT_EQ(valueUpdates[0], sub1);

    data = core->getValue(sub1);
    std::string str2(data->to_string());
    EXPECT_EQ(str1, str2);
    EXPECT_EQ(data->to_string(), "hello world");
    EXPECT_EQ(data->size(), str1.size());

    core->setValue(pub1, "hello\n\0helloAgain", 17);
    core->timeRequest(id, 150.0);
    valueUpdates = core->getValueUpdates(id);
    EXPECT_EQ(valueUpdates[0], sub1);
    EXPECT_EQ(valueUpdates.size(), 1u);
    data = core->getValue(sub1);
    EXPECT_EQ(data->to_string(), std::string("hello\n\0helloAgain", 17));
    EXPECT_EQ(data->size(), 17u);

    core->timeRequest(id, 200.0);
    valueUpdates = core->getValueUpdates(id);
    EXPECT_TRUE(valueUpdates.empty());
    core->finalize(id);
    core->disconnect();
    core = nullptr;
    helics::CoreFactory::cleanUpCores();
}

TEST(InprocCore_tests, send_receive_test)
{
    const char* initializationString =
        "--autobroker --broker=\"brk1\" --brokerinit=\"--name=brk1\"";
    auto core = create(helics::core_type::INPROC, initializationString);

    ASSERT_TRUE(core != nullptr);
    EXPECT_TRUE(core->isConfigured());

    EXPECT_EQ(core->getFederationSize(), 0);
    core->connect();
    ASSERT_TRUE(core->isConnected());
    auto id = core->registerFederate("sim1", helics::CoreFederateInfo());

    EXPECT_EQ(core->getFederateName(id), "sim1");
    EXPECT_TRUE(core->getFederateId("sim1") == id);

    core->setTimeProperty(id, helics_property_time_delta, 1.0);

    auto end1 = core->registerEndpoint(id, "end1", "type");
    EXPECT_EQ(core->getInjectionType(end1), "type");

    auto end2 = core->registerEndpoint(id, "end2", "type");
    EXPECT_EQ(core->getInjectionType(end2), "type");

    core->enterInitializingMode(id);

    core->enterExecutingMode(id);

    std::string str1 = "hello world";
    core->timeRequest(id, 50.0);
    core->send(end1, "end2", str1.data(), str1.size());

    core->timeRequest(id, 100.0);
    EXPECT_EQ(core->receiveCount(end1), 0);
    EXPECT_EQ(core->receiveCount(end2), 1u);
    auto msg = core->receive(end1);
    EXPECT_TRUE(msg == nullptr);
    msg = core->receive(end2);
    EXPECT_EQ(core->receiveCount(end2), 0);
    std::string str2(msg->data.to_string());
    EXPECT_EQ(str1, str2);
    EXPECT_EQ(msg->data.size(), str1.size());
    core->disconnect();
    core = nullptr;
    helics::CoreFactory::cleanUpCores();
}

TEST(InprocCore_tests, messagefilter_callback_test)
{
    // Create filter operator
    class TestOperator: public helics::FilterOperator {
      public:
        explicit TestOperator(const std::string& name): filterName(name) {}

        std::unique_ptr<helics::Message> process(std::unique_ptr<helics::Message> msg) override
        {
            msg->source = filterName;

            if (!msg->data.empty()) {
                ++msg->data[0];
            }
            return msg;
        }

        std::string filterName;
    };

    std::string configureString = "--autobroker";
    auto core = create(helics::core_type::INPROC, configureString);

    ASSERT_TRUE(core != nullptr);
    EXPECT_TRUE(core->isConfigured());
    core->connect();
    ASSERT_TRUE(core->isConnected());
    auto id = core->registerFederate("sim1", helics::CoreFederateInfo());

    auto end1 = core->registerEndpoint(id, "end1", "type");
    auto end2 = core->registerEndpoint(id, "end2", "type");

    auto srcFilter = core->registerFilter("srcFilter", "type", "type");
    core->addSourceTarget(srcFilter, "end1");
    auto dstFilter = core->registerFilter("dstFilter", "type", "type");
    core->addDestinationTarget(dstFilter, "end2");

    auto testSrcFilter = std::make_shared<TestOperator>("sourceFilter");
    EXPECT_EQ(testSrcFilter->filterName, "sourceFilter");

    auto testDstFilter = std::make_shared<TestOperator>("destinationFilter");
    EXPECT_EQ(testDstFilter->filterName, "destinationFilter");

    core->setFilterOperator(srcFilter, testSrcFilter);
    core->setFilterOperator(dstFilter, testDstFilter);

    core->enterInitializingMode(id);
    core->enterExecutingMode(id);

    std::string msgData = "hello world";
    core->send(end1, "end2", msgData.data(), msgData.size() + 1);

    core->timeRequest(id, 50.0);

    // Receive the filtered message
    EXPECT_EQ(core->receiveCount(end2), 1u);
    auto msg = core->receive(end2);
    EXPECT_EQ(msg->original_source, "end1");
    auto res = msg->data.to_string();
    EXPECT_EQ(res.compare(0, 11, "jello world"), 0);
    core->finalize(id);
    testSrcFilter = nullptr;
    testDstFilter = nullptr;
    core->disconnect();
    core = nullptr;
    helics::CoreFactory::cleanUpCores();
}
