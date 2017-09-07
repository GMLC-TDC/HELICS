/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include <boost/test/unit_test.hpp>

#include "helics/core/CoreFactory.h"
#include "helics/core/core.h"
#include "helics/core/core-types.h"
#include "helics/core/ipc/IpcCore.h"
#include "boost/interprocess/ipc/message_queue.hpp"
#include "helics/core/ipc/IpcComms.h"
#include "helics/core/ActionMessage.h"
//#include "boost/process.hpp"
#include <future>

BOOST_AUTO_TEST_SUITE(IPCCore_tests)

using helics::Core;
using helics::CoreFactory;

BOOST_AUTO_TEST_CASE(ipccomms_broker_test)
{
	std::atomic<int> counter{ 0 };
	std::string brokerLoc = "brokerIPC";
	std::string localLoc = "localIPC";
	helics::IpcComms comm(localLoc,brokerLoc);

	std::unique_ptr<boost::interprocess::message_queue> mq;
	try
	{
		mq = std::make_unique<boost::interprocess::message_queue>(boost::interprocess::create_only, brokerLoc.c_str(), 1024, 1024);
	}
	catch (boost::interprocess::interprocess_exception &ipe)
	{
		boost::interprocess::message_queue::remove(brokerLoc.c_str());
		mq = std::make_unique<boost::interprocess::message_queue>(boost::interprocess::create_only, brokerLoc.c_str(), 1024, 1024);
	}

	comm.setCallback([&counter](helics::ActionMessage m) {++counter; });

	bool connected=comm.connect();
	BOOST_REQUIRE(connected);
	comm.transmit(0,helics::CMD_IGNORE);

	char data[1024];
	size_t sz=0;
	unsigned int pri;
	while (sz < 32)
	{
		mq->receive(data, 1024, sz, pri);
	}
	
	BOOST_CHECK_GT(sz, 32);
	helics::ActionMessage rM(data, sz);
	BOOST_CHECK(rM.action() == helics::action_message_def::action_t::cmd_ignore);
	comm.disconnect();
	boost::interprocess::message_queue::remove(brokerLoc.c_str());
}

BOOST_AUTO_TEST_CASE(ipccomms_rx_test)
{
	std::atomic<int> counter{ 0 };
	helics::ActionMessage act;
	std::string brokerLoc = "";
	std::string localLoc = "localIPC";
	helics::IpcComms comm(localLoc,brokerLoc);

	std::unique_ptr<boost::interprocess::message_queue> mq;
	
	comm.setCallback([&counter, &act](helics::ActionMessage m) {++counter; act = m; });

	bool connected = comm.connect();
	BOOST_REQUIRE(connected);
	mq = std::make_unique<boost::interprocess::message_queue>(boost::interprocess::open_only, localLoc.c_str());
	
	helics::ActionMessage cmd(helics::CMD_ACK);
	std::string buffer = cmd.to_string();
	mq->send(buffer.data(), buffer.size(), 2);
	std::this_thread::sleep_for(std::chrono::milliseconds(250));
	BOOST_REQUIRE_EQUAL(counter, 1);
	BOOST_CHECK(act.action() == helics::action_message_def::action_t::cmd_ack);
	comm.disconnect();
	
}


BOOST_AUTO_TEST_CASE(ipcComm_transmit_through)
{
	
	std::atomic<int> counter{ 0 };
	std::string brokerLoc = "brokerIPC";
	std::string localLoc = "localIPC";
	//just to make sure these are not already present from a failure
	boost::interprocess::message_queue::remove(brokerLoc.c_str());
	boost::interprocess::message_queue::remove(localLoc.c_str());
	std::atomic<int> counter2{ 0 };
	helics::ActionMessage act;
	helics::ActionMessage act2;

	helics::IpcComms comm(localLoc, brokerLoc);
	helics::IpcComms comm2(brokerLoc, "");
	

	comm.setCallback([&counter, &act](helics::ActionMessage m) {++counter; act = m; });
	comm2.setCallback([&counter2, &act2](helics::ActionMessage m) {++counter2; act2 = m; });

	//need to launch the connection commands at the same time since they depend on eachother in this case
	//auto connected_fut = std::async(std::launch::async, [&comm] {return comm.connect(); });
	
	bool connected = comm2.connect();
	BOOST_REQUIRE(connected);
	//connected = connected_fut.get();
	connected = comm.connect();
	BOOST_REQUIRE(connected);

	comm.transmit(0, helics::CMD_ACK);

	std::this_thread::sleep_for(std::chrono::milliseconds(250));
	BOOST_REQUIRE_EQUAL(counter2, 1);
	BOOST_CHECK(act2.action() == helics::action_message_def::action_t::cmd_ack);

	/*
	comm2.transmit(0, helics::CMD_ACK);
	std::this_thread::sleep_for(std::chrono::milliseconds(250));
	BOOST_REQUIRE_EQUAL(counter, 1);
	BOOST_CHECK(act.action() == helics::action_message_def::action_t::cmd_ack);
	*/
	comm.disconnect();
	comm2.disconnect();
}

BOOST_AUTO_TEST_CASE(ipcComm_transmit_add_route)
{

	std::atomic<int> counter{ 0 };
	std::string brokerLoc = "brokerIPC";
	std::string localLoc = "localIPC";
	std::string localLocB = "localIPC2";
	//just to make sure these are not already present from a failure
	boost::interprocess::message_queue::remove(brokerLoc.c_str());
	boost::interprocess::message_queue::remove(localLoc.c_str());
	boost::interprocess::message_queue::remove(localLocB.c_str());

	std::atomic<int> counter2{ 0 };
	std::atomic<int> counter3{ 0 };
	helics::ActionMessage act;
	helics::ActionMessage act2;
	helics::ActionMessage act3;

	helics::IpcComms comm(localLoc, brokerLoc);
	helics::IpcComms comm2(brokerLoc, "");
	helics::IpcComms comm3(localLocB, brokerLoc);

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

	comm2.addRoute(3, localLocB);

	comm2.transmit(3, helics::CMD_ACK);

	std::this_thread::sleep_for(std::chrono::milliseconds(250));
	if (counter3 != 1)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
	}
	BOOST_REQUIRE_EQUAL(counter3, 1);
	BOOST_CHECK(act3.action() == helics::action_message_def::action_t::cmd_ack);

	comm2.addRoute(4, localLoc);

	comm2.transmit(4, helics::CMD_ACK);

	std::this_thread::sleep_for(std::chrono::milliseconds(250));
	BOOST_REQUIRE_EQUAL(counter, 1);
	BOOST_CHECK(act.action() == helics::action_message_def::action_t::cmd_ack);
	
	comm.disconnect();
	comm2.disconnect();
	comm3.disconnect();
}

BOOST_AUTO_TEST_CASE(ipccore_initialization_test)
{
	std::string initializationString = "1 --brokerloc=testbroker --name=core1";
	auto  core = CoreFactory::create(HELICS_INTERPROCESS, initializationString);

	BOOST_REQUIRE(core != nullptr);
	BOOST_CHECK(core->isInitialized());
	std::unique_ptr<boost::interprocess::message_queue> mq;
	try
	{

		mq=std::make_unique<boost::interprocess::message_queue>(boost::interprocess::create_only, "testbroker",1024,1024);
	}
	catch (boost::interprocess::interprocess_exception &ipe)
	{
		boost::interprocess::message_queue::remove("testbroker");
		mq = std::make_unique<boost::interprocess::message_queue>(boost::interprocess::create_only, "testbroker", 1024, 1024);
	}
	core->connect();

	char data[1024];
	size_t sz=0;
	unsigned int pri;
	while (sz < 31)
	{
		mq->receive(data, 1024, sz, pri);
	}
	
	BOOST_CHECK_GT(sz, 31);
	helics::ActionMessage rM(data, sz);
	BOOST_CHECK_EQUAL(rM.name, "core1");
	BOOST_CHECK(rM.action() == helics::action_message_def::action_t::cmd_reg_broker);
	core->disconnect();
	boost::interprocess::message_queue::remove("testbroker");
}

BOOST_AUTO_TEST_SUITE_END()