/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "exeTestHelper.h"

#include "helics/helics.hpp"

#include "helics/apps/BrokerServer.hpp"
#include "helics/core/Core.hpp"

#include <cstdio>

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
    cleanupHelicsLibrary ();
}

BOOST_AUTO_TEST_CASE (execution_tests)
{
    apps::BrokerServer brks (std::vector<std::string>{"--zmq"});
    brks.startServers ();

    FederateInfo fi (core_type::ZMQ);
    fi.coreName = "c1";
    fi.brokerInitString = "-f 2";
    auto fed1 = ValueFederate ("fed1", fi);
    fed1.registerGlobalPublication ("key1", "double");
    fi.coreName = "c2";
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    auto fed2 = ValueFederate ("fed2", fi);
    auto &sub = fed2.registerSubscription ("key1");
    sub.setOption (helics_handle_option_connection_required);
    fed1.enterExecutingModeAsync ();
    BOOST_CHECK_NO_THROW (fed2.enterExecutingMode ());
    fed1.enterExecutingModeComplete ();

    fed1.finalize ();
    fed2.finalize ();
    brks.forceTerminate ();
    cleanupHelicsLibrary ();
}

BOOST_AUTO_TEST_CASE (execution_tests_duplicate)
{
    apps::BrokerServer brks (std::vector<std::string>{"--zmq"});
    brks.startServers ();

    FederateInfo fi (core_type::ZMQ);
    fi.coreName = "c1b";

    auto fed1 = ValueFederate ("fed1", fi);
    auto &pub1 = fed1.registerGlobalPublication ("key1", "double");
    fi.coreName = "c2b";
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    auto fed2 = ValueFederate ("fed2", fi);
    auto &sub2 = fed2.registerSubscription ("key1");
    sub2.setOption (helics_handle_option_connection_required);
    fed1.enterExecutingModeAsync ();
    BOOST_CHECK_NO_THROW (fed2.enterExecutingMode ());
    fed1.enterExecutingModeComplete ();

    fi.coreName = "c3b";
    // this would test two ZMQ co-sim executing simultaneously
    auto fed3 = ValueFederate ("fed3", fi);
    auto &pub3 = fed3.registerGlobalPublication ("key1", "double");
    fi.coreName = "c4b";
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    auto fed4 = ValueFederate ("fed4", fi);
    auto &sub4 = fed4.registerSubscription ("key1");
    sub4.setOption (helics_handle_option_connection_required);
    fed3.enterExecutingModeAsync ();
    BOOST_CHECK_NO_THROW (fed4.enterExecutingMode ());
    fed3.enterExecutingModeComplete ();

    pub1.publish (27.5);
    pub3.publish (30.6);

    fed3.requestTimeAsync (1.0);
    fed4.requestTime (1.0);
    fed3.requestTimeComplete ();

    fed1.requestTimeAsync (1.0);
    fed2.requestTime (1.0);
    fed1.requestTimeComplete ();

    BOOST_CHECK_EQUAL (sub2.getValue<double> (), 27.5);
    BOOST_CHECK_EQUAL (sub4.getValue<double> (), 30.6);
    fed1.finalize ();
    fed2.finalize ();
    fed3.finalize ();
    fed4.finalize ();
    brks.forceTerminate ();
    cleanupHelicsLibrary ();
}

BOOST_AUTO_TEST_SUITE_END ()
