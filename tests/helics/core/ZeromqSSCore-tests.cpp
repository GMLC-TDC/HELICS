/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include <boost/test/unit_test.hpp>
#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include "helics/application_api/Inputs.hpp"
#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/common/cppzmq/zmq.hpp"
#include "helics/common/zmqContextManager.h"
#include "helics/core/ActionMessage.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/core-types.hpp"
#include "helics/core/zmq/ZmqBroker.h"
#include "helics/core/zmq/ZmqCommsSS.h"
#include "helics/core/zmq/ZmqCore.h"
#include "helics/common/GuardedTypes.hpp"

//#include "boost/process.hpp"
#include <future>
#include <iostream>

namespace utf = boost::unit_test;
using namespace std::literals::chrono_literals;

BOOST_AUTO_TEST_SUITE (ZMQSSCore_tests, *utf::label("ci"))

using helics::Core;

#define ZMQ_SS_BROKER_PORT 23414

const std::string host = "tcp://127.0.0.1";

BOOST_AUTO_TEST_CASE (zmqSSComm_transmit)
{
    std::this_thread::sleep_for (400ms);
    std::atomic<int> counter{0};
    std::atomic<int> counter2{0};
    guarded<helics::ActionMessage> act;
    guarded<helics::ActionMessage> act2;

    helics::zeromq::ZmqCommsSS comm;
    helics::zeromq::ZmqCommsSS comm2;
    comm.loadTargetInfo (host, host);
    // comm2 is the broker
    comm2.loadTargetInfo (host, std::string ());

    comm.setBrokerPort (ZMQ_SS_BROKER_PORT);
    comm.setName ("test_comms");
    comm.setServerMode (false);
    comm2.setName ("test_broker");
    comm2.setPortNumber (ZMQ_SS_BROKER_PORT);
    comm2.setServerMode (true);

    comm.setCallback ([&counter, &act](helics::ActionMessage m) {
        ++counter;
        act = m;
    });
    comm2.setCallback ([&counter2, &act2](helics::ActionMessage m) {
        ++counter2;
        act2 = m;
    });
    // need to launch the connection commands at the same time since they depend on each other in this case
    auto connected_fut = std::async (std::launch::async, [&comm] { return comm.connect (); });
    bool connected1 = comm2.connect ();
    BOOST_REQUIRE (connected1);
    bool connected2 = connected_fut.get ();
    if (!connected2)
    {  // lets just try again if it is not connected
        connected2 = comm.connect ();
    }
    BOOST_REQUIRE (connected2);

    comm.transmit (helics::parent_route_id, helics::CMD_ACK);
    std::this_thread::sleep_for (250ms);
    if (counter2 != 1)
    {
        std::this_thread::sleep_for (500ms);
    }
    BOOST_REQUIRE_EQUAL (counter2, 2);
    BOOST_CHECK (act2.lock ()->action () == helics::action_message_def::action_t::cmd_ack);

    comm2.disconnect ();
    BOOST_CHECK (!comm2.isConnected ());
    comm.disconnect ();
    BOOST_CHECK (!comm.isConnected ());

    std::this_thread::sleep_for (100ms);
}

BOOST_AUTO_TEST_CASE (zmqSSComm_addroute)
{
    std::this_thread::sleep_for (400ms);
    std::atomic<int> counter{0};
    std::atomic<int> counter2{0};
    std::atomic<int> counter3{0};
    guarded<helics::ActionMessage> act;
    guarded<helics::ActionMessage> act2;
    guarded<helics::ActionMessage> act3;

    helics::zeromq::ZmqCommsSS comm;
    helics::zeromq::ZmqCommsSS comm2;
    helics::zeromq::ZmqCommsSS comm3;
    comm.loadTargetInfo (host, host);
    comm2.loadTargetInfo (host, host);
    // comm3 is the broker
    comm3.loadTargetInfo (host, std::string ());

    comm.setBrokerPort (ZMQ_SS_BROKER_PORT);
    comm.setName ("test1");
    comm.setServerMode (false);

    comm2.setBrokerPort (ZMQ_SS_BROKER_PORT);
    comm2.setName ("test2");
    comm2.setServerMode (false);

    comm3.setName ("test_broker");
    comm3.setPortNumber (ZMQ_SS_BROKER_PORT);
    comm3.setServerMode (true);

    comm.setCallback ([&counter, &act](helics::ActionMessage m) {
        ++counter;
        act = m;
    });
    comm2.setCallback ([&counter2, &act2](helics::ActionMessage m) {
        ++counter2;
        act2 = m;
    });
    comm3.setCallback ([&counter3, &act3](helics::ActionMessage m) {
		++counter3;
		act3 = m;
    });
    // need to launch the connection commands at the same time since they depend on each other in this case
    auto connected_fut = std::async (std::launch::async, [&comm] { return comm.connect (); });
    auto connected_fut2 = std::async (std::launch::async, [&comm2] { return comm2.connect (); });
    bool connected1 = comm3.connect ();
    BOOST_REQUIRE (connected1);
    bool connected2 = connected_fut.get ();
    if (!connected2)
    {  // lets just try again if it is not connected
        connected2 = comm.connect ();
    }
    BOOST_REQUIRE (connected2);
    connected2 = connected_fut2.get ();
    if (!connected2)
    {  // lets just try again if it is not connected
        connected2 = comm2.connect ();
    }
    BOOST_REQUIRE (connected2);
    comm.transmit (helics::parent_route_id, helics::CMD_ACK);
    std::this_thread::sleep_for (250ms);
    if (counter3 != 3)
    {
        std::this_thread::sleep_for (500ms);
    }
    BOOST_REQUIRE_EQUAL (counter3, 3);
    comm3.addRoute (helics::route_id (2), comm2.getAddress ());
    comm3.transmit (helics::route_id (2), helics::CMD_ACK);
    std::this_thread::sleep_for (250ms);
    if (counter2 != 1)
    {
    	std::this_thread::sleep_for (500ms);
    }
    BOOST_REQUIRE_EQUAL (counter2, 1);
    BOOST_CHECK (act2.lock ()->action () == helics::action_message_def::action_t::cmd_ack);

    comm.disconnect ();
    BOOST_CHECK (!comm.isConnected ());
    comm2.disconnect ();
    BOOST_CHECK (!comm2.isConnected ());
    comm3.disconnect ();
    BOOST_CHECK (!comm3.isConnected ());
    std::this_thread::sleep_for (100ms);
}

BOOST_AUTO_TEST_CASE (zmqSSCore_initialization_test)
{
    std::atomic<int> counter{0};
    std::vector<helics::ActionMessage> msgs;
    helics::zeromq::ZmqCommsSS comm;
    comm.loadTargetInfo (host, std::string());
    comm.setName ("test_broker");
    comm.setPortNumber (ZMQ_SS_BROKER_PORT);
    comm.setServerMode (true);
    comm.setCallback ([&counter, &msgs](helics::ActionMessage m) {
            ++counter;
            msgs.push_back(m);
        });
    comm.connect();

    std::string initializationString = "-f 1 --name=core1";
    auto core = helics::CoreFactory::create (helics::core_type::ZMQ_SS, initializationString);

    BOOST_REQUIRE (core);
    BOOST_CHECK (core->isInitialized ());

    std::this_thread::sleep_for (100ms);
    bool connected = core->connect ();
    BOOST_CHECK (connected);

    if (connected)
    {
        int cnt = 0;
        while (counter == 0)
        {
            std::this_thread::sleep_for (100ms);
            ++cnt;
            if (cnt > 30)
            {
                break;
            }
        }
        BOOST_CHECK_GE (counter, 1);
        if(!msgs.empty()) {
        	auto rM = msgs.at(0);
        	BOOST_CHECK_EQUAL (rM.name, "core1");
        	std::cout << "rM.name: " << rM.name << std::endl;
        	BOOST_CHECK (rM.action () == helics::action_message_def::action_t::cmd_protocol);
        }

        cnt = 0;
        while (counter == 1)
		{
			std::this_thread::sleep_for (100ms);
			++cnt;
			if (cnt > 30)
			{
				break;
			}
		}
        BOOST_CHECK_GE (counter, 2);
        if(!msgs.empty()) {
            auto rM2 = msgs.at(1);
            BOOST_CHECK_EQUAL (rM2.name, "core1");
            std::cout << "rM.name: " << rM2.name << std::endl;
        	BOOST_CHECK (rM2.action () == helics::action_message_def::action_t::cmd_reg_broker);
        }
    }
    core->disconnect ();
    comm.disconnect ();
    core = nullptr;
    msgs.clear();
    helics::CoreFactory::cleanUpCores (100ms);
}

/** test case checks default values and makes sure they all mesh together
also tests the automatic port determination for cores
*/
BOOST_AUTO_TEST_CASE (zmqSSCore_core_broker_default_test)
{
    std::string initializationString = "-f 1";

    auto broker = helics::BrokerFactory::create (helics::core_type::ZMQ_SS, initializationString);

    auto core = helics::CoreFactory::create (helics::core_type::ZMQ_SS, initializationString);
    bool connected = broker->isConnected ();
    BOOST_CHECK (connected);
    connected = core->connect ();
    BOOST_CHECK (connected);

    core->disconnect ();

    BOOST_CHECK (!core->isConnected ());
    broker->disconnect ();
    BOOST_CHECK (!broker->isConnected ());
    helics::CoreFactory::cleanUpCores (200ms);
    helics::BrokerFactory::cleanUpBrokers (200ms);
}

class FedTest
{
  public:
    helics::Time deltaTime = helics::Time (10, timeUnits::ns);  // sampling rate
    helics::Time finalTime = helics::Time (200, timeUnits::ns);  // final time
  private:
    std::unique_ptr<helics::ValueFederate> vFed;
    helics::Publication pub;
    helics::Input sub;
    std::string fed_name;

    int pub_index = 0;
    int sub_index = 0;
    bool initialized = false;

  public:
    FedTest () = default;

    void run ()
    {
        if (!initialized)
        {
            throw ("must initialize first");
        }
        vFed->enterInitializingMode ();
        vFed->enterExecutingMode ();
        mainLoop ();
    };
    void initialize (const std::string &coreName, int p_index, int s_index)
    {
        fed_name = "fedtest_" + std::to_string (p_index);
        pub_index = p_index;
        sub_index = s_index;
        helics::FederateInfo fi;
        fi.coreName = coreName;
        vFed = std::make_unique<helics::ValueFederate> (fed_name, fi);
        pub = vFed->registerPublicationIndexed<std::string> ("fedrx", pub_index);
        sub = vFed->registerSubscriptionIndexed ("fedrx", pub_index);
        initialized = true;
    }

    void mainLoop ()
    {
        auto nextTime = deltaTime;
        std::string txstring (fed_name);
        while (nextTime < finalTime)
        {
            nextTime = vFed->requestTime (nextTime + deltaTime);
            vFed->publish (pub, txstring);

            if (vFed->isUpdated (sub))
            {
            	if (vFed->isUpdated(sub)) {
            		// get the latest value for the subscription
                    auto &nstring = vFed->getString (sub);
                    if (nstring != txstring)
                    {
                        std::cout << "incorrect string\n";
                        break;
                    }
            	}

            }
        }
        vFed->finalize ();
    }
};

BOOST_AUTO_TEST_CASE (zmqSSMultiCoreInitialization_test)
{
	int feds = 20;
	auto broker = helics::BrokerFactory::create (helics::core_type::ZMQ_SS, "ZMQ_SS_broker", std::to_string (feds));
	std::vector<std::shared_ptr<helics::Core>> cores (feds);
	std::vector<FedTest> leafs (feds);

	for (int ii = 0; ii < feds; ++ii)
	{
		std::string initializationString = "-f 1 --name=core" + std::to_string(ii);
		cores[ii] = helics::CoreFactory::create (helics::core_type::ZMQ_SS, initializationString);
		cores[ii]->connect ();
		int s_index = ii+1;
		if(ii == feds - 1) {
			s_index = 0;
		}
		leafs[ii].initialize (cores[ii]->getIdentifier (), ii, s_index);
	}
	std::this_thread::sleep_for (std::chrono::milliseconds (100));
    std::vector<std::thread> threads (feds);
    for (int ii = 0; ii < feds; ++ii)
    {
        threads[ii] = std::thread ([](FedTest &leaf) { leaf.run (); }, std::ref (leafs[ii]));
    }
    std::this_thread::yield ();
    std::this_thread::sleep_for (std::chrono::milliseconds (500));
    std::this_thread::yield ();
    for (auto &thrd : threads)
    {
        thrd.join ();
    }
    std::this_thread::sleep_for (std::chrono::milliseconds (1000));
    broker->disconnect ();
    std::this_thread::sleep_for (std::chrono::milliseconds (1000));
}

BOOST_AUTO_TEST_SUITE_END ()
