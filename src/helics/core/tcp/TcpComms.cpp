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
                goto CLOSE_RX_LOOP;

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

    while (true)
    {
        auto message = rxMessageQueue.pop();
        if (isProtocolCommand(message))
        {
            switch (message.index)
            {
            case CLOSE_RECEIVER:
            case DISCONNECT:
                goto CLOSE_RX_LOOP;

            }
        }
    }

CLOSE_RX_LOOP:
    disconnecting = true;
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
    tcp::resolver resolver (ioserv->getBaseService());
    bool closingRx = false;
    tcp::socket transmitSocket (ioserv->getBaseService());
    transmitSocket.open (tcp::v4 ());
  
    boost::system::error_code error;
    std::map<int, tcp::endpoint> routes;  // for all the other possible routes
    tcp::endpoint broker_endpoint;

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
            tcp::resolver::query query(tcp::v4(), brokerTarget_, std::to_string(brokerPort));
            // Setup the control socket for comms with the receiver
            broker_endpoint = *resolver.resolve(query);

            if (PortNumber <= 0)
            {
                ActionMessage m(CMD_PROTOCOL_PRIORITY);
                m.index = REQUEST_PORTS;
                transmitSocket.send_to(boost::asio::buffer(m.to_string()), broker_endpoint, 0, error);
                if (error)
                {
                    std::cerr << "error in initial send to broker " << error.message() << '\n';
                }
                std::vector<char> rx(128);
                tcp::endpoint brk;
                auto len=transmitSocket.receive_from(boost::asio::buffer(rx), brk);
                m = ActionMessage(rx.data(),len);
                if (isProtocolCommand(m))
                {
                    if (m.index == PORT_DEFINITIONS)
                    {
                        PortNumber = m.source_handle;
                        promisePort.set_value(PortNumber);
                    }
                    else if (m.index == DISCONNECT)
                    {
                        PortNumber = -1;
                        promisePort.set_value(-1);
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
            PortNumber = DEFAULT_Tcp_BROKER_PORT_NUMBER;
            promisePort.set_value(PortNumber);
        }
    }
    tcp::resolver::query queryLocal(tcp::v4(), localTarget_, std::to_string(PortNumber));

    tcp::endpoint rxEndpoint= *resolver.resolve(queryLocal);
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
                        tcp::resolver::query queryNew(tcp::v4(), interface,port);

                        routes.emplace (cmd.dest_id, *resolver.resolve(queryNew));
                    }
                    catch (std::exception &e)
                    {
                        // TODO:: do something???
                    }
                    processed = true;
                }
                break;
                case CLOSE_RECEIVER:
                    transmitSocket.send_to(boost::asio::buffer(cmd.to_string()), rxEndpoint, 0, error);
                    if (error)
                    {
                        std::cerr << "transmit failure to receiver " << error.message() << '\n';
                    }
                    closingRx = true;
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
                transmitSocket.send_to(boost::asio::buffer(cmd.to_string()), broker_endpoint,0,error);
                if (error)
                {
                    std::cerr << "transmit failure to broker " << error.message() << '\n';
                }
            }
        }
        else if (route_id == -1)
        {  // send to rx thread loop
            transmitSocket.send_to(boost::asio::buffer(cmd.to_string()), rxEndpoint,0,error);
            if (error)
            {
                std::cerr << "transmit failure to receiver " << error.message() << '\n';
            }
        }
        else
        {
            auto rt_find = routes.find (route_id);
            if (rt_find != routes.end ())
            {
                transmitSocket.send_to(boost::asio::buffer(cmd.to_string()), rt_find->second,0,error);
                if (error)
                {
                    std::cerr << "transmit failure to route to "<<route_id<<" " << error.message() << '\n';
                }
            }
            else
            {
                if (hasBroker)
                {
                    transmitSocket.send_to(boost::asio::buffer(cmd.to_string()), broker_endpoint,0,error);
                    if (error)
                    {
                        std::cerr << "transmit failure to broker " << error.message() << '\n';
                    }
                }
            }
        }
    }
CLOSE_TX_LOOP:
    
    routes.clear ();
    if (rx_status == connection_status::connected)
    {
        if (closingRx)
        {
            int cnt = 0;
            while (rx_status == connection_status::connected)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                ++cnt;
                if (cnt == 30)
                {
                    std::string cls("close");
                    transmitSocket.send_to(boost::asio::buffer(cls), rxEndpoint, 0, error);
                    if (error)
                    {
                        std::cerr << "transmit failure II to receiver" << error.message() << '\n';
                    }
                }
                if (cnt > 60)
                {
                    break;
                }
            }

        }
        else
        {
            std::string cls("close");
            transmitSocket.send_to(boost::asio::buffer(cls), rxEndpoint, 0, error);
            if (error)
            {
                std::cerr << "transmit failure to receiver" << error.message() << '\n';
            }
        }
       
    }


    tx_status = connection_status::terminated;
}

void TcpComms::closeTransmitter ()
{
    ActionMessage rt (CMD_PROTOCOL);
    rt.index = DISCONNECT;
    transmit (-1, rt);
}

void TcpComms::closeReceiver ()
{
    if (tx_status == connection_status::connected)
    {
        ActionMessage cmd (CMD_PROTOCOL);
        cmd.index = CLOSE_RECEIVER;
        transmit (-1, cmd);
    }
    else if (!disconnecting)
    {
       

        try
        {
            auto serv = AsioServiceManager::getServicePointer();
            if (serv)
            {
                // try connecting with the receiver socket
                tcp::resolver resolver(serv->getBaseService());
                tcp::resolver::query queryLocal(tcp::v4(), localTarget_, std::to_string(PortNumber));

                tcp::endpoint rxEndpoint = *resolver.resolve(queryLocal);

                tcp::socket transmit(serv->getBaseService(),tcp::endpoint(tcp::v4(),0));
                std::string cls("close");
                boost::system::error_code error;
                transmit.send_to(boost::asio::buffer(cls), rxEndpoint,0,error);
                if (error)
                {
                    std::cerr << "transmit failure on disconnect " << error.message() << '\n';
                }
            }
            
        }
        catch (...)
        {
            //ignore error here
       }
    }
}

std::string TcpComms::getAddress () const { return makePortAddress (localTarget_, PortNumber); }
}  // namespace helics