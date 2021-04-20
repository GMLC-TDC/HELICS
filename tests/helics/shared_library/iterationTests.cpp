/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "ctestFixtures.hpp"

#include <complex>
#include <future>
#include <gtest/gtest.h>
#include <iostream>

struct iteration_tests: public FederateTestFixture, public ::testing::Test {
};

// run a simple iteration on a single federate to test out iterative calls
TEST_F(iteration_tests, execution_iteration_test)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);
    // register the publications
    auto pubid = helicsFederateRegisterGlobalPublication(
        vFed1, "pub1", helics_data_type_double, "", nullptr);

    auto subid = helicsFederateRegisterSubscription(vFed1, "pub1", "", nullptr);
    helicsFederateSetTimeProperty(vFed1, helics_property_time_delta, 1.0, nullptr);

    helicsFederateEnterInitializingMode(vFed1, nullptr);
    helicsPublicationPublishDouble(pubid, 27.0, nullptr);

    auto comp =
        helicsFederateEnterExecutingModeIterative(vFed1,
                                                  helics_iteration_request_iterate_if_needed,
                                                  nullptr);
    EXPECT_TRUE(comp == helics_iteration_result_iterating);
    auto val = helicsInputGetDouble(subid, nullptr);
    EXPECT_EQ(val, 27.0);

    comp = helicsFederateEnterExecutingModeIterative(vFed1,
                                                     helics_iteration_request_iterate_if_needed,
                                                     nullptr);

    EXPECT_TRUE(comp == helics_iteration_result_next_step);

    auto val2 = helicsInputGetDouble(subid, nullptr);

    EXPECT_EQ(val2, val);
}

// do an init iteration loop for convergence
std::pair<double, int> runInitIterations(helics_federate vfed, int index, int total)
{
    auto pub = helicsFederateRegisterPublication(vfed, "pub", helics_data_type_double, "", nullptr);
    helicsPublicationSetMinimumChange(pub, 0.01, nullptr);
    std::string low_target = "fed";
    low_target += std::to_string((index == 0) ? total - 1 : index - 1);
    low_target += "/pub";
    std::string high_target = "fed";
    high_target += std::to_string((index == total - 1) ? (0) : index + 1);
    high_target += "/pub";
    auto sub_low = helicsFederateRegisterSubscription(vfed, low_target.c_str(), "", nullptr);
    auto sub_high = helicsFederateRegisterSubscription(vfed, high_target.c_str(), "", nullptr);
    double index2 = 2.0 * static_cast<double>(index);
    helicsInputSetDefaultDouble(sub_low, index2, nullptr);
    helicsInputSetDefaultDouble(sub_high, index2 + 1.0, nullptr);

    helicsFederateEnterInitializingMode(vfed, nullptr);
    auto cval = index2 + 0.5;

    auto itres = helics_iteration_result_iterating;
    int itcount = 0;
    while (itres == helics_iteration_result_iterating) {
        helicsPublicationPublishDouble(pub, cval, nullptr);
        itres =
            helicsFederateEnterExecutingModeIterative(vfed,
                                                      helics_iteration_request_iterate_if_needed,
                                                      nullptr);
        auto val1 = helicsInputGetDouble(sub_high, nullptr);
        auto val2 = helicsInputGetDouble(sub_low, nullptr);
        cval = (val1 + val2) / 2.0;
        ++itcount;
        //   printf("[%d]<%d> (%d)=%f,(%d)=%f, curr=%f\n", itcount,index, (index == 0) ? total - 1 :
        //   index - 1,val2, (index == total - 1) ? (0) : index + 1, val1, cval);
    }
    return {cval, itcount};
}

// a test with an iterative loop with a bunch of federates
std::vector<std::pair<double, int>> run_iteration_round_robin(std::vector<helics_federate>& fedVec)
{
    auto N = static_cast<int>(fedVec.size());
    std::vector<std::future<std::pair<double, int>>> futures;
    for (decltype(N) ii = 0; ii < N; ++ii) {
        auto vFed = fedVec[ii];
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
    SetupTest(helicsCreateValueFederate, GetParam(), 3);
    auto vFed1 = GetFederateAt(0);
    auto vFed2 = GetFederateAt(1);
    auto vFed3 = GetFederateAt(2);
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

INSTANTIATE_TEST_SUITE_P(iteration_tests, iteration_tests_type, ::testing::ValuesIn(core_types));

TEST_F(iteration_tests, execution_iteration_loop3)
{
    int N = 5;
    SetupTest(helicsCreateValueFederate, "test", N);
    std::vector<helics_federate> vfeds(N);
    for (int ii = 0; ii < N; ++ii) {
        vfeds[ii] = GetFederateAt(ii);
    }
    auto results = run_iteration_round_robin(vfeds);
    for (int ii = 1; ii < N; ++ii) {
        if (results[ii].second < 50) {
            EXPECT_NEAR(results[ii].first, results[0].first, 0.1);
        }
    }
}

// perform an iterative loop with two federates
TEST_F(iteration_tests, execution_iteration_test_2fed)
{
    SetupTest(helicsCreateValueFederate, "test", 2, 1.0);
    auto vFed1 = GetFederateAt(0);
    auto vFed2 = GetFederateAt(1);
    // register the publications
    auto pubid = helicsFederateRegisterGlobalPublication(
        vFed1, "pub1", helics_data_type_double, "", nullptr);

    auto subid = helicsFederateRegisterSubscription(vFed2, "pub1", "", nullptr);

    helicsFederateEnterInitializingModeAsync(vFed1, nullptr);
    helicsFederateEnterInitializingMode(vFed2, nullptr);
    helicsFederateEnterInitializingModeComplete(vFed1, nullptr);
    helicsPublicationPublishDouble(pubid, 27.0, nullptr);

    helicsFederateEnterExecutingModeAsync(vFed1, nullptr);
    auto comp =
        helicsFederateEnterExecutingModeIterative(vFed2,
                                                  helics_iteration_request_iterate_if_needed,
                                                  nullptr);

    EXPECT_TRUE(comp == helics_iteration_result_iterating);
    auto val = helicsInputGetDouble(subid, nullptr);
    EXPECT_EQ(val, 27.0);

    comp = helicsFederateEnterExecutingModeIterative(vFed2,
                                                     helics_iteration_request_iterate_if_needed,
                                                     nullptr);

    EXPECT_TRUE(comp == helics_iteration_result_next_step);

    auto val2 = helicsInputGetDouble(subid, nullptr);
    helicsFederateEnterExecutingModeComplete(vFed1, nullptr);
    EXPECT_EQ(val2, val);
}

/** just a check that in the simple case we do actually get the time back we requested*/
TEST_F(iteration_tests, time_iteration_test)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);
    // register the publications
    auto pubid = helicsFederateRegisterGlobalPublication(
        vFed1, "pub1", helics_data_type_double, "", nullptr);

    auto subid = helicsFederateRegisterSubscription(vFed1, "pub1", "", nullptr);

    helicsFederateSetTimeProperty(vFed1, helics_property_time_period, 1.0, nullptr);
    helicsFederateSetTimeProperty(vFed1, helics_property_time_delta, 1.0, nullptr);
    helicsFederateEnterExecutingMode(vFed1, nullptr);

    helicsPublicationPublishDouble(pubid, 27.0, nullptr);

    helics_iteration_result comp;
    auto grantedTime = helicsFederateRequestTimeIterative(
        vFed1, 1.0, helics_iteration_request_iterate_if_needed, &comp, nullptr);
    EXPECT_TRUE(comp == helics_iteration_result_iterating);
    EXPECT_EQ(grantedTime, 0.0);
    auto val = helicsInputGetDouble(subid, nullptr);
    EXPECT_EQ(val, 27.0);

    grantedTime = helicsFederateRequestTimeIterative(
        vFed1, 1.0, helics_iteration_request_iterate_if_needed, &comp, nullptr);
    EXPECT_TRUE(comp == helics_iteration_result_next_step);
    EXPECT_EQ(grantedTime, 1.0);
    auto val2 = helicsInputGetDouble(subid, nullptr);

    EXPECT_EQ(val2, val);
}

// run a test of iteration with two federates
TEST_F(iteration_tests, time_iteration_test_2fed)
{
    SetupTest(helicsCreateValueFederate, "test", 2, 1.0);
    auto vFed1 = GetFederateAt(0);
    auto vFed2 = GetFederateAt(1);
    // register the publications
    auto pubid = helicsFederateRegisterGlobalPublication(
        vFed1, "pub1", helics_data_type_double, "", nullptr);

    auto subid = helicsFederateRegisterSubscription(vFed2, "pub1", "", nullptr);

    helicsFederateSetTimeProperty(vFed1, helics_property_time_period, 1.0, nullptr);
    helicsFederateSetTimeProperty(vFed1, helics_property_time_delta, 1.0, nullptr);

    helicsFederateEnterExecutingModeAsync(vFed1, nullptr);
    helicsFederateEnterExecutingMode(vFed2, nullptr);
    helicsFederateEnterExecutingModeComplete(vFed1, nullptr);

    helicsPublicationPublishDouble(pubid, 27.0, nullptr);

    helicsFederateRequestTimeAsync(vFed1, 1.0, nullptr);
    helics_iteration_result comp;
    auto grantedTime = helicsFederateRequestTimeIterative(
        vFed2, 1.0, helics_iteration_request_iterate_if_needed, &comp, nullptr);

    EXPECT_TRUE(comp == helics_iteration_result_iterating);
    EXPECT_EQ(grantedTime, helics_time_zero);
    auto val = helicsInputGetDouble(subid, nullptr);
    EXPECT_EQ(val, 27.0);

    grantedTime = helicsFederateRequestTimeIterative(
        vFed2, 1.0, helics_iteration_request_iterate_if_needed, &comp, nullptr);

    EXPECT_TRUE(comp == helics_iteration_result_next_step);
    EXPECT_EQ(grantedTime, 1.0);
    auto val2 = helicsInputGetDouble(subid, nullptr);
    helicsFederateRequestTimeComplete(vFed1, nullptr);

    EXPECT_EQ(val2, val);
}

// force iteration a specific number of times and exercise some of the async calls
TEST_F(iteration_tests, test_iteration_counter)
{
    SetupTest(helicsCreateValueFederate, "test", 2, 1.0);
    auto vFed1 = GetFederateAt(0);
    auto vFed2 = GetFederateAt(1);
    // register the publications
    auto pub1 = helicsFederateRegisterGlobalPublication(
        vFed1, "pub1", helics_data_type::helics_data_type_int, nullptr, nullptr);

    auto sub1 = helicsFederateRegisterSubscription(vFed2, "pub1", nullptr, nullptr);
    auto pub2 = helicsFederateRegisterGlobalPublication(
        vFed2, "pub2", helics_data_type::helics_data_type_int, nullptr, nullptr);

    auto sub2 = helicsFederateRegisterSubscription(vFed1, "pub2", nullptr, nullptr);
    helicsFederateSetTimeProperty(vFed1, helics_property_time_period, 1.0, nullptr);
    helicsFederateSetTimeProperty(vFed2, helics_property_time_period, 1.0, nullptr);
    // vFed1->setLoggingLevel(5);
    // vFed2->setLoggingLevel(5);

    helicsFederateEnterInitializingModeAsync(vFed1, nullptr);
    helicsFederateEnterInitializingMode(vFed2, nullptr);
    helicsFederateEnterInitializingModeComplete(vFed1, nullptr);
    int64_t c1 = 0;
    int64_t c2 = 0;

    helicsPublicationPublishInteger(pub1, c1, nullptr);
    helicsPublicationPublishInteger(pub2, c2, nullptr);

    helicsFederateEnterExecutingModeAsync(vFed1, nullptr);
    helicsFederateEnterExecutingMode(vFed2, nullptr);
    helicsFederateEnterExecutingModeComplete(vFed1, nullptr);
    while (c1 <= 10) {
        EXPECT_EQ(helicsInputGetInteger(sub1, nullptr), c1);
        EXPECT_EQ(helicsInputGetInteger(sub2, nullptr), c2);
        ++c1;
        ++c2;
        if (c1 <= 10) {
            helicsPublicationPublishInteger(pub1, c1, nullptr);
            helicsPublicationPublishInteger(pub2, c2, nullptr);
        }

        helicsFederateRequestTimeIterativeAsync(vFed1,
                                                1.0,
                                                helics_iteration_request_iterate_if_needed,
                                                nullptr);
        helics_iteration_result state;

        auto grantedTime = helicsFederateRequestTimeIterative(
            vFed2, 1.0, helics_iteration_request_iterate_if_needed, &state, nullptr);
        if (c1 <= 10) {
            EXPECT_TRUE(state == helics_iteration_result_iterating);
            EXPECT_EQ(grantedTime, 0.0);
        } else {
            EXPECT_TRUE(state == helics_iteration_result_next_step);
            EXPECT_EQ(grantedTime, 1.0);
        }
        grantedTime = helicsFederateRequestTimeIterativeComplete(vFed1, &state, nullptr);
        if (c1 <= 10) {
            EXPECT_TRUE(state == helics_iteration_result_iterating);
            EXPECT_EQ(grantedTime, 0.0);
        } else {
            EXPECT_TRUE(state == helics_iteration_result_next_step);
            EXPECT_EQ(grantedTime, 1.0);
        }
    }
}
