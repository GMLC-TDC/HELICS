/*
Copyright (c) 2017-2026,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Energy
Innovation LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include <chrono>
#include <complex>
#include <future>
#include <gtest/gtest.h>
#include <string>
/** these test cases test out the value converters
 */
#include "ctestFixtures.hpp"

struct timing_tests: public FederateTestFixture, public ::testing::Test {};

namespace {
struct TimedPublisherResult {
    HelicsTime finalTime{HELICS_TIME_ZERO};
    int iterations{0};
    int errorCode{HELICS_OK};
    std::string errorMessage;
};

TimedPublisherResult runTimedPublisher(HelicsFederate fed,
                                       HelicsPublication pub,
                                       HelicsTime step,
                                       double value,
                                       HelicsTime stopTime)
{
    TimedPublisherResult result;
    HelicsError localErr = helicsErrorInitialize();

    auto recordError = [&]() {
        result.errorCode = localErr.error_code;
        result.errorMessage = (localErr.message != nullptr) ? localErr.message : "";
    };

    helicsFederateEnterExecutingMode(fed, &localErr);
    if (localErr.error_code != HELICS_OK) {
        recordError();
        return result;
    }

    while (result.finalTime < stopTime) {
        helicsPublicationPublishDouble(pub, value, &localErr);
        if (localErr.error_code != HELICS_OK) {
            recordError();
            return result;
        }

        result.finalTime = helicsFederateRequestTime(fed, result.finalTime + step, &localErr);
        if (localErr.error_code != HELICS_OK) {
            recordError();
            return result;
        }
        ++result.iterations;
    }

    helicsFederateRequestTime(fed, HELICS_TIME_MAXTIME, &localErr);
    if (localErr.error_code != HELICS_OK) {
        recordError();
        return result;
    }

    helicsFederateFinalize(fed, &localErr);
    if (localErr.error_code != HELICS_OK) {
        recordError();
    }

    return result;
}
}  // namespace

/** just a check that in the simple case we do actually get the time back we requested*/
TEST_F(timing_tests, simple_timing_test)
{
    SetupTest(helicsCreateValueFederate, "test", 2);
    auto vFed1 = GetFederateAt(0);
    auto vFed2 = GetFederateAt(1);

    CE(helicsFederateSetTimeProperty(vFed1, HELICS_PROPERTY_TIME_PERIOD, 0.5, &err));
    CE(helicsFederateSetTimeProperty(vFed2, HELICS_PROPERTY_TIME_PERIOD, 0.5, &err));

    CE(helicsFederateSetFlagOption(
        vFed1, HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS, HELICS_TRUE, &err));
    CE(helicsFederateSetFlagOption(
        vFed2, HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS, HELICS_TRUE, &err));

    auto pub = helicsFederateRegisterGlobalTypePublication(vFed1, "pub1", "double", "", &err);
    CE(helicsFederateRegisterSubscription(vFed2, "pub1", "", &err));
    CE(helicsFederateEnterExecutingModeAsync(vFed1, &err));
    CE(helicsFederateEnterExecutingMode(vFed2, &err));
    CE(helicsFederateEnterExecutingModeComplete(vFed1, &err));
    CE(helicsPublicationPublishDouble(pub, 0.27, &err));
    HelicsTime gtime;
    CE(gtime = helicsFederateRequestTime(vFed1, 2.0, &err));
    EXPECT_EQ(gtime, 2.0);

    CE(gtime = helicsFederateRequestTime(vFed2, 2.0, &err));
    EXPECT_EQ(gtime, 0.5);

    CE(gtime = helicsFederateRequestTime(vFed2, 2.0, &err));
    EXPECT_EQ(gtime, 2.0);

    CE(helicsFederateFinalize(vFed1, &err));
    CE(helicsFederateFinalize(vFed2, &err));
}

TEST_F(timing_tests, simple_timing_test2)
{
    SetupTest(helicsCreateValueFederate, "test", 2);
    auto vFed1 = GetFederateAt(0);
    auto vFed2 = GetFederateAt(1);

    CE(helicsFederateSetFlagOption(
        vFed1, HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS, HELICS_TRUE, &err));
    CE(helicsFederateSetFlagOption(
        vFed2, HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS, HELICS_TRUE, &err));

    CE(helicsFederateSetTimeProperty(vFed1, HELICS_PROPERTY_TIME_PERIOD, 0.5, &err));
    CE(helicsFederateSetTimeProperty(vFed2, HELICS_PROPERTY_TIME_PERIOD, 0.5, &err));

    auto pub = helicsFederateRegisterGlobalPublication(
        vFed1, "pub1", HELICS_DATA_TYPE_DOUBLE, "", nullptr);
    helicsFederateRegisterSubscription(vFed2, "pub1", "", nullptr);

    CE(helicsFederateEnterExecutingModeAsync(vFed1, &err));
    CE(helicsFederateEnterExecutingMode(vFed2, &err));
    CE(helicsFederateEnterExecutingModeComplete(vFed1, &err));

    HelicsTime gtime;
    CE(gtime = helicsFederateRequestTime(vFed1, 0.32, &err));

    // check that the request is only granted at the appropriate period
    EXPECT_EQ(gtime, 0.5);
    CE(helicsPublicationPublishDouble(pub, 0.27, &err));
    CE(gtime = helicsFederateRequestTime(vFed1, 1.85, &err));
    EXPECT_EQ(gtime, 2.0);
    CE(gtime = helicsFederateRequestTime(vFed2, 1.79, &err));
    EXPECT_EQ(gtime, 0.5);  // the result should show up at the next available time point
    CE(gtime = helicsFederateRequestTime(vFed2, 2.0, &err));
    EXPECT_EQ(gtime, 2.0);
    CE(helicsFederateFinalize(vFed1, &err));
    // just test the next step function with no others
    CE(gtime = helicsFederateRequestNextStep(vFed2, &err));
    EXPECT_EQ(gtime, 2.5);

    CE(helicsFederateFinalize(vFed2, &err));
}

TEST_F(timing_tests, simple_timing_test_message)
{
    SetupTest(helicsCreateMessageFederate, "test", 2);
    auto vFed1 = GetFederateAt(0);
    auto vFed2 = GetFederateAt(1);

    CE(helicsFederateSetTimeProperty(vFed1, HELICS_PROPERTY_TIME_PERIOD, 0.6, &err));
    CE(helicsFederateSetTimeProperty(vFed2, HELICS_PROPERTY_TIME_PERIOD, 0.45, &err));

    CE(helicsFederateSetFlagOption(
        vFed1, HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS, HELICS_TRUE, &err));
    CE(helicsFederateSetFlagOption(
        vFed2, HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS, HELICS_TRUE, &err));

    auto ept1 = helicsFederateRegisterGlobalEndpoint(vFed1, "e1", "", &err);
    helicsFederateRegisterGlobalEndpoint(vFed2, "e2", "", &err);
    ASSERT_EQ(err.error_code, 0);
    CE(helicsFederateEnterExecutingModeAsync(vFed1, &err));
    CE(helicsFederateEnterExecutingMode(vFed2, &err));
    CE(helicsFederateEnterExecutingModeComplete(vFed1, &err));
    CE(helicsFederateRequestTimeAsync(vFed2, 3.5, &err));

    HelicsTime gtime;
    CE(gtime = helicsFederateRequestTime(vFed1, 0.32, &err));

    // check that the request is only granted at the appropriate period
    EXPECT_NEAR(gtime, 0.6, 0.000000001);
    CE(helicsEndpointSendBytesTo(ept1, "test1", 5, "e2", &err));

    CE(helicsFederateRequestTimeAsync(vFed1, 1.85, &err));

    CE(gtime = helicsFederateRequestTimeComplete(vFed2, &err));

    EXPECT_EQ(gtime, 0.9);  // the message should show up at the next available time point
    CE(helicsFederateRequestTimeAsync(vFed2, 2.0, &err));
    CE(gtime = helicsFederateRequestTimeComplete(vFed2, &err));
    EXPECT_EQ(gtime, 2.25);  // the message should show up at the next available time point
    CE(helicsFederateRequestTimeAsync(vFed2, 3.0, &err));
    CE(gtime = helicsFederateRequestTimeComplete(vFed1, &err));
    EXPECT_EQ(gtime, 2.4);
    CE(helicsFederateFinalize(vFed1, &err));
    CE(helicsFederateFinalize(vFed2, &err));
}

TEST_F(timing_tests, timing_with_input_delay)
{
    SetupTest(helicsCreateMessageFederate, "test", 2);
    auto vFed1 = GetFederateAt(0);
    auto vFed2 = GetFederateAt(1);

    CE(helicsFederateSetTimeProperty(vFed1, HELICS_PROPERTY_TIME_PERIOD, 0.1, &err));
    CE(helicsFederateSetTimeProperty(vFed2, HELICS_PROPERTY_TIME_PERIOD, 0.1, &err));

    CE(helicsFederateSetTimeProperty(vFed2, HELICS_PROPERTY_TIME_INPUT_DELAY, 0.1, &err));

    CE(helicsFederateSetFlagOption(
        vFed1, HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS, HELICS_TRUE, &err));
    CE(helicsFederateSetFlagOption(
        vFed2, HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS, HELICS_TRUE, &err));

    auto ept1 = helicsFederateRegisterGlobalEndpoint(vFed1, "e1", "", nullptr);
    helicsFederateRegisterGlobalEndpoint(vFed2, "e2", "", nullptr);

    CE(helicsFederateEnterExecutingModeAsync(vFed1, &err));
    CE(helicsFederateEnterExecutingMode(vFed2, &err));
    CE(helicsFederateEnterExecutingModeComplete(vFed1, &err));
    CE(helicsFederateRequestTimeAsync(vFed2, 2.0, &err));
    HelicsTime gtime;
    CE(gtime = helicsFederateRequestTime(vFed1, 1.0, &err));
    // check that the request is only granted at the appropriate period
    EXPECT_EQ(gtime, 1.0);
    CE(helicsEndpointSendBytesTo(ept1, "test1", 5, "e2", &err));
    CE(helicsFederateRequestTimeAsync(vFed1, 1.9, &err));
    CE(gtime = helicsFederateRequestTimeComplete(vFed2, &err));
    EXPECT_DOUBLE_EQ(gtime,
                     1.1);  // the message should show up at the next available time point after the
                            // impact window
    CE(helicsFederateRequestTimeAsync(vFed2, 2.0, &err));
    CE(gtime = helicsFederateRequestTimeComplete(vFed1, &err));
    EXPECT_DOUBLE_EQ(gtime, 1.9);

    CE(auto tres = helicsFederateGetTimeProperty(vFed1, HELICS_PROPERTY_TIME_PERIOD, &err));
    EXPECT_DOUBLE_EQ(tres, 0.1);

    CE(gtime = helicsFederateRequestTimeComplete(vFed2, &err));
    EXPECT_DOUBLE_EQ(gtime, 2.0);
    CE(helicsFederateFinalize(vFed1, &err));
    CE(helicsFederateFinalize(vFed2, &err));
}

TEST_F(timing_tests, timing_with_minDelta_change)
{
    SetupTest(helicsCreateValueFederate, "test", 1, 1.0);
    auto vFed = GetFederateAt(0);

    CE(helicsFederateEnterExecutingMode(vFed, &err));

    HelicsTime gtime;
    CE(gtime = helicsFederateRequestTime(vFed, 1.0, &err));
    // check that the request is only granted at the appropriate period

    EXPECT_EQ(gtime, 1.0);

    // purposely requesting 1.0 to test min delta
    CE(gtime = helicsFederateRequestTime(vFed, 1.0, &err));
    EXPECT_EQ(gtime, 2.0);

    CE(helicsFederateSetTimeProperty(vFed, HELICS_PROPERTY_TIME_DELTA, 0.1, &err));
    CE(gtime = helicsFederateRequestTime(vFed, gtime, &err));
    EXPECT_EQ(gtime, 2.1);
    CE(helicsFederateFinalize(vFed, &err));
}

TEST_F(timing_tests, timing_with_period_change)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed = GetFederateAt(0);

    CE(helicsFederateSetFlagOption(
        vFed, HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS, HELICS_TRUE, &err));
    CE(helicsFederateSetTimeProperty(vFed, HELICS_PROPERTY_TIME_PERIOD, 1.0, &err));
    CE(helicsFederateEnterExecutingMode(vFed, &err));

    HelicsTime gtime;
    CE(gtime = helicsFederateRequestTime(vFed, 1.0, &err));
    // check that the request is only granted at the appropriate period

    EXPECT_EQ(gtime, 1.0);
    CE(auto val =
           helicsFederateGetFlagOption(vFed, HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS, &err));

    EXPECT_EQ(val, HELICS_TRUE);

    // purposely requesting 1.0 to test min delta
    CE(gtime = helicsFederateRequestTime(vFed, 1.0, &err));
    EXPECT_EQ(gtime, 2.0);

    CE(helicsFederateSetTimeProperty(vFed, HELICS_PROPERTY_TIME_PERIOD, 0.1, &err));
    CE(gtime = helicsFederateRequestTime(vFed, gtime, &err));
    EXPECT_EQ(gtime, 2.1);
    CE(helicsFederateFinalize(vFed, &err));
}

TEST_F(timing_tests, max_time_consistency)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed = GetFederateAt(0);
    CE(helicsFederateEnterExecutingMode(vFed, &err));
    HelicsTime gtime;
    CE(gtime = helicsFederateRequestTime(vFed, HELICS_TIME_MAXTIME, &err));
    EXPECT_DOUBLE_EQ(gtime, HELICS_TIME_MAXTIME);
    CE(helicsFederateFinalize(vFed, &err));
    CE(gtime = helicsFederateGetCurrentTime(vFed, &err));
    EXPECT_DOUBLE_EQ(gtime, HELICS_TIME_MAXTIME);
}

TEST_F(timing_tests, ordering)
{
    EXPECT_LT(HELICS_TIME_BIGTIME, HELICS_TIME_MAXTIME);
    EXPECT_GT(HELICS_TIME_BIGTIME, HELICS_TIME_TERMINATION);
    EXPECT_GT(HELICS_TIME_BIGTIME, 9000000000.0);
    EXPECT_GT(HELICS_TIME_TERMINATION, static_cast<double>(0xFFFFFFFFLL));
}

#ifdef HELICS_ENABLE_ZMQ_CORE
TEST_F(timing_tests, global_time_publish_only_different_periods_issue2795_ci_skip)  // ci_skip
{
    constexpr HelicsTime stopTime = 365.0 * 24.0 * 60.0 * 60.0;
    constexpr HelicsTime shortStep = 900.0;
    constexpr HelicsTime longStep = 43200.0;
    constexpr int expectedShortIterations = static_cast<int>(stopTime / shortStep);
    constexpr int expectedLongIterations = static_cast<int>(stopTime / longStep);

    ctype = "zmq";
    auto broker = helicsCreateBroker("zmq",
                                     "issue2795_broker",
                                     "--federates=2 --globaltime --timeout=60s --loglevel=warning",
                                     &err);
    ASSERT_EQ(err.error_code, HELICS_OK) << err.message;
    ASSERT_EQ(helicsBrokerIsConnected(broker), HELICS_TRUE);
    brokers.push_back(broker);

    auto createFed = [&](const char* name, HelicsTime timeDelta) {
        auto fedInfo = helicsCreateFederateInfo();
        helicsFederateInfoSetCoreTypeFromString(fedInfo, "zmq", &err);
        EXPECT_EQ(err.error_code, HELICS_OK) << err.message;
        std::string initString{"--federates=1 --broker="};
        initString.append(helicsBrokerGetIdentifier(broker));
        initString.append(" --broker_address=");
        initString.append(helicsBrokerGetAddress(broker));
        initString.append(" --error_timeout=0");
        helicsFederateInfoSetCoreInitString(fedInfo, initString.c_str(), &err);
        EXPECT_EQ(err.error_code, HELICS_OK) << err.message;
        helicsFederateInfoSetTimeProperty(fedInfo, HELICS_PROPERTY_TIME_DELTA, timeDelta, &err);
        EXPECT_EQ(err.error_code, HELICS_OK) << err.message;

        auto fed = helicsCreateCombinationFederate(name, fedInfo, &err);
        EXPECT_EQ(err.error_code, HELICS_OK) << err.message;
        helicsFederateInfoFree(fedInfo);
        federates.push_back(fed);
        return fed;
    };

    auto fed900 = createFed("Federate_900s", shortStep);
    auto fed43200 = createFed("Federate_43200s", longStep);
    ASSERT_NE(fed900, nullptr);
    ASSERT_NE(fed43200, nullptr);

    auto pub900 = helicsFederateRegisterGlobalPublication(
        fed900, "static_value_900s", HELICS_DATA_TYPE_DOUBLE, "", &err);
    ASSERT_EQ(err.error_code, HELICS_OK) << err.message;
    auto pub43200 = helicsFederateRegisterGlobalPublication(
        fed43200, "static_value_43200s", HELICS_DATA_TYPE_DOUBLE, "", &err);
    ASSERT_EQ(err.error_code, HELICS_OK) << err.message;

    auto fed900Result = std::async(
        std::launch::async, runTimedPublisher, fed900, pub900, shortStep, 42.0, stopTime);
    auto fed43200Result = std::async(
        std::launch::async, runTimedPublisher, fed43200, pub43200, longStep, 99.0, stopTime);

    constexpr auto timeout = std::chrono::seconds(30);
    auto fed900Ready = fed900Result.wait_for(timeout) == std::future_status::ready;
    auto fed43200Ready = fed43200Result.wait_for(timeout) == std::future_status::ready;

    if (!fed900Ready || !fed43200Ready) {
        helicsBrokerDisconnect(broker, nullptr);
        fed900Ready = fed900Result.wait_for(std::chrono::seconds(5)) == std::future_status::ready;
        fed43200Ready =
            fed43200Result.wait_for(std::chrono::seconds(5)) == std::future_status::ready;
    }

    ASSERT_TRUE(fed900Ready);
    ASSERT_TRUE(fed43200Ready);

    const auto result900 = fed900Result.get();
    const auto result43200 = fed43200Result.get();

    EXPECT_EQ(result900.errorCode, HELICS_OK) << result900.errorMessage;
    EXPECT_EQ(result43200.errorCode, HELICS_OK) << result43200.errorMessage;
    EXPECT_EQ(result900.iterations, expectedShortIterations);
    EXPECT_EQ(result43200.iterations, expectedLongIterations);
    EXPECT_DOUBLE_EQ(result900.finalTime, stopTime);
    EXPECT_DOUBLE_EQ(result43200.finalTime, stopTime);
}
#endif
