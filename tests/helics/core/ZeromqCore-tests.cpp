/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include <boost/test/unit_test.hpp>

#include "helics/core/CoreFactory.h"
#include "helics/core/core.h"
#include "helics/core/core-types.h"
#include "helics/core/zmq/ZmqCore.h"
#include "helics/core/zmq/ZmqComms.h"
#include "helics/common/cppzmq/zmq.hpp"
#include "helics/core/zmq/ZmqBroker.h"
#include "helics/common/zmqContextManager.h"
#include "helics/core/ActionMessage.h"
//#include "boost/process.hpp"
#include <future>

BOOST_AUTO_TEST_SUITE(ZMQCore_tests)

using helics::Core;
using helics::CoreFactory;

BOOST_AUTO_TEST_CASE(zmqComms_broker_test)
{
	std::atomic<int> counter{ 0 };
	std::string host = "tcp://127.0.0.1";
	helics::ZmqComms comm(host,host);

	auto ctx = zmqContextManager::getContextPointer();
	zmq::socket_t repSocket(ctx->getContext(), ZMQ_REP);
	repSocket.bind("tcp://127.0.0.1:23405");
	

	comm.setCallback([&counter](helics::ActionMessage m) {++counter; });
	comm.setBrokerPorts(23405);
	comm.setName("tests");
	auto confut = std::async(std::launch::async, [&comm]() {return comm.connect(); });
	
	zmq::message_t rxmsg;
	
	repSocket.recv(&rxmsg);
	
	BOOST_CHECK_GT(rxmsg.size(), 32);
	
	

	
	helics::ActionMessage rM(static_cast<char *>(rxmsg.data()), rxmsg.size());
	BOOST_CHECK(rM.action() == helics::action_message_def::action_t::cmd_protocol);
	rM.index = DISCONNECT;
	repSocket.send(rM.to_string());
	auto connected = confut.get();
	BOOST_CHECK(!connected);
}

BOOST_AUTO_TEST_CASE(zmqComms_broker_test_transmit)
{
	std::atomic<int> counter{ 0 };
	std::string host = "tcp://127.0.0.1";
	helics::ZmqComms comm(host, host);

	auto ctx = zmqContextManager::getContextPointer();
	zmq::socket_t repSocket(ctx->getContext(), ZMQ_REP);
	repSocket.bind("tcp://127.0.0.1:23405");

	zmq::socket_t pullSocket(ctx->getContext(), ZMQ_PULL);
	pullSocket.bind("tcp://127.0.0.1:23406");


	comm.setCallback([&counter](helics::ActionMessage m) {++counter; });
	comm.setBrokerPorts(23405);
	comm.setPortNumbers(23407);
	comm.setName("tests");
	bool connected=comm.connect();
	BOOST_REQUIRE(connected);
	comm.transmit(0, helics::CMD_IGNORE);
	zmq::message_t rxmsg;

	pullSocket.recv(&rxmsg);

	BOOST_CHECK_GT(rxmsg.size(), 32);
	helics::ActionMessage rM(static_cast<char *>(rxmsg.data()), rxmsg.size());
	BOOST_CHECK(rM.action() == helics::action_message_def::action_t::cmd_ignore);
	comm.disconnect();
}

BOOST_AUTO_TEST_CASE(zmqComms_rx_test)
{
	std::atomic<int> counter{ 0 };
	helics::ActionMessage act;
	std::string host = "tcp://127.0.0.1";
	helics::ZmqComms comm(host, host);

	auto ctx = zmqContextManager::getContextPointer();
	zmq::socket_t repSocket(ctx->getContext(), ZMQ_REP);
	repSocket.bind("tcp://127.0.0.1:23405");

	zmq::socket_t pullSocket(ctx->getContext(), ZMQ_PULL);
	pullSocket.bind("tcp://127.0.0.1:23406");


	comm.setBrokerPorts(23405,23406);
	comm.setPortNumbers(23407,23408);
	comm.setName("tests");
	comm.setCallback([&counter, &act](helics::ActionMessage m) {++counter; act = m; });

	bool connected = comm.connect();
	BOOST_REQUIRE(connected);

	zmq::socket_t pushSocket(ctx->getContext(), ZMQ_PUSH);
	pushSocket.connect("tcp://127.0.0.1:23408");

	helics::ActionMessage cmd(helics::CMD_ACK);
	std::string buffer = cmd.to_string();
	try
	{
		auto cnt=pushSocket.send(buffer, ZMQ_DONTWAIT);
		BOOST_REQUIRE_EQUAL(cnt,buffer.size());
	}
	catch (const zmq::error_t &ze)
	{
		BOOST_REQUIRE_MESSAGE(false,"Message failed to send");
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(250));
	BOOST_REQUIRE_EQUAL(counter, 1);
	BOOST_CHECK(act.action() == helics::action_message_def::action_t::cmd_ack);
	comm.disconnect();

}


BOOST_AUTO_TEST_CASE(zmqComm_transmit_through)
{

	std::atomic<int> counter{ 0 };
	std::atomic<int> counter2{ 0 };
	helics::ActionMessage act;
	helics::ActionMessage act2;

	std::string host = "tcp://127.0.0.1";
	helics::ZmqComms comm(host, host);
	helics::ZmqComms comm2(host, "");

	comm.setBrokerPorts(23405, 23406);
	comm.setName("tests");
	comm2.setName("test2");
	comm2.setPortNumbers(23405, 23406);
	comm.setPortNumbers(23407, 23408);

	comm.setCallback([&counter, &act](helics::ActionMessage m) {++counter; act = m; });
	comm2.setCallback([&counter2, &act2](helics::ActionMessage m) {++counter2; act2 = m; });

	//need to launch the connection commands at the same time since they depend on eachother in this case
	auto connected_fut = std::async(std::launch::async, [&comm] {return comm.connect(); });

	bool connected = comm2.connect();
	BOOST_REQUIRE(connected);
	connected = connected_fut.get();
	BOOST_REQUIRE(connected);

	comm.transmit(0, helics::CMD_ACK);

	std::this_thread::sleep_for(std::chrono::milliseconds(250));
	if (counter2 != 1)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	BOOST_REQUIRE_EQUAL(counter2, 1);
	BOOST_CHECK(act2.action() == helics::action_message_def::action_t::cmd_ack);


	comm.disconnect();
	comm2.disconnect();
}

BOOST_AUTO_TEST_CASE(zmqComm_transmit_add_route)
{

	std::atomic<int> counter{ 0 };
	std::atomic<int> counter2{ 0 };
	std::atomic<int> counter3{ 0 };

	std::string host = "tcp://127.0.0.1";
	helics::ZmqComms comm(host, host);
	helics::ZmqComms comm2(host, "");
	helics::ZmqComms comm3(host, host);

	
	comm.setBrokerPorts(23405, 23406);
	comm.setName("tests");
	comm2.setName("broker");
	comm3.setName("test3");
	comm3.setBrokerPorts(23405, 23406);

	comm2.setPortNumbers(23405, 23406);
	comm.setPortNumbers(23407, 23408);
	comm3.setPortNumbers(23409, 23410);
	
	helics::ActionMessage act;
	helics::ActionMessage act2;
	helics::ActionMessage act3;

	comm.setCallback([&counter, &act](helics::ActionMessage m) {++counter; act = m; });
	comm2.setCallback([&counter2, &act2](helics::ActionMessage m) {++counter2; act2 = m; });
	comm3.setCallback([&counter3, &act3](helics::ActionMessage m) {++counter3; act3 = m; });

	//need to launch the connection commands at the same time since they depend on eachother in this case
	//auto connected_fut = std::async(std::launch::async, [&comm] {return comm.connect(); });

	bool connected = comm2.connect();
	BOOST_REQUIRE(connected);
	//connected = connected_fut.get();
	connected = comm.connect();
	BOOST_REQUIRE(connected);
	connected = comm3.connect();

	comm.transmit(0, helics::CMD_ACK);

	std::this_thread::sleep_for(std::chrono::milliseconds(250));
	BOOST_REQUIRE_EQUAL(counter2, 1);
	BOOST_CHECK(act2.action() == helics::action_message_def::action_t::cmd_ack);

	comm3.transmit(0, helics::CMD_ACK);

	std::this_thread::sleep_for(std::chrono::milliseconds(250));
	BOOST_REQUIRE_EQUAL(counter2, 2);
	BOOST_CHECK(act2.action() == helics::action_message_def::action_t::cmd_ack);

	comm2.addRoute(3, comm3.getPushAddress());

	comm2.transmit(3, helics::CMD_ACK);

	std::this_thread::sleep_for(std::chrono::milliseconds(250));
	if (counter3 != 1)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
	}
	BOOST_REQUIRE_EQUAL(counter3, 1);
	BOOST_CHECK(act3.action() == helics::action_message_def::action_t::cmd_ack);

	comm2.addRoute(4, comm.getPushAddress());

	comm2.transmit(4, helics::CMD_ACK);

	std::this_thread::sleep_for(std::chrono::milliseconds(250));
	BOOST_REQUIRE_EQUAL(counter, 1);
	BOOST_CHECK(act.action() == helics::action_message_def::action_t::cmd_ack);

	comm.disconnect();
	comm2.disconnect();
	comm3.disconnect();
}

BOOST_AUTO_TEST_CASE(zmqCore_initialization_test)
{
	std::string initializationString = "1 --brokerport=23405 --repport=23410 --local_interface=tcp://127.0.0.1 --name=core1";
	auto  core = CoreFactory::create(HELICS_ZMQ, initializationString);

	BOOST_REQUIRE(core != nullptr);
	BOOST_CHECK(core->isInitialized());
	auto ctx = zmqContextManager::getContextPointer();
	zmq::socket_t repSocket(ctx->getContext(), ZMQ_REP);
	repSocket.bind("tcp://127.0.0.1:23405");

	zmq::socket_t pullSocket(ctx->getContext(), ZMQ_PULL);
	pullSocket.bind("tcp://127.0.0.1:23406");
	bool connected=core->connect();
	BOOST_REQUIRE(connected);

	zmq::message_t rxmsg;

	repSocket.recv(&rxmsg);
	
	

	BOOST_CHECK_GT(rxmsg.size(), 32);
	helics::ActionMessage rM(static_cast<char *>(rxmsg.data()), rxmsg.size());

	BOOST_CHECK_EQUAL(rM.name, "core1");
	BOOST_CHECK(rM.action() == helics::action_message_def::action_t::cmd_reg_broker);
	helics::ActionMessage resp(helics::CMD_PRIORITY_ACK);
	repSocket.send(resp.to_string());

	core->disconnect();
	//boost::interprocess::message_queue::remove("testbroker");
}

BOOST_AUTO_TEST_SUITE_END()