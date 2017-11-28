/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "helics/application_api/Federate.h"
#include "helics/application_api/MessageOperators.h"
#include "testFixtures.h"
#include "test_configuration.h"
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <future>
/** these test cases test out the message federates
 */

BOOST_FIXTURE_TEST_SUITE (message_filter_tests, FederateTestFixture)

namespace bdata = boost::unit_test::data;
namespace utf = boost::unit_test;
#ifdef QUICK_TESTS_ONLY
const std::string core_types[] = {"test", "test_2", "ipc_2", "zmq", "udp"};
#else
const std::string core_types[] = {"test", "test_2", "ipc", "ipc_2", "zmq", "zmq_2", "udp", "udp_2"};
//const std::string core_types[] = { "test_2"};
#endif

/** test registration of filters*/
#if ENABLE_TEST_TIMEOUTS>0 
 BOOST_TEST_DECORATOR (*utf::timeout(5))
 #endif
BOOST_DATA_TEST_CASE (message_filter_registration, bdata::make (core_types), core_type)
{
    auto broker = AddBroker (core_type, 2);
    AddFederates<helics::MessageFederate> (core_type, 2, broker, helics::timeZero, "filter");
    AddFederates<helics::MessageFederate> (core_type, 2, broker, helics::timeZero, "message");

    auto fFed = GetFederateAs<helics::MessageFederate> (0);
    auto mFed = GetFederateAs<helics::MessageFederate> (1);

    mFed->registerGlobalEndpoint ("port1");
    mFed->registerGlobalEndpoint ("port2");

    auto f1 = fFed->registerSourceFilter ("filter1", "port1");
    BOOST_CHECK (f1.value () != helics::invalid_id_value);
    auto f2 = fFed->registerDestinationFilter ("filter2", "port2");
    BOOST_CHECK (f2 != f1);
    auto ep1 = fFed->registerEndpoint ("fout");
    BOOST_CHECK (ep1.value () != helics::invalid_id_value);
    auto f3 = fFed->registerSourceFilter ("filter0/fout");
    BOOST_CHECK (f3 != f2);
    mFed->finalize ();
    fFed->finalize ();
    BOOST_CHECK (fFed->currentState () == helics::Federate::op_states::finalize);
}

/** test a filter operator
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the simulation
*/
#if ENABLE_TEST_TIMEOUTS>0 
 BOOST_TEST_DECORATOR (*utf::timeout(5))
 #endif
BOOST_DATA_TEST_CASE (message_filter_function, bdata::make (core_types), core_type)
{
    auto broker = AddBroker (core_type, 2);
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate> (0);
    auto mFed = GetFederateAs<helics::MessageFederate> (1);

    auto p1 = mFed->registerGlobalEndpoint ("port1");
    auto p2 = mFed->registerGlobalEndpoint ("port2");

    auto f1 = fFed->registerSourceFilter ("filter1", "port1");
    BOOST_CHECK (f1.value () != helics::invalid_id_value);
    auto timeOperator = std::make_shared<helics::MessageTimeOperator> ();
    timeOperator->setTimeFunction ([](helics::Time time_in) { return time_in + 2.5; });
    fFed->setFilterOperator (f1, timeOperator);

    fFed->enterExecutionStateAsync ();
    mFed->enterExecutionState ();
    fFed->enterExecutionStateFinalize (); 

    BOOST_CHECK (fFed->currentState () == helics::Federate::op_states::execution);
    helics::data_block data (500, 'a');
    mFed->sendMessage (p1, "port2", data);

    mFed->requestTimeAsync (1.0);
    fFed->requestTime (1.0);
    mFed->requestTimeFinalize ();

    auto res = mFed->hasMessage ();
    BOOST_CHECK (!res);

    mFed->requestTimeAsync (2.0);
    fFed->requestTime (2.0);
    mFed->requestTimeFinalize ();
    BOOST_REQUIRE (!mFed->hasMessage (p2));

    fFed->requestTimeAsync (3.0);
    /*auto retTime = */ mFed->requestTime (3.0);

    BOOST_REQUIRE (mFed->hasMessage (p2));

    auto m2 = mFed->getMessage (p2);
    BOOST_CHECK_EQUAL (m2->src, "port1");
    BOOST_CHECK_EQUAL (m2->origsrc, "port1");
    BOOST_CHECK_EQUAL (m2->dest, "port2");
    BOOST_CHECK_EQUAL (m2->data.size (), data.size ());
    BOOST_CHECK_EQUAL (m2->time, 2.5);

    mFed->requestTime (3.0);
    fFed->requestTimeFinalize ();
    mFed->finalize ();
    fFed->finalize ();
    BOOST_CHECK (fFed->currentState () == helics::Federate::op_states::finalize);
}

/** test two filter operators
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the simulation
*/
#if ENABLE_TEST_TIMEOUTS>0 
 BOOST_TEST_DECORATOR (*utf::timeout(5))
 #endif
BOOST_DATA_TEST_CASE(message_filter_function2, bdata::make(core_types), core_type)
{
    auto broker = AddBroker(core_type, 2);
    AddFederates<helics::MessageFederate>(core_type, 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate>(core_type, 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate>(0);
    auto mFed = GetFederateAs<helics::MessageFederate>(1);

    auto p1 = mFed->registerGlobalEndpoint("port1");
    auto p2 = mFed->registerGlobalEndpoint("port2");

    auto f1 = fFed->registerSourceFilter("filter1", "port1");
    auto f2 = fFed->registerSourceFilter("filter2", "port2");
    BOOST_CHECK(f1.value() != helics::invalid_id_value);
    auto timeOperator = std::make_shared<helics::MessageTimeOperator>();
    timeOperator->setTimeFunction([](helics::Time time_in) { return time_in + 2.5; });
    fFed->setFilterOperator({ f1,f2 }, timeOperator);

    fFed->enterExecutionStateAsync();
    mFed->enterExecutionState();
    fFed->enterExecutionStateFinalize();

    BOOST_CHECK(fFed->currentState() == helics::Federate::op_states::execution);
    helics::data_block data(500, 'a');
    mFed->sendMessage(p1, "port2", data);

    mFed->requestTimeAsync(1.0);
    fFed->requestTime(1.0);
    mFed->requestTimeFinalize();

    auto res = mFed->hasMessage();
    BOOST_CHECK(!res);
    mFed->sendMessage(p2, "port1", data);
    mFed->requestTimeAsync(2.0);
    fFed->requestTime(2.0);
    mFed->requestTimeFinalize();
    BOOST_REQUIRE(!mFed->hasMessage(p2));


    mFed->requestTime(3.0);

    BOOST_REQUIRE(mFed->hasMessage(p2));
    
    auto m2 = mFed->getMessage(p2);
    BOOST_CHECK_EQUAL(m2->src, "port1");
    BOOST_CHECK_EQUAL(m2->origsrc, "port1");
    BOOST_CHECK_EQUAL(m2->dest, "port2");
    BOOST_CHECK_EQUAL(m2->data.size(), data.size());
    BOOST_CHECK_EQUAL(m2->time, 2.5);
   
    BOOST_CHECK(!mFed->hasMessage(p1));
    mFed->requestTime(4.0);
    BOOST_CHECK(mFed->hasMessage(p1));
    mFed->finalize();
    fFed->finalize();
    BOOST_CHECK(fFed->currentState() == helics::Federate::op_states::finalize);
}

BOOST_AUTO_TEST_SUITE_END ()
