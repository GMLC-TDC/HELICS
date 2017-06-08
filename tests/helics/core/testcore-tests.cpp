/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include <boost/test/unit_test.hpp>

#include "helics/core/core-factory.h"
#include "helics/core/core.h"
#include "helics/core/core-types.h"

BOOST_AUTO_TEST_SUITE(testcore_tests)

using helics::Core;
using helics::CoreFactory;

BOOST_AUTO_TEST_CASE(testcore_initialization_test)
{
	const char *initializationString = "4";
	Core* core = CoreFactory::create(HELICS_TEST, initializationString);

	BOOST_REQUIRE(core != nullptr);
	BOOST_CHECK(core->isInitialized());

	BOOST_CHECK_EQUAL(core->getFederationSize(), 4u);

	delete core;
}

BOOST_AUTO_TEST_CASE(testcore_pubsub_value_test)
{
	const char *initializationString = "1";
	Core* core = CoreFactory::create(HELICS_TEST, initializationString);

	BOOST_REQUIRE(core != nullptr);
	BOOST_CHECK(core->isInitialized());

	BOOST_CHECK_EQUAL(core->getFederationSize(), 1u);

	Core::federate_id_t id = core->registerFederate("sim1", helics::FederateInfo());
	
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
	
	helics::data_t *data;
	const Core::Handle *valueUpdates;
	uint64_t update_size = 0;

	core->timeRequest(id, 50.0);
	std::string str1 = "hello world";
	core->setValue(pub1, str1.data(), str1.size());
	valueUpdates = core->getValueUpdates(id, &update_size);
	BOOST_CHECK(valueUpdates == nullptr);
	BOOST_CHECK_EQUAL(update_size, 0u);
	data = core->getValue(sub1);
	BOOST_CHECK(data->data == nullptr);
	BOOST_CHECK_EQUAL(data->len, 0u);
	core->dereference(data);
	
	core->timeRequest(id, 100.0);
	valueUpdates = core->getValueUpdates(id, &update_size);
	BOOST_CHECK_EQUAL(valueUpdates[0], sub1);
	BOOST_CHECK_EQUAL(update_size, 1u);
	data = core->getValue(sub1);
	std::string str2(data->data, data->len);
	BOOST_CHECK_EQUAL(str1, str2);
	BOOST_CHECK_EQUAL(data->data, "hello world");
	BOOST_CHECK_EQUAL(data->len, str1.size());
	core->dereference(data);

	core->setValue(pub1, "hello\n\0helloAgain", 17);
	core->timeRequest(id, 150.0);
	valueUpdates = core->getValueUpdates(id, &update_size);
	BOOST_CHECK_EQUAL(valueUpdates[0], sub1);
	BOOST_CHECK_EQUAL(update_size, 1u);
	data = core->getValue(sub1);
	BOOST_CHECK_EQUAL(data->data, "hello\n\0helloAgain");
	BOOST_CHECK_EQUAL(data->len, 17u);
	core->dereference(data);

	core->timeRequest(id, 200.0);
	valueUpdates = core->getValueUpdates(id, &update_size);
	BOOST_CHECK(valueUpdates == nullptr);
	BOOST_CHECK_EQUAL(update_size, 0);
	
	delete core;
}

BOOST_AUTO_TEST_CASE(testcore_send_receive_test)
{
	const char *initializationString = "1";
	Core* core = CoreFactory::create(HELICS_TEST, initializationString);

	BOOST_REQUIRE(core != nullptr);
	BOOST_CHECK(core->isInitialized());

	BOOST_CHECK_EQUAL(core->getFederationSize(), 1u);

	Core::federate_id_t id = core->registerFederate("sim1", helics::FederateInfo());

	BOOST_CHECK_EQUAL(core->getFederateName(id), "sim1");
	BOOST_CHECK_EQUAL(core->getFederateId("sim1"), id);

	core->setTimeDelta(id, 1.0);
	
	Core::Handle end1 = core->registerEndpoint(id, "end1", "type");
	BOOST_CHECK_EQUAL(core->getType(end1), "type");

	Core::Handle end2 = core->registerEndpoint(id, "end2", "type");
	BOOST_CHECK_EQUAL(core->getType(end2), "type");

	core->enterInitializingState(id);

	core->enterExecutingState(id);

	helics::message_t *msg;
	std::string str1 = "hello world";
	core->timeRequest(id, 50.0);
	core->send(end1, "end2", str1.data(), str1.size());

	core->timeRequest(id, 100.0);
	BOOST_CHECK_EQUAL(core->receiveCount(end1), 0);
	BOOST_CHECK_EQUAL(core->receiveCount(end2), 1u);
	msg = core->receive(end1);
	BOOST_CHECK(msg == nullptr);
	msg = core->receive(end2);
	BOOST_CHECK_EQUAL(core->receiveCount(end2), 0);
	std::string str2(msg->data, msg->len);
	BOOST_CHECK_EQUAL(str1, str2);
	BOOST_CHECK_EQUAL(msg->len, str1.size());
	core->dereference(msg);
	
	delete core;
}

BOOST_AUTO_TEST_CASE(testcore_messagefilter_source_test)
{
	const char *initializationString = "1";
	Core *core = CoreFactory::create(HELICS_TEST, initializationString);

	BOOST_REQUIRE(core != nullptr);
	BOOST_CHECK(core->isInitialized());

	Core::federate_id_t id = core->registerFederate("sim1", helics::FederateInfo());

	Core::Handle end1 = core->registerEndpoint(id, "end1", "type");
	Core::Handle end2 = core->registerEndpoint(id, "end2", "type");
	Core::Handle end_filt = core->registerEndpoint(id, "end_filt", "type");

	Core::Handle srcFilter = core->registerSourceFilter(id, "srcFilter", "end1", "type");

	core->enterInitializingState(id);
	core->enterExecutingState(id);

	helics::message_t *msg;
	std::pair<Core::Handle, helics::message_t*> msgAny;
	std::string msgData = "hello world";
	std::string srcFilterName = "sourceFilter";
	core->send(end1, "end2", msgData.data(), msgData.size());

	core->timeRequest(id, 50.0);
	
	// Get message to filter. Update src and send to destination.
	BOOST_CHECK_EQUAL(core->receiveFilterCount(id), 1u);
	msgAny = core->receiveAnyFilter(id);
	BOOST_CHECK_EQUAL(msgAny.second->origsrc, "end1");
	BOOST_CHECK_EQUAL(msgAny.second->src, "end1");
	msgAny.second->src = srcFilterName.c_str();
	core->sendMessage(helics::invalid_Handle,msgAny.second);

	// dereference the original message should not cause problems; set src back to origsrc (srcFilterName.c_str is on stack, not heap)
	msgAny.second->src = msgAny.second->origsrc;
	core->dereference(msgAny.second);

	// Receive the filtered message
	BOOST_CHECK_EQUAL(core->receiveCount(end2), 1u);
	msg = core->receive(end2);
	BOOST_CHECK_EQUAL(msg->origsrc, "end1");
	BOOST_CHECK_EQUAL(msg->src, "sourceFilter");

	core->dereference(msg);
	delete core;
}


BOOST_AUTO_TEST_CASE(testcore_messagefilter_callback_test)
{
	// Create filter operator
	class TestOperator : public helics::FilterOperator {
	public:
		TestOperator(const char *name) {
			filterName = new char[strlen(name) + 1];
			strcpy(filterName, name);
		}

		helics::message_t process (helics::message_t *msg) override {
			if (msg->origsrc != msg->src) {
				delete msg->src;
			}
			char *srcName = new char[strlen(filterName) + 1];
			strcpy(srcName, filterName);
			msg->src = srcName;

			if (msg->len > 0) {
				((char*)msg->data)[0] = ((char*)msg->data)[0]++;
			}
			return *msg;
		}

		char *filterName = 0;
	};

	const char *initializationString = "1";
	Core *core = CoreFactory::create(HELICS_TEST, initializationString);

	BOOST_REQUIRE(core != nullptr);
	BOOST_CHECK(core->isInitialized());

	Core::federate_id_t id = core->registerFederate("sim1", helics::FederateInfo());


	Core::Handle end1 = core->registerEndpoint(id, "end1", "type");
	Core::Handle end2 = core->registerEndpoint(id, "end2", "type");

	Core::Handle srcFilter = core->registerSourceFilter(id, "srcFilter", "end1", "type");
	Core::Handle dstFilter = core->registerDestinationFilter(id, "dstFilter", "end2", "type");

	TestOperator *testSrcFilter = new TestOperator("sourceFilter");
	BOOST_CHECK_EQUAL(testSrcFilter->filterName, "sourceFilter");

	TestOperator *testDstFilter = new TestOperator("destinationFilter");
	BOOST_CHECK_EQUAL(testDstFilter->filterName, "destinationFilter");

	core->setFilterOperator(srcFilter, testSrcFilter);
	core->setFilterOperator(dstFilter, testDstFilter);

	core->enterInitializingState(id);
	core->enterExecutingState(id);

	helics::message_t *msg;
	std::pair<Core::Handle, helics::message_t*> msgAny;
	std::string msgData = "hello world";
	core->send(end1, "end2", msgData.data(), msgData.size()+1);

	core->timeRequest(id, 50.0);

	// All filters are using callbacks, there should be none to filter
	BOOST_CHECK_EQUAL(core->receiveFilterCount(id), 0u);

	// Receive the filtered message
	BOOST_CHECK_EQUAL(core->receiveCount(end2), 1u);
	msg = core->receive(end2);
	BOOST_CHECK_EQUAL(msg->origsrc, "end1");
	BOOST_CHECK_EQUAL(msg->data, "jello world");

	core->dereference(msg);

	delete testSrcFilter;
	delete testDstFilter;
	delete core;
}

BOOST_AUTO_TEST_SUITE_END()