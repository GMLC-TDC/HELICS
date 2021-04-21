/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../application_api/testFixtures.hpp"
#include "helics/core/core-exceptions.hpp"

#include "gtest/gtest.h"
#include <complex>

/** these test cases test out the value converters
 */
#include "helics/application_api.hpp"
#include "helics/network/test/TestComms.h"
#include "helics/network/test/TestCore.h"

#include <future>

#define CORE_TYPE_TO_TEST helics::core_type::TEST

TEST(broker_timeout_tests, core_fail_timeout)
{
    auto brk = helics::BrokerFactory::create(CORE_TYPE_TO_TEST, "--timeout=200ms --tick 50ms");
    brk->connect();

    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreName = "c1";

    auto Fed1 = std::make_shared<helics::Federate>("test1", fi);

    fi.coreName = "c2";
    auto Fed2 = std::make_shared<helics::Federate>("test2", fi);

    Fed1->enterExecutingModeAsync();
    Fed2->enterExecutingMode();
    Fed1->enterExecutingModeComplete();

    Fed1->finalize();

    auto cr = Fed2->getCorePointer();
    auto tcr = std::dynamic_pointer_cast<helics::testcore::TestCore>(cr);
    EXPECT_TRUE(tcr);

    auto cms = tcr->getCommsObjectPointer();
    cms->haltComms();  // this will terminate communications abruptly
    tcr.reset();

    bool val = brk->waitForDisconnect(std::chrono::milliseconds(2000));
    EXPECT_TRUE(val);
    cr->disconnect();
    Fed2->finalize();
}
// this test is exactly like the previous one except the core was specified with no_ping so it won't
// fail
TEST(broker_timeout_tests, core_fail_timeout_no_ping_ci_skip)
{
    auto brk = helics::BrokerFactory::create(CORE_TYPE_TO_TEST, "--timeout=200ms --tick 50ms");
    brk->connect();

    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreName = "c1";

    auto Fed1 = std::make_shared<helics::Federate>("test1", fi);

    fi.coreName = "c2";
    fi.coreInitString = "--no_ping";

    auto Fed2 = std::make_shared<helics::Federate>("test2", fi);

    Fed1->enterExecutingModeAsync();
    Fed2->enterExecutingMode();
    Fed1->enterExecutingModeComplete();

    Fed1->finalize();

    auto cr = Fed2->getCorePointer();
    auto tcr = std::dynamic_pointer_cast<helics::testcore::TestCore>(cr);
    EXPECT_TRUE(tcr);

    auto cms = tcr->getCommsObjectPointer();
    cms->haltComms();  // this will terminate communications abruptly
    tcr.reset();

    bool val = brk->waitForDisconnect(std::chrono::milliseconds(2000));
    EXPECT_FALSE(val);
    brk->disconnect();

    cr->disconnect();
    Fed1->finalize();
    Fed2->finalize();
}

// this test is exactly like the previous one except the core was specified with no_ping so it won't
// fail
TEST(broker_timeout_tests, core_fail_debugging_ci_skip)
{
    auto brk = helics::BrokerFactory::create(CORE_TYPE_TO_TEST, "--timeout=200ms --tick 50ms ");
    brk->connect();

    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreName = "c1";

    auto Fed1 = std::make_shared<helics::Federate>("test1", fi);

    fi.coreName = "c2";
    fi.coreInitString = "--debugging";

    auto Fed2 = std::make_shared<helics::Federate>("test2", fi);

    Fed1->enterExecutingModeAsync();
    Fed2->enterExecutingMode();
    Fed1->enterExecutingModeComplete();

    Fed1->finalize();

    auto cr = Fed2->getCorePointer();
    auto tcr = std::dynamic_pointer_cast<helics::testcore::TestCore>(cr);
    EXPECT_TRUE(tcr);

    auto cms = tcr->getCommsObjectPointer();
    cms->haltComms();  // this will terminate communications abruptly
    tcr.reset();

    bool val = brk->waitForDisconnect(std::chrono::milliseconds(2000));
    EXPECT_FALSE(val);
    brk->disconnect();

    cr->disconnect();
    Fed1->finalize();
    Fed2->finalize();
}

// this test is similar in concept to the previous two but using --disable_timer flag
TEST(broker_timeout_tests, core_fail_timeout_no_timer_ci_skip)
{
    auto brk = helics::BrokerFactory::create(CORE_TYPE_TO_TEST,
                                             "--timeout=200ms --tick 50ms --disable_timer");
    brk->connect();

    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreName = "c1";

    auto Fed1 = std::make_shared<helics::Federate>("test1", fi);

    fi.coreName = "c2";

    auto Fed2 = std::make_shared<helics::Federate>("test2", fi);

    Fed1->enterExecutingModeAsync();
    Fed2->enterExecutingMode();
    Fed1->enterExecutingModeComplete();

    Fed1->finalize();

    auto cr = Fed2->getCorePointer();
    auto tcr = std::dynamic_pointer_cast<helics::testcore::TestCore>(cr);
    EXPECT_TRUE(tcr);

    auto cms = tcr->getCommsObjectPointer();
    cms->haltComms();  // this will terminate communications abruptly
    tcr.reset();

    bool val = brk->waitForDisconnect(std::chrono::milliseconds(2000));
    EXPECT_FALSE(val);
    brk->disconnect();

    cr->disconnect();
    Fed1->finalize();
    Fed2->finalize();
}

TEST(broker_timeout_tests, core_fail_error)
{
    auto brk =
        helics::BrokerFactory::create(helics::core_type::TEST, "--timeout=200ms --tick 50ms");
    brk->connect();

    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "c3";

    auto Fed1 = std::make_shared<helics::ValueFederate>("test1", fi);

    fi.coreName = "c4";
    auto Fed2 = std::make_shared<helics::ValueFederate>("test2", fi);

    Fed1->registerPublication("p1", "double");
    Fed2->registerSubscription("test1/p1");
    Fed1->enterExecutingModeAsync();
    Fed2->enterExecutingMode();
    Fed1->enterExecutingModeComplete();

    auto cr = Fed1->getCorePointer();
    auto tcr = std::dynamic_pointer_cast<helics::testcore::TestCore>(cr);
    EXPECT_TRUE(tcr);

    auto cms = tcr->getCommsObjectPointer();
    cms->haltComms();  // this will terminate communications abruptly
    tcr.reset();

    EXPECT_THROW(Fed2->requestTime(1.0), helics::HelicsException);
    cr->disconnect();
    Fed2->finalize();
    Fed1->finalize();
}

#ifdef ENABLE_ZMQ_CORE
TEST(broker_timeout_tests, maintain_connection_ci_skip)
{
    auto brk = helics::BrokerFactory::create(helics::core_type::ZMQ, "--timeout=100ms");
    brk->connect();

    helics::FederateInfo fi(helics::core_type::ZMQ);
    fi.coreName = "c3";
    fi.setProperty(helics_property_time_delta, 0.01);
    auto Fed1 = std::make_shared<helics::ValueFederate>("test1", fi);

    Fed1->registerGlobalPublication<std::string>("pub1");
    auto& sub1 = Fed1->registerSubscription("pub1");
    sub1.setDefault(std::string("String1"));

    Fed1->enterExecutingMode();
    int counter = 50;
    while (counter > 0) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        --counter;
    }
    EXPECT_TRUE(brk->isConnected());
    EXPECT_TRUE(Fed1->getCorePointer()->isConnected());
    Fed1->finalize();
    brk->waitForDisconnect();
}
#endif
