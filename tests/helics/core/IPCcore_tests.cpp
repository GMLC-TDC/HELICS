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
//#include "boost/process.hpp"

BOOST_AUTO_TEST_SUITE(IPCCore_tests)

using helics::Core;
using helics::CoreFactory;

BOOST_AUTO_TEST_CASE(ipcqcore_initialization_test)
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
	size_t sz;
	unsigned int pri;
	mq->receive(data, 1024, sz, pri);
	BOOST_CHECK_GT(sz, 0);
	helics::ActionMessage rM(data, sz);
	BOOST_CHECK_EQUAL(rM.name, "core1");
	BOOST_CHECK(rM.action() == helics::action_message_def::action_t::cmd_reg_broker);
	core->disconnect();
	boost::interprocess::message_queue::remove("testbroker");
}

BOOST_AUTO_TEST_SUITE_END()