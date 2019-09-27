/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "../application_api/testFixtures.hpp"
#include "gtest/gtest.h"
#include "helics/core/core-exceptions.hpp"
#include <complex>

/** these test cases test out the value converters
 */
#include "helics/application_api.hpp"
#include "helics/core/test/TestComms.h"
#include "helics/core/test/TestCore.h"
#include <future>

#define CORE_TYPE_TO_TEST helics::core_type::TEST

TEST (broker_timeout_tests, core_fail_timeout)
{
    auto brk = helics::BrokerFactory::create (CORE_TYPE_TO_TEST, "--timeout=200ms --tick 50ms");
    brk->connect ();

    helics::FederateInfo fi (CORE_TYPE_TO_TEST);
    fi.coreName = "c1";

    auto Fed1 = std::make_shared<helics::Federate> ("test1", fi);

    fi.coreName = "c2";
    auto Fed2 = std::make_shared<helics::Federate> ("test2", fi);

    Fed1->enterExecutingModeAsync ();
    Fed2->enterExecutingMode ();
    Fed1->enterExecutingModeComplete ();

    Fed1->finalize ();

    auto cr = Fed2->getCorePointer ();
    auto tcr = std::dynamic_pointer_cast<helics::testcore::TestCore> (cr);
    EXPECT_TRUE (tcr);

    auto cms = tcr->getCommsObjectPointer ();
    cms->haltComms ();  // this will terminate communications abruptly
    tcr.reset ();

    bool val = brk->waitForDisconnect (std::chrono::milliseconds (2000));
    EXPECT_TRUE (val);
    cr->disconnect ();
    Fed2->finalize ();
}

TEST (broker_timeout_tests, core_fail_error)
{
    auto brk = helics::BrokerFactory::create (CORE_TYPE_TO_TEST, "--timeout=200ms --tick 50ms");
    brk->connect ();

    helics::FederateInfo fi (CORE_TYPE_TO_TEST);
    fi.coreName = "c3";

    auto Fed1 = std::make_shared<helics::ValueFederate> ("test1", fi);

    fi.coreName = "c4";
    auto Fed2 = std::make_shared<helics::ValueFederate> ("test2", fi);

    Fed1->registerPublication ("p1", "double");
    Fed2->registerSubscription ("test1/p1");
    Fed1->enterExecutingModeAsync ();
    Fed2->enterExecutingMode ();
    Fed1->enterExecutingModeComplete ();

    auto cr = Fed1->getCorePointer ();
    auto tcr = std::dynamic_pointer_cast<helics::testcore::TestCore> (cr);
    EXPECT_TRUE (tcr);

    auto cms = tcr->getCommsObjectPointer ();
    cms->haltComms ();  // this will terminate communications abruptly
    tcr.reset ();

    EXPECT_THROW (Fed2->requestTime (1.0), helics::HelicsException);
    cr->disconnect ();
    Fed2->finalize ();
    Fed1->finalize ();
}
