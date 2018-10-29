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
#include "helics/core/tcp/TcpBroker.h"
#include "helics/core/tcp/TcpCommsSS.h"
#include "helics/core/tcp/TcpCore.h"
#include "helics/core/tcp/TcpHelperClasses.h"

#include <numeric>

#include <future>

namespace utf = boost::unit_test;
using namespace std::literals::chrono_literals;

BOOST_AUTO_TEST_SUITE (TcpSSCore_tests, *utf::label ("cic"))

using boost::asio::ip::tcp;
using helics::Core;

#define TCP_BROKER_PORT 33133
#define TCP_BROKER_PORT_STRING "33133"

#define TCP_BROKER_PORT_ALT 33134
#define TCP_BROKER_PORT_ALT_STRING "33134"

BOOST_AUTO_TEST_CASE (tcpSSComms_broker_test)
{
    std::atomic<int> counter{0};
    std::string host = "localhost";
    helics::tcp::TcpCommsSS comm;
    comm.loadTargetInfo (host, host);

    auto srv = AsioServiceManager::getServicePointer ();

    auto server = helics::tcp::TcpServer::create (srv->getBaseService (), TCP_BROKER_PORT);
    auto serviceLoop = srv->runServiceLoop ();
    std::vector<char> data (1024);
    server->setDataCall ([&counter](helics::tcp::TcpConnection::pointer, const char *, size_t data_avail) {
        ++counter;
        return data_avail;
    });
    server->start ();

    comm.setCallback ([&counter](helics::ActionMessage /*m*/) { ++counter; });
    comm.setBrokerPort (TCP_BROKER_PORT);
    comm.setName ("tests");
    comm.setTimeout (1000ms);
    comm.setServerMode (false);
    auto confut = std::async (std::launch::async, [&comm]() { return comm.connect (); });

    std::this_thread::sleep_for (100ms);
    int cnt = 0;
    while (counter < 1)
    {
        std::this_thread::sleep_for (100ms);
        ++cnt;
        if (cnt > 30)
        {
            break;
        }
    }
    BOOST_CHECK_EQUAL (counter, 1);

    server->close ();
    comm.disconnect ();
    std::this_thread::sleep_for (100ms);
}

BOOST_AUTO_TEST_CASE (tcpSSComms_broker_test_transmit)
{
    std::this_thread::sleep_for (400ms);
    std::atomic<int> counter{0};
    std::atomic<size_t> len{0};
    std::string host = "localhost";
    helics::tcp::TcpCommsSS comm;
    comm.loadTargetInfo (host, host);

    auto srv = AsioServiceManager::getServicePointer ();
    auto serviceLoop = srv->runServiceLoop();

    auto server = helics::tcp::TcpServer::create (srv->getBaseService (), host, TCP_BROKER_PORT);
    
    std::vector<char> data (1024);
    server->setDataCall (
      [&data, &counter, &len](helics::tcp::TcpConnection::pointer, const char *data_rec, size_t data_Size) {
          std::copy (data_rec, data_rec + data_Size, data.begin ());
          len = data_Size;
          ++counter;
          return data_Size;
      });
    BOOST_REQUIRE (server->isReady ());
    auto res=server->start ();
    BOOST_CHECK(res);
    std::this_thread::sleep_for(100ms);
    comm.setCallback ([](helics::ActionMessage /*m*/) {});
    comm.setBrokerPort (TCP_BROKER_PORT);
    comm.setName ("tests");
    comm.setServerMode (false);
    bool connected = comm.connect ();
    BOOST_REQUIRE (connected);
    comm.transmit (helics::parent_route_id, helics::CMD_IGNORE);

    boost::system::error_code error;
    int cnt = 0;
    while (counter < 2)
    {
        if (len > 130)
        {
            break;
        }
        std::this_thread::sleep_for (100ms);
        ++cnt;
        if (cnt > 30)
        {
            break;
        }
    }
    BOOST_CHECK_GE (counter, 1);

    BOOST_CHECK_GT (len, 50);
    helics::ActionMessage rM;
    auto loc=rM.depacketize(data.data(), len);
    if ((counter == 1) && (loc < len))
    {
        rM.depacketize(data.data() + loc, len - loc);
    }
    BOOST_CHECK (rM.action () == helics::action_message_def::action_t::cmd_ignore);
    server->close ();
    comm.disconnect ();
    std::this_thread::sleep_for (100ms);
}

BOOST_AUTO_TEST_CASE (tcpSSComms_rx_test)
{
    std::this_thread::sleep_for (400ms);
    std::atomic<int> ServerCounter{0};
    std::atomic<int> CommCounter{0};
    std::atomic<size_t> len{0};
    helics::ActionMessage act;
    std::string host = "127.0.0.1";
    helics::tcp::TcpCommsSS comm;
    comm.loadTargetInfo (host, "");
    std::mutex actguard;
    auto srv = AsioServiceManager::getServicePointer ();
    auto serviceLoop = srv->runServiceLoop();
    comm.setCallback ([&CommCounter, &act, &actguard](helics::ActionMessage m) {
        ++CommCounter;
        std::lock_guard<std::mutex> lock (actguard);
        act = m;
    });
    comm.setBrokerPort (TCP_BROKER_PORT);
    comm.setName ("tests");
    comm.setServerMode (true);

    bool connected = comm.connect ();
    BOOST_REQUIRE (connected);

    auto txconn = helics::tcp::TcpConnection::create (srv->getBaseService (), host, TCP_BROKER_PORT_STRING, 1024);
    auto res = txconn->waitUntilConnected (1000ms);
    BOOST_REQUIRE_EQUAL (res, true);

    BOOST_REQUIRE (txconn->isConnected ());

    helics::ActionMessage cmd (helics::CMD_ACK);
    std::string buffer = cmd.packetize ();

    txconn->send (buffer);

    std::this_thread::sleep_for (200ms);
    BOOST_CHECK_EQUAL (CommCounter, 1);
    std::lock_guard<std::mutex> lock (actguard);
    BOOST_CHECK (act.action () == helics::action_message_def::action_t::cmd_ack);
    txconn->close ();
    comm.disconnect ();
    std::this_thread::sleep_for (100ms);
}

BOOST_AUTO_TEST_CASE (tcpSSComm_transmit_through)
{
    std::this_thread::sleep_for (400ms);
    std::atomic<int> counter{0};
    std::atomic<int> counter2{0};
    guarded<helics::ActionMessage> act;
    guarded<helics::ActionMessage> act2;
    auto srv = AsioServiceManager::getServicePointer();
    auto serviceLoop = srv->runServiceLoop();

    std::string host = "localhost";
    helics::tcp::TcpCommsSS comm;
    helics::tcp::TcpCommsSS comm2;
    comm.loadTargetInfo (host, host);
    // comm2 is the broker
    comm2.loadTargetInfo (host, std::string ());

    comm.setBrokerPort (TCP_BROKER_PORT);
    comm.setName ("tests");
    comm.setServerMode (false);
    comm2.setName ("test2");
    comm2.setPortNumber (TCP_BROKER_PORT);
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
    BOOST_REQUIRE_EQUAL (counter2, 1);
    BOOST_CHECK (act2.lock ()->action () == helics::action_message_def::action_t::cmd_ack);

    comm2.disconnect ();
    BOOST_CHECK (!comm2.isConnected ());
    comm.disconnect ();
    BOOST_CHECK (!comm.isConnected ());

    std::this_thread::sleep_for (100ms);
}

BOOST_AUTO_TEST_CASE (tcpSSComm_transmit_add_route)
{
    std::this_thread::sleep_for (500ms);
    std::atomic<int> counter{0};
    std::atomic<int> counter2{0};
    std::atomic<int> counter3{0};

    std::string host = "localhost";
    helics::tcp::TcpCommsSS comm, comm2, comm3;
    auto srv = AsioServiceManager::getServicePointer();
    auto serviceLoop = srv->runServiceLoop();
    comm.loadTargetInfo (host, host);
    comm2.loadTargetInfo (host, std::string ());
    comm3.loadTargetInfo (host, host);

    comm.setBrokerPort (TCP_BROKER_PORT);
    comm.setName ("tests");
    comm.setServerMode (false);
    comm2.setName ("broker");
    comm2.setServerMode (true);
    comm3.setName ("test3");
    comm3.setServerMode (false);
    comm3.setBrokerPort (TCP_BROKER_PORT);

    comm2.setPortNumber (TCP_BROKER_PORT);

    guarded<helics::ActionMessage> act;
    guarded<helics::ActionMessage> act2;
    guarded<helics::ActionMessage> act3;

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

    comm.transmit (helics::route_id_t (0), helics::CMD_ACK);

    std::this_thread::sleep_for (250ms);
    BOOST_REQUIRE_EQUAL (counter2, 1);
    BOOST_CHECK (act2.lock ()->action () == helics::action_message_def::action_t::cmd_ack);

    comm3.transmit (helics::route_id_t (0), helics::CMD_ACK);

    std::this_thread::sleep_for (250ms);
    BOOST_REQUIRE_EQUAL (counter2, 2);
    BOOST_CHECK (act2.lock ()->action () == helics::action_message_def::action_t::cmd_ack);

    comm2.addRoute (helics::route_id_t (3), comm3.getAddress ());

    comm2.transmit (helics::route_id_t (3), helics::CMD_ACK);

    std::this_thread::sleep_for (250ms);
    if (counter3 != 1)
    {
        std::this_thread::sleep_for (250ms);
    }
    BOOST_REQUIRE_EQUAL (counter3, 1);
    BOOST_CHECK (act3.lock ()->action () == helics::action_message_def::action_t::cmd_ack);

    comm2.addRoute (helics::route_id_t (4), comm.getAddress ());

    comm2.transmit (helics::route_id_t (4), helics::CMD_ACK);

    std::this_thread::sleep_for (250ms);
    BOOST_REQUIRE_EQUAL (counter, 1);
    BOOST_CHECK (act.lock ()->action () == helics::action_message_def::action_t::cmd_ack);

    
    comm.disconnect ();
    comm3.disconnect ();
    comm2.disconnect();
    std::this_thread::sleep_for (100ms);
}

BOOST_AUTO_TEST_CASE (tcpSSCore_initialization_test)
{
    std::this_thread::sleep_for (400ms);
    std::atomic<int> counter{0};
    std::string initializationString = "1 --name=core1";
    auto core = helics::CoreFactory::create (helics::core_type::TCP_SS, initializationString);

    BOOST_REQUIRE (core);
    BOOST_CHECK (core->isInitialized ());
    auto srv = AsioServiceManager::getServicePointer();
    auto serviceLoop = srv->runServiceLoop();

    auto server = helics::tcp::TcpServer::create (srv->getBaseService (), "localhost", TCP_BROKER_PORT);
    std::vector<char> data (1024);
    std::atomic<size_t> len{0};
    server->setDataCall (
      [&data, &counter, &len](helics::tcp::TcpConnection::pointer, const char *data_rec, size_t data_Size) {
          std::copy (data_rec, data_rec + data_Size, data.begin () + len);
          len += data_Size;
          ++counter;
          return len.load ();
      });
    auto started=server->start ();

    BOOST_CHECK(started);
    std::this_thread::sleep_for(100ms);
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

        BOOST_CHECK_GT (len, 32);
        helics::ActionMessage rM;
        helics::ActionMessage rM2;
        auto used = rM.depacketize (data.data (), len);
        if (used < len)
        {
            auto use2 = rM2.depacketize (data.data () + used, len - used);
            if (use2 == 0)
            {
                while (counter != 2)
                {
                    std::this_thread::sleep_for (100ms);
                    ++cnt;
                    if (cnt > 30)
                    {
                        break;
                    }
                }
            }
            rM2.depacketize (data.data () + used, len - used);
        }
        else
        {
            while (counter != 2)
            {
                std::this_thread::sleep_for (100ms);
                ++cnt;
                if (cnt > 30)
                {
                    break;
                }
            }
            rM2.depacketize (data.data () + used, len - used);
        }
        BOOST_CHECK_EQUAL (rM.name, "core1");
        BOOST_CHECK (rM.action () == helics::action_message_def::action_t::cmd_protocol);

        BOOST_CHECK_EQUAL (rM2.name, "core1");
        BOOST_CHECK (rM2.action () == helics::action_message_def::action_t::cmd_reg_broker);
    }
    core->disconnect ();
    server->close ();
    core = nullptr;
    helics::CoreFactory::cleanUpCores (100ms);
}

/** test case checks default values and makes sure they all mesh together
also tests the automatic port determination for cores
*/

BOOST_AUTO_TEST_CASE (tcpSSCore_core_broker_default_test)
{
    std::this_thread::sleep_for (std::chrono::milliseconds (400));
    std::string initializationString = "1";

    auto broker = helics::BrokerFactory::create (helics::core_type::TCP_SS, initializationString);
    BOOST_REQUIRE (broker);
    auto core = helics::CoreFactory::create (helics::core_type::TCP_SS, initializationString);
    BOOST_REQUIRE (core);
    bool connected = broker->isConnected ();
    BOOST_CHECK (connected);
    connected = core->connect ();
    BOOST_CHECK (connected);

    auto ccore = static_cast<helics::tcp::TcpCoreSS *> (core.get ());
    // this will test the automatic port allocation
    BOOST_CHECK_EQUAL (ccore->getAddress (), ccore->getIdentifier ());
    core->disconnect ();
    broker->disconnect ();
    core = nullptr;
    broker = nullptr;
    helics::CoreFactory::cleanUpCores (100ms);
    helics::BrokerFactory::cleanUpBrokers (100ms);
}

BOOST_AUTO_TEST_SUITE_END ()
