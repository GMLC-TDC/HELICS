/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "TcpComms.h"
#include "../ActionMessage.h"
#include "../../common/AsioServiceManager.h"
#include <memory>
#include "TcpHelperClasses.h"

static const int BEGIN_OPEN_PORT_RANGE = 24228;
static const int BEGIN_OPEN_PORT_RANGE_SUBBROKER = 24357;

static const int DEFAULT_TCP_BROKER_PORT_NUMBER = 24160;


namespace helics
{
using boost::asio::ip::tcp;
TcpComms::TcpComms ()
{

}

TcpComms::TcpComms (const std::string &brokerTarget, const std::string &localTarget)
    : CommsInterface (brokerTarget, localTarget)
{
    if (localTarget_.empty ())
    {
        localTarget_ = "localhost";
    }
}
/** destructor*/
TcpComms::~TcpComms () { disconnect (); }

void TcpComms::setBrokerPort (int brokerPortNumber)
{
    if (rx_status == connection_status::startup)
    {
        brokerPort = brokerPortNumber;
    }
}

int TcpComms::findOpenPort ()
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
    return start;
}

void TcpComms::setPortNumber (int localPortNumber)
{
    if (rx_status == connection_status::startup)
    {
        PortNumber = localPortNumber;
    }
}

void TcpComms::setAutomaticPortStartPort (int startingPort) { openPortStart = startingPort; }

int TcpComms::processIncomingMessage (ActionMessage &M)
{
    if (isProtocolCommand(M))
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

ActionMessage TcpComms::generateReplyToIncomingMessage (ActionMessage &M)
{
    if (isProtocolCommand(M))
    {
        switch (M.index)
        {
        case QUERY_PORTS:
        {
            ActionMessage portReply (CMD_PROTOCOL);
            portReply.index = PORT_DEFINITIONS;
            portReply.source_id = PortNumber;
            return portReply;
        }
        break;
        case REQUEST_PORTS:
        {
            auto openPort = findOpenPort ();
            ActionMessage portReply (CMD_PROTOCOL);
            portReply.index = PORT_DEFINITIONS;
            portReply.source_id = PortNumber;
            portReply.source_handle = openPort;
            return portReply;
        }
        break;
        case CLOSE_RECEIVER:
            return M;
        default:
            M.index = NULL_REPLY;
            return M;
        }
    }
    ActionCallback (std::move (M));
    ActionMessage resp (CMD_PRIORITY_ACK);
    return resp;
}




void TcpComms::queue_rx_function ()
{
    while (PortNumber < 0)
    {
        auto message = rxMessageQueue.pop();
        if (isProtocolCommand(message))
        {
            switch (message.index)
            {
            case PORT_DEFINITIONS:
                PortNumber = message.source_handle;
                break;
            case CLOSE_RECEIVER:
            case DISCONNECT:
                disconnecting = true;
                rx_status = connection_status::terminated;
                return;

            }
        }
    }
    if (PortNumber < 0)
    {
        rx_status = connection_status::error;
        return;
    }
    auto ioserv = AsioServiceManager::getServicePointer();
    tcp_server server(ioserv->getBaseService(),PortNumber,maxMessageSize_);
    ioserv->runServiceLoop();
    server.start_accept();
    while (true)
    {
        auto message = rxMessageQueue.pop();
        if (isProtocolCommand(message))
        {
            switch (message.index)
            {
            case CLOSE_RECEIVER:
            case DISCONNECT:
                disconnecting = true;
                ioserv->haltServiceLoop();
                rx_status = connection_status::terminated;
                return;

            }
        }
    }

    disconnecting = true;
    ioserv->haltServiceLoop();
    rx_status = connection_status::terminated;
    return;
}
/*
        auto len=socket.receive_from(boost::asio::buffer(data), remote_endp, 0, error);
        if (error)
        {
            rx_status = connection_status::error;
            return;
        }
        if (len == 5)
        {
            std::string str(data.data(), len);
            if (str == "close")
            {
                goto CLOSE_RX_LOOP;
            }
        }
        ActionMessage M(data.data(),len);
        if (isProtocolCommand(M))
        {
           if (M.index == CLOSE_RECEIVER)
            {
               goto CLOSE_RX_LOOP;
            }
        }
        if (isPriorityCommand(M))
        {
            auto reply = generateReplyToIncomingMessage(M);
            if (isProtocolCommand(reply))
            {
                if (reply.index == DISCONNECT)
                {
                    goto CLOSE_RX_LOOP;
                }
            }
            socket.send_to(boost::asio::buffer(reply.to_string()), remote_endp, 0, ignored_error);
        }
        else
        {
            auto res=processIncomingMessage(M);
            if (res < 0)
            {
                goto CLOSE_RX_LOOP;
            }
        }
        
    }

*/


void TcpComms::queue_tx_function ()
{
    std::vector<char> buffer;
    auto ioserv = AsioServiceManager::getServicePointer();
    tcp_connection::pointer brokerConnection;
  
    boost::system::error_code error;
    std::map<int, tcp_connection::pointer> routes;  // for all the other possible routes
    if (!brokerTarget_.empty())
    {
        hasBroker = true;
    }
    if (hasBroker)
    {
        if (brokerPort < 0)
        {
            brokerPort = DEFAULT_TCP_BROKER_PORT_NUMBER;
        }
        try
        {
            brokerConnection = tcp_connection::create(ioserv->getBaseService(), brokerTarget_, std::to_string(brokerPort), maxMessageSize_);
           

            if (PortNumber <= 0)
            {
                ActionMessage m(CMD_PROTOCOL_PRIORITY);
                m.index = REQUEST_PORTS;
                brokerConnection->send(m.to_string());
             
                if (error)
                {
                    std::cerr << "error in initial send to broker " << error.message() << '\n';
                }
                std::vector<char> rx(128);
                tcp::endpoint brk;
                auto len=brokerConnection->receive(rx.data(),128);
                m = ActionMessage(rx.data(),len);
                if (isProtocolCommand(m))
                {
                    if (m.index == PORT_DEFINITIONS)
                    {
                        rxMessageQueue.push(m);
                    }
                    else if (m.index == DISCONNECT)
                    {
                        PortNumber = -1;
                        rxMessageQueue.push(m);
                        tx_status = connection_status::terminated;
                        return;
                    }
                }
            }
        }
        catch (std::exception &e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
    else
    {
        if (PortNumber < 0)
        {
            PortNumber = DEFAULT_TCP_BROKER_PORT_NUMBER;
            ActionMessage m(CMD_PROTOCOL);
            m.index = PORT_DEFINITIONS;
            m.source_handle = PortNumber;
            rxMessageQueue.push(m);
        }
    }
    tx_status = connection_status::connected;
    
    while (true)
    {
        int route_id;
        ActionMessage cmd;
       
        std::tie (route_id, cmd) = txQueue.pop ();
        bool processed = false;
        if (isProtocolCommand(cmd))
        {
            if (route_id == -1)
            {
                switch (cmd.index)
                {
                case NEW_ROUTE:
                {
                    auto newroute = cmd.payload;
                 
                    try
                    {
                        std::string interface;
                        std::string port;
                        std::tie(interface, port) = extractInterfaceandPortString(newroute);
                        auto new_connect = tcp_connection::create(ioserv->getBaseService(), interface, port);

                        routes.emplace (cmd.dest_id, std::move(new_connect));
                    }
                    catch (std::exception &e)
                    {
                        // TODO:: do something???
                    }
                    processed = true;
                }
                break;
                case CLOSE_RECEIVER:
                    rxMessageQueue.push(cmd);
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
       
        if (route_id == 0)
        {
            if (hasBroker)
            {
                brokerConnection->send(cmd.to_string());
                
                if (error)
                {
                    std::cerr << "transmit failure to broker " << error.message() << '\n';
                }
            }
        }
        else if (route_id == -1)
        {  // send to rx thread loop
            rxMessageQueue.push(cmd);
        }
        else
        {
            auto rt_find = routes.find (route_id);
            if (rt_find != routes.end ())
            {
                rt_find->second->send(cmd.to_string());
            }
            else
            {
                if (hasBroker)
                {
                    brokerConnection->send(cmd.to_string());
                }
            }
        }
    }
CLOSE_TX_LOOP:
    
    routes.clear ();
    if (rx_status == connection_status::connected)
    {
        closeReceiver();
    }
    tx_status = connection_status::terminated;
}

void TcpComms::closeReceiver ()
{
    ActionMessage cmd (CMD_PROTOCOL);
    cmd.index = CLOSE_RECEIVER;
    rxMessageQueue.push(cmd);
}

std::string TcpComms::getAddress () const { return makePortAddress (localTarget_, PortNumber); }
}  // namespace helics