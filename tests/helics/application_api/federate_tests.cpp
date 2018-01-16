/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "helics/application_api/Federate.hpp"
#include "helics/core/BrokerFactory.h"
#include "helics/core/CoreFactory.h"
#include "helics/core/core-exceptions.h"
#include "helics/core/core.h"
#include "test_configuration.h"
#include <future>
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
/** these test cases test out the value converters
 */

BOOST_AUTO_TEST_SUITE (federate_tests)

BOOST_AUTO_TEST_CASE (federate_initialize_tests)
{
    helics::FederateInfo fi ("test1");
    fi.coreType = CORE_TYPE_TO_TEST;
    fi.coreInitString = "1";

    auto Fed = std::make_shared<helics::Federate> (fi);

    auto core = Fed->getCorePointer ();
    BOOST_REQUIRE ((core));

    auto name = std::string (core->getFederateName (Fed->getID ()));

    BOOST_CHECK_EQUAL (name, Fed->getName ());
    BOOST_CHECK (Fed->getCurrentState () == helics::Federate::op_states::startup);
    Fed->enterInitializationState ();
    BOOST_CHECK (Fed->getCurrentState () == helics::Federate::op_states::initialization);
    Fed->enterExecutionState ();
    BOOST_CHECK (Fed->getCurrentState () == helics::Federate::op_states::execution);
    Fed = nullptr;  // force the destructor
}

BOOST_AUTO_TEST_CASE (federate_time_step_tests)
{
    helics::FederateInfo fi ("test1");
    fi.coreType = CORE_TYPE_TO_TEST;
    fi.coreInitString = "1";

    auto Fed = std::make_shared<helics::Federate> (fi);

    BOOST_CHECK (Fed->getCurrentState () == helics::Federate::op_states::startup);
    Fed->enterInitializationState ();
    BOOST_CHECK (Fed->getCurrentState () == helics::Federate::op_states::initialization);
    Fed->enterExecutionState ();
    BOOST_CHECK (Fed->getCurrentState () == helics::Federate::op_states::execution);

    auto res = Fed->requestTime (1.0);
    BOOST_CHECK_EQUAL (res, 1.0);
    res = Fed->requestTime (2.0);
    BOOST_CHECK_EQUAL (res, 2.0);

    res = Fed->requestTime (3.0);
    BOOST_CHECK_EQUAL (res, 3.0);
}

#ifndef QUICK_TESTS_ONLY
BOOST_AUTO_TEST_CASE (federate_timeout_test)
{
    auto brk = helics::BrokerFactory::create (helics::core_type::TEST, "b1", "1");
    brk->connect ();
    helics::FederateInfo fi ("test1");
    fi.coreType = CORE_TYPE_TO_TEST;
    fi.coreInitString = "1 --broker=b1 --tick=1000 --timeout=3000";

    auto Fed = std::make_shared<helics::Federate> (fi);

    BOOST_CHECK (Fed->getCurrentState () == helics::Federate::op_states::startup);
    Fed->enterInitializationState ();
    BOOST_CHECK (Fed->getCurrentState () == helics::Federate::op_states::initialization);
    Fed->enterExecutionState ();
    BOOST_CHECK (Fed->getCurrentState () == helics::Federate::op_states::execution);

    auto res = Fed->requestTime (1.0);
    BOOST_CHECK_EQUAL (res, 1.0);
    res = Fed->requestTime (2.0);
    BOOST_CHECK_EQUAL (res, 2.0);

    res = Fed->requestTime (3.0);
    BOOST_CHECK_EQUAL (res, 3.0);
    brk->disconnect ();
    std::this_thread::sleep_for (std::chrono::seconds (5));
    auto cptr = Fed->getCorePointer ();
    BOOST_CHECK (!cptr->isConnected ());
    BOOST_CHECK_THROW (res = Fed->requestTime (4.0), helics::FunctionExecutionFailure);
    BOOST_CHECK (Fed->getCurrentState () == helics::Federate::op_states::error);
}
#endif

BOOST_AUTO_TEST_CASE (federate_multiple_federates)
{
    helics::FederateInfo fi ("fed1");
    fi.coreType = CORE_TYPE_TO_TEST;
    fi.coreName = "core1";
    fi.coreInitString = "2";

    auto Fed1 = std::make_shared<helics::Federate> (fi);

    fi.name = "fed2";
    auto Fed2 = std::make_shared<helics::Federate> (fi);

    BOOST_CHECK (Fed1->getCurrentState () == helics::Federate::op_states::startup);
    BOOST_CHECK (Fed2->getCurrentState () == helics::Federate::op_states::startup);

    BOOST_CHECK_NE (Fed1->getID (), Fed2->getID ());

    auto f1finish = std::async (std::launch::async, [&]() { Fed1->enterInitializationState (); });
    Fed2->enterInitializationState ();

    f1finish.wait ();
    BOOST_CHECK (Fed1->getCurrentState () == helics::Federate::op_states::initialization);
    BOOST_CHECK (Fed2->getCurrentState () == helics::Federate::op_states::initialization);

    f1finish = std::async (std::launch::async, [&]() { Fed1->enterExecutionState (); });
    Fed2->enterExecutionState ();
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

    BOOST_CHECK_THROW (Fed1->enterInitializationState (), helics::InvalidFunctionCall);
    Fed1->finalize ();
    Fed2->finalize ();
}

/** the same as the previous test except with multiple cores and a single broker*/
BOOST_AUTO_TEST_CASE (federate_multiple_federates_multi_cores)
{
    helics::FederateInfo fi ("fed1");
    fi.coreType = CORE_TYPE_TO_TEST;
    fi.coreName = "core_mc1";
    fi.coreInitString = "1 --broker=brk1 --brokerinit=2";

    auto Fed1 = std::make_shared<helics::Federate> (fi);

    fi.name = "fed2";
    fi.coreName = "core_mc2";

    auto Fed2 = std::make_shared<helics::Federate> (fi);

    BOOST_CHECK (Fed1->getCurrentState () == helics::Federate::op_states::startup);
    BOOST_CHECK (Fed2->getCurrentState () == helics::Federate::op_states::startup);

    auto f1finish = std::async (std::launch::async, [&]() { Fed1->enterInitializationState (); });
    Fed2->enterInitializationState ();

    f1finish.wait ();
    BOOST_CHECK (Fed1->getCurrentState () == helics::Federate::op_states::initialization);
    BOOST_CHECK (Fed2->getCurrentState () == helics::Federate::op_states::initialization);

    f1finish = std::async (std::launch::async, [&]() { Fed1->enterExecutionState (); });
    Fed2->enterExecutionState ();
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

    BOOST_CHECK_THROW (Fed1->enterInitializationState (), helics::InvalidFunctionCall);
    Fed1->finalize ();
    Fed2->finalize ();
}

BOOST_AUTO_TEST_CASE (federate_multiple_federates_async_calls)
{
    helics::FederateInfo fi ("fed1");
    fi.coreType = CORE_TYPE_TO_TEST;
    fi.coreName = "core_async";
    fi.coreInitString = "2";

    auto Fed1 = std::make_shared<helics::Federate> (fi);

    fi.name = "fed2";
    auto Fed2 = std::make_shared<helics::Federate> (fi);

    BOOST_CHECK (Fed1->getCurrentState () == helics::Federate::op_states::startup);
    BOOST_CHECK (Fed2->getCurrentState () == helics::Federate::op_states::startup);

    BOOST_CHECK_NE (Fed1->getID (), Fed2->getID ());

    Fed1->enterInitializationStateAsync ();
    Fed2->enterInitializationState ();

    Fed1->enterInitializationStateComplete ();

    BOOST_CHECK (Fed1->getCurrentState () == helics::Federate::op_states::initialization);
    BOOST_CHECK (Fed2->getCurrentState () == helics::Federate::op_states::initialization);

    Fed1->enterExecutionStateAsync ();
    Fed2->enterExecutionState ();
    Fed1->enterExecutionStateComplete ();
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

    BOOST_CHECK_THROW (Fed1->enterInitializationState (), helics::InvalidFunctionCall);
    BOOST_CHECK_THROW (Fed1->requestTimeComplete (), helics::InvalidFunctionCall);
    Fed1->finalize ();
    Fed2->finalize ();
}
BOOST_AUTO_TEST_SUITE_END ()