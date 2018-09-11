/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include <boost/test/unit_test.hpp>

#include "helics/core/ActionMessage.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/CoreBroker.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/core-types.hpp"
#include "helics/core/ipc/IpcComms.h"
#include "helics/core/ipc/IpcCore.h"

#include <boost/interprocess/ipc/message_queue.hpp>

#include "helics/core/ipc/IpcQueueHelper.h"
#include "helics/common/GuardedTypes.hpp"

//#include "boost/process.hpp"
#include <future>

namespace utf = boost::unit_test;
using namespace std::literals::chrono_literals;

BOOST_AUTO_TEST_SUITE (IPCCore_tests, *utf::label("ci"))

using helics::Core;

BOOST_AUTO_TEST_CASE (ipccomms_broker_test)
{
    std::atomic<int> counter{0};
    std::string brokerLoc = "brokerIPC";
    std::string localLoc = "localIPC";
    helics::ipc::IpcComms comm (localLoc, brokerLoc);

    helics::ipc::ownedQueue mq;
    bool mqConn = mq.connect (brokerLoc, 1024, 1024);
    BOOST_REQUIRE (mqConn);

    comm.setCallback ([&counter](helics::ActionMessage /*m*/) { ++counter; });

    bool connected = comm.connect ();
    BOOST_REQUIRE (connected);
    comm.transmit (0, helics::CMD_IGNORE);

    helics::ActionMessage rM = mq.getMessage ();
    BOOST_CHECK (rM.action () == helics::action_message_def::action_t::cmd_ignore);
    comm.disconnect ();
    std::this_thread::sleep_for (100ms);
}

BOOST_AUTO_TEST_CASE (ipccomms_rx_test)
{
    std::atomic<int> counter{0};
    guarded<helics::ActionMessage> act;
    std::string brokerLoc = "";
    std::string localLoc = "localIPC";
    helics::ipc::IpcComms comm (localLoc, brokerLoc);

    comm.setCallback ([&counter, &act](helics::ActionMessage m) {
        ++counter;
        act = m;
    });

    bool connected = comm.connect ();
    BOOST_REQUIRE (connected);
    helics::ipc::sendToQueue mq;
    mq.connect (localLoc, true, 2);

    helics::ActionMessage cmd (helics::CMD_ACK);

    mq.sendMessage (cmd, 1);
    std::this_thread::sleep_for (250ms);
    BOOST_REQUIRE_EQUAL (counter, 1);
    BOOST_CHECK (act.lock()->action () == helics::action_message_def::action_t::cmd_ack);
    comm.disconnect ();
    std::this_thread::sleep_for (100ms);
}

BOOST_AUTO_TEST_CASE (ipcComm_transmit_through)
{
    std::atomic<int> counter{0};
    std::string brokerLoc = "brokerIPC";
    std::string localLoc = "localIPC";
    // just to make sure these are not already present from a failure
    std::atomic<int> counter2{0};
    guarded<helics::ActionMessage> act;
    guarded<helics::ActionMessage> act2;

    helics::ipc::IpcComms comm (localLoc, brokerLoc);
    helics::ipc::IpcComms comm2 (brokerLoc, "");

    comm.setCallback ([&counter, &act](helics::ActionMessage m) {
        ++counter;
        act = m;
    });
    comm2.setCallback ([&counter2, &act2](helics::ActionMessage m) {
        ++counter2;
        act2 = m;
    });

    // need to launch the connection commands at the same time since they depend on eachother in this case
    // auto connected_fut = std::async(std::launch::async, [&comm] {return comm.connect(); });

    bool connected = comm2.connect ();
    BOOST_REQUIRE (connected);
    // connected = connected_fut.get();
    connected = comm.connect ();
    BOOST_REQUIRE (connected);

    comm.transmit (0, helics::CMD_ACK);

    std::this_thread::sleep_for (250ms);
    BOOST_REQUIRE_EQUAL (counter2, 1);
    BOOST_CHECK (act2.lock()->action () == helics::action_message_def::action_t::cmd_ack);

    comm.disconnect ();
    comm2.disconnect ();
    std::this_thread::sleep_for (100ms);
}

BOOST_AUTO_TEST_CASE (ipcComm_transmit_add_route)
{
    std::atomic<int> counter{0};
    std::string brokerLoc = "brokerIPC";
    std::string localLoc = "localIPC";
    std::string localLocB = "localIPC2";
    // just to make sure these are not already present from a failure
    boost::interprocess::message_queue::remove (brokerLoc.c_str ());
    boost::interprocess::message_queue::remove (localLoc.c_str ());
    boost::interprocess::message_queue::remove (localLocB.c_str ());

    std::atomic<int> counter2{0};
    std::atomic<int> counter3{0};
    guarded<helics::ActionMessage> act;
    guarded<helics::ActionMessage> act2;
    guarded<helics::ActionMessage> act3;

    helics::ipc::IpcComms comm (localLoc, brokerLoc);
    helics::ipc::IpcComms comm2 (brokerLoc, "");
    helics::ipc::IpcComms comm3 (localLocB, brokerLoc);

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
    std::this_thread::sleep_for (100ms);
    // need to launch the connection commands at the same time since they depend on eachother in this case
    // auto connected_fut = std::async(std::launch::async, [&comm] {return comm.connect(); });

    bool connected = comm2.connect ();
    BOOST_REQUIRE (connected);
    // connected = connected_fut.get();
    connected = comm.connect ();
    BOOST_REQUIRE (connected);
    connected = comm3.connect ();
    std::this_thread::sleep_for (std::chrono::milliseconds (100));
    comm.transmit (0, helics::CMD_ACK);

    std::this_thread::sleep_for (std::chrono::milliseconds (100));
    if (counter2 != 1)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (350));
    }
    BOOST_REQUIRE_EQUAL (counter2, 1);
    BOOST_CHECK (act2.lock()->action () == helics::action_message_def::action_t::cmd_ack);

    comm3.transmit (0, helics::CMD_ACK);

    std::this_thread::sleep_for (std::chrono::milliseconds (100));
    if (counter2 != 1)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (350));
    }
    BOOST_REQUIRE_EQUAL (counter2, 2);
    BOOST_CHECK (act2.lock()->action () == helics::action_message_def::action_t::cmd_ack);

    comm2.addRoute (3, localLocB);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    comm2.transmit (3, helics::CMD_ACK);

    std::this_thread::sleep_for (std::chrono::milliseconds (100));
    if (counter3 != 1)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (350));
    }
    if (counter3 != 1)
    {
        std::cout << "ipc core extra sleep required\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(350));
    }
    BOOST_REQUIRE_EQUAL (counter3, 1);
    BOOST_CHECK (act3.lock()->action () == helics::action_message_def::action_t::cmd_ack);

    comm2.addRoute (4, localLoc);
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    comm2.transmit (4, helics::CMD_ACK);

    std::this_thread::sleep_for (std::chrono::milliseconds (100));
    if (counter.load () != 1)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (350));
    }
    BOOST_REQUIRE_EQUAL (counter.load (), 1);
    BOOST_CHECK (act.lock()->action () == helics::action_message_def::action_t::cmd_ack);

    comm.disconnect ();
    comm2.disconnect ();
    comm3.disconnect ();
    std::this_thread::sleep_for (std::chrono::milliseconds (100));
}

BOOST_AUTO_TEST_CASE (ipccore_initialization_test)
{
    std::string initializationString = "1 --broker_address=testBroker --name=core1";
    auto core = helics::CoreFactory::create (helics::core_type::INTERPROCESS, initializationString);

    BOOST_REQUIRE (core != nullptr);
    BOOST_CHECK (core->isInitialized ());

    helics::ipc::ownedQueue mq;
    bool mqConn = mq.connect ("testBroker", 1024, 1024);
    BOOST_REQUIRE (mqConn);

    bool crConn = core->connect ();
    BOOST_REQUIRE (crConn);

    helics::ActionMessage rM = mq.getMessage ();
    BOOST_CHECK_EQUAL (rM.name, "core1");
    BOOST_CHECK (rM.action () == helics::action_message_def::action_t::cmd_reg_broker);
    core->disconnect ();
    core = nullptr;
    boost::interprocess::message_queue::remove ("testbroker");
    helics::CoreFactory::cleanUpCores (100ms);
}

/** test case checks default values and makes sure they all mesh together
also tests the automatic port determination for cores
*/
BOOST_AUTO_TEST_CASE (ipcCore_core_broker_default_test)
{
    std::string initializationString = "1";

    auto broker = helics::BrokerFactory::create (helics::core_type::INTERPROCESS, initializationString);

    auto core = helics::CoreFactory::create (helics::core_type::INTERPROCESS, initializationString);
    bool connected = broker->isConnected ();
    BOOST_CHECK (connected);
    connected = core->connect ();
    BOOST_CHECK (connected);

    // auto ccore = static_cast<helics::IpcCore *>(core.get());
    // this will test the automatic port allocation
    // BOOST_CHECK_EQUAL(ccore->getAddress(), "tcp://127.0.0.1:23500;tcp://127.0.0.1:23501");
    core->disconnect ();
    broker->disconnect ();
    core = nullptr;
    broker = nullptr;
    helics::CoreFactory::cleanUpCores (100ms);
    helics::BrokerFactory::cleanUpBrokers (100ms);
}

BOOST_AUTO_TEST_SUITE_END ()
