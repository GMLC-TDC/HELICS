/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include <boost/test/unit_test.hpp>

#include "helics/common/AsioServiceManager.h"
#include "helics/core/ActionMessage.h"
#include "helics/core/BrokerFactory.h"
#include "helics/core/CoreFactory.h"
#include "helics/core/core-types.h"
#include "helics/core/core.h"
#include "helics/core/udp/UdpBroker.h"
#include "helics/core/udp/UdpComms.h"
#include "helics/core/udp/UdpCore.h"
#include <boost/asio/ip/udp.hpp>

//#include "boost/process.hpp"
#include <future>

BOOST_AUTO_TEST_SUITE (UDPCore_tests)

using boost::asio::ip::udp;
using helics::Core;

#define UDP_BROKER_PORT 23901
#define UDP_SECONDARY_PORT 23905
BOOST_AUTO_TEST_CASE (udpComms_broker_test)
{
    std::atomic<int> counter{0};
    std::string host = "localhost";
    helics::UdpComms comm (host, host);

    auto srv = AsioServiceManager::getServicePointer ();

    udp::socket rxSocket (AsioServiceManager::getService (), udp::endpoint (udp::v4 (), 23901));

    comm.setCallback ([&counter](helics::ActionMessage m) { ++counter; });
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
    rM.index = DISCONNECT;
    rxSocket.send_to (boost::asio::buffer (rM.to_string ()), remote_endpoint, 0, error);
    BOOST_CHECK (!error);
    auto connected = confut.get ();
    BOOST_CHECK (!connected);
    rxSocket.close ();
    comm.disconnect ();
    std::this_thread::sleep_for (std::chrono::milliseconds (100));
}

BOOST_AUTO_TEST_CASE (udpComms_broker_test_transmit)
{
    std::atomic<int> counter{0};
    std::string host = "localhost";
    helics::UdpComms comm (host, host);

    auto srv = AsioServiceManager::getServicePointer ();

    udp::socket rxSocket (AsioServiceManager::getService (), udp::endpoint (udp::v4 (), 23901));

    BOOST_CHECK (rxSocket.is_open ());
    comm.setCallback ([&counter](helics::ActionMessage m) { ++counter; });
    comm.setBrokerPort (UDP_BROKER_PORT);
    comm.setPortNumber (UDP_SECONDARY_PORT);
    comm.setName ("tests");
    bool connected = comm.connect ();
    BOOST_REQUIRE (connected);
    comm.transmit (0, helics::CMD_IGNORE);

    std::vector<char> data (1024);

    udp::endpoint remote_endpoint;
    boost::system::error_code error;
    auto len = rxSocket.receive_from (boost::asio::buffer (data), remote_endpoint, 0, error);

    BOOST_CHECK_GT (len, 32);
    helics::ActionMessage rM (data.data (), len);
    BOOST_CHECK (rM.action () == helics::action_message_def::action_t::cmd_ignore);
    rxSocket.close ();
    comm.disconnect ();
    std::this_thread::sleep_for (std::chrono::milliseconds (100));
}

BOOST_AUTO_TEST_CASE (udpComms_rx_test)
{
    std::atomic<int> counter{0};
    helics::ActionMessage act;
    std::string host = "localhost";
    helics::UdpComms comm (host, host);

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
    BOOST_CHECK (act.action () == helics::action_message_def::action_t::cmd_ack);
    rxSocket.close ();
    comm.disconnect ();
    std::this_thread::sleep_for (std::chrono::milliseconds (100));
}

BOOST_AUTO_TEST_CASE (udpComm_transmit_through)
{
    std::atomic<int> counter{0};
    std::atomic<int> counter2{0};
    helics::ActionMessage act;
    helics::ActionMessage act2;

    std::string host = "localhost";
    helics::UdpComms comm (host, host);
    helics::UdpComms comm2 (host, "");

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

    comm.transmit (0, helics::CMD_ACK);

    std::this_thread::sleep_for (std::chrono::milliseconds (250));
    if (counter2 != 1)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (500));
    }
    BOOST_REQUIRE_EQUAL (counter2, 1);
    BOOST_CHECK (act2.action () == helics::action_message_def::action_t::cmd_ack);

    comm.disconnect ();
    comm2.disconnect ();
    std::this_thread::sleep_for (std::chrono::milliseconds (100));
}

BOOST_AUTO_TEST_CASE (udpComm_transmit_add_route)
{
    std::atomic<int> counter{0};
    std::atomic<int> counter2{0};
    std::atomic<int> counter3{0};

    std::string host = "localhost";
    helics::UdpComms comm (host, host);
    helics::UdpComms comm2 (host, "");
    helics::UdpComms comm3 (host, host);

    comm.setBrokerPort (UDP_BROKER_PORT);
    comm.setName ("tests");
    comm2.setName ("broker");
    comm3.setName ("test3");
    comm3.setBrokerPort (UDP_BROKER_PORT);

    comm2.setPortNumber (UDP_BROKER_PORT);
    comm.setPortNumber (UDP_SECONDARY_PORT);
    comm3.setPortNumber (23920);

    helics::ActionMessage act;
    helics::ActionMessage act2;
    helics::ActionMessage act3;

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

    // need to launch the connection commands at the same time since they depend on eachother in this case
    // auto connected_fut = std::async(std::launch::async, [&comm] {return comm.connect(); });

    bool connected = comm2.connect ();
    BOOST_REQUIRE (connected);
    // connected = connected_fut.get();
    connected = comm.connect ();
    BOOST_REQUIRE (connected);
    connected = comm3.connect ();

    comm.transmit (0, helics::CMD_ACK);

    std::this_thread::sleep_for (std::chrono::milliseconds (250));
    BOOST_REQUIRE_EQUAL (counter2, 1);
    BOOST_CHECK (act2.action () == helics::action_message_def::action_t::cmd_ack);

    comm3.transmit (0, helics::CMD_ACK);

    std::this_thread::sleep_for (std::chrono::milliseconds (250));
    BOOST_REQUIRE_EQUAL (counter2, 2);
    BOOST_CHECK (act2.action () == helics::action_message_def::action_t::cmd_ack);

    comm2.addRoute (3, comm3.getAddress ());

    comm2.transmit (3, helics::CMD_ACK);

    std::this_thread::sleep_for (std::chrono::milliseconds (250));
    if (counter3 != 1)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (250));
    }
    BOOST_REQUIRE_EQUAL (counter3, 1);
    BOOST_CHECK (act3.action () == helics::action_message_def::action_t::cmd_ack);

    comm2.addRoute (4, comm.getAddress ());

    comm2.transmit (4, helics::CMD_ACK);

    std::this_thread::sleep_for (std::chrono::milliseconds (250));
    BOOST_REQUIRE_EQUAL (counter, 1);
    BOOST_CHECK (act.action () == helics::action_message_def::action_t::cmd_ack);

    comm.disconnect ();
    comm2.disconnect ();
    comm3.disconnect ();
    std::this_thread::sleep_for (std::chrono::milliseconds (100));
}

BOOST_AUTO_TEST_CASE (udpCore_initialization_test)
{
    std::string initializationString =
      "1 --brokerport=23901  --port=23950 --local_interface=localhost --name=core1";
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
    helics::CoreFactory::cleanUpCores (100);
}

/** test case checks default values and makes sure they all mesh together
also tests the automatic port determination for cores
*/

BOOST_AUTO_TEST_CASE (udpCore_core_broker_default_test)
{
    std::string initializationString = "1";

    auto broker = helics::BrokerFactory::create (helics::core_type::UDP, initializationString);

    auto core = helics::CoreFactory::create (helics::core_type::UDP, initializationString);
    bool connected = broker->isConnected ();
    BOOST_CHECK (connected);
    connected = core->connect ();
    BOOST_CHECK (connected);

    auto ccore = static_cast<helics::UdpCore *> (core.get ());
    // this will test the automatic port allocation
    BOOST_CHECK_EQUAL (ccore->getAddress (), "localhost:23964");
    core->disconnect ();
    broker->disconnect ();
    core = nullptr;
    broker = nullptr;
    helics::CoreFactory::cleanUpCores (100);
    helics::BrokerFactory::cleanUpBrokers (100);
}

BOOST_AUTO_TEST_SUITE_END ()