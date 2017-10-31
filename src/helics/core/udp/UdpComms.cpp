/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "UdpComms.h"
#include "helics/core/ActionMessage.h"
#include "helics/common/AsioServiceManager.h"
#include <memory>
#include <boost/asio/ip/udp.hpp>

static const int BEGIN_OPEN_PORT_RANGE = 23964;
static const int BEGIN_OPEN_PORT_RANGE_SUBBROKER = 24093;

static const int DEFAULT_UDP_BROKER_PORT_NUMBER = 23901;


namespace helics
{
using boost::asio::ip::udp;
UdpComms::UdpComms ()
{
    promisePort = std::promise<int> ();
    futurePort = promisePort.get_future ();
}

UdpComms::UdpComms (const std::string &brokerTarget, const std::string &localTarget)
    : CommsInterface (brokerTarget, localTarget)
{
    if (localTarget_.empty ())
    {
        localTarget_ = "localhost";
    }
    promisePort = std::promise<int> ();
    futurePort = promisePort.get_future ();
}
/** destructor*/
UdpComms::~UdpComms () { disconnect (); }

void UdpComms::setBrokerPort (int brokerPortNumber)
{
    if (rx_status == connection_status::startup)
    {
        brokerPort = brokerPortNumber;
    }
}

int UdpComms::findOpenPort ()
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

void UdpComms::setPortNumber (int localPortNumber)
{
    if (rx_status == connection_status::startup)
    {
        PortNumber = localPortNumber;
    }
}

void UdpComms::setAutomaticPortStartPort (int startingPort) { openPortStart = startingPort; }

int UdpComms::processIncomingMessage (ActionMessage &M)
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

ActionMessage UdpComms::generateReplyToIncomingMessage (ActionMessage &M)
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

void UdpComms::queue_rx_function ()
{
    if (PortNumber < 0)
    {
        PortNumber = futurePort.get();
    }
    if (PortNumber < 0)
    {
        rx_status = connection_status::error;
        return;
    }
    auto ioserv = AsioServiceManager::getServicePointer();
    udp::socket socket(ioserv->getBaseService(), udp::endpoint(udp::v4(), PortNumber));
    std::vector<char> data(10192);
    udp::endpoint remote_endp;
    boost::system::error_code error;
    boost::system::error_code ignored_error;
    rx_status = connection_status::connected;
    while (true)
    {
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
                disconnecting = true;
                rx_status = connection_status::terminated;
            }
        }
        ActionMessage M(data.data(),len);
        if (isProtocolCommand(M))
        {
           if (M.index == CLOSE_RECEIVER)
            {
                disconnecting = true;
                rx_status = connection_status::terminated;
                return;
            }
        }
        if (isPriorityCommand(M))
        {
            auto reply = generateReplyToIncomingMessage(M);
            if (isProtocolCommand(reply))
            {
                if (reply.index == DISCONNECT)
                {
                    disconnecting = true;
                    rx_status = connection_status::terminated;
                    return;
                }
            }
            socket.send_to(boost::asio::buffer(reply.to_string()), remote_endp, 0, ignored_error);
        }
        else
        {
            auto res=processIncomingMessage(M);
            if (res < 0)
            {
                disconnecting = true;
                rx_status = connection_status::terminated;
            }
        }
        
    }
}


void UdpComms::queue_tx_function ()
{
    std::vector<char> buffer;
    auto ioserv = AsioServiceManager::getServicePointer();
    udp::resolver resolver (ioserv->getBaseService());
  
    udp::socket transmitSocket (ioserv->getBaseService());
    transmitSocket.open (udp::v4 ());
    if (PortNumber >= 0)
    {
        promisePort.set_value(PortNumber);
    }
    
    std::map<int, udp::endpoint> routes;  // for all the other possible routes
    udp::endpoint broker_endpoint;

    if (!brokerTarget_.empty())
    {
        hasBroker = true;
    }
    if (hasBroker)
    {
        if (brokerPort < 0)
        {
            brokerPort = DEFAULT_UDP_BROKER_PORT_NUMBER;
        }
        try
        {
            udp::resolver::query query(udp::v4(), brokerTarget_, std::to_string(brokerPort));
            // Setup the control socket for comms with the receiver
            broker_endpoint = *resolver.resolve(query);

            if (PortNumber <= 0)
            {
                ActionMessage m(CMD_PROTOCOL_PRIORITY);
                m.index = REQUEST_PORTS;
                transmitSocket.send_to(boost::asio::buffer(m.to_string()), broker_endpoint);
                std::vector<char> rx(128);
                udp::endpoint brk;
                auto len=transmitSocket.receive_from(boost::asio::buffer(rx), brk);
                m = ActionMessage(rx.data(),len);
                if (isProtocolCommand(m))
                {
                    if (m.index == PORT_DEFINITIONS)
                    {
                        PortNumber = m.dest_id;
                        promisePort.set_value(PortNumber);
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
            PortNumber = DEFAULT_UDP_BROKER_PORT_NUMBER;
            promisePort.set_value(PortNumber);
        }
    }
    udp::resolver::query queryLocal(udp::v4(), localTarget_, std::to_string(PortNumber));

    udp::endpoint rxEndpoint= *resolver.resolve(queryLocal);
    tx_status = connection_status::connected;
    
    while (true)
    {
        int route_id;
        ActionMessage cmd;
       
        std::tie (route_id, cmd) = txQueue.pop ();

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
                        udp::resolver::query queryNew(udp::v4(), interface,port);

                       rxEndpoint = *resolver.resolve(queryNew);
                       
                        routes.emplace (cmd.dest_id, std::move (rxEndpoint));
                    }
                    catch (std::exception &e)
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

       
        if (route_id == 0)
        {
            if (hasBroker)
            {
                transmitSocket.send_to(boost::asio::buffer(cmd.to_string()), broker_endpoint);
            }
        }
        else if (route_id == -1)
        {  // send to rx thread loop
            transmitSocket.send_to(boost::asio::buffer(cmd.to_string()), rxEndpoint);
        }
        else
        {
            auto rt_find = routes.find (route_id);
            if (rt_find != routes.end ())
            {
                transmitSocket.send_to(boost::asio::buffer(cmd.to_string()), rt_find->second);
            }
            else
            {
                if (hasBroker)
                {
                    transmitSocket.send_to(boost::asio::buffer(cmd.to_string()), broker_endpoint);
                }
            }
        }
    }
CLOSE_TX_LOOP:
    
    routes.clear ();
    if (rx_status == connection_status::connected)
    {
        std::string cls("close");
        transmitSocket.send_to(boost::asio::buffer(cls), rxEndpoint);
    }


    tx_status = connection_status::terminated;
}

void UdpComms::closeTransmitter ()
{
    ActionMessage rt (CMD_PROTOCOL);
    rt.index = DISCONNECT;
    transmit (-1, rt);
}

void UdpComms::closeReceiver ()
{
    if (tx_status == connection_status::connected)
    {
        ActionMessage cmd (CMD_PROTOCOL);
        cmd.index = CLOSE_RECEIVER;
        transmit (-1, cmd);
    }
    else if (!disconnecting)
    {
        // try connecting with the receiver socket
        udp::resolver resolver(AsioServiceManager::getService());
        udp::resolver::query queryLocal(udp::v4(), localTarget_, std::to_string(PortNumber));

        udp::endpoint rxEndpoint = *resolver.resolve(queryLocal);

        udp::socket transmit(AsioServiceManager::getService());
        std::string cls("close");
        transmit.send_to(boost::asio::buffer(cls), rxEndpoint);
    }
}

std::string UdpComms::getAddress () const { return makePortAddress (localTarget_, PortNumber); }
}  // namespace helics