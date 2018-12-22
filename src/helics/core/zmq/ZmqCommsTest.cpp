/*
 Copyright © 2017-2018,
 Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
 All rights reserved. See LICENSE file and DISCLAIMER for more details.
 */
#include "ZmqCommsTest.h"
#include "../../common/zmqContextManager.h"
#include "../../common/zmqHelper.h"
#include "../../common/zmqSocketDescriptor.h"
#include "../ActionMessage.hpp"
#include "../NetworkBrokerData.hpp"
#include "ZmqRequestSets.h"
//#include <boost/asio.hpp>
//#include <csignal>
#include <memory>
#include <iostream>

static const int DEFAULT_BROKER_PORT_NUMBER = 23414;  // Todo define a different port number
static const int TX_RX_MSG_COUNT = 20;

using namespace std::chrono;
/** bind a zmq socket, with a timeout and timeout period*/
static bool bindzmqSocket (zmq::socket_t &socket,
                           const std::string &address,
                           int port,
                           milliseconds timeout,
                           milliseconds period = milliseconds (200))
{
    bool bindsuccess = false;
    milliseconds tcount{0};
    while (!bindsuccess)
    {
        try
        {
            socket.bind (helics::makePortAddress (address, port));
            bindsuccess = true;
        }
        catch (const zmq::error_t &)
        {
            if (tcount == milliseconds (0))
            {
                std::cerr << "zmq binding error on socket sleeping then will try again \n";
            }
            if (tcount > timeout)
            {
                break;
            }
            std::this_thread::sleep_for (period);
            tcount += period;
        }
    }
    return bindsuccess;
}

namespace helics
{
namespace zeromq
{
void ZmqCommsTest::loadNetworkInfo (const NetworkBrokerData &netInfo)
{
    NetworkCommsInterface::loadNetworkInfo (netInfo);
    if (!propertyLock ())
    {
        return;
    }
    if (!brokerTarget_.empty ())
    {
        insertProtocol (brokerTarget_, interface_type::tcp);
    }
    if (!localTarget_.empty ())
    {
        insertProtocol (localTarget_, interface_type::tcp);
    }
    if (localTarget_ == "tcp://localhost")
    {
        localTarget_ = "tcp://127.0.0.1";
    }
    else if (localTarget_ == "udp://localhost")
    {
        localTarget_ = "udp://127.0.0.1";
    }
    if (brokerTarget_ == "tcp://localhost")
    {
        brokerTarget_ = "tcp://127.0.0.1";
    }
    else if (brokerTarget_ == "udp://localhost")
    {
        brokerTarget_ = "udp://127.0.0.1";
    }
    propertyUnLock ();
}

ZmqCommsTest::ZmqCommsTest () noexcept : NetworkCommsInterface (interface_type::ip) {}

/** destructor*/
ZmqCommsTest::~ZmqCommsTest () { disconnect (); }

int ZmqCommsTest::getDefaultBrokerPort () const { return DEFAULT_BROKER_PORT_NUMBER; }

int ZmqCommsTest::processIncomingMessage (zmq::message_t &msg, std::map<std::string, std::string> &connection_info)
{
    int status = 0;
    if (msg.size () == 5)
    {
        std::string str (static_cast<char *> (msg.data ()), msg.size ());
        if (str == "close")
        {
            return (-1);
        }
    }
    ActionMessage M (static_cast<char *> (msg.data ()), msg.size ());
    //std::cout << "****NAME: " << name << " RX Message " << " Protocol command action: " << prettyPrintString(M) << " source ID: " << M.source_id << " dest ID: " << M.dest_id << std::endl;

    if (!isValidCommand (M))
    {
    	std::cerr << "invalid command received" << M.action() << std::endl;
        return 0;
    }
    if (isProtocolCommand (M))
    {
        switch (M.messageID)
        {
        case PORT_DEFINITIONS:
            loadPortDefinitions (M);
            break;
        case NAME_NOT_FOUND:
            //std::cout << "broker name " << brokerName_ << " does not match broker connection\n";
            disconnecting = true;
            setRxStatus (connection_status::error);
            status = -1;
            break;
        case DISCONNECT:
            disconnecting = true;
            std::cout << "name " << name << " processIncomingMessage DISCONNECTING\n";
            setRxStatus (connection_status::terminated);
            status = -1;
        case DISCONNECT_ERROR:
            disconnecting = true;
            setRxStatus (connection_status::error);
            status = -1;
            break;
        case CLOSE_RECEIVER:
        	//std::cout << name << "Received Close Receiver: " << std::endl;
        	setRxStatus (connection_status::terminated);
            status = -1;
            break;
        case RECONNECT_RECEIVER:
            setRxStatus (connection_status::connected);
            break;
        case CONNECTION_INFORMATION:
            if (serverMode)
            {
                //std::cout << name << "Adding connection info: " << M.payload << std::endl;
                connection_info.emplace (M.name, M.payload);
                //for(auto &mc : connection_info) {
                //std::cout << name << "conn info: " << mc.first << "second: " << mc.second << std::endl;
                //}
            }
            break;
        default:
            break;
        }
    }
    ActionCallback (std::move (M));
    return status;
}

int ZmqCommsTest::replyToIncomingMessage (zmq::message_t &msg, zmq::socket_t &sock)
{
    ActionMessage M (static_cast<char *> (msg.data ()), msg.size ());
    if (isProtocolCommand (M))
    {
        if (M.messageID == CLOSE_RECEIVER)
        {
            return (-1);
        }
        auto reply = generateReplyToIncomingMessage (M);
        auto str = reply.to_string ();
        sock.send (str.data (), str.size ());
        return 0;
    }
    else
    {
        ActionCallback (std::move (M));
        ActionMessage resp (CMD_PRIORITY_ACK);
        auto str = resp.to_string ();
        sock.send (str.data (), str.size ());
        return 0;
    }
}

void ZmqCommsTest::queue_rx_function ()
{
    // Everything is handled by tx thread
}

int ZmqCommsTest::initializeBrokerConnections (zmq::socket_t &brokerSocket, zmq::socket_t &brokerConnection)
{
    if (serverMode)
    {
        brokerSocket.setsockopt (ZMQ_LINGER, 500);
    	auto bindsuccess = bindzmqSocket (brokerSocket, localTarget_, brokerPort, connectionTimeout);
        if (!bindsuccess)
        {
            brokerSocket.close ();
            disconnecting = true;
            logError (std::string ("Unable to bind zmq router socket giving up ") +
                      makePortAddress (localTarget_, brokerPort));
            setRxStatus (connection_status::error);
            return -1;
        }
    }
    if (hasBroker)
    {
        brokerConnection.setsockopt (ZMQ_IDENTITY, name.c_str (), name.size ());
        brokerConnection.setsockopt (ZMQ_LINGER, 500);
        try
        {
            brokerConnection.connect (makePortAddress (brokerTarget_, brokerPort));
        }
        catch (zmq::error_t &ze)
        {
            logError (std::string ("unable to connect with broker at ") +
                      makePortAddress (brokerTarget_, brokerPort + 1) + ":(" + name + ")" + ze.what ());
            setTxStatus (connection_status::error);
            return -1;
        }
        std::vector<char> buffer;
        // generate a local protocol connection string to send it's identity
        ActionMessage cmessage (CMD_PROTOCOL);
        cmessage.messageID = CONNECTION_INFORMATION;
        cmessage.name = name;
        cmessage.payload = getAddress ();
        cmessage.to_vector (buffer);
        //std::cout << "***NAME: " << name << " action: " << cmessage.action() << '\n';
        brokerConnection.send (buffer.data (), buffer.size (), ZMQ_NOBLOCK);
    }
    return 0;
}

bool ZmqCommsTest::processTxControlCmd (ActionMessage cmd,
                                        std::map<route_id, std::string> &routes,
                                        std::map<std::string, std::string> &connection_info)
{
    bool close_tx = false;

    switch (cmd.messageID)
    {
    case RECONNECT_TRANSMITTER:
        setTxStatus (connection_status::connected);
        break;
    case CONNECTION_INFORMATION:
        // Shouldn't reach here ideally
        if (serverMode)
        {
            connection_info.emplace (cmd.name, cmd.payload);
        }
        break;
    case NEW_ROUTE:
        //std::cout << name << " update connection info" << std::endl;
        try
        {
            for (auto &mc : connection_info)
            {
                //std::cout << name << " mc.second: " << mc.second << " cmd.payload: " << cmd.payload << std::endl;
                if (mc.second == cmd.payload)
                {
                    routes.emplace (route_id (cmd.getExtraData ()), mc.first);
                    //std::cout << "New Route id: " << cmd.getExtraData() << std::endl;
                    break;
                }
            }
        }
        catch (const zmq::error_t &e)
        {
            // TODO:: do something???
            logError (std::string ("unable to connect route") + cmd.payload + "::" + e.what ());
        }
        break;
    case REMOVE_ROUTE:
        routes.erase (route_id (cmd.getExtraData ()));
        break;
    case CLOSE_RECEIVER:
        setRxStatus (connection_status::terminated);
        close_tx = true;
        break;
    case DISCONNECT:
        std::cout << name << " processTxControlCmd    DISCONNECTING" << std::endl;
        close_tx = true;
        break;
    }
    return close_tx;
}

void ZmqCommsTest::queue_tx_function ()
{
    std::vector<char> buffer;
    auto ctx = zmqContextManager::getContextPointer ();
    zmq::message_t msg;

    if (!brokerTarget_.empty ())
    {
        hasBroker = true;
    }
    //std::cout << "In queue_tx_function: "
    //          << name << " server mode: " << serverMode << " hasBroker: " << hasBroker << std::endl;
    // contains mapping between route id and core name
    std::map<route_id, std::string> routes;
    // contains mapping between core name and address
    std::map<std::string, std::string> connection_info;

    if (brokerPort < 0)
    {
        brokerPort = DEFAULT_BROKER_PORT_NUMBER;
    }

    zmq::socket_t brokerSocket (ctx->getContext (), ZMQ_ROUTER);
    zmq::socket_t brokerConnection (ctx->getContext (), ZMQ_DEALER);
    auto res = initializeBrokerConnections (brokerSocket, brokerConnection);
    if (res < 0)
    {
        setTxStatus (connection_status::error);
        brokerSocket.close ();
        brokerConnection.close ();
        return;
    }
    // Root broker is set
    if (!serverMode)
        brokerSocket.close ();
    if (!hasBroker)
        brokerConnection.close ();
    setTxStatus (connection_status::connected);

    std::vector<zmq::pollitem_t> poller (2);
    if (serverMode && hasBroker)
    {
        poller[0].socket = static_cast<void *> (brokerSocket);
        poller[0].events = ZMQ_POLLIN;
        poller[1].socket = static_cast<void *> (brokerConnection);
        poller[1].events = ZMQ_POLLIN;
    }
    else
    {
        if (serverMode)
        {
            poller.resize (1);
            poller[0].socket = static_cast<void *> (brokerSocket);
            poller[0].events = ZMQ_POLLIN;
        }
        if (hasBroker)
        {
            poller.resize (1);
            poller[0].socket = static_cast<void *> (brokerConnection);
            poller[0].events = ZMQ_POLLIN;
        }
    }

    setRxStatus (connection_status::connected);

    bool close_tx = false;
    int status = 0;

    while (true)
    {
        route_id rid;
        ActionMessage cmd;
        int count = 0;
        int tx_count = 0;
        int rc = 1;
        //std::tie (rid, cmd) = txQueue.try_pop ();

        // Handle Tx messages first
        auto tx_msg = txQueue.try_pop ();
        //rc = zmq::poll (poller, std::chrono::milliseconds (1));
        rc = zmq::poll (poller, std::chrono::milliseconds (10));
        if (!tx_msg || (rc <= 0))
        {
            //std::cout << "yield" << std::endl;
            std::this_thread::yield ();
        }
        tx_count = 0;
        // Balance between tx and rx processing since both running on single thread
        while (tx_msg && (tx_count < TX_RX_MSG_COUNT))
        {
            ////std::cout << "TX LOOP **** NAME: " << name << " " << count << std::endl;

            bool processed = false;
            cmd = std::move (tx_msg->second);
            rid = std::move (tx_msg->first);
            //std::cout << name << " rid: " << rid << " parent route id: " << parent_route_id << '\n';
            //std::cout << "TX LOOP **** NAME: " << name << " count:" << tx_count << "proto cmd: " << prettyPrintString(cmd) << " source id: " << cmd.source_id << " dest id: " << cmd.dest_id << std::endl;
            if (isProtocolCommand (cmd))
            {
                if (rid == control_route)
                {
                	processed = true;
                    //std::cout << "Enter control route" << std::endl;
                    close_tx = processTxControlCmd (cmd, routes, connection_info);

                    if (close_tx)
                    {
                        goto CLOSE_TX_LOOP;
                    }
                }
            }
            if (!processed)
            {
                buffer.clear ();
                cmd.to_vector (buffer);
                if (rid == parent_route_id)
                {
                    if (hasBroker)
                    {
                        //std::cout << "**** NAME: " << name << " Sending message to broker: "<< cmd.action() << "message ID: " << cmd.messageID << std::endl;
                        std::string empty = "";
                        brokerConnection.send (buffer.data (), buffer.size (), ZMQ_NOBLOCK);
                    }
                    else
                    {
                        logWarning ("no route to broker for message");
                    }
                }
                else if (rid == control_route)
                {
                    status = processIncomingMessage (msg, connection_info);  //----------> ToCheck
                    if (status < 0)
                    {
                    	goto CLOSE_TX_LOOP;
                    }
                }
                else
                {
                    // If route found send out through the front end socket connection
                    auto rt_find = routes.find (rid);
                    if (rt_find != routes.end ())
                    {
                        std::string route_name = rt_find->second;
                        std::string empty = "";
                        //std::cout << name << "::Sending message to core: " << route_name.c_str() << " cmd: " << cmd.action() << std::endl;
                        // Need to first send identity and empty string
                        brokerSocket.send (route_name.c_str (), route_name.size (), ZMQ_SNDMORE);
                        brokerSocket.send (empty.c_str(), empty.size (), ZMQ_SNDMORE);
                        // Send the actual data
                        brokerSocket.send (buffer.data (), buffer.size (), ZMQ_NOBLOCK);
                    }
                    else
                    {
                        if (hasBroker)
                        {
                        	//std::cout << name << "::Sending message to broker: " << " cmd: " << cmd.action() << std::endl;
                            brokerConnection.send (buffer.data (), buffer.size (), ZMQ_NOBLOCK);
                        }
                        else
                        {
                            logWarning ("unknown route and no broker, dropping message");
                        }
                    }
                }
            }
            tx_count++;
            if (tx_count < TX_RX_MSG_COUNT) {
            	tx_msg = txQueue.try_pop ();
            }
        }

        count = 0;
        rc = 1;
        while ((rc > 0) && (count < 5))
        {
        	////std::cout << "Checking RX  **** NAME: " << name << std::endl;
            rc = zmq::poll (poller, std::chrono::milliseconds (10));

            if (rc > 0)
            {
                ////std::cout << " RX LOOP:: FRONTEND MESSAGE: Identity" << name << " count: " << '\n';
                if ((poller[0].revents & ZMQ_POLLIN) != 0)
                {
                    status = processRxMessage (brokerSocket, brokerConnection, connection_info);
                }
                if (serverMode && hasBroker)
                {
                    if ((poller[1].revents & ZMQ_POLLIN) != 0)
                    {
                        status = processRxMessage (brokerSocket, brokerConnection, connection_info);
                    }
                }

                if (status < 0)
                {
                	goto CLOSE_TX_LOOP;
                }
            }
            count++;
        }
        //std::cout << "out of rx loop " << name << std::endl;
    }
    CLOSE_TX_LOOP:
		routes.clear ();
		connection_info.clear ();
		if (getRxStatus () == connection_status::connected)
		{
			setRxStatus (connection_status::terminated);
		}
		if (serverMode)
		{
			std::cout << "Closing broker socket" << std::endl;
			std::this_thread::sleep_for (std::chrono::milliseconds (50));
			brokerSocket.close ();
		}
		if (hasBroker)
		{
			std::cout << name << ":: Closing connection to broker" << std::endl;
			brokerConnection.close ();
		}
		setTxStatus (connection_status::terminated);

    std::cout << "exit from queue tx loop" << std::endl;
}

int ZmqCommsTest::processRxMessage (zmq::socket_t &brokerSocket,
                                    zmq::socket_t &brokerConnection,
                                    std::map<std::string, std::string> &connection_info)
{
    int status = 0;
    zmq::message_t msg1;
    zmq::message_t msg2;

    if (serverMode)
    {
        brokerSocket.recv (&msg1);
        std::string str (static_cast<char *> (msg1.data ()), msg1.size ());

        //std::cout << " BROKER SOCKET RX MESSAGE: Identity" << name << " *****1st DATA: " << str.c_str() << '\n';
        brokerSocket.recv (&msg2);
    }
    else
    {
        brokerConnection.recv (&msg1);
        std::string str (static_cast<char *> (msg1.data ()), msg1.size ());

        //std::cout << " BROKER CONNECTION RX MESSAGE: Identity" << name << " *****1st DATA: " << str.c_str() << '\n';
        brokerConnection.recv (&msg2);
    }
    std::string str2 (static_cast<char *> (msg2.data ()), msg2.size ());
    status = processIncomingMessage (msg2, connection_info);
    //	for (auto &mc : connection_info) {
    //		//std::cout << name << " mc.second: " << mc.second << std::endl;
    //	}
    return status;
}

void ZmqCommsTest::closeReceiver ()
{
    switch (getTxStatus ())
    {
    case connection_status::startup:
    case connection_status::connected:
    {
    	std::cout << name << "Transmit CLOSE_RECEIVER" << std::endl;
        ActionMessage cmd (CMD_PROTOCOL);
        cmd.messageID = CLOSE_RECEIVER;
        transmit (control_route, cmd);
    }

    break;
    default:
        if (!disconnecting)
        {
            // try connecting with the receivers push socket
            ActionMessage cmd (CMD_PROTOCOL);
            cmd.messageID = CLOSE_RECEIVER;
            transmit (control_route, cmd);
        }
        break;
    }
}

}  // namespace zeromq
// namespace zeromq
}  // namespace helics
