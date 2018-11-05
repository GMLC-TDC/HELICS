/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "helics/application_api/Federate.hpp"
#include "helics/core/BrokerFactory.hpp"
//#include "helics/core/CoreFactory.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/core-exceptions.hpp"
#include <future>
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
/** these test cases test out the value converters
 */

#define CORE_TYPE_TO_TEST helics::core_type::TEST

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE (federate_tests)

BOOST_AUTO_TEST_CASE (federate_initialize_tests, *utf::label("ci"))
{
    helics::FederateInfo fi (CORE_TYPE_TO_TEST);
    fi.coreInitString = "1 --autobroker";

    auto Fed = std::make_shared<helics::Federate> ("test1",fi);

    auto core = Fed->getCorePointer ();
    BOOST_REQUIRE ((core));

    auto name = std::string (core->getFederateName (Fed->getID ()));

    BOOST_CHECK_EQUAL (name, Fed->getName ());
    BOOST_CHECK (Fed->getCurrentState () == helics::Federate::op_states::startup);
    Fed->enterInitializingMode ();
    BOOST_CHECK (Fed->getCurrentState () == helics::Federate::op_states::initialization);
    Fed->enterExecutingMode ();
    BOOST_CHECK (Fed->getCurrentState () == helics::Federate::op_states::execution);
    Fed = nullptr;  // force the destructor
}

BOOST_AUTO_TEST_CASE (federate_time_step_tests, *utf::label("ci"))
{
    helics::FederateInfo fi (CORE_TYPE_TO_TEST);
    fi.coreInitString = "1 --autobroker";

    auto Fed = std::make_shared<helics::Federate> ("test1",fi);

    BOOST_CHECK (Fed->getCurrentState () == helics::Federate::op_states::startup);
    Fed->enterInitializingMode ();
    BOOST_CHECK (Fed->getCurrentState () == helics::Federate::op_states::initialization);
    Fed->enterExecutingMode ();
    BOOST_CHECK (Fed->getCurrentState () == helics::Federate::op_states::execution);

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

    fi.coreInitString = "1 --broker=b1 --tick=1000 --timeout=3000";

    auto Fed = std::make_shared<helics::Federate> ("test1",fi);

    BOOST_CHECK (Fed->getCurrentState () == helics::Federate::op_states::startup);
    Fed->enterInitializingMode ();
    BOOST_CHECK (Fed->getCurrentState () == helics::Federate::op_states::initialization);
    Fed->enterExecutingMode ();
    BOOST_CHECK (Fed->getCurrentState () == helics::Federate::op_states::execution);

    auto res = Fed->requestTime (1.0);
    BOOST_CHECK_EQUAL (res, 1.0);
    res = Fed->requestTime (2.0);
    BOOST_CHECK_EQUAL (res, 2.0);

    res = Fed->requestTime (3.0);
    BOOST_CHECK_EQUAL (res, 3.0);
    brk->disconnect ();
    std::this_thread::sleep_for (std::chrono::seconds (2));
    auto cptr = Fed->getCorePointer ();
    BOOST_CHECK (!cptr->isConnected ());
    res = Fed->requestTime(4.0);
    BOOST_CHECK_EQUAL(res, helics::Time::maxVal());
    BOOST_CHECK (Fed->getCurrentState () == helics::Federate::op_states::finalize);
}

//TODO PT:: make this work for all test types
BOOST_AUTO_TEST_CASE (federate_bad_broker_error_zmq)
{
    helics::FederateInfo fi (helics::core_type::ZMQ);
    fi.coreInitString = "1 --broker=b1 --tick=100 --timeout=2000";

    BOOST_CHECK_THROW (std::make_shared<helics::Federate> ("test1",fi), helics::RegistrationFailure);
}

BOOST_AUTO_TEST_CASE (federate_timeout_error_zmq)
{
    helics::FederateInfo fi (helics::core_type::ZMQ);
    fi.coreInitString = "1 --tick=100 --timeout=2000";

    BOOST_CHECK_THROW (std::make_shared<helics::Federate> ("test1",fi), helics::RegistrationFailure);
}

BOOST_AUTO_TEST_CASE (federate_multiple_federates, *utf::label("ci"))
{
    helics::FederateInfo fi (CORE_TYPE_TO_TEST);
    fi.coreName = "core1";
    fi.coreInitString = "2 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate> ("fed1",fi);

    auto Fed2 = std::make_shared<helics::Federate> ("fed2",fi);

    BOOST_CHECK (Fed1->getCurrentState () == helics::Federate::op_states::startup);
    BOOST_CHECK (Fed2->getCurrentState () == helics::Federate::op_states::startup);

    BOOST_CHECK (Fed1->getID ()!=Fed2->getID ());

    auto f1finish = std::async (std::launch::async, [&]() { Fed1->enterInitializingMode (); });
    Fed2->enterInitializingMode ();

    f1finish.wait ();
    BOOST_CHECK (Fed1->getCurrentState () == helics::Federate::op_states::initialization);
    BOOST_CHECK (Fed2->getCurrentState () == helics::Federate::op_states::initialization);

    f1finish = std::async (std::launch::async, [&]() { Fed1->enterExecutingMode (); });
    Fed2->enterExecutingMode ();
    f1finish.wait ();
    BOOST_CHECK (Fed1->getCurrentState () == helics::Federate::op_states::execution);
    BOOST_CHECK (Fed2->getCurrentState () == helics::Federate::op_states::execution);

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
BOOST_AUTO_TEST_CASE (federate_multiple_federates_multi_cores, *utf::label("ci"))
{
    helics::FederateInfo fi (CORE_TYPE_TO_TEST);
    fi.coreName = "core_mc1";
    fi.coreInitString = "1 --autobroker --broker=brk1 --brokerinit=2";

    auto Fed1 = std::make_shared<helics::Federate> ("fed1",fi);
    fi.coreName = "core_mc2";

    auto Fed2 = std::make_shared<helics::Federate> ("fed2",fi);

    BOOST_CHECK (Fed1->getCurrentState () == helics::Federate::op_states::startup);
    BOOST_CHECK (Fed2->getCurrentState () == helics::Federate::op_states::startup);

    auto f1finish = std::async (std::launch::async, [&]() { Fed1->enterInitializingMode (); });
    Fed2->enterInitializingMode ();

    f1finish.wait ();
    BOOST_CHECK (Fed1->getCurrentState () == helics::Federate::op_states::initialization);
    BOOST_CHECK (Fed2->getCurrentState () == helics::Federate::op_states::initialization);

    f1finish = std::async (std::launch::async, [&]() { Fed1->enterExecutingMode (); });
    Fed2->enterExecutingMode ();
    f1finish.wait ();
    BOOST_CHECK (Fed1->getCurrentState () == helics::Federate::op_states::execution);
    BOOST_CHECK (Fed2->getCurrentState () == helics::Federate::op_states::execution);

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

BOOST_AUTO_TEST_CASE (federate_multiple_federates_async_calls, *utf::label("ci"))
{
    helics::FederateInfo fi (CORE_TYPE_TO_TEST);
    fi.coreName = "core_async";
    fi.coreInitString = "2 --autobroker";

    auto Fed1 = std::make_shared<helics::Federate> ("fed1",fi);

    auto Fed2 = std::make_shared<helics::Federate> ("fed2",fi);

    BOOST_CHECK (Fed1->getCurrentState () == helics::Federate::op_states::startup);
    BOOST_CHECK (Fed2->getCurrentState () == helics::Federate::op_states::startup);

    BOOST_CHECK_NE (Fed1->getID (), Fed2->getID ());

    Fed1->enterInitializingModeAsync ();
    Fed2->enterInitializingMode ();

    Fed1->enterInitializingModeComplete ();

    BOOST_CHECK (Fed1->getCurrentState () == helics::Federate::op_states::initialization);
    BOOST_CHECK (Fed2->getCurrentState () == helics::Federate::op_states::initialization);

    Fed1->enterExecutingModeAsync ();
    Fed2->enterExecutingMode ();
    Fed1->enterExecutingModeComplete ();
    BOOST_CHECK (Fed1->getCurrentState () == helics::Federate::op_states::execution);
    BOOST_CHECK (Fed2->getCurrentState () == helics::Federate::op_states::execution);

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
BOOST_AUTO_TEST_SUITE_END ()
