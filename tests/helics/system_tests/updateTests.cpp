/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../application_api/testFixtures.hpp"
#include "helics/ValueFederates.hpp"

#include "gtest/gtest.h"
#include <array>
#include <future>
/** tests for the different flag options and considerations*/

struct update_tests: public FederateTestFixture, public ::testing::Test {
};

/** test simple creation and destruction*/
TEST_F(update_tests, test_single_update)
{
    using namespace helics;
    SetupTest<ValueFederate>("test", 2);
    auto fedA = GetFederateAs<ValueFederate>(0);
    auto fedB = GetFederateAs<ValueFederate>(1);

    // register the publications
    auto pub = fedA->registerGlobalPublication<std::string>("pub1");

    auto sub = fedB->registerSubscription("pub1");
    fedA->setProperty(helics_property_time_delta, 1.0);
    fedB->setProperty(helics_property_time_delta, 1.0);

    sub.setDefault(3.1);
    double testValue = 4.79;
    auto f1finish = std::async(std::launch::async, [&]() { fedA->enterExecutingMode(); });
    fedB->enterExecutingMode();
    f1finish.wait();
    // publish string1 at time=0.0;
    pub.publish(testValue);

    double val{0};
    sub.getValue(val);

    EXPECT_EQ(val, 3.1);

    auto f1time = std::async(std::launch::async, [&]() { return fedA->requestTime(1.0); });
    auto gtime = fedB->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(f1time.get(), 1.0);
    EXPECT_TRUE(sub.isUpdated());
    // get the value
    sub.getValue(val);
    EXPECT_TRUE(!sub.isUpdated());
    // make sure the string is what we expect
    EXPECT_EQ(val, testValue);
    sub.getValue(val);
    EXPECT_TRUE(!sub.isUpdated());

    fedA->finalizeAsync();
    fedB->finalize();
    fedA->finalizeComplete();
    helics::cleanupHelicsLibrary();
}

/** test simple creation and destruction*/
TEST_F(update_tests, test_single_update_string)
{
    using namespace helics;
    SetupTest<ValueFederate>("test", 2);
    auto fedA = GetFederateAs<ValueFederate>(0);
    auto fedB = GetFederateAs<ValueFederate>(1);

    // register the publications
    auto pub = fedA->registerGlobalPublication<double>("pub1");

    auto sub = fedB->registerSubscription("pub1");
    fedA->setProperty(helics_property_time_delta, 1.0);
    fedB->setProperty(helics_property_time_delta, 1.0);

    sub.setDefault(3.1);
    double testValue = 4.79;
    double testValue2 = 9.34;
    auto f1finish = std::async(std::launch::async, [&]() { fedA->enterExecutingMode(); });
    fedB->enterExecutingMode();
    f1finish.wait();
    // publish string1 at time=0.0;
    pub.publish(testValue);

    std::string val;
    sub.getValue(val);

    EXPECT_EQ(val, "3.100000");

    auto f1time = std::async(std::launch::async, [&]() { return fedA->requestTime(1.0); });
    auto gtime = fedB->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(f1time.get(), 1.0);
    EXPECT_TRUE(sub.isUpdated());
    // get the value
    sub.getValue(val);
    EXPECT_TRUE(!sub.isUpdated());
    // make sure the string is what we expect
    EXPECT_EQ(val, "4.790000");
    sub.getValue(val);
    EXPECT_TRUE(!sub.isUpdated());
    pub.publish(testValue2);

    f1time = std::async(std::launch::async, [&]() { return fedA->requestTime(2.0); });
    gtime = fedB->requestTime(2.0);
    EXPECT_EQ(f1time.get(), 2.0);
    EXPECT_TRUE(sub.isUpdated());
    // get the value
    sub.getValue(val);
    EXPECT_TRUE(!sub.isUpdated());
    // make sure the string is what we expect
    EXPECT_EQ(val, "9.340000");
    sub.getValue(val);
    EXPECT_TRUE(!sub.isUpdated());
    double v2{0};
    sub.getValue(v2);
    EXPECT_TRUE(!sub.isUpdated());

    fedA->finalizeAsync();
    fedB->finalize();
    fedA->finalizeComplete();
    helics::cleanupHelicsLibrary();
}

/** test simple creation and destruction*/
TEST_F(update_tests, test_single_update_vector)
{
    using namespace helics;
    SetupTest<ValueFederate>("test", 2);
    auto fedA = GetFederateAs<ValueFederate>(0);
    auto fedB = GetFederateAs<ValueFederate>(1);

    // register the publications
    auto pub = fedA->registerGlobalPublication<std::vector<double>>("pub1");

    auto sub = fedB->registerSubscription("pub1");
    fedA->setProperty(helics_property_time_delta, 1.0);
    fedB->setProperty(helics_property_time_delta, 1.0);

    sub.setDefault(3.1);
    double testValue = 4.79;
    double testValue2 = 9.34;
    auto f1finish = std::async(std::launch::async, [&]() { fedA->enterExecutingMode(); });
    fedB->enterExecutingMode();
    f1finish.wait();
    // publish string1 at time=0.0;
    pub.publish(testValue);

    std::string val;
    sub.getValue(val);

    EXPECT_EQ(val, "3.100000");

    auto f1time = std::async(std::launch::async, [&]() { return fedA->requestTime(1.0); });
    auto gtime = fedB->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(f1time.get(), 1.0);
    EXPECT_TRUE(sub.isUpdated());
    // get the value
    sub.getValue(val);
    EXPECT_TRUE(!sub.isUpdated());
    // make sure the string is what we expect
    EXPECT_EQ(val, "v1[4.790000]");
    sub.getValue(val);
    EXPECT_TRUE(!sub.isUpdated());
    pub.publish(testValue2);

    f1time = std::async(std::launch::async, [&]() { return fedA->requestTime(2.0); });
    gtime = fedB->requestTime(2.0);
    EXPECT_EQ(f1time.get(), 2.0);
    EXPECT_TRUE(sub.isUpdated());
    // get the value
    sub.getValue(val);
    EXPECT_TRUE(!sub.isUpdated());
    // make sure the string is what we expect
    EXPECT_EQ(val, "v1[9.340000]");
    sub.getValue(val);
    EXPECT_TRUE(!sub.isUpdated());
    double v2{0};
    sub.getValue(v2);
    EXPECT_TRUE(!sub.isUpdated());

    fedA->finalizeAsync();
    fedB->finalize();
    fedA->finalizeComplete();
    helics::cleanupHelicsLibrary();
}

/** test simple creation and destruction*/
TEST_F(update_tests, test_single_update_vector_char_ptr)
{
    using namespace helics;
    SetupTest<ValueFederate>("test", 2);
    auto fedA = GetFederateAs<ValueFederate>(0);
    auto fedB = GetFederateAs<ValueFederate>(1);

    // register the publications
    auto pub = fedA->registerGlobalPublication<std::vector<double>>("pub1");

    auto sub = fedB->registerSubscription("pub1");
    fedA->setProperty(helics_property_time_delta, 1.0);
    fedB->setProperty(helics_property_time_delta, 1.0);

    sub.setDefault(3.1);
    double testValue = 4.79;
    double testValue2 = 9.34;
    auto f1finish = std::async(std::launch::async, [&]() { fedA->enterExecutingMode(); });
    fedB->enterExecutingMode();
    f1finish.wait();
    // publish string1 at time=0.0;
    pub.publish(testValue);

    std::array<char, 50> val;
    sub.getValue(val.data(), 50);

    EXPECT_EQ(std::string(val.data()), "3.100000");

    auto f1time = std::async(std::launch::async, [&]() { return fedA->requestTime(1.0); });
    auto gtime = fedB->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(f1time.get(), 1.0);
    EXPECT_TRUE(sub.isUpdated());
    // get the value
    sub.getValue(val.data(), 50);
    EXPECT_TRUE(!sub.isUpdated());
    // make sure the string is what we expect
    EXPECT_EQ(std::string(val.data()), "v1[4.790000]");
    sub.getValue(val.data(), 50);
    EXPECT_TRUE(!sub.isUpdated());
    pub.publish(testValue2);

    f1time = std::async(std::launch::async, [&]() { return fedA->requestTime(2.0); });
    gtime = fedB->requestTime(2.0);
    EXPECT_EQ(f1time.get(), 2.0);
    EXPECT_TRUE(sub.isUpdated());
    // get the value
    sub.getValue(val.data(), 50);
    EXPECT_TRUE(!sub.isUpdated());
    // make sure the string is what we expect
    EXPECT_EQ(std::string(val.data()), "v1[9.340000]");
    sub.getValue(val.data(), 50);
    EXPECT_TRUE(!sub.isUpdated());
    double v2{0};
    sub.getValue(v2);
    EXPECT_TRUE(!sub.isUpdated());

    fedA->finalizeAsync();
    fedB->finalize();
    fedA->finalizeComplete();
    helics::cleanupHelicsLibrary();
}

/** test simple creation and destruction*/
TEST_F(update_tests, test_single_update_vector_ptr)
{
    using namespace helics;
    SetupTest<ValueFederate>("test", 2);
    auto fedA = GetFederateAs<ValueFederate>(0);
    auto fedB = GetFederateAs<ValueFederate>(1);

    // register the publications
    auto pub = fedA->registerGlobalPublication<std::vector<double>>("pub1");

    auto sub = fedB->registerSubscription("pub1");
    fedA->setProperty(helics_property_time_delta, 1.0);
    fedB->setProperty(helics_property_time_delta, 1.0);

    sub.setDefault(3.1);
    double testValue = 4.79;
    double testValue2 = 9.34;
    auto f1finish = std::async(std::launch::async, [&]() { fedA->enterExecutingMode(); });
    fedB->enterExecutingMode();
    f1finish.wait();
    // publish string1 at time=0.0;
    pub.publish(testValue);

    std::vector<double> val(3, 0.0);
    sub.getValue(val.data(), 3);

    EXPECT_EQ(val[0], 3.100000);

    auto f1time = std::async(std::launch::async, [&]() { return fedA->requestTime(1.0); });
    auto gtime = fedB->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(f1time.get(), 1.0);
    EXPECT_TRUE(sub.isUpdated());
    // get the value
    sub.getValue(val.data(), 3);
    EXPECT_TRUE(!sub.isUpdated());
    // make sure the string is what we expect
    EXPECT_EQ(val[0], 4.790000);
    sub.getValue(val.data(), 3);
    EXPECT_TRUE(!sub.isUpdated());
    pub.publish(testValue2);

    f1time = std::async(std::launch::async, [&]() { return fedA->requestTime(2.0); });
    gtime = fedB->requestTime(2.0);
    EXPECT_EQ(f1time.get(), 2.0);
    EXPECT_TRUE(sub.isUpdated());
    // get the value
    sub.getValue(val.data(), 3);
    EXPECT_TRUE(!sub.isUpdated());
    // make sure the string is what we expect
    EXPECT_EQ(val[0], 9.340000);
    sub.getValue(val.data(), 50);
    EXPECT_TRUE(!sub.isUpdated());
    double v2{0};
    sub.getValue(v2);
    EXPECT_TRUE(!sub.isUpdated());

    fedA->finalizeAsync();
    fedB->finalize();
    fedA->finalizeComplete();
    helics::cleanupHelicsLibrary();
}
