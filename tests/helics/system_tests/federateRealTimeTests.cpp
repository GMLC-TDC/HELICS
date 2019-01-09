/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <future>

#include "../application_api/testFixtures.hpp"
#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/helics_definitions.hpp"
#include <chrono>

/** @file these test cases test out the real time mode for HELICS
 */

#define CORE_TYPE_TO_TEST helics::core_type::TEST
BOOST_FIXTURE_TEST_SUITE (federate_realtime_tests, FederateTestFixture)

BOOST_AUTO_TEST_CASE (federate_delay_tests)
{
    auto broker = AddBroker ("test", 1);
    helics::FederateInfo fi (CORE_TYPE_TO_TEST);
    fi.coreName = "cdelay";
    fi.coreInitString = std::string ("-f 1 --broker=") + broker->getIdentifier ();
    fi.setFlagOption (helics::defs::flags::realtime);
    fi.setProperty (helics::defs::properties::rt_lead, 0.1);
    fi.setProperty (helics::defs::properties::period, 0.5);
    auto fed = std::make_shared<helics::ValueFederate> ("test1", fi);

    helics::Publication pubid (helics::GLOBAL, fed, "pub1", helics::data_type::helics_double);

    fed->registerSubscription ("pub1");
    fed->enterExecutingMode ();
    // publish string1 at time=0.0;
    auto now = std::chrono::steady_clock::now ();
    helics::Time reqTime = 0.5;
    int outofTimeBounds = 0;
    for (int ii = 0; ii < 8; ++ii)
    {
        pubid.publish (static_cast<double> (reqTime));
        auto gtime = fed->requestTime (reqTime);
        auto ctime = std::chrono::steady_clock::now ();
        BOOST_CHECK_EQUAL (gtime, reqTime);
        auto td = ctime - now;
        auto tdiff = helics::Time (td) - reqTime;

        if (tdiff < -0.15)
        {
            ++outofTimeBounds;
            //   printf ("tdiff=%f at time %f\n", static_cast<double> (tdiff), static_cast<double> (reqTime));
        }
        reqTime += 0.5;
    }
    BOOST_CHECK_LT (outofTimeBounds, 3);
    fed->finalize ();
    broker->disconnect ();
}

BOOST_AUTO_TEST_CASE (federate_trigger_tests_adelay)
{
    auto broker = AddBroker ("test", 1);
    helics::FederateInfo fi (CORE_TYPE_TO_TEST);

    fi.coreName = "adelay";
    fi.coreInitString = std::string ("-f 2 --broker=") + broker->getIdentifier ();
    fi.setFlagOption (helics::defs::flags::realtime);
    fi.setProperty (helics::defs::properties::rt_lag, 0.1);
    fi.setProperty (helics::defs::properties::rt_lead, 0.1);
    fi.setProperty (helics::defs::properties::period, 0.5);
    auto fed = std::make_shared<helics::ValueFederate> ("test1", fi);
    fi.setFlagOption (helics::defs::flags::realtime, false);
    auto fed2 = std::make_shared<helics::ValueFederate> ("test2", fi);
    helics::Publication pubid (helics::GLOBAL, fed2, "pub1", helics::data_type::helics_double);
    std::atomic<int> warnCounter{0};
    fed->setLoggingCallback ([&warnCounter](int logLevel, const std::string &, const std::string &) {
        if (logLevel == 1)
        {
            ++warnCounter;
        }
    });
    fed->registerSubscription ("pub1");
    fed2->enterExecutingModeAsync ();
    fed->enterExecutingMode ();
    fed2->enterExecutingModeComplete ();
    // publish string1 at time=0.0;
    std::this_thread::sleep_for (std::chrono::seconds (5));
    helics::Time reqTime = 0.5;
    for (int ii = 0; ii < 8; ++ii)
    {
        if (ii < 5)
        {  // this is designed to go faster than real time for a little while, then the dependency will halt and
           // should trigger the force grant
            pubid.publish (static_cast<double> (reqTime));
            fed2->requestTimeAsync (reqTime);
        }

        auto gtime = fed->requestTime (reqTime);
        BOOST_CHECK_EQUAL (gtime, reqTime);

        reqTime += 0.5;
        if (ii < 5)
        {
            fed2->requestTimeComplete ();
        }
    }
    BOOST_CHECK_EQUAL (warnCounter, 8);
    fed2->finalize ();
    fed->finalize ();
    broker->disconnect ();
}

BOOST_AUTO_TEST_CASE (federate_trigger_tests)
{
    auto broker = AddBroker ("test", 1);
    helics::FederateInfo fi (CORE_TYPE_TO_TEST);
    fi.coreName = "ctrig";
    fi.coreInitString = std::string ("-f 2 --broker=") + broker->getIdentifier ();
    fi.setFlagOption (helics::defs::flags::realtime);
    fi.setProperty (helics::defs::properties::rt_lag, 0.1);
    fi.setProperty (helics::defs::properties::rt_lead, 0.1);
    fi.setProperty (helics::defs::properties::period, 0.5);
    fi.setProperty (helics::defs::properties::log_level, 0);

    auto fed = std::make_shared<helics::ValueFederate> ("test1", fi);
    fi.setFlagOption (helics::defs::flags::realtime, false);
    auto fed2 = std::make_shared<helics::ValueFederate> ("test2", fi);
    helics::Publication pubid (helics::GLOBAL, fed2, "pub1", helics::data_type::helics_double);

    fed->registerSubscription ("pub1");
    fed2->enterExecutingModeAsync ();
    fed->enterExecutingMode ();
    fed2->enterExecutingModeComplete ();
    // publish string1 at time=0.0;
    auto now = std::chrono::steady_clock::now ();
    helics::Time reqTime = 0.5;
    int outofTimeBounds = 0;
    for (int ii = 0; ii < 8; ++ii)
    {
        if (ii < 5)
        {  // this is designed to go faster than real time for a little while, then the dependency will halt and
           // should trigger the force grant
            pubid.publish (static_cast<double> (reqTime));
            fed2->requestTimeAsync (reqTime);
        }

        auto gtime = fed->requestTime (reqTime);
        auto ctime = std::chrono::steady_clock::now ();
        BOOST_CHECK_EQUAL (gtime, reqTime);
        auto td = ctime - now;
        auto tdiff = helics::Time (td) - reqTime;
        BOOST_CHECK (tdiff >= -0.15);
        if (tdiff < -0.15)
        {
            ++outofTimeBounds;
            // printf ("tdiff=%f at time %f\n", static_cast<double> (tdiff), static_cast<double> (reqTime));
        }
        if (tdiff > 0.15)
        {
            ++outofTimeBounds;
            // printf ("tdiff=%f at time %f\n", static_cast<double> (tdiff), static_cast<double> (reqTime));
        }
        reqTime += 0.5;
        if (ii < 5)
        {
            fed2->requestTimeComplete ();
        }
    }
    BOOST_CHECK_LT (outofTimeBounds, 3);
    fed2->finalize ();
    fed->finalize ();
    broker->disconnect ();
}
BOOST_AUTO_TEST_SUITE_END ()
