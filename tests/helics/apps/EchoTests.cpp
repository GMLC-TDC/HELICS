/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

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

BOOST_AUTO_TEST_SUITE(echo_tests)

// this test will test basic echo functionality
BOOST_AUTO_TEST_CASE(echo_test1)
{
    helics::FederateInfo fi("echo1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core1";
    fi.coreInitString = "2";
    helics::Echo echo1(fi);
    fi.name = "source";
    echo1.addEndpoint("test");

    helics::MessageFederate mfed(fi);
    helics::Endpoint ep1(&mfed, "src");
    auto fut = std::async(std::launch::async, [&echo1]() { echo1.run(5.0); });
    mfed.enterExecutionState();
    ep1.send("test", "hello world");
    auto retTime=mfed.requestTime(1.0);
    BOOST_CHECK(ep1.hasMessage());
    BOOST_CHECK_LT(retTime, 1.0);
    auto m = ep1.getMessage();
    BOOST_REQUIRE(m);
    BOOST_CHECK_EQUAL(m->data.to_string(), "hello world");
    BOOST_CHECK_EQUAL(m->source, "test");
    mfed.finalize();
    fut.get();
}

BOOST_AUTO_TEST_CASE(echo_test_delay)
{
    helics::FederateInfo fi("echo1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core1";
    fi.coreInitString = "2";
    helics::Echo echo1(fi);
    fi.name = "source";
    echo1.addEndpoint("test");
    echo1.setEchoDelay(1.2);
    helics::MessageFederate mfed(fi);
    helics::Endpoint ep1(&mfed, "src");
    auto fut = std::async(std::launch::async, [&echo1]() { echo1.run(5.0); });
    mfed.enterExecutionState();
    ep1.send("test", "hello world");
    mfed.requestTime(1.0);
    BOOST_CHECK(!ep1.hasMessage());
    auto ntime=mfed.requestTime(2.0);
    BOOST_CHECK_EQUAL(ntime, helics::timeEpsilon+1.2);
    BOOST_CHECK(ep1.hasMessage());
    auto m = ep1.getMessage();
    BOOST_REQUIRE(m);
    BOOST_CHECK_EQUAL(m->data.to_string(), "hello world");
    BOOST_CHECK_EQUAL(m->source, "test");
    mfed.finalize();
    fut.get();
}

BOOST_AUTO_TEST_SUITE_END()