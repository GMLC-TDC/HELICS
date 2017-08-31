/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "helics/application_api/MessageFederate.h"
#include "test_configuration.h"
#include "testFixtures.h"

#include <future>
#include <iostream>
#include <thread>
/** these test cases test out the message federates
*/

BOOST_FIXTURE_TEST_SUITE(message_federate_tests, MessageFederateTestFixture)

namespace bdata = boost::unit_test::data;
const std::string core_types[] = { "test" };


/** test simple creation and destruction*/
BOOST_DATA_TEST_CASE(message_federate_initialize_tests, bdata::make(core_types), core_type)
{
    Setup1FederateTest(core_type);

	mFed1->enterExecutionState();

	BOOST_CHECK(mFed1->currentState() == helics::Federate::op_states::execution);

	mFed1->finalize();

	BOOST_CHECK(mFed1->currentState() == helics::Federate::op_states::finalize);
}


BOOST_DATA_TEST_CASE(message_federate_endpoint_registration, bdata::make(core_types), core_type)
{
    Setup1FederateTest(core_type);

	auto epid = mFed1->registerEndpoint("ep1");
	auto epid2 = mFed1->registerGlobalEndpoint("ep2","random");

	
	mFed1->enterExecutionState();

	BOOST_CHECK(mFed1->currentState() == helics::Federate::op_states::execution);

	auto sv = mFed1->getEndpointName(epid);
	auto sv2 = mFed1->getEndpointName(epid2);
	BOOST_CHECK_EQUAL(sv, "test1/ep1");
	BOOST_CHECK_EQUAL(sv2, "ep2");
	

	BOOST_CHECK_EQUAL(mFed1->getEndpointType(epid), "");
	BOOST_CHECK_EQUAL(mFed1->getEndpointType(epid2), "random");

	BOOST_CHECK(mFed1->getEndpointId("ep1") == epid);
	BOOST_CHECK(mFed1->getEndpointId("test1/ep1") == epid);
	BOOST_CHECK(mFed1->getEndpointId("ep2") == epid2);
	mFed1->finalize();

	BOOST_CHECK(mFed1->currentState() == helics::Federate::op_states::finalize);
}


BOOST_DATA_TEST_CASE(message_federate_send_receive, bdata::make(core_types), core_type)
{
    Setup1FederateTest(core_type);

	auto epid = mFed1->registerEndpoint("ep1");
	auto epid2 = mFed1->registerGlobalEndpoint("ep2", "random");
	mFed1->setTimeDelta(1.0);

	mFed1->enterExecutionState();

	
	BOOST_CHECK(mFed1->currentState() == helics::Federate::op_states::execution);
	helics::data_block data(500, 'a');


	mFed1->sendMessage(epid, "ep2", data);

	auto time=mFed1->requestTime(1.0);
	BOOST_CHECK_EQUAL(time, 1.0);

	auto res = mFed1->hasMessage();
	BOOST_CHECK(res);
	res = mFed1->hasMessage(epid);
	BOOST_CHECK(res == false);
	res = mFed1->hasMessage(epid2);
	BOOST_CHECK(res);

	auto M = mFed1->getMessage(epid2);
	BOOST_REQUIRE(M);
	BOOST_REQUIRE_EQUAL(M->data.size(), data.size());

	BOOST_CHECK_EQUAL(M->data[245], data[245]);
	mFed1->finalize();

	BOOST_CHECK(mFed1->currentState() == helics::Federate::op_states::finalize);
}


BOOST_DATA_TEST_CASE(message_federate_send_receive_2fed, bdata::make(core_types), core_type)
{
    Setup2FederateTest(core_type);

	auto epid = mFed1->registerEndpoint("ep1");
	auto epid2 = mFed2->registerGlobalEndpoint("ep2", "random");
	mFed1->setTimeDelta(1.0);
	mFed2->setTimeDelta(1.0);

	auto f1finish = std::async(std::launch::async, [&]() { mFed1->enterExecutionState(); });
	mFed2->enterExecutionState();
	f1finish.wait();



	BOOST_CHECK(mFed1->currentState() == helics::Federate::op_states::execution);
	BOOST_CHECK(mFed2->currentState() == helics::Federate::op_states::execution);

	helics::data_block data(500, 'a');
	helics::data_block data2(400, 'b');

	mFed1->sendMessage(epid, "ep2", data);
	mFed2->sendMessage(epid2, "test1/ep1", data2);
	//move the time to 1.0
	auto f1time = std::async(std::launch::async, [&]() { return mFed1->requestTime(1.0); });
	auto gtime = mFed2->requestTime(1.0);

	BOOST_CHECK_EQUAL(gtime, 1.0);
	BOOST_CHECK_EQUAL(f1time.get(), 1.0);

	auto res = mFed1->hasMessage();
	BOOST_CHECK(res);
	res = mFed1->hasMessage(epid);
	BOOST_CHECK(res);
	res = mFed2->hasMessage(epid2);
	BOOST_CHECK(res);

	auto M1 = mFed1->getMessage(epid);
	BOOST_REQUIRE_EQUAL(M1->data.size(), data2.size());

	BOOST_CHECK_EQUAL(M1->data[245], data2[245]);

	auto M2 = mFed2->getMessage(epid2);
	BOOST_REQUIRE_EQUAL(M2->data.size(), data.size());

	BOOST_CHECK_EQUAL(M2->data[245], data[245]);
	mFed1->finalize();
	mFed2->finalize();

	BOOST_CHECK(mFed1->currentState() == helics::Federate::op_states::finalize);
	BOOST_CHECK(mFed2->currentState() == helics::Federate::op_states::finalize);
}


BOOST_DATA_TEST_CASE(message_federate_send_receive_2fed_multisend, bdata::make(core_types), core_type)
{
    Setup2FederateTest(core_type);

	auto epid = mFed1->registerEndpoint("ep1");
	auto epid2 = mFed2->registerGlobalEndpoint("ep2", "random");
	mFed1->setTimeDelta(1.0);
	mFed2->setTimeDelta(1.0);

	auto f1finish = std::async(std::launch::async, [&]() { mFed1->enterExecutionState(); });
	mFed2->enterExecutionState();
	f1finish.wait();



	BOOST_CHECK(mFed1->currentState() == helics::Federate::op_states::execution);
	BOOST_CHECK(mFed2->currentState() == helics::Federate::op_states::execution);

	helics::data_block data1(500, 'a');
	helics::data_block data2(400, 'b');
	helics::data_block data3(300, 'c');
	helics::data_block data4(200, 'd');
	mFed1->sendMessage(epid, "ep2", data1);
	mFed1->sendMessage(epid, "ep2", data2);
	mFed1->sendMessage(epid, "ep2", data3);
	mFed1->sendMessage(epid, "ep2", data4);
	//move the time to 1.0
	auto f1time = std::async(std::launch::async, [&]() { return mFed1->requestTime(1.0); });
	auto gtime = mFed2->requestTime(1.0);

	BOOST_CHECK_EQUAL(gtime, 1.0);
	BOOST_CHECK_EQUAL(f1time.get(), 1.0);

	BOOST_CHECK(!mFed1->hasMessage());

	BOOST_CHECK(!mFed1->hasMessage(epid));
	auto cnt = mFed2->receiveCount(epid2);
	BOOST_CHECK_EQUAL(cnt,4);

	auto M1 = mFed2->getMessage(epid2);
	BOOST_REQUIRE_EQUAL(M1->data.size(), data1.size());
	

	BOOST_CHECK_EQUAL(M1->data[245], data1[245]);
	//check the count decremented
	cnt = mFed2->receiveCount(epid2);
	BOOST_CHECK_EQUAL(cnt, 3);
	auto M2 = mFed2->getMessage();
	BOOST_REQUIRE_EQUAL(M2->data.size(), data2.size());
	BOOST_CHECK_EQUAL(M2->data[245], data2[245]);
	cnt = mFed2->receiveCount(epid2);
	BOOST_CHECK_EQUAL(cnt, 2);
	
	auto M3 = mFed2->getMessage();
	auto M4 = mFed2->getMessage(epid2);
	BOOST_CHECK_EQUAL(M3->data.size(), data3.size());
	BOOST_CHECK_EQUAL(M4->data.size(), data4.size());

	BOOST_CHECK_EQUAL(M4->src, "test1/ep1");
	BOOST_CHECK_EQUAL(M4->dest, "ep2");
	BOOST_CHECK_EQUAL(M4->origsrc, "test1/ep1");
	BOOST_CHECK_EQUAL(M4->time, 0.0);
	mFed1->finalize();
	mFed2->finalize();

	BOOST_CHECK(mFed1->currentState() == helics::Federate::op_states::finalize);
	BOOST_CHECK(mFed2->currentState() == helics::Federate::op_states::finalize);
}


#define ENABLE_OUTPUT
/**trivial Federate that sends Messages and echos a ping with a pong
*/
class pingpongFed
{
private:
	std::unique_ptr<helics::MessageFederate> mFed;
	helics::Time delta; //the minimum time delta for the federate
	std::string name;	//!< the name of the federate
    std::string coreType;
	std::vector<std::pair<helics::Time, std::string>> triggers;
	helics::endpoint_id_t ep;
	int index = 0;
public:
	int pings=0;  //!< the number of pings received
	int pongs=0;	//!< the number of pongs received
public:
	pingpongFed(std::string fname,helics::Time tDelta,std::string core_type) :
		delta(tDelta),name(std::move(fname)),coreType(core_type)
	{
		if (delta <= 0.0)
		{
			delta = 0.2;
		}
	}

private:
	void initialize()
	{
		helics::FederateInfo fi(name);
		fi.coreName = "pptest";
		fi.coreType = coreType;
		fi.coreInitString = "3";
		fi.timeDelta = delta;
#ifdef ENABLE_OUTPUT
		std::cout << std::string("about to create federate ") + name + "\n";
#endif
		mFed = std::make_unique<helics::MessageFederate>(fi);
#ifdef ENABLE_OUTPUT
		std::cout << std::string("registering federate ")+name+"\n";
#endif
		ep = mFed->registerEndpoint("port");
	}

private:
	void processMessages(helics::Time currentTime)
	{
		while (mFed->hasMessage(ep))
		{
			auto mess = mFed->getMessage(ep);
			auto messString = mess->data.to_string();
			if (messString == "ping")
			{
#ifdef ENABLE_OUTPUT
				std::cout << name << " :receive ping from " << std::string(mess->src) << " at time " << static_cast<double>(currentTime) << '\n';
#endif
				mess->data = "pong";
				mess->dest = mess->src;
				mess->src = name;
				mess->origsrc = mess->src;
				mess->time = currentTime;
				mFed->sendMessage(ep, std::move(mess));
				pings++;
			}
			else if (messString == "pong")
			{
				pongs++;
#ifdef ENABLE_OUTPUT
				std::cout << name << " :receive pong from " << std::string(mess->src) << " at time " << static_cast<double>(currentTime) << '\n';
#endif
			}
		}
	}
	void mainLoop(helics::Time finish)
	{
		helics::Time nextTime = 0;
		while (nextTime <= finish)
		{
			
			processMessages(nextTime);
			if (index<static_cast<int>(triggers.size()))
			{
				while (triggers[index].first <= nextTime)
				{
#ifdef ENABLE_OUTPUT
					std::cout << name << ": send ping to " << triggers[index].second << " at time " << static_cast<double>(nextTime) << '\n';
#endif
					mFed->sendMessage(ep, triggers[index].second, "ping");
					++index;
					if (index >= static_cast<int>(triggers.size()))
					{
						break;
					}
				}
				
			}
			nextTime += delta;
			nextTime=mFed->requestTime(nextTime);
				
		}
		mFed->finalize();
	}
public:
	void run(helics::Time finish)
	{
		initialize();
		mFed->enterExecutionState();
#ifdef ENABLE_OUTPUT
		std::cout << std::string("entering Execute Mode ")+name+"\n";
#endif
		mainLoop(finish);

	}
	void addTrigger(helics::Time triggerTime, const std::string &target)
	{
		triggers.emplace_back(triggerTime, target);
	}
};


BOOST_DATA_TEST_CASE(threefedPingPong, bdata::make(core_types), core_type)
{
    StartBroker(core_type, "3");

	pingpongFed p1("fedA", 0.5, core_type);
	pingpongFed p2("fedB", 0.5, core_type);
	pingpongFed p3("fedC", 0.5, core_type);

	p1.addTrigger(0.5, "fedB/port");
	p1.addTrigger(0.5, "fedC/port");
	p1.addTrigger(3.0, "fedB/port");
	p2.addTrigger(1.5, "fedA/port");
	p3.addTrigger(3.0, "fedB/port");
	p3.addTrigger(4.0, "fedA/port");

	auto t1 = std::thread([&p1]() {p1.run(6.0); });
	auto t2 = std::thread([&p2]() {p2.run(6.0); });
	auto t3 = std::thread([&p3]() {p3.run(6.0); });

	t1.join();
	t2.join();
	t3.join();
	BOOST_CHECK_EQUAL(p1.pings, 2);
	BOOST_CHECK_EQUAL(p2.pings, 3);
	BOOST_CHECK_EQUAL(p3.pings, 1);
	BOOST_CHECK_EQUAL(p1.pongs, 3);
	BOOST_CHECK_EQUAL(p2.pongs, 1);
	BOOST_CHECK_EQUAL(p3.pongs, 2);
}


BOOST_DATA_TEST_CASE(test_time_interruptions, bdata::make(core_types), core_type)
{
    Setup2FederateTest(core_type);

	auto epid = mFed1->registerEndpoint("ep1");
	auto epid2 = mFed2->registerGlobalEndpoint("ep2", "random");
	mFed1->setTimeDelta(1.0);
	mFed2->setTimeDelta(0.5);

	auto f1finish = std::async(std::launch::async, [&]() { mFed1->enterExecutionState(); });
	mFed2->enterExecutionState();
	f1finish.wait();



	BOOST_CHECK(mFed1->currentState() == helics::Federate::op_states::execution);
	BOOST_CHECK(mFed2->currentState() == helics::Federate::op_states::execution);

	helics::data_block data(500, 'a');
	helics::data_block data2(400, 'b');

	mFed1->sendMessage(epid, "ep2", data);
	mFed2->sendMessage(epid2, "test1/ep1", data2);
	//move the time to 1.0
	auto f1time = std::async(std::launch::async, [&]() { return mFed1->requestTime(1.0); });
	auto gtime = mFed2->requestTime(1.0);
	

	BOOST_REQUIRE_EQUAL(gtime,0.5);
	
	BOOST_REQUIRE(mFed2->hasMessage(epid));
	

	auto M2 = mFed2->getMessage(epid2);
	BOOST_REQUIRE_EQUAL(M2->data.size(), data.size());

	BOOST_CHECK_EQUAL(M2->data[245], data[245]);

	gtime = mFed2->requestTime(1.0);
	BOOST_CHECK_EQUAL(gtime, 1.0);

	BOOST_CHECK_EQUAL(f1time.get(), 1.0);
	auto M1 = mFed1->getMessage(epid);
	BOOST_REQUIRE_EQUAL(M1->data.size(), data2.size());

	BOOST_CHECK_EQUAL(M1->data[245], data2[245]);

	BOOST_CHECK(mFed1->hasMessage()==false);
	mFed1->finalize();
	mFed2->finalize();

	BOOST_CHECK(mFed1->currentState() == helics::Federate::op_states::finalize);
	BOOST_CHECK(mFed2->currentState() == helics::Federate::op_states::finalize);
}
BOOST_AUTO_TEST_SUITE_END()