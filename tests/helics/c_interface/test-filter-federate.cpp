/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include "ctestFixtures.h"
#include "test_configuration.h"

#include <future>
/** these test cases test out the message federates
 */

//BOOST_FIXTURE_TEST_SUITE (message_filter_federate_tests, FederateTestFixture)

namespace bdata = boost::unit_test::data;
const std::string core_types[] = {"test", "test_2", "ipc", "ipc_2", "zmq", "zmq_2"};

/** test simple creation and destruction*/
// BOOST_DATA_TEST_CASE (message_filter_federate_initialize_tests, bdata::make (core_types), core_type)
//{
//    auto broker = AddBroker (core_type, 1);
//    AddFederates<helics::MessageFilterFederate> (core_type, 1, broker);
//
//    auto fFed = GetFederateAs<helics::MessageFilterFederate> (0);
//
//    fFed->enterExecutionState ();
//
//    BOOST_CHECK (fFed->currentState () == helics::Federate::op_states::execution);
//
//    fFed->finalize ();
//
//    BOOST_CHECK (fFed->currentState () == helics::Federate::op_states::finalize);
//}
//
///** test registration of filters*/
// BOOST_DATA_TEST_CASE (message_filter_federate_registration, bdata::make (core_types), core_type)
//{
//    auto broker = AddBroker (core_type, 2);
//    AddFederates<helics::MessageFilterFederate> (core_type, 2, broker, helics::timeZero, "filter");
//    AddFederates<helics::MessageFederate> (core_type, 2, broker, helics::timeZero, "message");
//
//    auto fFed = GetFederateAs<helics::MessageFilterFederate> (0);
//    auto mFed = GetFederateAs<helics::MessageFederate> (1);
//
//    mFed->registerGlobalEndpoint ("port1");
//    mFed->registerGlobalEndpoint ("port2");
//
//    auto f1 = fFed->registerSourceFilter ("filter1", "port1");
//    BOOST_CHECK (f1.value () != helics::invalid_id_value);
//    auto f2 = fFed->registerDestinationFilter ("filter2", "port2");
//    BOOST_CHECK (f2 != f1);
//    auto ep1 = fFed->registerEndpoint ("fout");
//    BOOST_CHECK (ep1.value () != helics::invalid_id_value);
//    auto f3 = fFed->registerSourceFilter ("filter0/fout");
//    BOOST_CHECK (f3 != f2);
//    mFed->finalize ();
//    fFed->finalize ();
//    BOOST_CHECK (fFed->currentState () == helics::Federate::op_states::finalize);
//}
//
///** test basic operation of filters*/
// BOOST_DATA_TEST_CASE (message_filter_basic_ops, bdata::make (core_types), core_type)
//{
//    auto broker = AddBroker (core_type, 2);
//    AddFederates<helics::MessageFilterFederate> (core_type, 1, broker, 1.0, "filter");
//    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "message");
//
//    auto fFed = GetFederateAs<helics::MessageFilterFederate> (0);
//    auto mFed = GetFederateAs<helics::MessageFederate> (1);
//
//    auto p1 = mFed->registerGlobalEndpoint ("port1");
//    auto p2 = mFed->registerGlobalEndpoint ("port2");
//
//    auto f1 = fFed->registerSourceFilter ("filter1", "port1");
//    BOOST_CHECK (f1.value () != helics::invalid_id_value);
//
//    auto ep1 = fFed->registerEndpoint ("fout");
//    BOOST_CHECK (ep1.value () != helics::invalid_id_value);
//
//    fFed->enterExecutionStateAsync ();
//    mFed->enterExecutionState ();
//    fFed->enterExecutionStateFinalize ();
//
//    BOOST_CHECK (fFed->currentState () == helics::Federate::op_states::execution);
//    helics::data_block data (500, 'a');
//    mFed->sendMessage (p1, "port2", data);
//
//    mFed->requestTimeAsync (1.0);
//    fFed->requestTime (1.0);
//    mFed->requestTimeFinalize ();
//    auto res = fFed->hasMessageToFilter (f1);
//    BOOST_CHECK (res);
//    auto m = fFed->getMessageToFilter (f1);
//    BOOST_REQUIRE (m);
//    BOOST_REQUIRE_EQUAL (m->data.size (), data.size ());
//    fFed->sendMessage (ep1, std::move (m));
//
//    mFed->requestTimeAsync (2.0);
//    fFed->requestTime (2.0);
//    mFed->requestTimeFinalize ();
//    BOOST_REQUIRE (mFed->hasMessage (p2));
//    auto m2 = mFed->getMessage (p2);
//    BOOST_CHECK_EQUAL (m2->src, "filter0/fout");
//    BOOST_CHECK_EQUAL (m2->origsrc, "port1");
//    BOOST_CHECK_EQUAL (m2->dest, "port2");
//    BOOST_CHECK_EQUAL (m2->data.size (), data.size ());
//    mFed->finalize ();
//    fFed->finalize ();
//    BOOST_CHECK (fFed->currentState () == helics::Federate::op_states::finalize);
//}
//
///** test a filter operator
// The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the simulation
//*/
// BOOST_DATA_TEST_CASE (message_filter_function, bdata::make (core_types), core_type)
//{
//    auto broker = AddBroker (core_type, 2);
//    AddFederates<helics::MessageFilterFederate> (core_type, 1, broker, 1.0, "filter");
//    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "message");
//
//    auto fFed = GetFederateAs<helics::MessageFilterFederate> (0);
//    auto mFed = GetFederateAs<helics::MessageFederate> (1);
//
//    auto p1 = mFed->registerGlobalEndpoint ("port1");
//    auto p2 = mFed->registerGlobalEndpoint ("port2");
//
//    auto f1 = fFed->registerSourceFilter ("filter1", "port1");
//    BOOST_CHECK (f1.value () != helics::invalid_id_value);
//    auto timeOperator = std::make_shared<helics::MessageTimeOperator> ();
//    timeOperator->setTimeFunction ([](helics::Time time_in) { return time_in + 2.5; });
//    fFed->registerMessageOperator (f1, timeOperator);
//
//    fFed->enterExecutionStateAsync ();
//    mFed->enterExecutionState ();
//    fFed->enterExecutionStateFinalize ();
//
//    BOOST_CHECK (fFed->currentState () == helics::Federate::op_states::execution);
//    helics::data_block data (500, 'a');
//    mFed->sendMessage (p1, "port2", data);
//
//    mFed->requestTimeAsync (1.0);
//    fFed->requestTime (1.0);
//    mFed->requestTimeFinalize ();
//    auto res = fFed->hasMessageToFilter (f1);
//    BOOST_CHECK (!res);
//    res = mFed->hasMessage ();
//    BOOST_CHECK (!res);
//
//    mFed->requestTimeAsync (2.0);
//    fFed->requestTime (2.0);
//    mFed->requestTimeFinalize ();
//    BOOST_REQUIRE (!mFed->hasMessage (p2));
//
//
//    fFed->requestTimeAsync (3.0);
//	auto retTime=mFed->requestTime(3.0);
//
//    BOOST_REQUIRE (mFed->hasMessage (p2));
//
//    auto m2 = mFed->getMessage (p2);
//    BOOST_CHECK_EQUAL (m2->src, "port1");
//    BOOST_CHECK_EQUAL (m2->origsrc, "port1");
//    BOOST_CHECK_EQUAL (m2->dest, "port2");
//    BOOST_CHECK_EQUAL (m2->data.size (), data.size ());
//    BOOST_CHECK_EQUAL (m2->time, 2.5);
//	//There is a bug here but It may get fixed by some API changes so i don't want to spend the time debugging
// right now 	mFed->requestTime(3.0); 	fFed->requestTimeFinalize();
//    mFed->finalize ();
//    fFed->finalize ();
//    BOOST_CHECK (fFed->currentState () == helics::Federate::op_states::finalize);
//}

//BOOST_AUTO_TEST_SUITE_END ()