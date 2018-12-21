/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include <boost/test/unit_test.hpp>

#include "helics/common/AsioServiceManager.h"
#include "helics/common/GuardedTypes.hpp"
#include "helics/core/ActionMessage.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/core-types.hpp"
#include "helics/core/udp/UdpBroker.h"
#include "helics/core/udp/UdpComms.h"
#include "helics/core/udp/UdpCore.h"
#include <boost/asio/ip/udp.hpp>

//#include "boost/process.hpp"
#include <future>

namespace utf = boost::unit_test;
using namespace std::literals::chrono_literals;

BOOST_AUTO_TEST_SUITE (UdpCore_tests, *utf::label ("ci"))

using boost::asio::ip::udp;
using helics::Core;

#define UDP_BROKER_PORT 23901
#define UDP_SECONDARY_PORT 23905
BOOST_AUTO_TEST_CASE (udpComms_broker_test)
{
    std::atomic<int> counter{0};
    std::string host = "localhost";
    helics::udp::UdpComms comm;
    comm.loadTargetInfo (host, host);

    auto srv = AsioServiceManager::getServicePointer ();

    udp::socket rxSocket (AsioServiceManager::getService (), udp::endpoint (udp::v4 (), 23901));

    comm.setCallback ([&counter](helics::ActionMessage /*m*/) { ++counter; });
    comm.setBrokerPort (UDP_BROKER_PORT);
    comm.setName ("tests");
    auto confut = std::async (std::launch::async, [&comm]() { return comm.connect (); });

    std::vector<char> data (1024);

    udp::endpoint remote_endpoint;
    boost::system::error_code error;
    auto len = rxSocket.receive_from (boost::asio::buffer (data), remote_endpoint, 0, error);

    BOOST_CHECK (!error);
    BOOST_CHECK_GT (len, 32);

    helics::ActionMessage rM (data.data (), len);
    BOOST_CHECK (helics::isProtocolCommand (rM));
    rM.messageID = DISCONNECT;
    rxSocket.send_to (boost::asio::buffer (rM.to_string ()), remote_endpoint, 0, error);
    BOOST_CHECK (!error);
    auto connected = confut.get ();
    BOOST_CHECK (!connected);
    rxSocket.close ();
    comm.disconnect ();
    std::this_thread::sleep_for (100ms);
}

BOOST_AUTO_TEST_CASE (udpComms_broker_test_transmit)
{
    std::this_thread::sleep_for (std::chrono::milliseconds (500));
    std::atomic<int> counter{0};
    std::string host = "localhost";
    helics::udp::UdpComms comm;
    comm.loadTargetInfo (host, host);
    auto srv = AsioServiceManager::getServicePointer ();

    udp::socket rxSocket (AsioServiceManager::getService (), udp::endpoint (udp::v4 (), 23901));

    BOOST_CHECK (rxSocket.is_open ());
    comm.setCallback ([&counter](helics::ActionMessage /*m*/) { ++counter; });
    comm.setBrokerPort (UDP_BROKER_PORT);
    comm.setPortNumber (UDP_SECONDARY_PORT);
    comm.setName ("tests");
    bool connected = comm.connect ();
    BOOST_REQUIRE (connected);
    comm.transmit (helics::parent_route_id, helics::CMD_IGNORE);

    std::vector<char> data (1024);

    udp::endpoint remote_endpoint;
    boost::system::error_code error;
    auto len = rxSocket.receive_from (boost::asio::buffer (data), remote_endpoint, 0, error);

    BOOST_CHECK_GT (len, 32);
    helics::ActionMessage rM (data.data (), len);
    BOOST_CHECK (rM.action () == helics::action_message_def::action_t::cmd_ignore);
    rxSocket.close ();
    comm.disconnect ();
    std::this_thread::sleep_for (100ms);
}

BOOST_AUTO_TEST_CASE (udpComms_rx_test)
{
    std::this_thread::sleep_for (std::chrono::milliseconds (500));
    std::atomic<int> counter{0};
    guarded<helics::ActionMessage> act;
    std::string host = "localhost";
    helics::udp::UdpComms comm;
    comm.loadTargetInfo (host, host);

    auto srv = AsioServiceManager::getServicePointer ();

    udp::resolver resolver (AsioServiceManager::getService ());
    udp::socket rxSocket (AsioServiceManager::getService (), udp::endpoint (udp::v4 (), 23901));

    BOOST_CHECK (rxSocket.is_open ());
    comm.setCallback ([&counter, &act](helics::ActionMessage m) {
        ++counter;
        act = m;
    });
    comm.setBrokerPort (UDP_BROKER_PORT);
    comm.setPortNumber (23903);
    comm.setName ("tests");

    bool connected = comm.connect ();
    BOOST_REQUIRE (connected);

    udp::resolver::query queryNew (udp::v4 (), "localhost", "23903");

    auto txendpoint = *resolver.resolve (queryNew);

    helics::ActionMessage cmd (helics::CMD_ACK);
    std::string buffer = cmd.to_string ();

    auto cnt = rxSocket.send_to (boost::asio::buffer (buffer), txendpoint);
    BOOST_REQUIRE_EQUAL (cnt, buffer.size ());

    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    BOOST_REQUIRE_EQUAL (counter, 1);
    BOOST_CHECK (act.lock ()->action () == helics::action_message_def::action_t::cmd_ack);
    rxSocket.close ();
    comm.disconnect ();
    std::this_thread::sleep_for (100ms);
}

BOOST_AUTO_TEST_CASE (udpComm_transmit_through)
{
    std::this_thread::sleep_for (std::chrono::milliseconds (500));
    std::atomic<int> counter{0};
    std::atomic<int> counter2{0};
    guarded<helics::ActionMessage> act;
    guarded<helics::ActionMessage> act2;

    std::string host = "localhost";
    helics::udp::UdpComms comm;
    comm.loadTargetInfo (host, host);
    helics::udp::UdpComms comm2;
    comm2.loadTargetInfo (host, "");

    comm.setBrokerPort (UDP_BROKER_PORT);
    comm.setName ("tests");
    comm2.setName ("test2");
    comm2.setPortNumber (UDP_BROKER_PORT);
    comm.setPortNumber (UDP_SECONDARY_PORT);

    comm.setCallback ([&counter, &act](helics::ActionMessage m) {
        ++counter;
        act = m;
    });
    comm2.setCallback ([&counter2, &act2](helics::ActionMessage m) {
        ++counter2;
        act2 = m;
    });

    // need to launch the connection commands at the same time since they depend on eachother in this case
    auto connected_fut = std::async (std::launch::async, [&comm] { return comm.connect (); });

    bool connected = comm2.connect ();
    BOOST_REQUIRE (connected);
    connected = connected_fut.get ();
    BOOST_REQUIRE (connected);

    comm.transmit (helics::parent_route_id, helics::CMD_ACK);

    std::this_thread::sleep_for (250ms);
    if (counter2 != 1)
    {
        std::this_thread::sleep_for (500ms);
    }
    BOOST_REQUIRE_EQUAL (counter2, 1);
    BOOST_CHECK (act2.lock ()->action () == helics::action_message_def::action_t::cmd_ack);

    comm.disconnect ();
    comm2.disconnect ();
    std::this_thread::sleep_for (100ms);
}

BOOST_AUTO_TEST_CASE (udpComm_transmit_add_route)
{
    std::this_thread::sleep_for (std::chrono::milliseconds (500));
    std::atomic<int> counter{0};
    std::atomic<int> counter2{0};
    std::atomic<int> counter3{0};

    std::string host = "localhost";
    helics::udp::UdpComms comm, comm2, comm3;
    comm.loadTargetInfo (host, host);
    comm2.loadTargetInfo (host, "");
    comm3.loadTargetInfo (host, host);

    comm.setBrokerPort (UDP_BROKER_PORT);
    comm.setName ("tests");
    comm2.setName ("broker");
    comm3.setName ("test3");
    comm3.setBrokerPort (UDP_BROKER_PORT);

    comm2.setPortNumber (UDP_BROKER_PORT);
    comm.setPortNumber (UDP_SECONDARY_PORT);
    comm3.setPortNumber (23920);

    guarded<helics::ActionMessage> act;
    guarded<helics::ActionMessage> act2;
    guarded<helics::ActionMessage> act3;

    comm.setCallback ([&counter, &act](helics::ActionMessage &&m) {
        ++counter;
        act = std::move(m);
    });
    comm2.setCallback ([&counter2, &act2](helics::ActionMessage &&m) {
        ++counter2;
        act2 = std::move(m);
    });
    comm3.setCallback ([&counter3, &act3](helics::ActionMessage &&m) {
        ++counter3;
        act3 = std::move(m);
    });

    // need to launch the connection commands at the same time since they depend on eachother in this case
    // auto connected_fut = std::async(std::launch::async, [&comm] {return comm.connect(); });

    bool connected = comm2.connect ();
    BOOST_REQUIRE (connected);
    // connected = connected_fut.get();
    connected = comm.connect ();
    BOOST_REQUIRE (connected);
    connected = comm3.connect ();

    comm.transmit (helics::route_id (0), helics::CMD_ACK);

    std::this_thread::sleep_for (250ms);
    BOOST_REQUIRE_EQUAL (counter2, 1);
    BOOST_CHECK (act2.lock ()->action () == helics::action_message_def::action_t::cmd_ack);

    comm3.transmit (helics::route_id (0), helics::CMD_ACK);

    std::this_thread::sleep_for (250ms);
    BOOST_REQUIRE_EQUAL (counter2, 2);
    BOOST_CHECK (act2.lock ()->action () == helics::action_message_def::action_t::cmd_ack);

    comm2.addRoute (helics::route_id (3), comm3.getAddress ());
    std::this_thread::sleep_for (250ms);
    comm2.transmit (helics::route_id (3), helics::CMD_ACK);

    std::this_thread::sleep_for (250ms);
    if (counter3 != 1)
    {
        std::this_thread::sleep_for (250ms);
    }
    BOOST_REQUIRE_EQUAL (counter3, 1);
    BOOST_CHECK (act3.lock ()->action () == helics::action_message_def::action_t::cmd_ack);

    comm2.addRoute (helics::route_id (4), comm.getAddress ());

    comm2.transmit (helics::route_id (4), helics::CMD_ACK);

    std::this_thread::sleep_for (250ms);
    BOOST_REQUIRE_EQUAL (counter, 1);
    BOOST_CHECK (act.lock ()->action () == helics::action_message_def::action_t::cmd_ack);

    comm.disconnect ();
    comm2.disconnect ();
    comm3.disconnect ();
    std::this_thread::sleep_for (100ms);
}

BOOST_AUTO_TEST_CASE (udpCore_initialization_test)
{
    std::this_thread::sleep_for (std::chrono::milliseconds (500));
    std::string initializationString =
      "-f 1 --brokerport=23901  --port=23950 --local_interface=localhost --name=core1";
    auto core = helics::CoreFactory::create (helics::core_type::UDP, initializationString);

    BOOST_REQUIRE (core != nullptr);
    BOOST_CHECK (core->isInitialized ());
    auto srv = AsioServiceManager::getServicePointer ();
    udp::socket rxSocket (AsioServiceManager::getService (), udp::endpoint (udp::v4 (), 23901));

    BOOST_CHECK (rxSocket.is_open ());

    bool connected = core->connect ();
    BOOST_REQUIRE (connected);

    std::vector<char> data (1024);

    udp::endpoint remote_endpoint;
    boost::system::error_code error;

    auto len = rxSocket.receive_from (boost::asio::buffer (data), remote_endpoint, 0, error);
    BOOST_REQUIRE (!error);
    BOOST_CHECK_GT (len, 32);
    helics::ActionMessage rM (data.data (), len);

    BOOST_CHECK_EQUAL (rM.name, "core1");
    BOOST_CHECK (rM.action () == helics::action_message_def::action_t::cmd_reg_broker);
    helics::ActionMessage resp (helics::CMD_PRIORITY_ACK);
    rxSocket.send_to (boost::asio::buffer (resp.to_string ()), remote_endpoint, 0, error);
    BOOST_CHECK (!error);
    core->disconnect ();
    core = nullptr;
    helics::CoreFactory::cleanUpCores (100ms);
}

/** test case checks default values and makes sure they all mesh together
also tests the automatic port determination for cores
*/

BOOST_AUTO_TEST_CASE (udpCore_core_broker_default_test)
{
    std::this_thread::sleep_for (500ms);
    std::string initializationString = "-f 1";

    auto broker = helics::BrokerFactory::create (helics::core_type::UDP, initializationString);

    auto core = helics::CoreFactory::create (helics::core_type::UDP, initializationString);
    bool connected = broker->isConnected ();
    BOOST_CHECK (connected);
    connected = core->connect ();
    BOOST_CHECK (connected);

    auto ccore = static_cast<helics::udp::UdpCore *> (core.get ());
    // this will test the automatic port allocation
    BOOST_CHECK_EQUAL (ccore->getAddress (), "localhost:23961");
    core->disconnect ();
    broker->disconnect ();
    core = nullptr;
    broker = nullptr;
    helics::CoreFactory::cleanUpCores (100ms);
    helics::BrokerFactory::cleanUpBrokers (100ms);
}

BOOST_AUTO_TEST_SUITE_END ()
