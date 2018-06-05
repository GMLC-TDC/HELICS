/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <future>

#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"
#include "testFixtures.hpp"
#include <chrono>

/** @file these test cases test out the real time mode for HELICS
*/


#define CORE_TYPE_TO_TEST helics::core_type::TEST
BOOST_AUTO_TEST_SUITE(federate_realtime_tests)

BOOST_AUTO_TEST_CASE(federate_delay_tests)
{
    helics::FederateInfo fi("test1");
    fi.coreType = CORE_TYPE_TO_TEST;
    fi.coreInitString = "1";
    fi.realtime = true;
    fi.rt_lead = 0.1;
    fi.period = 0.5;
    auto fed = std::make_shared<helics::ValueFederate>(fi);

    helics::Publication pubid(helics::GLOBAL, fed, "pub1", helics::helics_type_t::helicsDouble);

    helics::Subscription subid(fed, "pub1");
    fed->enterExecutionState();
    // publish string1 at time=0.0;
    auto now = std::chrono::steady_clock::now();
    helics::Time reqTime = 0.5;
    for (int ii = 0; ii < 8; ++ii)
    {
        pubid.publish(static_cast<double>(reqTime));
        auto gtime = fed->requestTime(reqTime);
        auto ctime = std::chrono::steady_clock::now();
        BOOST_CHECK_EQUAL(gtime, reqTime);
        auto td = ctime - now;
        auto tdiff = helics::Time(td) - reqTime;
        BOOST_CHECK(tdiff >= -0.106);
        if (tdiff < -0.11)
        {
            printf("tdiff=%f at time %f\n", static_cast<double>(tdiff), static_cast<double>(reqTime));
        }
        reqTime += 0.5;
    }
    fed->finalize();
}

BOOST_AUTO_TEST_SUITE_END()