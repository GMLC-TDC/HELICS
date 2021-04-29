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
#include "helics/network/tcp/TcpCommsSS.h"
#include "helics/network/tcp/TcpCore.h"
#include "helics/network/tcp/TcpHelperClasses.h"

#include "gtest/gtest.h"
#include <future>
#include <numeric>

using namespace std::literals::chrono_literals;

using asio::ip::tcp;
using helics::Core;

#define TCP_BROKER_PORT_STRING "33133"

#define TCP_BROKER_PORT_ALT 33134
#define TCP_BROKER_PORT_ALT_STRING "33134"

TEST(TcpSSCore, tcpSSComms_broker)
{
    std::atomic<int> counter{0};
    std::string host = "localhost";
    helics::tcp::TcpCommsSS comm;
    comm.loadTargetInfo(host, host);

    auto srv = AsioContextManager::getContextPointer();

    auto server = helics::tcp::TcpServer::create(srv->getBaseContext(), DEFAULT_TCPSS_PORT);
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
    comm.setBrokerPort(DEFAULT_TCPSS_PORT);
    comm.setName("tests");
    comm.setTimeout(1000ms);
    comm.setServerMode(false);
    auto confut = std::async(std::launch::async, [&comm]() { return comm.connect(); });

    std::this_thread::sleep_for(100ms);
    int cnt = 0;
    while (counter < 1) {
        std::this_thread::sleep_for(100ms);
        ++cnt;
        if (cnt > 30) {
            break;
        }
    }
    EXPECT_EQ(counter, 1);

    server->close();
    comm.disconnect();
    std::this_thread::sleep_for(100ms);
}

TEST(TcpSSCore, tcpSSComms_broker_test_transmit)
{
    std::this_thread::sleep_for(400ms);
    std::atomic<int> counter{0};
    std::atomic<size_t> len{0};
    std::string host = "localhost";
    helics::tcp::TcpCommsSS comm;
    comm.loadTargetInfo(host, host);

    auto srv = AsioContextManager::getContextPointer();
    auto contextLoop = srv->startContextLoop();

    auto server = helics::tcp::TcpServer::create(srv->getBaseContext(), host, DEFAULT_TCPSS_PORT);

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
    auto res = server->start();
    EXPECT_TRUE(res);
    std::this_thread::sleep_for(100ms);
    comm.setCallback([](const helics::ActionMessage& /*m*/) {});
    comm.setBrokerPort(DEFAULT_TCPSS_PORT);
    comm.setName("tests");
    comm.setServerMode(false);
    bool connected = comm.connect();
    ASSERT_TRUE(connected);
    comm.transmit(helics::parent_route_id, helics::CMD_IGNORE);

    asio::error_code error;
    int cnt = 0;
    while (counter < 2) {
        if (len > 130) {
            break;
        }
        std::this_thread::sleep_for(100ms);
        ++cnt;
        if (cnt > 30) {
            break;
        }
    }
    EXPECT_GE(counter, 1);

    EXPECT_GT(len.load(), 50U);
    helics::ActionMessage rM;
    auto loc = rM.depacketize(data.data(), static_cast<int>(len));
    if ((counter == 1) && (loc < static_cast<int>(len.load()))) {
        rM.depacketize(data.data() + loc, static_cast<int>(len.load() - loc));
    }
    EXPECT_TRUE(rM.action() == helics::action_message_def::action_t::cmd_ignore);
    server->close();
    comm.disconnect();
    std::this_thread::sleep_for(100ms);
}

TEST(TcpSSCore, tcpSSComms_rx)
{
    std::this_thread::sleep_for(400ms);
    // std::atomic<int> ServerCounter{0};
    std::atomic<int> CommCounter{0};
    // std::atomic<size_t> len{0};
    helics::ActionMessage act;
    std::string host = "127.0.0.1";
    helics::tcp::TcpCommsSS comm;
    comm.loadTargetInfo(host, "");
    std::mutex actguard;
    auto srv = AsioContextManager::getContextPointer();
    auto contextLoop = srv->startContextLoop();
    comm.setCallback([&CommCounter, &act, &actguard](const helics::ActionMessage& m) {
        ++CommCounter;
        std::lock_guard<std::mutex> lock(actguard);
        act = m;
    });
    comm.setBrokerPort(DEFAULT_TCPSS_PORT);
    comm.setName("tests");
    comm.setServerMode(true);

    bool connected = comm.connect();
    ASSERT_TRUE(connected);

    auto txconn = helics::tcp::TcpConnection::create(srv->getBaseContext(),
                                                     host,
                                                     TCP_BROKER_PORT_STRING,
                                                     1024);
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
    std::this_thread::sleep_for(100ms);
}

TEST(TcpSSCore, tcpSSComm_transmit_through)
{
    std::this_thread::sleep_for(400ms);
    std::atomic<int> counter{0};
    std::atomic<int> counter2{0};
    guarded<helics::ActionMessage> act;
    guarded<helics::ActionMessage> act2;
    auto srv = AsioContextManager::getContextPointer();
    auto contextLoop = srv->startContextLoop();

    std::string host = "localhost";
    helics::tcp::TcpCommsSS comm;
    helics::tcp::TcpCommsSS comm2;
    comm.loadTargetInfo(host, host);
    // comm2 is the broker
    comm2.loadTargetInfo(host, std::string());

    comm.setBrokerPort(DEFAULT_TCPSS_PORT);
    comm.setName("tests");
    comm.setServerMode(false);
    comm2.setName("test2");
    comm2.setPortNumber(DEFAULT_TCPSS_PORT);
    comm2.setServerMode(true);

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
    auto connected_fut = std::async(std::launch::async, [&comm] { return comm.connect(); });
    bool connected1 = comm2.connect();
    ASSERT_TRUE(connected1);
    bool connected2 = connected_fut.get();
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

    comm2.disconnect();
    EXPECT_TRUE(!comm2.isConnected());
    comm.disconnect();
    EXPECT_TRUE(!comm.isConnected());

    std::this_thread::sleep_for(100ms);
}

TEST(TcpSSCore, tcpSSComm_transmit_add_route)
{
    std::this_thread::sleep_for(500ms);
    std::atomic<int> counter{0};
    std::atomic<int> counter2{0};
    std::atomic<int> counter3{0};

    std::string host = "localhost";
    helics::tcp::TcpCommsSS comm;
    helics::tcp::TcpCommsSS comm2;
    helics::tcp::TcpCommsSS comm3;
    auto srv = AsioContextManager::getContextPointer();
    auto contextLoop = srv->startContextLoop();
    comm.loadTargetInfo(host, host);
    comm2.loadTargetInfo(host, std::string());
    comm3.loadTargetInfo(host, host);

    comm.setBrokerPort(DEFAULT_TCPSS_PORT);
    comm.setName("tests");
    comm.setServerMode(false);
    comm2.setName("broker");
    comm2.setServerMode(true);
    comm3.setName("test3");
    comm3.setServerMode(false);
    comm3.setBrokerPort(DEFAULT_TCPSS_PORT);

    comm2.setPortNumber(DEFAULT_TCPSS_PORT);

    guarded<helics::ActionMessage> act;
    guarded<helics::ActionMessage> act2;
    guarded<helics::ActionMessage> act3;

    comm.setCallback([&counter, &act](const helics::ActionMessage& m) {
        ++counter;
        act = m;
    });
    comm2.setCallback([&counter2, &act2](const helics::ActionMessage& m) {
        ++counter2;
        act2 = m;
    });
    comm3.setCallback([&counter3, &act3](const helics::ActionMessage& m) {
        ++counter3;
        act3 = m;
    });

    // need to launch the connection commands at the same time since they depend on each other in
    // this case auto connected_fut = std::async(std::launch::async, [&comm] {return comm.connect();
    // });

    bool connected = comm2.connect();
    ASSERT_TRUE(connected);
    // connected = connected_fut.get();
    connected = comm.connect();
    ASSERT_TRUE(connected);
    connected = comm3.connect();
    ASSERT_TRUE(connected);
    comm.transmit(helics::route_id(0), helics::CMD_ACK);

    std::this_thread::sleep_for(250ms);
    ASSERT_EQ(counter2, 1);
    EXPECT_TRUE(act2.lock()->action() == helics::action_message_def::action_t::cmd_ack);

    comm3.transmit(helics::route_id(0), helics::CMD_ACK);

    std::this_thread::sleep_for(250ms);
    ASSERT_EQ(counter2, 2);
    EXPECT_TRUE(act2.lock()->action() == helics::action_message_def::action_t::cmd_ack);

    comm2.addRoute(helics::route_id(3), comm3.getAddress());

    comm2.transmit(helics::route_id(3), helics::CMD_ACK);

    std::this_thread::sleep_for(250ms);
    if (counter3 != 1) {
        std::this_thread::sleep_for(250ms);
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

TEST(TcpSSCore, tcpSSCore_initialization)
{
    std::this_thread::sleep_for(400ms);
    std::atomic<int> counter{0};
    std::string initializationString = "-f 1 --name=core1";
    auto core = helics::CoreFactory::create(helics::core_type::TCP_SS, initializationString);

    ASSERT_TRUE(core);
    EXPECT_TRUE(core->isConfigured());
    auto srv = AsioContextManager::getContextPointer();
    auto contextLoop = srv->startContextLoop();

    auto server =
        helics::tcp::TcpServer::create(srv->getBaseContext(), "localhost", DEFAULT_TCPSS_PORT);
    std::vector<char> data(1024);
    std::atomic<size_t> len{0};
    server->setDataCall(
        [&data, &counter, &len](const helics::tcp::TcpConnection::pointer& /*unused*/,
                                const char* data_rec,
                                size_t data_Size) {
            std::copy(data_rec, data_rec + data_Size, data.begin() + len);
            len += data_Size;
            ++counter;
            return len.load();
        });
    auto started = server->start();

    EXPECT_TRUE(started);
    std::this_thread::sleep_for(100ms);
    bool connected = core->connect();
    EXPECT_TRUE(connected);

    if (connected) {
        int cnt = 0;
        while (counter == 0) {
            std::this_thread::sleep_for(100ms);
            ++cnt;
            if (cnt > 30) {
                break;
            }
        }
        EXPECT_GE(counter, 1);

        EXPECT_GT(len, 32U);
        helics::ActionMessage rM;
        helics::ActionMessage rM2;
        auto used = rM.depacketize(data.data(), static_cast<int>(len.load()));
        if (used < static_cast<int>(len.load())) {
            auto use2 = rM2.depacketize(data.data() + used, static_cast<int>(len.load() - used));
            if (use2 == 0) {
                while (counter != 2) {
                    std::this_thread::sleep_for(100ms);
                    ++cnt;
                    if (cnt > 30) {
                        break;
                    }
                }
            }
            rM2.depacketize(data.data() + used, static_cast<int>(len.load() - used));
        } else {
            while (counter != 2) {
                std::this_thread::sleep_for(100ms);
                ++cnt;
                if (cnt > 30) {
                    break;
                }
            }
            rM2.depacketize(data.data() + used, static_cast<int>(len.load() - used));
        }
        EXPECT_EQ(rM.name, "core1");
        EXPECT_TRUE(rM.action() == helics::action_message_def::action_t::cmd_protocol);

        EXPECT_EQ(rM2.name, "core1");
        EXPECT_TRUE(rM2.action() == helics::action_message_def::action_t::cmd_reg_broker);
    }
    core->disconnect();
    server->close();
    core = nullptr;
    helics::CoreFactory::cleanUpCores(100ms);
}

/** test case checks default values and makes sure they all mesh together
also tests the automatic port determination for cores
*/

TEST(TcpSSCore, tcpSSCore_core_broker_default)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    std::string initializationString = "-f 1";

    auto broker = helics::BrokerFactory::create(helics::core_type::TCP_SS, initializationString);
    ASSERT_TRUE(broker);
    auto core = helics::CoreFactory::create(helics::core_type::TCP_SS, initializationString);
    ASSERT_TRUE(core);
    bool connected = broker->isConnected();
    EXPECT_TRUE(connected);
    connected = core->connect();
    EXPECT_TRUE(connected);

    auto ccore = static_cast<helics::tcp::TcpCoreSS*>(core.get());
    // this will test the automatic port allocation
    EXPECT_EQ(ccore->getAddress(), ccore->getIdentifier());
    core->disconnect();
    broker->disconnect();
    core = nullptr;
    broker = nullptr;
    helics::CoreFactory::cleanUpCores(100ms);
    helics::BrokerFactory::cleanUpBrokers(100ms);
}

TEST(TcpSSCore, commFactory)
{
    auto comm = helics::CommFactory::create("tcpss");
    auto comm2 = helics::CommFactory::create(helics::core_type::TCP_SS);

    EXPECT_TRUE(dynamic_cast<helics::tcp::TcpCommsSS*>(comm.get()) != nullptr);
    EXPECT_TRUE(dynamic_cast<helics::tcp::TcpCommsSS*>(comm2.get()) != nullptr);
}
