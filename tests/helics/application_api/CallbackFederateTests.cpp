/*
Copyright (c) 2017-2026,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Energy
Innovation LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/CallbackFederate.hpp"
#include "helics/application_api/CoreApp.hpp"
#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/Publications.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/core-exceptions.hpp"
#include "testFixtures.hpp"

#include <future>
#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

class callbackFed: public ::testing::Test, public FederateTestFixture {};

TEST_F(callbackFed, create)
{
    SetupTest<helics::CallbackFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::CallbackFederate>(0);

    EXPECT_TRUE(vFed1->getFlagOption(HELICS_FLAG_CALLBACK_FEDERATE));
    vFed1->disconnect();
}

// test that the initialize callback gets executed and handles termination properly
TEST_F(callbackFed, initialize)
{
    SetupTest<helics::CallbackFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::CallbackFederate>(0);
    int callbackCount{0};
    vFed1->setInitializeCallback([&callbackCount]() {
        ++callbackCount;
        return helics::IterationRequest::HALT_OPERATIONS;
    });

    auto terminationPromise = std::make_shared<std::promise<int>>();
    auto vfut = terminationPromise->get_future();
    vFed1->setCosimulationTerminatedCallback(
        [terminationPromise = std::move(terminationPromise)]() {
            terminationPromise->set_value(5);
        });

    vFed1->enterInitializingMode();

    auto res = vfut.get();
    EXPECT_EQ(callbackCount, 1);
    EXPECT_EQ(res, 5);
    EXPECT_TRUE(vFed1->isAsyncOperationCompleted());
}

// test that the executing callback gets executed and handles termination properly
TEST_F(callbackFed, execute)
{
    SetupTest<helics::CallbackFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::CallbackFederate>(0);
    int callbackCount{0};

    vFed1->setExecutingEntryCallback([&callbackCount]() { ++callbackCount; });
    vFed1->setNextTimeIterativeCallback([](auto /*t*/) {
        return std::make_pair(helics::Time::maxVal(), helics::IterationRequest::HALT_OPERATIONS);
    });
    auto terminationPromise = std::make_shared<std::promise<int>>();
    auto vfut = terminationPromise->get_future();
    vFed1->setCosimulationTerminatedCallback(
        [terminationPromise = std::move(terminationPromise)]() {
            terminationPromise->set_value(5);
        });

    vFed1->enterInitializingMode();

    auto res = vfut.get();
    EXPECT_EQ(callbackCount, 1);
    EXPECT_EQ(res, 5);
    EXPECT_TRUE(vFed1->isAsyncOperationCompleted());
}

// test that the pre/post time request work properly
TEST_F(callbackFed, timeSteps)
{
    SetupTest<helics::CallbackFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::CallbackFederate>(0);
    int cb1{0};
    int cb2{0};
    vFed1->setTimeRequestReturnCallback([&cb1](auto /*unused*/, bool /*unused*/) { ++cb1; });
    vFed1->setTimeRequestEntryCallback(
        [&cb2](auto /*unused*/, auto /*unused*/, bool /*unused*/) { ++cb2; });

    vFed1->setNextTimeIterativeCallback([](auto grant) {
        if (grant.grantedTime >= 3.0) {
            return std::make_pair(helics::Time::maxVal(),
                                  helics::IterationRequest::HALT_OPERATIONS);
        }
        return std::make_pair(grant.grantedTime + 1.0, helics::IterationRequest::NO_ITERATIONS);
    });
    auto terminationPromise = std::make_shared<std::promise<int>>();
    auto vfut = terminationPromise->get_future();
    vFed1->setCosimulationTerminatedCallback(
        [terminationPromise = std::move(terminationPromise)]() {
            terminationPromise->set_value(7);
        });

    vFed1->enterInitializingMode();

    auto res = vfut.get();
    EXPECT_EQ(cb1, 4);
    EXPECT_EQ(cb2, 3);
    EXPECT_EQ(res, 7);
    EXPECT_TRUE(vFed1->isAsyncOperationCompleted());
}

// test that the max time and period works correctly with the callback federate
TEST_F(callbackFed, timeStepsPeriodMax)
{
    SetupTest<helics::CallbackFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::CallbackFederate>(0);
    int cb1{0};
    int cb2{0};
    vFed1->setProperty(HELICS_PROPERTY_TIME_STOPTIME, 3.0);
    vFed1->setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    vFed1->setTimeRequestReturnCallback([&cb1](auto /*unused*/, bool /*unused*/) { ++cb1; });
    vFed1->setTimeRequestEntryCallback(
        [&cb2](auto /*unused*/, auto /*unused*/, bool /*unused*/) { ++cb2; });

    auto terminationPromise = std::make_shared<std::promise<int>>();
    auto vfut = terminationPromise->get_future();
    vFed1->setCosimulationTerminatedCallback(
        [terminationPromise = std::move(terminationPromise)]() {
            terminationPromise->set_value(8);
        });

    vFed1->enterInitializingMode();

    auto res = vfut.get();
    EXPECT_EQ(cb1, 4);
    EXPECT_EQ(cb2, 3);
    EXPECT_EQ(res, 8);
    EXPECT_TRUE(vFed1->isAsyncOperationCompleted());
}

TEST(callbackFederateConfig, tomlCoreBrokerAndStopTime)
{
    helics::CallbackFederate fed(std::string(TEST_DIR) + "callback_federate_config.toml");
    auto core = fed.getCorePointer();
    EXPECT_EQ(core->getIdentifier(), "callback_config_core");
    EXPECT_TRUE(core->getFlagOption(helics::gLocalCoreId, HELICS_FLAG_FORCE_LOGGING_FLUSH));
    EXPECT_EQ(fed.getTimeProperty(HELICS_PROPERTY_TIME_STOPTIME), 3.0);

    helics::Time lastGrant = helics::timeZero;
    fed.setTimeRequestReturnCallback(
        [&lastGrant](auto granted, bool /*iterating*/) { lastGrant = granted; });
    fed.setNextTimeIterativeCallback([](auto grant) {
        if (grant.grantedTime >= 4.0) {
            return std::make_pair(helics::Time::maxVal(),
                                  helics::IterationRequest::HALT_OPERATIONS);
        }
        return std::make_pair(grant.grantedTime + 1.0, helics::IterationRequest::NO_ITERATIONS);
    });

    std::promise<helics::Time> stopped;
    auto stopTime = stopped.get_future();
    fed.setCosimulationTerminatedCallback([&]() { stopped.set_value(lastGrant); });
    fed.enterInitializingMode();
    EXPECT_EQ(stopTime.get(), 3.0);
}

// test that an error return in the init call routes properly
TEST_F(callbackFed, errorInit)
{
    SetupTest<helics::CallbackFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::CallbackFederate>(0);
    vFed1->setInitializeCallback([]() { return helics::IterationRequest::ERROR_CONDITION; });

    auto errorPromise = std::make_shared<std::promise<int>>();
    auto vfut = errorPromise->get_future();
    vFed1->setErrorHandlerCallback(
        [errorPromise = std::move(errorPromise)](int code, std::string_view /*message*/) {
            errorPromise->set_value(code);
        });

    vFed1->enterInitializingMode();

    auto res = vfut.get();
    EXPECT_EQ(res, HELICS_USER_EXCEPTION);
    EXPECT_TRUE(vFed1->isAsyncOperationCompleted());
    EXPECT_EQ(vFed1->getCurrentMode(), helics::Federate::Modes::ERROR_STATE);
}

// test that an error is thrown in the init call
TEST_F(callbackFed, initException)
{
    SetupTest<helics::CallbackFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::CallbackFederate>(0);
    vFed1->setInitializeCallback(
        []() -> helics::IterationRequest { throw std::runtime_error("ETEST"); });

    auto errorPromise = std::make_shared<std::promise<std::string>>();
    auto vfut = errorPromise->get_future();
    vFed1->setErrorHandlerCallback(
        [errorPromise = std::move(errorPromise)](int /*code*/, std::string_view message) {
            errorPromise->set_value(std::string(message));
        });

    vFed1->enterInitializingMode();

    auto res = vfut.get();
    EXPECT_EQ(res, "ETEST");
    EXPECT_TRUE(vFed1->isAsyncOperationCompleted());
    EXPECT_EQ(vFed1->getCurrentMode(), helics::Federate::Modes::ERROR_STATE);
}

// test that an error return in the pre/post time request work properly
TEST_F(callbackFed, timeStepError)
{
    SetupTest<helics::CallbackFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::CallbackFederate>(0);
    int cb1{0};
    int cb2{0};
    vFed1->setTimeRequestReturnCallback([&cb1](auto /*unused*/, bool /*unused*/) { ++cb1; });
    vFed1->setTimeRequestEntryCallback(
        [&cb2](auto /*unused*/, auto /*unused*/, bool /*unused*/) { ++cb2; });

    vFed1->setNextTimeIterativeCallback([](auto grant) {
        if (grant.grantedTime >= 3.0) {
            return std::make_pair(helics::Time::maxVal(),
                                  helics::IterationRequest::ERROR_CONDITION);
        }
        return std::make_pair(grant.grantedTime + 1.0, helics::IterationRequest::NO_ITERATIONS);
    });
    auto errorPromise = std::make_shared<std::promise<int>>();
    auto vfut = errorPromise->get_future();

    vFed1->setErrorHandlerCallback(
        [errorPromise = std::move(errorPromise)](int code, std::string_view /*message*/) {
            errorPromise->set_value(code);
        });

    vFed1->enterInitializingMode();

    auto res = vfut.get();
    EXPECT_EQ(cb1, 4);
    EXPECT_EQ(cb2, 3);
    EXPECT_EQ(res, HELICS_USER_EXCEPTION);
    EXPECT_TRUE(vFed1->isAsyncOperationCompleted());
    EXPECT_EQ(vFed1->getCurrentMode(), helics::Federate::Modes::ERROR_STATE);
}

// test that the callback exception traps work
TEST_F(callbackFed, timeStepException)
{
    SetupTest<helics::CallbackFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::CallbackFederate>(0);
    int cb1{0};
    int cb2{0};
    vFed1->setTimeRequestReturnCallback([&cb1](auto /*unused*/, bool /*unused*/) { ++cb1; });
    vFed1->setTimeRequestEntryCallback(
        [&cb2](auto /*unused*/, auto /*unused*/, bool /*unused*/) { ++cb2; });

    vFed1->setNextTimeIterativeCallback([](auto grant) {
        if (grant.grantedTime >= 3.0) {
            throw std::runtime_error("ETEST");
        }

        return std::make_pair(grant.grantedTime + 1.0, helics::IterationRequest::NO_ITERATIONS);
    });
    auto errorPromise = std::make_shared<std::promise<std::string>>();
    auto vfut = errorPromise->get_future();
    vFed1->setErrorHandlerCallback(
        [errorPromise = std::move(errorPromise)](int /*code*/, std::string_view message) {
            errorPromise->set_value(std::string(message));
        });

    vFed1->enterInitializingMode();

    auto res = vfut.get();
    EXPECT_EQ(cb1, 4);
    EXPECT_EQ(cb2, 3);
    EXPECT_EQ(res, "ETEST");
    EXPECT_TRUE(vFed1->isAsyncOperationCompleted());
    EXPECT_EQ(vFed1->getCurrentMode(), helics::Federate::Modes::ERROR_STATE);
}

// test that the max time and period works correctly with two callback federates
TEST_F(callbackFed, timeSteps2Fed)
{
    SetupTest<helics::CallbackFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::CallbackFederate>(0);
    auto vFed2 = GetFederateAs<helics::CallbackFederate>(1);
    vFed1->setProperty(HELICS_PROPERTY_TIME_STOPTIME, 3.0);
    vFed1->setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    vFed2->setProperty(HELICS_PROPERTY_TIME_STOPTIME, 3.0);
    vFed2->setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    auto& pub1 = vFed1->registerGlobalPublication<double>("pub1");
    auto& pub2 = vFed2->registerGlobalPublication<double>("pub2");
    auto& sub1 = vFed1->registerSubscription("pub2");
    auto& sub2 = vFed2->registerSubscription("pub1");
    sub1.setDefault(0.3);
    sub2.setDefault(0.4);
    std::vector<double> receivedValues1;

    vFed1->setTimeRequestReturnCallback([&](auto time, bool /*unused*/) {
        pub1.publish(18.7 + static_cast<double>(time));
        receivedValues1.push_back(sub1.getValue<double>());
    });
    std::vector<double> receivedValues2;
    vFed2->setTimeRequestReturnCallback([&](auto time, bool /*unused*/) {
        pub2.publish(23.7 + static_cast<double>(time));
        receivedValues2.push_back(sub2.getValue<double>());
    });

    auto term1 = std::make_shared<std::promise<int>>();
    auto vfut1 = term1->get_future();
    auto term2 = std::make_shared<std::promise<int>>();
    auto vfut2 = term2->get_future();

    vFed1->setCosimulationTerminatedCallback([term1 = std::move(term1)]() { term1->set_value(9); });
    vFed2->setCosimulationTerminatedCallback(
        [term2 = std::move(term2)]() { term2->set_value(10); });
    vFed1->enterInitializingMode();
    vFed2->enterInitializingMode();
    auto res1 = vfut1.get();
    auto res2 = vfut2.get();

    EXPECT_EQ(res1, 9);
    EXPECT_EQ(res2, 10);
    EXPECT_TRUE(vFed1->isAsyncOperationCompleted());
    EXPECT_TRUE(vFed2->isAsyncOperationCompleted());
    ASSERT_EQ(receivedValues1.size(), 4);
    EXPECT_DOUBLE_EQ(receivedValues1[0], 0.3);
    EXPECT_DOUBLE_EQ(receivedValues1[1], 23.7);
    EXPECT_DOUBLE_EQ(receivedValues1[2], 24.7);
    EXPECT_DOUBLE_EQ(receivedValues1[3], 25.7);

    ASSERT_EQ(receivedValues2.size(), 4);
    EXPECT_DOUBLE_EQ(receivedValues2[0], 0.4);
    EXPECT_DOUBLE_EQ(receivedValues2[1], 18.7);
    EXPECT_DOUBLE_EQ(receivedValues2[2], 19.7);
    EXPECT_DOUBLE_EQ(receivedValues2[3], 20.7);
}

TEST_F(callbackFed, 100Fed_nosan)
{
    const int fedCount{100};
    SetupTest<helics::CallbackFederate>("test", fedCount);
    std::vector<double> vals(100);
    for (int ii = 0; ii < fedCount; ++ii) {
        auto fed = GetFederateAs<helics::CallbackFederate>(ii);
        fed->setProperty(HELICS_PROPERTY_TIME_STOPTIME, 120.0);
        fed->setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
        fed->registerPublication<double>("p1");
        auto& subscription = fed->registerSubscription(
            std::string("fed") + std::to_string((ii < 99) ? ii + 1 : 0) + "/p1");
        subscription.setDefault(0.7);

        fed->setTimeRequestReturnCallback([fed, ii, &vals](auto time, bool /*unused*/) {
            fed->getPublication(0).publish(static_cast<double>(ii) + static_cast<double>(time));
            vals[ii] = fed->getInput(0).getValue<double>();
        });
        fed->enterInitializingMode();
    }

    brokers.front()->waitForDisconnect();
    GetFederateAs<helics::Federate>(1)->getCorePointer()->waitForDisconnect();
    EXPECT_TRUE(GetFederateAs<helics::CallbackFederate>(1)->isAsyncOperationCompleted());

    EXPECT_DOUBLE_EQ(vals[27], 120.0 + 27.0);
    FullDisconnect();
}

TEST_F(callbackFed, 1000Fed_ci_skip_nosan)
{
    constexpr int fedCount{1000};
    SetupTest<helics::CallbackFederate>("test", fedCount);
    std::vector<double> vals(fedCount);
    for (int ii = 0; ii < fedCount; ++ii) {
        auto fed = GetFederateAs<helics::CallbackFederate>(ii);
        fed->setProperty(HELICS_PROPERTY_TIME_STOPTIME, 25.0);
        fed->setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
        fed->registerPublication<double>("p1");
        auto& subscription = fed->registerSubscription(
            std::string("fed") + std::to_string((ii < (fedCount - 1)) ? ii + 1 : 0) + "/p1");
        subscription.setDefault(0.7);

        fed->setTimeRequestReturnCallback([fed, ii, &vals](auto time, bool /*unused*/) {
            fed->getPublication(0).publish(static_cast<double>(ii) + static_cast<double>(time));
            vals[ii] = fed->getInput(0).getValue<double>();
        });
        fed->enterInitializingMode();
    }
    brokers.front()->waitForDisconnect();
    EXPECT_TRUE(GetFederateAs<helics::CallbackFederate>(1)->isAsyncOperationCompleted());
    EXPECT_DOUBLE_EQ(vals[227], 25.0 + 227.0);
    EXPECT_DOUBLE_EQ(vals[879], 25.0 + 879.0);
}

// test that time interruptions work correctly with the callbacks
TEST_F(callbackFed, timeSteps2FedIterruption)
{
    SetupTest<helics::CallbackFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::CallbackFederate>(0);
    auto vFed2 = GetFederateAs<helics::CallbackFederate>(1);
    vFed1->setProperty(HELICS_PROPERTY_TIME_STOPTIME, 18.0);
    vFed2->setProperty(HELICS_PROPERTY_TIME_STOPTIME, 18.0);
    auto& pub1 = vFed1->registerGlobalPublication<double>("pub1");
    auto& pub2 = vFed2->registerGlobalPublication<double>("pub2");
    auto& sub1 = vFed1->registerSubscription("pub2");
    auto& sub2 = vFed2->registerSubscription("pub1");
    sub1.setDefault(0.3);
    sub2.setDefault(0.4);
    std::vector<double> receivedValues1;
    std::vector<helics::Time> grantTimes1;

    vFed1->setTimeRequestReturnCallback([&](auto time, bool /*unused*/) {
        static bool trigger = false;
        if (trigger) {
            pub1.publish(22.1 + static_cast<double>(time));
        } else {
            receivedValues1.push_back(sub1.getValue<double>());
        }
        grantTimes1.push_back(time);
        trigger = !trigger;
    });

    std::vector<double> receivedValues2;
    std::vector<helics::Time> grantTimes2;
    vFed2->setTimeRequestReturnCallback([&](auto time, bool /*unused*/) {
        static bool trigger = false;
        if (trigger) {
            pub2.publish(98.7 + static_cast<double>(time));
        } else {
            receivedValues2.push_back(sub2.getValue<double>());
        }
        grantTimes2.push_back(time);
        trigger = !trigger;
    });

    auto term1 = std::make_shared<std::promise<int>>();
    auto vfut1 = term1->get_future();
    auto term2 = std::make_shared<std::promise<int>>();
    auto vfut2 = term2->get_future();

    vFed1->setNextTimeCallback([](helics::Time time) { return 1.1 * time + 1.0; });
    vFed2->setNextTimeCallback([](helics::Time time) { return 1.5 * time + 1.5; });

    vFed1->setCosimulationTerminatedCallback(
        [term1 = std::move(term1)]() { term1->set_value(11); });
    vFed2->setCosimulationTerminatedCallback(
        [term2 = std::move(term2)]() { term2->set_value(17); });
    vFed1->enterInitializingMode();
    vFed2->enterInitializingMode();
    vfut1.get();
    vfut2.get();

    EXPECT_TRUE(vFed1->isAsyncOperationCompleted());
    EXPECT_TRUE(vFed2->isAsyncOperationCompleted());
    ASSERT_GT(grantTimes2.size(), 8);
    EXPECT_EQ(grantTimes2[1], 1.0);
    ASSERT_GT(grantTimes1.size(), 8);
    EXPECT_EQ(grantTimes1[1], 1.0);
    EXPECT_GT(grantTimes1[2], 1.0);
    EXPECT_LT(grantTimes1[2], 1.1);
}
