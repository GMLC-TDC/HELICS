/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../application_api/testFixtures.hpp"

#include "gtest/gtest.h"
#include <atomic>
#include <complex>

/** these test cases test out the value converters
 */
#include "helics/helics.hpp"

#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

/** these test cases test out the message federates
 */

class mfed_type_tests: public ::testing::TestWithParam<const char*>, public FederateTestFixture {};

class mfed_tests: public ::testing::Test, public FederateTestFixture {};
/** test simple creation and destruction*/

static const auto testNamer = [](const ::testing::TestParamInfo<const char*>& parameter) {
    return std::string(parameter.param);
};

TEST_P(mfed_type_tests, big_message_10MB)
{
    if (std::string_view{GetParam()}.compare(0, 3, "ipc") == 0) {
        return;
    }
    if (std::string_view{GetParam()}.compare(0, 3, "tcp") == 0) {
        return;
    }
    if (std::string_view{GetParam()}.compare(0, 3, "udp") == 0) {
        return;
    }
    SetupTest<helics::MessageFederate>(GetParam(), 2);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    auto epid = mFed1->registerEndpoint("ep1");
    auto epid2 = mFed2->registerGlobalEndpoint("ep2", "random");

    mFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    mFed2->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);

    auto f1finish = std::async(std::launch::async, [&]() { mFed1->enterExecutingMode(); });
    mFed2->enterExecutingMode();
    f1finish.wait();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    constexpr std::size_t dataSize = 10'000'000;
    helics::SmallBuffer data(dataSize, 'a');
    helics::SmallBuffer data2(dataSize, 'b');

    epid.sendTo(data, "ep2");
    epid2.sendTo(data2, "fed0/ep1");
    // move the time to 1.0
    auto f1time = std::async(std::launch::async, [&]() { return mFed1->requestTime(1.0); });
    auto gtime = mFed2->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(f1time.get(), 1.0);

    auto res = mFed1->hasMessage();
    EXPECT_TRUE(res);
    res = mFed1->hasMessage(epid);
    EXPECT_TRUE(res);
    res = mFed2->hasMessage(epid2);
    EXPECT_TRUE(res);

    auto message1 = mFed1->getMessage(epid);
    ASSERT_TRUE(message1);
    ASSERT_EQ(message1->data.size(), data2.size());

    EXPECT_EQ(message1->data[245], data2[245]);

    auto message2 = mFed2->getMessage(epid2);
    ASSERT_TRUE(message2);
    ASSERT_EQ(message2->data.size(), data.size());

    EXPECT_EQ(message2->data[245], data[245]);
    mFed1->finalize();
    mFed2->finalize();
}

TEST_P(mfed_type_tests, big_message_50MB)
{
    if (std::string_view{GetParam()}.compare(0, 3, "ipc") == 0) {
        return;
    }
    if (std::string_view{GetParam()}.compare(0, 3, "tcp") == 0) {
        return;
    }
    if (std::string_view{GetParam()}.compare(0, 3, "udp") == 0) {
        return;
    }
    SetupTest<helics::MessageFederate>(GetParam(), 2);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    auto epid = mFed1->registerEndpoint("ep1");
    auto epid2 = mFed2->registerGlobalEndpoint("ep2", "random");

    mFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    mFed2->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);

    auto f1finish = std::async(std::launch::async, [&]() { mFed1->enterExecutingMode(); });
    mFed2->enterExecutingMode();
    f1finish.wait();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    constexpr std::size_t dataSize = 20'000'000;
    helics::SmallBuffer data(dataSize, 'a');
    helics::SmallBuffer data2(dataSize, 'b');

    epid.sendTo(data, "ep2");
    epid2.sendTo(data2, "fed0/ep1");
    // move the time to 1.0
    auto f1time = std::async(std::launch::async, [&]() { return mFed1->requestTime(1.0); });
    auto gtime = mFed2->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(f1time.get(), 1.0);

    auto res = mFed1->hasMessage();
    EXPECT_TRUE(res);
    res = mFed1->hasMessage(epid);
    EXPECT_TRUE(res);
    res = mFed2->hasMessage(epid2);
    EXPECT_TRUE(res);

    auto message1 = mFed1->getMessage(epid);
    ASSERT_TRUE(message1);
    ASSERT_EQ(message1->data.size(), data2.size());

    EXPECT_EQ(message1->data[245], data2[245]);

    auto message2 = mFed2->getMessage(epid2);
    ASSERT_TRUE(message2);
    ASSERT_EQ(message2->data.size(), data.size());

    EXPECT_EQ(message2->data[245], data[245]);
    mFed1->finalize();
    mFed2->finalize();
}

TEST_P(mfed_type_tests, big_message_200MB_ci_skip_nosan)
{
    if (std::string_view{GetParam()}.compare(0, 3, "ipc") == 0) {
        return;
    }
    if (std::string_view{GetParam()}.compare(0, 3, "tcp") == 0) {
        return;
    }
    if (std::string_view{GetParam()}.compare(0, 3, "udp") == 0) {
        return;
    }
    SetupTest<helics::MessageFederate>(GetParam(), 2);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    auto epid = mFed1->registerEndpoint("ep1");
    auto epid2 = mFed2->registerGlobalEndpoint("ep2", "random");

    mFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    mFed2->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);

    auto f1finish = std::async(std::launch::async, [&]() { mFed1->enterExecutingMode(); });
    mFed2->enterExecutingMode();
    f1finish.wait();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    constexpr std::size_t dataSize = 200'000'000;
    helics::SmallBuffer data(dataSize, 'a');
    helics::SmallBuffer data2(dataSize, 'b');

    epid.sendTo(data, "ep2");
    epid2.sendTo(data2, "fed0/ep1");
    // move the time to 1.0
    auto f1time = std::async(std::launch::async, [&]() { return mFed1->requestTime(1.0); });
    auto gtime = mFed2->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(f1time.get(), 1.0);

    auto res = mFed1->hasMessage();
    EXPECT_TRUE(res);
    res = mFed1->hasMessage(epid);
    EXPECT_TRUE(res);
    res = mFed2->hasMessage(epid2);
    EXPECT_TRUE(res);

    auto message1 = mFed1->getMessage(epid);
    ASSERT_TRUE(message1);
    ASSERT_EQ(message1->data.size(), data2.size());

    EXPECT_EQ(message1->data[24500], data2[24500]);

    auto message2 = mFed2->getMessage(epid2);
    ASSERT_TRUE(message2);
    ASSERT_EQ(message2->data.size(), data.size());

    EXPECT_EQ(message2->data[24500], data[24500]);
    mFed1->finalize();
    mFed2->finalize();
}

TEST_P(mfed_type_tests, big_message_1500MB_ci_skip__nosan)
{
    if (std::string_view{GetParam()}.compare(0, 3, "ipc") == 0) {
        return;
    }
    if (std::string_view{GetParam()}.compare(0, 3, "tcp") == 0) {
        return;
    }
    if (std::string_view{GetParam()}.compare(0, 3, "udp") == 0) {
        return;
    }
    SetupTest<helics::MessageFederate>(GetParam(), 2);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    auto epid = mFed1->registerEndpoint("ep1");
    auto epid2 = mFed2->registerGlobalEndpoint("ep2", "random");

    mFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    mFed2->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);

    auto f1finish = std::async(std::launch::async, [&]() { mFed1->enterExecutingMode(); });
    mFed2->enterExecutingMode();
    f1finish.wait();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    constexpr std::size_t dataSize = 1'500'000'000;
    helics::SmallBuffer data(dataSize, 'a');
    helics::SmallBuffer data2(dataSize, 'b');

    epid.sendTo(data, "ep2");
    epid2.sendTo(data2, "fed0/ep1");
    // move the time to 1.0
    auto f1time = std::async(std::launch::async, [&]() { return mFed1->requestTime(1.0); });
    auto gtime = mFed2->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(f1time.get(), 1.0);

    auto res = mFed1->hasMessage();
    EXPECT_TRUE(res);
    res = mFed1->hasMessage(epid);
    EXPECT_TRUE(res);
    res = mFed2->hasMessage(epid2);
    EXPECT_TRUE(res);

    auto message1 = mFed1->getMessage(epid);
    ASSERT_TRUE(message1);
    ASSERT_EQ(message1->data.size(), data2.size());

    EXPECT_EQ(message1->data[24500], data2[24500]);

    auto message2 = mFed2->getMessage(epid2);
    ASSERT_TRUE(message2);
    ASSERT_EQ(message2->data.size(), data.size());

    EXPECT_EQ(message2->data[24500], data[24500]);
    mFed1->finalize();
    mFed2->finalize();
}

INSTANTIATE_TEST_SUITE_P(mfed_tests, mfed_type_tests, ::testing::ValuesIn(CoreTypes_2), testNamer);
