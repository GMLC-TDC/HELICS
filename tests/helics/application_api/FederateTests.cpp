/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/Federate.hpp"
#include "helics/core/BrokerFactory.hpp"
//#include "helics/core/CoreFactory.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/core-exceptions.hpp"
#include <future>
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/floating_point_comparison.hpp>
/** these test cases test out the value converters
 */

#define CORE_TYPE_TO_TEST helics::core_type::TEST

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE (federate_tests)

BOOST_AUTO_TEST_CASE (federate_initialize_tests, *utf::label ("ci"))
{
    helics::FederateInfo fi (CORE_TYPE_TO_TEST);
    fi.coreInitString = "--autobroker";

    auto Fed = std::make_shared<helics::Federate> ("test1", fi);

    auto core = Fed->getCorePointer ();
    BOOST_REQUIRE ((core));

    auto name = std::string (core->getFederateName (Fed->getID ()));

    BOOST_CHECK_EQUAL (name, Fed->getName ());
    BOOST_CHECK (Fed->getCurrentMode () == helics::Federate::modes::startup);
    Fed->enterInitializingMode ();
    BOOST_CHECK (Fed->getCurrentMode () == helics::Federate::modes::initializing);
    Fed->enterExecutingMode ();
    BOOST_CHECK (Fed->getCurrentMode () == helics::Federate::modes::executing);
    Fed = nullptr;  // force the destructor
}

BOOST_AUTO_TEST_CASE (federate_time_step_tests, *utf::label ("ci"))
{
    helics::FederateInfo fi (CORE_TYPE_TO_TEST);
    fi.coreInitString = "--autobroker";

    auto Fed = std::make_shared<helics::Federate> ("test1", fi);

    BOOST_CHECK (Fed->getCurrentMode () == helics::Federate::modes::startup);
    Fed->enterInitializingMode ();
    BOOST_CHECK (Fed->getCurrentMode () == helics::Federate::modes::initializing);
    Fed->enterExecutingMode ();
    BOOST_CHECK (Fed->getCurrentMode () == helics::Federate::modes::executing);

    auto res = Fed->requestTime (1.0);
    BOOST_CHECK_EQUAL (res, 1.0);
    res = Fed->requestTime (2.0);
    BOOST_CHECK_EQUAL (res, 2.0);

    res = Fed->requestTime (3.0);
    BOOST_CHECK_EQUAL (res, 3.0);
}

BOOST_AUTO_TEST_CASE (federate_broker_disconnect_test)
{
    auto brk = helics::BrokerFactory::create (helics::core_type::TEST, "b1", "1");
    brk->connect ();
    helics::FederateInfo fi (CORE_TYPE_TO_TEST);

    fi.coreInitString = "--broker=b1 --tick=200 --timeout=1000";

    auto Fed = std::make_shared<helics::Federate> ("test1", fi);

    BOOST_CHECK (Fed->getCurrentMode () == helics::Federate::modes::startup);
    Fed->enterInitializingMode ();
    BOOST_CHECK (Fed->getCurrentMode () == helics::Federate::modes::initializing);
    Fed->enterExecutingMode ();
    BOOST_CHECK (Fed->getCurrentMode () == helics::Federate::modes::executing);

    auto res = Fed->requestTime (1.0);
    BOOST_CHECK_EQUAL (res, 1.0);
    res = Fed->requestTime (2.0);
    BOOST_CHECK_EQUAL (res, 2.0);

    res = Fed->requestTime (3.0);
    BOOST_CHECK_EQUAL (res, 3.0);
    brk->disconnect ();
    std::this_thread::sleep_for (std::chrono::seconds (1));
    auto cptr = Fed->getCorePointer ();
    BOOST_CHECK (!cptr->isConnected ());
    res = Fed->requestTime (4.0);
    BOOST_CHECK_EQUAL (res, helics::Time::maxVal ());
    BOOST_CHECK (Fed->getCurrentMode () == helics::Federate::modes::finalize);
}

// TODO PT:: make this work for all test types
BOOST_AUTO_TEST_CASE (federate_bad_broker_error_zmq)
{
    helics::FederateInfo fi (helics::core_type::ZMQ);
    fi.coreInitString = "--broker=b1 --tick=200 --timeout=800";

    BOOST_CHECK_THROW (std::make_shared<helics::Federate> ("test1", fi), helics::RegistrationFailure);
}

BOOST_AUTO_TEST_CASE (federate_timeout_error_zmq)
{
    helics::FederateInfo fi (helics::core_type::ZMQ);
    fi.coreInitString = "--tick=200 --timeout=800";

    BOOST_CHECK_THROW (std::make_shared<helics::Federate> ("test1", fi), helics::RegistrationFailure);
}

BOOST_AUTO_TEST_CASE (federate_multiple_federates, *utf::label ("ci"))
{
    helics::FederateInfo fi (CORE_TYPE_TO_TEST);
    fi.coreName = "core1-mult";
    fi.coreInitString = "-f 2 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate> ("fed1", fi);

    auto Fed2 = std::make_shared<helics::Federate> ("fed2", fi);

    BOOST_CHECK (Fed1->getCurrentMode () == helics::Federate::modes::startup);
    BOOST_CHECK (Fed2->getCurrentMode () == helics::Federate::modes::startup);

    BOOST_CHECK (Fed1->getID () != Fed2->getID ());

    auto f1finish = std::async (std::launch::async, [&]() { Fed1->enterInitializingMode (); });
    Fed2->enterInitializingMode ();

    f1finish.wait ();
    BOOST_CHECK (Fed1->getCurrentMode () == helics::Federate::modes::initializing);
    BOOST_CHECK (Fed2->getCurrentMode () == helics::Federate::modes::initializing);

    f1finish = std::async (std::launch::async, [&]() { Fed1->enterExecutingMode (); });
    Fed2->enterExecutingMode ();
    f1finish.wait ();
    BOOST_CHECK (Fed1->getCurrentMode () == helics::Federate::modes::executing);
    BOOST_CHECK (Fed2->getCurrentMode () == helics::Federate::modes::executing);

    auto f1step = std::async (std::launch::async, [&]() { return Fed1->requestTime (1.0); });
    auto f2step = Fed2->requestTime (1.0);

    auto f1stepVal = f1step.get ();
    BOOST_CHECK_EQUAL (f2step, 1.0);
    BOOST_CHECK_EQUAL (f1stepVal, 1.0);

    BOOST_CHECK_EQUAL (Fed1->getCurrentTime (), 1.0);

    f1step = std::async (std::launch::async, [&]() { return Fed1->requestTime (3.0); });
    f2step = Fed2->requestTime (3.0);

    f1stepVal = f1step.get ();
    BOOST_CHECK_EQUAL (f2step, 3.0);
    BOOST_CHECK_EQUAL (f1stepVal, 3.0);

    BOOST_CHECK_THROW (Fed1->enterInitializingMode (), helics::InvalidFunctionCall);
    Fed1->finalize ();
    Fed2->finalize ();
}

/** the same as the previous test except with multiple cores and a single broker*/
BOOST_AUTO_TEST_CASE (federate_multiple_federates_multi_cores, *utf::label ("ci"))
{
    helics::FederateInfo fi (CORE_TYPE_TO_TEST);
    fi.coreName = "core_mc1";
    fi.coreInitString = "--autobroker --broker=brk1 --brokerinit=2";

    auto Fed1 = std::make_shared<helics::Federate> ("fed1", fi);
    fi.coreName = "core_mc2";

    auto Fed2 = std::make_shared<helics::Federate> ("fed2", fi);

    BOOST_CHECK (Fed1->getCurrentMode () == helics::Federate::modes::startup);
    BOOST_CHECK (Fed2->getCurrentMode () == helics::Federate::modes::startup);

    auto f1finish = std::async (std::launch::async, [&]() { Fed1->enterInitializingMode (); });
    Fed2->enterInitializingMode ();

    f1finish.wait ();
    BOOST_CHECK (Fed1->getCurrentMode () == helics::Federate::modes::initializing);
    BOOST_CHECK (Fed2->getCurrentMode () == helics::Federate::modes::initializing);

    f1finish = std::async (std::launch::async, [&]() { Fed1->enterExecutingMode (); });
    Fed2->enterExecutingMode ();
    f1finish.wait ();
    BOOST_CHECK (Fed1->getCurrentMode () == helics::Federate::modes::executing);
    BOOST_CHECK (Fed2->getCurrentMode () == helics::Federate::modes::executing);

    auto f1step = std::async (std::launch::async, [&]() { return Fed1->requestTime (1.0); });
    auto f2step = Fed2->requestTime (1.0);

    auto f1stepVal = f1step.get ();
    BOOST_CHECK_EQUAL (f2step, 1.0);
    BOOST_CHECK_EQUAL (f1stepVal, 1.0);

    BOOST_CHECK_EQUAL (Fed1->getCurrentTime (), 1.0);

    f1step = std::async (std::launch::async, [&]() { return Fed1->requestTime (3.0); });
    f2step = Fed2->requestTime (3.0);

    f1stepVal = f1step.get ();
    BOOST_CHECK_EQUAL (f2step, 3.0);
    BOOST_CHECK_EQUAL (f1stepVal, 3.0);

    BOOST_CHECK_THROW (Fed1->enterInitializingMode (), helics::InvalidFunctionCall);
    Fed1->finalize ();
    Fed2->finalize ();
}

BOOST_AUTO_TEST_CASE (federate_multiple_federates_async_calls, *utf::label ("ci"))
{
    helics::FederateInfo fi (CORE_TYPE_TO_TEST);
    fi.coreName = "core_async";
    fi.coreInitString = "-f 2 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate> ("fed1", fi);

    auto Fed2 = std::make_shared<helics::Federate> ("fed2", fi);

    BOOST_CHECK (Fed1->getCurrentMode () == helics::Federate::modes::startup);
    BOOST_CHECK (Fed2->getCurrentMode () == helics::Federate::modes::startup);

    BOOST_CHECK_NE (Fed1->getID (), Fed2->getID ());

    Fed1->enterInitializingModeAsync ();
    Fed2->enterInitializingMode ();

    Fed1->enterInitializingModeComplete ();

    BOOST_CHECK (Fed1->getCurrentMode () == helics::Federate::modes::initializing);
    BOOST_CHECK (Fed2->getCurrentMode () == helics::Federate::modes::initializing);

    Fed1->enterExecutingModeAsync ();
    Fed2->enterExecutingMode ();
    Fed1->enterExecutingModeComplete ();
    BOOST_CHECK (Fed1->getCurrentMode () == helics::Federate::modes::executing);
    BOOST_CHECK (Fed2->getCurrentMode () == helics::Federate::modes::executing);

    Fed1->requestTimeAsync (1.0);
    auto f2step = Fed2->requestTime (1.0);

    auto f1stepVal = Fed1->requestTimeComplete ();
    BOOST_CHECK_EQUAL (f2step, 1.0);
    BOOST_CHECK_EQUAL (f1stepVal, 1.0);

    BOOST_CHECK_EQUAL (Fed1->getCurrentTime (), 1.0);

    Fed1->requestTimeAsync (3.0);
    f2step = Fed2->requestTime (3.0);

    f1stepVal = Fed1->requestTimeComplete ();
    BOOST_CHECK_EQUAL (f2step, 3.0);
    BOOST_CHECK_EQUAL (f1stepVal, 3.0);

    BOOST_CHECK_THROW (Fed1->enterInitializingMode (), helics::InvalidFunctionCall);
    BOOST_CHECK_THROW (Fed1->requestTimeComplete (), helics::InvalidFunctionCall);
    Fed1->finalize ();
    Fed2->finalize ();
}

static const std::vector<std::string> simple_global_files{"example_globals1.json", "example_globals1.toml",
                                                          "example_globals2.json"};
namespace bdata = boost::unit_test::data;

BOOST_DATA_TEST_CASE (federate_global_file, bdata::make (simple_global_files), file_name)
{
    auto brk = helics::BrokerFactory::create (helics::core_type::TEST, "b1", "2");
    brk->connect ();
    auto testFile = std::string (TEST_DIR) + file_name;
    brk->makeConnections (testFile);

    helics::FederateInfo fi (CORE_TYPE_TO_TEST);
    fi.coreName = "core_global1";
    fi.coreInitString = "-f 2";

    auto Fed1 = std::make_shared<helics::Federate> ("fed1", fi);

    auto Fed2 = std::make_shared<helics::Federate> ("fed2", fi);

    Fed1->enterInitializingModeAsync ();
    Fed2->enterInitializingMode ();

    Fed1->enterInitializingModeComplete ();

    auto str1 = Fed1->query ("global", "global1");
    BOOST_CHECK_EQUAL (str1, "this is a global1 value");
    str1 = Fed2->query ("global", "global1");
    BOOST_CHECK_EQUAL (str1, "this is a global1 value");

    str1 = Fed1->query ("global", "global2");
    BOOST_CHECK_EQUAL (str1, "this is another global value");
    str1 = Fed2->query ("global", "global2");
    BOOST_CHECK_EQUAL (str1, "this is another global value");
    Fed1->finalize ();
    Fed2->finalize ();
    brk->waitForDisconnect ();
}

BOOST_DATA_TEST_CASE (federate_core_global_file, bdata::make (simple_global_files), file_name)
{
    auto brk = helics::BrokerFactory::create (helics::core_type::TEST, "b1", "2");
    brk->connect ();

    helics::FederateInfo fi (CORE_TYPE_TO_TEST);
    fi.coreName = "core_global3";
    fi.coreInitString = "-f 1";

    auto Fed1 = std::make_shared<helics::Federate> ("fed1", fi);
    fi.coreName = "core_global4";
    auto Fed2 = std::make_shared<helics::Federate> ("fed2", fi);

    auto cr = Fed1->getCorePointer ();
    auto testFile = std::string (TEST_DIR) + file_name;
    cr->makeConnections (testFile);
    Fed1->enterInitializingModeAsync ();
    Fed2->enterInitializingMode ();

    Fed1->enterInitializingModeComplete ();

    auto str1 = Fed1->query ("global", "global1");
    BOOST_CHECK_EQUAL (str1, "this is a global1 value");
    str1 = Fed2->query ("global", "global1");
    BOOST_CHECK_EQUAL (str1, "this is a global1 value");
    str1 = cr->query ("global", "global1");
    BOOST_CHECK_EQUAL (str1, "this is a global1 value");
    str1 = brk->query ("global", "global1");
    BOOST_CHECK_EQUAL (str1, "this is a global1 value");

    str1 = Fed1->query ("global", "global2");
    BOOST_CHECK_EQUAL (str1, "this is another global value");
    str1 = Fed2->query ("global", "global2");
    BOOST_CHECK_EQUAL (str1, "this is another global value");
    str1 = cr->query ("global", "global2");
    BOOST_CHECK_EQUAL (str1, "this is another global value");
    str1 = brk->query ("global", "global2");
    BOOST_CHECK_EQUAL (str1, "this is another global value");

    auto str2 = Fed1->query ("global", "list");
    BOOST_CHECK ((str2 == "[global1;global2]") || (str2 == "[global2;global1]"));

    auto str3 = Fed1->query ("global", "all");
    BOOST_CHECK_NE (str3, "#invalid");
    Fed1->finalize ();
    Fed2->finalize ();
    cr = nullptr;
    brk->waitForDisconnect ();
}

BOOST_AUTO_TEST_SUITE_END ()
