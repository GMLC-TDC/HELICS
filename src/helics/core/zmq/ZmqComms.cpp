/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "ZmqComms.h"
#include "ZmqRequestSets.h"
#include "helics/common/zmqContextManager.h"
#include "helics/common/zmqHelper.h"
#include "helics/common/zmqSocketDescriptor.h"
#include "helics/core/ActionMessage.h"
#include <memory>

static const int BEGIN_OPEN_PORT_RANGE = 23500;
static const int BEGIN_OPEN_PORT_RANGE_SUBBROKER = 23757;

static const int DEFAULT_BROKER_PULL_PORT_NUMBER = 23405;
static const int DEFAULT_BROKER_REP_PORT_NUMBER = 23404;

namespace helics
{
ZmqComms::ZmqComms (const std::string &brokerTarget, const std::string &localTarget)
    : CommsInterface (brokerTarget, localTarget)
{
    if (localTarget_.empty ())
    {
        localTarget_ = "tcp://127.0.0.1";
    }
}
/** destructor*/
ZmqComms::~ZmqComms () { disconnect (); }

std::string makePortAddress (const std::string &networkInterface, int portNumber)
{
    if (networkInterface.compare (0, 3, "ipc") == 0)
    {
        return networkInterface;
    }
    if (networkInterface.compare (0, 3, "tcp") == 0)
    {
        std::string newAddress = networkInterface;
        newAddress.push_back (':');
        newAddress.append (std::to_string (portNumber));
        return newAddress;
    }
    if (networkInterface.compare (0, 5, "inproc") == 0)
    {
        return networkInterface;
    }
    return networkInterface;
}

std::pair<std::string, int> extractInterfaceandPort (const std::string &address)
{
    std::pair<std::string, int> ret;
    auto lastColon = address.find_last_of (':');
    try
    {
        auto val = std::stoi (address.substr (lastColon + 1));
        ret.first = address.substr (0, lastColon);
        ret.second = val;
    }
    catch (const std::invalid_argument &)
    {
        ret = std::make_pair (address, -1);
    }
    return ret;
}

void ZmqComms::setBrokerPorts (int reqPort, int pushPort)
{
    if (rx_status == connection_status::startup)
    {
        brokerReqPort = reqPort;
        if (pushPort < 0)
        {
            if (brokerPushPort < 0)
            {
                brokerPushPort = reqPort + 1;
            }
        }
        else
        {
            brokerPushPort = pushPort;
        }
    }
}

std::pair<int, int> ZmqComms::findOpenPorts ()
{
    int start = openPortStart;
    if (openPortStart < 0)
    {
        start = (hasBroker) ? BEGIN_OPEN_PORT_RANGE_SUBBROKER : BEGIN_OPEN_PORT_RANGE;
    }
    while (usedPortNumbers.find (start) != usedPortNumbers.end ())
    {
        start += 2;
    }
    usedPortNumbers.insert (start);
    usedPortNumbers.insert (start + 1);
    return std::make_pair (start, start + 1);
}

void ZmqComms::setPortNumbers (int repPort, int pullPort)
{
    if (rx_status == connection_status::startup)
    {
        repPortNumber = repPort;
        if (repPort > 0)
        {
            auto currentPullPort = pullPortNumber.load ();
            pullPortNumber = (pullPort > 0) ? pullPort : (currentPullPort < 0) ? repPort + 1 : currentPullPort;
        }
    }
}

void ZmqComms::setAutomaticPortStartPort (int startingPort) { openPortStart = startingPort; }

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
    if (M.action () == CMD_PROTOCOL)
    {
        switch (M.index)
        {
        case CLOSE_RECEIVER:
            return (-1);
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
    if (M.action () == CMD_PROTOCOL)
    {
        switch (M.index)
        {
        case QUERY_PORTS:
        {
            ActionMessage portReply (CMD_PROTOCOL);
            portReply.index = PORT_DEFINITIONS;
            portReply.source_id = pullPortNumber;
            sock.send (portReply.to_string ());
        }
        break;
        case REQUEST_PORTS:
        {
            auto openPorts = findOpenPorts ();
            ActionMessage portReply (CMD_PROTOCOL);
            portReply.index = PORT_DEFINITIONS;
            portReply.source_id = pullPortNumber;
            portReply.source_handle = openPorts.first;
            portReply.dest_id = openPorts.second;
            sock.send (portReply.to_string ());
        }
        break;
        case CLOSE_RECEIVER:
            return (-1);
        default:
            M.index = NULL_REPLY;
            sock.send (M.to_string ());
            break;
        }
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
    auto ctx = zmqContextManager::getContextPointer ();
    zmq::socket_t pullSocket (ctx->getContext (), ZMQ_PULL);

    zmq::socket_t controlSocket (ctx->getContext (), ZMQ_PAIR);
    std::string controlsockString = std::string ("inproc://") + name + "_control";
    controlSocket.bind (controlsockString.c_str ());

    zmq::socket_t repSocket (ctx->getContext (), ZMQ_REP);
    while (pullPortNumber == -1)
    {
        zmq::message_t msg;
        controlSocket.recv (&msg);
        ActionMessage M (static_cast<char *> (msg.data ()), msg.size ());
        if (M.action () == CMD_PROTOCOL)
        {
            if (M.index == PORT_DEFINITIONS)
            {
                pullPortNumber = M.dest_id;
                if (repPortNumber < 0)
                {
                    repPortNumber = M.source_handle;  // aliased for the protocol message
                }
            }
            else if (M.index == DISCONNECT)
            {
                disconnecting = true;
                rx_status = connection_status::terminated;
                return;
            }
        }
    }
    try
    {
        repSocket.bind (makePortAddress (localTarget_, repPortNumber));
    }
    catch (const zmq::error_t &)
    {
        std::cerr << "binding error on reply socket sleeping then will try again \n";
        std::this_thread::sleep_for (std::chrono::milliseconds (100));
        try
        {
            repSocket.bind (makePortAddress (localTarget_, repPortNumber));
        }
        catch (const zmq::error_t &ze)
        {
            pullSocket.close ();
            disconnecting = true;
            std::cerr << "zmqError binding rep port " << makePortAddress (localTarget_, repPortNumber)
                      << ze.what () << '\n';
            rx_status = connection_status::error;
            return;
        }
    }
    try
    {
        pullSocket.bind (makePortAddress (localTarget_, pullPortNumber));
    }
    catch (const zmq::error_t &)
    {
        std::cerr << "binding error on pull socket sleeping then will try again \n";
        std::this_thread::sleep_for (std::chrono::milliseconds (100));
        try
        {
            pullSocket.bind (makePortAddress (localTarget_, pullPortNumber));
        }
        catch (const zmq::error_t &ze)
        {
            disconnecting = true;
            std::cerr << "zmqError binding pull port " << makePortAddress (localTarget_, pullPortNumber)
                      << ze.what () << '\n';
            rx_status = connection_status::error;
            return;
        }
    }

    std::vector<zmq::pollitem_t> poller (3);
    poller[0].socket = static_cast<void *> (controlSocket);
    poller[0].events = ZMQ_POLLIN;
    poller[1].socket = static_cast<void *> (pullSocket);
    poller[1].events = ZMQ_POLLIN;
    poller[2].socket = static_cast<void *> (repSocket);
    poller[2].events = ZMQ_POLLIN;
    rx_status = connection_status::connected;  // this is a atomic indicator that the rx queue is ready
    while (true)
    {
        auto rc = zmq::poll (poller);
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
    disconnecting = true;
    rx_status = connection_status::terminated;
}

int ZmqComms::initializeBrokerConnections (zmq::socket_t &controlSocket)
{
    zmq::pollitem_t poller;
    if (!brokerTarget_.empty ())
    {
        auto ctx = zmqContextManager::getContextPointer ();
        if (brokerReqPort < 0)
        {
            brokerReqPort = DEFAULT_BROKER_REP_PORT_NUMBER;
        }
        zmq::socket_t brokerReq (ctx->getContext (), ZMQ_REQ);
        try
        {
            brokerReq.connect (makePortAddress (brokerTarget_, brokerReqPort));
        }
        catch (zmq::error_t &ze)
        {
            std::cerr << "unable to connect with broker:" << ze.what () << '\n';
            tx_status = connection_status::error;
            return (-1);
        }

        hasBroker = true;
        int cnt = 0;
        zmq::message_t msg;
        if (pullPortNumber > 0)
        {
            while (brokerPushPort < 0)
            {
                ActionMessage getPorts (CMD_PROTOCOL);
                getPorts.index = QUERY_PORTS;
                auto str = getPorts.to_string ();
                brokerReq.send (str);
                poller.socket = static_cast<void *> (brokerReq);
                poller.events = ZMQ_POLLIN;
                auto rc = zmq::poll (&poller, 1, std::chrono::milliseconds (3000));
                if (rc < 0)
                {
                    std::cerr << "unable to connect with broker\n";
                    tx_status = connection_status::error;
                    return (-1);
                }
                if (rc == 0)
                {
                    std::cerr << "broker connection timed out\n";
                    tx_status = connection_status::error;
                    return (-1);
                }
                brokerReq.recv (&msg);

                ActionMessage rxcmd (static_cast<char *> (msg.data ()), msg.size ());
                if (rxcmd.action () == CMD_PROTOCOL)
                {
                    if (rxcmd.index == PORT_DEFINITIONS)
                    {
                        if (rxcmd.source_id > 0)
                        {
                            brokerPushPort = rxcmd.source_id;
                        }
                    }
                    else if (rxcmd.index == DISCONNECT)
                    {
                        controlSocket.send (msg);
                        tx_status = connection_status::terminated;
                        return (-3);
                    }
                }

                ++cnt;
                if (cnt > 10)
                {
                    // we can't get the broker to respond with port numbers
                    tx_status = connection_status::error;
                    return (-1);
                }
            }
        }
        else
        {
            while (pullPortNumber == -1)
            {
                ActionMessage getPorts (CMD_PROTOCOL);
                getPorts.index = REQUEST_PORTS;
                auto str = getPorts.to_string ();
                brokerReq.send (str);
                poller.socket = static_cast<void *> (brokerReq);
                poller.events = ZMQ_POLLIN;
                auto rc = zmq::poll (&poller, 1, std::chrono::milliseconds (3000));
                if (rc < 0)
                {
                    std::cerr << "unable to connect with broker\n";
                    tx_status = connection_status::error;
                    return (-1);
                }
                if (rc == 0)
                {
                    std::cerr << "broker connection timed out\n";
                    tx_status = connection_status::error;
                    return (-1);
                }
                brokerReq.recv (&msg);

                ActionMessage rxcmd (static_cast<char *> (msg.data ()), msg.size ());
                if (rxcmd.action () == CMD_PROTOCOL)
                {
                    if (rxcmd.index == PORT_DEFINITIONS)
                    {
                        if (rxcmd.source_id > 0)
                        {
                            brokerPushPort = rxcmd.source_id;
                        }
                        controlSocket.send (msg);
                    }
                    else if (rxcmd.index == DISCONNECT)
                    {
                        controlSocket.send (msg);
                        tx_status = connection_status::terminated;
                        return (-3);
                    }
                }

                ++cnt;
                if (cnt > 10)
                {
                    // we can't get the broker to respond with port numbers
                    tx_status = connection_status::error;
                    return (-1);
                }
            }
        }
    }
    else
    {
        if ((pullPortNumber == -1) || (repPortNumber == -1))
        {
            if (pullPortNumber == -1)
            {
                pullPortNumber = DEFAULT_BROKER_PULL_PORT_NUMBER;
            }

            if (repPortNumber == -1)
            {
                repPortNumber = DEFAULT_BROKER_REP_PORT_NUMBER;
            }
            ActionMessage setPorts (CMD_PROTOCOL);
            setPorts.index = PORT_DEFINITIONS;
            setPorts.dest_id = pullPortNumber;
            setPorts.source_handle = repPortNumber;
            auto str = setPorts.to_string ();
            controlSocket.send (str);
        }
    }
    return 0;
}

void ZmqComms::queue_tx_function ()
{
    std::vector<char> buffer;

    auto ctx = zmqContextManager::getContextPointer ();

    // Setup the control socket for comms with the receiver
    zmq::socket_t controlSocket (ctx->getContext (), ZMQ_PAIR);

    std::string controlsockString = std::string ("inproc://") + name + "_control";
    controlSocket.connect (controlsockString);
    auto res = initializeBrokerConnections (controlSocket);
    if (res < 0)
    {
        return;
    }

    zmq::socket_t brokerPushSocket (ctx->getContext (), ZMQ_PUSH);
    std::map<int, zmq::socket_t> routes;  // for all the other possible routes
    ZmqRequestSets priority_routes;  //!< object to handle the management of the priority routes

    if (hasBroker)
    {
        priority_routes.addRoutes (0, makePortAddress (brokerTarget_, brokerReqPort));
        brokerPushSocket.connect (makePortAddress (brokerTarget_, brokerPushPort));
    }
    tx_status = connection_status::connected;
    zmq::message_t msg;
    while (true)
    {
        int route_id;
        ActionMessage cmd;
        if (priority_routes.waiting ())
        {
            if (priority_routes.checkForMessages () != 0)
            {
                auto M = priority_routes.getMessage ();
                if (M)
                {
                    ActionCallback (std::move (*M));
                }
                continue;
            }
            auto txm = txQueue.try_pop ();
            if (txm)
            {
                std::tie (route_id, cmd) = *txm;
            }
            else
            {
                if (priority_routes.checkForMessages (std::chrono::milliseconds (50)) != 0)
                {
                    auto M = priority_routes.getMessage ();
                    if (M)
                    {
                        ActionCallback (std::move (*M));
                    }
                }
                continue;
            }
        }
        else
        {
            std::tie (route_id, cmd) = txQueue.pop ();
        }

        if (cmd.action () == CMD_PROTOCOL)
        {
            if (route_id == -1)
            {
                switch (cmd.index)
                {
                case NEW_ROUTE:
                {
                    auto newroute = cmd.payload;
                    auto splitPt = newroute.find_first_of (';');
                    std::string priority_route;
                    std::string push_route;
                    if (splitPt != std::string::npos)
                    {
                        priority_route = newroute.substr (0, splitPt);
                        push_route = newroute.substr (splitPt + 1);
                    }
                    else
                    {
                        push_route = newroute;
                    }
                    if (!priority_route.empty ())
                    {
                        try
                        {
                            priority_routes.addRoutes (cmd.dest_id, priority_route);
                        }
                        catch (const zmq::error_t &)
                        {
                            // TODO:: do something???
                        }
                    }
                    try
                    {
                        auto zsock = zmq::socket_t (ctx->getContext (), ZMQ_PUSH);
                        zsock.connect (push_route);
                        routes.emplace (cmd.dest_id, std::move (zsock));
                    }
                    catch (const zmq::error_t &)
                    {
                        // TODO:: do something???
                    }
                }
                break;
                case DISCONNECT:
                    goto CLOSE_TX_LOOP;  // break out of loop
                }
            }
        }

        if (isPriorityCommand (cmd))
        {
            if ((route_id == 0) && (!hasBroker))
            {
                // drop the packet
                continue;
            }
            auto tx = priority_routes.transmit (route_id, cmd);
            if (tx)
            {
                continue;
            }
        }
        cmd.to_vector (buffer);
        if (route_id == 0)
        {
            if (hasBroker)
            {
                brokerPushSocket.send (buffer.data (), buffer.size ());
            }
        }
        else if (route_id == -1)
        {  // send to rx thread loop
            controlSocket.send (buffer.data (), buffer.size ());
        }
        else
        {
            auto rt_find = routes.find (route_id);
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
            }
        }
    }
CLOSE_TX_LOOP:
    brokerPushSocket.close ();

    routes.clear ();
    if (rx_status == connection_status::connected)
    {
        controlSocket.send ("close");
    }

    controlSocket.close ();

    tx_status = connection_status::terminated;
}

void ZmqComms::closeTransmitter ()
{
    ActionMessage rt (CMD_PROTOCOL);
    rt.index = DISCONNECT;
    transmit (-1, rt);
}

void ZmqComms::closeReceiver ()
{
    if (tx_status == connection_status::connected)
    {
        ActionMessage cmd (CMD_PROTOCOL);
        cmd.index = CLOSE_RECEIVER;
        transmit (-1, cmd);
    }
    else if (!disconnecting)
    {
        // try connecting with the receivers push socket
        auto ctx = zmqContextManager::getContextPointer ();
        zmq::socket_t pushSocket (ctx->getContext (), ZMQ_PUSH);
        if (localTarget_ == "tcp://*")
        {
            pushSocket.connect (makePortAddress ("tcp://127.0.0.1", pullPortNumber));
        }
        else
        {
            pushSocket.connect (makePortAddress (localTarget_, pullPortNumber));
        }

        ActionMessage cmd (CMD_PROTOCOL);
        cmd.index = CLOSE_RECEIVER;
        pushSocket.send (cmd.to_string ());
    }
}

std::string ZmqComms::getPushAddress () const { return makePortAddress (localTarget_, pullPortNumber); }

std::string ZmqComms::getRequestAddress () const { return makePortAddress (localTarget_, repPortNumber); }
}  // namespace helics