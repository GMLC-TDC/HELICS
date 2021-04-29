/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helics/common/AsioContextManager.h"
#include "helics/common/GuardedTypes.hpp"
#include "helics/core/ActionMessage.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/core-types.hpp"
#include "helics/network/networkDefaults.hpp"
#include "helics/network/tcp/TcpBroker.h"
#include "helics/network/tcp/TcpComms.h"
#include "helics/network/tcp/TcpCore.h"
#include "helics/network/tcp/TcpHelperClasses.h"

#include "gtest/gtest.h"
#include <future>
#include <numeric>

using namespace std::literals::chrono_literals;

using asio::ip::tcp;
using helics::Core;

#define TCP_BROKER_PORT_STRING "24160"
#define TCP_SECONDARY_PORT 24180

TEST(TcpCore, tcpComms_broker)
{
    std::atomic<int> counter{0};
    std::string host = "localhost";
    helics::tcp::TcpComms comm;
    comm.loadTargetInfo(host, host);

    auto srv = AsioContextManager::getContextPointer();

    auto server =
        helics::tcp::TcpServer::create(srv->getBaseContext(), DEFAULT_TCP_BROKER_PORT_NUMBER);
    auto contextLoop = srv->startContextLoop();
    std::vector<char> data(1024);
    server->setDataCall([&counter](const helics::tcp::TcpConnection::pointer& /*unused*/,
                                   const char* /*unused*/,
                                   size_t data_avail) {
        ++counter;
        return data_avail;
    });
    server->start();

    comm.setCallback([&counter](const helics::ActionMessage& /*m*/) { ++counter; });
    comm.setBrokerPort(DEFAULT_TCP_BROKER_PORT_NUMBER);
    comm.setName("tests");
    comm.setTimeout(200ms);
    bool connected = comm.connect();
    EXPECT_TRUE(!connected);
    EXPECT_GE(counter, 1);
    comm.disconnect();
    server->close();

    std::this_thread::sleep_for(100ms);
}

TEST(TcpCore, tcpComms_broker_test_transmit)
{
    std::this_thread::sleep_for(300ms);
    std::atomic<int> counter{0};
    std::atomic<size_t> len{0};
    std::string host = "localhost";
    helics::tcp::TcpComms comm;
    comm.loadTargetInfo(host, host);

    auto srv = AsioContextManager::getContextPointer();
    auto server =
        helics::tcp::TcpServer::create(srv->getBaseContext(), host, DEFAULT_TCP_BROKER_PORT_NUMBER);
    auto contextLoop = srv->startContextLoop();
    std::vector<char> data(1024);
    server->setDataCall(
        [&data, &counter, &len](const helics::tcp::TcpConnection::pointer& /*unused*/,
                                const char* data_rec,
                                size_t data_Size) {
            std::copy(data_rec, data_rec + data_Size, data.begin());
            len = data_Size;
            ++counter;
            return data_Size;
        });
    ASSERT_TRUE(server->isReady());
    server->start();

    comm.setCallback([](const helics::ActionMessage& /*m*/) {});
    comm.setBrokerPort(DEFAULT_TCP_BROKER_PORT_NUMBER);
    comm.setPortNumber(TCP_SECONDARY_PORT);
    comm.setName("tests");
    comm.setFlag("noack_connect", true);
    bool connected = comm.connect();
    EXPECT_TRUE(connected) << "connection has failed-bb";
    if (connected) {
        comm.transmit(helics::parent_route_id, helics::CMD_IGNORE);

        asio::error_code error;
        int cnt = 0;
        while (counter != 1) {
            std::this_thread::sleep_for(100ms);
            ++cnt;
            if (cnt > 30) {
                break;
            }
        }
        EXPECT_EQ(counter, 1);

        EXPECT_GT(len, 32U);
        helics::ActionMessage rM(data.data(), len);
        EXPECT_TRUE(rM.action() == helics::action_message_def::action_t::cmd_ignore);
    }
    comm.disconnect();
    server->close();
    std::this_thread::sleep_for(100ms);
}

TEST(TcpCore, tcpComms_rx)
{
    std::this_thread::sleep_for(300ms);
    std::atomic<int> ServerCounter{0};
    std::atomic<int> CommCounter{0};
    std::atomic<size_t> len{0};
    helics::ActionMessage act;
    std::string host = "127.0.0.1";
    helics::tcp::TcpComms comm;
    comm.loadTargetInfo(host, host);
    std::mutex actguard;
    auto srv = AsioContextManager::getContextPointer();

    auto server =
        helics::tcp::TcpServer::create(srv->getBaseContext(), host, TCP_BROKER_PORT_STRING);
    auto contextLoop = srv->startContextLoop();
    std::vector<char> data(1024);
    server->setDataCall(
        [&data, &ServerCounter, &len](const helics::tcp::TcpConnection::pointer& /*unused*/,
                                      const char* data_rec,
                                      size_t data_Size) {
            std::copy(data_rec, data_rec + data_Size, data.begin());
            len = data_Size;
            ++ServerCounter;
            return data_Size;
        });
    ASSERT_TRUE(server->isReady());
    server->start();

    comm.setCallback([&CommCounter, &act, &actguard](helics::ActionMessage m) {
        ++CommCounter;
        std::lock_guard<std::mutex> lock(actguard);
        act = std::move(m);
    });
    comm.setBrokerPort(DEFAULT_TCP_BROKER_PORT_NUMBER);
    comm.setPortNumber(TCP_SECONDARY_PORT);
    comm.setName("tests");
    comm.setFlag("noack_connect", true);

    bool connected = comm.connect();
    ASSERT_TRUE(connected);

    auto txconn = helics::tcp::TcpConnection::create(srv->getBaseContext(), host, "24180", 1024);
    auto res = txconn->waitUntilConnected(1000ms);
    ASSERT_EQ(res, true);

    ASSERT_TRUE(txconn->isConnected());

    helics::ActionMessage cmd(helics::CMD_ACK);
    std::string buffer = cmd.packetize();

    txconn->send(buffer);

    std::this_thread::sleep_for(200ms);
    EXPECT_EQ(CommCounter, 1);
    std::lock_guard<std::mutex> lock(actguard);
    EXPECT_TRUE(act.action() == helics::action_message_def::action_t::cmd_ack);
    txconn->close();
    comm.disconnect();
    server->close();
    std::this_thread::sleep_for(200ms);
}

TEST(TcpCore, tcpServerConnections1)
{
    std::atomic<int> counter{0};
    std::string host = "127.0.0.1";

    auto srv = AsioContextManager::getContextPointer();
    auto server =
        helics::tcp::TcpServer::create(srv->getBaseContext(), host, DEFAULT_TCP_BROKER_PORT_NUMBER);
    ASSERT_TRUE(server->isReady());
    auto contextLoop = srv->startContextLoop();
    std::vector<char> data(1024);
    std::atomic<bool> validData{true};

    auto dataCheck = [&counter, &validData](const helics::tcp::TcpConnection::pointer& /*unused*/,
                                            const char* datablock,
                                            size_t datasize) {
        size_t used = 0;
        while (datasize - used >= 20) {
            ++counter;
            if (datasize < 20) {
                validData = false;
            }
            for (int ii = 1; ii < 20; ++ii) {
                if (ii + datablock[used + 0] != datablock[used + ii]) {
                    validData = false;
                }
            }
            used += 20;
        }

        return used;
    };

    server->setDataCall(dataCheck);
    server->start();

    auto conn1 = helics::tcp::TcpConnection::create(srv->getBaseContext(), host, "24160", 1024);
    auto conn2 = helics::tcp::TcpConnection::create(srv->getBaseContext(), host, "24160", 1024);
    auto conn3 = helics::tcp::TcpConnection::create(srv->getBaseContext(), host, "24160", 1024);
    auto conn4 = helics::tcp::TcpConnection::create(srv->getBaseContext(), host, "24160", 1024);
    ASSERT_TRUE(conn1);
    ASSERT_TRUE(conn2);
    ASSERT_TRUE(conn3);
    ASSERT_TRUE(conn4);
    auto res = conn1->waitUntilConnected(1000ms);
    EXPECT_EQ(res, true);
    res = conn2->waitUntilConnected(1000ms);
    EXPECT_EQ(res, true);
    res = conn3->waitUntilConnected(1000ms);
    EXPECT_EQ(res, true);
    res = conn4->waitUntilConnected(1000ms);
    EXPECT_EQ(res, true);

    auto transmitFunc = [](const helics::tcp::TcpConnection::pointer& obj) {
        std::vector<char> dataB(20);
        for (char ii = 0; ii < 50; ++ii) {
            std::iota(dataB.begin(), dataB.end(), ii);
            obj->send(dataB.data(), 20);
        }
    };

    auto thr1 = std::thread(transmitFunc, conn1);
    auto thr2 = std::thread(transmitFunc, conn2);
    auto thr3 = std::thread(transmitFunc, conn3);
    auto thr4 = std::thread(transmitFunc, conn4);

    thr1.join();
    thr2.join();
    thr3.join();
    thr4.join();
    int cnt = 0;
    while (counter < 200) {
        std::this_thread::sleep_for(50ms);
        ++cnt;
        if (cnt > 20) {
            break;
        }
    }
    EXPECT_EQ(counter, 200);
    EXPECT_TRUE(validData);
    conn1->close();
    conn2->close();
    conn3->close();
    conn4->close();
    server->close();
}

TEST(TcpCore, tcpComm_transmit_through)
{
    std::this_thread::sleep_for(300ms);
    std::atomic<int> counter{0};
    std::atomic<int> counter2{0};
    guarded<helics::ActionMessage> act;
    guarded<helics::ActionMessage> act2;

    std::string host = "localhost";
    helics::tcp::TcpComms comm;
    comm.loadTargetInfo(host, host);
    comm.setFlag("reuse_address", true);
    helics::tcp::TcpComms comm2;
    comm2.loadTargetInfo(host, std::string());

    comm.setBrokerPort(DEFAULT_TCP_BROKER_PORT_NUMBER + 1);
    comm.setName("tests");
    comm2.setName("test2");
    comm2.setPortNumber(DEFAULT_TCP_BROKER_PORT_NUMBER + 1);
    comm2.setFlag("reuse_address", true);
    comm.setPortNumber(TCP_SECONDARY_PORT);

    comm.setCallback([&counter, &act](const helics::ActionMessage& m) {
        ++counter;
        act = m;
    });
    comm2.setCallback([&counter2, &act2](const helics::ActionMessage& m) {
        ++counter2;
        act2 = m;
    });
    // need to launch the connection commands at the same time since they depend on each other in
    // this case

    bool connected1 = comm2.connect();
    ASSERT_TRUE(connected1);
    bool connected2 = comm.connect();
    if (!connected2) {  // lets just try again if it is not connected
        connected2 = comm.connect();
    }
    ASSERT_TRUE(connected2);

    comm.transmit(helics::parent_route_id, helics::CMD_ACK);
    std::this_thread::sleep_for(250ms);
    if (counter2 != 1) {
        std::this_thread::sleep_for(500ms);
    }
    ASSERT_EQ(counter2, 1);
    EXPECT_TRUE(act2.lock()->action() == helics::action_message_def::action_t::cmd_ack);

    comm.disconnect();
    EXPECT_TRUE(!comm.isConnected());

    comm2.disconnect();
    EXPECT_TRUE(!comm2.isConnected());

    std::this_thread::sleep_for(100ms);
}

TEST(TcpCore, tcpComm_transmit_add_route)
{
    std::this_thread::sleep_for(300ms);
    std::atomic<int> counter{0};
    std::atomic<int> counter2{0};
    std::atomic<int> counter3{0};

    std::string host = "localhost";
    helics::tcp::TcpComms comm;
    helics::tcp::TcpComms comm2;
    helics::tcp::TcpComms comm3;

    comm.loadTargetInfo(host, host);
    comm2.loadTargetInfo(host, std::string());
    comm3.loadTargetInfo(host, host);
    auto srv = AsioContextManager::getContextPointer();
    auto contextLoop = srv->startContextLoop();

    comm.setBrokerPort(DEFAULT_TCP_BROKER_PORT_NUMBER + 2);
    comm.setFlag("reuse_address", true);
    comm.setName("tests");
    comm2.setName("broker");
    comm2.setFlag("reuse_address", true);
    comm3.setName("test3");
    comm3.setBrokerPort(DEFAULT_TCP_BROKER_PORT_NUMBER + 2);
    comm3.setFlag("reuse_address", true);
    comm2.setPortNumber(DEFAULT_TCP_BROKER_PORT_NUMBER + 2);
    comm.setPortNumber(TCP_SECONDARY_PORT);
    comm3.setPortNumber(23920);

    guarded<helics::ActionMessage> act;
    guarded<helics::ActionMessage> act2;
    guarded<helics::ActionMessage> act3;

    comm.setCallback([&counter, &act](helics::ActionMessage&& m) {
        ++counter;
        act = std::move(m);
    });
    comm2.setCallback([&counter2, &act2](helics::ActionMessage&& m) {
        ++counter2;
        act2 = std::move(m);
    });
    comm3.setCallback([&counter3, &act3](helics::ActionMessage&& m) {
        ++counter3;
        act3 = std::move(m);
    });

    bool connected = comm2.connect();
    ASSERT_TRUE(connected);
    // connected = connected_fut.get();
    connected = comm.connect();
    ASSERT_TRUE(connected);
    connected = comm3.connect();
    ASSERT_TRUE(connected);
    comm.transmit(helics::parent_route_id, helics::CMD_ACK);

    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    ASSERT_EQ(counter2, 1);
    EXPECT_TRUE(act2.lock()->action() == helics::action_message_def::action_t::cmd_ack);

    comm3.transmit(helics::parent_route_id, helics::CMD_ACK);

    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    ASSERT_EQ(counter2, 2);
    EXPECT_TRUE(act2.lock()->action() == helics::action_message_def::action_t::cmd_ack);

    comm2.addRoute(helics::route_id(3), comm3.getAddress());

    comm2.transmit(helics::route_id(3), helics::CMD_ACK);

    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    if (counter3 != 1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
    ASSERT_EQ(counter3, 1);
    EXPECT_TRUE(act3.lock()->action() == helics::action_message_def::action_t::cmd_ack);

    comm2.addRoute(helics::route_id(4), comm.getAddress());

    comm2.transmit(helics::route_id(4), helics::CMD_ACK);

    std::this_thread::sleep_for(250ms);
    ASSERT_EQ(counter, 1);
    EXPECT_TRUE(act.lock()->action() == helics::action_message_def::action_t::cmd_ack);

    comm.disconnect();
    comm3.disconnect();
    comm2.disconnect();

    std::this_thread::sleep_for(100ms);
}

TEST(TcpCore, tcpCore_initialization)
{
    std::this_thread::sleep_for(300ms);
    std::atomic<int> counter{0};
    std::string initializationString =
        "-f 1 --brokerport=24160  --port=24180 --local_interface=localhost --name=core1 --noack_connect";
    auto core = helics::CoreFactory::create(helics::core_type::TCP, initializationString);

    ASSERT_TRUE(core);
    EXPECT_TRUE(core->isConfigured());
    auto srv = AsioContextManager::getContextPointer();

    auto server = helics::tcp::TcpServer::create(srv->getBaseContext(),
                                                 "localhost",
                                                 DEFAULT_TCP_BROKER_PORT_NUMBER);
    auto contextLoop = srv->startContextLoop();
    std::vector<char> data(1024);
    std::atomic<size_t> len{0};
    server->setDataCall(
        [&data, &counter, &len](const helics::tcp::TcpConnection::pointer& /*unused*/,
                                const char* data_rec,
                                size_t data_Size) {
            std::copy(data_rec, data_rec + data_Size, data.begin());
            len = data_Size;
            ++counter;
            return data_Size;
        });
    server->setPortReuse(true);
    EXPECT_TRUE(server->start());
    bool connected = core->connect();
    EXPECT_TRUE(connected);

    if (connected) {
        int cnt = 0;
        while (counter != 1) {
            std::this_thread::sleep_for(100ms);
            ++cnt;
            if (cnt > 30) {
                break;
            }
        }
        EXPECT_EQ(counter, 1);

        EXPECT_GT(len, 32U);
        helics::ActionMessage rM(data.data(), len);

        EXPECT_EQ(rM.name, "core1");
        EXPECT_TRUE(rM.action() == helics::action_message_def::action_t::cmd_reg_broker);
        // helics::ActionMessage resp (helics::CMD_PRIORITY_ACK);
        //  rxSocket.send_to (asio::buffer (resp.packetize ()), remote_endpoint, 0, error);
        // EXPECT_TRUE (!error);
    }
    core->disconnect();
    server->close();

    core = nullptr;
    helics::CoreFactory::cleanUpCores(100ms);
}

/** test case checks default values and makes sure they all mesh together
also tests the automatic port determination for cores
*/

TEST(TcpCore, tcpCore_core_broker_default)
{
    std::this_thread::sleep_for(300ms);
    std::string initializationString = "--reuse_address";

    auto broker = helics::BrokerFactory::create(helics::core_type::TCP, initializationString);
    ASSERT_TRUE(broker);
    auto core = helics::CoreFactory::create(helics::core_type::TCP, initializationString);
    ASSERT_TRUE(core);
    bool connected = broker->isConnected();
    EXPECT_TRUE(connected);
    connected = core->connect();
    EXPECT_TRUE(connected);

    auto ccore = static_cast<helics::tcp::TcpCore*>(core.get());
    // this will test the automatic port allocation
    int match = ccore->getAddress().compare(0, 12, "localhost:24");
    EXPECT_EQ(match, 0) << ccore->getAddress() << " does not match expected>localhost:24XXX\n";

    core->disconnect();
    broker->disconnect();
    core = nullptr;
    broker = nullptr;
    helics::CoreFactory::cleanUpCores(100ms);
    helics::BrokerFactory::cleanUpBrokers(100ms);
}

TEST(TcpCore, commFactory)
{
    auto comm = helics::CommFactory::create("tcp");
    auto comm2 = helics::CommFactory::create(helics::core_type::TCP);

    EXPECT_TRUE(dynamic_cast<helics::tcp::TcpComms*>(comm.get()) != nullptr);
    EXPECT_TRUE(dynamic_cast<helics::tcp::TcpComms*>(comm2.get()) != nullptr);
}
