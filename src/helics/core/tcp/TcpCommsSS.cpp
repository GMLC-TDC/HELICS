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

static constexpr int DEFAULT_TCPSS_PORT = 33133;

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

int TcpCommsSS::processIncomingMessage (ActionMessage &&M)
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
        if (isProtocolCommand (m))
        {
            m.setExtraData (connection->getIdentifier ());
            txQueue.emplace (control_route, std::move (m));
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
                logError ("error message while connected " + error.message () + "code " +
                          std::to_string (error.value ()));
            }
        }
    }
    return false;
}

void TcpCommsSS::queue_rx_function ()
{
    // this function does nothing since everything is handled in the other thread
}

static TcpConnection::pointer
generateConnection (std::shared_ptr<AsioServiceManager> &ioserv, const std::string &address)
{
    try
    {
        std::string interface;
        std::string port;
        std::tie (interface, port) = extractInterfaceandPortString (address);
        return TcpConnection::create (ioserv->getBaseService (), interface, port);
    }
    catch (std::exception &e)
    {
        // TODO:: do something???
    }
    return nullptr;
}

void TcpCommsSS::queue_tx_function ()
{
    if (serverMode && (PortNumber < 0))
    {
        PortNumber = DEFAULT_TCPSS_PORT;
    }
    TcpServer::pointer server;
    auto ioserv = AsioServiceManager::getServicePointer ();
    auto serviceLoop = ioserv->runServiceLoop ();
    auto dataCall = [this](TcpConnection::pointer connection, const char *data, size_t datasize) {
        return dataReceive (connection, data, datasize);
    };

    auto errorCall = [this](TcpConnection::pointer connection, const boost::system::error_code &error) {
        return commErrorHandler (connection, error);
    };

    if (serverMode)
    {
        server = TcpServer::create (ioserv->getBaseService (), localTarget_, PortNumber, true, maxMessageSize_);
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
        server->setDataCall (dataCall);
        server->setErrorCall (errorCall);
        server->start ();
    }

    // generate a local protocol connection string
    ActionMessage cmessage (CMD_PROTOCOL);
    cmessage.messageID = CONNECTION_INFORMATION;
    cmessage.payload = getAddress ();
    auto cstring = cmessage.packetize ();

    std::vector<std::pair<std::string, TcpConnection::pointer>> made_connections;
    std::map<std::string, route_id_t> established_routes;

    for (const auto &conn : connections)
    {
        auto new_connect = generateConnection (ioserv, conn);

        if (new_connect)
        {
            new_connect->setDataCall (dataCall);
            new_connect->setErrorCall (errorCall);
            new_connect->send (cstring);
            new_connect->startReceive ();

            made_connections.emplace_back (conn, std::move (new_connect));
        }
    }

    setRxStatus (connection_status::connected);
    std::vector<char> buffer;

    TcpConnection::pointer brokerConnection;

    std::map<route_id_t, TcpConnection::pointer> routes;  // for all the other possible routes
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
            brokerConnection->setDataCall (dataCall);
            brokerConnection->setErrorCall (errorCall);
            
            brokerConnection->send (cstring);
            brokerConnection->startReceive ();
        }
        catch (std::exception &e)
        {
            logError (e.what ());
        }
        established_routes[makePortAddress (brokerTarget_, brokerPort)] = parent_route_id;
    }

    setTxStatus (connection_status::connected);

    //  std::vector<ActionMessage> txlist;
    while (true)
    {
        route_id_t route_id;
        ActionMessage cmd;

        std::tie (route_id, cmd) = txQueue.pop ();
        bool processed = false;
        if (isProtocolCommand (cmd))
        {
            if (route_id == control_route)
            {
                processed = true;
                switch (cmd.messageID)
                {
                case CONNECTION_INFORMATION:
                    if (server)
                    {
                        auto conn = server->findSocket (cmd.getExtraData ());
                        if (conn)
                        {
                            made_connections.emplace_back (cmd.payload, std::move (conn));
                        }
                    }
                    break;
                case NEW_ROUTE:
                {
                    bool established = false;

                    for (auto &mc : made_connections)
                    {
                        if ((mc.second) && (cmd.payload == mc.first))
                        {
                            routes.emplace (route_id_t(cmd.getExtraData ()), std::move (mc.second));
                            established = true;
                            established_routes[mc.first] = route_id_t (cmd.getExtraData ());
                        }
                    }
                    if (!established)
                    {
                        auto efind = established_routes.find (cmd.payload);
                        if (efind != established_routes.end ())
                        {
                            established = true;
                            if (efind->second == parent_route_id)
                            {
                                routes.emplace (route_id_t(cmd.getExtraData ()), brokerConnection);
                            }
                            else
                            {
                                routes.emplace (route_id_t(cmd.getExtraData ()), routes[efind->second]);
                            }
                        }
                    }

                    if (!established)
                    {
                        auto new_connect = generateConnection (ioserv, cmd.payload);
                        if (new_connect)
                        {
                            new_connect->setDataCall (dataCall);
                            new_connect->setErrorCall (errorCall);
                            new_connect->send (cstring);
                            new_connect->startReceive ();
                            routes.emplace (route_id_t(cmd.getExtraData ()), std::move (new_connect));
                            established_routes[cmd.payload] = route_id_t (cmd.getExtraData ());
                        }
                    }
                }
                break;
                case CLOSE_RECEIVER:
                    setRxStatus (connection_status::terminated);
                    break;
                case DISCONNECT:
                    goto CLOSE_TX_LOOP;  // break out of loop
                default:
                    logWarning ("unrecognized control command");
                    break;
                }
            }
        }
        if (processed)
        {
            continue;
        }

        if (route_id == parent_route_id)
        {
            if (hasBroker)
            {
                try
                {
                    brokerConnection->send (cmd.packetize ());
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
            }
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
                }
                catch (const boost::system::system_error &se)
                {
                    if (se.code () != boost::asio::error::connection_aborted)
                    {
                        if (!isDisconnectCommand (cmd))
                        {
                            logError (std::string ("rt send ") + std::to_string (route_id.baseValue ()) +
                                      "::" + se.what ());
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
                    }
                    catch (const boost::system::system_error &se)
                    {
                        if (se.code () != boost::asio::error::connection_aborted)
                        {
                            if (!isDisconnectCommand (cmd))
                            {
                                logError (std::string ("broker send ") + std::to_string (route_id.baseValue ()) +
                                          " ::" + se.what ());
                            }
                        }
                    }
                }
                else
                {
                    logWarning ("unknown message destination message dropped");
                }
            }
        }
    }
CLOSE_TX_LOOP:
    for (auto &rt : routes)
    {
        rt.second->close ();
    }
	if (brokerConnection)
	{
        brokerConnection->close ();
	}
    routes.clear ();
    brokerConnection = nullptr;
    if (getRxStatus () == connection_status::connected)
    {
        setRxStatus (connection_status::terminated);
    }
    setTxStatus (connection_status::terminated);
}

void TcpCommsSS::closeReceiver ()
{
    ActionMessage cmd (CMD_PROTOCOL);
    cmd.messageID = CLOSE_RECEIVER;
    transmit (control_route, cmd);
}

}  // namespace tcp
}  // namespace helics
