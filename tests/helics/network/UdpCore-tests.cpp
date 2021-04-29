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
#include "helics/network/udp/UdpBroker.h"
#include "helics/network/udp/UdpComms.h"
#include "helics/network/udp/UdpCore.h"

#include "gtest/gtest.h"
#include <asio/ip/udp.hpp>
#include <future>

using namespace std::literals::chrono_literals;

using asio::ip::udp;
using helics::Core;

#define UDP_BROKER_PORT 23901
#define UDP_SECONDARY_PORT 23905
TEST(UdpCore, udpComms_broker)
{
    std::atomic<int> counter{0};
    std::string host = "localhost";
    helics::udp::UdpComms comm;
    comm.loadTargetInfo(host, host);

    auto srv = AsioContextManager::getContextPointer();

    udp::socket rxSocket(AsioContextManager::getContext(), udp::endpoint(udp::v4(), 23901));

    comm.setCallback([&counter](const helics::ActionMessage& /*m*/) { ++counter; });
    comm.setBrokerPort(UDP_BROKER_PORT);
    comm.setName("tests");

    auto confut = std::async(std::launch::async, [&comm]() { return comm.connect(); });

    std::vector<char> data(1024);

    udp::endpoint remote_endpoint;
    asio::error_code error;
    auto len = rxSocket.receive_from(asio::buffer(data), remote_endpoint, 0, error);

    EXPECT_TRUE(!error);
    EXPECT_GT(len, 32U);

    helics::ActionMessage rM(data.data(), len);
    EXPECT_TRUE(helics::isProtocolCommand(rM));
    rM.messageID = DISCONNECT;
    rxSocket.send_to(asio::buffer(rM.to_string()), remote_endpoint, 0, error);
    EXPECT_TRUE(!error);
    auto connected = confut.get();
    EXPECT_TRUE(!connected);
    rxSocket.close();
    comm.disconnect();
    std::this_thread::sleep_for(100ms);
}

TEST(UdpCore, udpComms_broker_test_transmit)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::atomic<int> counter{0};
    std::string host = "localhost";
    helics::udp::UdpComms comm;
    comm.loadTargetInfo(host, host);
    auto srv = AsioContextManager::getContextPointer();

    udp::socket rxSocket(AsioContextManager::getContext(), udp::endpoint(udp::v4(), 23901));

    EXPECT_TRUE(rxSocket.is_open());
    comm.setCallback([&counter](const helics::ActionMessage& /*m*/) { ++counter; });
    comm.setBrokerPort(UDP_BROKER_PORT);
    comm.setPortNumber(UDP_SECONDARY_PORT);
    comm.setName("tests");
    comm.setFlag("noack_connect", true);
    bool connected = comm.connect();
    ASSERT_TRUE(connected);
    comm.transmit(helics::parent_route_id, helics::CMD_IGNORE);

    std::vector<char> data(1024);

    udp::endpoint remote_endpoint;
    asio::error_code error;
    auto len = rxSocket.receive_from(asio::buffer(data), remote_endpoint, 0, error);

    EXPECT_GT(len, 32U);
    helics::ActionMessage rM(data.data(), len);
    EXPECT_TRUE(rM.action() == helics::action_message_def::action_t::cmd_ignore);
    rxSocket.close();
    comm.disconnect();
    std::this_thread::sleep_for(100ms);
}

TEST(UdpCore, udpComms_rx)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::atomic<int> counter{0};
    guarded<helics::ActionMessage> act;
    std::string host = "localhost";
    helics::udp::UdpComms comm;
    comm.loadTargetInfo(host, host);

    auto srv = AsioContextManager::getContextPointer();

    udp::resolver resolver(AsioContextManager::getContext());
    udp::socket rxSocket(AsioContextManager::getContext(), udp::endpoint(udp::v4(), 23901));

    EXPECT_TRUE(rxSocket.is_open());
    comm.setCallback([&counter, &act](const helics::ActionMessage& m) {
        ++counter;
        act = m;
    });
    comm.setBrokerPort(UDP_BROKER_PORT);
    comm.setPortNumber(23903);
    comm.setName("tests");
    comm.setFlag("noack_connect", true);
    bool connected = comm.connect();
    ASSERT_TRUE(connected);

    udp::resolver::query queryNew(udp::v4(), "localhost", "23903");

    auto txendpoint = *resolver.resolve(queryNew);

    helics::ActionMessage cmd(helics::CMD_ACK);
    std::string buffer = cmd.to_string();

    auto cnt = rxSocket.send_to(asio::buffer(buffer), txendpoint);
    ASSERT_EQ(cnt, buffer.size());

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    ASSERT_EQ(counter, 1);
    EXPECT_TRUE(act.lock()->action() == helics::action_message_def::action_t::cmd_ack);
    rxSocket.close();
    comm.disconnect();
    std::this_thread::sleep_for(100ms);
}

TEST(UdpCore, udpComm_transmit_through)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::atomic<int> counter{0};
    std::atomic<int> counter2{0};
    guarded<helics::ActionMessage> act;
    guarded<helics::ActionMessage> act2;

    std::string host = "localhost";
    helics::udp::UdpComms comm;
    comm.loadTargetInfo(host, host);
    helics::udp::UdpComms comm2;
    comm2.loadTargetInfo(host, "");

    comm.setBrokerPort(UDP_BROKER_PORT);
    comm.setName("tests");
    comm2.setName("test2");
    comm2.setPortNumber(UDP_BROKER_PORT);
    comm.setPortNumber(UDP_SECONDARY_PORT);

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

    bool connected = comm2.connect();
    ASSERT_TRUE(connected);
    connected = connected_fut.get();
    ASSERT_TRUE(connected);

    comm.transmit(helics::parent_route_id, helics::CMD_ACK);

    std::this_thread::sleep_for(250ms);
    if (counter2 != 1) {
        std::this_thread::sleep_for(500ms);
    }
    ASSERT_EQ(counter2, 1);
    EXPECT_TRUE(act2.lock()->action() == helics::action_message_def::action_t::cmd_ack);

    comm.disconnect();
    comm2.disconnect();
    std::this_thread::sleep_for(100ms);
}

TEST(UdpCore, udpComm_transmit_add_route)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::atomic<int> counter{0};
    std::atomic<int> counter2{0};
    std::atomic<int> counter3{0};

    std::string host = "localhost";
    helics::udp::UdpComms comm;
    helics::udp::UdpComms comm2;
    helics::udp::UdpComms comm3;
    comm.loadTargetInfo(host, host);
    comm2.loadTargetInfo(host, "");
    comm3.loadTargetInfo(host, host);

    comm.setBrokerPort(UDP_BROKER_PORT);
    comm.setName("tests");
    comm2.setName("broker");
    comm3.setName("test3");
    comm3.setBrokerPort(UDP_BROKER_PORT);

    comm2.setPortNumber(UDP_BROKER_PORT);
    comm.setPortNumber(UDP_SECONDARY_PORT);
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

    // need to launch the connection commands at the same time since they depend on each other in
    // this case auto connected_fut = std::async(std::launch::async, [&comm] {return comm.connect();
    // });

    bool connected = comm2.connect();
    ASSERT_TRUE(connected);
    // connected = connected_fut.get();
    connected = comm.connect();
    ASSERT_TRUE(connected);
    connected = comm3.connect();
    EXPECT_TRUE(connected);
    comm.transmit(helics::route_id(0), helics::CMD_ACK);

    std::this_thread::sleep_for(250ms);
    ASSERT_EQ(counter2, 1);
    EXPECT_TRUE(act2.lock()->action() == helics::action_message_def::action_t::cmd_ack);

    comm3.transmit(helics::route_id(0), helics::CMD_ACK);

    std::this_thread::sleep_for(250ms);
    ASSERT_EQ(counter2, 2);
    EXPECT_TRUE(act2.lock()->action() == helics::action_message_def::action_t::cmd_ack);

    comm2.addRoute(helics::route_id(3), comm3.getAddress());
    std::this_thread::sleep_for(250ms);
    comm2.transmit(helics::route_id(3), helics::CMD_ACK);

    std::this_thread::sleep_for(250ms);
    if (counter3 != 1) {
        std::this_thread::sleep_for(250ms);
    }
    EXPECT_EQ(counter3, 1);
    if (counter3 == 1) {  // previous test was an assert but that can trigger some out of scope
                          // errors if we don't disconnect property
        // so only do these test actions if the previous test has passed.
        EXPECT_TRUE(act3.lock()->action() == helics::action_message_def::action_t::cmd_ack);

        comm2.addRoute(helics::route_id(4), comm.getAddress());

        comm2.transmit(helics::route_id(4), helics::CMD_ACK);

        std::this_thread::sleep_for(250ms);
        ASSERT_EQ(counter, 1);
        EXPECT_TRUE(act.lock()->action() == helics::action_message_def::action_t::cmd_ack);
    }

    comm.disconnect();
    comm2.disconnect();
    comm3.disconnect();
    std::this_thread::sleep_for(100ms);
}

TEST(UdpCore, udpCore_initialization)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::string initializationString =
        "-f 1 --brokerport=23901  --port=23950 --local_interface=localhost --name=core1 --noack_connect";
    auto core = helics::CoreFactory::create(helics::core_type::UDP, initializationString);

    ASSERT_TRUE(core != nullptr);
    EXPECT_TRUE(core->isConfigured());
    auto srv = AsioContextManager::getContextPointer();
    udp::socket rxSocket(AsioContextManager::getContext(), udp::endpoint(udp::v4(), 23901));

    EXPECT_TRUE(rxSocket.is_open());

    bool connected = core->connect();
    ASSERT_TRUE(connected);

    std::vector<char> data(1024);

    udp::endpoint remote_endpoint;
    asio::error_code error;

    auto len = rxSocket.receive_from(asio::buffer(data), remote_endpoint, 0, error);
    ASSERT_TRUE(!error);
    EXPECT_GT(len, 32U);
    helics::ActionMessage rM(data.data(), len);

    EXPECT_EQ(rM.name, "core1");
    EXPECT_TRUE(rM.action() == helics::action_message_def::action_t::cmd_reg_broker);
    helics::ActionMessage resp(helics::CMD_PRIORITY_ACK);
    rxSocket.send_to(asio::buffer(resp.to_string()), remote_endpoint, 0, error);
    EXPECT_TRUE(!error);
    core->disconnect();
    core = nullptr;
    helics::CoreFactory::cleanUpCores(100ms);
    srv.reset();
}

/** test case checks default values and makes sure they all mesh together
also tests the automatic port determination for cores
*/

TEST(UdpCore, udpCore_core_broker_default)
{
    std::this_thread::sleep_for(500ms);
    std::string initializationString = "-f 1";

    auto broker = helics::BrokerFactory::create(helics::core_type::UDP, initializationString);

    auto core = helics::CoreFactory::create(helics::core_type::UDP, initializationString);
    bool connected = broker->isConnected();
    EXPECT_TRUE(connected);
    connected = core->connect();
    EXPECT_TRUE(connected);

    auto ccore = static_cast<helics::udp::UdpCore*>(core.get());
    // this will test the automatic port allocation
    EXPECT_EQ(ccore->getAddress(), "localhost:23921");
    core->disconnect();
    broker->disconnect();
    core = nullptr;
    broker = nullptr;
    helics::CoreFactory::cleanUpCores(100ms);
    helics::BrokerFactory::cleanUpBrokers(100ms);
}

TEST(UdpCore, commFactory)
{
    auto comm = helics::CommFactory::create("udp");
    auto comm2 = helics::CommFactory::create(helics::core_type::UDP);

    EXPECT_TRUE(dynamic_cast<helics::udp::UdpComms*>(comm.get()) != nullptr);
    EXPECT_TRUE(dynamic_cast<helics::udp::UdpComms*>(comm2.get()) != nullptr);
}
