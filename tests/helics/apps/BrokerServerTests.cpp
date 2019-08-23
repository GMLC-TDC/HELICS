/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#ifdef _MSC_VER
#pragma warning(push, 0)
#include "helics/external/filesystem.hpp"
#pragma warning(pop)
#else
#include "helics/external/filesystem.hpp"
#endif

#include "exeTestHelper.h"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/apps/BrokerServer.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/CoreFactory.hpp"

#include <cstdio>
#include <future>

namespace utf = boost::unit_test;
using namespace helics;

BOOST_AUTO_TEST_SUITE (broker_server_tests, *utf::label ("ci"))

BOOST_AUTO_TEST_CASE (startup_tests)
{
    apps::BrokerServer brks (std::vector<std::string>{"--zmq"});
    bool active = brks.hasActiveBrokers ();
    if (active)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (300));
        active = brks.hasActiveBrokers ();
    }
    BOOST_CHECK (!active);
    brks.startServers ();

    auto cr = helics::CoreFactory::create (helics::core_type::ZMQ, "--brokername=fred");
    BOOST_CHECK (cr->isConfigured ());
    cr->connect ();
    BOOST_CHECK (cr->isConnected ());

    auto cr2 = helics::CoreFactory::create (helics::core_type::ZMQ, "--brokername=fred2");
    BOOST_CHECK (cr2->isConfigured ());
    cr2->connect ();
    BOOST_CHECK (cr2->isConnected ());

    auto objs = helics::BrokerFactory::getAllBrokers ();
    BOOST_CHECK_EQUAL (objs.size (), 2u);

    brks.forceTerminate ();

    BOOST_CHECK (cr->waitForDisconnect (std::chrono::milliseconds (1000)));
    BOOST_CHECK (cr2->waitForDisconnect (std::chrono::milliseconds (1000)));
    cr->disconnect ();
    cr2->disconnect ();
}

BOOST_AUTO_TEST_CASE (execution_tests)
{
    apps::BrokerServer brks (std::vector<std::string>{"--zmq"});
    brks.startServers ();

    FederateInfo fi (core_type::ZMQ);
    fi.coreName = "c1";

    auto fed1 = ValueFederate ("fed1", fi);
    fed1.registerGlobalPublication ("key1", "double");
    fi.coreName = "c2";
    auto fed2 = ValueFederate ("fed2", fi);
    auto &sub = fed2.registerSubscription ("key1");
    sub.setOption (helics_handle_option_connection_required);
    fed1.enterExecutingModeAsync ();
    BOOST_CHECK_NO_THROW (fed2.enterExecutingMode ());
    fed1.enterExecutingModeComplete ();

    fed1.finalize ();
    fed2.finalize ();
}

BOOST_AUTO_TEST_SUITE_END ()
