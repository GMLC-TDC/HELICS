/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "TcpComms.h"

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
    using asio::ip::tcp;

    TcpComms::TcpComms() noexcept: NetworkCommsInterface(interface_type::tcp) {}

    int TcpComms::getDefaultBrokerPort() const { return DEFAULT_TCP_BROKER_PORT_NUMBER; }

    /** load network information into the comms object*/
    void TcpComms::loadNetworkInfo(const NetworkBrokerData& netInfo)
    {
        NetworkCommsInterface::loadNetworkInfo(netInfo);
        if (!propertyLock()) {
            return;
        }
        reuse_address = netInfo.reuse_address;
        propertyUnLock();
    }

    void TcpComms::setFlag(const std::string& flag, bool val)
    {
        if (flag == "reuse_address") {
            if (propertyLock()) {
                reuse_address = val;
                propertyUnLock();
            }
        } else {
            NetworkCommsInterface::setFlag(flag, val);
        }
    }

    /** destructor*/
    TcpComms::~TcpComms() { disconnect(); }

    int TcpComms::processIncomingMessage(ActionMessage&& cmd)
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

    size_t TcpComms::dataReceive(TcpConnection* connection, const char* data, size_t bytes_received)
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
                // if the reply is not ignored respond with it otherwise
                // forward the original message on to the receiver to handle
                auto rep = generateReplyToIncomingMessage(m);
                if (rep.action() != CMD_IGNORE) {
                    try {
                        connection->send(rep.packetize());
                    }
                    catch (const std::system_error&) {
                    }
                } else {
                    rxMessageQueue.push(std::move(m));
                }
            } else {
                if (ActionCallback) {
                    ActionCallback(std::move(m));
                }
            }
            used_total += used;
        }

        return used_total;
    }

    void TcpComms::queue_rx_function()
    {
        while (PortNumber < 0) {
            auto message = rxMessageQueue.pop();
            if (isProtocolCommand(message)) {
                switch (message.messageID) {
                    case PORT_DEFINITIONS: {
                        loadPortDefinitions(message);
                    }

                    break;
                    case CLOSE_RECEIVER:
                    case DISCONNECT:
                        disconnecting = true;
                        setRxStatus(connection_status::terminated);
                        return;
                }
            }
        }
        if (PortNumber < 0) {
            setRxStatus(connection_status::error);
            return;
        }
        auto ioctx = AsioContextManager::getContextPointer();
        auto server = helics::tcp::TcpServer::create(ioctx->getBaseContext(),
                                                     localTargetAddress,
                                                     static_cast<uint16_t>(PortNumber.load()),
                                                     reuse_address,
                                                     maxMessageSize);
        while (!server->isReady()) {
            if ((autoPortNumber) &&
                (hasBroker)) {  // If we failed and we are on an automatically assigned port number,
                                // just try a different port
                server->close();
                ++PortNumber;
                server = helics::tcp::TcpServer::create(ioctx->getBaseContext(),
                                                        localTargetAddress,
                                                        static_cast<uint16_t>(PortNumber),
                                                        reuse_address,
                                                        maxMessageSize);
            } else {
                logWarning("retrying tcp bind");
                std::this_thread::sleep_for(std::chrono::milliseconds(150));
                auto connected = server->reConnect(connectionTimeout);
                if (!connected) {
                    logError("unable to bind to tcp connection socket");
                    server->close();
                    setRxStatus(connection_status::error);
                    return;
                }
            }
        }
        auto contextLoop = ioctx->startContextLoop();
        server->setDataCall(
            [this](const TcpConnection::pointer& connection, const char* data, size_t datasize) {
                return dataReceive(connection.get(), data, datasize);
            });
        CommsInterface* ci = this;
        server->setErrorCall(
            [ci](const TcpConnection::pointer& connection, const std::error_code& error) {
                return commErrorHandler(ci, connection.get(), error);
            });
        server->start();
        setRxStatus(connection_status::connected);
        bool loopRunning = true;
        while (loopRunning) {
            auto message = rxMessageQueue.pop();
            if (isProtocolCommand(message)) {
                switch (message.messageID) {
                    case CLOSE_RECEIVER:
                    case DISCONNECT:
                        loopRunning = false;
                        break;
                }
            }
        }

        disconnecting = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        server->close();
        setRxStatus(connection_status::terminated);
    }

    void TcpComms::txReceive(const char* data,
                             size_t bytes_received,
                             const std::string& errorMessage)
    {
        if (errorMessage.empty()) {
            ActionMessage m(data, bytes_received);
            if (isProtocolCommand(m)) {
                txQueue.emplace(control_route, m);
            }
        } else {
            logError(errorMessage);
        }
    }

    bool TcpComms::establishBrokerConnection(std::shared_ptr<AsioContextManager>& ioctx,
                                             std::shared_ptr<TcpConnection>& brokerConnection)
    {
        // lambda function that does the proper termination
        auto terminate = [&, this](connection_status status) -> bool {
            if (brokerConnection) {
                brokerConnection->close();
                brokerConnection = nullptr;
            }
            setTxStatus(status);
            return false;
        };

        if (brokerPort < 0) {
            brokerPort = DEFAULT_TCP_BROKER_PORT_NUMBER;
        }
        try {
            brokerConnection = makeConnection(ioctx->getBaseContext(),
                                              brokerTargetAddress,
                                              std::to_string(brokerPort),
                                              maxMessageSize,
                                              connectionTimeout);
            int retries = 0;
            while (!brokerConnection) {
                if (requestDisconnect.load(std::memory_order::memory_order_acquire)) {
                    return terminate(connection_status::terminated);
                }
                if (retries == 0) {
                    logWarning("initial connection to broker timed out ");
                }
                ++retries;
                if (retries > maxRetries) {
                    logWarning(
                        "initial connection to broker timed out exceeding max number of retries ");
                    return terminate(connection_status::error);
                }
                if (retries % 2 == 1) {
                    std::this_thread::yield();
                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }

                if (requestDisconnect.load(std::memory_order::memory_order_acquire)) {
                    return terminate(connection_status::terminated);
                }
                brokerConnection = makeConnection(ioctx->getBaseContext(),
                                                  brokerTargetAddress,
                                                  std::to_string(brokerPort),
                                                  maxMessageSize,
                                                  connectionTimeout);
            }
            if (requestDisconnect.load(std::memory_order::memory_order_acquire)) {
                return terminate(connection_status::terminated);
            }
            // monitor the total waiting time before connections
            std::chrono::milliseconds cumulativeSleep{0};
            const std::chrono::milliseconds popTimeout{200};

            bool connectionEstablished{false};
            if (PortNumber > 0 && NetworkCommsInterface::noAckConnection) {
                connectionEstablished = true;
            }
            while (!connectionEstablished) {
                ActionMessage m(CMD_PROTOCOL_PRIORITY);
                m.messageID = (PortNumber <= 0) ? REQUEST_PORTS : CONNECTION_REQUEST;

                m.setStringData(brokerName, brokerInitString);
                try {
                    brokerConnection->send(m.packetize());
                }
                catch (const std::system_error& error) {
                    logError(std::string("error in initial send to broker ") + error.what());
                    return terminate(connection_status::error);
                }
                std::vector<char> rx(512);
                tcp::endpoint brk;
                brokerConnection->async_receive(
                    rx.data(), 128, [this, &rx](const std::error_code& error, size_t bytes) {
                        if (!error) {
                            txReceive(rx.data(), bytes, std::string());
                        } else {
                            if (error != asio::error::operation_aborted) {
                                txReceive(rx.data(), bytes, error.message());
                            }
                        }
                    });
                auto mess = txQueue.pop(popTimeout);
                if (mess) {
                    if (isProtocolCommand(mess->second)) {
                        if (mess->second.messageID == PORT_DEFINITIONS) {
                            if (PortNumber <= 0) {
                                rxMessageQueue.push(mess->second);
                                connectionEstablished = true;
                                continue;
                            }
                        }
                        if (mess->second.messageID == CONNECTION_ACK) {
                            if (PortNumber > 0) {
                                connectionEstablished = true;
                                continue;
                            }
                        }
                        if (mess->second.messageID == DISCONNECT) {
                            return terminate(connection_status::terminated);
                        }
                        if (mess->second.messageID == NEW_BROKER_INFORMATION) {
                            logMessage("got new broker information");
                            brokerConnection->close();

                            auto brkprt = extractInterfaceandPort(mess->second.getString(0));
                            brokerPort = brkprt.second;
                            if (brkprt.first != "?") {
                                brokerTargetAddress = brkprt.first;
                            }
                            brokerConnection = makeConnection(ioctx->getBaseContext(),
                                                              brokerTargetAddress,
                                                              std::to_string(brokerPort),
                                                              maxMessageSize,
                                                              connectionTimeout);
                            continue;
                        }
                        if (mess->second.messageID == DELAY_CONNECTION) {
                            std::this_thread::sleep_for(std::chrono::seconds(2));
                            continue;
                        }
                        rxMessageQueue.push(mess->second);
                    } else {
                        logWarning("unexpected message received in transmit queue");
                    }
                } else {
                    cumulativeSleep += popTimeout;
                    if (cumulativeSleep >= connectionTimeout) {
                        brokerConnection->cancel();
                        logError("port number query to broker timed out");
                        return terminate(connection_status::error);
                    }
                }
            }
        }
        catch (std::exception& e) {
            logError(std::string("error connecting with Broker") + e.what());
            return terminate(connection_status::error);
        }
        return true;
    }

    void TcpComms::queue_tx_function()
    {
        // std::vector<char> buffer;
        auto ioctx = AsioContextManager::getContextPointer();
        auto contextLoop = ioctx->startContextLoop();
        TcpConnection::pointer brokerConnection;

        std::map<route_id, TcpConnection::pointer> routes;  // for all the other possible routes
        if (!brokerTargetAddress.empty()) {
            hasBroker = true;
        }
        if (hasBroker) {
            if (!establishBrokerConnection(ioctx, brokerConnection)) {
                ActionMessage m(CMD_PROTOCOL);
                m.messageID = CLOSE_RECEIVER;
                rxMessageQueue.push(m);
                return;
            }
        } else {
            if (PortNumber < 0) {
                PortNumber = DEFAULT_TCP_BROKER_PORT_NUMBER;
                ActionMessage m(CMD_PROTOCOL);
                m.messageID = PORT_DEFINITIONS;
                m.setExtraData(PortNumber);
                rxMessageQueue.push(m);
            }
        }
        setTxStatus(connection_status::connected);

        //  std::vector<ActionMessage> txlist;
        bool processing{true};
        while (processing) {
            route_id rid;
            ActionMessage cmd;

            std::tie(rid, cmd) = txQueue.pop();
            bool processed = false;
            if (isProtocolCommand(cmd)) {
                if (rid == control_route) {
                    switch (cmd.messageID) {
                        case NEW_ROUTE: {
                            auto& newroute = cmd.payload;

                            try {
                                std::string interface;
                                std::string port;
                                std::tie(interface, port) = extractInterfaceandPortString(newroute);
                                auto new_connect =
                                    TcpConnection::create(ioctx->getBaseContext(), interface, port);

                                routes.emplace(route_id{cmd.getExtraData()},
                                               std::move(new_connect));
                            }
                            catch (std::exception&) {
                                // TODO(PT):: do something???
                            }
                            processed = true;
                        } break;
                        case REMOVE_ROUTE:
                            routes.erase(route_id{cmd.getExtraData()});
                            processed = true;
                            break;
                        case CLOSE_RECEIVER:
                            rxMessageQueue.push(cmd);
                            processed = true;
                            break;
                        case DISCONNECT:
                            processing = false;
                            processed = true;
                            break;
                    }
                }
            }
            if (processed) {
                continue;
            }

            if (rid == parent_route_id) {
                if (hasBroker) {
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

                    // if (error)
                    {
                        //     std::cerr << "transmit failure to broker " << error.message() <<
                        //     '\n';
                    }
                }
            } else if (rid == control_route) {  // send to rx thread loop
                rxMessageQueue.push(cmd);
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
                                    logError(std::string("broker send") +
                                             std::to_string(rid.baseValue()) + " ::" + se.what());
                                }
                            }
                        }
                    } else {
                        if (!isDisconnectCommand(cmd)) {
                            logWarning(
                                std::string("(tcp) unknown message destination message dropped ") +
                                prettyPrintString(cmd));
                        }
                    }
                }
            }
        }
        for (auto& rt : routes) {
            rt.second->close();
        }
        routes.clear();
        if (getRxStatus() == connection_status::connected) {
            closeReceiver();
        }
        setTxStatus(connection_status::terminated);
    }

    void TcpComms::closeReceiver()
    {
        ActionMessage cmd(CMD_PROTOCOL);
        cmd.messageID = CLOSE_RECEIVER;
        rxMessageQueue.push(cmd);
    }

}  // namespace tcp
}  // namespace helics
