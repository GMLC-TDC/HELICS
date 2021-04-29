/*
Copyright (c) 2017-2021,
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
#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/application_api/ValueConverter.hpp"

#include <future>

struct iteration_tests: public FederateTestFixture, public ::testing::Test {
};

/** just a check that in the simple case we do actually get the time back we requested*/

TEST_F(iteration_tests, execution_iteration_test)
{
    SetupTest<helics::ValueFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    // register the publications
    auto& pubid = vFed1->registerGlobalPublication<double>("pub1");

    auto& subid = vFed1->registerSubscription("pub1");
    vFed1->setProperty(helics_property_time_delta, 1.0);
    vFed1->enterInitializingMode();
    vFed1->publish(pubid, 27.0);

    auto comp = vFed1->enterExecutingMode(helics::iteration_request::iterate_if_needed);

    EXPECT_TRUE(comp == helics::iteration_result::iterating);
    auto val = subid.getValue<double>();
    EXPECT_EQ(val, 27.0);

    comp = vFed1->enterExecutingMode(helics::iteration_request::iterate_if_needed);

    EXPECT_TRUE(comp == helics::iteration_result::next_step);

    auto val2 = subid.getValue<double>();

    EXPECT_EQ(val2, val);
}

std::pair<double, int> runInitIterations(helics::ValueFederate* vfed, int index, int total)
{
    using namespace helics;
    Publication pub(vfed, "pub", data_type::helics_double);
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

    auto itres = iteration_result::iterating;
    int itcount = 0;
    while (itres == iteration_result::iterating) {
        pub.publish(cval);
        itres = vfed->enterExecutingMode(iteration_request::iterate_if_needed);
        auto val1 = sub_high.getValue<double>();
        auto val2 = sub_low.getValue<double>();
        cval = (val1 + val2) / 2.0;
        ++itcount;
        //   printf("[%d]<%d> (%d)=%f,(%d)=%f, curr=%f\n", itcount,index, (index == 0) ? total - 1 :
        //   index - 1,val2, (index == total - 1) ? (0) : index + 1, val1, cval);
    }
    return {cval, itcount};
}

std::vector<std::pair<double, int>>
    run_iteration_round_robin(std::vector<std::shared_ptr<helics::ValueFederate>>& fedVec)
{
    auto N = static_cast<int>(fedVec.size());
    std::vector<std::future<std::pair<double, int>>> futures;
    for (decltype(N) ii = 0; ii < N; ++ii) {
        auto vFed = fedVec[ii].get();
        futures.push_back(std::async(std::launch::async,
                                     [vFed, ii, N]() { return runInitIterations(vFed, ii, N); }));
    }
    std::vector<std::pair<double, int>> results(N);
    for (decltype(N) ii = 0; ii < N; ++ii) {
        results[ii] = futures[ii].get();
    }
    return results;
}

class iteration_tests_type:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {
};

TEST_P(iteration_tests_type, execution_iteration_round_robin_ci_skip)
{
    SetupTest<helics::ValueFederate>(GetParam(), 3);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0).get();
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1).get();
    auto vFed3 = GetFederateAs<helics::ValueFederate>(2).get();
    auto fut1 =
        std::async(std::launch::async, [vFed1]() { return runInitIterations(vFed1, 0, 3); });
    auto fut2 =
        std::async(std::launch::async, [vFed2]() { return runInitIterations(vFed2, 1, 3); });

    auto res3 = runInitIterations(vFed3, 2, 3);
    auto res2 = fut2.get();
    auto res1 = fut1.get();
    EXPECT_NEAR(res3.first, 2.5, 0.1);
    EXPECT_NEAR(res2.first, 2.5, 0.1);
    EXPECT_NEAR(res1.first, 2.5, 0.1);
}

INSTANTIATE_TEST_SUITE_P(iteration_tests,
                         iteration_tests_type,
                         ::testing::ValuesIn(core_types_all));

TEST_F(iteration_tests, execution_iteration_loop3)
{
    int N = 5;
    SetupTest<helics::ValueFederate>("test", N);
    std::vector<std::shared_ptr<helics::ValueFederate>> vfeds(N);
    for (int ii = 0; ii < N; ++ii) {
        vfeds[ii] = GetFederateAs<helics::ValueFederate>(ii);
    }
    auto results = run_iteration_round_robin(vfeds);
    for (int ii = 1; ii < N; ++ii) {
        if (results[ii].second < 50) {
            EXPECT_NEAR(results[ii].first, results[0].first, 0.1);
        }
    }
}

TEST_F(iteration_tests, execution_iteration_test_2fed)
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
    vFed1->publish(pubid, 27.0);
    vFed1->enterExecutingModeAsync();
    auto comp = vFed2->enterExecutingMode(helics::iteration_request::iterate_if_needed);

    EXPECT_TRUE(comp == helics::iteration_result::iterating);
    auto val = vFed2->getDouble(subid);
    EXPECT_EQ(val, 27.0);

    comp = vFed2->enterExecutingMode(helics::iteration_request::iterate_if_needed);

    EXPECT_TRUE(comp == helics::iteration_result::next_step);

    auto val2 = vFed2->getDouble(subid);
    vFed1->enterExecutingModeComplete();
    EXPECT_EQ(val2, val);
}

/** just a check that in the simple case we do actually get the time back we requested*/

TEST_F(iteration_tests, time_iteration_test)
{
    SetupTest<helics::ValueFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    // register the publications
    auto pubid = vFed1->registerGlobalPublication<double>("pub1");

    auto subid = vFed1->registerSubscription("pub1");
    vFed1->setProperty(helics_property_time_period, 1.0);
    vFed1->setProperty(helics_property_time_delta, 1.0);
    vFed1->enterExecutingMode();
    vFed1->publish(pubid, 27.0);

    auto comp = vFed1->requestTimeIterative(1.0, helics::iteration_request::iterate_if_needed);

    EXPECT_TRUE(comp.state == helics::iteration_result::iterating);
    EXPECT_EQ(comp.grantedTime, helics::timeZero);
    auto val = vFed1->getDouble(subid);
    EXPECT_EQ(val, 27.0);

    comp = vFed1->requestTimeIterative(1.0, helics::iteration_request::iterate_if_needed);

    EXPECT_TRUE(comp.state == helics::iteration_result::next_step);
    EXPECT_EQ(comp.grantedTime, 1.0);
    auto val2 = vFed1->getDouble(subid);

    EXPECT_EQ(val2, val);
}

TEST_F(iteration_tests, time_iteration_test_2fed)
{
    SetupTest<helics::ValueFederate>("test", 2, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    // register the publications
    auto pubid = vFed1->registerGlobalPublication<double>("pub1");

    auto subid = vFed2->registerSubscription("pub1");

    vFed1->setProperty(helics_property_time_period, 1);
    vFed2->setProperty(helics_property_time_period, 1.0);

    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();
    vFed1->publish(pubid, 27.0);

    vFed1->requestTimeAsync(1.0);
    auto comp = vFed2->requestTimeIterative(1.0, helics::iteration_request::iterate_if_needed);

    EXPECT_TRUE(comp.state == helics::iteration_result::iterating);
    EXPECT_EQ(comp.grantedTime, helics::timeZero);
    auto val = vFed2->getDouble(subid);
    EXPECT_EQ(val, 27.0);

    comp = vFed2->requestTimeIterative(1.0, helics::iteration_request::iterate_if_needed);

    EXPECT_TRUE(comp.state == helics::iteration_result::next_step);
    EXPECT_EQ(comp.grantedTime, 1.0);
    auto val2 = vFed2->getDouble(subid);
    vFed1->requestTimeComplete();

    EXPECT_EQ(val2, val);
}

TEST_F(iteration_tests, time_iteration_test_message)
{
    SetupTest<helics::MessageFederate>("test", 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    // register the publications
    auto eptid = mFed1->registerGlobalEndpoint("ept");

    mFed1->setProperty(helics_property_time_period, 1.0);
    mFed1->setProperty(helics_property_time_delta, 1.0);
    mFed1->enterExecutingMode();
    eptid.send("ept", "message1");

    auto comp = mFed1->requestTimeIterative(1.0, helics::iteration_request::iterate_if_needed);

    EXPECT_TRUE(comp.state == helics::iteration_result::iterating);
    EXPECT_EQ(comp.grantedTime, helics::timeZero);
    auto val = mFed1->getMessage(eptid);
    EXPECT_EQ(val->to_string(), "message1");

    comp = mFed1->requestTimeIterative(1.0, helics::iteration_request::iterate_if_needed);

    EXPECT_TRUE(comp.state == helics::iteration_result::next_step);
    EXPECT_EQ(comp.grantedTime, 1.0);
    EXPECT_FALSE(mFed1->hasMessage());
}

TEST_F(iteration_tests, time_iteration_test_2fed_message)
{
    SetupTest<helics::MessageFederate>("test", 2, 1.0);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);
    // register the publications
    auto eptid1 = mFed1->registerGlobalEndpoint("ept1");

    auto eptid2 = mFed2->registerGlobalEndpoint("ept2");

    mFed1->setProperty(helics_property_time_period, 1);
    mFed2->setProperty(helics_property_time_period, 1.0);

    mFed1->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();
    eptid1.send("ept2", "message1");

    mFed1->requestTimeAsync(1.0);
    auto comp = mFed2->requestTimeIterative(1.0, helics::iteration_request::iterate_if_needed);

    EXPECT_TRUE(comp.state == helics::iteration_result::iterating);
    EXPECT_EQ(comp.grantedTime, helics::timeZero);
    auto message = mFed2->getMessage();
    EXPECT_EQ(message->to_string(), "message1");

    comp = mFed2->requestTimeIterative(1.0, helics::iteration_request::iterate_if_needed);

    EXPECT_TRUE(comp.state == helics::iteration_result::next_step);
    EXPECT_EQ(comp.grantedTime, 1.0);
    EXPECT_FALSE(mFed2->hasMessage());
    mFed1->requestTimeComplete();
}

TEST_F(iteration_tests, test2fed_withSubPub)
{
    SetupTest<helics::ValueFederate>("test", 2, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    // register the publications
    auto pub1 =
        helics::Publication(helics::GLOBAL, vFed1.get(), "pub1", helics::data_type::helics_double);

    auto& sub1 = vFed2->registerSubscription("pub1");
    vFed1->setProperty(helics_property_time_delta, 1.0);
    vFed2->setProperty(helics_property_time_delta, 1.0);
    vFed1->setProperty(helics_property_time_period, 1.0);
    vFed2->setProperty(helics_property_time_period, 1.0);

    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();
    pub1.publish(27.0);

    vFed1->requestTimeAsync(1.0);
    auto comp = vFed2->requestTimeIterative(1.0, helics::iteration_request::iterate_if_needed);

    EXPECT_TRUE(comp.state == helics::iteration_result::iterating);
    EXPECT_EQ(comp.grantedTime, helics::timeZero);

    EXPECT_TRUE(sub1.isUpdated());
    auto val = sub1.getValue<double>();
    EXPECT_EQ(val, 27.0);
    EXPECT_TRUE(!sub1.isUpdated());
    comp = vFed2->requestTimeIterative(1.0, helics::iteration_request::iterate_if_needed);

    EXPECT_TRUE(comp.state == helics::iteration_result::next_step);
    EXPECT_EQ(comp.grantedTime, 1.0);
    EXPECT_TRUE(!sub1.isUpdated());
    auto val2 = sub1.getValue<double>();
    vFed1->requestTimeComplete();

    EXPECT_EQ(val2, val);
}

TEST_F(iteration_tests, iteration_counter)
{
    SetupTest<helics::ValueFederate>("test", 2, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    // register the publications
    auto pub1 =
        helics::Publication(helics::GLOBAL, vFed1.get(), "pub1", helics::data_type::helics_int);

    auto& sub1 = vFed2->registerSubscription("pub1");

    auto pub2 =
        helics::Publication(helics::GLOBAL, vFed2.get(), "pub2", helics::data_type::helics_int);

    auto& sub2 = vFed1->registerSubscription("pub2");
    vFed1->setProperty(helics_property_time_period, 1.0);
    vFed2->setProperty(helics_property_time_period, 1.0);
    // vFed1->setLoggingLevel(5);
    // vFed2->setLoggingLevel(5);
    vFed1->enterInitializingModeAsync();
    vFed2->enterInitializingMode();
    vFed1->enterInitializingModeComplete();
    std::atomic<int64_t> cc{0};
    int64_t c1 = 0;
    int64_t c2 = 0;
    pub1.publish(c1);
    pub2.publish(c2);
    vFed1->enterExecutingModeAsync();
    vFed2->enterExecutingMode();
    vFed1->enterExecutingModeComplete();
    helics::iteration_time res;
    std::thread deadlock([&] {
        int64_t cb{0};
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            auto nc1 = cc.load();
            if (nc1 > 10) {
                return;
            }
            if (nc1 == cb) {
                auto qres = vFed1->query("root", "global_time_debugging");
                std::cout << qres << std::endl;
                return;
            }
            cb = nc1;
        }
    });

    while (c1 <= 10) {
        EXPECT_EQ(sub1.getValue<int64_t>(), c1);
        EXPECT_EQ(sub2.getValue<int64_t>(), c2);
        ++c1;
        ++c2;

        if (c1 <= 10) {
            pub1.publish(c1);
            pub2.publish(c2);
        }
        std::cout << "iteration " << c1 << std::endl;
        vFed1->requestTimeIterativeAsync(1.0, helics::iteration_request::iterate_if_needed);
        if (c1 <= 10) {
            res = vFed2->requestTimeIterative(1.0, helics::iteration_request::iterate_if_needed);
        } else {
            vFed2->requestTimeIterativeAsync(1.0, helics::iteration_request::iterate_if_needed);
            //  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

            //  auto td = vFed2->query("root", "global_time_debugging");
            res = vFed2->requestTimeIterativeComplete();
        }

        if (c1 <= 10) {
            EXPECT_TRUE(res.state == helics::iteration_result::iterating);
            EXPECT_EQ(res.grantedTime, 0.0);
        } else {
            EXPECT_TRUE(res.state == helics::iteration_result::next_step);
            EXPECT_EQ(res.grantedTime, 1.0);
        }
        res = vFed1->requestTimeIterativeComplete();
        ++cc;
        std::cout << "iteration granted " << c1 << std::endl;
        if (c1 <= 10) {
            EXPECT_TRUE(res.state == helics::iteration_result::iterating);
            EXPECT_EQ(res.grantedTime, 0.0);
        } else {
            EXPECT_TRUE(res.state == helics::iteration_result::next_step);
            EXPECT_EQ(res.grantedTime, 1.0);
        }
    }
    deadlock.join();
}
