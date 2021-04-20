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
#include "helics/network/zmq/ZmqBroker.h"
#include "helics/network/zmq/ZmqComms.h"
#include "helics/network/zmq/ZmqContextManager.h"
#include "helics/network/zmq/ZmqCore.h"
#include "helics/network/zmq/ZmqRequestSets.h"

#include "gtest/gtest.h"
#include <future>
#include <iostream>

using namespace std::literals::chrono_literals;

using helics::Core;
constexpr const char* defServer{"tcp://127.0.0.1:23406"};
constexpr const char* host{"tcp://127.0.0.1"};

constexpr const char* defRoute1{"tcp://127.0.0.1:23405"};

constexpr const char* defRoute2{"tcp://127.0.0.1:23407"};

TEST(ZMQCore, zmqComms_broker)
{
    std::atomic<int> counter{0};

    helics::zeromq::ZmqComms comm;
    comm.loadTargetInfo(host, host);

    auto ctx = ZmqContextManager::getContextPointer();
    zmq::socket_t repSocket(ctx->getContext(), ZMQ_REP);
    repSocket.setsockopt(ZMQ_LINGER, 200);
    repSocket.bind(defServer);

    comm.setCallback([&counter](const helics::ActionMessage& /*m*/) { ++counter; });
    comm.setBrokerPort(23405);
    comm.setName("tests");
    auto confut = std::async(std::launch::async, [&comm]() { return comm.connect(); });

    zmq::message_t rxmsg;

    repSocket.recv(rxmsg);

    EXPECT_GT(rxmsg.size(), 32U);

    helics::ActionMessage rM(static_cast<char*>(rxmsg.data()), rxmsg.size());
    EXPECT_TRUE(helics::isProtocolCommand(rM));
    rM.messageID = DISCONNECT;
    repSocket.send(rM.to_string());
    auto connected = confut.get();
    EXPECT_TRUE(!connected);
    repSocket.close();
}

/** test the request set class with various scenarios*/
TEST(ZMQCore, zmqRequestSet1)
{
    // sleep to clear any residual from the previous test
    std::this_thread::sleep_for(500ms);
    helics::zeromq::ZmqRequestSets reqset;

    auto ctx = ZmqContextManager::getContextPointer();
    zmq::socket_t repSocket1(ctx->getContext(), ZMQ_REP);
    repSocket1.bind(defServer);
    zmq::socket_t repSocket2(ctx->getContext(), ZMQ_REP);
    repSocket2.bind(defRoute1);
    zmq::socket_t repSocket3(ctx->getContext(), ZMQ_REP);
    repSocket3.bind(defRoute2);

    reqset.addRoutes(1, defServer);
    reqset.addRoutes(2, defRoute1);
    reqset.addRoutes(3, defRoute2);

    helics::ActionMessage M(helics::CMD_IGNORE);
    M.messageID = 1;

    reqset.transmit(1, M);
    EXPECT_TRUE(reqset.waiting());

    zmq::message_t msg;
    repSocket1.recv(msg);

    repSocket1.send(msg, zmq::send_flags::none);
    // should still be waiting
    EXPECT_TRUE(reqset.waiting());
    auto msgCnt = reqset.checkForMessages(100ms);

    EXPECT_TRUE(!reqset.waiting());
    EXPECT_EQ(msgCnt, 1);

    auto M2 = reqset.getMessage();

    EXPECT_TRUE(M2->action() == helics::CMD_IGNORE);

    // send two messages
    reqset.transmit(2, M);
    reqset.transmit(2, M);
    EXPECT_TRUE(reqset.waiting());

    repSocket2.recv(msg);

    repSocket2.send(msg, zmq::send_flags::none);
    reqset.checkForMessages(100ms);

    EXPECT_TRUE(reqset.waiting());
    repSocket2.recv(msg);

    repSocket2.send(msg, zmq::send_flags::none);
    reqset.checkForMessages(100ms);
    EXPECT_TRUE(!reqset.waiting());

    EXPECT_TRUE(reqset.hasMessages());
    repSocket1.close();
    repSocket2.close();
    repSocket3.close();
    reqset.close();
    std::this_thread::sleep_for(200ms);
}

/** test the request set class with various scenarios*/
TEST(ZMQCore, zmqRequestSet2)
{
    // sleep to clear any residual from the previous test
    std::this_thread::sleep_for(500ms);
    helics::zeromq::ZmqRequestSets reqset;

    auto ctx = ZmqContextManager::getContextPointer();
    zmq::socket_t repSocket1(ctx->getContext(), ZMQ_REP);
    repSocket1.bind(defServer);
    zmq::socket_t repSocket2(ctx->getContext(), ZMQ_REP);
    repSocket2.bind(defRoute1);
    zmq::socket_t repSocket3(ctx->getContext(), ZMQ_REP);
    repSocket3.bind(defRoute2);

    repSocket1.setsockopt(ZMQ_LINGER, 100);
    repSocket2.setsockopt(ZMQ_LINGER, 100);
    repSocket3.setsockopt(ZMQ_LINGER, 100);

    reqset.addRoutes(1, defServer);
    reqset.addRoutes(2, defRoute1);
    reqset.addRoutes(3, defRoute2);

    helics::ActionMessage M(helics::CMD_IGNORE);
    M.messageID = 1;

    reqset.transmit(1, M);
    reqset.transmit(2, M);
    reqset.transmit(3, M);
    zmq::message_t msg;
    repSocket1.recv(msg);

    repSocket1.send(msg, zmq::send_flags::none);

    repSocket2.recv(msg);

    repSocket2.send(msg, zmq::send_flags::none);
    repSocket3.recv(msg);

    repSocket3.send(msg, zmq::send_flags::none);

    // make sure the check receives all messages
    reqset.checkForMessages(50ms);
    if (reqset.waiting()) {
        reqset.checkForMessages(50ms);
    }
    // since we have 3 sockets we might have to do this twice since it returns immediately if it has
    // a message
    if (reqset.waiting()) {
        reqset.checkForMessages(50ms);
    }
    ASSERT_TRUE(!reqset.waiting());
    std::this_thread::yield();
    reqset.transmit(1, M);
    reqset.transmit(2, M);
    reqset.transmit(3, M);
    reqset.transmit(1, M);
    reqset.transmit(2, M);
    reqset.transmit(3, M);
    reqset.transmit(1, M);
    reqset.transmit(2, M);
    reqset.transmit(3, M);
    std::this_thread::yield();
    repSocket1.recv(msg);

    repSocket1.send(msg, zmq::send_flags::none);
    std::this_thread::yield();
    repSocket2.recv(msg);

    repSocket2.send(msg, zmq::send_flags::none);
    std::this_thread::yield();
    repSocket3.recv(msg);

    repSocket3.send(msg, zmq::send_flags::none);
    auto res = reqset.checkForMessages(400ms);
    if (res != 6) {
        auto res2 = reqset.checkForMessages(400ms);
        if ((res + res2 == 6) || (res2 == 6)) {
            res = 6;
        }
        if (res != 6) {
            res2 = reqset.checkForMessages(400ms);
            if ((res + res2 == 6) || (res2 == 6)) {
                res = 6;
            }
        }
        if (res != 6) {
            res2 = reqset.checkForMessages(400ms);
            if ((res + res2 == 6) || (res2 == 6)) {
                res = 6;
            }
        }
    }
    EXPECT_EQ(res, 6);
    repSocket1.close();
    repSocket2.close();
    repSocket3.close();
    reqset.close();
    std::this_thread::sleep_for(200ms);
}

TEST(ZMQCore, zmqComms_broker_test_transmit)
{
    // sleep to clear any residual from the previous test
    std::this_thread::sleep_for(500ms);
    std::atomic<int> counter{0};
    helics::zeromq::ZmqComms comm;
    comm.loadTargetInfo(host, host);

    auto ctx = ZmqContextManager::getContextPointer();
    zmq::socket_t repSocket(ctx->getContext(), ZMQ_REP);
    try {
        repSocket.bind(defServer);
    }
    catch (const zmq::error_t& ze) {
        std::cerr << "error repbind (broker test transmit) " << ze.what() << std::endl;
        std::this_thread::sleep_for(200ms);
        GTEST_FAIL() << "Unable to bind Socket";
    }
    repSocket.setsockopt(ZMQ_LINGER, 100);
    zmq::socket_t pullSocket(ctx->getContext(), ZMQ_PULL);
    try {
        pullSocket.bind(defRoute1);
    }
    catch (const zmq::error_t& ze) {
        std::cerr << "error pullbind (broker test transmit)" << ze.what() << std::endl;
        repSocket.close();
        std::this_thread::sleep_for(200ms);
        GTEST_FAIL() << "Unable to bind Socket";
    }

    pullSocket.setsockopt(ZMQ_LINGER, 100);
    comm.setCallback([&counter](const helics::ActionMessage& /*m*/) { ++counter; });
    comm.setBrokerPort(23405);
    comm.setPortNumber(23407);
    comm.setName("tests");
    bool connected = comm.connect();
    ASSERT_TRUE(connected);
    comm.transmit(helics::parent_route_id, helics::CMD_IGNORE);
    zmq::message_t rxmsg;

    pullSocket.recv(rxmsg);

    EXPECT_GT(rxmsg.size(), 32U);
    helics::ActionMessage rM(static_cast<char*>(rxmsg.data()), rxmsg.size());
    EXPECT_TRUE(rM.action() == helics::action_message_def::action_t::cmd_ignore);
    comm.disconnect();
    repSocket.close();
    pullSocket.close();
    std::this_thread::sleep_for(200ms);
}

TEST(ZMQCore, zmqComms_rx)
{
    // sleep to clear any residual from the previous test
    std::this_thread::sleep_for(400ms);
    std::atomic<int> counter{0};
    guarded<helics::ActionMessage> act;
    helics::zeromq::ZmqComms comm;
    comm.loadTargetInfo(host, host);

    auto ctx = ZmqContextManager::getContextPointer();
    zmq::socket_t repSocket(ctx->getContext(), ZMQ_REP);
    try {
        repSocket.bind(defServer);
    }
    catch (const zmq::error_t& ze) {
        std::cerr << "error repbind (zmqComms_rx_test) " << ze.what() << std::endl;
        std::this_thread::sleep_for(200ms);
        GTEST_FAIL() << "Unable to bind Socket";
    }
    repSocket.setsockopt(ZMQ_LINGER, 100);
    zmq::socket_t pullSocket(ctx->getContext(), ZMQ_PULL);
    try {
        pullSocket.bind(defRoute1);
    }
    catch (const zmq::error_t& ze) {
        std::cerr << "error pullbind (zmqComms_rx_test)" << ze.what() << std::endl;
        repSocket.close();
        std::this_thread::sleep_for(200ms);
        GTEST_FAIL() << "Unable to bind Socket";
    }
    pullSocket.setsockopt(ZMQ_LINGER, 100);
    comm.setBrokerPort(23405);
    comm.setPortNumber(23407);
    comm.setName("tests");
    comm.setCallback([&counter, &act](const helics::ActionMessage& m) {
        ++counter;
        act = m;
    });

    bool connected = comm.connect();
    EXPECT_TRUE(connected);

    zmq::socket_t pushSocket(ctx->getContext(), ZMQ_PUSH);
    pushSocket.connect("tcp://127.0.0.1:23407");

    helics::ActionMessage cmd(helics::CMD_ACK);
    std::string buffer = cmd.to_string();
    try {
        auto cnt = pushSocket.send(buffer, zmq::send_flags::dontwait);
        EXPECT_EQ(cnt, buffer.size());
    }
    catch (const zmq::error_t&) {
        GTEST_FAIL() << "Message failed to send";
    }

    std::this_thread::sleep_for(200ms);
    if (counter != 1) {
        std::this_thread::sleep_for(200ms);
    }
    EXPECT_EQ(counter, 1);
    EXPECT_TRUE(act.lock()->action() == helics::action_message_def::action_t::cmd_ack);
    comm.disconnect();
    repSocket.close();
    pullSocket.close();
    std::this_thread::sleep_for(200ms);
}

TEST(ZMQCore, zmqComm_transmit_through)
{
    // sleep to clear any residual from the previous test
    std::this_thread::sleep_for(300ms);
    std::atomic<int> counter{0};
    std::atomic<int> counter2{0};
    guarded<helics::ActionMessage> act;
    guarded<helics::ActionMessage> act2;

    helics::zeromq::ZmqComms comm;
    helics::zeromq::ZmqComms comm2;

    comm.loadTargetInfo(host, host);
    comm2.loadTargetInfo(host, "");

    comm.setBrokerPort(23405);
    comm.setName("tests");
    comm2.setName("test2");
    comm2.setPortNumber(23405);
    comm.setPortNumber(23407);

    comm.setCallback([&counter, &act](const helics::ActionMessage& m) {
        ++counter;
        act = m;
    });
    comm2.setCallback([&counter2, &act2](const helics::ActionMessage& m) {
        ++counter2;
        act2 = m;
    });

    bool connected = comm2.connect();
    EXPECT_TRUE(connected);
    connected = comm.connect();
    EXPECT_TRUE(connected);

    comm.transmit(helics::parent_route_id, helics::CMD_ACK);

    std::this_thread::sleep_for(250ms);
    if (counter2 != 1) {
        std::this_thread::sleep_for(500ms);
    }
    EXPECT_EQ(counter2, 1);
    if (counter2 == 1) {
        EXPECT_TRUE(act2.lock()->action() == helics::action_message_def::action_t::cmd_ack);
    }

    comm.disconnect();
    comm2.disconnect();
    std::this_thread::sleep_for(200ms);
}

TEST(ZMQCore, zmqComm_transmit_add_route)
{
    // sleep to clear any residual from the previous test
    std::this_thread::sleep_for(300ms);
    std::atomic<int> counter{0};
    std::atomic<int> counter2{0};
    std::atomic<int> counter3{0};
    helics::zeromq::ZmqComms comm;
    helics::zeromq::ZmqComms comm2;
    helics::zeromq::ZmqComms comm3;

    comm.loadTargetInfo(host, host);
    comm2.loadTargetInfo(host, std::string());
    comm3.loadTargetInfo(host, host);

    comm.setBrokerPort(23405);
    comm.setName("tests");
    comm2.setName("broker");
    comm3.setName("test3");
    comm3.setBrokerPort(23405);

    comm2.setPortNumber(23405);
    comm.setPortNumber(23407);
    comm3.setPortNumber(23409);

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

    bool connected = comm2.connect();
    ASSERT_TRUE(connected);
    connected = comm.connect();
    // connected = comm.connect ();
    ASSERT_TRUE(connected);
    connected = comm3.connect();
    ASSERT_TRUE(connected);

    comm.transmit(helics::parent_route_id, helics::CMD_ACK);

    std::this_thread::yield();
    if (counter2 != 1) {
        std::this_thread::sleep_for(200ms);
    }
    if (counter2 != 1) {
        std::this_thread::sleep_for(200ms);
    }
    if (counter2 != 1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    ASSERT_EQ(counter2, 1);
    EXPECT_TRUE(act2.lock()->action() == helics::action_message_def::action_t::cmd_ack);

    comm3.transmit(helics::parent_route_id, helics::CMD_ACK);

    std::this_thread::yield();
    if (counter2 != 2) {
        std::this_thread::sleep_for(200ms);
    }
    if (counter2 != 2) {
        std::this_thread::sleep_for(200ms);
    }
    ASSERT_EQ(counter2, 2);
    EXPECT_TRUE(act2.lock()->action() == helics::action_message_def::action_t::cmd_ack);

    comm2.addRoute(helics::route_id(3), comm3.getAddress());

    comm2.transmit(helics::route_id(3), helics::CMD_ACK);

    std::this_thread::yield();
    int lcnt = 0;
    while (counter3 != 1) {
        std::this_thread::sleep_for(200ms);
        ++lcnt;
        if (lcnt > 10) {
            break;
        }
    }

    ASSERT_EQ(counter3, 1);
    EXPECT_TRUE(act3.lock()->action() == helics::action_message_def::action_t::cmd_ack);

    comm2.addRoute(helics::route_id(4), comm.getAddress());

    comm2.transmit(helics::route_id(4), helics::CMD_ACK);

    std::this_thread::yield();
    while (counter != 1) {
        std::this_thread::sleep_for(200ms);
    }

    ASSERT_EQ(counter, 1);
    EXPECT_TRUE(act.lock()->action() == helics::action_message_def::action_t::cmd_ack);

    comm.disconnect();
    comm2.disconnect();
    comm3.disconnect();
    std::this_thread::sleep_for(200ms);
}

TEST(ZMQCore, zmqCore_initialization)
{
    // sleep to clear any residual from the previous test
    std::this_thread::sleep_for(400ms);
    std::string initializationString =
        "-f 1 --brokerport=23405 --port=23410 --local_interface=tcp://127.0.0.1 --name=core1";
    auto core = helics::CoreFactory::create(helics::core_type::ZMQ, initializationString);

    ASSERT_TRUE(core != nullptr);
    EXPECT_TRUE(core->isConfigured());
    auto ctx = ZmqContextManager::getContextPointer();
    zmq::socket_t repSocket(ctx->getContext(), ZMQ_REP);
    try {
        repSocket.bind(defServer);
    }
    catch (const zmq::error_t& ze) {
        std::cerr << "error repbind (zmqCore_initialization_test) " << ze.what() << std::endl;
        std::this_thread::sleep_for(200ms);
        GTEST_FAIL() << "Unable to bind Socket";
    }
    repSocket.setsockopt(ZMQ_LINGER, 100);
    zmq::socket_t pullSocket(ctx->getContext(), ZMQ_PULL);
    try {
        pullSocket.bind(defRoute1);
    }
    catch (const zmq::error_t& ze) {
        std::cerr << "error pullbind (zmqCore_initialization_test)" << ze.what() << std::endl;
        repSocket.close();
        std::this_thread::sleep_for(200ms);
        GTEST_FAIL() << "Unable to bind Socket";
    }
    pullSocket.setsockopt(ZMQ_LINGER, 100);
    bool connected = core->connect();
    ASSERT_TRUE(connected);

    zmq::message_t rxmsg;

    pullSocket.recv(rxmsg);

    EXPECT_GT(rxmsg.size(), 32U);
    helics::ActionMessage rM(static_cast<char*>(rxmsg.data()), rxmsg.size());

    EXPECT_EQ(rM.name, "core1");
    EXPECT_TRUE(rM.action() == helics::action_message_def::action_t::cmd_reg_broker);

    repSocket.close();
    pullSocket.close();
    core->disconnect();
    core = nullptr;
    helics::CoreFactory::cleanUpCores(200ms);
    std::this_thread::sleep_for(200ms);
}

/** test case checks default values and makes sure they all mesh together
also tests the automatic port determination for cores
*/
TEST(ZMQCore, zmqCore_core_broker_default)
{
    // sleep to clear any residual from the previous test
    std::this_thread::sleep_for(400ms);
    std::string initializationString = "-f 1";

    auto broker = helics::BrokerFactory::create(helics::core_type::ZMQ, initializationString);

    auto core = helics::CoreFactory::create(helics::core_type::ZMQ, initializationString);
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

TEST(ZMQCore, commFactory)
{
    auto comm = helics::CommFactory::create("zmq");
    auto comm2 = helics::CommFactory::create(helics::core_type::ZMQ);

    EXPECT_TRUE(dynamic_cast<helics::zeromq::ZmqComms*>(comm.get()) != nullptr);
    EXPECT_TRUE(dynamic_cast<helics::zeromq::ZmqComms*>(comm2.get()) != nullptr);
}
