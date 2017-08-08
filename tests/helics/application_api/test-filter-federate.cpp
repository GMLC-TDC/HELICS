/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>

#include "helics/application_api/MessageFilterFederate.h"
#include "test_configuration.h"
#include <future>
/** these test cases test out the message federates
*/

BOOST_AUTO_TEST_SUITE(message_filter_federate_tests)

/** test simple creation and destruction*/
BOOST_AUTO_TEST_CASE(message_filter_federate_initialize_tests)
{
	helics::FederateInfo fi("test1");
	fi.coreType = CORE_TYPE_TO_TEST;
	fi.coreInitString = "1";

	auto mFed = std::make_shared<helics::MessageFilterFederate>(fi);

	mFed->enterExecutionState();

	BOOST_CHECK(mFed->currentState() == helics::Federate::op_states::execution);

	mFed->finalize();

	BOOST_CHECK(mFed->currentState() == helics::Federate::op_states::finalize);
}

/** test registration of filters*/
BOOST_AUTO_TEST_CASE(message_filter_federate_registration)
{
	helics::FederateInfo fi("filter");
	fi.coreType = CORE_TYPE_TO_TEST;
	fi.coreInitString = "2";

	auto fFed = std::make_unique<helics::MessageFilterFederate>(fi);
	fi.name = "message";
	auto mFed = std::make_unique<helics::MessageFederate>(fi);

	mFed->registerGlobalEndpoint("port1");
	mFed->registerGlobalEndpoint("port2");

	auto f1 = fFed->registerSourceFilter("filter1", "port1");
	BOOST_CHECK(f1.value() != helics::invalid_id_value);
	auto f2 = fFed->registerDestinationFilter("filter2", "port2");
	BOOST_CHECK(f2 != f1);
	auto ep1 = fFed->registerEndpoint("fout");
	BOOST_CHECK(ep1.value() != helics::invalid_id_value);
	auto f3 = fFed->registerSourceFilter("filter/fout");
	BOOST_CHECK(f3 != f2);
	mFed->finalize();
	fFed->finalize();
	BOOST_CHECK(fFed->currentState() == helics::Federate::op_states::finalize);
}


/** test basic operation of filters*/
BOOST_AUTO_TEST_CASE(message_filter_basic_ops)
{
	helics::FederateInfo fi("filter");
	fi.coreType = CORE_TYPE_TO_TEST;
	fi.coreInitString = "2";
	fi.timeDelta = 1.0;
	auto fFed = std::make_unique<helics::MessageFilterFederate>(fi);
	
	fi.name = "message";
	auto mFed = std::make_unique<helics::MessageFederate>(fi);

	auto p1=mFed->registerGlobalEndpoint("port1");
	auto p2=mFed->registerGlobalEndpoint("port2");

	auto f1 = fFed->registerSourceFilter("filter1", "port1");
	BOOST_CHECK(f1.value() != helics::invalid_id_value);
	
	auto ep1 = fFed->registerEndpoint("fout");
	BOOST_CHECK(ep1.value() != helics::invalid_id_value);
	
	fFed->enterExecutionStateAsync();
	mFed->enterExecutionState();
	fFed->enterExecutionStateFinalize();

	BOOST_CHECK(fFed->currentState() == helics::Federate::op_states::execution);
	helics::data_block data(500, 'a');
	mFed->sendMessage(p1, "port2", data);

	mFed->requestTimeAsync(1.0);
	fFed->requestTime(1.0);
	mFed->requestTimeFinalize();
	auto res = fFed->hasMessageToFilter(f1);
	BOOST_CHECK(res);
	auto m = fFed->getMessageToFilter(f1);
	BOOST_REQUIRE_EQUAL(m->data.size(),data.size());
	fFed->sendMessage(ep1, std::move(m));

	mFed->requestTimeAsync(2.0);
	fFed->requestTime(2.0);
	mFed->requestTimeFinalize();
	BOOST_REQUIRE(mFed->hasMessage(p2));
	auto m2 = mFed->getMessage(p2);
	BOOST_CHECK_EQUAL(m2->src,"filter1.fout");
	BOOST_CHECK_EQUAL(m2->origsrc,"port1");
	BOOST_CHECK_EQUAL(m2->dest, "port2");
	BOOST_CHECK_EQUAL(m2->data.size(), data.size());
	mFed->finalize();
	fFed->finalize();
	BOOST_CHECK(fFed->currentState() == helics::Federate::op_states::finalize);
}
BOOST_AUTO_TEST_SUITE_END()