/*
Copyright (c) 2017-2021,
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

/** @file these test cases test out the real time mode for HELICS
 */

struct federate_realtime_tests: public FederateTestFixture, public ::testing::Test {
};
#define CORE_TYPE_TO_TEST helics::core_type::TEST

TEST_F(federate_realtime_tests, federate_delay_tests_ci_skip)
{
    auto broker = AddBroker("test", 1);
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreName = "cdelay";
    fi.coreInitString = std::string("-f 1 --broker=") + broker->getIdentifier();
    fi.setFlagOption(helics::defs::flags::realtime);
    fi.setProperty(helics::defs::properties::rt_lead, 0.1);
    fi.setProperty(helics::defs::properties::period, 0.5);
    auto fed = std::make_shared<helics::ValueFederate>("test1", fi);

    helics::Publication pubid(helics::GLOBAL, fed, "pub1", helics::data_type::helics_double);

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
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);

    fi.coreName = "adelay";
    fi.coreInitString = std::string("-f 2 --broker=") + broker->getIdentifier();
    fi.setFlagOption(helics::defs::flags::realtime);
    fi.setProperty(helics::defs::properties::rt_lag, 0.1);
    fi.setProperty(helics::defs::properties::rt_lead, 0.1);
    fi.setProperty(helics::defs::properties::period, 0.5);
    auto fed = std::make_shared<helics::ValueFederate>("test1", fi);
    fi.setFlagOption(helics::defs::flags::realtime, false);
    auto fed2 = std::make_shared<helics::ValueFederate>("test2", fi);
    helics::Publication pubid(helics::GLOBAL, fed2, "pub1", helics::data_type::helics_double);
    std::atomic<int> warnCounter{0};
    fed->setLoggingCallback([&warnCounter](int logLevel, const std::string&, const std::string&) {
        if (logLevel == 1) {
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
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreName = "ctrig";
    fi.coreInitString = std::string("-f 2 --broker=") + broker->getIdentifier();
    fi.setFlagOption(helics::defs::flags::realtime);
    fi.setProperty(helics::defs::properties::rt_lag, 0.1);
    fi.setProperty(helics::defs::properties::rt_lead, 0.1);
    fi.setProperty(helics::defs::properties::period, 0.5);
    fi.setProperty(helics::defs::properties::log_level, 0);

    auto fed = std::make_shared<helics::ValueFederate>("test1", fi);
    fi.setFlagOption(helics::defs::flags::realtime, false);
    auto fed2 = std::make_shared<helics::ValueFederate>("test2", fi);
    helics::Publication pubid(helics::GLOBAL, fed2, "pub1", helics::data_type::helics_double);

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
