/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "ctestFixtures.hpp"

#include <complex>
#include <functional>
#include <future>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

struct callback_federate_tests: public FederateTestFixture, public ::testing::Test {};

static HelicsIterationRequest initFunction(void* userdata)
{
    auto* fc = reinterpret_cast<std::function<HelicsIterationRequest(void)>*>(userdata);
    return (*fc)();
}

static void zargFunction(void* userdata)
{
    auto* fc = reinterpret_cast<std::function<void(void)>*>(userdata);
    (*fc)();
}

// run a simple callback federate
TEST_F(callback_federate_tests, initialize)
{
    SetupTest(helicsCreateCallbackFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);

    int cb{0};
    std::function<HelicsIterationRequest()> initCall = [&cb]() {
        ++cb;
        return HELICS_ITERATION_REQUEST_HALT_OPERATIONS;
    };
    helicsCallbackFederateInitializeCallback(vFed1, initFunction, &initCall, NULL);

    auto v = std::make_shared<std::promise<int>>();
    auto vfut = v->get_future();

    std::function<void()> term = [v = std::move(v)]() { v->set_value(5); };

    helicsFederateCosimulationTerminationCallback(vFed1, zargFunction, &term, NULL);

    helicsFederateEnterInitializingMode(vFed1, NULL);

    auto res = vfut.get();
    EXPECT_EQ(cb, 1);
    EXPECT_EQ(res, 5);
    EXPECT_EQ(helicsFederateIsAsyncOperationCompleted(vFed1, NULL), HELICS_TRUE);
}

static HelicsTime timeIterativeCall(HelicsTime time,
                                    HelicsIterationResult res,
                                    HelicsIterationRequest* req,
                                    void* userData)
{
    auto* fc = reinterpret_cast<
        std::function<HelicsTime(HelicsTime, HelicsIterationResult, HelicsIterationRequest*)>*>(
        userData);
    return (*fc)(time, res, req);
}
//// test that the initialize callback gets executed and handles termination properly
TEST_F(callback_federate_tests, execute)
{
    SetupTest(helicsCreateCallbackFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);
    int cb{0};

    std::function<void()> execCall = [&cb]() { ++cb; };

    helicsFederateExecutingEntryCallback(vFed1, zargFunction, &execCall, NULL);

    std::function<HelicsTime(HelicsTime, HelicsIterationResult, HelicsIterationRequest*)> itcall =
        [](HelicsTime /*t*/, HelicsIterationResult /*res*/, HelicsIterationRequest* req) {
            *req = HELICS_ITERATION_REQUEST_HALT_OPERATIONS;
            return HELICS_TIME_MAXTIME;
        };

    helicsCallbackFederateNextTimeIterativeCallback(vFed1, timeIterativeCall, &itcall, NULL);
    auto v = std::make_shared<std::promise<int>>();
    auto vfut = v->get_future();

    std::function<void()> term = [v = std::move(v)]() { v->set_value(5); };

    helicsFederateCosimulationTerminationCallback(vFed1, zargFunction, &term, NULL);

    helicsFederateEnterInitializingMode(vFed1, NULL);

    auto res = vfut.get();
    EXPECT_EQ(cb, 1);
    EXPECT_EQ(res, 5);
    EXPECT_EQ(helicsFederateIsAsyncOperationCompleted(vFed1, NULL), HELICS_TRUE);
}

static void timeReqRet(HelicsTime time, HelicsBool iterating, void* userData)
{
    auto* fc = reinterpret_cast<std::function<void(HelicsTime, HelicsBool)>*>(userData);
    return (*fc)(time, iterating);
}

static void
    timeReqEntry(HelicsTime time, HelicsTime requestTime, HelicsBool iterating, void* userData)
{
    auto* fc = reinterpret_cast<std::function<void(HelicsTime, HelicsTime, HelicsBool)>*>(userData);
    return (*fc)(time, requestTime, iterating);
}

// test that the max time and period works correctly with the callback federate
TEST_F(callback_federate_tests, timeStepsPeriodMax)
{
    SetupTest(helicsCreateCallbackFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);

    helicsFederateSetTimeProperty(vFed1, HELICS_PROPERTY_TIME_STOPTIME, 3.0, NULL);
    helicsFederateSetTimeProperty(vFed1, HELICS_PROPERTY_TIME_PERIOD, 1.0, NULL);

    int cb1{0};
    int cb2{0};

    std::function<void(HelicsTime, HelicsBool)> reqert = [&cb1](HelicsTime, HelicsBool) { ++cb1; };

    std::function<void(HelicsTime, HelicsTime, HelicsBool)> reqenter =
        [&cb2](HelicsTime, HelicsTime, HelicsBool) { ++cb2; };

    helicsFederateSetTimeRequestReturnCallback(vFed1, timeReqRet, &reqert, NULL);
    helicsFederateSetTimeRequestEntryCallback(vFed1, timeReqEntry, &reqenter, NULL);

    auto v = std::make_shared<std::promise<int>>();
    auto vfut = v->get_future();

    std::function<void()> term = [v = std::move(v)]() { v->set_value(8); };

    helicsFederateCosimulationTerminationCallback(vFed1, zargFunction, &term, NULL);

    helicsFederateEnterInitializingMode(vFed1, NULL);

    auto res = vfut.get();
    EXPECT_EQ(cb1, 4);
    EXPECT_EQ(cb2, 3);
    EXPECT_EQ(res, 8);
    EXPECT_EQ(helicsFederateIsAsyncOperationCompleted(vFed1, NULL), HELICS_TRUE);
}

static void errorHandle(int errorCode, const char* message, void* userData)
{
    auto* fc = reinterpret_cast<std::function<void(int, const char*)>*>(userData);
    return (*fc)(errorCode, message);
}

//// test that an error is thrown in the init call
// run a simple callback federate
TEST_F(callback_federate_tests, initException)
{
    SetupTest(helicsCreateCallbackFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);

    std::function<HelicsIterationRequest()> initCall = []() -> HelicsIterationRequest {
        throw std::runtime_error("ETEST");
    };
    helicsCallbackFederateInitializeCallback(vFed1, initFunction, &initCall, NULL);

    auto v = std::make_shared<std::promise<std::string>>();
    auto vfut = v->get_future();

    std::function<void(int, const char*)> eHandle = [v = std::move(v)](int /*code*/,
                                                                       const char* message) {
        v->set_value(std::string(message));
    };
    helicsFederateErrorHandlerCallback(vFed1, errorHandle, &eHandle, NULL);

    helicsFederateEnterInitializingMode(vFed1, NULL);

    auto res = vfut.get();
    EXPECT_EQ(res, "ETEST");
    EXPECT_EQ(helicsFederateIsAsyncOperationCompleted(vFed1, NULL), HELICS_TRUE);
    EXPECT_EQ(helicsFederateGetState(vFed1, NULL), HELICS_STATE_ERROR);
}
