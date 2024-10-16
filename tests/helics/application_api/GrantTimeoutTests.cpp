/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/MessageFederate.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/core/BrokerFactory.hpp"
// #include "helics/core/CoreFactory.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/core-exceptions.hpp"
#include "helics/core/helics_definitions.hpp"

#include <filesystem>
#include <future>
#include <gmlc/libguarded/guarded.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

/** these test cases test out user-directed logging functionality
 */

#define CORE_TYPE_TO_TEST helics::CoreType::TEST

TEST(grant_timeout, phase1)
{
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString = "--autobroker --name gtcore1";
    fedInfo.setProperty(helics::defs::Properties::LOG_LEVEL, HELICS_LOG_LEVEL_SUMMARY);

    auto Fed1 = std::make_shared<helics::ValueFederate>("test1", fedInfo);
    fedInfo.coreInitString.clear();
    fedInfo.coreName = "gtcore1";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_GRANT_TIMEOUT, 0.3);
    auto Fed2 = std::make_shared<helics::ValueFederate>("test2", fedInfo);
    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    Fed2->setLoggingCallback(
        [&mlog](int level, std::string_view /*unused*/, std::string_view message) {
            mlog.lock()->emplace_back(level, message);
        });
    Fed1->registerGlobalPublication<double>("pub1");
    Fed2->registerSubscription("pub1");

    Fed1->enterExecutingModeAsync();
    Fed2->enterExecutingMode();
    Fed1->enterExecutingModeComplete();

    Fed2->requestTimeAsync(2.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(400));

    auto llock = mlog.lock();
    while (llock->empty()) {
        llock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        llock = mlog.lock();
    }

    EXPECT_NE((*llock)[0].second.find("grant timeout"), std::string::npos);
    EXPECT_NE((*llock)[0].second.find("131072"), std::string::npos);

    Fed1->requestTime(3.0);
    auto res = Fed2->requestTimeComplete();
    EXPECT_EQ(res, 2.0);
    Fed1->finalize();
    Fed2->finalize();
}

TEST(grant_timeout, phase2)
{
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString = "--autobroker --name gtcore2";
    fedInfo.setProperty(helics::defs::Properties::LOG_LEVEL, HELICS_LOG_LEVEL_SUMMARY);

    auto Fed1 = std::make_shared<helics::ValueFederate>("test1", fedInfo);
    fedInfo.coreInitString.clear();
    fedInfo.coreName = "gtcore2";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_GRANT_TIMEOUT, 0.2);
    auto Fed2 = std::make_shared<helics::ValueFederate>("test2", fedInfo);
    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    Fed2->setLoggingCallback(
        [&mlog](int level, std::string_view /*unused*/, std::string_view message) {
            mlog.lock()->emplace_back(level, message);
        });
    Fed1->registerGlobalPublication<double>("pub1");
    Fed2->registerSubscription("pub1");

    Fed1->enterExecutingModeAsync();
    Fed2->enterExecutingMode();
    Fed1->enterExecutingModeComplete();

    Fed2->requestTimeAsync(2.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    auto llock = mlog.lock();
    while (llock->size() < 2) {
        llock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        llock = mlog.lock();
    }

    EXPECT_NE((*llock)[0].second.find("grant timeout"), std::string::npos);
    EXPECT_NE((*llock)[0].second.find("131072"), std::string::npos);
    EXPECT_NE((*llock)[1].second.find("stage 2"), std::string::npos);
    Fed1->requestTime(3.0);
    auto res = Fed2->requestTimeComplete();
    EXPECT_EQ(res, 2.0);
    Fed1->finalize();
    Fed2->finalize();
}

TEST(grant_timeout, phase3)
{
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString = "--autobroker --name gtcore3";
    fedInfo.setProperty(helics::defs::Properties::LOG_LEVEL, HELICS_LOG_LEVEL_SUMMARY);

    auto Fed1 = std::make_shared<helics::ValueFederate>("test1", fedInfo);
    fedInfo.coreInitString.clear();
    fedInfo.coreName = "gtcore3";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_GRANT_TIMEOUT, 0.1);
    auto Fed2 = std::make_shared<helics::ValueFederate>("test2", fedInfo);
    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    Fed2->setLoggingCallback(
        [&mlog](int level, std::string_view /*unused*/, std::string_view message) {
            mlog.lock()->emplace_back(level, message);
        });
    Fed1->registerGlobalPublication<double>("pub1");
    Fed2->registerSubscription("pub1");

    Fed1->enterExecutingModeAsync();
    Fed2->enterExecutingMode();
    Fed1->enterExecutingModeComplete();

    Fed2->requestTimeAsync(2.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(600));

    auto llock = mlog.lock();
    while (llock->size() < 4) {
        llock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        llock = mlog.lock();
    }

    EXPECT_NE((*llock)[2].second.find("stage 3"), std::string::npos);
    EXPECT_NE((*llock)[3].second.find("TIME DEBUGGING"), std::string::npos);
    llock.unlock();
    Fed1->requestTime(3.0);
    auto res = Fed2->requestTimeComplete();
    EXPECT_EQ(res, 2.0);
    Fed1->finalize();
    Fed2->finalize();
}

TEST(grant_timeout, phase3_core)
{
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString = "--autobroker --name gtcore3b";
    fedInfo.setProperty(helics::defs::Properties::LOG_LEVEL, HELICS_LOG_LEVEL_SUMMARY);

    auto Fed1 = std::make_shared<helics::MessageFederate>("test1", fedInfo);
    fedInfo.coreInitString.clear();
    fedInfo.coreName = "gtcore3b";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_GRANT_TIMEOUT, 0.1);
    auto Fed2 = std::make_shared<helics::MessageFederate>("test2", fedInfo);
    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> clog;
    Fed2->setLoggingCallback(
        [&mlog](int level, std::string_view /*unused*/, std::string_view message) {
            mlog.lock()->emplace_back(level, message);
        });

    Fed2->getCorePointer()->setLoggingCallback(helics::gLocalCoreId,
                                               [&clog](int level,
                                                       std::string_view /*unused*/,
                                                       std::string_view message) {
                                                   clog.lock()->emplace_back(level, message);
                                               });
    Fed1->registerGlobalEndpoint("e1");
    Fed2->registerGlobalEndpoint("e2");

    Fed1->enterExecutingModeAsync();
    Fed2->enterExecutingMode();
    Fed1->enterExecutingModeComplete();

    Fed2->requestTimeAsync(2.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(600));

    auto llock = mlog.lock();
    while (llock->size() < 4) {
        llock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        llock = mlog.lock();
    }

    EXPECT_NE((*llock)[2].second.find("stage 3"), std::string::npos);
    EXPECT_NE((*llock)[3].second.find("TIME DEBUGGING"), std::string::npos);
    llock.unlock();
    auto clock = clog.lock();
    EXPECT_NE((*clock)[1].second.find("TIME DEBUGGING"), std::string::npos);
    clock.unlock();
    Fed1->requestTimeAsync(3.0);
    auto res = Fed2->requestTimeComplete();
    Fed2->finalize();
    EXPECT_EQ(res, 2.0);
    res = Fed1->requestTimeComplete();
    Fed1->finalize();
    EXPECT_EQ(res, 3.0);
}

TEST(grant_timeout, phase4)
{
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString = "--autobroker --name gtcore4";
    fedInfo.setProperty(helics::defs::Properties::LOG_LEVEL, HELICS_LOG_LEVEL_SUMMARY);

    auto Fed1 = std::make_shared<helics::ValueFederate>("test1", fedInfo);
    fedInfo.coreInitString.clear();
    fedInfo.coreName = "gtcore4";
    fedInfo.setProperty(HELICS_PROPERTY_TIME_GRANT_TIMEOUT, 0.1);
    auto Fed2 = std::make_shared<helics::ValueFederate>("test2", fedInfo);
    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    Fed2->setLoggingCallback(
        [&mlog](int level, std::string_view /*unused*/, std::string_view message) {
            mlog.lock()->emplace_back(level, message);
        });
    Fed1->registerGlobalPublication<double>("pub1");
    Fed2->registerSubscription("pub1");

    Fed1->enterExecutingModeAsync();
    Fed2->enterExecutingMode();
    Fed1->enterExecutingModeComplete();

    Fed2->requestTimeAsync(2.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(700));

    auto llock = mlog.lock();
    while (llock->size() < 5) {
        llock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        llock = mlog.lock();
    }

    EXPECT_NE((*llock)[4].second.find("stage 4"), std::string::npos);
    Fed1->requestTime(3.0);
    auto res = Fed2->requestTimeComplete();
    EXPECT_EQ(res, 2.0);
    Fed1->finalize();
    Fed2->finalize();
}
