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

static const auto testNamer = [](const ::testing::TestParamInfo<const char*>& parameter) {
    return std::string(parameter.param);
};

struct iteration: public FederateTestFixture, public ::testing::Test {};

/** just a check that in the simple case we do actually get the time back we requested*/

TEST_F(iteration, execution_iteration)
{
    SetupTest<helics::ValueFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    // register the publications
    auto& pubid = vFed1->registerGlobalPublication<double>("pub1");

    auto& subid = vFed1->registerSubscription("pub1");
    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    vFed1->enterInitializingMode();
    pubid.publish(27.0);

    auto comp = vFed1->enterExecutingMode(helics::IterationRequest::ITERATE_IF_NEEDED);

    EXPECT_TRUE(comp == helics::IterationResult::ITERATING);
    auto val = subid.getValue<double>();
    EXPECT_EQ(val, 27.0);
    EXPECT_EQ(vFed1->getIntegerProperty(HELICS_PROPERTY_INT_CURRENT_ITERATION), 1);

    comp = vFed1->enterExecutingMode(helics::IterationRequest::ITERATE_IF_NEEDED);
    EXPECT_TRUE(comp == helics::IterationResult::NEXT_STEP);

    auto val2 = subid.getValue<double>();

    EXPECT_EQ(val2, val);
}

TEST_F(iteration, execution_iteration_endpoint)
{
    SetupTest<helics::MessageFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::MessageFederate>(0);
    // register the publications
    auto& epid1 = vFed1->registerGlobalEndpoint("ep1");

    auto& epid2 = vFed1->registerGlobalEndpoint("ep2");
    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    vFed1->enterInitializingMode();
    epid1.sendTo("test", "ep2");

    auto comp = vFed1->enterExecutingMode(helics::IterationRequest::ITERATE_IF_NEEDED);

    EXPECT_TRUE(comp == helics::IterationResult::ITERATING);
    EXPECT_TRUE(epid2.hasMessage());
    EXPECT_EQ(epid2.getMessage()->data.to_string(), "test");

    comp = vFed1->enterExecutingMode(helics::IterationRequest::ITERATE_IF_NEEDED);

    EXPECT_TRUE(comp == helics::IterationResult::NEXT_STEP);

    EXPECT_FALSE(epid2.hasMessage());
}

std::pair<double, int> runInitIterations(helics::ValueFederate* vfed, int index, int total)
{
    using namespace helics;
    Publication pub(vfed, "pub", DataType::HELICS_DOUBLE);
    pub.setMinimumChange(0.001);
    std::string low_target = "fed";
    low_target += std::to_string((index == 0) ? total - 1 : index - 1);
    low_target += "/pub";
    std::string high_target = "fed";
    high_target += std::to_string((index == total - 1) ? (0) : index + 1);
    high_target += "/pub";
    auto& sub_low = vfed->registerSubscription(low_target);
    auto& sub_high = vfed->registerSubscription(high_target);
    double index2 = 2.0 * static_cast<double>(index);
    sub_low.setDefault(index2);
    sub_high.setDefault(index2 + 1.0);
    vfed->enterInitializingMode();
    auto cval = index2 + 0.5;

    auto itres = IterationResult::ITERATING;
    int itcount = 0;
    while (itres == IterationResult::ITERATING) {
        pub.publish(cval);
        itres = vfed->enterExecutingMode(IterationRequest::ITERATE_IF_NEEDED);
        auto val1 = sub_high.getValue<double>();
        auto val2 = sub_low.getValue<double>();
        cval = (val1 + val2) / 2.0;
        ++itcount;
        /*
        printf("[%d]<%d> (%d)=%f,(%d)=%f, curr=%f\n",
               itcount,
               index,
               (index == 0) ? total - 1 : index - 1,
               val2,
               (index == total - 1) ? (0) : index + 1,
               val1,
               cval);
               */
    }
    return {cval, itcount};
}

std::vector<std::pair<double, int>>
    run_iteration_round_robin(std::vector<std::shared_ptr<helics::ValueFederate>>& fedVec)
{
    auto fedCount = static_cast<int>(fedVec.size());
    std::vector<std::future<std::pair<double, int>>> futures;
    for (decltype(fedCount) ii = 0; ii < fedCount; ++ii) {
        auto vFed = fedVec[ii].get();
        futures.push_back(std::async(std::launch::async, [vFed, ii, fedCount]() {
            return runInitIterations(vFed, ii, fedCount);
        }));
    }
    std::vector<std::pair<double, int>> results(fedCount);
    for (decltype(fedCount) ii = 0; ii < fedCount; ++ii) {
        results[ii] = futures[ii].get();
    }
    return results;
}

class iteration_type: public ::testing::TestWithParam<const char*>, public FederateTestFixture {};

TEST_P(iteration_type, execution_iteration_round_robin_ci_skip)
{
    try {
        SetupTest<helics::ValueFederate>(GetParam(), 3);

        auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
        auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
        auto vFed3 = GetFederateAs<helics::ValueFederate>(2);
        vFed1->setFlagOption(helics::defs::RESTRICTIVE_TIME_POLICY);
        vFed2->setFlagOption(helics::defs::RESTRICTIVE_TIME_POLICY);
        vFed3->setFlagOption(helics::defs::RESTRICTIVE_TIME_POLICY);

        auto fut1 = std::async(std::launch::async,
                               [vFed1]() { return runInitIterations(vFed1.get(), 0, 3); });
        auto fut2 = std::async(std::launch::async,
                               [vFed2]() { return runInitIterations(vFed2.get(), 1, 3); });

        auto res3 = runInitIterations(vFed3.get(), 2, 3);
        auto res2 = fut2.get();
        auto res1 = fut1.get();
        EXPECT_NEAR(res3.first, 2.5, 0.1);
        EXPECT_NEAR(res2.first, 2.5, 0.1);
        EXPECT_NEAR(res1.first, 2.5, 0.1);
        vFed1->finalize();
        vFed2->finalize();
        vFed3->finalize();
        helics::cleanupHelicsLibrary();
    }
    catch (...) {
        std::cerr << "unable to run for " << GetParam() << '\n';
        return;
    }
}

INSTANTIATE_TEST_SUITE_P(iteration, iteration_type, ::testing::ValuesIn(CoreTypes_all), testNamer);

TEST_F(iteration, execution_iteration_loop3)
{
    int fedCount{5};
    SetupTest<helics::ValueFederate>("test", fedCount);
    std::vector<std::shared_ptr<helics::ValueFederate>> vfeds(fedCount);
    for (int ii = 0; ii < fedCount; ++ii) {
        vfeds[ii] = GetFederateAs<helics::ValueFederate>(ii);
        vfeds[ii]->setFlagOption(helics::defs::RESTRICTIVE_TIME_POLICY);
    }
    auto results = run_iteration_round_robin(vfeds);
    for (int ii = 1; ii < fedCount; ++ii) {
        if (results[ii].second < 50) {
            EXPECT_NEAR(results[ii].first, results[0].first, 0.1);
        }
    }
}

TEST_F(iteration, execution_iteration_2fed)
{
    SetupTest<helics::ValueFederate>("test", 2, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    // register the publications
    auto pubid = vFed1->registerGlobalPublication<double>("pub1");

    auto subid = vFed2->registerSubscription("pub1");

    vFed1->enterInitializingModeAsync();
    vFed2->enterInitializingMode();
    vFed1->enterInitializingModeComplete();
    pubid.publish(27.0);
    vFed1->enterExecutingModeAsync();
    auto comp = vFed2->enterExecutingMode(helics::IterationRequest::ITERATE_IF_NEEDED);

    EXPECT_TRUE(comp == helics::IterationResult::ITERATING);
    auto val = subid.getValue<double>();
    EXPECT_EQ(val, 27.0);

    comp = vFed2->enterExecutingMode(helics::IterationRequest::ITERATE_IF_NEEDED);

    EXPECT_TRUE(comp == helics::IterationResult::NEXT_STEP);

    auto val2 = subid.getValue<double>();
    vFed1->enterExecutingModeComplete();
    EXPECT_EQ(val2, val);
    vFed2->finalize();
    vFed1->finalize();
}

TEST_F(iteration, execution_iteration_2fed_endpoint)
{
    SetupTest<helics::MessageFederate>("test", 2, 1.0);
    auto vFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto vFed2 = GetFederateAs<helics::MessageFederate>(1);
    // register the publications
    auto& epid1 = vFed1->registerGlobalEndpoint("ep1");

    auto& epid2 = vFed2->registerGlobalEndpoint("ep2");

    vFed1->enterInitializingModeAsync();
    vFed2->enterInitializingMode();
    vFed1->enterInitializingModeComplete();
    epid1.sendTo("test122", "ep2");
    vFed1->enterExecutingModeAsync();
    auto comp = vFed2->enterExecutingMode(helics::IterationRequest::ITERATE_IF_NEEDED);

    EXPECT_TRUE(comp == helics::IterationResult::ITERATING);
    EXPECT_TRUE(epid2.hasMessage());
    EXPECT_EQ(epid2.getMessage()->data.to_string(), "test122");

    comp = vFed2->enterExecutingMode(helics::IterationRequest::ITERATE_IF_NEEDED);

    EXPECT_TRUE(comp == helics::IterationResult::NEXT_STEP);

    EXPECT_FALSE(epid2.hasMessage());
    vFed1->enterExecutingModeComplete();
    vFed2->finalize();
    vFed1->finalize();
}

TEST_F(iteration, execution_iteration_2fed_targeted_endpoint)
{
    SetupTest<helics::MessageFederate>("test", 2, 1.0);
    auto vFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto vFed2 = GetFederateAs<helics::MessageFederate>(1);
    // register the publications
    auto& epid1 = vFed1->registerGlobalTargetedEndpoint("ep1");

    auto& epid2 = vFed2->registerGlobalTargetedEndpoint("ep2");

    epid2.addSourceTarget("ep1");

    vFed1->enterInitializingModeAsync();
    vFed2->enterInitializingMode();
    vFed1->enterInitializingModeComplete();
    epid1.send("test122");
    vFed1->enterExecutingModeAsync();
    auto comp = vFed2->enterExecutingMode(helics::IterationRequest::ITERATE_IF_NEEDED);

    EXPECT_TRUE(comp == helics::IterationResult::ITERATING);
    EXPECT_TRUE(epid2.hasMessage());
    EXPECT_EQ(epid2.getMessage()->data.to_string(), "test122");

    comp = vFed2->enterExecutingMode(helics::IterationRequest::ITERATE_IF_NEEDED);

    EXPECT_TRUE(comp == helics::IterationResult::NEXT_STEP);

    EXPECT_FALSE(epid2.hasMessage());
    vFed1->enterExecutingModeComplete();
    vFed2->finalize();
    vFed1->finalize();
}
/** just a check that in the simple case we do actually get the time back we requested*/

TEST_F(iteration, time_iteration)
{
    SetupTest<helics::ValueFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    // register the publications
    auto pubid = vFed1->registerGlobalPublication<double>("pub1");

    auto subid = vFed1->registerSubscription("pub1");
    vFed1->setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    vFed1->enterExecutingMode();
    pubid.publish(27.0);

    auto comp = vFed1->requestTimeIterative(1.0, helics::IterationRequest::ITERATE_IF_NEEDED);

    EXPECT_TRUE(comp.state == helics::IterationResult::ITERATING);
    EXPECT_EQ(comp.grantedTime, helics::timeZero);
    auto val = subid.getValue<double>();
    EXPECT_EQ(val, 27.0);
    EXPECT_EQ(vFed1->getIntegerProperty(HELICS_PROPERTY_INT_CURRENT_ITERATION), 1);
    comp = vFed1->requestTimeIterative(1.0, helics::IterationRequest::ITERATE_IF_NEEDED);

    EXPECT_TRUE(comp.state == helics::IterationResult::NEXT_STEP);
    EXPECT_EQ(comp.grantedTime, 1.0);
    auto val2 = subid.getValue<double>();

    EXPECT_EQ(val2, val);
}

TEST_F(iteration, time_iteration_2fed)
{
    SetupTest<helics::ValueFederate>("test", 2, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    // register the publications
    auto pubid = vFed1->registerGlobalPublication<double>("pub1");

    auto subid = vFed2->registerSubscription("pub1");

    vFed1->setProperty(HELICS_PROPERTY_TIME_PERIOD, 1);
    vFed2->setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);

    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();
    pubid.publish(27.0);

    vFed1->requestTimeAsync(1.0);
    auto comp = vFed2->requestTimeIterative(1.0, helics::IterationRequest::ITERATE_IF_NEEDED);

    EXPECT_TRUE(comp.state == helics::IterationResult::ITERATING);
    EXPECT_EQ(comp.grantedTime, helics::timeZero);
    auto val = subid.getValue<double>();
    EXPECT_EQ(val, 27.0);

    comp = vFed2->requestTimeIterative(1.0, helics::IterationRequest::ITERATE_IF_NEEDED);

    EXPECT_TRUE(comp.state == helics::IterationResult::NEXT_STEP);
    EXPECT_EQ(comp.grantedTime, 1.0);
    auto val2 = subid.getValue<double>();
    vFed1->requestTimeComplete();

    EXPECT_EQ(val2, val);
}

TEST_F(iteration, time_iteration_message)
{
    SetupTest<helics::MessageFederate>("test", 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    // register the publications
    auto eptid = mFed1->registerGlobalEndpoint("ept");

    mFed1->setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    mFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    mFed1->enterExecutingMode();
    eptid.sendTo("message1", "ept");

    auto comp = mFed1->requestTimeIterative(1.0, helics::IterationRequest::ITERATE_IF_NEEDED);

    EXPECT_TRUE(comp.state == helics::IterationResult::ITERATING);
    EXPECT_EQ(comp.grantedTime, helics::timeZero);
    auto val = mFed1->getMessage(eptid);
    EXPECT_EQ(val->to_string(), "message1");

    comp = mFed1->requestTimeIterative(1.0, helics::IterationRequest::ITERATE_IF_NEEDED);

    EXPECT_TRUE(comp.state == helics::IterationResult::NEXT_STEP);
    EXPECT_EQ(comp.grantedTime, 1.0);
    EXPECT_FALSE(mFed1->hasMessage());
}

TEST_F(iteration, time_iteration_2fed_message)
{
    SetupTest<helics::MessageFederate>("test", 2, 1.0);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);
    // register the publications
    auto& eptid1 = mFed1->registerGlobalEndpoint("ept1");

    auto& eptid2 = mFed2->registerGlobalEndpoint("ept2");

    mFed1->setProperty(HELICS_PROPERTY_TIME_PERIOD, 1);
    mFed2->setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);

    mFed1->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();
    eptid1.sendTo("message1", "ept2");

    mFed1->requestTimeAsync(1.0);
    auto comp = mFed2->requestTimeIterative(1.0, helics::IterationRequest::ITERATE_IF_NEEDED);

    EXPECT_TRUE(comp.state == helics::IterationResult::ITERATING);
    EXPECT_EQ(comp.grantedTime, helics::timeZero);
    EXPECT_TRUE(eptid2.hasMessage());
    auto message = eptid2.getMessage();
    EXPECT_EQ(message->to_string(), "message1");

    comp = mFed2->requestTimeIterative(1.0, helics::IterationRequest::ITERATE_IF_NEEDED);

    EXPECT_TRUE(comp.state == helics::IterationResult::NEXT_STEP);
    EXPECT_EQ(comp.grantedTime, 1.0);
    EXPECT_FALSE(mFed2->hasMessage());
    mFed1->requestTimeComplete();
}

TEST_F(iteration, two_fed_withSubPub)
{
    SetupTest<helics::ValueFederate>("test", 2, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    // register the publications
    auto pub1 = helics::Publication(helics::InterfaceVisibility::GLOBAL,
                                    vFed1.get(),
                                    "pub1",
                                    helics::DataType::HELICS_DOUBLE);

    auto& sub1 = vFed2->registerSubscription("pub1");
    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    vFed2->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    vFed1->setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    vFed2->setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);

    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();
    pub1.publish(27.0);

    vFed1->requestTimeAsync(1.0);
    auto comp = vFed2->requestTimeIterative(1.0, helics::IterationRequest::ITERATE_IF_NEEDED);

    EXPECT_TRUE(comp.state == helics::IterationResult::ITERATING);
    EXPECT_EQ(comp.grantedTime, helics::timeZero);

    EXPECT_TRUE(sub1.isUpdated());
    auto val = sub1.getValue<double>();
    EXPECT_EQ(val, 27.0);
    EXPECT_TRUE(!sub1.isUpdated());
    comp = vFed2->requestTimeIterative(1.0, helics::IterationRequest::ITERATE_IF_NEEDED);

    EXPECT_TRUE(comp.state == helics::IterationResult::NEXT_STEP);
    EXPECT_EQ(comp.grantedTime, 1.0);
    EXPECT_TRUE(!sub1.isUpdated());
    auto val2 = sub1.getValue<double>();
    vFed1->requestTimeComplete();

    EXPECT_EQ(val2, val);
}

TEST_F(iteration, iteration_counter)
{
    SetupTest<helics::ValueFederate>("test", 2, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    // register the publications
    auto pub1 = helics::Publication(helics::InterfaceVisibility::GLOBAL,
                                    vFed1.get(),
                                    "pub1",
                                    helics::DataType::HELICS_INT);

    auto& sub1 = vFed2->registerSubscription("pub1");

    auto pub2 = helics::Publication(helics::InterfaceVisibility::GLOBAL,
                                    vFed2.get(),
                                    "pub2",
                                    helics::DataType::HELICS_INT);

    auto& sub2 = vFed1->registerSubscription("pub2");
    vFed1->setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    vFed2->setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    // vFed1->setLoggingLevel(5);
    // vFed2->setLoggingLevel(5);
    vFed1->enterInitializingModeAsync();
    vFed2->enterInitializingMode();
    vFed1->enterInitializingModeComplete();
    std::atomic<int64_t> counter{0};
    int64_t iterationCount1 = 0;
    int64_t iterationCount2 = 0;
    pub1.publish(iterationCount1);
    pub2.publish(iterationCount2);
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();
    helics::iteration_time res;
    std::thread deadlock([&] {
        int64_t deadlockCount{0};
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            auto nc1 = counter.load();
            if (nc1 > 10) {
                return;
            }
            if (nc1 == deadlockCount) {
                auto qres = vFed1->query("root", "global_time_debugging");
                std::cout << qres << '\n';
                return;
            }
            deadlockCount = nc1;
        }
    });

    while (iterationCount1 <= 10) {
        EXPECT_EQ(sub1.getValue<int64_t>(), iterationCount1);
        EXPECT_EQ(sub2.getValue<int64_t>(), iterationCount2);
        ++iterationCount1;
        ++iterationCount2;

        if (iterationCount1 <= 10) {
            pub1.publish(iterationCount1);
            pub2.publish(iterationCount2);
        }
        // std::cout << "iteration " << iterationCount1 << std::endl;
        vFed1->requestTimeIterativeAsync(1.0, helics::IterationRequest::ITERATE_IF_NEEDED);
        if (iterationCount1 <= 10) {
            res = vFed2->requestTimeIterative(1.0, helics::IterationRequest::ITERATE_IF_NEEDED);
        } else {
            vFed2->requestTimeIterativeAsync(1.0, helics::IterationRequest::ITERATE_IF_NEEDED);
            //  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

            //  auto td = vFed2->query("root", "global_time_debugging");
            res = vFed2->requestTimeIterativeComplete();
        }

        if (iterationCount1 <= 10) {
            EXPECT_TRUE(res.state == helics::IterationResult::ITERATING);
            EXPECT_EQ(res.grantedTime, 0.0);
        } else {
            EXPECT_TRUE(res.state == helics::IterationResult::NEXT_STEP);
            EXPECT_EQ(res.grantedTime, 1.0);
        }
        res = vFed1->requestTimeIterativeComplete();
        ++counter;
        // std::cout << "iteration granted " << iterationCount1 << std::endl;
        if (iterationCount1 <= 10) {
            EXPECT_TRUE(res.state == helics::IterationResult::ITERATING);
            EXPECT_EQ(res.grantedTime, 0.0);
            EXPECT_EQ(vFed1->getIntegerProperty(HELICS_PROPERTY_INT_CURRENT_ITERATION),
                      iterationCount1);
        } else {
            EXPECT_TRUE(res.state == helics::IterationResult::NEXT_STEP);
            EXPECT_EQ(res.grantedTime, 1.0);
            EXPECT_EQ(vFed1->getIntegerProperty(HELICS_PROPERTY_INT_CURRENT_ITERATION), 0);
        }
    }
    deadlock.join();
}

TEST_F(iteration, wait_for_current_time_iterative)
{
    auto broker = AddBroker("test", 2);
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0);
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0);

    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    vFed2->setFlagOption(helics::defs::WAIT_FOR_CURRENT_TIME_UPDATE);

    auto& pub1_1 = vFed1->registerGlobalPublication<double>("pub1_1");
    auto& sub1_1 = vFed1->registerSubscription("pub2_1");

    auto& pub2_1 = vFed2->registerGlobalPublication<double>("pub2_1");

    auto& sub2_1 = vFed2->registerSubscription("pub1_1");
    sub2_1.setDefault(9.9);
    sub1_1.setDefault(10.22);

    vFed2->enterExecutingModeAsync();
    vFed1->enterExecutingMode();

    pub1_1.publish(3.5);

    vFed1->requestTimeAsync(1.0);

    vFed2->enterExecutingModeComplete();
    EXPECT_EQ(sub2_1.getValue<double>(), 3.5);

    vFed2->requestTimeAsync(1.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(vFed2->isAsyncOperationCompleted());

    auto retTime = vFed1->requestTimeComplete();
    EXPECT_EQ(retTime, 1.0);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(vFed2->isAsyncOperationCompleted());

    vFed1->requestTimeIterativeAsync(3.0, helics::IterationRequest::ITERATE_IF_NEEDED);
    retTime = vFed2->requestTimeComplete();
    EXPECT_EQ(retTime, 1.0);

    EXPECT_EQ(sub2_1.getValue<double>(), 3.5);

    pub2_1.publish(3.1);

    vFed2->requestTimeIterativeAsync(7.0, helics::IterationRequest::ITERATE_IF_NEEDED);

    auto itTime = vFed1->requestTimeIterativeComplete();
    EXPECT_EQ(itTime.grantedTime, 1.0);
    EXPECT_EQ(itTime.state, helics::IterationResult::ITERATING);
    EXPECT_EQ(sub1_1.getValue<double>(), 3.1);

    pub1_1.publish(5.4);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(vFed2->isAsyncOperationCompleted());
    vFed1->requestTimeIterativeAsync(3.0, helics::IterationRequest::ITERATE_IF_NEEDED);

    // iteration 3

    itTime = vFed2->requestTimeIterativeComplete();
    EXPECT_EQ(itTime.grantedTime, 1.0);
    EXPECT_EQ(sub2_1.getValue<double>(), 5.4);
    pub2_1.publish(8.7);
    vFed2->requestTimeIterativeAsync(7.0, helics::IterationRequest::ITERATE_IF_NEEDED);

    // iteration 4
    itTime = vFed1->requestTimeIterativeComplete();
    EXPECT_EQ(itTime.grantedTime, 1.0);
    EXPECT_EQ(itTime.state, helics::IterationResult::ITERATING);
    EXPECT_EQ(sub1_1.getValue<double>(), 8.7);

    pub1_1.publish(7.3);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(vFed2->isAsyncOperationCompleted());
    vFed1->requestTimeIterativeAsync(3.0, helics::IterationRequest::ITERATE_IF_NEEDED);
    // iteration 5

    itTime = vFed2->requestTimeIterativeComplete();
    EXPECT_EQ(itTime.grantedTime, 1.0);
    EXPECT_EQ(sub2_1.getValue<double>(), 7.3);

    vFed2->finalize();

    itTime = vFed1->requestTimeIterativeComplete();
    EXPECT_EQ(itTime.grantedTime, 3.0);
    EXPECT_EQ(itTime.state, helics::IterationResult::NEXT_STEP);

    broker.reset();
    vFed1->finalize();
}

TEST_F(iteration, wait_for_current_time_iterative_enter_exec)
{
    auto broker = AddBroker("test", 2);
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0);
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0);

    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    vFed2->setFlagOption(helics::defs::WAIT_FOR_CURRENT_TIME_UPDATE);

    auto& pub1_1 = vFed1->registerGlobalPublication<int>("pub1_1");
    auto& sub1_1 = vFed1->registerSubscription("pub2_1");

    auto& pub2_1 = vFed2->registerGlobalPublication<int>("pub2_1");

    auto& sub2_1 = vFed2->registerSubscription("pub1_1");
    sub2_1.setDefault(-6);
    sub1_1.setDefault(-28);
    vFed2->enterInitializingModeAsync();
    vFed1->enterInitializingMode();

    pub1_1.publish(4);

    vFed1->enterExecutingModeAsync(ITERATE_IF_NEEDED);

    vFed2->enterInitializingModeComplete();
    vFed2->enterExecutingModeAsync(ITERATE_IF_NEEDED);
    for (int ii = 0; ii < 10; ++ii) {
        auto iterating = vFed2->enterExecutingModeComplete();
        EXPECT_EQ(iterating, helics::IterationResult::ITERATING);
        EXPECT_EQ(sub2_1.getValue<int>(), ii + 4);
        pub2_1.publish(ii + 27);
        vFed2->enterExecutingModeAsync(ITERATE_IF_NEEDED);
        iterating = vFed1->enterExecutingModeComplete();
        EXPECT_EQ(iterating, helics::IterationResult::ITERATING);
        EXPECT_EQ(sub1_1.getValue<int>(), ii + 27);
        pub1_1.publish(ii + 5);
        vFed1->enterExecutingModeAsync(ITERATE_IF_NEEDED);
    }
    auto iterating = vFed2->enterExecutingModeComplete();
    EXPECT_EQ(iterating, helics::IterationResult::ITERATING);
    EXPECT_EQ(sub2_1.getValue<int>(), 14);
    vFed2->enterExecutingModeAsync(ITERATE_IF_NEEDED);
    iterating = vFed1->enterExecutingModeComplete();
    EXPECT_EQ(iterating, helics::IterationResult::NEXT_STEP);
    EXPECT_EQ(sub1_1.getValue<int>(), 36);
    vFed1->requestTimeAsync(1.0);
    iterating = vFed2->enterExecutingModeComplete();
    EXPECT_EQ(iterating, helics::IterationResult::NEXT_STEP);

    vFed2->finalize();
    auto time = vFed1->requestTimeComplete();
    EXPECT_EQ(time, 1.0);
    broker.reset();
    vFed1->finalize();
}

TEST_F(iteration, wait_for_current_time_iterative_enter_exec_endpoint)
{
    auto broker = AddBroker("test", 2);
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0);
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0);

    auto vFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto vFed2 = GetFederateAs<helics::MessageFederate>(1);
    vFed2->setFlagOption(helics::defs::WAIT_FOR_CURRENT_TIME_UPDATE);

    auto& epid1 = vFed1->registerGlobalEndpoint("ep1");

    auto& epid2 = vFed2->registerGlobalEndpoint("ep2");

    vFed2->enterInitializingModeAsync();
    vFed1->enterInitializingMode();

    epid1.sendTo("test_5", "ep2");

    vFed1->enterExecutingModeAsync(ITERATE_IF_NEEDED);

    vFed2->enterInitializingModeComplete();
    vFed2->enterExecutingModeAsync(ITERATE_IF_NEEDED);
    for (int ii = 0; ii < 10; ++ii) {
        auto iterating = vFed2->enterExecutingModeComplete();
        EXPECT_EQ(iterating, helics::IterationResult::ITERATING);
        EXPECT_TRUE(epid2.hasMessage());
        if (epid2.hasMessage()) {
            EXPECT_EQ(epid2.getMessage()->data.to_string(), "test_" + std::to_string(ii + 5));
        }

        epid2.sendTo("test_" + std::to_string(ii + 27), "ep1");

        vFed2->enterExecutingModeAsync(ITERATE_IF_NEEDED);
        iterating = vFed1->enterExecutingModeComplete();
        EXPECT_EQ(iterating, helics::IterationResult::ITERATING);
        EXPECT_TRUE(epid1.hasMessage());
        if (epid1.hasMessage()) {
            EXPECT_EQ(epid1.getMessage()->data.to_string(), "test_" + std::to_string(ii + 27));
        }

        epid1.sendTo("test_" + std::to_string(ii + 6), "ep2");
        vFed1->enterExecutingModeAsync(ITERATE_IF_NEEDED);
    }
    auto iterating = vFed2->enterExecutingModeComplete();
    EXPECT_EQ(iterating, helics::IterationResult::ITERATING);
    EXPECT_TRUE(epid2.hasMessage());
    if (epid2.hasMessage()) {
        EXPECT_EQ(epid2.getMessage()->data.to_string(), "test_15");
    }

    vFed2->enterExecutingModeAsync(ITERATE_IF_NEEDED);
    iterating = vFed1->enterExecutingModeComplete();
    EXPECT_EQ(iterating, helics::IterationResult::NEXT_STEP);
    EXPECT_FALSE(epid1.hasMessage());
    vFed1->requestTimeAsync(1.0);
    iterating = vFed2->enterExecutingModeComplete();
    EXPECT_EQ(iterating, helics::IterationResult::NEXT_STEP);

    vFed2->finalize();
    auto time = vFed1->requestTimeComplete();
    EXPECT_EQ(time, 1.0);
    broker.reset();
    vFed1->finalize();
}

TEST_F(iteration, wait_for_current_time_iterative_enter_exec_endpoint_iterating_time_request)
{
    auto broker = AddBroker("test", 2);
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0);
    AddFederates<helics::MessageFederate>("test", 1, broker, 1.0);

    auto vFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto vFed2 = GetFederateAs<helics::MessageFederate>(1);
    vFed2->setFlagOption(helics::defs::WAIT_FOR_CURRENT_TIME_UPDATE);

    auto& epid1 = vFed1->registerGlobalEndpoint("ep1");

    auto& epid2 = vFed2->registerGlobalEndpoint("ep2");

    vFed2->enterInitializingModeAsync();
    vFed1->enterInitializingMode();

    epid1.sendTo("test_5", "ep2");

    vFed1->enterExecutingModeAsync(ITERATE_IF_NEEDED);

    vFed2->enterInitializingModeComplete();
    vFed2->enterExecutingModeAsync(ITERATE_IF_NEEDED);
    for (int ii = 0; ii < 5; ++ii) {
        auto iterating = vFed2->enterExecutingModeComplete();
        EXPECT_EQ(iterating, helics::IterationResult::ITERATING);
        EXPECT_TRUE(epid2.hasMessage());
        if (epid2.hasMessage()) {
            EXPECT_EQ(epid2.getMessage()->data.to_string(), "test_" + std::to_string(ii + 5));
        }

        epid2.sendTo("test_" + std::to_string(ii + 27), "ep1");

        vFed2->enterExecutingModeAsync(ITERATE_IF_NEEDED);
        iterating = vFed1->enterExecutingModeComplete();
        EXPECT_EQ(iterating, helics::IterationResult::ITERATING);
        EXPECT_TRUE(epid1.hasMessage());
        if (epid1.hasMessage()) {
            EXPECT_EQ(epid1.getMessage()->data.to_string(), "test_" + std::to_string(ii + 27));
        }

        epid1.sendTo("test_" + std::to_string(ii + 6), "ep2");
        vFed1->enterExecutingModeAsync(ITERATE_IF_NEEDED);
    }
    auto iterating = vFed2->enterExecutingModeComplete();
    EXPECT_EQ(iterating, helics::IterationResult::ITERATING);
    EXPECT_TRUE(epid2.hasMessage());
    if (epid2.hasMessage()) {
        EXPECT_EQ(epid2.getMessage()->data.to_string(), "test_10");
    }

    vFed2->enterExecutingModeAsync(ITERATE_IF_NEEDED);
    iterating = vFed1->enterExecutingModeComplete();
    EXPECT_EQ(iterating, helics::IterationResult::NEXT_STEP);
    EXPECT_FALSE(epid1.hasMessage());
    vFed1->requestTimeIterativeAsync(1.0, ITERATE_IF_NEEDED);
    iterating = vFed2->enterExecutingModeComplete();
    EXPECT_EQ(iterating, helics::IterationResult::NEXT_STEP);

    vFed2->finalize();
    auto time = vFed1->requestTimeIterativeComplete();
    EXPECT_EQ(time.grantedTime, 1.0);
    EXPECT_EQ(time.state, helics::IterationResult::NEXT_STEP);
    broker.reset();
    vFed1->finalize();
}

TEST_F(iteration, wait_for_current_time_iterative_enter_exec_iterating_time_request)
{
    auto broker = AddBroker("test", 2);
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0);
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0);

    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    vFed2->setFlagOption(helics::defs::WAIT_FOR_CURRENT_TIME_UPDATE);

    auto& pub1_1 = vFed1->registerGlobalPublication<int>("pub1_1");
    auto& sub1_1 = vFed1->registerSubscription("pub2_1");

    auto& pub2_1 = vFed2->registerGlobalPublication<int>("pub2_1");

    auto& sub2_1 = vFed2->registerSubscription("pub1_1");
    sub2_1.setDefault(-6);
    sub1_1.setDefault(-28);
    vFed2->enterInitializingModeAsync();
    vFed1->enterInitializingMode();

    pub1_1.publish(4);

    vFed1->enterExecutingModeAsync(ITERATE_IF_NEEDED);

    vFed2->enterInitializingModeComplete();
    vFed2->enterExecutingModeAsync(ITERATE_IF_NEEDED);
    for (int ii = 0; ii < 5; ++ii) {
        auto iterating = vFed2->enterExecutingModeComplete();
        EXPECT_EQ(iterating, helics::IterationResult::ITERATING);
        EXPECT_EQ(sub2_1.getValue<int>(), ii + 4);
        pub2_1.publish(ii + 27);
        vFed2->enterExecutingModeAsync(ITERATE_IF_NEEDED);
        iterating = vFed1->enterExecutingModeComplete();
        EXPECT_EQ(iterating, helics::IterationResult::ITERATING);
        EXPECT_EQ(sub1_1.getValue<int>(), ii + 27);
        pub1_1.publish(ii + 5);
        vFed1->enterExecutingModeAsync(ITERATE_IF_NEEDED);
    }
    auto iterating = vFed2->enterExecutingModeComplete();
    EXPECT_EQ(iterating, helics::IterationResult::ITERATING);
    EXPECT_EQ(sub2_1.getValue<int>(), 9);
    vFed2->enterExecutingModeAsync(ITERATE_IF_NEEDED);
    iterating = vFed1->enterExecutingModeComplete();
    EXPECT_EQ(iterating, helics::IterationResult::NEXT_STEP);
    EXPECT_EQ(sub1_1.getValue<int>(), 31);
    vFed1->requestTimeIterativeAsync(1.0, ITERATE_IF_NEEDED);
    iterating = vFed2->enterExecutingModeComplete();
    EXPECT_EQ(iterating, helics::IterationResult::NEXT_STEP);

    vFed2->finalize();
    auto time = vFed1->requestTimeIterativeComplete();
    EXPECT_EQ(time.grantedTime, 1.0);
    EXPECT_EQ(time.state, helics::IterationResult::NEXT_STEP);
    broker.reset();
    vFed1->finalize();
}

TEST_F(iteration, iteration_high_count_nocov_ci_skip_nosan)
{
    auto broker = AddBroker("test", 2);
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0);
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0);

    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    auto& pub1_1 = vFed1->registerGlobalPublication<int>("pub1_1");
    vFed1->registerSubscription("pub2_1");

    auto& pub2_1 = vFed2->registerGlobalPublication<int>("pub2_1");

    vFed2->registerSubscription("pub1_1");

    vFed2->enterInitializingModeAsync();
    vFed1->enterInitializingMode();
    vFed2->enterInitializingModeComplete();

    pub1_1.publish(4);
    pub2_1.publish(8);

    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();

    helics::Time ctime = helics::timeZero;
    size_t itCount = 0;

    while (itCount < 200000) {
        for (int ii = 0; ii < 10; ++ii) {
            pub2_1.publish(itCount + 400000);
            pub1_1.publish(itCount + 800000);
            ++itCount;
            vFed1->requestTimeIterativeAsync(ctime + 1.0,
                                             helics::IterationRequest::ITERATE_IF_NEEDED);
            vFed2->requestTimeIterative(ctime + 1.0, helics::IterationRequest::ITERATE_IF_NEEDED);
            vFed1->requestTimeIterativeComplete();
        }
        vFed1->requestTimeIterativeAsync(ctime + 1.0, helics::IterationRequest::ITERATE_IF_NEEDED);
        vFed2->requestTimeIterative(ctime + 1.0, helics::IterationRequest::ITERATE_IF_NEEDED);
        vFed1->requestTimeIterativeComplete();
    }

    EXPECT_GE(itCount, 200000U);
    vFed2->finalize();
    vFed1->finalize();
}
