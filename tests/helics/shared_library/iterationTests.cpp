/*
Copyright (c) 2017-2026,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Energy
Innovation LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "ctestFixtures.hpp"

#include <complex>
#include <future>
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

struct iteration_tests: public FederateTestFixture, public ::testing::Test {};

// run a simple iteration on a single federate to test out iterative calls
TEST_F(iteration_tests, execution_iteration_test)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);
    // register the publications
    auto pubid = helicsFederateRegisterGlobalPublication(
        vFed1, "pub1", HELICS_DATA_TYPE_DOUBLE, "", nullptr);

    auto subid = helicsFederateRegisterSubscription(vFed1, "pub1", "", nullptr);
    helicsFederateSetTimeProperty(vFed1, HELICS_PROPERTY_TIME_DELTA, 1.0, nullptr);

    helicsFederateEnterInitializingMode(vFed1, nullptr);
    helicsPublicationPublishDouble(pubid, 27.0, nullptr);

    auto comp =
        helicsFederateEnterExecutingModeIterative(vFed1,
                                                  HELICS_ITERATION_REQUEST_ITERATE_IF_NEEDED,
                                                  nullptr);
    EXPECT_TRUE(comp == HELICS_ITERATION_RESULT_ITERATING);
    auto val = helicsInputGetDouble(subid, nullptr);
    EXPECT_EQ(val, 27.0);

    comp = helicsFederateEnterExecutingModeIterative(vFed1,
                                                     HELICS_ITERATION_REQUEST_ITERATE_IF_NEEDED,
                                                     nullptr);

    EXPECT_TRUE(comp == HELICS_ITERATION_RESULT_NEXT_STEP);

    auto val2 = helicsInputGetDouble(subid, nullptr);

    EXPECT_EQ(val2, val);
}

// do an init iteration loop for convergence
std::pair<double, int> runInitIterations(HelicsFederate vfed, int index, int total)
{
    auto pub = helicsFederateRegisterPublication(vfed, "pub", HELICS_DATA_TYPE_DOUBLE, "", nullptr);
    helicsPublicationSetMinimumChange(pub, 0.001, nullptr);
    std::string low_target = "fed";
    low_target += std::to_string((index == 0) ? total - 1 : index - 1);
    low_target += "/pub";
    std::string high_target = "fed";
    high_target += std::to_string((index == total - 1) ? 0 : index + 1);
    high_target += "/pub";
    auto sub_low = helicsFederateRegisterSubscription(vfed, low_target.c_str(), "", nullptr);
    auto sub_high = helicsFederateRegisterSubscription(vfed, high_target.c_str(), "", nullptr);
    double index2 = 2.0 * static_cast<double>(index);
    helicsInputSetDefaultDouble(sub_low, index2, nullptr);
    helicsInputSetDefaultDouble(sub_high, index2 + 1.0, nullptr);

    helicsFederateEnterInitializingMode(vfed, nullptr);
    auto cval = index2 + 0.5;

    auto itres = HELICS_ITERATION_RESULT_ITERATING;
    int itcount = 0;
    while (itres == HELICS_ITERATION_RESULT_ITERATING) {
        helicsPublicationPublishDouble(pub, cval, nullptr);
        itres =
            helicsFederateEnterExecutingModeIterative(vfed,
                                                      HELICS_ITERATION_REQUEST_ITERATE_IF_NEEDED,
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
std::vector<std::pair<double, int>> run_iteration_round_robin(std::vector<HelicsFederate>& fedVec)
{
    auto fedCount = static_cast<int>(fedVec.size());
    std::vector<std::future<std::pair<double, int>>> futures;
    for (decltype(fedCount) ii = 0; ii < fedCount; ++ii) {
        auto vFed = fedVec[ii];
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

class iteration_tests_type:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {};

static const auto coreTypeTestNamer = [](const ::testing::TestParamInfo<const char*>& parameter) {
    return std::string(parameter.param);
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

TEST_F(iteration_tests, execution_iteration_loop3)
{
    int fedCount = 5;
    SetupTest(helicsCreateValueFederate, "test", fedCount);
    std::vector<HelicsFederate> vfeds(fedCount);
    for (int ii = 0; ii < fedCount; ++ii) {
        vfeds[ii] = GetFederateAt(ii);
        helicsFederateSetFlagOption(vfeds[ii],
                                    HELICS_FLAG_RESTRICTIVE_TIME_POLICY,
                                    HELICS_TRUE,
                                    nullptr);
    }
    auto results = run_iteration_round_robin(vfeds);
    for (int ii = 1; ii < fedCount; ++ii) {
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
        vFed1, "pub1", HELICS_DATA_TYPE_DOUBLE, "", nullptr);

    auto subid = helicsFederateRegisterSubscription(vFed2, "pub1", "", nullptr);

    helicsFederateEnterInitializingModeAsync(vFed1, nullptr);
    helicsFederateEnterInitializingMode(vFed2, nullptr);
    helicsFederateEnterInitializingModeComplete(vFed1, nullptr);
    helicsPublicationPublishDouble(pubid, 27.0, nullptr);

    helicsFederateEnterExecutingModeAsync(vFed1, nullptr);
    auto comp =
        helicsFederateEnterExecutingModeIterative(vFed2,
                                                  HELICS_ITERATION_REQUEST_ITERATE_IF_NEEDED,
                                                  nullptr);

    EXPECT_TRUE(comp == HELICS_ITERATION_RESULT_ITERATING);
    auto val = helicsInputGetDouble(subid, nullptr);
    EXPECT_EQ(val, 27.0);

    comp = helicsFederateEnterExecutingModeIterative(vFed2,
                                                     HELICS_ITERATION_REQUEST_ITERATE_IF_NEEDED,
                                                     nullptr);

    EXPECT_TRUE(comp == HELICS_ITERATION_RESULT_NEXT_STEP);

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
        vFed1, "pub1", HELICS_DATA_TYPE_DOUBLE, "", nullptr);

    auto subid = helicsFederateRegisterSubscription(vFed1, "pub1", "", nullptr);

    helicsFederateSetTimeProperty(vFed1, HELICS_PROPERTY_TIME_PERIOD, 1.0, nullptr);
    helicsFederateSetTimeProperty(vFed1, HELICS_PROPERTY_TIME_DELTA, 1.0, nullptr);
    helicsFederateEnterExecutingMode(vFed1, nullptr);

    helicsPublicationPublishDouble(pubid, 27.0, nullptr);

    HelicsIterationResult comp;
    auto grantedTime = helicsFederateRequestTimeIterative(
        vFed1, 1.0, HELICS_ITERATION_REQUEST_ITERATE_IF_NEEDED, &comp, nullptr);
    EXPECT_TRUE(comp == HELICS_ITERATION_RESULT_ITERATING);
    EXPECT_EQ(grantedTime, 0.0);
    auto val = helicsInputGetDouble(subid, nullptr);
    EXPECT_EQ(val, 27.0);

    grantedTime = helicsFederateRequestTimeIterative(
        vFed1, 1.0, HELICS_ITERATION_REQUEST_ITERATE_IF_NEEDED, &comp, nullptr);
    EXPECT_TRUE(comp == HELICS_ITERATION_RESULT_NEXT_STEP);
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
        vFed1, "pub1", HELICS_DATA_TYPE_DOUBLE, "", nullptr);

    auto subid = helicsFederateRegisterSubscription(vFed2, "pub1", "", nullptr);

    helicsFederateSetTimeProperty(vFed1, HELICS_PROPERTY_TIME_PERIOD, 1.0, nullptr);
    helicsFederateSetTimeProperty(vFed1, HELICS_PROPERTY_TIME_DELTA, 1.0, nullptr);

    helicsFederateEnterExecutingModeAsync(vFed1, nullptr);
    helicsFederateEnterExecutingMode(vFed2, nullptr);
    helicsFederateEnterExecutingModeComplete(vFed1, nullptr);

    helicsPublicationPublishDouble(pubid, 27.0, nullptr);

    helicsFederateRequestTimeAsync(vFed1, 1.0, nullptr);
    HelicsIterationResult comp;
    auto grantedTime = helicsFederateRequestTimeIterative(
        vFed2, 1.0, HELICS_ITERATION_REQUEST_ITERATE_IF_NEEDED, &comp, nullptr);

    EXPECT_TRUE(comp == HELICS_ITERATION_RESULT_ITERATING);
    EXPECT_EQ(grantedTime, HELICS_TIME_ZERO);
    auto val = helicsInputGetDouble(subid, nullptr);
    EXPECT_EQ(val, 27.0);

    grantedTime = helicsFederateRequestTimeIterative(
        vFed2, 1.0, HELICS_ITERATION_REQUEST_ITERATE_IF_NEEDED, &comp, nullptr);

    EXPECT_TRUE(comp == HELICS_ITERATION_RESULT_NEXT_STEP);
    EXPECT_EQ(grantedTime, 1.0);
    auto val2 = helicsInputGetDouble(subid, nullptr);
    helicsFederateRequestTimeComplete(vFed1, nullptr);

    EXPECT_EQ(val2, val);
}

struct paired_iteration_result {
    HelicsTime time1{HELICS_TIME_INVALID};
    HelicsTime time2{HELICS_TIME_INVALID};
    HelicsIterationResult state1{HELICS_ITERATION_RESULT_ERROR};
    HelicsIterationResult state2{HELICS_ITERATION_RESULT_ERROR};
};

static paired_iteration_result requestTimeIterativePair(HelicsFederate fed1,
                                                        HelicsFederate fed2,
                                                        HelicsTime requestTime)
{
    paired_iteration_result result;
    helicsFederateRequestTimeIterativeAsync(fed1,
                                            requestTime,
                                            HELICS_ITERATION_REQUEST_ITERATE_IF_NEEDED,
                                            nullptr);
    result.time2 = helicsFederateRequestTimeIterative(fed2,
                                                      requestTime,
                                                      HELICS_ITERATION_REQUEST_ITERATE_IF_NEEDED,
                                                      &result.state2,
                                                      nullptr);
    result.time1 = helicsFederateRequestTimeIterativeComplete(fed1, &result.state1, nullptr);
    return result;
}

TEST_P(iteration_tests_type, time_iteration_value_loop_with_endpoint_federate)
{
    auto broker = AddBroker(GetParam(), 3);
    AddFederates(helicsCreateCombinationFederate, GetParam(), 1, broker, 1.0, "Battery");
    AddFederates(helicsCreateCombinationFederate, GetParam(), 1, broker, 1.0, "Charger");
    AddFederates(helicsCreateMessageFederate, GetParam(), 1, broker, 1.0, "Controller");

    auto batteryFed = GetFederateAt(0);
    auto chargerFed = GetFederateAt(1);
    auto controllerFed = GetFederateAt(2);

    auto batteryPub = helicsFederateRegisterGlobalPublication(
        batteryFed, "Battery/EV1_current", HELICS_DATA_TYPE_DOUBLE, "A", nullptr);
    auto batterySub = helicsFederateRegisterSubscription(
        batteryFed, "Charger/EV1_voltage", "V", nullptr);
    auto chargerPub = helicsFederateRegisterGlobalPublication(
        chargerFed, "Charger/EV1_voltage", HELICS_DATA_TYPE_DOUBLE, "V", nullptr);
    auto chargerSub = helicsFederateRegisterSubscription(
        chargerFed, "Battery/EV1_current", "A", nullptr);

    auto batteryEndpoint =
        helicsFederateRegisterGlobalEndpoint(batteryFed, "Battery/EV1.co", "message", nullptr);
    helicsEndpointSetDefaultDestination(batteryEndpoint, "Controller/battery_ep", nullptr);
    auto chargerEndpoint =
        helicsFederateRegisterGlobalEndpoint(chargerFed, "Charger/EV1.so", "message", nullptr);
    helicsEndpointSetDefaultDestination(chargerEndpoint, "Controller/charger_ep", nullptr);

    auto controllerBatteryEndpoint = helicsFederateRegisterGlobalEndpoint(
        controllerFed, "Controller/battery_ep", "message", nullptr);
    helicsEndpointSetDefaultDestination(controllerBatteryEndpoint, "Battery/EV1.co", nullptr);
    auto controllerChargerEndpoint = helicsFederateRegisterGlobalEndpoint(
        controllerFed, "Controller/charger_ep", "message", nullptr);
    helicsEndpointSetDefaultDestination(controllerChargerEndpoint, "Charger/EV1.so", nullptr);

    constexpr int timeStepCount = 20;
    constexpr int iterationRoundsPerStep = 4;
    constexpr int maxIterations = iterationRoundsPerStep + 2;

    helicsFederateSetIntegerProperty(
        batteryFed, HELICS_PROPERTY_INT_MAX_ITERATIONS, maxIterations, nullptr);
    helicsFederateSetIntegerProperty(
        chargerFed, HELICS_PROPERTY_INT_MAX_ITERATIONS, maxIterations, nullptr);

    helicsFederateEnterExecutingModeAsync(batteryFed, nullptr);
    helicsFederateEnterExecutingModeAsync(chargerFed, nullptr);
    helicsFederateEnterExecutingMode(controllerFed, nullptr);
    helicsFederateEnterExecutingModeComplete(batteryFed, nullptr);
    helicsFederateEnterExecutingModeComplete(chargerFed, nullptr);

    double batteryValue = 5.0;
    double chargerValue = 10.0;
    HelicsTime currentTime = HELICS_TIME_ZERO;
    for (int step = 1; step <= timeStepCount; ++step) {
        const auto requestTime = static_cast<HelicsTime>(step);
        batteryValue += 1.0;
        chargerValue += 2.0;
        helicsPublicationPublishDouble(batteryPub, batteryValue, nullptr);
        helicsPublicationPublishDouble(chargerPub, chargerValue, nullptr);

        helicsFederateRequestTimeAsync(controllerFed, requestTime, nullptr);
        for (int iter = 0; iter < iterationRoundsPerStep; ++iter) {
            auto iteration = requestTimeIterativePair(batteryFed, chargerFed, requestTime);
            EXPECT_EQ(iteration.state1, HELICS_ITERATION_RESULT_ITERATING);
            EXPECT_EQ(iteration.state2, HELICS_ITERATION_RESULT_ITERATING);
            EXPECT_EQ(iteration.time1, currentTime);
            EXPECT_EQ(iteration.time2, currentTime);
            EXPECT_EQ(helicsInputGetDouble(batterySub, nullptr), chargerValue);
            EXPECT_EQ(helicsInputGetDouble(chargerSub, nullptr), batteryValue);

            if (iter + 1 < iterationRoundsPerStep) {
                batteryValue += 0.5;
                chargerValue += 0.75;
                helicsPublicationPublishDouble(batteryPub, batteryValue, nullptr);
                helicsPublicationPublishDouble(chargerPub, chargerValue, nullptr);
            }
        }

        auto iteration = requestTimeIterativePair(batteryFed, chargerFed, requestTime);
        EXPECT_EQ(iteration.state1, HELICS_ITERATION_RESULT_NEXT_STEP);
        EXPECT_EQ(iteration.state2, HELICS_ITERATION_RESULT_NEXT_STEP);
        EXPECT_EQ(iteration.time1, requestTime);
        EXPECT_EQ(iteration.time2, requestTime);

        auto controllerTime = helicsFederateRequestTimeComplete(controllerFed, nullptr);
        EXPECT_EQ(controllerTime, requestTime);
        currentTime = requestTime;
    }

    helicsFederateFinalizeAsync(batteryFed, nullptr);
    helicsFederateFinalizeAsync(chargerFed, nullptr);
    helicsFederateFinalize(controllerFed, nullptr);
    helicsFederateFinalizeComplete(batteryFed, nullptr);
    helicsFederateFinalizeComplete(chargerFed, nullptr);
}

INSTANTIATE_TEST_SUITE_P(iteration_tests,
                         iteration_tests_type,
                         ::testing::ValuesIn(CoreTypes_2),
                         coreTypeTestNamer);

// force iteration a specific number of times and exercise some of the async calls
TEST_F(iteration_tests, test_iteration_counter)
{
    SetupTest(helicsCreateValueFederate, "test", 2, 1.0);
    auto vFed1 = GetFederateAt(0);
    auto vFed2 = GetFederateAt(1);
    // register the publications
    auto pub1 = helicsFederateRegisterGlobalPublication(
        vFed1, "pub1", HelicsDataTypes::HELICS_DATA_TYPE_INT, nullptr, nullptr);

    auto sub1 = helicsFederateRegisterSubscription(vFed2, "pub1", nullptr, nullptr);
    auto pub2 = helicsFederateRegisterGlobalPublication(
        vFed2, "pub2", HelicsDataTypes::HELICS_DATA_TYPE_INT, nullptr, nullptr);

    auto sub2 = helicsFederateRegisterSubscription(vFed1, "pub2", nullptr, nullptr);
    helicsFederateSetTimeProperty(vFed1, HELICS_PROPERTY_TIME_PERIOD, 1.0, nullptr);
    helicsFederateSetTimeProperty(vFed2, HELICS_PROPERTY_TIME_PERIOD, 1.0, nullptr);
    // vFed1->setLoggingLevel(5);
    // vFed2->setLoggingLevel(5);

    helicsFederateEnterInitializingModeAsync(vFed1, nullptr);
    helicsFederateEnterInitializingMode(vFed2, nullptr);
    helicsFederateEnterInitializingModeComplete(vFed1, nullptr);
    int64_t counter1 = 0;
    int64_t counter2 = 0;

    helicsPublicationPublishInteger(pub1, counter1, nullptr);
    helicsPublicationPublishInteger(pub2, counter2, nullptr);

    helicsFederateEnterExecutingModeAsync(vFed1, nullptr);
    helicsFederateEnterExecutingMode(vFed2, nullptr);
    helicsFederateEnterExecutingModeComplete(vFed1, nullptr);
    while (counter1 <= 10) {
        EXPECT_EQ(helicsInputGetInteger(sub1, nullptr), counter1);
        EXPECT_EQ(helicsInputGetInteger(sub2, nullptr), counter2);
        ++counter1;
        ++counter2;
        if (counter1 <= 10) {
            helicsPublicationPublishInteger(pub1, counter1, nullptr);
            helicsPublicationPublishInteger(pub2, counter2, nullptr);
        }

        helicsFederateRequestTimeIterativeAsync(vFed1,
                                                1.0,
                                                HELICS_ITERATION_REQUEST_ITERATE_IF_NEEDED,
                                                nullptr);
        HelicsIterationResult state;

        auto grantedTime = helicsFederateRequestTimeIterative(
            vFed2, 1.0, HELICS_ITERATION_REQUEST_ITERATE_IF_NEEDED, &state, nullptr);
        if (counter1 <= 10) {
            EXPECT_TRUE(state == HELICS_ITERATION_RESULT_ITERATING);
            EXPECT_EQ(grantedTime, 0.0);
        } else {
            EXPECT_TRUE(state == HELICS_ITERATION_RESULT_NEXT_STEP);
            EXPECT_EQ(grantedTime, 1.0);
        }
        grantedTime = helicsFederateRequestTimeIterativeComplete(vFed1, &state, nullptr);
        if (counter1 <= 10) {
            EXPECT_TRUE(state == HELICS_ITERATION_RESULT_ITERATING);
            EXPECT_EQ(grantedTime, 0.0);
        } else {
            EXPECT_TRUE(state == HELICS_ITERATION_RESULT_NEXT_STEP);
            EXPECT_EQ(grantedTime, 1.0);
        }
    }
}
