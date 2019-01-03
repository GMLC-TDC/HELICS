/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "ZmqComms.h"
#include "../../common/zmqContextManager.h"
#include "../../common/zmqHelper.h"
#include "../../common/zmqSocketDescriptor.h"
#include "../ActionMessage.hpp"
#include "../NetworkBrokerData.hpp"
#include "ZmqRequestSets.h"
//#include <boost/asio.hpp>
//#include <csignal>
#include <memory>

static const int DEFAULT_BROKER_PORT_NUMBER = 23404;

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
                // std::cerr << "zmq binding error on socket sleeping then will try again \n";
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
void ZmqComms::loadNetworkInfo (const NetworkBrokerData &netInfo)
{
    NetworkCommsInterface::loadNetworkInfo (netInfo);
    if (!propertyLock ())
    {
        return;
    }
    if (!brokerTargetAddress.empty ())
    {
        insertProtocol (brokerTargetAddress, interface_type::tcp);
    }
    if (!localTargetAddress.empty ())
    {
        insertProtocol (localTargetAddress, interface_type::tcp);
    }
    if (localTargetAddress == "tcp://localhost")
    {
        localTargetAddress = "tcp://127.0.0.1";
    }
    else if (localTargetAddress == "udp://localhost")
    {
        localTargetAddress = "udp://127.0.0.1";
    }
    if (brokerTargetAddress == "tcp://localhost")
    {
        brokerTargetAddress = "tcp://127.0.0.1";
    }
    else if (brokerTargetAddress == "udp://localhost")
    {
        brokerTargetAddress = "udp://127.0.0.1";
    }
    propertyUnLock ();
}

ZmqComms::ZmqComms () noexcept : NetworkCommsInterface (interface_type::ip) {}

/** destructor*/
ZmqComms::~ZmqComms () { disconnect (); }

int ZmqComms::getDefaultBrokerPort () const { return DEFAULT_BROKER_PORT_NUMBER; }

int ZmqComms::processIncomingMessage (zmq::message_t &msg)
{
    if (msg.size () == 5)
    {
        std::string str (static_cast<char *> (msg.data ()), msg.size ());
        if (str == "close")
        {
            return (-1);
        }
    }
    ActionMessage M (static_cast<char *> (msg.data ()), msg.size ());
    if (!isValidCommand (M))
    {
        logError ("invalid command received");
        ActionMessage Q (static_cast<char *> (msg.data ()), msg.size ());
        return 0;
    }
    if (isProtocolCommand (M))
    {
        switch (M.messageID)
        {
        case CLOSE_RECEIVER:
            return (-1);
        case RECONNECT_RECEIVER:
            setRxStatus (connection_status::connected);
            break;
        default:
            break;
        }
    }
    ActionCallback (std::move (M));
    return 0;
}

int ZmqComms::replyToIncomingMessage (zmq::message_t &msg, zmq::socket_t &sock)
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

    ActionCallback (std::move (M));
    ActionMessage resp (CMD_PRIORITY_ACK);
    auto str = resp.to_string ();
    sock.send (str.data (), str.size ());
    return 0;
}

void ZmqComms::queue_rx_function ()
{
    auto ctx = ZmqContextManager::getContextPointer ();
    zmq::socket_t pullSocket (ctx->getContext (), ZMQ_PULL);
    pullSocket.setsockopt (ZMQ_LINGER, 200);
    zmq::socket_t controlSocket (ctx->getContext (), ZMQ_PAIR);
    std::string controlsockString = std::string ("inproc://") + name + "_control";
    try
    {
        controlSocket.bind (controlsockString.c_str ());
    }
    catch (const zmq::error_t &e)
    {
        logError (std::string ("binding error on internal comms socket:") + e.what ());
        setRxStatus (connection_status::error);
        return;
    }
    controlSocket.setsockopt (ZMQ_LINGER, 200);

    zmq::socket_t repSocket (ctx->getContext (), ZMQ_REP);
    if (serverMode)
    {
        repSocket.setsockopt (ZMQ_LINGER, 500);
    }

    while (PortNumber == -1)
    {
        zmq::message_t msg;
        controlSocket.recv (&msg);
        if (msg.size () < 10)
        {
            continue;
        }
        ActionMessage M (static_cast<char *> (msg.data ()), msg.size ());

        if (isProtocolCommand (M))
        {
            if (M.messageID == PORT_DEFINITIONS)
            {
                loadPortDefinitions (M);
            }
            else if (M.messageID == NAME_NOT_FOUND)
            {
                logError (std::string ("broker name ") + brokerName + " does not match broker connection");
                disconnecting = true;
                setRxStatus (connection_status::error);
                return;
            }
            else if (M.messageID == DISCONNECT)
            {
                disconnecting = true;
                setRxStatus (connection_status::terminated);
                return;
            }
            else if (M.messageID == DISCONNECT_ERROR)
            {
                disconnecting = true;
                setRxStatus (connection_status::error);
                return;
            }
        }
    }
    if (serverMode)
    {
        auto bindsuccess = bindzmqSocket (repSocket, localTargetAddress, PortNumber + 1, connectionTimeout);
        if (!bindsuccess)
        {
            pullSocket.close ();
            repSocket.close ();
            disconnecting = true;
            logError (std::string ("Unable to bind zmq reply socket giving up ") +
                      makePortAddress (localTargetAddress, PortNumber + 1));
            setRxStatus (connection_status::error);
            return;
        }
    }

    auto bindsuccess = bindzmqSocket (pullSocket, localTargetAddress, PortNumber, connectionTimeout);

    if (!bindsuccess)
    {
        pullSocket.close ();
        repSocket.close ();
        disconnecting = true;
        logError (std::string ("Unable to bind zmq pull socket giving up ") +
                  makePortAddress (localTargetAddress, PortNumber));
        setRxStatus (connection_status::error);
        return;
    }

    std::vector<zmq::pollitem_t> poller (3);
    poller[0].socket = static_cast<void *> (controlSocket);
    poller[0].events = ZMQ_POLLIN;
    poller[1].socket = static_cast<void *> (pullSocket);
    poller[1].events = ZMQ_POLLIN;
    if (serverMode)
    {
        poller[2].socket = static_cast<void *> (repSocket);
        poller[2].events = ZMQ_POLLIN;
    }
    else
    {
        poller.resize (2);
    }
    setRxStatus (connection_status::connected);
    while (true)
    {
        auto rc = zmq::poll (poller, std::chrono::milliseconds (1000));
        if (rc > 0)
        {
            zmq::message_t msg;
            if ((poller[0].revents & ZMQ_POLLIN) != 0)
            {
                controlSocket.recv (&msg);

                auto status = processIncomingMessage (msg);
                if (status < 0)
                {
                    break;
                }
            }
            if ((poller[1].revents & ZMQ_POLLIN) != 0)
            {
                pullSocket.recv (&msg);
                auto status = processIncomingMessage (msg);
                if (status < 0)
                {
                    break;
                }
            }
            if (serverMode)
            {
                if ((poller[2].revents & ZMQ_POLLIN) != 0)
                {
                    repSocket.recv (&msg);
                    auto status = replyToIncomingMessage (msg, repSocket);
                    if (status < 0)
                    {
                        break;
                    }
                    continue;
                }
            }
        }
        if (requestDisconnect.load (std::memory_order::memory_order_acquire))
        {
            break;
        }
    }
    disconnecting = true;
    setRxStatus (connection_status::terminated);
}

int ZmqComms::initializeBrokerConnections (zmq::socket_t &controlSocket)
{
    zmq::pollitem_t poller;
    if (hasBroker)
    {
        auto ctx = ZmqContextManager::getContextPointer ();
        if (brokerPort < 0)
        {
            brokerPort = DEFAULT_BROKER_PORT_NUMBER;
        }

        zmq::socket_t brokerReq (ctx->getContext (), ZMQ_REQ);
        brokerReq.setsockopt (ZMQ_LINGER, 50);
        try
        {
            brokerReq.connect (makePortAddress (brokerTargetAddress, brokerPort + 1));
        }
        catch (zmq::error_t &ze)
        {
            logError (std::string ("unable to connect with broker at ") +
                      makePortAddress (brokerTargetAddress, brokerPort + 1) + ":(" + name + ")" + ze.what ());
            setTxStatus (connection_status::error);
            ActionMessage M (CMD_PROTOCOL);
            M.messageID = DISCONNECT_ERROR;
            controlSocket.send (M.to_string ());
            return (-1);
        }

        hasBroker = true;
        int cnt = 0;
        zmq::message_t msg;
        if (PortNumber < 0)
        {
            while (PortNumber < 0)
            {
                ActionMessage getPorts = generatePortRequest ((serverMode) ? 2 : 1);
                auto str = getPorts.to_string ();
                brokerReq.send (str);
                poller.socket = static_cast<void *> (brokerReq);
                poller.events = ZMQ_POLLIN;
                auto rc = zmq::poll (&poller, 1, connectionTimeout);
                if (rc < 0)
                {
                    logError ("unable to connect with zmq broker (2)");
                    setTxStatus (connection_status::error);
                }
                else if (rc == 0)
                {
                    logError ("zmq broker connection timed out (2)");
                    setTxStatus (connection_status::error);
                }
                if (getTxStatus () == connection_status::error)
                {
                    ActionMessage M (CMD_PROTOCOL);
                    M.messageID = DISCONNECT_ERROR;
                    controlSocket.send (M.to_string ());
                    return (-1);
                }
                brokerReq.recv (&msg);

                ActionMessage rxcmd (static_cast<char *> (msg.data ()), msg.size ());
                if (isProtocolCommand (rxcmd))
                {
                    if (rxcmd.messageID == PORT_DEFINITIONS)
                    {
                        controlSocket.send (msg);
                        return 0;
                    }
                    if (rxcmd.messageID == DISCONNECT)
                    {
                        controlSocket.send (msg);
                        setTxStatus (connection_status::terminated);
                        return (-3);
                    }
                    if (rxcmd.messageID == DISCONNECT_ERROR)
                    {
                        controlSocket.send (msg);
                        setTxStatus (connection_status::error);
                        return (-4);
                    }
                }

                ++cnt;
                if (cnt > 10)
                {
                    // we can't get the broker to respond with port numbers
                    setTxStatus (connection_status::error);
                    return (-1);
                }
            }
        }
    }
    else
    {
        if ((PortNumber < 0))
        {
            PortNumber = DEFAULT_BROKER_PORT_NUMBER;
            ActionMessage setPorts (CMD_PROTOCOL);
            setPorts.messageID = PORT_DEFINITIONS;
            setPorts.setExtraData (PortNumber);
            auto str = setPorts.to_string ();
            controlSocket.send (str);
        }
    }
    return 0;
}

void ZmqComms::queue_tx_function ()
{
    std::vector<char> buffer;
    if (!brokerTargetAddress.empty ())
    {
        hasBroker = true;
    }
    auto ctx = ZmqContextManager::getContextPointer ();
    // Setup the control socket for comms with the receiver
    zmq::socket_t controlSocket (ctx->getContext (), ZMQ_PAIR);
    controlSocket.setsockopt (ZMQ_LINGER, 200);
    std::string controlsockString = std::string ("inproc://") + name + "_control";
    controlSocket.connect (controlsockString);
    try
    {
        auto res = initializeBrokerConnections (controlSocket);
        if (res < 0)
        {
            setTxStatus (connection_status::error);
            controlSocket.close ();
            return;
        }
    }
    catch (const zmq::error_t &e)
    {
        controlSocket.close ();
        return;
    }

    zmq::socket_t brokerPushSocket (ctx->getContext (), ZMQ_PUSH);
    brokerPushSocket.setsockopt (ZMQ_LINGER, 200);
    std::map<route_id, zmq::socket_t> routes;  // for all the other possible routes
    // ZmqRequestSets priority_routes;  //!< object to handle the management of the priority routes

    if (hasBroker)
    {
        //   priority_routes.addRoutes (0, makePortAddress (brokerTargetAddress, brokerPort+1));
        brokerPushSocket.connect (makePortAddress (brokerTargetAddress, brokerPort));
    }
    setTxStatus (connection_status::connected);
    zmq::message_t msg;
    while (true)
    {
        route_id rid;
        ActionMessage cmd;

        std::tie (rid, cmd) = txQueue.pop ();
        bool processed = false;
        if (isProtocolCommand (cmd))
        {
            if (control_route == rid)
            {
                switch (cmd.messageID)
                {
                case RECONNECT_TRANSMITTER:
                    setTxStatus (connection_status::connected);
                    break;
                case NEW_ROUTE:
                {
                    try
                    {
                        auto interfaceAndPort = extractInterfaceandPort (cmd.payload);

                        auto zsock = zmq::socket_t (ctx->getContext (), ZMQ_PUSH);
                        zsock.setsockopt (ZMQ_LINGER, 100);
                        zsock.connect (makePortAddress (interfaceAndPort.first, interfaceAndPort.second));
                        routes.emplace (route_id{cmd.getExtraData ()}, std::move (zsock));
                    }
                    catch (const zmq::error_t &e)
                    {
                        // TODO:: do something???
                        logError (std::string ("unable to connect route") + cmd.payload + "::" + e.what ());
                    }
                    processed = true;
                }
                break;
                case REMOVE_ROUTE:
                    routes.erase (route_id{cmd.getExtraData ()});
                    processed = true;
                    break;
                case DISCONNECT:
                    goto CLOSE_TX_LOOP;  // break out of loop
                }
            }
        }
        if (processed)
        {
            continue;
        }
        cmd.to_vector (buffer);
        if (rid == parent_route_id)
        {
            if (hasBroker)
            {
                brokerPushSocket.send (buffer.data (), buffer.size ());
            }
            else
            {
                logWarning ("no route to broker for message");
            }
        }
        else if (rid == control_route)
        {  // send to rx thread loop
            try
            {
                controlSocket.send (buffer.data (), buffer.size (), ZMQ_NOBLOCK);
            }
            catch (const zmq::error_t &e)
            {
                if ((getRxStatus () == connection_status::terminated) ||
                    (getRxStatus () == connection_status::error))
                {
                    goto CLOSE_TX_LOOP;  // break out of loop
                }
                else
                {
                    logError (e.what ());
                }
            }
            continue;
        }
        else
        {
            auto rt_find = routes.find (rid);
            if (rt_find != routes.end ())
            {
                rt_find->second.send (buffer.data (), buffer.size ());
            }
            else
            {
                if (hasBroker)
                {
                    brokerPushSocket.send (buffer.data (), buffer.size ());
                }
                else
                {
                    if (!isDisconnectCommand (cmd))
                    {
                        logWarning (std::string ("unknown route and no broker, dropping message ") +
                                    prettyPrintString (cmd));
                    }
                }
            }
        }
    }
CLOSE_TX_LOOP:
    brokerPushSocket.close ();

    routes.clear ();
    if (getRxStatus () == connection_status::connected)
    {
        try
        {
            controlSocket.send (std::string ("close"), ZMQ_NOBLOCK);
        }
        catch (const zmq::error_t &)
        {
            // this probably just means it got closed simultaneously which would be unusual but not impossible
        }
    }

    controlSocket.close ();

    setTxStatus (connection_status::terminated);
}

void ZmqComms::closeReceiver ()
{
    switch (getTxStatus ())
    {
    case connection_status::startup:
    case connection_status::connected:
    {
        ActionMessage cmd (CMD_PROTOCOL);
        cmd.messageID = CLOSE_RECEIVER;
        transmit (control_route, cmd);
    }

    break;
    default:
        if (!disconnecting)
        {
            // try connecting with the receivers push socket
            auto ctx = ZmqContextManager::getContextPointer ();
            zmq::socket_t pushSocket (ctx->getContext (), ZMQ_PUSH);
            pushSocket.setsockopt (ZMQ_LINGER, 200);
            if (localTargetAddress == "tcp://*")
            {
                pushSocket.connect (makePortAddress ("tcp://127.0.0.1", PortNumber));
            }
            else
            {
                pushSocket.connect (makePortAddress (localTargetAddress, PortNumber));
            }

            ActionMessage cmd (CMD_PROTOCOL);
            cmd.messageID = CLOSE_RECEIVER;
            pushSocket.send (cmd.to_string ());
        }
        break;
    }
}

}  // namespace zeromq
}  // namespace helics
