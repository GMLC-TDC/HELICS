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
#include "helics/core/tcp/TcpComms.h"
#include "helics/core/tcp/TcpCore.h"
#include "helics/core/tcp/TcpHelperClasses.h"

#include <numeric>

//#include "boost/process.hpp"
#include <future>

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE (TcpCore_tests, *utf::label("ci") *utf::label("daily") *utf::label("release"))

using boost::asio::ip::tcp;
using helics::Core;

#define TCP_BROKER_PORT 24160
#define TCP_SECONDARY_PORT 24180
BOOST_AUTO_TEST_CASE (test_tcpServerConnections1)
{
    std::atomic<int> counter{0};
    std::string host = "localhost";

    auto srv = AsioServiceManager::getServicePointer ();
    auto server = helics::tcp::TcpServer::create (srv->getBaseService (), TCP_BROKER_PORT);
    auto serviceLoop = srv->runServiceLoop ();
    std::vector<char> data (1024);
    std::atomic<bool> validData{true};

    auto dataCheck = [&counter, &validData](helics::tcp::TcpRxConnection::pointer, const char *datablock,
                                            size_t datasize) {
        size_t used = 0;
        while (datasize - used >= 20)
        {
            ++counter;
            if (datasize < 20)
            {
                validData = false;
            }
            for (int ii = 1; ii < 20; ++ii)
            {
                if (ii + datablock[used + 0] != datablock[used + ii])
                {
                    validData = false;
                }
            }
            used += 20;
        }

        return used;
    };

    server->setDataCall (dataCheck);
    server->start ();

    auto conn1 = helics::tcp::TcpConnection::create (srv->getBaseService (), host, "24160", 1024);
    auto conn2 = helics::tcp::TcpConnection::create (srv->getBaseService (), host, "24160", 1024);
    auto conn3 = helics::tcp::TcpConnection::create (srv->getBaseService (), host, "24160", 1024);
    auto conn4 = helics::tcp::TcpConnection::create (srv->getBaseService (), host, "24160", 1024);

    auto res = conn1->waitUntilConnected (1000);
    BOOST_CHECK_EQUAL (res, 0);
    res = conn2->waitUntilConnected (1000);
    BOOST_CHECK_EQUAL (res, 0);
    res = conn3->waitUntilConnected (1000);
    BOOST_CHECK_EQUAL (res, 0);
    res = conn4->waitUntilConnected (1000);
    BOOST_CHECK_EQUAL (res, 0);

    auto transmitFunc = [](helics::tcp::TcpConnection::pointer obj) {
        std::vector<char> dataB (20);
        for (char ii = 0; ii < 50; ++ii)
        {
            std::iota (dataB.begin (), dataB.end (), ii);
            obj->send (dataB.data (), 20);
        }
    };

    auto thr1 = std::thread (transmitFunc, conn1);
    auto thr2 = std::thread (transmitFunc, conn2);
    auto thr3 = std::thread (transmitFunc, conn3);
    auto thr4 = std::thread (transmitFunc, conn4);

    thr1.join ();
    thr2.join ();
    thr3.join ();
    thr4.join ();
    int cnt = 0;
    while (counter < 200)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (50));
        ++cnt;
        if (cnt > 20)
        {
            break;
        }
    }
    BOOST_CHECK_EQUAL (counter, 200);
    BOOST_CHECK (validData);
    conn1->close ();
    conn2->close ();
    conn3->close ();
    conn4->close ();
    server->close ();
}

BOOST_AUTO_TEST_CASE (tcpComms_broker_test)
{
    std::atomic<int> counter{0};
    std::string host = "localhost";
    helics::tcp::TcpComms comm (host, host);

    auto srv = AsioServiceManager::getServicePointer ();

    auto server = helics::tcp::TcpServer::create (srv->getBaseService (), TCP_BROKER_PORT);
    auto serviceLoop = srv->runServiceLoop ();
    std::vector<char> data (1024);
    server->setDataCall ([&counter](helics::tcp::TcpRxConnection::pointer, const char *, size_t data_avail) {
        ++counter;
        return data_avail;
    });
    server->start ();

    comm.setCallback ([&counter](helics::ActionMessage /*m*/) { ++counter; });
    comm.setBrokerPort (TCP_BROKER_PORT);
    comm.setName ("tests");
    comm.setTimeout (1000);
    auto confut = std::async (std::launch::async, [&comm]() { return comm.connect (); });

    std::this_thread::sleep_for (std::chrono::milliseconds (100));
    int cnt = 0;
    while (counter != 1)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (100));
        ++cnt;
        if (cnt > 30)
        {
            break;
        }
    }
    BOOST_CHECK_EQUAL (counter, 1);

    server->close ();
    comm.disconnect ();
}

BOOST_AUTO_TEST_CASE (tcpComms_broker_test_transmit)
{
    std::atomic<int> counter{0};
    std::atomic<size_t> len{0};
    std::string host = "localhost";
    helics::tcp::TcpComms comm (host, host);

    auto srv = AsioServiceManager::getServicePointer ();
    auto server = helics::tcp::TcpServer::create (srv->getBaseService (), TCP_BROKER_PORT);
    srv->runServiceLoop ();
    std::vector<char> data (1024);
    server->setDataCall (
      [&data, &counter, &len](helics::tcp::TcpRxConnection::pointer, const char *data_rec, size_t data_Size) {
          std::copy (data_rec, data_rec + data_Size, data.begin ());
          len = data_Size;
          ++counter;
          return data_Size;
      });
    server->start ();

    comm.setCallback ([](helics::ActionMessage /*m*/) {});
    comm.setBrokerPort (TCP_BROKER_PORT);
    comm.setPortNumber (TCP_SECONDARY_PORT);
    comm.setName ("tests");
    bool connected = comm.connect ();
    BOOST_REQUIRE (connected);
    comm.transmit (0, helics::CMD_IGNORE);

    boost::system::error_code error;
    int cnt = 0;
    while (counter != 1)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (100));
        ++cnt;
        if (cnt > 30)
        {
            break;
        }
    }
    BOOST_CHECK_EQUAL (counter, 1);

    BOOST_CHECK_GT (len, 32);
    helics::ActionMessage rM (data.data (), len);
    BOOST_CHECK (rM.action () == helics::action_message_def::action_t::cmd_ignore);
    server->close ();
    comm.disconnect ();
    std::this_thread::sleep_for (std::chrono::milliseconds (100));
}

BOOST_AUTO_TEST_CASE (tcpComms_rx_test)
{
    std::atomic<int> ServerCounter{0};
    std::atomic<int> CommCounter{0};
    std::atomic<size_t> len{0};
    helics::ActionMessage act;
    std::string host = "localhost";
    helics::tcp::TcpComms comm (host, host);
    std::mutex actguard;
    auto srv = AsioServiceManager::getServicePointer ();

    auto server = helics::tcp::TcpServer::create (srv->getBaseService (), TCP_BROKER_PORT);
    srv->runServiceLoop ();
    std::vector<char> data (1024);
    server->setDataCall ([&data, &ServerCounter, &len](helics::tcp::TcpRxConnection::pointer, const char *data_rec,
                                                       size_t data_Size) {
        std::copy (data_rec, data_rec + data_Size, data.begin ());
        len = data_Size;
        ++ServerCounter;
        return data_Size;
    });
    server->start ();

    comm.setCallback ([&CommCounter, &act, &actguard](helics::ActionMessage m) {
        ++CommCounter;
        std::lock_guard<std::mutex> lock (actguard);
        act = m;
    });
    comm.setBrokerPort (TCP_BROKER_PORT);
    comm.setPortNumber (TCP_SECONDARY_PORT);
    comm.setName ("tests");

    bool connected = comm.connect ();
    BOOST_REQUIRE (connected);

    auto txconn = helics::tcp::TcpConnection::create (srv->getBaseService (), host, "24180", 1024);
    auto res = txconn->waitUntilConnected (1000);
    BOOST_REQUIRE_EQUAL (res, 0);

    BOOST_REQUIRE (txconn->isConnected ());

    helics::ActionMessage cmd (helics::CMD_ACK);
    std::string buffer = cmd.packetize ();

    txconn->send (buffer);

    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    BOOST_CHECK_EQUAL (CommCounter, 1);
    std::lock_guard<std::mutex> lock (actguard);
    BOOST_CHECK (act.action () == helics::action_message_def::action_t::cmd_ack);
    txconn->close ();
    server->close ();
    comm.disconnect ();
    std::this_thread::sleep_for (std::chrono::milliseconds (100));
}

BOOST_AUTO_TEST_CASE (tcpComm_transmit_through)
{
    std::atomic<int> counter{0};
    std::atomic<int> counter2{0};
    guarded<helics::ActionMessage> act;
    guarded<helics::ActionMessage> act2;

    std::string host = "localhost";
    helics::tcp::TcpComms comm (host, host);
    helics::tcp::TcpComms comm2 (host, std::string ());

    comm.setBrokerPort (TCP_BROKER_PORT);
    comm.setName ("tests");
    comm2.setName ("test2");
    comm2.setPortNumber (TCP_BROKER_PORT);
    comm.setPortNumber (TCP_SECONDARY_PORT);

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

    bool connected = comm2.connect ();
    BOOST_REQUIRE (connected);
    connected = connected_fut.get ();
    BOOST_REQUIRE (connected);

    comm.transmit (0, helics::CMD_ACK);
    BOOST_CHECK (connected);
    std::this_thread::sleep_for (std::chrono::milliseconds (250));
    if (counter2 != 1)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (500));
    }
    BOOST_REQUIRE_EQUAL (counter2, 1);
    BOOST_CHECK (act2.lock ()->action () == helics::action_message_def::action_t::cmd_ack);

    comm2.disconnect ();
    BOOST_CHECK (!comm2.isConnected ());
    comm.disconnect ();
    BOOST_CHECK (!comm.isConnected ());

    std::this_thread::sleep_for (std::chrono::milliseconds (100));
}

BOOST_AUTO_TEST_CASE (tcpComm_transmit_add_route)
{
    std::atomic<int> counter{0};
    std::atomic<int> counter2{0};
    std::atomic<int> counter3{0};

    std::string host = "localhost";
    helics::tcp::TcpComms comm (host, host);
    helics::tcp::TcpComms comm2 (host, "");
    helics::tcp::TcpComms comm3 (host, host);

    comm.setBrokerPort (TCP_BROKER_PORT);
    comm.setName ("tests");
    comm2.setName ("broker");
    comm3.setName ("test3");
    comm3.setBrokerPort (TCP_BROKER_PORT);

    comm2.setPortNumber (TCP_BROKER_PORT);
    comm.setPortNumber (TCP_SECONDARY_PORT);
    comm3.setPortNumber (23920);

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

    comm.transmit (0, helics::CMD_ACK);

    std::this_thread::sleep_for (std::chrono::milliseconds (250));
    BOOST_REQUIRE_EQUAL (counter2, 1);
    BOOST_CHECK (act2.lock ()->action () == helics::action_message_def::action_t::cmd_ack);

    comm3.transmit (0, helics::CMD_ACK);

    std::this_thread::sleep_for (std::chrono::milliseconds (250));
    BOOST_REQUIRE_EQUAL (counter2, 2);
    BOOST_CHECK (act2.lock ()->action () == helics::action_message_def::action_t::cmd_ack);

    comm2.addRoute (3, comm3.getAddress ());

    comm2.transmit (3, helics::CMD_ACK);

    std::this_thread::sleep_for (std::chrono::milliseconds (250));
    if (counter3 != 1)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (250));
    }
    BOOST_REQUIRE_EQUAL (counter3, 1);
    BOOST_CHECK (act3.lock ()->action () == helics::action_message_def::action_t::cmd_ack);

    comm2.addRoute (4, comm.getAddress ());

    comm2.transmit (4, helics::CMD_ACK);

    std::this_thread::sleep_for (std::chrono::milliseconds (250));
    BOOST_REQUIRE_EQUAL (counter, 1);
    BOOST_CHECK (act.lock ()->action () == helics::action_message_def::action_t::cmd_ack);

    comm.disconnect ();
    comm2.disconnect ();
    comm3.disconnect ();
    std::this_thread::sleep_for (std::chrono::milliseconds (100));
}

BOOST_AUTO_TEST_CASE (tcpCore_initialization_test)
{
    std::atomic<int> counter{0};
    std::string initializationString =
      "1 --brokerport=24160  --port=24180 --local_interface=localhost --name=core1";
    auto core = helics::CoreFactory::create (helics::core_type::TCP, initializationString);

    BOOST_REQUIRE (core);
    BOOST_CHECK (core->isInitialized ());
    auto srv = AsioServiceManager::getServicePointer ();

    auto server = helics::tcp::TcpServer::create (srv->getBaseService (), TCP_BROKER_PORT);
    srv->runServiceLoop ();
    std::vector<char> data (1024);
    std::atomic<size_t> len{0};
    server->setDataCall (
      [&data, &counter, &len](helics::tcp::TcpRxConnection::pointer, const char *data_rec, size_t data_Size) {
          std::copy (data_rec, data_rec + data_Size, data.begin ());
          len = data_Size;
          ++counter;
          return data_Size;
      });
    server->start ();
    BOOST_TEST_PASSPOINT ();
    bool connected = core->connect ();
    BOOST_CHECK (connected);

    if (connected)
    {
        int cnt = 0;
        while (counter != 1)
        {
            std::this_thread::sleep_for (std::chrono::milliseconds (100));
            ++cnt;
            if (cnt > 30)
            {
                break;
            }
        }
        BOOST_CHECK_EQUAL (counter, 1);

        BOOST_CHECK_GT (len, 32);
        helics::ActionMessage rM (data.data (), len);

        BOOST_CHECK_EQUAL (rM.name, "core1");
        BOOST_CHECK (rM.action () == helics::action_message_def::action_t::cmd_reg_broker);
        // helics::ActionMessage resp (helics::CMD_PRIORITY_ACK);
        //  rxSocket.send_to (boost::asio::buffer (resp.packetize ()), remote_endpoint, 0, error);
        // BOOST_CHECK (!error);
    }
    server->close ();
    core->disconnect ();
    core = nullptr;
    helics::CoreFactory::cleanUpCores (100);
}

/** test case checks default values and makes sure they all mesh together
also tests the automatic port determination for cores
*/

BOOST_AUTO_TEST_CASE (tcpCore_core_broker_default_test)
{
    std::string initializationString = "1";

    auto broker = helics::BrokerFactory::create (helics::core_type::TCP, initializationString);
    BOOST_REQUIRE (broker);
    auto core = helics::CoreFactory::create (helics::core_type::TCP, initializationString);
    BOOST_REQUIRE (core);
    bool connected = broker->isConnected ();
    BOOST_CHECK (connected);
    connected = core->connect ();
    BOOST_CHECK (connected);

    auto ccore = static_cast<helics::tcp::TcpCore *> (core.get ());
    // this will test the automatic port allocation
    BOOST_CHECK_EQUAL (ccore->getAddress (), "localhost:24228");
    core->disconnect ();
    broker->disconnect ();
    core = nullptr;
    broker = nullptr;
    helics::CoreFactory::cleanUpCores (100);
    helics::BrokerFactory::cleanUpBrokers (100);
}

BOOST_AUTO_TEST_SUITE_END ()
