/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "cppzmq/zmq.hpp"
#include "helics/common/GuardedTypes.hpp"
#include "helics/core/ActionMessage.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/core-types.hpp"
#include "helics/network/networkDefaults.hpp"
#include "helics/network/zmq/ZmqBroker.h"
#include "helics/network/zmq/ZmqCommsSS.h"
#include "helics/network/zmq/ZmqContextManager.h"
#include "helics/network/zmq/ZmqCore.h"

#include "gtest/gtest.h"
#include <future>
#include <iostream>

using namespace std::literals::chrono_literals;

using helics::Core;

constexpr const char* host = "tcp://127.0.0.1";

TEST(ZMQSSCore, transmit)
{
    std::this_thread::sleep_for(400ms);
    std::atomic<int> counter{0};
    std::atomic<int> counter2{0};
    guarded<helics::ActionMessage> act;
    guarded<helics::ActionMessage> act2;

    helics::zeromq::ZmqCommsSS comm;
    helics::zeromq::ZmqCommsSS comm2;
    comm.loadTargetInfo(host, host);
    // comm2 is the broker
    comm2.loadTargetInfo(host, std::string{});

    comm.setBrokerPort(DEFAULT_ZMQSS_BROKER_PORT_NUMBER);
    comm.setName("test_comms");
    comm.setServerMode(false);
    comm2.setName("test_broker");
    comm2.setPortNumber(DEFAULT_ZMQSS_BROKER_PORT_NUMBER);
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
    bool connected2 = comm2.connect();
    ASSERT_TRUE(connected2);
    bool connected1 = connected_fut.get();
    if (!connected1) {  // lets just try again if it is not connected
        connected1 = comm.connect();
    }
    ASSERT_TRUE(connected1);

    comm.transmit(helics::parent_route_id, helics::CMD_ACK);
    std::this_thread::sleep_for(250ms);
    if (counter2 != 1) {
        std::this_thread::sleep_for(500ms);
    }
    ASSERT_EQ(counter2, 1);
    EXPECT_TRUE(act2.lock()->action() == helics::action_message_def::action_t::cmd_ack);

    comm2.disconnect();
    EXPECT_FALSE(comm2.isConnected());
    comm.disconnect();
    EXPECT_FALSE(comm.isConnected());

    std::this_thread::sleep_for(100ms);
}

TEST(ZMQSSCore, addroute)
{
    std::this_thread::sleep_for(400ms);
    std::atomic<int> counter{0};
    std::atomic<int> counter2{0};
    std::atomic<int> counter3{0};
    guarded<helics::ActionMessage> act;
    guarded<helics::ActionMessage> act2;
    guarded<helics::ActionMessage> act3;

    helics::zeromq::ZmqCommsSS comm;
    helics::zeromq::ZmqCommsSS comm2;
    helics::zeromq::ZmqCommsSS comm3;
    comm.loadTargetInfo(host, host);
    comm2.loadTargetInfo(host, host);
    // comm3 is the broker
    comm3.loadTargetInfo(host, std::string());

    comm.setBrokerPort(DEFAULT_ZMQSS_BROKER_PORT_NUMBER);
    comm.setName("test1");
    comm.setServerMode(false);

    comm2.setBrokerPort(DEFAULT_ZMQSS_BROKER_PORT_NUMBER);
    comm2.setName("test2");
    comm2.setServerMode(false);

    comm3.setName("test_broker");
    comm3.setPortNumber(DEFAULT_ZMQSS_BROKER_PORT_NUMBER);
    comm3.setServerMode(true);

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
    // this case
    auto connected_fut = std::async(std::launch::async, [&comm] { return comm.connect(); });
    auto connected_fut2 = std::async(std::launch::async, [&comm2] { return comm2.connect(); });
    bool connected1 = comm3.connect();
    ASSERT_TRUE(connected1);
    bool connected2 = connected_fut.get();
    if (!connected2) {  // lets just try again if it is not connected
        connected2 = comm.connect();
    }
    ASSERT_TRUE(connected2);
    connected2 = connected_fut2.get();
    if (!connected2) {  // lets just try again if it is not connected
        connected2 = comm2.connect();
    }
    ASSERT_TRUE(connected2);
    comm.transmit(helics::parent_route_id, helics::CMD_ACK);
    std::this_thread::sleep_for(250ms);
    if (counter3 != 1) {
        std::this_thread::sleep_for(500ms);
    }
    ASSERT_EQ(counter3, 1);
    comm3.addRoute(helics::route_id(2), comm2.getAddress());
    comm3.transmit(helics::route_id(2), helics::CMD_ACK);
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
    comm3.disconnect();
    EXPECT_TRUE(!comm3.isConnected());
    std::this_thread::sleep_for(100ms);
}

TEST(ZMQSSCore, initialization)
{
    std::atomic<int> counter{0};
    std::vector<helics::ActionMessage> msgs;
    helics::zeromq::ZmqCommsSS comm;
    std::mutex msgLock;
    comm.loadTargetInfo(host, std::string());
    comm.setName("test_broker");
    comm.setPortNumber(DEFAULT_ZMQSS_BROKER_PORT_NUMBER);
    comm.setServerMode(true);
    comm.setCallback([&counter, &msgs, &msgLock](const helics::ActionMessage& m) {
        ++counter;
        std::lock_guard<std::mutex> lock(msgLock);
        msgs.push_back(m);
    });
    comm.connect();

    std::string initializationString = "--name=core1";
    auto core = helics::CoreFactory::create(helics::core_type::ZMQ_SS, initializationString);

    ASSERT_TRUE(core);
    EXPECT_TRUE(core->isConfigured());

    std::this_thread::sleep_for(100ms);
    bool connected = core->connect();
    EXPECT_TRUE(connected);

    if (connected) {
        int cnt{0};
        while (counter == 0) {
            std::this_thread::sleep_for(100ms);
            ++cnt;
            if (cnt > 30) {
                break;
            }
        }
        EXPECT_GE(counter, 1);
        std::unique_lock<std::mutex> mLock(msgLock);
        if (!msgs.empty()) {
            auto rM2 = msgs.at(0);
            mLock.unlock();
            EXPECT_EQ(rM2.name, "core1");
            // std::cout << "rM.name: " << rM2.name << std::endl;
            EXPECT_TRUE(rM2.action() == helics::action_message_def::action_t::cmd_reg_broker);
        } else {
            mLock.unlock();
        }
    }
    core->disconnect();
    comm.disconnect();
    core = nullptr;
    msgs.clear();
    helics::CoreFactory::cleanUpCores(100ms);
}

/** test case checks default values and makes sure they all mesh together
also tests the automatic port determination for cores
*/
TEST(ZMQSSCore, core_broker_default)
{
    std::string initializationString = "-f 1";

    auto broker = helics::BrokerFactory::create(helics::core_type::ZMQ_SS, initializationString);

    auto core = helics::CoreFactory::create(helics::core_type::ZMQ_SS, initializationString);
    bool connected = broker->isConnected();
    EXPECT_TRUE(connected);
    connected = core->connect();
    EXPECT_TRUE(connected);

    core->disconnect();

    EXPECT_TRUE(!core->isConnected());
    broker->disconnect();
    EXPECT_TRUE(!broker->isConnected());
    helics::CoreFactory::cleanUpCores(200ms);
    helics::BrokerFactory::cleanUpBrokers(200ms);
}

TEST(ZMQSSCore, commFactory)
{
    auto comm = helics::CommFactory::create("zmqss");
    auto comm2 = helics::CommFactory::create(helics::core_type::ZMQ_SS);

    EXPECT_TRUE(dynamic_cast<helics::zeromq::ZmqCommsSS*>(comm.get()) != nullptr);
    EXPECT_TRUE(dynamic_cast<helics::zeromq::ZmqCommsSS*>(comm2.get()) != nullptr);
}
