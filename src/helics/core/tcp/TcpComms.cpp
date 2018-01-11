/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "TcpComms.h"
#include "../../common/AsioServiceManager.h"
#include "../ActionMessage.h"
#include "TcpHelperClasses.h"
#include "../NetworkBrokerData.h"
#include <memory>

static const int BEGIN_OPEN_PORT_RANGE = 24228;
static const int BEGIN_OPEN_PORT_RANGE_SUBBROKER = 24357;

static const int DEFAULT_TCP_BROKER_PORT_NUMBER = 24160;

namespace helics
{
using boost::asio::ip::tcp;
TcpComms::TcpComms () {}

TcpComms::TcpComms (const std::string &brokerTarget, const std::string &localTarget)
    : CommsInterface (brokerTarget, localTarget)
{
    if (localTarget_.empty ())
    {
        localTarget_ = "localhost";
    }
}

TcpComms::TcpComms(const NetworkBrokerData &netInfo) :CommsInterface(netInfo),brokerPort(netInfo.brokerPort),PortNumber(netInfo.portNumber)
{
    if (localTarget_.empty())
    {
        localTarget_ = "localhost";
    }
    if (netInfo.portStart > 0)
    {
        openPortStart = netInfo.portStart;
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
    if (isProtocolCommand (M))
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
    if (isProtocolCommand (M))
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

void TcpComms::txPriorityReceive (std::shared_ptr<tcp_connection> connection,
                                  const char *data,
                                  size_t bytes_received,
                                  const boost::system::error_code &error)
{
    if (error)
    {
        return;
    }
    ActionMessage m;
    auto used = m.depacketize (data, bytes_received);
    if (used > 0)
    {
        if (m.action () == CMD_INVALID)
        {
            return;
        }
        if (m.action () == CMD_PRIORITY_ACK)
        {
            return;
        }
        if (isProtocolCommand (m))
        {
            txQueue.emplace (-1, m);
        }
        else
        {
            ActionCallback (std::move (m));
        }
    }
}

size_t
TcpComms::dataReceive (std::shared_ptr<tcp_rx_connection> connection, const char *data, size_t bytes_received)
{
    ActionMessage m;
    size_t used_total = 0;
    while (used_total < bytes_received)
    {
        auto used = m.depacketize (data + used_total, bytes_received - used_total);
        if (used == 0)
        {
            break;
        }
        if (isPriorityCommand (m))
        {
            auto rep = generateReplyToIncomingMessage (m);
            try
            {
                connection->send(rep.packetize());
            }
            catch(const boost::system::system_error &se)
            {
                
           }
        }
        else if (isProtocolCommand (m))
        {
            rxMessageQueue.push (m);
        }
        else
        {
            if (ActionCallback)
            {
                ActionCallback(std::move(m));
            }
        }
        used_total += used;
    }

    return used_total;
}

bool TcpComms::commErrorHandler (std::shared_ptr<tcp_rx_connection> connection,
                                 const boost::system::error_code &error)
{
    if (rx_status == connection_status::connected)
    {
        if ((error != boost::asio::error::eof) && (error != boost::asio::error::operation_aborted))
        {
            if (error != boost::asio::error::connection_reset)
            {
                std::cerr << "error message while connected " << error.message () << "code " << error.value ()
                          << std::endl;
            }
        }
    }
    return false;
}

void TcpComms::queue_rx_function ()
{
    while (PortNumber < 0)
    {
        auto message = rxMessageQueue.pop ();
        if (isProtocolCommand (message))
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
    auto ioserv = AsioServiceManager::getServicePointer ();
    tcp_server server (ioserv->getBaseService (), PortNumber, maxMessageSize_);

    ioserv->runServiceLoop ();
    server.setDataCall ([this](tcp_rx_connection::pointer connection, const char *data, size_t datasize) {
        return dataReceive (connection, data, datasize);
    });
    server.setErrorCall ([this](tcp_rx_connection::pointer connection, const boost::system::error_code &error) {
        return commErrorHandler (connection, error);
    });
    server.start ();
    rx_status = connection_status::connected;
    bool loopRunning = true;
    while (loopRunning)
    {
        auto message = rxMessageQueue.pop ();
        if (isProtocolCommand (message))
        {
            switch (message.index)
            {
            case CLOSE_RECEIVER:
            case DISCONNECT:
                loopRunning = false;
                break;
            }
        }
    }

    disconnecting = true;
    server.close();
    ioserv->haltServiceLoop();
    rx_status = connection_status::terminated;
}

void TcpComms::txReceive (const char *data, size_t bytes_received, const std::string &errorMessage)
{
    if (errorMessage.empty ())
    {
        ActionMessage m (data, bytes_received);
        if (isProtocolCommand (m))
        {
            if (m.index == PORT_DEFINITIONS)
            {
                rxMessageQueue.push (m);
            }
            else if (m.index == DISCONNECT)
            {
                txQueue.emplace (-1, m);
            }
        }
    }
}

void TcpComms::queue_tx_function ()
{
    std::vector<char> buffer;
    auto ioserv = AsioServiceManager::getServicePointer ();
    ioserv->runServiceLoop ();
    tcp_connection::pointer brokerConnection;

    std::map<int, tcp_connection::pointer> routes;  // for all the other possible routes
    if (!brokerTarget_.empty ())
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
            brokerConnection = tcp_connection::create (ioserv->getBaseService (), brokerTarget_,
                                                       std::to_string (brokerPort), maxMessageSize_);
            int cumsleep = 0;
            while (!brokerConnection->isConnected ())
            {
                std::this_thread::sleep_for (std::chrono::milliseconds (100));
                cumsleep += 100;
                if (cumsleep >= connectionTimeout)
                {
                    ioserv->haltServiceLoop ();
                    std::cerr << "initial connection to broker timed out\n" << std::endl;
                    tx_status = connection_status::terminated;
                    return;
                }
            }

            if (PortNumber <= 0)
            {
                ActionMessage m (CMD_PROTOCOL_PRIORITY);
                m.index = REQUEST_PORTS;
                try
                {
                    brokerConnection->send (m.packetize ());
                }
                catch (const boost::system::system_error &error)
                {
                    std::cerr << "error in initial send to broker " << error.what () << '\n';
                    ioserv->haltServiceLoop ();
                    tx_status = connection_status::terminated;
                    return;
                }
                std::vector<char> rx (512);
                tcp::endpoint brk;
                brokerConnection->async_receive (rx.data (), 128,
                                                 [this, &rx](const boost::system::error_code &error,
                                                             size_t bytes) {
                                                     if (error != boost::asio::error::operation_aborted)
                                                     {
                                                         if (!error)
                                                         {
                                                             txReceive (rx.data (), bytes, std::string ());
                                                         }
                                                         else
                                                         {
                                                             txReceive (rx.data (), bytes, error.message ());
                                                         }
                                                     }
                                                 });
                cumsleep = 0;
                while (PortNumber < 0)
                {
                    std::this_thread::sleep_for (std::chrono::milliseconds (100));
                    auto mess = txQueue.try_pop ();
                    if (mess)
                    {
                        if (isProtocolCommand (mess->second))
                        {
                            if (mess->second.index == PORT_DEFINITIONS)
                            {
                                rxMessageQueue.push (mess->second);
                            }
                            else if (mess->second.index == DISCONNECT)
                            {
                                brokerConnection->cancel ();
                                ioserv->haltServiceLoop ();
                                tx_status = connection_status::terminated;
                                return;
                            }
                        }
                    }
                    cumsleep += 100;
                    if (cumsleep >= connectionTimeout)
                    {
                        ioserv->haltServiceLoop ();
                        brokerConnection->cancel ();
                        std::cerr << "port number query to broker timed out\n" << std::endl;
                        tx_status = connection_status::terminated;
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
            PortNumber = DEFAULT_TCP_BROKER_PORT_NUMBER;
            ActionMessage m (CMD_PROTOCOL);
            m.index = PORT_DEFINITIONS;
            m.source_handle = PortNumber;
            rxMessageQueue.push (m);
        }
    }
    tx_status = connection_status::connected;

    //  std::vector<ActionMessage> txlist;
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
                switch (cmd.index)
                {
                case NEW_ROUTE:
                {
                    auto newroute = cmd.payload;

                    try
                    {
                        std::string interface;
                        std::string port;
                        std::tie (interface, port) = extractInterfaceandPortString (newroute);
                        auto new_connect = tcp_connection::create (ioserv->getBaseService (), interface, port);

                        routes.emplace (cmd.dest_id, std::move (new_connect));
                    }
                    catch (std::exception &e)
                    {
                        // TODO:: do something???
                    }
                    processed = true;
                }
                break;
                case CLOSE_RECEIVER:
                    rxMessageQueue.push (cmd);
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
                try
                {
                    brokerConnection->send(cmd.packetize());
                    if (isPriorityCommand(cmd))
                    {
                        brokerConnection->async_receive([this](std::shared_ptr<tcp_connection> connection,
                            const char *data, size_t bytes_received,
                            const boost::system::error_code &error) {
                            txPriorityReceive(connection, data, bytes_received, error);
                        });
                    }
                }
                catch (const boost::system::system_error &se)
                {
                    if (se.code() != boost::asio::error::connection_aborted)
                    {
                        if (!isDisconnectCommand(cmd))
                        {
                            std::cerr << "broker send0 " << se.what() << '\n';
                        }
                       
                   }
                }
                
                // if (error)
                {
                    //     std::cerr << "transmit failure to broker " << error.message() << '\n';
                }
            }
        }
        else if (route_id == -1)
        {  // send to rx thread loop
            rxMessageQueue.push (cmd);
        }
        else
        {
            //  txlist.push_back(cmd);
            auto rt_find = routes.find (route_id);
            if (rt_find != routes.end ())
            {
                try
                {
                    rt_find->second->send(cmd.packetize());
                    if (isPriorityCommand(cmd))
                    {
                        rt_find->second->async_receive([this](std::shared_ptr<tcp_connection> connection,
                            const char *data, size_t bytes_received,
                            const boost::system::error_code &error) {
                            txPriorityReceive(connection, data, bytes_received, error);
                        });
                    }
                }
                catch (const boost::system::system_error &se)
                {
                    if (se.code() != boost::asio::error::connection_aborted)
                    {
                        if (!isDisconnectCommand(cmd))
                        {
                            std::cerr << "rt send " << route_id << "::" << se.what() << '\n';
                        }
                    }
                }
            }
            else
            {
                if (hasBroker)
                {
                    try
                    {
                        brokerConnection->send(cmd.packetize());
                        if (isPriorityCommand(cmd))
                        {
                            brokerConnection->async_receive([this](std::shared_ptr<tcp_connection> connection,
                                const char *data, size_t bytes_received,
                                const boost::system::error_code &error) {
                                txPriorityReceive(connection, data, bytes_received, error);
                            });
                        }
                    }
                    catch (const  boost::system::system_error &se)
                    {
                        if (se.code() != boost::asio::error::connection_aborted)
                        {
                            if (!isDisconnectCommand(cmd))
                            {
                                std::cerr << "broker send" << route_id << " ::" << se.what() << '\n';
                            }
                        }
                    }
                }
                else
                {
                    assert (false);
                }
            }
        }
    }
CLOSE_TX_LOOP:
    for (auto &rt : routes)
    {
        rt.second->close ();
    }
    routes.clear ();
    if (rx_status == connection_status::connected)
    {
        closeReceiver ();
    }
    ioserv->haltServiceLoop ();
    tx_status = connection_status::terminated;
}

void TcpComms::closeReceiver ()
{
    ActionMessage cmd (CMD_PROTOCOL);
    cmd.index = CLOSE_RECEIVER;
    rxMessageQueue.push (cmd);
}

std::string TcpComms::getAddress () const { return makePortAddress (localTarget_, PortNumber); }
}  // namespace helics