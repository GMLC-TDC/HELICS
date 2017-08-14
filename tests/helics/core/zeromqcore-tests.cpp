/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include <boost/test/unit_test.hpp>

#include "helics/core/CoreFactory.h"
#include "helics/core/core.h"
#include "helics/core/core-types.h"

BOOST_AUTO_TEST_SUITE(zeromqcore_tests)

using helics::Core;
using helics::CoreFactory;

BOOST_AUTO_TEST_CASE(zeromqcore_initialization_test)
{
	std::string initializationString = "4";
	auto  core = CoreFactory::create(HELICS_ZMQ, initializationString);

	BOOST_REQUIRE(core != nullptr);
	BOOST_CHECK(core->isInitialized());

	BOOST_CHECK_EQUAL(core->getFederationSize(), 4);

}

BOOST_AUTO_TEST_CASE(zeromqcore_pubsub_value_test)
{
	std::string initializationString = "1";
	auto core = CoreFactory::create(HELICS_ZMQ, initializationString);

	BOOST_REQUIRE(core != nullptr);
	BOOST_CHECK(core->isInitialized());

	BOOST_CHECK_EQUAL(core->getFederationSize(), 1);

	Core::federate_id_t id = core->registerFederate("sim1", helics::CoreFederateInfo());
	
	BOOST_CHECK_EQUAL(core->getFederateName(id), "sim1");
	BOOST_CHECK_EQUAL(core->getFederateId("sim1"), id);

	core->setTimeDelta(id, 1.0);
	
	Core::Handle sub1 = core->registerSubscription(id, "sim1_pub", "type", "units", false);
	BOOST_CHECK_EQUAL(core->getSubscription(id, "sim1_pub"), sub1);
	BOOST_CHECK_EQUAL(core->getType(sub1), "type");
	BOOST_CHECK_EQUAL(core->getUnits(sub1), "units");

	Core::Handle pub1 = core->registerPublication(id, "sim1_pub", "type", "units");
	BOOST_CHECK_EQUAL(core->getPublication(id, "sim1_pub"), pub1);
	BOOST_CHECK_EQUAL(core->getType(pub1), "type");
	BOOST_CHECK_EQUAL(core->getUnits(pub1), "units");

	core->enterInitializingState(id);

	core->enterExecutingState(id);
	
	core->timeRequest(id, 50.0);
	std::string str1 = "hello world";
	core->setValue(pub1, str1.data(), str1.size());
	auto valueUpdates = core->getValueUpdates(id);
	BOOST_CHECK(valueUpdates.empty());
	auto data = core->getValue(sub1);
	BOOST_CHECK(data == nullptr);
	BOOST_CHECK_EQUAL(data->size(), 0u);
	
	core->timeRequest(id, 100.0);
	valueUpdates = core->getValueUpdates(id);
	BOOST_CHECK_EQUAL(valueUpdates[0], sub1);
	BOOST_CHECK_EQUAL(valueUpdates.size(), 1u);
	data = core->getValue(sub1);
	std::string str2(data->to_string());
	BOOST_CHECK_EQUAL(str1, str2);
	BOOST_CHECK_EQUAL(data->to_string(), "hello world");
	BOOST_CHECK_EQUAL(data->size(), str1.size());

	core->setValue(pub1, "hello\n\0helloAgain", 17);
	core->timeRequest(id, 150.0);
	valueUpdates = core->getValueUpdates(id);
	BOOST_CHECK_EQUAL(valueUpdates[0], sub1);
	BOOST_CHECK_EQUAL(valueUpdates.size(), 1u);
	data = core->getValue(sub1);
	BOOST_CHECK_EQUAL(data->to_string(), "hello\n\0helloAgain");
	BOOST_CHECK_EQUAL(data->size(), 17u);

	core->timeRequest(id, 200.0);
	valueUpdates = core->getValueUpdates(id);
	BOOST_CHECK(valueUpdates.empty());
	
}

BOOST_AUTO_TEST_CASE(zeromqcore_send_receive_test)
{
	std::string initializationString = "1";
	auto core = CoreFactory::create(HELICS_ZMQ, initializationString);

	BOOST_REQUIRE(core != nullptr);
	BOOST_CHECK(core->isInitialized());

	BOOST_CHECK_EQUAL(core->getFederationSize(), 1);

	Core::federate_id_t id = core->registerFederate("sim1", helics::CoreFederateInfo());

	BOOST_CHECK_EQUAL(core->getFederateName(id), "sim1");
	BOOST_CHECK_EQUAL(core->getFederateId("sim1"), id);

	core->setTimeDelta(id, 1.0);
	
	Core::Handle end1 = core->registerEndpoint(id, "end1", "type");
	BOOST_CHECK_EQUAL(core->getType(end1), "type");

	Core::Handle end2 = core->registerEndpoint(id, "end2", "type");
	BOOST_CHECK_EQUAL(core->getType(end2), "type");

	core->enterInitializingState(id);

	core->enterExecutingState(id);

	std::string str1 = "hello world";
	core->timeRequest(id, 50.0);
	core->send(end1, "end2", str1.data(), str1.size());

	core->timeRequest(id, 100.0);
	BOOST_CHECK_EQUAL(core->receiveCount(end1), 0);
	BOOST_CHECK_EQUAL(core->receiveCount(end2), 1u);
	auto msg = core->receive(end1);
	BOOST_CHECK(msg == nullptr);
	msg = core->receive(end2);
	BOOST_CHECK_EQUAL(core->receiveCount(end2), 0);
	std::string str2(msg->data.to_string());
	BOOST_CHECK_EQUAL(str1, str2);
	BOOST_CHECK_EQUAL(msg->data.size(), str1.size());
	
}

BOOST_AUTO_TEST_CASE(zeromqcore_messagefilter_test)
{
	// Create filter operator
	class TestOperator : public helics::FilterOperator {
	public:
		TestOperator(const std::string &name):filterName(name) {
		}

		std::unique_ptr<helics::Message> process (std::unique_ptr<helics::Message> msg) override {
			
			msg->src = filterName;
			return msg;
		}

	std::string filterName = 0;
	};

	std::string initializationString = "1";
	auto core = CoreFactory::create(HELICS_ZMQ, initializationString);

	BOOST_REQUIRE(core != nullptr);
	BOOST_CHECK(core->isInitialized());

	Core::federate_id_t id = core->registerFederate("sim1", helics::CoreFederateInfo());

	Core::Handle end1 = core->registerEndpoint(id, "end1", "type");
	Core::Handle end2 = core->registerEndpoint(id, "end2", "type");
	Core::Handle end3 = core->registerEndpoint(id, "end3", "type");

	Core::Handle srcFilter = core->registerSourceFilter(id, "srcFilter", "end1", "type");
	Core::Handle dstFilter = core->registerDestinationFilter(id, "dstFilter", "end3", "type");

	auto testSrcFilter = std::make_shared<TestOperator>("sourceFilter");
	BOOST_CHECK_EQUAL(testSrcFilter->filterName, "sourceFilter");

	auto testDstFilter = std::make_shared<TestOperator>("destinationFilter");
	BOOST_CHECK_EQUAL(testDstFilter->filterName, "destinationFilter");

	core->setFilterOperator(srcFilter, testSrcFilter);
	core->setFilterOperator(dstFilter, testDstFilter);

	core->enterInitializingState(id);
	core->enterExecutingState(id);

	std::string msgData = "hello world";
	core->send(end1, "end2", msgData.data(), msgData.size());

	core->timeRequest(id, 50.0);
	BOOST_CHECK_EQUAL(core->receiveCount(end2), 1u);
	auto msg = core->receive(end2);
	BOOST_CHECK_EQUAL(msg->origsrc, "end1");
	BOOST_CHECK_EQUAL(msg->src, testSrcFilter->filterName);

	core->send(end1, "end3", msgData.data(), msgData.size());

	core->timeRequest(id, 100.0);
	BOOST_CHECK_EQUAL(core->receiveCount(end3), 1u);
	msg = core->receive(end3);
	BOOST_CHECK_EQUAL(msg->origsrc, "end1");
	BOOST_CHECK_EQUAL(msg->src, testDstFilter->filterName);

	core->send(end3, "end1", msgData.data(), msgData.size());

	core->timeRequest(id, 150.0);
	BOOST_CHECK_EQUAL(core->receiveCount(end1), 1u);
	msg = core->receive(end1);
	BOOST_CHECK_EQUAL(msg->origsrc, "end3");
	BOOST_CHECK_EQUAL(msg->src, "end3");

}

BOOST_AUTO_TEST_SUITE_END()
