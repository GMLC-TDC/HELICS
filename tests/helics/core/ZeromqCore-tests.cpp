/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include <boost/test/unit_test.hpp>

#include "helics/common/cppzmq/zmq.hpp"
#include "helics/common/zmqContextManager.h"
#include "helics/core/ActionMessage.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/core-types.hpp"
#include "helics/core/zmq/ZmqBroker.h"
#include "helics/core/zmq/ZmqComms.h"
#include "helics/core/zmq/ZmqCore.h"
#include "helics/core/zmq/ZmqRequestSets.h"
#include "helics/common/GuardedTypes.hpp"
//#include "boost/process.hpp"
#include <future>

namespace utf = boost::unit_test;
using namespace std::literals::chrono_literals;

BOOST_AUTO_TEST_SUITE (ZMQCore_tests, *utf::label("ci"))

using helics::Core;
const std::string defServer ("tcp://127.0.0.1:23406");
const std::string host = "tcp://127.0.0.1";

const std::string defRoute1 ("tcp://127.0.0.1:23405");

const std::string defRoute2 ("tcp://127.0.0.1:23407");

BOOST_AUTO_TEST_CASE (zmqComms_broker_test)
{
    std::atomic<int> counter{0};

    helics::zeromq::ZmqComms comm;
	comm.loadTargetInfo(host, host);

    auto ctx = zmqContextManager::getContextPointer ();
    zmq::socket_t repSocket (ctx->getContext (), ZMQ_REP);
    repSocket.setsockopt (ZMQ_LINGER, 200);
    repSocket.bind (defServer);
    
    comm.setCallback ([&counter](helics::ActionMessage /*m*/) { ++counter; });
    comm.setBrokerPort (23405);
    comm.setName ("tests");
    auto confut = std::async (std::launch::async, [&comm]() { return comm.connect (); });

    zmq::message_t rxmsg;

    repSocket.recv (&rxmsg);

    BOOST_CHECK_GT (rxmsg.size (), 32);

    helics::ActionMessage rM (static_cast<char *> (rxmsg.data ()), rxmsg.size ());
    BOOST_CHECK (helics::isProtocolCommand (rM));
    rM.messageID = DISCONNECT;
    repSocket.send (rM.to_string ());
    auto connected = confut.get ();
    BOOST_CHECK (!connected);
    repSocket.close ();
}

/** test the request set class with various scenarios*/
BOOST_AUTO_TEST_CASE (zmqRequestSet_test1)
{
    //sleep to clear any residual from the previous test
    std::this_thread::sleep_for(500ms);
    helics::zeromq::ZmqRequestSets reqset;

    auto ctx = zmqContextManager::getContextPointer ();
    zmq::socket_t repSocket1 (ctx->getContext (), ZMQ_REP);
    repSocket1.bind (defServer);
    zmq::socket_t repSocket2 (ctx->getContext (), ZMQ_REP);
    repSocket2.bind (defRoute1);
    zmq::socket_t repSocket3 (ctx->getContext (), ZMQ_REP);
    repSocket3.bind (defRoute2);

    reqset.addRoutes (1, defServer);
    reqset.addRoutes (2, defRoute1);
    reqset.addRoutes (3, defRoute2);

    helics::ActionMessage M (helics::CMD_IGNORE);
    M.messageID = 1;

    reqset.transmit (1, M);
    BOOST_CHECK (reqset.waiting ());

    zmq::message_t msg;
    repSocket1.recv (&msg);

    repSocket1.send (msg);
    // should still be waiting
    BOOST_CHECK (reqset.waiting ());
    auto msgCnt = reqset.checkForMessages (100ms);

    BOOST_CHECK (!reqset.waiting ());
    BOOST_CHECK_EQUAL (msgCnt, 1);

    auto M2 = reqset.getMessage ();

    BOOST_CHECK (M2->action () == helics::CMD_IGNORE);

    // send two messages
    reqset.transmit (2, M);
    reqset.transmit (2, M);
    BOOST_CHECK (reqset.waiting ());

    repSocket2.recv (&msg);

    repSocket2.send (msg);
    msgCnt = reqset.checkForMessages (100ms);

    BOOST_CHECK (reqset.waiting ());
    repSocket2.recv (&msg);

    repSocket2.send (msg);
    reqset.checkForMessages (100ms);
    BOOST_CHECK (!reqset.waiting ());

    BOOST_CHECK (reqset.hasMessages ());
    repSocket1.close ();
    repSocket2.close ();
    repSocket3.close ();
    reqset.close ();
    std::this_thread::sleep_for (200ms);
}

/** test the request set class with various scenarios*/
BOOST_AUTO_TEST_CASE (zmqRequestSet_test2)
{
    //sleep to clear any residual from the previous test
    std::this_thread::sleep_for (500ms);
    helics::zeromq::ZmqRequestSets reqset;

    auto ctx = zmqContextManager::getContextPointer ();
    zmq::socket_t repSocket1 (ctx->getContext (), ZMQ_REP);
    repSocket1.bind (defServer);
    zmq::socket_t repSocket2 (ctx->getContext (), ZMQ_REP);
    repSocket2.bind (defRoute1);
    zmq::socket_t repSocket3 (ctx->getContext (), ZMQ_REP);
    repSocket3.bind (defRoute2);

    repSocket1.setsockopt (ZMQ_LINGER, 100);
    repSocket2.setsockopt (ZMQ_LINGER, 100);
    repSocket3.setsockopt (ZMQ_LINGER, 100);

    reqset.addRoutes (1, defServer);
    reqset.addRoutes (2, defRoute1);
    reqset.addRoutes (3, defRoute2);

    helics::ActionMessage M (helics::CMD_IGNORE);
    M.messageID = 1;

    reqset.transmit (1, M);
    reqset.transmit (2, M);
    reqset.transmit (3, M);
    zmq::message_t msg;
    repSocket1.recv (&msg);

    repSocket1.send (msg);

    repSocket2.recv (&msg);

    repSocket2.send (msg);
    repSocket3.recv (&msg);

    repSocket3.send (msg);
    
    // make sure the check receives all messages
    reqset.checkForMessages (50ms);
    if (reqset.waiting ())
    {
        reqset.checkForMessages (50ms);
    }
    //since we have 3 sockets we might have to do this twice since it returns immediately if it has a message
    if (reqset.waiting())
    {
        reqset.checkForMessages(50ms);
    }
    BOOST_REQUIRE (!reqset.waiting ());
    std::this_thread::yield();
    reqset.transmit (1, M);
    reqset.transmit (2, M);
    reqset.transmit (3, M);
    reqset.transmit (1, M);
    reqset.transmit (2, M);
    reqset.transmit (3, M);
    reqset.transmit (1, M);
    reqset.transmit (2, M);
    reqset.transmit (3, M);
    std::this_thread::yield();
    repSocket1.recv (&msg);

    repSocket1.send (msg);
    std::this_thread::yield();
    repSocket2.recv (&msg);

    repSocket2.send (msg);
    std::this_thread::yield();
    repSocket3.recv (&msg);

    repSocket3.send (msg);
    auto res = reqset.checkForMessages (400ms);
    if (res != 6)
    {
        auto res2 = reqset.checkForMessages (400ms);
        if ((res + res2 == 6) || (res2 == 6))
        {
            res = 6;
        }
        if (res != 6)
        {
            res2 = reqset.checkForMessages(400ms);
            if ((res + res2 == 6) || (res2 == 6))
            {
                res = 6;
            }
        }
        if (res != 6)
        {
            res2 = reqset.checkForMessages(400ms);
            if ((res + res2 == 6) || (res2 == 6))
            {
                res = 6;
            }
        }
    }
    BOOST_CHECK_EQUAL (res, 6);
    repSocket1.close ();
    repSocket2.close ();
    repSocket3.close ();
    reqset.close ();
    std::this_thread::sleep_for (200ms);
}

BOOST_AUTO_TEST_CASE (zmqComms_broker_test_transmit)
{
    //sleep to clear any residual from the previous test
    std::this_thread::sleep_for(500ms);
    std::atomic<int> counter{0};
    helics::zeromq::ZmqComms comm;
    comm.loadTargetInfo (host, host);

    auto ctx = zmqContextManager::getContextPointer ();
    zmq::socket_t repSocket (ctx->getContext (), ZMQ_REP);
    try
    {
        repSocket.bind (defServer);
    }
    catch (const zmq::error_t &ze)
    {
        std::cerr << "error repbind (broker test transmit) " << ze.what () << std::endl;
        std::this_thread::sleep_for (200ms);
        BOOST_FAIL ("Unable to bind Socket");
    }
    repSocket.setsockopt (ZMQ_LINGER, 100);
    zmq::socket_t pullSocket (ctx->getContext (), ZMQ_PULL);
    try
    {
        pullSocket.bind (defRoute1);
    }
    catch (const zmq::error_t &ze)
    {
        std::cerr << "error pullbind (broker test transmit)" << ze.what () << std::endl;
        repSocket.close ();
        std::this_thread::sleep_for (200ms);
        BOOST_FAIL ("Unable to bind Socket");
    }

    pullSocket.setsockopt (ZMQ_LINGER, 100);
    comm.setCallback ([&counter](helics::ActionMessage /*m*/) { ++counter; });
    comm.setBrokerPort (23405);
    comm.setPortNumber (23407);
    comm.setName ("tests");
    bool connected = comm.connect ();
    BOOST_REQUIRE (connected);
    comm.transmit (helics::parent_route_id, helics::CMD_IGNORE);
    zmq::message_t rxmsg;

    pullSocket.recv (&rxmsg);

    BOOST_CHECK_GT (rxmsg.size (), 32);
    helics::ActionMessage rM (static_cast<char *> (rxmsg.data ()), rxmsg.size ());
    BOOST_CHECK (rM.action () == helics::action_message_def::action_t::cmd_ignore);
    comm.disconnect ();
    repSocket.close ();
    pullSocket.close ();
    std::this_thread::sleep_for (200ms);
}

BOOST_AUTO_TEST_CASE (zmqComms_rx_test)
{
    //sleep to clear any residual from the previous test
    std::this_thread::sleep_for(400ms);
    std::atomic<int> counter{0};
    guarded<helics::ActionMessage> act;
    helics::zeromq::ZmqComms comm;
    comm.loadTargetInfo (host, host);

    auto ctx = zmqContextManager::getContextPointer ();
    zmq::socket_t repSocket (ctx->getContext (), ZMQ_REP);
    try
    {
        repSocket.bind (defServer);
    }
    catch (const zmq::error_t &ze)
    {
        std::cerr << "error repbind (zmqComms_rx_test) " << ze.what () << std::endl;
        std::this_thread::sleep_for (200ms);
        BOOST_FAIL ("Unable to bind Socket");
    }
    repSocket.setsockopt (ZMQ_LINGER, 100);
    zmq::socket_t pullSocket (ctx->getContext (), ZMQ_PULL);
    try
    {
        pullSocket.bind (defRoute1);
    }
    catch (const zmq::error_t &ze)
    {
        std::cerr << "error pullbind (zmqComms_rx_test)" << ze.what () << std::endl;
        repSocket.close ();
        std::this_thread::sleep_for (200ms);
        BOOST_FAIL ("Unable to bind Socket");
    }
    pullSocket.setsockopt (ZMQ_LINGER, 100);
    comm.setBrokerPort (23405);
    comm.setPortNumber (23407);
    comm.setName ("tests");
    comm.setCallback ([&counter, &act](helics::ActionMessage m) {
        ++counter;
        act = m;
    });

    bool connected = comm.connect ();
    BOOST_REQUIRE (connected);

    zmq::socket_t pushSocket (ctx->getContext (), ZMQ_PUSH);
    pushSocket.connect ("tcp://127.0.0.1:23407");

    helics::ActionMessage cmd (helics::CMD_ACK);
    std::string buffer = cmd.to_string ();
    try
    {
        auto cnt = pushSocket.send (buffer, ZMQ_DONTWAIT);
        BOOST_REQUIRE_EQUAL (cnt, buffer.size ());
    }
    catch (const zmq::error_t &ze)
    {
        BOOST_REQUIRE_MESSAGE (false, "Message failed to send");
    }

    std::this_thread::sleep_for (200ms);
    BOOST_REQUIRE_EQUAL (counter, 1);
    BOOST_CHECK (act.lock()->action () == helics::action_message_def::action_t::cmd_ack);
    comm.disconnect ();
    repSocket.close ();
    pullSocket.close ();
    std::this_thread::sleep_for (200ms);
}

BOOST_AUTO_TEST_CASE (zmqComm_transmit_through)
{
    //sleep to clear any residual from the previous test
    std::this_thread::sleep_for(300ms);
    std::atomic<int> counter{0};
    std::atomic<int> counter2{0};
    guarded<helics::ActionMessage> act;
    guarded<helics::ActionMessage> act2;

     helics::zeromq::ZmqComms comm,comm2;
    comm.loadTargetInfo (host, host);
     comm2.loadTargetInfo (host, "");

    comm.setBrokerPort (23405);
    comm.setName ("tests");
    comm2.setName ("test2");
    comm2.setPortNumber (23405);
    comm.setPortNumber (23407);

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

    comm.transmit (helics::parent_route_id, helics::CMD_ACK);

    std::this_thread::sleep_for (std::chrono::milliseconds (250));
    if (counter2 != 1)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (500));
    }
    BOOST_REQUIRE_EQUAL (counter2, 1);
    BOOST_CHECK (act2.lock()->action () == helics::action_message_def::action_t::cmd_ack);

    comm.disconnect ();
    comm2.disconnect ();
    std::this_thread::sleep_for (200ms);
}

BOOST_AUTO_TEST_CASE (zmqComm_transmit_add_route)
{
    //sleep to clear any residual from the previous test
    std::this_thread::sleep_for(300ms);
    std::atomic<int> counter{0};
    std::atomic<int> counter2{0};
    std::atomic<int> counter3{0};
    helics::zeromq::ZmqComms comm, comm2, comm3;

    comm.loadTargetInfo (host, host);
    comm2.loadTargetInfo (host, std::string());
    comm3.loadTargetInfo (host, host);

    comm.setBrokerPort (23405);
    comm.setName ("tests");
    comm2.setName ("broker");
    comm3.setName ("test3");
    comm3.setBrokerPort (23405);

    comm2.setPortNumber (23405);
    comm.setPortNumber (23407);
    comm3.setPortNumber (23409);

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
     auto connected_fut = std::async(std::launch::async, [&comm] {return comm.connect(); });

    bool connected = comm2.connect ();
    BOOST_REQUIRE (connected);
    connected = connected_fut.get();
    //connected = comm.connect ();
    BOOST_REQUIRE (connected);
    connected = comm3.connect ();

    comm.transmit (helics::parent_route_id, helics::CMD_ACK);

    std::this_thread::yield();
    if (counter2 != 1)
    {
        std::this_thread::sleep_for(200ms);
    }
    if (counter2 != 1)
    {
        std::this_thread::sleep_for (200ms);
    }
    BOOST_REQUIRE_EQUAL (counter2, 1);
    BOOST_CHECK (act2.lock()->action () == helics::action_message_def::action_t::cmd_ack);

    comm3.transmit (helics::parent_route_id, helics::CMD_ACK);

    std::this_thread::yield();
    if (counter2 != 2)
    {
        std::this_thread::sleep_for(200ms);
    }
    if (counter2 != 2)
    {
        std::this_thread::sleep_for(200ms);
    }
    BOOST_REQUIRE_EQUAL (counter2, 2);
    BOOST_CHECK (act2.lock()->action () == helics::action_message_def::action_t::cmd_ack);

    comm2.addRoute (helics::route_id_t(3), comm3.getAddress ());

    comm2.transmit (helics::route_id_t(3), helics::CMD_ACK);

    std::this_thread::yield();
    int lcnt = 0;
    while (counter3 != 1)
    {
        std::this_thread::sleep_for (200ms);
        ++lcnt;
        if (lcnt > 10)
        {
            break;
        }
    }
    
    BOOST_REQUIRE_EQUAL (counter3, 1);
    BOOST_CHECK (act3.lock()->action () == helics::action_message_def::action_t::cmd_ack);

    comm2.addRoute (helics::route_id_t(4), comm.getAddress ());

    comm2.transmit (helics::route_id_t(4), helics::CMD_ACK);

    std::this_thread::yield();
    while (counter != 1)
    {
        std::this_thread::sleep_for(200ms);
    }
 
    BOOST_REQUIRE_EQUAL (counter, 1);
    BOOST_CHECK (act.lock()->action () == helics::action_message_def::action_t::cmd_ack);

    comm.disconnect ();
    comm2.disconnect ();
    comm3.disconnect ();
    std::this_thread::sleep_for (200ms);
}

BOOST_AUTO_TEST_CASE (zmqCore_initialization_test)
{
    //sleep to clear any residual from the previous test
    std::this_thread::sleep_for(400ms);
    std::string initializationString =
      "1 --brokerport=23405 --port=23410 --local_interface=tcp://127.0.0.1 --name=core1";
    auto core = helics::CoreFactory::create (helics::core_type::ZMQ, initializationString);

    BOOST_REQUIRE (core != nullptr);
    BOOST_CHECK (core->isInitialized ());
    auto ctx = zmqContextManager::getContextPointer ();
    zmq::socket_t repSocket (ctx->getContext (), ZMQ_REP);
    try
    {
        repSocket.bind (defServer);
    }
    catch (const zmq::error_t &ze)
    {
        std::cerr << "error repbind (zmqCore_initialization_test) " << ze.what () << std::endl;
        std::this_thread::sleep_for (200ms);
        BOOST_FAIL ("Unable to bind Socket");
    }
    repSocket.setsockopt (ZMQ_LINGER, 100);
    zmq::socket_t pullSocket (ctx->getContext (), ZMQ_PULL);
    try
    {
        pullSocket.bind (defRoute1);
    }
    catch (const zmq::error_t &ze)
    {
        std::cerr << "error pullbind (zmqCore_initialization_test)" << ze.what () << std::endl;
        repSocket.close ();
        std::this_thread::sleep_for (200ms);
        BOOST_FAIL ("Unable to bind Socket");
    }
    pullSocket.setsockopt (ZMQ_LINGER, 100);
    bool connected = core->connect ();
    BOOST_REQUIRE (connected);

    zmq::message_t rxmsg;

    pullSocket.recv (&rxmsg);

    BOOST_CHECK_GT (rxmsg.size (), 32);
    helics::ActionMessage rM (static_cast<char *> (rxmsg.data ()), rxmsg.size ());

    BOOST_CHECK_EQUAL (rM.name, "core1");
    BOOST_CHECK (rM.action () == helics::action_message_def::action_t::cmd_reg_broker);

    repSocket.close ();
    pullSocket.close ();
    core->disconnect ();
    core = nullptr;
    helics::CoreFactory::cleanUpCores (200ms);
    std::this_thread::sleep_for (200ms);
}

/** test case checks default values and makes sure they all mesh together
also tests the automatic port determination for cores
*/
BOOST_AUTO_TEST_CASE (zmqCore_core_broker_default_test)
{
    //sleep to clear any residual from the previous test
    std::this_thread::sleep_for(400ms);
    std::string initializationString = "1";

    auto broker = helics::BrokerFactory::create (helics::core_type::ZMQ, initializationString);

    auto core = helics::CoreFactory::create (helics::core_type::ZMQ, initializationString);
    bool connected = broker->isConnected ();
    BOOST_CHECK (connected);
    connected = core->connect ();
    BOOST_CHECK (connected);

    core->disconnect ();

    BOOST_CHECK (!core->isConnected ());
    broker->disconnect ();
    BOOST_CHECK (!broker->isConnected ());
    helics::CoreFactory::cleanUpCores (200ms);
    helics::BrokerFactory::cleanUpBrokers (200ms);
}

BOOST_AUTO_TEST_SUITE_END ()
