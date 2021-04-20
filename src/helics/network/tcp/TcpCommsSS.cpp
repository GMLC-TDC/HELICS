/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "TcpCommsSS.h"

#include "../../common/AsioContextManager.h"
#include "../../core/ActionMessage.hpp"
#include "../NetworkBrokerData.hpp"
#include "../networkDefaults.hpp"
#include "TcpCommsCommon.h"
#include "TcpHelperClasses.h"

#include <map>
#include <memory>
#include <utility>
#include <vector>

namespace helics {
namespace tcp {
    TcpCommsSS::TcpCommsSS() noexcept:
        NetworkCommsInterface(interface_type::tcp, CommsInterface::thread_generation::single)
    {
    }

    /** destructor*/
    TcpCommsSS::~TcpCommsSS() { disconnect(); }

    int TcpCommsSS::getDefaultBrokerPort() const { return DEFAULT_TCPSS_PORT; }

    void TcpCommsSS::addConnection(const std::string& newConn)
    {
        if (propertyLock()) {
            connections.emplace_back(newConn);
            propertyUnLock();
        }
    }

    void TcpCommsSS::addConnections(const std::vector<std::string>& newConnections)
    {
        if (propertyLock()) {
            if (connections.empty()) {
                connections = newConnections;
            } else {
                connections.reserve(connections.size() + newConnections.size());
                connections.insert(connections.end(), newConnections.begin(), newConnections.end());
            }
            propertyUnLock();
        }
    }

    void TcpCommsSS::setFlag(const std::string& flag, bool val)
    {
        if (flag == "reuse_address") {
            if (propertyLock()) {
                reuse_address = val;
                propertyUnLock();
            }
        } else if (flag == "allow_outgoing") {
            if (propertyLock()) {
                outgoingConnectionsAllowed = val;
                propertyUnLock();
            }
        } else {
            NetworkCommsInterface::setFlag(flag, val);
        }
    }

    int TcpCommsSS::processIncomingMessage(ActionMessage&& cmd)
    {
        if (isProtocolCommand(cmd)) {
            switch (cmd.messageID) {
                case CLOSE_RECEIVER:
                    return (-1);
                default:
                    break;
            }
        }
        ActionCallback(std::move(cmd));
        return 0;
    }

    size_t
        TcpCommsSS::dataReceive(TcpConnection* connection, const char* data, size_t bytes_received)
    {
        size_t used_total = 0;
        while (used_total < bytes_received) {
            ActionMessage m;
            auto used =
                m.depacketize(data + used_total, static_cast<int>(bytes_received - used_total));
            if (used == 0) {
                break;
            }
            if (isProtocolCommand(m)) {
                m.setExtraData(connection->getIdentifier());
                txQueue.emplace(control_route, std::move(m));
            } else {
                if (ActionCallback) {
                    ActionCallback(std::move(m));
                }
            }
            used_total += used;
        }

        return used_total;
    }

    void TcpCommsSS::queue_rx_function()
    {
        // this function does nothing since everything is handled in the other thread
    }

    static TcpConnection::pointer generateConnection(std::shared_ptr<AsioContextManager>& ioctx,
                                                     const std::string& address)
    {
        try {
            std::string interface;
            std::string port;
            std::tie(interface, port) = extractInterfaceandPortString(address);
            return TcpConnection::create(ioctx->getBaseContext(), interface, port);
        }
        catch (std::exception&) {
            // TODO(PT):: do something???
        }
        return nullptr;
    }

    void TcpCommsSS::queue_tx_function()
    {
        if (serverMode && (PortNumber < 0)) {
            PortNumber = DEFAULT_TCPSS_PORT;
        }
        if (!serverMode && !outgoingConnectionsAllowed) {
            logError("no server and no outgoing connections-> no way to connect to comms");
            setRxStatus(connection_status::error);
            setTxStatus(connection_status::error);
            return;
        }
        TcpServer::pointer server;
        auto ioctx = AsioContextManager::getContextPointer();
        auto contextLoop = ioctx->startContextLoop();
        auto dataCall =
            [this](const TcpConnection::pointer& connection, const char* data, size_t datasize) {
                return dataReceive(connection.get(), data, datasize);
            };
        CommsInterface* ci = this;
        auto errorCall = [ci](const TcpConnection::pointer& connection,
                              const std::error_code& error) {
            return commErrorHandler(ci, connection.get(), error);
        };

        if (serverMode) {
            server = TcpServer::create(ioctx->getBaseContext(),
                                       localTargetAddress,
                                       static_cast<uint16_t>(PortNumber.load()),
                                       true,
                                       maxMessageSize);
            while (!server->isReady()) {
                logWarning("retrying tcp bind");
                std::this_thread::sleep_for(std::chrono::milliseconds(150));
                auto connected = server->reConnect(connectionTimeout);
                if (!connected) {
                    logError("unable to bind to tcp connection socket");
                    server->close();
                    setRxStatus(connection_status::error);
                    setTxStatus(connection_status::error);
                    return;
                }
            }
            server->setDataCall(dataCall);
            server->setErrorCall(errorCall);
            server->start();
        }

        // generate a local protocol connection string
        ActionMessage cmessage(CMD_PROTOCOL);
        cmessage.messageID = CONNECTION_INFORMATION;
        cmessage.payload = getAddress();
        auto cstring = cmessage.packetize();

        std::vector<std::pair<std::string, TcpConnection::pointer>> made_connections;
        std::map<std::string, route_id> established_routes;
        if (outgoingConnectionsAllowed) {
            for (const auto& conn : connections) {
                auto new_connect = generateConnection(ioctx, conn);

                if (new_connect) {
                    new_connect->setDataCall(dataCall);
                    new_connect->setErrorCall(errorCall);
                    new_connect->send(cstring);
                    new_connect->startReceive();

                    made_connections.emplace_back(conn, std::move(new_connect));
                }
            }
        }
        setRxStatus(connection_status::connected);
        std::vector<char> buffer;

        TcpConnection::pointer brokerConnection;

        std::map<route_id, TcpConnection::pointer> routes;  // for all the other possible routes
        if (!brokerTargetAddress.empty()) {
            hasBroker = true;
        }
        if (hasBroker) {
            if (brokerPort < 0) {
                brokerPort = DEFAULT_TCPSS_PORT;
            }
            if (outgoingConnectionsAllowed) {
                try {
                    brokerConnection = makeConnection(ioctx->getBaseContext(),
                                                      brokerTargetAddress,
                                                      std::to_string(brokerPort),
                                                      maxMessageSize,
                                                      std::chrono::milliseconds(connectionTimeout));
                    if (!brokerConnection) {
                        logError("initial connection to broker timed out");

                        if (server) {
                            server->close();
                        }
                        setTxStatus(connection_status::error);
                        setRxStatus(connection_status::error);
                        return;
                    }

                    brokerConnection->setDataCall(dataCall);
                    brokerConnection->setErrorCall(errorCall);

                    brokerConnection->send(cstring);
                    brokerConnection->startReceive();
                }
                catch (std::exception& e) {
                    logError(e.what());
                    setTxStatus(connection_status::error);
                    setRxStatus(connection_status::error);
                    return;
                }
                established_routes[makePortAddress(brokerTargetAddress, brokerPort)] =
                    parent_route_id;
            }
        }

        setTxStatus(connection_status::connected);

        bool haltLoop{false};
        //  std::vector<ActionMessage> txlist;
        while (!haltLoop) {
            route_id rid;
            ActionMessage cmd;

            std::tie(rid, cmd) = txQueue.pop();
            bool processed = false;
            if (isProtocolCommand(cmd)) {
                if (rid == control_route) {
                    processed = true;
                    switch (cmd.messageID) {
                        case CONNECTION_INFORMATION:
                            if (server) {
                                auto conn = server->findSocket(cmd.getExtraData());
                                if (conn) {
                                    if (!brokerConnection) {  // check if the connection matches the
                                                              // broker
                                        if ((cmd.payload == brokerName) ||
                                            (cmd.payload ==
                                             makePortAddress(brokerTargetAddress, brokerPort))) {
                                            brokerConnection = std::move(conn);
                                        }
                                    }
                                    if (conn) {
                                        made_connections.emplace_back(cmd.payload, std::move(conn));
                                    }
                                } else {
                                    logWarning("(tcpss) unable to locate socket");
                                }
                            }
                            break;
                        case NEW_ROUTE: {
                            bool established = false;

                            for (auto& mc : made_connections) {
                                if ((mc.second) && (cmd.payload == mc.first)) {
                                    routes.emplace(route_id{cmd.getExtraData()},
                                                   std::move(mc.second));
                                    established = true;
                                    established_routes[mc.first] = route_id{cmd.getExtraData()};
                                }
                            }
                            if (!established) {
                                auto efind = established_routes.find(cmd.payload);
                                if (efind != established_routes.end()) {
                                    established = true;
                                    if (efind->second == parent_route_id) {
                                        routes.emplace(route_id{cmd.getExtraData()},
                                                       brokerConnection);
                                    } else {
                                        routes.emplace(route_id{cmd.getExtraData()},
                                                       routes[efind->second]);
                                    }
                                }
                            }

                            if (!established) {
                                if (outgoingConnectionsAllowed) {
                                    auto new_connect = generateConnection(ioctx, cmd.payload);
                                    if (new_connect) {
                                        new_connect->setDataCall(dataCall);
                                        new_connect->setErrorCall(errorCall);
                                        new_connect->send(cstring);
                                        new_connect->startReceive();
                                        routes.emplace(route_id{cmd.getExtraData()},
                                                       std::move(new_connect));
                                        established_routes[cmd.payload] =
                                            route_id{cmd.getExtraData()};
                                    }
                                } else {
                                    logWarning(std::string("unable to make connection ") +
                                               cmd.payload);
                                }
                            }
                        } break;
                        case REMOVE_ROUTE:
                            routes.erase(route_id{cmd.getExtraData()});
                            processed = true;
                            break;
                        case CLOSE_RECEIVER:
                            setRxStatus(connection_status::terminated);
                            break;
                        case DISCONNECT:
                            haltLoop = true;
                            continue;
                        default:
                            logWarning("unrecognized control command");
                            break;
                    }
                }
            }
            if (processed) {
                continue;
            }

            if (rid == parent_route_id) {
                if ((hasBroker) && (brokerConnection)) {
                    try {
                        brokerConnection->send(cmd.packetize());
                    }
                    catch (const std::system_error& se) {
                        if (se.code() != asio::error::connection_aborted) {
                            if (!isDisconnectCommand(cmd)) {
                                logError(std::string("broker send 0 ") +
                                         actionMessageType(cmd.action()) + ':' + se.what());
                            }
                        }
                    }
                } else {
                    logWarning(
                        std::string("(tcpss) no route to broker for message, message dropped :") +
                        actionMessageType(cmd.action()));
                }
            } else {
                //  txlist.push_back(cmd);
                auto rt_find = routes.find(rid);
                if (rt_find != routes.end()) {
                    try {
                        rt_find->second->send(cmd.packetize());
                    }
                    catch (const std::system_error& se) {
                        if (se.code() != asio::error::connection_aborted) {
                            if (!isDisconnectCommand(cmd)) {
                                logError(std::string("rt send ") + std::to_string(rid.baseValue()) +
                                         "::" + se.what());
                            }
                        }
                    }
                } else {
                    if (hasBroker) {
                        try {
                            brokerConnection->send(cmd.packetize());
                        }
                        catch (const std::system_error& se) {
                            if (se.code() != asio::error::connection_aborted) {
                                if (!isDisconnectCommand(cmd)) {
                                    logError(std::string("broker send ") +
                                             std::to_string(rid.baseValue()) + " ::" + se.what());
                                }
                            }
                        }
                    } else {
                        if (!isDisconnectCommand(cmd)) {
                            logWarning(std::string(
                                           "(tcpss) unknown message destination message dropped ") +
                                       prettyPrintString(cmd));
                        }
                    }
                }
            }
        }  // while (!haltLoop)

        for (auto& rt : made_connections) {
            if (rt.second) {
                rt.second->close();
            }
        }
        made_connections.clear();
        for (auto& rt : routes) {
            if (rt.second) {
                rt.second->close();
            }
        }
        if (brokerConnection) {
            brokerConnection->close();
        }
        routes.clear();
        brokerConnection = nullptr;
        setTxStatus(connection_status::terminated);
        if (server) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            server->close();
            server = nullptr;
        }
        if (getRxStatus() == connection_status::connected) {
            setRxStatus(connection_status::terminated);
        }
    }

}  // namespace tcp
}  // namespace helics
