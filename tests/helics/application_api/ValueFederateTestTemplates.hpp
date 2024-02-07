/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "helics/application_api/Subscriptions.hpp"
#include "helics/application_api/ValueFederate.hpp"
#ifndef HELICS_SHARED_LIBRARY
#    include "testFixtures.hpp"
#else
#    include "testFixtures_shared.hpp"
#endif
#include <future>
#include <gtest/gtest.h>
#include <string>

template<class X>
void runFederateTest(const std::string& CoreType_str,
                     const X& defaultValue,
                     const X& testValue1,
                     const X& testValue2)
{
    FederateTestFixture fixture;

    fixture.SetupTest<helics::ValueFederate>(CoreType_str, 1);
    auto vFed = fixture.GetFederateAs<helics::ValueFederate>(0);

    // register the publications
    auto& pubid = vFed->registerGlobalPublication<X>("pub1");

    auto& subid = vFed->registerSubscription("pub1");
    vFed->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    subid.setDefault(defaultValue);
    vFed->enterExecutingMode();
    // publish string1 at time=0.0;
    pubid.publish(testValue1);

    auto val = subid.getValue<X>();

    EXPECT_EQ(val, defaultValue);

    auto gtime = vFed->requestTime(1.0);
    EXPECT_EQ(gtime, 1.0);
    // get the value
    subid.getValue(val);
    // make sure the string is what we expect
    EXPECT_EQ(val, testValue1);
    // publish a second string
    pubid.publish(testValue2);
    // make sure the value is still what we expect
    subid.getValue(val);
    EXPECT_EQ(val, testValue1);

    // advance time
    gtime = vFed->requestTime(2.0);
    // make sure the value was updated
    EXPECT_EQ(gtime, 2.0);
    subid.getValue(val);
    EXPECT_EQ(val, testValue2);

    vFed->finalize();
    EXPECT_TRUE(vFed->getCurrentMode() == helics::Federate::Modes::FINALIZE);
    helics::cleanupHelicsLibrary();
}

template<class X>
void runFederateTestObj(const std::string& CoreType_str,
                        const X& defaultValue,
                        const X& testValue1,
                        const X& testValue2)
{
    FederateTestFixture fixture;

    fixture.SetupTest<helics::ValueFederate>(CoreType_str, 1);
    auto vFed = fixture.GetFederateAs<helics::ValueFederate>(0);

    // register the publications
    auto& pubid = vFed->registerGlobalPublication<X>("pub1");

    auto& subid = vFed->registerSubscription("pub1");

    vFed->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    subid.setDefault(defaultValue);
    vFed->enterExecutingMode();
    // publish string1 at time=0.0;
    pubid.publish(testValue1);
    X val;
    subid.getValue(val);
    EXPECT_EQ(val, defaultValue);

    auto gtime = vFed->requestTime(1.0);
    EXPECT_EQ(gtime, 1.0);
    // get the value
    subid.getValue(val);
    // make sure the string is what we expect
    EXPECT_EQ(val, testValue1);
    // publish a second string
    pubid.publish(testValue2);
    // make sure the value is still what we expect
    val = subid.getValue<X>();
    EXPECT_EQ(val, testValue1);

    // advance time
    gtime = vFed->requestTime(2.0);
    // make sure the value was updated
    EXPECT_EQ(gtime, 2.0);
    val = subid.getValue<X>();
    EXPECT_EQ(val, testValue2);

    vFed->finalize();
}

template<class X>
void runFederateTestv2(const std::string& CoreType_str,
                       const X& defaultValue,
                       const X& testValue1,
                       const X& testValue2)
{
    FederateTestFixture fixture;

    fixture.SetupTest<helics::ValueFederate>(CoreType_str, 1);
    auto vFed = fixture.GetFederateAs<helics::ValueFederate>(0);

    // register the publications
    auto& pubid = vFed->registerGlobalPublication<X>("pub1");

    auto& subid = vFed->registerSubscription("pub1");
    vFed->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    subid.setDefault(defaultValue);
    vFed->enterExecutingMode();
    // publish string1 at time=0.0;
    pubid.publish(testValue1);

    X val = subid.getValue<X>();
    EXPECT_TRUE(val == defaultValue);

    auto gtime = vFed->requestTime(1.0);
    EXPECT_EQ(gtime, 1.0);
    // get the value
    subid.getValue(val);
    // make sure the string is what we expect

    EXPECT_TRUE(val == testValue1);
    // publish a second string
    pubid.publish(testValue2);
    // make sure the value is still what we expect
    subid.getValue(val);

    EXPECT_TRUE(val == testValue1);
    // advance time
    gtime = vFed->requestTime(2.0);
    // make sure the value was updated
    EXPECT_EQ(gtime, 2.0);
    subid.getValue(val);
    EXPECT_TRUE(val == testValue2);
    vFed->finalize();
    helics::cleanupHelicsLibrary();
}

template<class X>
void runDualFederateTest(const std::string& CoreType_str,
                         const X& defaultValue,
                         const X& testValue1,
                         const X& testValue2)
{
    FederateTestFixture fixture;

    fixture.SetupTest<helics::ValueFederate>(CoreType_str, 2);
    auto fedA = fixture.GetFederateAs<helics::ValueFederate>(0);
    auto fedB = fixture.GetFederateAs<helics::ValueFederate>(1);

    // register the publications
    auto& pubid = fedA->registerGlobalPublication<X>("pub1");

    auto& subid = fedB->registerSubscription("pub1");
    fedA->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    fedB->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);

    subid.setDefault(defaultValue);

    auto f1finish = std::async(std::launch::async, [&]() { fedA->enterExecutingMode(); });
    fedB->enterExecutingMode();
    f1finish.wait();
    // publish string1 at time=0.0;
    pubid.publish(testValue1);

    X val = subid.getValue<X>();

    EXPECT_EQ(val, defaultValue);

    auto f1time = std::async(std::launch::async, [&]() { return fedA->requestTime(1.0); });
    auto gtime = fedB->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(f1time.get(), 1.0);
    // get the value
    subid.getValue(val);
    // make sure the string is what we expect
    EXPECT_EQ(val, testValue1);

    // publish a second string
    pubid.publish(testValue2);
    // make sure the value is still what we expect
    subid.getValue(val);

    EXPECT_EQ(val, testValue1);

    // advance time
    f1time = std::async(std::launch::async, [&]() { return fedA->requestTime(2.0); });
    gtime = fedB->requestTime(2.0);

    EXPECT_EQ(gtime, 2.0);
    EXPECT_EQ(f1time.get(), 2.0);

    // make sure the value was updated
    subid.getValue(val);
    EXPECT_EQ(val, testValue2);
    fedA->finalizeAsync();
    fedB->finalize();
    fedA->finalizeComplete();
    helics::cleanupHelicsLibrary();
}

template<class X>
void runDualFederateTestv2(const std::string& CoreType_str,
                           X& defaultValue,
                           const X& testValue1,
                           const X& testValue2)
{
    FederateTestFixture fixture;

    fixture.SetupTest<helics::ValueFederate>(CoreType_str, 2);
    auto fedA = fixture.GetFederateAs<helics::ValueFederate>(0);
    auto fedB = fixture.GetFederateAs<helics::ValueFederate>(1);

    // register the publications
    auto& pubid = fedA->registerGlobalPublication<X>("pub1");

    auto& subid = fedB->registerSubscription("pub1");
    fedA->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    fedB->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);

    subid.setDefault(defaultValue);
    auto f1finish = std::async(std::launch::async, [&]() { fedA->enterExecutingMode(); });
    fedB->enterExecutingMode();
    f1finish.wait();
    // publish string1 at time=0.0;
    pubid.publish(testValue1);
    X val = subid.getValue<X>();
    EXPECT_TRUE(val == defaultValue);
    auto f1time = std::async(std::launch::async, [&]() { return fedA->requestTime(1.0); });
    auto gtime = fedB->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(f1time.get(), 1.0);
    // get the value
    subid.getValue(val);
    // make sure the string is what we expect
    EXPECT_TRUE(val == testValue1);
    // publish a second string
    pubid.publish(testValue2);
    // make sure the value is still what we expect
    subid.getValue(val);
    EXPECT_TRUE(val == testValue1);
    // advance time
    f1time = std::async(std::launch::async, [&]() { return fedA->requestTime(2.0); });
    gtime = fedB->requestTime(2.0);

    EXPECT_EQ(gtime, 2.0);
    EXPECT_EQ(f1time.get(), 2.0);

    // make sure the value was updated
    subid.getValue(val);
    EXPECT_TRUE(val == testValue2);
    fedA->finalizeAsync();
    fedB->finalize();
    fedA->finalizeComplete();
    helics::cleanupHelicsLibrary();
}

template<class X>
void runDualFederateTestJson(const std::string& core_type_str,
                             const X& defaultValue,
                             const X& testValue1,
                             const X& testValue2)
{
    FederateTestFixture fixture;

    fixture.SetupTest<helics::ValueFederate>(core_type_str, 1);
    auto fedA = fixture.GetFederateAs<helics::ValueFederate>(0);
    fixture.extraCoreArgs = "--json";
    fixture.AddFederates<helics::ValueFederate>(core_type_str, 1, fixture.brokers[0]);
    auto fedB = fixture.GetFederateAs<helics::ValueFederate>(1);
    fedB->setFlagOption(HELICS_FLAG_USE_JSON_SERIALIZATION);

    // register the publications
    auto& pubid = fedA->registerGlobalPublication<X>("pub1");

    auto& subid = fedB->registerSubscription("pub1");
    fedA->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    fedB->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);

    EXPECT_TRUE(fedB->getFlagOption(HELICS_FLAG_USE_JSON_SERIALIZATION));
    subid.setDefault(defaultValue);

    auto f1finish = std::async(std::launch::async, [&]() { fedA->enterExecutingMode(); });
    fedB->enterExecutingMode();
    f1finish.wait();
    // publish string1 at time=0.0;
    pubid.publish(testValue1);

    X val = subid.getValue<X>();

    EXPECT_EQ(val, defaultValue);

    auto f1time = std::async(std::launch::async, [&]() { return fedA->requestTime(1.0); });
    auto gtime = fedB->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(f1time.get(), 1.0);
    // get the value
    subid.getValue(val);
    // make sure the string is what we expect
    EXPECT_EQ(val, testValue1);

    // publish a second string
    pubid.publish(testValue2);
    // make sure the value is still what we expect
    subid.getValue(val);

    EXPECT_EQ(val, testValue1);

    // advance time
    f1time = std::async(std::launch::async, [&]() { return fedA->requestTime(2.0); });
    gtime = fedB->requestTime(2.0);

    EXPECT_EQ(gtime, 2.0);
    EXPECT_EQ(f1time.get(), 2.0);

    // make sure the value was updated
    subid.getValue(val);
    EXPECT_EQ(val, testValue2);
    fedA->finalizeAsync();
    fedB->finalize();
    fedA->finalizeComplete();
    helics::cleanupHelicsLibrary();
}

template<class X>
void runDualFederateTestObj(const std::string& CoreType_str,
                            const X& defaultValue,
                            const X& testValue1,
                            const X& testValue2)
{
    FederateTestFixture fixture;
    using namespace helics;
    fixture.SetupTest<ValueFederate>(CoreType_str, 2);
    auto fedA = fixture.GetFederateAs<ValueFederate>(0);
    auto fedB = fixture.GetFederateAs<ValueFederate>(1);

    // register the publications
    auto& pubid = fedA->registerGlobalPublication<X>("pub1");

    auto subid = fedB->registerSubscription("pub1");
    fedA->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    fedB->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);

    subid.setDefault(defaultValue);

    auto f1finish = std::async(std::launch::async, [&]() { fedA->enterExecutingMode(); });
    fedB->enterExecutingMode();
    f1finish.wait();
    // publish string1 at time=0.0;
    pubid.publish(testValue1);

    X val;
    subid.getValue(val);

    EXPECT_EQ(val, defaultValue);

    auto f1time = std::async(std::launch::async, [&]() { return fedA->requestTime(1.0); });
    auto gtime = fedB->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(f1time.get(), 1.0);
    // get the value
    subid.getValue(val);
    // make sure the string is what we expect
    EXPECT_EQ(val, testValue1);

    // publish a second string
    pubid.publish(testValue2);

    subid.getValue(val);
    EXPECT_EQ(val, testValue1);

    // advance time
    f1time = std::async(std::launch::async, [&]() { return fedA->requestTime(2.0); });
    gtime = fedB->requestTime(2.0);

    EXPECT_EQ(gtime, 2.0);
    EXPECT_EQ(f1time.get(), 2.0);

    // make sure the value was updated
    subid.getValue(val);
    EXPECT_EQ(val, testValue2);
    fedA->finalizeAsync();
    fedB->finalize();
    fedA->finalizeComplete();
    helics::cleanupHelicsLibrary();
}
