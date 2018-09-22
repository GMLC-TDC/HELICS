/*

Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "UdpComms.h"
#include "../../common/AsioServiceManager.h"
#include "../ActionMessage.hpp"
#include "../NetworkBrokerData.hpp"
#include <memory>
#include <boost/asio/ip/udp.hpp>
#include "../../common/fmt_format.h"

static const int BEGIN_OPEN_PORT_RANGE = 23964;
static const int BEGIN_OPEN_PORT_RANGE_SUBBROKER = 24053;

static const int DEFAULT_UDP_BROKER_PORT_NUMBER = 23901;

namespace helics
{
namespace udp
{
using boost::asio::ip::udp;
UdpComms::UdpComms ()
{
    promisePort = std::promise<int> ();
    futurePort = promisePort.get_future ();
}

   /** load network information into the comms object*/
void UdpComms::loadNetworkInfo (const NetworkBrokerData &netInfo)
{
    CommsInterface::loadNetworkInfo (netInfo);
    if (!propertyLock ())
    {
        return;
    }
    brokerPort = netInfo.brokerPort;
    PortNumber = netInfo.portNumber;
    if (localTarget_.empty ())
    {
        if ((brokerTarget_ == "udp://127.0.0.1") || (brokerTarget_ == "udp://localhost") || (brokerTarget_ == "localhost"))
        {
            localTarget_ = "localhost";
        }
        else if (brokerTarget_.empty())
        {
            switch (interfaceNetwork)
            {
            case interface_networks::local:
                localTarget_ = "localhost";
                break;
            default:
                localTarget_ = "*";
                break;
            }
        }
        else
        {
            localTarget_ = generateMatchingInterfaceAddress(brokerTarget_, interfaceNetwork);

        }
    }
    if (netInfo.portStart > 0)
    {
        openPortStart = netInfo.portStart;
    }
    promisePort = std::promise<int> ();
    futurePort = promisePort.get_future ();
    if (PortNumber > 0)
    {
        autoPortNumber = false;
    }
    propertyUnLock ();
}
/** destructor*/
UdpComms::~UdpComms () { disconnect (); }

void UdpComms::setBrokerPort (int brokerPortNumber)
{
    if (propertyLock())
    {
        brokerPort = brokerPortNumber;
        propertyUnLock ();
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
    if (propertyLock())
    {
        PortNumber = localPortNumber;
        if (PortNumber > 0)
        {
            autoPortNumber = false;
        }
        propertyUnLock ();
    }
}

void UdpComms::setAutomaticPortStartPort (int startingPort) 
{ 
	if (propertyLock ())
    {
        openPortStart = startingPort;
        propertyUnLock ();
    }
   
}

int UdpComms::processIncomingMessage (ActionMessage &M)
{
    if (isProtocolCommand (M))
    {
        switch (M.messageID)
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
    if (isProtocolCommand (M))
    {
        switch (M.messageID)
        {
        case QUERY_PORTS:
        {
            ActionMessage portReply (CMD_PROTOCOL);
            portReply.messageID = PORT_DEFINITIONS;
            portReply.source_id = PortNumber;
            return portReply;
        }
        break;
        case REQUEST_PORTS:
        {
            auto openPort = findOpenPort ();
            ActionMessage portReply (CMD_PROTOCOL);
            portReply.messageID = PORT_DEFINITIONS;
            portReply.source_id = PortNumber;
            portReply.source_handle = openPort;
            return portReply;
        }
        break;
        case CLOSE_RECEIVER:
            return M;
        default:
            M.messageID = NULL_REPLY;
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
        PortNumber = futurePort.get ();
    }
    if (PortNumber < 0)
    {
        setRxStatus( connection_status::error);
        return;
    }
    auto ioserv = AsioServiceManager::getServicePointer ();
    udp::socket socket (ioserv->getBaseService ());
    socket.open (udp::v4 ());
    int t_cnt = 0;
    bool bindsuccess = false;
    while (!bindsuccess)
    {
        try
        {
            socket.bind (udp::endpoint (udp::v4 (), PortNumber));
            bindsuccess = true;
        }
        catch (const boost::system::system_error &error)
        {
            if ((autoPortNumber) && (hasBroker))
            {  // If we failed and we are on an automatically assigned port number,  just try a different port
                int tries = 0;
				while (!bindsuccess)
				{
                    ++PortNumber;
                    try
                    {
                        socket.bind (udp::endpoint (udp::v4 (), PortNumber));
                        bindsuccess = true;
                    }
					catch (const boost::system::system_error &)
					{
                        ++tries;
						if (tries > 10)
						{
                            break;
						}
					}
				}
				if (bindsuccess)
				{
                    continue;
				}
				else
				{
                    disconnecting = true;
                    logError (fmt::format ("unable to bind socket {} :{}",
                                           makePortAddress (localTarget_, PortNumber), error.what ()));
                    socket.close ();
                    setRxStatus( connection_status::error);
                    return;
				}
                
            }
            if (t_cnt == 0)
            {
                logWarning (fmt::format ("bind error on UDP socket {} :{}", makePortAddress (localTarget_, PortNumber),
                                       error.what ()));
            }
            std::this_thread::sleep_for (std::chrono::milliseconds (200));
            t_cnt += 200;
            if (t_cnt > connectionTimeout)
            {
                disconnecting = true;
                logError (fmt::format ("unable to bind socket {} :{}", makePortAddress (localTarget_, PortNumber),
                                       error.what ()));
                socket.close ();
                setRxStatus( connection_status::error);
                return;
            }
        }
    }

    std::vector<char> data (10192);
    udp::endpoint remote_endp;
    boost::system::error_code error;
    boost::system::error_code ignored_error;
    setRxStatus( connection_status::connected);
    while (true)
    {
        auto len = socket.receive_from (boost::asio::buffer (data), remote_endp, 0, error);
        if (error)
        {
            setRxStatus( connection_status::error);
            return;
        }
        if (len == 5)
        {
            std::string str (data.data (), len);
            if (str == "close")
            {
                goto CLOSE_RX_LOOP;
            }
        }
        ActionMessage M (data.data (), len);
        if (!isValidCommand (M))
        {
            logWarning ("invalid command received udp");
            continue;
        }
        if (isProtocolCommand (M))
        {
            if (M.messageID == CLOSE_RECEIVER)
            {
                goto CLOSE_RX_LOOP;
            }
        }
        if (isPriorityCommand (M))
        {
            auto reply = generateReplyToIncomingMessage (M);
            if (isProtocolCommand (reply))
            {
                if (reply.messageID == DISCONNECT)
                {
                    goto CLOSE_RX_LOOP;
                }
            }
            socket.send_to (boost::asio::buffer (reply.to_string ()), remote_endp, 0, ignored_error);
        }
        else
        {
            auto res = processIncomingMessage (M);
            if (res < 0)
            {
                goto CLOSE_RX_LOOP;
            }
        }
    }
CLOSE_RX_LOOP:
    disconnecting = true;
    setRxStatus( connection_status::terminated);
}

void UdpComms::queue_tx_function ()
{
    std::vector<char> buffer;
    auto ioserv = AsioServiceManager::getServicePointer ();
    udp::resolver resolver (ioserv->getBaseService ());
    bool closingRx = false;
    udp::socket transmitSocket (ioserv->getBaseService ());
    transmitSocket.open (udp::v4 ());
    if (PortNumber >= 0)
    {
        promisePort.set_value (PortNumber);
    }

    boost::system::error_code error;
    std::map<int, udp::endpoint> routes;  // for all the other possible routes
    udp::endpoint broker_endpoint;

    if (!brokerTarget_.empty ())
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
            udp::resolver::query query (udp::v4 (), brokerTarget_, std::to_string (brokerPort));
            // Setup the control socket for comms with the receiver
            broker_endpoint = *resolver.resolve (query);

            if (PortNumber <= 0)
            {
                ActionMessage m (CMD_PROTOCOL_PRIORITY);
                m.messageID = REQUEST_PORTS;
                transmitSocket.send_to (boost::asio::buffer (m.to_string ()), broker_endpoint, 0, error);
                if (error)
                {
                    logError (fmt::format ("error in initial send to broker {}",
                                            error.message ()));
                }
                std::vector<char> rx (128);
                udp::endpoint brk;
                auto len = transmitSocket.receive_from (boost::asio::buffer (rx), brk);
                m = ActionMessage (rx.data (), len);
                if (isProtocolCommand (m))
                {
                    if (m.messageID == PORT_DEFINITIONS)
                    {
                        PortNumber = m.source_handle;
                        if (openPortStart < 0)
                        {
                            if (PortNumber < BEGIN_OPEN_PORT_RANGE_SUBBROKER)
                            {
                                openPortStart =
                                  BEGIN_OPEN_PORT_RANGE_SUBBROKER + (PortNumber - BEGIN_OPEN_PORT_RANGE) * 10;
                            }
                            else
                            {
                                openPortStart = BEGIN_OPEN_PORT_RANGE_SUBBROKER +
                                                (PortNumber - BEGIN_OPEN_PORT_RANGE_SUBBROKER) * 10 + 10;
                            }
                        }
                        promisePort.set_value (PortNumber);
                    }
                    else if (m.messageID == DISCONNECT)
                    {
                        PortNumber = -1;
                        promisePort.set_value (-1);
                        setTxStatus( connection_status::terminated);
                        return;
                    }
                }
            }
        }
        catch (std::exception &e)
        {
            std::cerr << e.what () << std::endl;
        }
    }
    else
    {
        if (PortNumber < 0)
        {
            PortNumber = DEFAULT_UDP_BROKER_PORT_NUMBER;
            promisePort.set_value (PortNumber);
        }
    }
    udp::resolver::query queryLocal (udp::v4 (), localTarget_, std::to_string (PortNumber));

    udp::endpoint rxEndpoint = *resolver.resolve (queryLocal);
    setTxStatus( connection_status::connected);

    while (true)
    {
        int route_id;
        ActionMessage cmd;

        std::tie (route_id, cmd) = txQueue.pop ();
        bool processed = false;
        if (isProtocolCommand (cmd))
        {
            if (route_id == -1)
            {
                switch (cmd.messageID)
                {
                case NEW_ROUTE:
                {
                    auto newroute = cmd.payload;

                    try
                    {
                        std::string interface;
                        std::string port;
                        std::tie (interface, port) = extractInterfaceandPortString (newroute);
                        udp::resolver::query queryNew (udp::v4 (), interface, port);

                        routes.emplace (cmd.dest_id, *resolver.resolve (queryNew));
                    }
                    catch (std::exception &e)
                    {
                        // TODO:: do something???
                    }
                    processed = true;
                }
                break;
                case CLOSE_RECEIVER:
                    transmitSocket.send_to (boost::asio::buffer (cmd.to_string ()), rxEndpoint, 0, error);
                    if (error)
                    {
                        logError (fmt::format ("transmit failure on sending 'close' to receiver  {}", error.message ()));
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
                transmitSocket.send_to (boost::asio::buffer (cmd.to_string ()), broker_endpoint, 0, error);
                if (error)
                {
                    logWarning (
                      fmt::format ("transmit failure sending to broker  {}", error.message ()));
                }
            }
        }
        else if (route_id == -1)
        {  // send to rx thread loop
            transmitSocket.send_to (boost::asio::buffer (cmd.to_string ()), rxEndpoint, 0, error);
            if (error)
            {
                logWarning (fmt::format ("transmit failure sending control message to receiver  {}", error.message ()));
            }
        }
        else
        {
            auto rt_find = routes.find (route_id);
            if (rt_find != routes.end ())
            {
                transmitSocket.send_to (boost::asio::buffer (cmd.to_string ()), rt_find->second, 0, error);
                if (error)
                {
                    logWarning (fmt::format ("transmit failure sending to route {}:{}", route_id,error.message ()));
                }
            }
            else
            {
                if (hasBroker)
                {
                    transmitSocket.send_to (boost::asio::buffer (cmd.to_string ()), broker_endpoint, 0, error);
                    if (error)
                    {
                        logWarning (fmt::format ("transmit failure sending to broker  {}", error.message ()));
                    }
                }
            }
        }
    }
CLOSE_TX_LOOP:

    routes.clear ();
    if (getRxStatus() == connection_status::connected)
    {
        if (closingRx)
        {
            int cnt = 0;
            while (getRxStatus() == connection_status::connected)
            {
                std::this_thread::sleep_for (std::chrono::milliseconds (100));
                ++cnt;
                if (cnt == 30)
                {
                    std::string cls ("close");
                    transmitSocket.send_to (boost::asio::buffer (cls), rxEndpoint, 0, error);
                    if (error)
                    {
                        logWarning (fmt::format ("transmit failure sending close to receiver III:{}", error.message ()));
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
            std::string cls ("close");
            transmitSocket.send_to (boost::asio::buffer (cls), rxEndpoint, 0, error);
            if (error)
            {
                logWarning (fmt::format ("transmit failure sending close to receiver II:{}", error.message ()));
            }
        }
    }

    setTxStatus( connection_status::terminated);
}

void UdpComms::closeReceiver ()
{
    if (getTxStatus() == connection_status::connected)
    {
        ActionMessage cmd (CMD_PROTOCOL);
        cmd.messageID = CLOSE_RECEIVER;
        transmit (-1, cmd);
    }
    else if (!disconnecting)
    {
        try
        {
            auto serv = AsioServiceManager::getServicePointer ();
            if (serv)
            {
                // try connecting with the receiver socket
                udp::resolver resolver (serv->getBaseService ());
                udp::resolver::query queryLocal (udp::v4 (), localTarget_, std::to_string (PortNumber));

                udp::endpoint rxEndpoint = *resolver.resolve (queryLocal);

                udp::socket transmitter (serv->getBaseService (), udp::endpoint (udp::v4 (), 0));
                std::string cls ("close");
                boost::system::error_code error;
                transmitter.send_to (boost::asio::buffer (cls), rxEndpoint, 0, error);
                if (error)
                {
                    logWarning (
                      fmt::format ("transmit failure on disconnect:{}", error.message ()));
                }
            }
        }
        catch (...)
        {
            // ignore error here since we are already disconnecting
        }
    }
}

std::string UdpComms::getAddress () const { return makePortAddress (localTarget_, PortNumber); }

}  // namespace udp

}  // namespace helics
