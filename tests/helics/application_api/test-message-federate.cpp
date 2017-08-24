/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>

#include "helics/application_api/MessageFederate.h"
#include "test_configuration.h"

#include <future>
#include <iostream>
#include <thread>
/** these test cases test out the message federates
*/

BOOST_AUTO_TEST_SUITE(message_federate_tests)

/** test simple creation and destruction*/
BOOST_AUTO_TEST_CASE(message_federate_initialize_tests)
{
	helics::FederateInfo fi("test1");
	fi.coreType = CORE_TYPE_TO_TEST;
	fi.coreInitString = "1";

	auto mFed = std::make_shared<helics::MessageFederate>(fi);

	mFed->enterExecutionState();

	BOOST_CHECK(mFed->currentState() == helics::Federate::op_states::execution);

	mFed->finalize();

	BOOST_CHECK(mFed->currentState() == helics::Federate::op_states::finalize);
}

BOOST_AUTO_TEST_CASE(message_federate_endpoint_registration)
{
	helics::FederateInfo fi("test1");
	fi.coreType = CORE_TYPE_TO_TEST;
	fi.coreInitString = "1";

	auto mFed = std::make_shared<helics::MessageFederate>(fi);

	auto epid = mFed->registerEndpoint("ep1");
	auto epid2 = mFed->registerGlobalEndpoint("ep2","random");

	
	mFed->enterExecutionState();

	BOOST_CHECK(mFed->currentState() == helics::Federate::op_states::execution);

	auto sv = mFed->getEndpointName(epid);
	auto sv2 = mFed->getEndpointName(epid2);
	BOOST_CHECK_EQUAL(sv, "test1/ep1");
	BOOST_CHECK_EQUAL(sv2, "ep2");
	

	BOOST_CHECK_EQUAL(mFed->getEndpointType(epid), "");
	BOOST_CHECK_EQUAL(mFed->getEndpointType(epid2), "random");

	BOOST_CHECK(mFed->getEndpointId("ep1") == epid);
	BOOST_CHECK(mFed->getEndpointId("test1/ep1") == epid);
	BOOST_CHECK(mFed->getEndpointId("ep2") == epid2);
	mFed->finalize();

	BOOST_CHECK(mFed->currentState() == helics::Federate::op_states::finalize);
}


BOOST_AUTO_TEST_CASE(message_federate_send_receive)
{
	helics::FederateInfo fi("test1");
	fi.coreType = CORE_TYPE_TO_TEST;
	fi.coreInitString = "1";

	auto mFed = std::make_shared<helics::MessageFederate>(fi);

	auto epid = mFed->registerEndpoint("ep1");
	auto epid2 = mFed->registerGlobalEndpoint("ep2", "random");
	mFed->setTimeDelta(1.0);

	mFed->enterExecutionState();

	
	BOOST_CHECK(mFed->currentState() == helics::Federate::op_states::execution);
	helics::data_block data(500, 'a');


	mFed->sendMessage(epid, "ep2", data);

	auto time=mFed->requestTime(1.0);
	BOOST_CHECK_EQUAL(time, 1.0);

	auto res = mFed->hasMessage();
	BOOST_CHECK(res);
	res = mFed->hasMessage(epid);
	BOOST_CHECK(res == false);
	res = mFed->hasMessage(epid2);
	BOOST_CHECK(res);

	auto M = mFed->getMessage(epid2);
	BOOST_REQUIRE(M);
	BOOST_REQUIRE_EQUAL(M->data.size(), data.size());

	BOOST_CHECK_EQUAL(M->data[245], data[245]);
	mFed->finalize();

	BOOST_CHECK(mFed->currentState() == helics::Federate::op_states::finalize);
}


BOOST_AUTO_TEST_CASE(message_federate_send_receive_2fed)
{
	helics::FederateInfo fi("test1");
	fi.coreType = CORE_TYPE_TO_TEST;
	fi.coreInitString = "2";

	auto mFed1 = std::make_shared<helics::MessageFederate>(fi);
	fi.name = "test2";
	auto mFed2 = std::make_shared<helics::MessageFederate>(fi);

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

BOOST_AUTO_TEST_CASE(message_federate_send_receive_2fed_multisend)
{
	helics::FederateInfo fi("test1");
	fi.coreType = CORE_TYPE_TO_TEST;
	fi.coreInitString = "2";

	auto mFed1 = std::make_shared<helics::MessageFederate>(fi);
	fi.name = "test2";
	auto mFed2 = std::make_shared<helics::MessageFederate>(fi);

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

//#define ENABLE_OUTPUT
//trivial Federate that sends Messages and echos a ping with a pong
class pingpongFed
{
private:
	std::unique_ptr<helics::MessageFederate> mFed;
	helics::Time delta;
	std::string name;
	std::vector<std::pair<helics::Time, std::string>> triggers;
	helics::endpoint_id_t ep;
	int index = 0;
public:
	int pings=0;
	int pongs=0;
public:
	pingpongFed(std::string fname,helics::Time tDelta) :
		delta(tDelta),name(std::move(fname))
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
		fi.coreName = "test";
		fi.coreType = CORE_TYPE_TO_TEST;
		fi.coreInitString = "3";
		fi.timeDelta = delta;
		mFed = std::make_unique<helics::MessageFederate>(fi);
		ep=mFed->registerEndpoint("port");
#ifdef ENABLE_OUTPUT
		std::cout << std::string("registering federate ")+name+"\n";
#endif
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


BOOST_AUTO_TEST_CASE(threefedPingPong)
{
	pingpongFed p1("fedA",0.5);
	pingpongFed p2("fedB", 0.5);
	pingpongFed p3("fedC", 0.5);

	p1.addTrigger(0.5, "fedB.port");
	p1.addTrigger(0.5, "fedC.port");
	p1.addTrigger(3.0, "fedB.port");
	p2.addTrigger(1.5, "fedA.port");
	p3.addTrigger(3.0, "fedB.port");
	p3.addTrigger(4.0, "fedA.port");

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


BOOST_AUTO_TEST_CASE(test_time_interruptions)
{
	helics::FederateInfo fi("test1");
	fi.coreType = CORE_TYPE_TO_TEST;
	fi.coreInitString = "2";

	auto mFed1 = std::make_shared<helics::MessageFederate>(fi);
	fi.name = "test2";
	auto mFed2 = std::make_shared<helics::MessageFederate>(fi);

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
	mFed2->sendMessage(epid2, "test1.ep1", data2);
	//move the time to 1.0
	auto f1time = std::async(std::launch::async, [&]() { return mFed1->requestTime(1.0); });
	auto gtime = mFed2->requestTime(1.0);
	

	BOOST_REQUIRE_EQUAL(gtime,0.5);
	
	BOOST_REQUIRE(mFed1->hasMessage(epid));
	

	auto M1 = mFed1->getMessage(epid);
	BOOST_REQUIRE_EQUAL(M1->data.size(), data2.size());

	BOOST_CHECK_EQUAL(M1->data[245], data2[245]);

	gtime = mFed2->requestTime(1.0);
	BOOST_CHECK_EQUAL(f1time.get(), 0.5);
	BOOST_CHECK_EQUAL(gtime, 1.0);

	auto M2 = mFed2->getMessage(epid2);
	BOOST_REQUIRE_EQUAL(M2->data.size(), data.size());

	BOOST_CHECK_EQUAL(M2->data[245], data[245]);

	BOOST_CHECK(mFed1->hasMessage()==false);
	mFed1->finalize();
	mFed2->finalize();

	BOOST_CHECK(mFed1->currentState() == helics::Federate::op_states::finalize);
	BOOST_CHECK(mFed2->currentState() == helics::Federate::op_states::finalize);
}
BOOST_AUTO_TEST_SUITE_END()