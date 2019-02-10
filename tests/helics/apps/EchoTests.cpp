/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/test/data/test_case.hpp>

#include <cstdio>

#include "exeTestHelper.h"
#include "helics/application_api/Endpoints.hpp"
#include "helics/apps/Echo.hpp"
#include "helics/common/stringToCmdLine.h"
#include "helics/core/BrokerFactory.hpp"
#include <future>

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE (echo_tests, *utf::label ("ci"))

// this test will test basic echo functionality
BOOST_AUTO_TEST_CASE (echo_test1)
{
    helics::FederateInfo fi (helics::core_type::TEST);

    fi.coreName = "ecore1";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Echo echo1 ("echo1", fi);

    echo1.addEndpoint ("test");
    // fi.logLevel = 4;
    helics::MessageFederate mfed ("source", fi);
    helics::Endpoint ep1 (&mfed, "src");
    auto fut = std::async (std::launch::async, [&echo1]() { echo1.runTo (5.0); });
    mfed.enterExecutingMode ();
    ep1.send ("test", "hello world");
    auto retTime = mfed.requestTime (1.0);
    BOOST_CHECK (ep1.hasMessage ());
    BOOST_CHECK_LT (retTime, 1.0);
    auto m = ep1.getMessage ();
    BOOST_REQUIRE (m);
    BOOST_CHECK_EQUAL (m->data.to_string (), "hello world");
    BOOST_CHECK_EQUAL (m->source, "test");
    mfed.finalize ();
    fut.get ();
}

BOOST_AUTO_TEST_CASE (echo_test_delay)
{
    helics::FederateInfo fi (helics::core_type::TEST);

    fi.coreName = "ecore2";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Echo echo1 ("echo1", fi);

    echo1.addEndpoint ("test");
    echo1.setEchoDelay (1.2);
    helics::MessageFederate mfed ("source", fi);
    helics::Endpoint ep1 (&mfed, "src");
    auto fut = std::async (std::launch::async, [&echo1]() { echo1.runTo (5.0); });
    mfed.enterExecutingMode ();
    ep1.send ("test", "hello world");
    mfed.requestTime (1.0);
    BOOST_CHECK (!ep1.hasMessage ());
    auto ntime = mfed.requestTime (2.0);
    BOOST_CHECK_EQUAL (ntime, helics::timeEpsilon + 1.2);
    BOOST_CHECK (ep1.hasMessage ());
    auto m = ep1.getMessage ();
    BOOST_REQUIRE (m);
    BOOST_CHECK_EQUAL (m->data.to_string (), "hello world");
    BOOST_CHECK_EQUAL (m->source, "test");
    mfed.finalize ();
    fut.get ();
}

BOOST_AUTO_TEST_CASE (echo_test_delay_period)
{
    helics::FederateInfo fi (helics::core_type::TEST);

    fi.coreName = "ecore3";
    fi.coreInitString = "-f 2 --autobroker";
    fi.setProperty (helics_property_time_period, 1.1);
    helics::apps::Echo echo1 ("echo1", fi);
    fi.setProperty (helics_property_time_period, 0);

    echo1.addEndpoint ("test");
    echo1.setEchoDelay (1.2);
    helics::MessageFederate mfed ("source", fi);
    helics::Endpoint ep1 (&mfed, "src");
    auto fut = std::async (std::launch::async, [&echo1]() { echo1.runTo (5.0); });
    mfed.enterExecutingMode ();
    ep1.send ("test", "hello world");
    mfed.requestTime (1.0);
    BOOST_CHECK (!ep1.hasMessage ());
    auto ntime = mfed.requestTime (4.0);
    BOOST_CHECK_EQUAL (ntime, 2.3);
    BOOST_CHECK (ep1.hasMessage ());
    auto m = ep1.getMessage ();
    BOOST_REQUIRE (m);
    BOOST_CHECK_EQUAL (m->data.to_string (), "hello world");
    BOOST_CHECK_EQUAL (m->source, "test");
    mfed.finalize ();
    fut.get ();
}

BOOST_AUTO_TEST_CASE (echo_test_multiendpoint)
{
    helics::FederateInfo fi (helics::core_type::TEST);

    fi.coreName = "ecore4";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Echo echo1 ("echo1", fi);
    echo1.addEndpoint ("test");
    echo1.addEndpoint ("test2");
    echo1.setEchoDelay (1.2);
    helics::MessageFederate mfed ("source", fi);
    helics::Endpoint ep1 (&mfed, "src");
    auto fut = std::async (std::launch::async, [&echo1]() { echo1.runTo (5.0); });
    mfed.enterExecutingMode ();
    ep1.send ("test", "hello world");
    mfed.requestTime (1.0);
    ep1.send ("test2", "hello again");
    BOOST_CHECK (!ep1.hasMessage ());
    auto ntime = mfed.requestTime (2.0);
    BOOST_CHECK_EQUAL (ntime, helics::timeEpsilon + 1.2);
    BOOST_CHECK (ep1.hasMessage ());
    auto m = ep1.getMessage ();
    BOOST_REQUIRE (m);
    BOOST_CHECK_EQUAL (m->data.to_string (), "hello world");
    BOOST_CHECK_EQUAL (m->source, "test");

    ntime = mfed.requestTime (3.0);
    BOOST_CHECK_EQUAL (ntime, 2.2);
    BOOST_CHECK (ep1.hasMessage ());
    m = ep1.getMessage ();
    BOOST_REQUIRE (m);
    BOOST_CHECK_EQUAL (m->data.to_string (), "hello again");
    BOOST_CHECK_EQUAL (m->source, "test2");
    mfed.finalize ();
    fut.get ();
}

BOOST_AUTO_TEST_CASE (echo_test_fileload)
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "ecore4-file";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Echo echo1 ("echo1", fi);
    echo1.loadFile (std::string (TEST_DIR) + "/echo_example.json");

    helics::MessageFederate mfed ("source", fi);
    helics::Endpoint ep1 (&mfed, "src");
    auto fut = std::async (std::launch::async, [&echo1]() { echo1.runTo (5.0); });
    mfed.enterExecutingMode ();
    ep1.send ("test", "hello world");
    mfed.requestTime (1.0);
    ep1.send ("test2", "hello again");
    BOOST_CHECK (!ep1.hasMessage ());
    auto ntime = mfed.requestTime (2.0);
    BOOST_CHECK_EQUAL (ntime, helics::timeEpsilon + 1.2);
    BOOST_CHECK (ep1.hasMessage ());
    auto m = ep1.getMessage ();
    BOOST_REQUIRE (m);
    BOOST_CHECK_EQUAL (m->data.to_string (), "hello world");
    BOOST_CHECK_EQUAL (m->source, "test");

    ntime = mfed.requestTime (3.0);
    BOOST_CHECK_EQUAL (ntime, 2.2);
    BOOST_CHECK (ep1.hasMessage ());
    m = ep1.getMessage ();
    BOOST_REQUIRE (m);
    BOOST_CHECK_EQUAL (m->data.to_string (), "hello again");
    BOOST_CHECK_EQUAL (m->source, "test2");
    mfed.finalize ();
    fut.get ();
}

BOOST_AUTO_TEST_SUITE_END ()
