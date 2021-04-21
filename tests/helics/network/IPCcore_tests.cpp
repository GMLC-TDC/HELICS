/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/common/GuardedTypes.hpp"
#include "helics/core/ActionMessage.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/CoreBroker.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/core-types.hpp"
#include "helics/network/ipc/IpcComms.h"
#include "helics/network/ipc/IpcCore.h"
#include "helics/network/ipc/IpcQueueHelper.h"

#include <boost/interprocess/ipc/message_queue.hpp>
#include <future>
#include <gtest/gtest.h>

using namespace std::literals::chrono_literals;

TEST(IPCCore, ipccomms_broker)
{
    std::atomic<int> counter{0};
    std::string brokerLoc = "brokerIPC";
    std::string localLoc = "localIPC";
    helics::ipc::IpcComms comm;
    comm.loadTargetInfo(localLoc, brokerLoc);

    helics::ipc::OwnedQueue mq;
    bool mqConn = mq.connect(brokerLoc, 1024, 1024);
    ASSERT_TRUE(mqConn);

    comm.setCallback([&counter](const helics::ActionMessage& /*m*/) { ++counter; });

    bool connected = comm.connect();
    ASSERT_TRUE(connected);
    comm.transmit(helics::parent_route_id, helics::CMD_IGNORE);

    helics::ActionMessage rM = mq.getMessage();
    EXPECT_TRUE(rM.action() == helics::action_message_def::action_t::cmd_ignore);
    comm.disconnect();
    std::this_thread::sleep_for(100ms);
}

TEST(IPCCore, ipccomms_rx)
{
    std::atomic<int> counter{0};
    guarded<helics::ActionMessage> act;
    std::string brokerLoc;
    std::string localLoc = "localIPC";
    helics::ipc::IpcComms comm;
    comm.loadTargetInfo(localLoc, brokerLoc);

    comm.setCallback([&counter, &act](const helics::ActionMessage& m) {
        ++counter;
        act = m;
    });

    bool connected = comm.connect();
    ASSERT_TRUE(connected);
    helics::ipc::SendToQueue mq;
    mq.connect(localLoc, true, 2);

    helics::ActionMessage cmd(helics::CMD_ACK);

    mq.sendMessage(cmd, 1);
    std::this_thread::sleep_for(250ms);
    ASSERT_EQ(counter, 1);
    EXPECT_TRUE(act.lock()->action() == helics::action_message_def::action_t::cmd_ack);
    comm.disconnect();
    std::this_thread::sleep_for(100ms);
}

TEST(IPCCore, ipcComm_transmit_through)
{
    std::atomic<int> counter{0};
    std::string brokerLoc = "brokerIPC";
    std::string localLoc = "localIPC";
    // just to make sure these are not already present from a failure
    std::atomic<int> counter2{0};
    guarded<helics::ActionMessage> act;
    guarded<helics::ActionMessage> act2;

    helics::ipc::IpcComms comm;
    comm.loadTargetInfo(localLoc, brokerLoc);
    helics::ipc::IpcComms comm2;
    comm2.loadTargetInfo(brokerLoc, std::string());

    comm.setCallback([&counter, &act](const helics::ActionMessage& m) {
        ++counter;
        act = m;
    });
    comm2.setCallback([&counter2, &act2](const helics::ActionMessage& m) {
        ++counter2;
        act2 = m;
    });

    // need to launch the connection commands at the same time since they depend on each other in
    // this case auto connected_fut = std::async(std::launch::async, [&comm] {return comm.connect();
    // });

    bool connected = comm2.connect();
    ASSERT_TRUE(connected);
    // connected = connected_fut.get();
    connected = comm.connect();
    ASSERT_TRUE(connected);

    comm.transmit(helics::parent_route_id, helics::CMD_ACK);

    std::this_thread::sleep_for(250ms);
    ASSERT_EQ(counter2, 1);
    EXPECT_TRUE(act2.lock()->action() == helics::action_message_def::action_t::cmd_ack);

    comm.disconnect();
    comm2.disconnect();
    std::this_thread::sleep_for(100ms);
}

TEST(IPCCore, ipcComm_transmit_add_route)
{
    std::atomic<int> counter{0};
    std::string brokerLoc = "brokerIPC";
    std::string localLoc = "localIPC";
    std::string localLocB = "localIPC2";
    // just to make sure these are not already present from a failure
    boost::interprocess::message_queue::remove(brokerLoc.c_str());
    boost::interprocess::message_queue::remove(localLoc.c_str());
    boost::interprocess::message_queue::remove(localLocB.c_str());

    std::atomic<int> counter2{0};
    std::atomic<int> counter3{0};
    guarded<helics::ActionMessage> act;
    guarded<helics::ActionMessage> act2;
    guarded<helics::ActionMessage> act3;

    helics::ipc::IpcComms comm;
    helics::ipc::IpcComms comm2;
    helics::ipc::IpcComms comm3;
    comm.loadTargetInfo(localLoc, brokerLoc);

    comm2.loadTargetInfo(brokerLoc, std::string());
    comm3.loadTargetInfo(localLocB, brokerLoc);

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
    std::this_thread::sleep_for(100ms);
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
    std::this_thread::sleep_for(100ms);
    comm.transmit(helics::parent_route_id, helics::CMD_ACK);

    std::this_thread::sleep_for(100ms);
    if (counter2 != 1) {
        std::this_thread::sleep_for(350ms);
    }
    ASSERT_EQ(counter2, 1);
    EXPECT_TRUE(act2.lock()->action() == helics::action_message_def::action_t::cmd_ack);

    comm3.transmit(helics::parent_route_id, helics::CMD_ACK);

    std::this_thread::sleep_for(100ms);
    if (counter2 != 1) {
        std::this_thread::sleep_for(350ms);
    }
    ASSERT_EQ(counter2, 2);
    EXPECT_TRUE(act2.lock()->action() == helics::action_message_def::action_t::cmd_ack);

    comm2.addRoute(helics::route_id(3), localLocB);
    std::this_thread::sleep_for(100ms);
    comm2.transmit(helics::route_id(3), helics::CMD_ACK);

    std::this_thread::sleep_for(100ms);
    if (counter3 != 1) {
        std::this_thread::sleep_for(350ms);
    }
    if (counter3 != 1) {
        std::cout << "ipc core extra sleep required\n";
        std::this_thread::sleep_for(350ms);
    }
    ASSERT_EQ(counter3, 1);
    EXPECT_TRUE(act3.lock()->action() == helics::action_message_def::action_t::cmd_ack);

    comm2.addRoute(helics::route_id(4), localLoc);
    std::this_thread::sleep_for(200ms);
    comm2.transmit(helics::route_id(4), helics::CMD_ACK);

    std::this_thread::sleep_for(100ms);
    if (counter.load() != 1) {
        std::this_thread::sleep_for(350ms);
    }
    ASSERT_EQ(counter.load(), 1);
    EXPECT_TRUE(act.lock()->action() == helics::action_message_def::action_t::cmd_ack);

    comm.disconnect();
    comm2.disconnect();
    comm3.disconnect();
    std::this_thread::sleep_for(100ms);
}

TEST(IPCCore, ipccore_initialization)
{
    std::string initializationString = "--broker_address=testBroker --name=core1";
    auto core = helics::CoreFactory::create(helics::core_type::INTERPROCESS, initializationString);

    ASSERT_TRUE(core != nullptr);
    EXPECT_TRUE(core->isConfigured());

    helics::ipc::OwnedQueue mq;
    bool mqConn = mq.connect("testBroker", 1024, 1024);
    ASSERT_TRUE(mqConn);

    bool crConn = core->connect();
    ASSERT_TRUE(crConn);

    helics::ActionMessage rM = mq.getMessage();
    EXPECT_EQ(rM.name, "core1");
    EXPECT_TRUE(rM.action() == helics::action_message_def::action_t::cmd_reg_broker);
    core->disconnect();
    core = nullptr;
    boost::interprocess::message_queue::remove("testbroker");
    helics::CoreFactory::cleanUpCores(100ms);
}

/** test case checks default values and makes sure they all mesh together
also tests the automatic port determination for cores
*/
TEST(IPCCore, ipcCore_core_broker_default)
{
    std::string initializationString = "-f 1";

    auto broker =
        helics::BrokerFactory::create(helics::core_type::INTERPROCESS, initializationString);

    auto core = helics::CoreFactory::create(helics::core_type::IPC, initializationString);
    bool connected = broker->isConnected();
    EXPECT_TRUE(connected);
    connected = core->connect();
    EXPECT_TRUE(connected);

    core->disconnect();
    broker->disconnect();
    core = nullptr;
    broker = nullptr;
    helics::CoreFactory::cleanUpCores(100ms);
    helics::BrokerFactory::cleanUpBrokers(100ms);
}

TEST(IPCCore, commFactory)
{
    auto comm = helics::CommFactory::create("ipc");
    auto comm2 = helics::CommFactory::create(helics::core_type::IPC);
    auto comm3 = helics::CommFactory::create(helics::core_type::INTERPROCESS);

    EXPECT_TRUE(dynamic_cast<helics::ipc::IpcComms*>(comm.get()) != nullptr);
    EXPECT_TRUE(dynamic_cast<helics::ipc::IpcComms*>(comm2.get()) != nullptr);
    EXPECT_TRUE(dynamic_cast<helics::ipc::IpcComms*>(comm3.get()) != nullptr);
}
