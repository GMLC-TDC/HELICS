/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
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

BOOST_AUTO_TEST_SUITE (echo_tests, *utf::label("daily") *utf::label("release"))

// this test will test basic echo functionality
BOOST_AUTO_TEST_CASE (echo_test1)
{
    helics::FederateInfo fi ("echo1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core1";
    fi.coreInitString = "2";
    helics::apps::Echo echo1 (fi);
    fi.name = "source";
    echo1.addEndpoint ("test");
    // fi.logLevel = 4;
    helics::MessageFederate mfed (fi);
    helics::Endpoint ep1 (&mfed, "src");
    auto fut = std::async (std::launch::async, [&echo1]() { echo1.runTo (5.0); });
    mfed.enterExecutionState ();
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
    helics::FederateInfo fi ("echo1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core2";
    fi.coreInitString = "2";
    helics::apps::Echo echo1 (fi);
    fi.name = "source";
    echo1.addEndpoint ("test");
    echo1.setEchoDelay (1.2);
    helics::MessageFederate mfed (fi);
    helics::Endpoint ep1 (&mfed, "src");
    auto fut = std::async (std::launch::async, [&echo1]() { echo1.runTo (5.0); });
    mfed.enterExecutionState ();
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
    helics::FederateInfo fi ("echo1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core3";
    fi.coreInitString = "2";
    fi.period = 1.1;
    helics::apps::Echo echo1 (fi);
    fi.period = 0;
    fi.name = "source";
    echo1.addEndpoint ("test");
    echo1.setEchoDelay (1.2);
    helics::MessageFederate mfed (fi);
    helics::Endpoint ep1 (&mfed, "src");
    auto fut = std::async (std::launch::async, [&echo1]() { echo1.runTo (5.0); });
    mfed.enterExecutionState ();
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
    helics::FederateInfo fi ("echo1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core4";
    fi.coreInitString = "2";
    helics::apps::Echo echo1 (fi);
    fi.name = "source";
    echo1.addEndpoint ("test");
    echo1.addEndpoint ("test2");
    echo1.setEchoDelay (1.2);
    helics::MessageFederate mfed (fi);
    helics::Endpoint ep1 (&mfed, "src");
    auto fut = std::async (std::launch::async, [&echo1]() { echo1.runTo (5.0); });
    mfed.enterExecutionState ();
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
    helics::FederateInfo fi ("echo1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core4";
    fi.coreInitString = "2";
    helics::apps::Echo echo1 (fi);
    echo1.loadFile (std::string (TEST_DIR) + "/test_files/echo_example.json");
    fi.name = "source";

    helics::MessageFederate mfed (fi);
    helics::Endpoint ep1 (&mfed, "src");
    auto fut = std::async (std::launch::async, [&echo1]() { echo1.runTo (5.0); });
    mfed.enterExecutionState ();
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
