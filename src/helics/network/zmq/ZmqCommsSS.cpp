/*
 Copyright (c) 2017-2021,
 Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
 */
#include "ZmqCommsSS.h"

#include "../../core/ActionMessage.hpp"
#include "../NetworkBrokerData.hpp"
#include "../networkDefaults.hpp"
#include "ZmqCommsCommon.h"
#include "ZmqContextManager.h"
#include "ZmqHelper.h"
#include "ZmqRequestSets.h"
#include "zmqSocketDescriptor.h"

#include <iostream>
#include <map>
#include <memory>
#include <utility>
#include <vector>

static const int TX_RX_MSG_COUNT = 20;

namespace helics {
namespace zeromq {
    void ZmqCommsSS::loadNetworkInfo(const NetworkBrokerData& netInfo)
    {
        NetworkCommsInterface::loadNetworkInfo(netInfo);
        if (!propertyLock()) {
            return;
        }
        if (!brokerTargetAddress.empty()) {
            insertProtocol(brokerTargetAddress, interface_type::tcp);
        }
        if (!localTargetAddress.empty()) {
            insertProtocol(localTargetAddress, interface_type::tcp);
        }
        if (localTargetAddress == "tcp://localhost") {
            localTargetAddress = "tcp://127.0.0.1";
        } else if (localTargetAddress == "udp://localhost") {
            localTargetAddress = "udp://127.0.0.1";
        }
        if (brokerTargetAddress == "tcp://localhost") {
            brokerTargetAddress = "tcp://127.0.0.1";
        } else if (brokerTargetAddress == "udp://localhost") {
            brokerTargetAddress = "udp://127.0.0.1";
        }
        propertyUnLock();
    }

    ZmqCommsSS::ZmqCommsSS() noexcept:
        NetworkCommsInterface(interface_type::ip, CommsInterface::thread_generation::single)
    {
        appendNameToAddress = true;
    }

    /** destructor*/
    ZmqCommsSS::~ZmqCommsSS()
    {
        if (requestDisconnect.load() || disconnecting.load()) {
            auto status = getRxStatus();
            while (status != connection_status::terminated && status != connection_status::error) {
                std::this_thread::yield();
                status = getRxStatus();
            }
        } else {
            disconnect();
        }
    }

    int ZmqCommsSS::getDefaultBrokerPort() const { return DEFAULT_ZMQSS_BROKER_PORT_NUMBER; }

    int ZmqCommsSS::processIncomingMessage(zmq::message_t& msg,
                                           std::map<std::string, std::string>& connection_info)
    {
        int status = 0;
        if (msg.size() == 5) {
            std::string str(static_cast<char*>(msg.data()), msg.size());
            if (str == "close") {
                return (-1);
            }
        }
        ActionMessage M(static_cast<char*>(msg.data()), msg.size());

        if (!isValidCommand(M)) {
            std::cerr << "invalid command received" << M.action() << std::endl;
            return 0;
        }
        bool handled{false};
        if (isProtocolCommand(M)) {
            handled = true;
            switch (M.messageID) {
                case PORT_DEFINITIONS:
                    loadPortDefinitions(M);
                    break;
                case NAME_NOT_FOUND:
                    disconnecting = true;
                    setRxStatus(connection_status::error);
                    status = -1;
                    break;
                case CONNECTION_ACK:
                    setTxStatus(connection_status::connected);
                    break;
                case DISCONNECT:
                    disconnecting = true;
                    setRxStatus(connection_status::terminated);
                    status = -1;
                    break;
                case DISCONNECT_ERROR:
                    disconnecting = true;
                    setRxStatus(connection_status::error);
                    status = -1;
                    break;
                case CLOSE_RECEIVER:
                    setRxStatus(connection_status::terminated);
                    status = -1;
                    break;
                case RECONNECT_RECEIVER:
                    setRxStatus(connection_status::connected);
                    break;
                case CONNECTION_INFORMATION:
                    if (serverMode) {
                        auto sdata = M.getStringData();
                        if (sdata.size() == 3) {
                            connection_info.emplace(M.name, sdata[2]);
                        } else {
                            connection_info.emplace(M.name, M.payload);
                        }
                        status = 3;
                    }
                    break;
                case NEW_BROKER_INFORMATION: {
                    logMessage("got new broker information");
                    auto brkprt = extractInterfaceandPort(M.getString(0));
                    brokerPort = brkprt.second;
                    if (brkprt.first != "?") {
                        brokerTargetAddress = brkprt.first;
                    }
                    status = 5;
                } break;
                case DELAY_CONNECTION:
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                    status = 5;  // need to reconnect after this
                    break;
                default:
                    handled = false;
                    break;
            }
        }
        if (!handled) {
            ActionCallback(std::move(M));
        }
        return status;
    }

    int ZmqCommsSS::replyToIncomingMessage(zmq::message_t& msg, zmq::socket_t& sock)
    {
        ActionMessage M(static_cast<char*>(msg.data()), msg.size());
        if (isProtocolCommand(M)) {
            if (M.messageID == CLOSE_RECEIVER) {
                return (-1);
            }
            auto reply = generateReplyToIncomingMessage(M);
            auto str = reply.to_string();
            sock.send(str);
            return 0;
        }
        ActionCallback(std::move(M));
        ActionMessage resp(CMD_PRIORITY_ACK);
        auto str = resp.to_string();
        sock.send(str);
        return 0;
    }

    void ZmqCommsSS::queue_rx_function()
    {
        // Everything is handled by tx thread
    }

    int ZmqCommsSS::initializeConnectionToBroker(zmq::socket_t& brokerConnection)
    {
        brokerConnection.setsockopt(ZMQ_IDENTITY, name.c_str(), name.size());
        brokerConnection.setsockopt(ZMQ_LINGER, 500);
        try {
            brokerConnection.connect(makePortAddress(brokerTargetAddress, brokerPort));
        }
        catch (zmq::error_t& ze) {
            logError(std::string("unable to connect with broker at ") +
                     makePortAddress(brokerTargetAddress, brokerPort + 1) + ":(" + name + ")" +
                     ze.what());
            setTxStatus(connection_status::error);
            return -1;
        }
        std::vector<char> buffer;
        // generate a local protocol connection string to send it's identity
        ActionMessage cmessage(CMD_PROTOCOL);
        cmessage.messageID = CONNECTION_INFORMATION;
        cmessage.name = name;
        cmessage.setStringData(brokerName, brokerInitString, getAddress());
        cmessage.to_vector(buffer);
        brokerConnection.send(zmq::const_buffer(buffer.data(), buffer.size()),
                              zmq::send_flags::dontwait);
        return 0;
    }

    int ZmqCommsSS::initializeBrokerConnections(zmq::socket_t& brokerSocket,
                                                zmq::socket_t& brokerConnection)
    {
        if (serverMode) {
            brokerSocket.setsockopt(ZMQ_LINGER, 500);
            auto bindsuccess =
                bindzmqSocket(brokerSocket, localTargetAddress, PortNumber, connectionTimeout);
            if (!bindsuccess) {
                brokerSocket.close();
                disconnecting = true;
                logError(std::string("Unable to bind zmq router socket giving up ") +
                         makePortAddress(localTargetAddress, PortNumber));
                setRxStatus(connection_status::error);
                return -1;
            }
        }
        if (hasBroker) {
            auto ret = initializeConnectionToBroker(brokerConnection);
            if (ret != 0) {
                return ret;
            }
        }
        return 0;
    }

    bool ZmqCommsSS::processTxControlCmd(const ActionMessage& cmd,
                                         std::map<route_id, std::string>& routes,
                                         std::map<std::string, std::string>& connection_info)
    {
        bool close_tx{false};

        switch (cmd.messageID) {
            case RECONNECT_TRANSMITTER:
                setTxStatus(connection_status::connected);
                break;
            case CONNECTION_INFORMATION:
                // Shouldn't reach here ideally
                if (serverMode) {
                    connection_info.emplace(cmd.name, cmd.payload);
                }
                break;
            case NEW_ROUTE:
                for (auto& mc : connection_info) {
                    if (mc.second == cmd.payload) {
                        routes.emplace(route_id(cmd.getExtraData()), mc.first);
                        break;
                    }
                }
                break;
            case REMOVE_ROUTE:
                routes.erase(route_id(cmd.getExtraData()));
                break;
            case CLOSE_RECEIVER:
            case DISCONNECT:
                close_tx = true;
                break;
        }
        return close_tx;
    }

    static void loadPoller(std::vector<zmq::pollitem_t>& poller,
                           std::vector<zmq::socket_t*>& sockets,
                           zmq::socket_t& brokerSocket,
                           zmq::socket_t& brokerConnection,
                           bool serverMode,
                           bool hasBroker)
    {
        if (serverMode && hasBroker) {
            poller.resize(2);
            sockets.resize(2);
            poller[0].socket = static_cast<void*>(brokerSocket);
            poller[0].events = ZMQ_POLLIN;
            sockets[0] = &brokerSocket;
            poller[1].socket = static_cast<void*>(brokerConnection);
            poller[1].events = ZMQ_POLLIN;
            sockets[1] = &brokerConnection;
        } else {
            if (serverMode) {
                poller.resize(1);
                sockets.resize(1);
                poller[0].socket = static_cast<void*>(brokerSocket);
                poller[0].events = ZMQ_POLLIN;
                sockets[0] = &brokerSocket;
            }
            if (hasBroker) {
                poller.resize(1);
                sockets.resize(1);
                poller[0].socket = static_cast<void*>(brokerConnection);
                poller[0].events = ZMQ_POLLIN;
                sockets[0] = &brokerConnection;
            }
        }
    }

    void ZmqCommsSS::queue_tx_function()
    {
        std::vector<char> buffer;
        auto ctx = ZmqContextManager::getContextPointer();
        zmq::message_t msg;

        if (!brokerTargetAddress.empty()) {
            hasBroker = true;
        }

        // contains mapping between route id and core name
        std::map<route_id, std::string> routes;
        // contains mapping between core name and address
        std::map<std::string, std::string> connection_info;

        if (brokerPort < 0) {
            brokerPort = DEFAULT_ZMQSS_BROKER_PORT_NUMBER;
        }
        if (PortNumber < 0) {
            PortNumber = DEFAULT_ZMQSS_BROKER_PORT_NUMBER;
        }
        zmq::socket_t brokerSocket(ctx->getContext(), ZMQ_ROUTER);
        zmq::socket_t brokerConnection(ctx->getContext(), ZMQ_DEALER);
        auto res = initializeBrokerConnections(brokerSocket, brokerConnection);
        if (res < 0) {
            setTxStatus(connection_status::error);
            brokerSocket.close();
            brokerConnection.close();
            return;
        }
        // Root broker is set
        if (!serverMode) {
            brokerSocket.close();
        }
        if (!hasBroker) {
            brokerConnection.close();
            setTxStatus(connection_status::connected);
        }
        // setTxStatus(connection_status::connected);

        std::vector<zmq::pollitem_t> poller(2);
        std::vector<zmq::socket_t*> sockets(2);
        loadPoller(poller, sockets, brokerSocket, brokerConnection, serverMode, hasBroker);

        setRxStatus(connection_status::connected);

        int status{0};

        bool haltLoop{false};
        //  std::vector<ActionMessage> txlist;
        while (!haltLoop) {
            route_id rid;
            ActionMessage cmd;
            int count{0};

            // Handle Tx messages first
            auto tx_msg = txQueue.try_pop();
            int rc = zmq::poll(poller, 0L);
            if (!tx_msg || (rc <= 0)) {
                std::this_thread::yield();
            }
            int tx_count{0};
            // Balance between tx and rx processing since both running on single thread
            while (tx_msg && (tx_count < TX_RX_MSG_COUNT)) {
                bool processed{false};
                cmd = std::move(tx_msg->second);
                rid = tx_msg->first;
                if (isProtocolCommand(cmd)) {
                    if (rid == control_route) {
                        processed = true;
                        auto close_tx = processTxControlCmd(cmd, routes, connection_info);

                        if (close_tx) {
                            haltLoop = true;
                            break;
                        }
                    }
                }
                if (!processed) {
                    buffer.clear();
                    cmd.to_vector(buffer);
                    if (rid == parent_route_id) {
                        if (hasBroker) {
                            std::string empty;
                            brokerConnection.send(zmq::const_buffer(buffer.data(), buffer.size()),
                                                  zmq::send_flags::dontwait);
                        } else {
                            logWarning("no route to broker for message");
                        }
                    } else if (rid == control_route) {
                        status =
                            processIncomingMessage(msg, connection_info);  //----------> ToCheck
                        if (status < 0) {
                            haltLoop = true;
                            break;
                        }
                    } else {
                        // If route found send out through the front end socket connection
                        auto rt_find = routes.find(rid);
                        if (rt_find != routes.end()) {
                            std::string route_name = rt_find->second;
                            std::string empty;
                            // Need to first send identity and empty string
                            brokerSocket.send(route_name, zmq::send_flags::sndmore);
                            brokerSocket.send(empty, zmq::send_flags::sndmore);
                            // Send the actual data
                            brokerSocket.send(zmq::const_buffer(buffer.data(), buffer.size()),
                                              zmq::send_flags::dontwait);
                        } else {
                            if (hasBroker) {
                                brokerConnection.send(zmq::const_buffer(buffer.data(),
                                                                        buffer.size()),
                                                      zmq::send_flags::dontwait);
                            } else {
                                if (!isDisconnectCommand(cmd)) {
                                    logWarning(
                                        std::string(
                                            "unknown route and no broker, dropping message ") +
                                        prettyPrintString(cmd));
                                }
                            }
                        }
                    }
                }
                tx_count++;
                if (tx_count < TX_RX_MSG_COUNT) {
                    tx_msg = txQueue.try_pop();
                }
            }

            count = 0;
            rc = 1;
            while ((rc > 0) && (count < 5)) {
                rc = zmq::poll(poller, 0L);

                if (rc > 0) {
                    if (zmq::has_message(poller[0])) {
                        status = processRxMessage(*sockets[0], connection_info);
                    }
                    if (serverMode && hasBroker) {
                        if (zmq::has_message(poller[1])) {
                            status = processRxMessage(*sockets[1], connection_info);
                        }
                    }

                    if (status < 0) {
                        haltLoop = true;
                    }
                    if (status == 5) {
                        brokerConnection.close();
                        brokerConnection = zmq::socket_t(ctx->getContext(), ZMQ_DEALER);
                        initializeConnectionToBroker(brokerConnection);
                        loadPoller(
                            poller, sockets, brokerSocket, brokerConnection, serverMode, hasBroker);
                    }
                }
                count++;
            }
        }
        routes.clear();
        connection_info.clear();
        if (serverMode) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            brokerSocket.close();
        }
        if (hasBroker) {
            brokerConnection.close();
        }
        setTxStatus(connection_status::terminated);
        if (getRxStatus() == connection_status::connected) {
            setRxStatus(connection_status::terminated);
        }
    }

    int ZmqCommsSS::processRxMessage(zmq::socket_t& socket,
                                     std::map<std::string, std::string>& connection_info)
    {
        int status = 0;
        zmq::message_t msg1;
        zmq::message_t msg2;

        socket.recv(msg1);
        socket.recv(msg2);
        status = processIncomingMessage(msg2, connection_info);

        if (status == 3) {
            ActionMessage rep(CMD_PROTOCOL);
            rep.messageID = CONNECTION_ACK;
            socket.send(msg1, zmq::send_flags::sndmore);
            socket.send(std::string{}, zmq::send_flags::sndmore);
            socket.send(rep.to_string(), zmq::send_flags::dontwait);
            status = 0;
        }
        return status;
    }

}  // namespace zeromq
}  // namespace helics
