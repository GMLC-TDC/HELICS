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

static const int DEFAULT_TCPSS_PORT = 24151;

namespace helics
{
namespace tcp
{
using boost::asio::ip::tcp;
TcpCommsSS::TcpCommsSS () noexcept {}

TcpCommsSS::TcpCommsSS (const std::string &brokerTarget,
                    const std::string &localTarget,
                    interface_networks targetNetwork)
    : CommsInterface (brokerTarget, localTarget, targetNetwork)
{
    if (localTarget_.empty ())
    {
        if ((brokerTarget_ == "tcp://127.0.0.1") || (brokerTarget_ == "tcp://localhost") ||
            (brokerTarget_ == "localhost"))
        {
            localTarget_ = "localhost";
        }
        else if (brokerTarget_.empty ())
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
            localTarget_ = generateMatchingInterfaceAddress (brokerTarget_, interfaceNetwork);
        }
    }
}

TcpCommsSS::TcpCommsSS (const NetworkBrokerData &netInfo)
    : CommsInterface (netInfo), brokerPort (netInfo.brokerPort), localPort (netInfo.portNumber)
{
    if (localTarget_.empty ())
    {
        if ((brokerTarget_ == "tcp://127.0.0.1") || (brokerTarget_ == "tcp://localhost") ||
            (brokerTarget_ == "localhost"))
        {
            localTarget_ = "localhost";
        }
        else if (brokerTarget_.empty ())
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
            localTarget_ = generateMatchingInterfaceAddress (brokerTarget_, interfaceNetwork);
        }
    }
    
}

/** destructor*/
TcpCommsSS::~TcpCommsSS () { disconnect (); }

void TcpCommsSS::setBrokerPort (int brokerPortNumber)
{
    if (getRxStatus () == connection_status::startup)
    {
        brokerPort = brokerPortNumber;
    }
}

void TcpCommsSS::setPortNumber (int localPortNumber)
{
    if (getRxStatus () == connection_status::startup)
    {
        localPort = localPortNumber;
    }
}


void TcpCommsSS::setServerMode(bool mode) 
{ 
    if (getRxStatus() == connection_status::startup)
    {
        serverMode = mode;
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

ActionMessage TcpCommsSS::generateReplyToIncomingMessage (ActionMessage &M)
{
    if (isProtocolCommand (M))
    {
        switch (M.messageID)
        {
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

size_t TcpCommsSS::dataReceive (std::shared_ptr<TcpRxConnection> connection, const char *data, size_t bytes_received)
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
            //rxMessageQueue.push (m);
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

bool TcpCommsSS::commErrorHandler (std::shared_ptr<TcpRxConnection> /*connection*/,
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
   //this function does nothing since everything is handled in the other thread
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
    if (serverMode&&localPort < 0)
    {
        localPort = DEFAULT_TCPSS_PORT;
    }
    if (serverMode)
    {
        auto ioserv = AsioServiceManager::getServicePointer();
        auto server = helics::tcp::TcpServer::create(ioserv->getBaseService(), localTarget_, localPort,
            true, maxMessageSize_);
        while (!server->isReady())
        {

            std::cerr << "retrying tcp bind\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
            auto connected = server->reConnect(connectionTimeout);
            if (!connected)
            {
                std::cerr << "unable to bind to tcp connection socket\n";
                server->close();
                setRxStatus(connection_status::error);
                return;
            }
        }
        auto serviceLoop = ioserv->runServiceLoop();
        server->setDataCall([this](TcpRxConnection::pointer connection, const char *data, size_t datasize) {
            return dataReceive(connection, data, datasize);
        });
        server->setErrorCall([this](TcpRxConnection::pointer connection, const boost::system::error_code &error) {
            return commErrorHandler(connection, error);
        });
        server->start();
        setRxStatus(connection_status::connected);
        disconnecting = true;
        server->close();
        setRxStatus(connection_status::terminated);

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
                    std::cerr << "initial connection to broker timed out\n" << std::endl;
                    setTxStatus (connection_status::terminated);
                    return;
                }
            }

        }
        catch (std::exception &e)
        {
            std::cerr << e.what () << std::endl;
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
                    //rxMessageQueue.push (cmd);
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
                            std::cerr << "broker send 0 " << actionMessageType (cmd.action ()) << ':' << se.what ()
                                      << '\n';
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
                            std::cerr << "rt send " << route_id << "::" << se.what () << '\n';
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
                                std::cerr << "broker send" << route_id << " ::" << se.what () << '\n';
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
    transmit(-1,cmd);
}

std::string TcpCommsSS::getAddress () const { return makePortAddress (localTarget_, localPort); }

}  // namespace tcp
}  // namespace helics
