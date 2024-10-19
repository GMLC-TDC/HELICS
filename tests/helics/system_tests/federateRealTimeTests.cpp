/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include "../application_api/testFixtures.hpp"
#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/helics_definitions.hpp"

#include "gtest/gtest.h"
#include <chrono>
#include <future>
#include <memory>
#include <string>
#include <thread>

/** @file these test cases test out the real time mode for HELICS
 */

struct federate_realtime_tests: public FederateTestFixture, public ::testing::Test {};
#define CORE_TYPE_TO_TEST helics::CoreType::TEST

TEST_F(federate_realtime_tests, federate_delay_tests_ci_skip)
{
    auto broker = AddBroker("test", 1);
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreName = "cdelay";
    fedInfo.coreInitString = std::string("-f 1 --broker=") + broker->getIdentifier();
    fedInfo.setFlagOption(helics::defs::Flags::REALTIME);
    fedInfo.setProperty(helics::defs::Properties::RT_LEAD, 0.1);
    fedInfo.setProperty(helics::defs::Properties::PERIOD, 0.5);
    auto fed = std::make_shared<helics::ValueFederate>("test1", fedInfo);

    helics::Publication pubid(helics::InterfaceVisibility::GLOBAL,
                              fed,
                              "pub1",
                              helics::DataType::HELICS_DOUBLE);

    fed->registerSubscription("pub1");
    fed->enterExecutingMode();
    // publish string1 at time=0.0;
    auto now = std::chrono::steady_clock::now();
    helics::Time reqTime = 0.5;
    int outofTimeBounds = 0;
    for (int ii = 0; ii < 8; ++ii) {
        pubid.publish(static_cast<double>(reqTime));
        auto gtime = fed->requestTime(reqTime);
        auto ctime = std::chrono::steady_clock::now();
        EXPECT_EQ(gtime, reqTime);
        auto td = ctime - now;
        auto tdiff = helics::Time(td) - reqTime;

        if (tdiff < -0.15) {
            ++outofTimeBounds;
            //   printf ("tdiff=%f at time %f\n", static_cast<double> (tdiff), static_cast<double>
            //   (reqTime));
        }
        reqTime += 0.5;
    }
    EXPECT_LT(outofTimeBounds, 3);
    fed->finalize();
    broker->disconnect();
}

TEST_F(federate_realtime_tests, federate_trigger_tests_adelay_ci_skip)
{
    auto broker = AddBroker("test", 1);
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);

    fedInfo.coreName = "adelay";
    fedInfo.coreInitString = std::string("-f 2 --broker=") + broker->getIdentifier();
    fedInfo.setFlagOption(helics::defs::Flags::REALTIME);
    fedInfo.setProperty(helics::defs::Properties::RT_LAG, 0.1);
    fedInfo.setProperty(helics::defs::Properties::RT_LEAD, 0.1);
    fedInfo.setProperty(helics::defs::Properties::PERIOD, 0.5);
    auto fed = std::make_shared<helics::ValueFederate>("test1", fedInfo);
    fedInfo.setFlagOption(helics::defs::Flags::REALTIME, false);
    auto fed2 = std::make_shared<helics::ValueFederate>("test2", fedInfo);
    helics::Publication pubid(helics::InterfaceVisibility::GLOBAL,
                              fed2,
                              "pub1",
                              helics::DataType::HELICS_DOUBLE);
    std::atomic<int> warnCounter{0};
    fed->setLoggingCallback(
        [&warnCounter](int logLevel, std::string_view /*unused*/, std::string_view /*unused*/) {
            if (logLevel == HELICS_LOG_LEVEL_WARNING) {
                ++warnCounter;
            }
        });
    fed->registerSubscription("pub1");
    fed2->enterExecutingModeAsync();
    fed->enterExecutingMode();
    fed2->enterExecutingModeComplete();
    // publish string1 at time=0.0;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    helics::Time reqTime = 0.5;
    for (int ii = 0; ii < 8; ++ii) {
        if (ii < 5) {  // this is designed to go faster than real time for a little while, then the
                       // dependency will halt and
            // should trigger the force grant
            pubid.publish(static_cast<double>(reqTime));
            fed2->requestTimeAsync(reqTime);
        }

        auto gtime = fed->requestTime(reqTime);
        EXPECT_EQ(gtime, reqTime);

        reqTime += 0.5;
        if (ii < 5) {
            fed2->requestTimeComplete();
        }
    }
    EXPECT_EQ(warnCounter, 8);
    fed2->finalize();
    fed->finalize();
    broker->disconnect();
}

TEST_F(federate_realtime_tests, federate_trigger_tests_ci_skip)
{
    auto broker = AddBroker("test", 1);
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreName = "ctrig";
    fedInfo.coreInitString = std::string("-f 2 --broker=") + broker->getIdentifier();
    fedInfo.setFlagOption(helics::defs::Flags::REALTIME);
    fedInfo.setProperty(helics::defs::Properties::RT_LAG, 0.1);
    fedInfo.setProperty(helics::defs::Properties::RT_LEAD, 0.1);
    fedInfo.setProperty(helics::defs::Properties::PERIOD, 0.5);
    fedInfo.setProperty(helics::defs::Properties::LOG_LEVEL, HELICS_LOG_LEVEL_ERROR);

    auto fed = std::make_shared<helics::ValueFederate>("test1", fedInfo);
    fedInfo.setFlagOption(helics::defs::Flags::REALTIME, false);
    auto fed2 = std::make_shared<helics::ValueFederate>("test2", fedInfo);
    helics::Publication pubid(helics::InterfaceVisibility::GLOBAL,
                              fed2,
                              "pub1",
                              helics::DataType::HELICS_DOUBLE);

    fed->registerSubscription("pub1");
    fed2->enterExecutingModeAsync();
    fed->enterExecutingMode();
    fed2->enterExecutingModeComplete();
    // publish string1 at time=0.0;
    auto now = std::chrono::steady_clock::now();
    helics::Time reqTime = 0.5;
    int outofTimeBounds = 0;
    for (int ii = 0; ii < 8; ++ii) {
        if (ii < 5) {  // this is designed to go faster than real time for a little while, then the
                       // dependency will halt and
            // should trigger the force grant
            pubid.publish(static_cast<double>(reqTime));
            fed2->requestTimeAsync(reqTime);
        }

        auto gtime = fed->requestTime(reqTime);
        auto ctime = std::chrono::steady_clock::now();
        EXPECT_EQ(gtime, reqTime);
        auto td = ctime - now;
        auto tdiff = helics::Time(td) - reqTime;
        EXPECT_TRUE(tdiff >= -0.15);
        if (tdiff < -0.15) {
            ++outofTimeBounds;
            // printf ("tdiff=%f at time %f\n", static_cast<double> (tdiff), static_cast<double>
            // (reqTime));
        }
        if (tdiff > 0.15) {
            ++outofTimeBounds;
            // printf ("tdiff=%f at time %f\n", static_cast<double> (tdiff), static_cast<double>
            // (reqTime));
        }
        reqTime += 0.5;
        if (ii < 5) {
            fed2->requestTimeComplete();
        }
    }
    EXPECT_LT(outofTimeBounds, 3);
    fed2->finalize();
    fed->finalize();
    broker->disconnect();
}
