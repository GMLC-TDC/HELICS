/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "TcpCommsSS.h"

#include "../../core/ActionMessage.hpp"
#include "../NetworkBrokerData.hpp"
#include "../networkDefaults.hpp"
#include "TcpCommsCommon.h"
#include "gmlc/networking/AsioContextManager.h"
#include "gmlc/networking/TcpHelperClasses.h"
#include "gmlc/networking/TcpOperations.h"

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace helics::tcp {
using gmlc::networking::TcpConnection;

TcpCommsSS::TcpCommsSS() noexcept:
    NetworkCommsInterface(gmlc::networking::InterfaceTypes::TCP,
                          CommsInterface::thread_generation::single)
{
}

/** destructor*/
TcpCommsSS::~TcpCommsSS()
{
    disconnect();
}

/** load network information into the comms object*/
void TcpCommsSS::loadNetworkInfo(const NetworkBrokerData& netInfo)
{
    NetworkCommsInterface::loadNetworkInfo(netInfo);
    if (!propertyLock()) {
        return;
    }
    reuse_address = netInfo.reuse_address;
    encryption_config = netInfo.encryptionConfig;
    propertyUnLock();
}

int TcpCommsSS::getDefaultBrokerPort() const
{
    return getDefaultPort(HELICS_CORE_TYPE_TCP_SS);
}

void TcpCommsSS::addConnection(std::string_view newConn)
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

void TcpCommsSS::setFlag(std::string_view flag, bool val)
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
    } else if (flag == "encrypted") {
        if (propertyLock()) {
            encrypted = val;
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

size_t TcpCommsSS::dataReceive(TcpConnection* connection, const char* data, size_t bytes_received)
{
    size_t used_total = 0;
    while (used_total < bytes_received) {
        ActionMessage m;
        auto used = m.depacketize(reinterpret_cast<const std::byte*>(data) + used_total,
                                  bytes_received - used_total);
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

void TcpCommsSS::queue_tx_function()
{
    if (serverMode && (PortNumber < 0)) {
        PortNumber = getDefaultBrokerPort();
    }
    if (!serverMode && !outgoingConnectionsAllowed) {
        logError("no server and no outgoing connections-> no way to connect to comms");
        setRxStatus(ConnectionStatus::ERRORED);
        setTxStatus(ConnectionStatus::ERRORED);
        return;
    }
    gmlc::networking::TcpServer::pointer server;
    auto ioctx = gmlc::networking::AsioContextManager::getContextPointer();
    auto sf = encrypted ? gmlc::networking::SocketFactory(encryption_config) :
                          gmlc::networking::SocketFactory();
    auto contextLoop = ioctx->startContextLoop();
    auto dataCall =
        [this](const TcpConnection::pointer& connection, const char* data, size_t datasize) {
            return dataReceive(connection.get(), data, datasize);
        };
    CommsInterface* ci = this;
    auto errorCall = [ci](const TcpConnection::pointer& connection, const std::error_code& error) {
        return commErrorHandler(ci, connection.get(), error);
    };

    if (serverMode) {
        server = gmlc::networking::TcpServer::create(sf,
                                                     ioctx->getBaseContext(),
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
                setRxStatus(ConnectionStatus::ERRORED);
                setTxStatus(ConnectionStatus::ERRORED);
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
            try {
                auto new_connect =
                    gmlc::networking::establishConnection(sf, ioctx->getBaseContext(), conn);

                if (new_connect) {
                    new_connect->setDataCall(dataCall);
                    new_connect->setErrorCall(errorCall);
                    new_connect->send(cstring);
                    new_connect->startReceive();

                    made_connections.emplace_back(conn, std::move(new_connect));
                }
            }
            catch (const std::exception& e) {
                logWarning(std::string("unable to establish connection with ") + conn +
                           "::" + e.what());
            }
        }
    }
    setRxStatus(ConnectionStatus::CONNECTED);
    std::vector<char> buffer;

    TcpConnection::pointer brokerConnection;

    std::map<route_id, TcpConnection::pointer> routes;  // for all the other possible routes
    if (!brokerTargetAddress.empty()) {
        hasBroker = true;
    }
    if (hasBroker) {
        if (brokerPort < 0) {
            brokerPort = getDefaultBrokerPort();
        }
        if (outgoingConnectionsAllowed) {
            try {
                brokerConnection = gmlc::networking::establishConnection(sf,
                                                                         ioctx->getBaseContext(),
                                                                         brokerTargetAddress,
                                                                         std::to_string(brokerPort),
                                                                         std::chrono::milliseconds(
                                                                             connectionTimeout));
                if (!brokerConnection) {
                    logError("initial connection to broker timed out");

                    if (server) {
                        server->close();
                    }
                    setTxStatus(ConnectionStatus::ERRORED);
                    setRxStatus(ConnectionStatus::ERRORED);
                    return;
                }

                brokerConnection->setDataCall(dataCall);
                brokerConnection->setErrorCall(errorCall);

                brokerConnection->send(cstring);
                brokerConnection->startReceive();
            }
            catch (std::exception& e) {
                logError(std::string("unable to establish connection with ") + brokerTargetAddress +
                         "::" + e.what());
                setTxStatus(ConnectionStatus::ERRORED);
                setRxStatus(ConnectionStatus::ERRORED);
                return;
            }
            established_routes[gmlc::networking::makePortAddress(brokerTargetAddress, brokerPort)] =
                parent_route_id;
        }
    }

    setTxStatus(ConnectionStatus::CONNECTED);

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
                                    if ((cmd.payload.to_string() == brokerName) ||
                                        (cmd.payload.to_string() ==
                                         gmlc::networking::makePortAddress(brokerTargetAddress,
                                                                           brokerPort))) {
                                        brokerConnection = std::move(conn);
                                        break;
                                    }
                                }
                                if (conn) {
                                    made_connections.emplace_back(cmd.payload.to_string(),
                                                                  std::move(conn));
                                }
                            } else {
                                logWarning("(tcpss) unable to locate socket");
                            }
                        }
                        break;
                    case NEW_ROUTE: {
                        bool established = false;

                        for (auto& mc : made_connections) {
                            if ((mc.second) && (cmd.payload.to_string() == mc.first)) {
                                routes.emplace(route_id{cmd.getExtraData()}, std::move(mc.second));
                                established = true;
                                established_routes[mc.first] = route_id{cmd.getExtraData()};
                            }
                        }
                        if (!established) {
                            auto efind =
                                established_routes.find(std::string(cmd.payload.to_string()));
                            if (efind != established_routes.end()) {
                                established = true;
                                if (efind->second == parent_route_id) {
                                    routes.emplace(route_id{cmd.getExtraData()}, brokerConnection);
                                } else {
                                    routes.emplace(route_id{cmd.getExtraData()},
                                                   routes[efind->second]);
                                }
                            }
                        }

                        if (!established) {
                            if (outgoingConnectionsAllowed) {
                                try {
                                    auto new_connect = gmlc::networking::establishConnection(
                                        sf,
                                        ioctx->getBaseContext(),
                                        std::string(cmd.payload.to_string()));
                                    if (new_connect) {
                                        new_connect->setDataCall(dataCall);
                                        new_connect->setErrorCall(errorCall);
                                        new_connect->send(cstring);
                                        new_connect->startReceive();
                                        routes.emplace(route_id{cmd.getExtraData()},
                                                       std::move(new_connect));
                                        established_routes[std::string(cmd.payload.to_string())] =
                                            route_id{cmd.getExtraData()};
                                    }
                                }
                                catch (const std::exception& e) {
                                    logWarning(std::string("unable to establish connection with ") +
                                               std::string(cmd.payload.to_string()) +
                                               "::" + e.what());
                                }
                            } else {
                                logWarning(std::string("outgoing connections not allowed ") +
                                           std::string(cmd.payload.to_string()));
                            }
                        }
                    } break;
                    case REMOVE_ROUTE:
                        routes.erase(route_id{cmd.getExtraData()});
                        processed = true;
                        break;
                    case CLOSE_RECEIVER:
                        setRxStatus(ConnectionStatus::TERMINATED);
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
                        logWarning(
                            std::string("(tcpss) unknown message destination message dropped ") +
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
    setTxStatus(ConnectionStatus::TERMINATED);
    if (server) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        server->close();
        server = nullptr;
    }
    if (getRxStatus() == ConnectionStatus::CONNECTED) {
        setRxStatus(ConnectionStatus::TERMINATED);
    }
}

}  // namespace helics::tcp
