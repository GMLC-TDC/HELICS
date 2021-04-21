/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/MessageFederate.hpp"

#include <gtest/gtest.h>
#ifndef HELICS_SHARED_LIBRARY
#    include "testFixtures.hpp"
#else
#    include "testFixtures_shared.hpp"
#endif

#include <future>
#include <iostream>
#include <thread>
/** these test cases test out the message federates
 */

class mfed_single_type_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {
};

class mfed_type_tests: public ::testing::TestWithParam<const char*>, public FederateTestFixture {
};

class mfed_all_type_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {
};

class mfed_add_type_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {
};

class mfed_tests: public ::testing::Test, public FederateTestFixture {
};
/** test simple creation and destruction*/

TEST_P(mfed_single_type_tests, send_receive)
{
    SetupTest<helics::MessageFederate>(GetParam(), 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);

    auto epid = mFed1->registerEndpoint("ep1");
    auto epid2 = mFed1->registerGlobalEndpoint("ep2", "random");
    mFed1->setProperty(helics_property_time_delta, 1.0);

    mFed1->enterExecutingMode();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::executing);
    helics::data_block data(500, 'a');

    mFed1->sendMessage(epid, "ep2", data);

    auto time = mFed1->requestTime(1.0);
    EXPECT_EQ(time, 1.0);

    auto res = mFed1->hasMessage();
    EXPECT_TRUE(res);
    res = mFed1->hasMessage(epid);
    EXPECT_TRUE(res == false);
    res = mFed1->hasMessage(epid2);
    EXPECT_TRUE(res);

    auto M = mFed1->getMessage(epid2);
    ASSERT_TRUE(M);
    ASSERT_EQ(M->data.size(), data.size());

    EXPECT_EQ(M->data[245], data[245]);
    mFed1->finalize();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::finalize);
}

TEST_P(mfed_single_type_tests, send_receive_obj)
{
    using namespace helics;
    SetupTest<helics::MessageFederate>(GetParam(), 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);

    Endpoint epid(mFed1.get(), "ep1");

    Endpoint epid2(GLOBAL, mFed1.get(), "ep2", "random");
    mFed1->setProperty(helics_property_time_delta, 1.0);

    mFed1->enterExecutingMode();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::executing);
    helics::data_block data(500, 'a');

    epid.send("ep2", data);

    auto time = mFed1->requestTime(1.0);
    EXPECT_EQ(time, 1.0);

    auto res = mFed1->hasMessage();
    EXPECT_TRUE(res);
    res = epid.hasMessage();
    EXPECT_TRUE(res == false);
    res = epid2.hasMessage();
    EXPECT_TRUE(res);

    auto M = epid2.getMessage();
    ASSERT_TRUE(M);
    ASSERT_EQ(M->data.size(), data.size());

    EXPECT_EQ(M->data[245], data[245]);
    mFed1->finalize();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::finalize);
}

TEST_P(mfed_type_tests, send_receive_2fed)
{
    SetupTest<helics::MessageFederate>(GetParam(), 2);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    auto epid = mFed1->registerEndpoint("ep1");
    auto epid2 = mFed2->registerGlobalEndpoint("ep2", "random");

    mFed1->setProperty(helics_property_time_delta, 1.0);
    mFed2->setProperty(helics_property_time_delta, 1.0);

    auto f1finish = std::async(std::launch::async, [&]() { mFed1->enterExecutingMode(); });
    mFed2->enterExecutingMode();
    f1finish.wait();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::executing);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::modes::executing);

    helics::data_block data(500, 'a');
    helics::data_block data2(400, 'b');

    mFed1->sendMessage(epid, "ep2", data);
    mFed2->sendMessage(epid2, "fed0/ep1", data2);
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

    auto M1 = mFed1->getMessage(epid);
    ASSERT_TRUE(M1);
    ASSERT_EQ(M1->data.size(), data2.size());

    EXPECT_EQ(M1->data[245], data2[245]);

    auto M2 = mFed2->getMessage(epid2);
    ASSERT_TRUE(M2);
    ASSERT_EQ(M2->data.size(), data.size());

    EXPECT_EQ(M2->data[245], data[245]);
    mFed1->finalize();
    mFed2->finalize();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::finalize);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::modes::finalize);
}

TEST_F(mfed_tests, send_receive_2fed_extra)
{
    SetupTest<helics::MessageFederate>("test_7", 2);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto epid = mFed1->registerEndpoint("ep1");
    auto epid2 = mFed2->registerGlobalEndpoint("ep2", "random");

    mFed1->setProperty(helics_property_time_delta, 1.0);
    mFed2->setProperty(helics_property_time_delta, 1.0);
    auto f1finish = std::async(std::launch::async, [&]() { mFed1->enterExecutingMode(); });
    mFed2->enterExecutingMode();
    f1finish.wait();
    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::executing);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::modes::executing);

    helics::data_block data(500, 'a');
    helics::data_block data2(400, 'b');

    mFed1->sendMessage(epid, "ep2", data);
    mFed2->sendMessage(epid2, "fed0/ep1", data2);
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

    auto M1 = mFed1->getMessage(epid);
    ASSERT_TRUE(M1);
    ASSERT_EQ(M1->data.size(), data2.size());

    EXPECT_EQ(M1->data[245], data2[245]);

    auto M2 = mFed2->getMessage(epid2);
    ASSERT_TRUE(M2);
    ASSERT_EQ(M2->data.size(), data.size());

    EXPECT_EQ(M2->data[245], data[245]);
    mFed1->finalizeAsync();
    mFed2->finalize();
    mFed1->finalizeComplete();
    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::finalize);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::modes::finalize);
}

TEST_P(mfed_type_tests, send_receive_2fed_obj)
{
    using namespace helics;
    SetupTest<MessageFederate>(GetParam(), 2);
    auto mFed1 = GetFederateAs<MessageFederate>(0);
    auto mFed2 = GetFederateAs<MessageFederate>(1);

    Endpoint epid(mFed1, "ep1");

    Endpoint epid2(GLOBAL, mFed2.get(), "ep2", "random");

    mFed1->setProperty(helics_property_time_delta, 1.0);
    mFed2->setProperty(helics_property_time_delta, 1.0);

    auto f1finish = std::async(std::launch::async, [&]() { mFed1->enterExecutingMode(); });
    mFed2->enterExecutingMode();
    f1finish.wait();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::executing);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::modes::executing);

    helics::data_block data(500, 'a');
    helics::data_block data2(400, 'b');

    epid.send("ep2", data);
    epid2.send("fed0/ep1", data2);
    // move the time to 1.0
    auto f1time = std::async(std::launch::async, [&]() { return mFed1->requestTime(1.0); });
    auto gtime = mFed2->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(f1time.get(), 1.0);

    auto res = mFed1->hasMessage();
    EXPECT_TRUE(res);
    res = epid.hasMessage();
    EXPECT_TRUE(res);
    epid2.hasMessage();
    EXPECT_TRUE(res);

    auto M1 = epid.getMessage();
    ASSERT_TRUE(M1);
    ASSERT_EQ(M1->data.size(), data2.size());

    EXPECT_EQ(M1->data[245], data2[245]);

    auto M2 = epid2.getMessage();
    ASSERT_TRUE(M2);
    ASSERT_EQ(M2->data.size(), data.size());

    EXPECT_EQ(M2->data[245], data[245]);
    mFed1->finalizeAsync();
    mFed2->finalize();
    mFed1->finalizeComplete();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::finalize);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::modes::finalize);
}

TEST_P(mfed_all_type_tests, send_receive_2fed_multisend)
{
    SetupTest<helics::MessageFederate>(GetParam(), 2);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    auto epid = mFed1->registerEndpoint("ep1");
    auto epid2 = mFed2->registerGlobalEndpoint("ep2", "random");
    // mFed1->getCorePointer()->setLoggingLevel(0, 5);
    mFed1->setProperty(helics_property_time_delta, 1.0);
    mFed2->setProperty(helics_property_time_delta, 1.0);

    auto f1finish = std::async(std::launch::async, [&]() { mFed1->enterExecutingMode(); });
    mFed2->enterExecutingMode();
    f1finish.wait();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::executing);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::modes::executing);

    helics::data_block data1(500, 'a');
    helics::data_block data2(400, 'b');
    helics::data_block data3(300, 'c');
    helics::data_block data4(200, 'd');
    epid.setDefaultDestination("ep2");
    mFed1->sendMessage(epid, "ep2", data1);
    mFed1->sendMessage(epid, "ep2", data2);
    epid.send(data3);
    epid.send(data4);
    // move the time to 1.0
    auto f1time = std::async(std::launch::async, [&]() { return mFed1->requestTime(1.0); });
    auto gtime = mFed2->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(f1time.get(), 1.0);

    EXPECT_TRUE(!mFed1->hasMessage());

    EXPECT_TRUE(!mFed1->hasMessage(epid));
    auto cnt = mFed2->pendingMessages(epid2);
    EXPECT_EQ(cnt, 4);

    EXPECT_EQ(epid.getDefaultDestination(), "ep2");
    auto M1 = mFed2->getMessage(epid2);
    ASSERT_TRUE(M1);
    ASSERT_EQ(M1->data.size(), data1.size());

    EXPECT_EQ(M1->data[245], data1[245]);
    // check the count decremented
    cnt = mFed2->pendingMessages(epid2);
    EXPECT_EQ(cnt, 3);
    auto M2 = mFed2->getMessage();
    ASSERT_TRUE(M2);
    ASSERT_EQ(M2->data.size(), data2.size());
    EXPECT_EQ(M2->data[245], data2[245]);
    cnt = mFed2->pendingMessages(epid2);
    EXPECT_EQ(cnt, 2);

    auto M3 = mFed2->getMessage();
    auto M4 = mFed2->getMessage(epid2);
    ASSERT_TRUE(M3);
    ASSERT_TRUE(M4);
    EXPECT_EQ(M3->data.size(), data3.size());
    EXPECT_EQ(M4->data.size(), data4.size());

    EXPECT_EQ(M4->source, "fed0/ep1");
    EXPECT_EQ(M4->dest, "ep2");
    EXPECT_EQ(M4->original_source, "fed0/ep1");
    EXPECT_EQ(M4->time, 0.0);
    mFed1->finalizeAsync();
    mFed2->finalize();
    mFed1->finalizeComplete();
    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::finalize);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::modes::finalize);
}

TEST_P(mfed_all_type_tests, time_interruptions)
{
    SetupTest<helics::MessageFederate>(GetParam(), 2);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    auto epid = mFed1->registerEndpoint("ep1");
    auto epid2 = mFed2->registerGlobalEndpoint("ep2", "random");
    mFed1->setProperty(helics_property_time_delta, 1);
    mFed2->setProperty(helics_property_time_delta, 0.5);

    auto f1finish = std::async(std::launch::async, [&]() { mFed1->enterExecutingMode(); });
    mFed2->enterExecutingMode();
    f1finish.wait();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::executing);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::modes::executing);

    helics::data_block data(500, 'a');
    helics::data_block data2(400, 'b');

    mFed1->sendMessage(epid, "ep2", data);
    mFed2->sendMessage(epid2, "fed0/ep1", data2);
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
    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::finalize);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::modes::finalize);
}

INSTANTIATE_TEST_SUITE_P(mfed_tests,
                         mfed_single_type_tests,
                         ::testing::ValuesIn(core_types_single));
INSTANTIATE_TEST_SUITE_P(mfed_tests, mfed_type_tests, ::testing::ValuesIn(core_types));
INSTANTIATE_TEST_SUITE_P(mfed_tests, mfed_all_type_tests, ::testing::ValuesIn(core_types_all));
