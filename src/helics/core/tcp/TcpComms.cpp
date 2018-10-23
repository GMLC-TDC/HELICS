/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "../../common/AsioServiceManager.h"
#include "../ActionMessage.hpp"
#include "../NetworkBrokerData.hpp"
#include "TcpComms.h"
#include "TcpCommsCommon.h"
#include "TcpHelperClasses.h"
#include <memory>

static constexpr int DEFAULT_TCP_BROKER_PORT_NUMBER = 24160;

namespace helics
{
namespace tcp
{
using boost::asio::ip::tcp;

TcpComms::TcpComms () noexcept : NetworkCommsInterface (interface_type::tcp) {}

int TcpComms::getDefaultBrokerPort () const { return DEFAULT_TCP_BROKER_PORT_NUMBER; }

/** load network information into the comms object*/
void TcpComms::loadNetworkInfo (const NetworkBrokerData &netInfo)
{
    NetworkCommsInterface::loadNetworkInfo (netInfo);
    if (!propertyLock ())
    {
        return;
    }
    reuse_address = netInfo.reuse_address;
    propertyUnLock ();
}

/** destructor*/
TcpComms::~TcpComms () { disconnect (); }

int TcpComms::processIncomingMessage (ActionMessage &&M)
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

size_t TcpComms::dataReceive (std::shared_ptr<TcpConnection> connection, const char *data, size_t bytes_received)
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
            // if the reply is not ignored respond with it otherwise
            // forward the original message on to the receiver to handle
            auto rep = generateReplyToIncomingMessage (m);
            if (rep.action () != CMD_IGNORE)
            {
                try
                {
                    connection->send (rep.packetize ());
                }
                catch (const boost::system::system_error &se)
                {
                }
            }
            else
            {
                rxMessageQueue.push (std::move (m));
            }
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

bool TcpComms::commErrorHandler (std::shared_ptr<TcpConnection> /*connection*/,
                                 const boost::system::error_code &error)
{
    if (getRxStatus () == connection_status::connected)
    {
        if ((error != boost::asio::error::eof) && (error != boost::asio::error::operation_aborted))
        {
            if (error != boost::asio::error::connection_reset)
            {
                logError (std::string ("error message while connected ") + error.message () + " code " +
                          std::to_string (error.value ()));
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
            switch (message.messageID)
            {
            case PORT_DEFINITIONS:
            {
                loadPortDefinitions (message);
            }

            break;
            case CLOSE_RECEIVER:
            case DISCONNECT:
                disconnecting = true;
                setRxStatus (connection_status::terminated);
                return;
            }
        }
    }
    if (PortNumber < 0)
    {
        setRxStatus (connection_status::error);
        return;
    }
    auto ioserv = AsioServiceManager::getServicePointer ();
    auto server = helics::tcp::TcpServer::create (ioserv->getBaseService (), localTarget_, PortNumber,
                                                  reuse_address, maxMessageSize_);
    while (!server->isReady ())
    {
        if ((autoPortNumber) && (hasBroker))
        {  // If we failed and we are on an automatically assigned port number,  just try a different port
            server->close ();
            ++PortNumber;
            server = helics::tcp::TcpServer::create (ioserv->getBaseService (), localTarget_, PortNumber,
                                                     reuse_address, maxMessageSize_);
        }
        else
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
    bool loopRunning = true;
    while (loopRunning)
    {
        auto message = rxMessageQueue.pop ();
        if (isProtocolCommand (message))
        {
            switch (message.messageID)
            {
            case CLOSE_RECEIVER:
            case DISCONNECT:
                loopRunning = false;
                break;
            }
        }
    }

    disconnecting = true;
    server->close ();
    setRxStatus (connection_status::terminated);
}

void TcpComms::txReceive (const char *data, size_t bytes_received, const std::string &errorMessage)
{
    if (errorMessage.empty ())
    {
        ActionMessage m (data, bytes_received);
        if (isProtocolCommand (m))
        {
            if (m.messageID == PORT_DEFINITIONS)
            {
                rxMessageQueue.push (m);
            }
            else if (m.messageID == DISCONNECT)
            {
                txQueue.emplace (control_route, m);
            }
        }
    }
    else
    {
        logError(errorMessage);
    }
}

bool TcpComms::establishBrokerConnection (std::shared_ptr<AsioServiceManager> &ioserv,
                                          std::shared_ptr<TcpConnection> &brokerConnection)
{
    auto terminate = [&,this](connection_status status) -> bool {
        if (brokerConnection)
        {
            brokerConnection->close ();
            brokerConnection = nullptr;
        }
        setTxStatus (status);
        return false;
    };

    if (brokerPort < 0)
    {
        brokerPort = DEFAULT_TCP_BROKER_PORT_NUMBER;
    }
    try
    {
        brokerConnection = makeConnection (ioserv->getBaseService (), brokerTarget_, std::to_string (brokerPort),
                                           maxMessageSize_, std::chrono::milliseconds (connectionTimeout));
        if (!brokerConnection)
        {
            logError ("initial connection to broker timed out");
            return terminate (connection_status::error);
        }
        if (PortNumber <= 0)
        {
            ActionMessage m (CMD_PROTOCOL_PRIORITY);
            m.messageID = REQUEST_PORTS;
            try
            {
                brokerConnection->send (m.packetize ());
            }
            catch (const boost::system::system_error &error)
            {
                logError (std::string ("error in initial send to broker ") + error.what ());
                return terminate (connection_status::error);
            }
            std::vector<char> rx (512);
            tcp::endpoint brk;
            brokerConnection->async_receive (rx.data (), 128,
                                             [this, &rx](const boost::system::error_code &error, size_t bytes) {

                                                 if (!error)
                                                 {
                                                     txReceive (rx.data (), bytes, std::string ());
                                                 }
                                                 else
                                                 {
                                                     if (error != boost::asio::error::operation_aborted)
                                                     {
                                                         txReceive (rx.data (), bytes, error.message ());
                                                     }
                                                     else
                                                     {

                                                     }
                                                 }
                                             });
            std::chrono::milliseconds cumsleep{ 0 };
            while (PortNumber < 0)
            {
                auto mess = txQueue.pop(std::chrono::milliseconds(100));
                if (mess)
                {
                    if (isProtocolCommand (mess->second))
                    {
                        if (mess->second.messageID == PORT_DEFINITIONS)
                        {
                            rxMessageQueue.push (mess->second);
                            break;
                        }
                        else if (mess->second.messageID == DISCONNECT)
                        {
                            return terminate (connection_status::terminated);
                        }
                    }
                }
                cumsleep += std::chrono::milliseconds(100);
                if (cumsleep >= connectionTimeout)
                {
                    brokerConnection->cancel();
                    logError ("port number query to broker timed out");
                    return terminate (connection_status::error);
                }
            }
        }
    }
    catch (std::exception &e)
    {
        logError (std::string ("error connecting with Broker") + e.what ());
        return terminate (connection_status::error);
    }
    return true;
}

void TcpComms::queue_tx_function ()
{
    std::vector<char> buffer;
    auto ioserv = AsioServiceManager::getServicePointer ();
    auto serviceLoop = ioserv->runServiceLoop ();
    TcpConnection::pointer brokerConnection;

    std::map<route_id_t, TcpConnection::pointer> routes;  // for all the other possible routes
    if (!brokerTarget_.empty ())
    {
        hasBroker = true;
    }
    if (hasBroker)
    {
        if (!establishBrokerConnection (ioserv, brokerConnection))
        {
            ActionMessage m (CMD_PROTOCOL);
            m.messageID = CLOSE_RECEIVER;
            rxMessageQueue.push (m);
            return;
        }
    }
    else
    {
        if (PortNumber < 0)
        {
            PortNumber = DEFAULT_TCP_BROKER_PORT_NUMBER;
            ActionMessage m (CMD_PROTOCOL);
            m.messageID = PORT_DEFINITIONS;
            m.setExtraData (PortNumber);
            rxMessageQueue.push (m);
        }
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
                switch (cmd.messageID)
                {
                case NEW_ROUTE:
                {
                    auto &newroute = cmd.payload;

                    try
                    {
                        std::string interface;
                        std::string port;
                        std::tie (interface, port) = extractInterfaceandPortString (newroute);
                        auto new_connect = TcpConnection::create (ioserv->getBaseService (), interface, port);

                        routes.emplace (route_id_t (cmd.getExtraData ()), std::move (new_connect));
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

                // if (error)
                {
                    //     std::cerr << "transmit failure to broker " << error.message() << '\n';
                }
            }
        }
        else if (route_id == control_route)
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
                                logError (std::string ("broker send") + std::to_string (route_id.baseValue ()) +
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
    routes.clear ();
    if (getRxStatus () == connection_status::connected)
    {
        closeReceiver ();
    }
    setTxStatus (connection_status::terminated);
}

void TcpComms::closeReceiver ()
{
    ActionMessage cmd (CMD_PROTOCOL);
    cmd.messageID = CLOSE_RECEIVER;
    rxMessageQueue.push (cmd);
}

}  // namespace tcp
}  // namespace helics
