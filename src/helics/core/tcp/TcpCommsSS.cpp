/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "TcpCommsSS.h"
#include "../../common/AsioServiceManager.h"
#include "../ActionMessage.hpp"
#include "../NetworkBrokerData.hpp"
#include "TcpHelperClasses.h"
#include <memory>

static constexpr int DEFAULT_TCPSS_PORT = 24151;

namespace helics
{
namespace tcp
{
using boost::asio::ip::tcp;
TcpCommsSS::TcpCommsSS () noexcept : NetworkCommsInterface (interface_type::tcp) {}

/** destructor*/
TcpCommsSS::~TcpCommsSS () { disconnect (); }


int TcpCommsSS::getDefaultBrokerPort () const { return DEFAULT_TCPSS_PORT; }


void TcpCommsSS::addConnection (const std::string &newConn)
{
    if (propertyLock ())
    {
        connections.emplace_back (newConn);
        propertyUnLock ();
    }
}

void TcpCommsSS::addConnections (const std::vector<std::string> &newConnections)
{
    if (propertyLock ())
    {
        if (connections.empty ())
        {
            connections = newConnections;
        }
        else
        {
            connections.reserve (connections.size () + newConnections.size ());
            connections.insert (connections.end (), newConnections.begin (), newConnections.end ());
        }
        propertyUnLock ();
    }
}

int TcpCommsSS::processIncomingMessage (ActionMessage &M)
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


void TcpCommsSS::txPriorityReceive (std::shared_ptr<TcpConnection> /*connection*/,
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

size_t TcpCommsSS::dataReceive (std::shared_ptr<TcpConnection> connection, const char *data, size_t bytes_received)
{
    size_t used_total = 0;
    while (used_total < bytes_received)
    {
        ActionMessage m;
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
                connection->send (rep.packetize ());
            }
            catch (const boost::system::system_error &se)
            {
            }
        }
        else if (isProtocolCommand (m))
        {
            // rxMessageQueue.push (m);
        }
        else
        {
            if (ActionCallback)
            {
                ActionCallback (std::move (m));
            }
        }
        used_total += used;
    }

    return used_total;
}

bool TcpCommsSS::commErrorHandler (std::shared_ptr<TcpConnection> /*connection*/,
                                   const boost::system::error_code &error)
{
    if (getRxStatus () == connection_status::connected)
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

void TcpCommsSS::queue_rx_function ()
{
    // this function does nothing since everything is handled in the other thread
}

void TcpCommsSS::txReceive (const char *data, size_t bytes_received, const std::string &errorMessage)
{
    if (errorMessage.empty ())
    {
        ActionMessage m (data, bytes_received);
        if (isProtocolCommand (m))
        {
            if (m.messageID == DISCONNECT)
            {
                txQueue.emplace (-1, m);
            }
        }
    }
}

void TcpCommsSS::queue_tx_function ()
{
    if (serverMode && (PortNumber < 0))
    {
        PortNumber = DEFAULT_TCPSS_PORT;
    }
    if (serverMode)
    {
        auto ioserv = AsioServiceManager::getServicePointer ();
        auto server = helics::tcp::TcpServer::create (ioserv->getBaseService (), localTarget_, PortNumber, true,
                                                      maxMessageSize_);
        while (!server->isReady ())
        {
            logWarning ("retrying tcp bind");
            std::this_thread::sleep_for (std::chrono::milliseconds (150));
            auto connected = server->reConnect (connectionTimeout);
            if (!connected)
            {
                logError ("unable to bind to tcp connection socket");
                server->close ();
                setRxStatus (connection_status::error);
                return;
            }
        }
        auto serviceLoop = ioserv->runServiceLoop ();
        server->setDataCall ([this](TcpConnection::pointer connection, const char *data, size_t datasize) {
            return dataReceive (connection, data, datasize);
        });
        server->setErrorCall ([this](TcpConnection::pointer connection, const boost::system::error_code &error) {
            return commErrorHandler (connection, error);
        });
        server->start ();
        setRxStatus (connection_status::connected);
        disconnecting = true;
        server->close ();
        setRxStatus (connection_status::terminated);
    }
    std::vector<char> buffer;
    auto ioserv = AsioServiceManager::getServicePointer ();
    auto serviceLoop = ioserv->runServiceLoop ();
    TcpConnection::pointer brokerConnection;

    std::map<int, TcpConnection::pointer> routes;  // for all the other possible routes
    if (!brokerTarget_.empty ())
    {
        hasBroker = true;
    }
    if (hasBroker)
    {
        if (brokerPort < 0)
        {
            brokerPort = DEFAULT_TCPSS_PORT;
        }
        try
        {
            brokerConnection = TcpConnection::create (ioserv->getBaseService (), brokerTarget_,
                                                      std::to_string (brokerPort), maxMessageSize_);
            int cumsleep = 0;
            while (!brokerConnection->isConnected ())
            {
                std::this_thread::sleep_for (std::chrono::milliseconds (100));
                cumsleep += 100;
                if (cumsleep >= connectionTimeout)
                {
                    logError ("initial connection to broker timed out");
                    setTxStatus (connection_status::terminated);
                    return;
                }
            }
        }
        catch (std::exception &e)
        {
            logError (e.what ());
        }
    }

    setTxStatus (connection_status::connected);

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
                        auto new_connect = TcpConnection::create (ioserv->getBaseService (), interface, port);

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
                    // rxMessageQueue.push (cmd);
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
                    brokerConnection->send (cmd.packetize ());
                    if (isPriorityCommand (cmd))
                    {
                        brokerConnection->async_receive ([this](std::shared_ptr<TcpConnection> connection,
                                                                const char *data, size_t bytes_received,
                                                                const boost::system::error_code &error) {
                            txPriorityReceive (connection, data, bytes_received, error);
                        });
                    }
                }
                catch (const boost::system::system_error &se)
                {
                    if (se.code () != boost::asio::error::connection_aborted)
                    {
                        if (!isDisconnectCommand (cmd))
                        {
                            logError (std::string ("broker send 0 ") + actionMessageType (cmd.action ()) + ':' +
                                      se.what ());
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
           //  rxMessageQueue.push (cmd);
        }
        else
        {
            //  txlist.push_back(cmd);
            auto rt_find = routes.find (route_id);
            if (rt_find != routes.end ())
            {
                try
                {
                    rt_find->second->send (cmd.packetize ());
                    if (isPriorityCommand (cmd))
                    {
                        rt_find->second->async_receive ([this](std::shared_ptr<TcpConnection> connection,
                                                               const char *data, size_t bytes_received,
                                                               const boost::system::error_code &error) {
                            txPriorityReceive (connection, data, bytes_received, error);
                        });
                    }
                }
                catch (const boost::system::system_error &se)
                {
                    if (se.code () != boost::asio::error::connection_aborted)
                    {
                        if (!isDisconnectCommand (cmd))
                        {
                            logError (std::string ("rt send ") + std::to_string (route_id) + "::" + se.what ());
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
                        brokerConnection->send (cmd.packetize ());
                        if (isPriorityCommand (cmd))
                        {
                            brokerConnection->async_receive ([this](std::shared_ptr<TcpConnection> connection,
                                                                    const char *data, size_t bytes_received,
                                                                    const boost::system::error_code &error) {
                                txPriorityReceive (connection, data, bytes_received, error);
                            });
                        }
                    }
                    catch (const boost::system::system_error &se)
                    {
                        if (se.code () != boost::asio::error::connection_aborted)
                        {
                            if (!isDisconnectCommand (cmd))
                            {
                                logError (std::string ("broker send ") + std::to_string (route_id) +
                                          " ::" + se.what ());
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
    if (getRxStatus () == connection_status::connected)
    {
        closeReceiver ();
    }
    setTxStatus (connection_status::terminated);
}

void TcpCommsSS::closeReceiver ()
{
    ActionMessage cmd (CMD_PROTOCOL);
    cmd.messageID = CLOSE_RECEIVER;
    transmit (-1, cmd);
}

}  // namespace tcp
}  // namespace helics
