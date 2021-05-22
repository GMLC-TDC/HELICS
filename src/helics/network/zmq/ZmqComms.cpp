/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause

*/
#include "ZmqComms.h"

#include "../../core/ActionMessage.hpp"
#include "../NetworkBrokerData.hpp"
#include "../networkDefaults.hpp"
#include "ZmqCommsCommon.h"
#include "ZmqContextManager.h"
#include "ZmqHelper.h"
#include "ZmqRequestSets.h"
#include "zmqSocketDescriptor.h"

#include <csignal>
#include <map>
#include <memory>
#include <utility>
#include <vector>

namespace helics {
namespace zeromq {
    void ZmqComms::loadNetworkInfo(const NetworkBrokerData& netInfo)
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

    ZmqComms::ZmqComms() noexcept: NetworkCommsInterface(interface_type::ip) {}

    /** destructor*/
    ZmqComms::~ZmqComms() { disconnect(); }

    int ZmqComms::getDefaultBrokerPort() const { return DEFAULT_ZMQ_BROKER_PORT_NUMBER; }

    int ZmqComms::processIncomingMessage(zmq::message_t& msg)
    {
        if (msg.size() == 5) {
            std::string str(static_cast<char*>(msg.data()), msg.size());
            if (str == "close") {
                return (-1);
            }
        }
        ActionMessage M(static_cast<char*>(msg.data()), msg.size());
        if (!isValidCommand(M)) {
            logError("invalid command received");
            ActionMessage Q(static_cast<char*>(msg.data()), msg.size());
            return 0;
        }
        if (isProtocolCommand(M)) {
            switch (M.messageID) {
                case CLOSE_RECEIVER:
                    return (-1);
                case RECONNECT_RECEIVER:
                    setRxStatus(connection_status::connected);
                    break;
                default:
                    break;
            }
        }
        ActionCallback(std::move(M));
        return 0;
    }

    int ZmqComms::replyToIncomingMessage(zmq::message_t& msg, zmq::socket_t& sock)
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

    void ZmqComms::queue_rx_function()
    {
        auto ctx = ZmqContextManager::getContextPointer();
        zmq::socket_t pullSocket(ctx->getContext(), ZMQ_PULL);
        pullSocket.setsockopt(ZMQ_LINGER, 200);
        zmq::socket_t controlSocket(ctx->getContext(), ZMQ_PAIR);
        std::string controlsockString =
            std::string("inproc://") + name + '_' + getRandomID() + "_control";
        try {
            controlSocket.bind(controlsockString.c_str());
        }
        catch (const zmq::error_t& e) {
            logError(std::string("binding error on internal comms socket:") + e.what());
            setRxStatus(connection_status::error);
            return;
        }
        controlSocket.setsockopt(ZMQ_LINGER, 200);

        zmq::socket_t repSocket(ctx->getContext(), ZMQ_REP);
        if (serverMode) {
            repSocket.setsockopt(ZMQ_LINGER, 500);
        }

        while (PortNumber == -1) {
            zmq::message_t msg;
            controlSocket.recv(msg);
            if (msg.size() < 10) {
                continue;
            }
            ActionMessage M(static_cast<char*>(msg.data()), msg.size());

            if (isProtocolCommand(M)) {
                if (M.messageID == PORT_DEFINITIONS) {
                    loadPortDefinitions(M);
                } else if (M.messageID == NAME_NOT_FOUND) {
                    logError(std::string("broker name ") + brokerName +
                             " does not match broker connection");
                    disconnecting = true;
                    setRxStatus(connection_status::error);
                    return;
                } else if (M.messageID == DISCONNECT || M.messageID == CLOSE_RECEIVER) {
                    disconnecting = true;
                    setRxStatus(connection_status::terminated);
                    return;
                } else if (M.messageID == DISCONNECT_ERROR) {
                    disconnecting = true;
                    setRxStatus(connection_status::error);
                    return;
                }
            }
        }
        if (serverMode) {
            auto bindsuccess =
                bindzmqSocket(repSocket, localTargetAddress, PortNumber + 1, connectionTimeout);
            if (!bindsuccess) {
                pullSocket.close();
                repSocket.close();
                disconnecting = true;
                logError(std::string("Unable to bind zmq reply socket giving up ") +
                         makePortAddress(localTargetAddress, PortNumber + 1));
                setRxStatus(connection_status::error);
                return;
            }
        }

        auto bindsuccess =
            bindzmqSocket(pullSocket, localTargetAddress, PortNumber, connectionTimeout);

        if (!bindsuccess) {
            pullSocket.close();
            repSocket.close();
            disconnecting = true;
            logError(std::string("Unable to bind zmq pull socket giving up ") +
                     makePortAddress(localTargetAddress, PortNumber));
            setRxStatus(connection_status::error);
            return;
        }

        std::vector<zmq::pollitem_t> poller(3);
        poller[0].socket = static_cast<void*>(controlSocket);
        poller[0].events = ZMQ_POLLIN;
        poller[1].socket = static_cast<void*>(pullSocket);
        poller[1].events = ZMQ_POLLIN;
        if (serverMode) {
            poller[2].socket = static_cast<void*>(repSocket);
            poller[2].events = ZMQ_POLLIN;
        } else {
            poller.resize(2);
        }
        setRxStatus(connection_status::connected);
        while (true) {
            auto rc = zmq::poll(poller, std::chrono::milliseconds(1000));
            if (rc > 0) {
                zmq::message_t msg;
                if (zmq::has_message(poller[0])) {
                    controlSocket.recv(msg);

                    auto status = processIncomingMessage(msg);
                    if (status < 0) {
                        break;
                    }
                }
                if (zmq::has_message(poller[1])) {
                    pullSocket.recv(msg);
                    auto status = processIncomingMessage(msg);
                    if (status < 0) {
                        break;
                    }
                }
                if (serverMode) {
                    if (zmq::has_message(poller[2])) {
                        repSocket.recv(msg);
                        auto status = replyToIncomingMessage(msg, repSocket);
                        if (status < 0) {
                            break;
                        }
                        continue;
                    }
                }
            }
            if (requestDisconnect.load(std::memory_order::memory_order_acquire)) {
                break;
            }
        }
        disconnecting = true;
        setRxStatus(connection_status::terminated);
    }

    int ZmqComms::initializeBrokerConnections(zmq::socket_t& controlSocket)
    {
        zmq::pollitem_t poller;
        if (hasBroker) {
            auto ctx = ZmqContextManager::getContextPointer();
            if (brokerPort < 0) {
                brokerPort = DEFAULT_ZMQ_BROKER_PORT_NUMBER;
            }

            zmq::socket_t brokerReq(ctx->getContext(), ZMQ_REQ);
            brokerReq.setsockopt(ZMQ_LINGER, 50);
            try {
                brokerReq.connect(makePortAddress(brokerTargetAddress, brokerPort + 1));
            }
            catch (zmq::error_t& ze) {
                logError(std::string("unable to connect with broker at ") +
                         makePortAddress(brokerTargetAddress, brokerPort + 1) + ":(" + name + ")" +
                         ze.what());
                setTxStatus(connection_status::error);
                ActionMessage M(CMD_PROTOCOL);
                M.messageID = DISCONNECT_ERROR;
                controlSocket.send(M.to_string());
                return (-1);
            }

            hasBroker = true;
            zmq::message_t msg;
            if (PortNumber < 0) {
                int cnt = 0;

                int cnt2 = 0;
                while (PortNumber < 0) {
                    if (requestDisconnect.load(std::memory_order::memory_order_acquire)) {
                        ActionMessage M(CMD_PROTOCOL);
                        M.messageID = DISCONNECT;
                        controlSocket.send(M.to_string());
                        return (-3);
                    }
                    ActionMessage getPorts = generatePortRequest((serverMode) ? 2 : 1);
                    auto str = getPorts.to_string();

                    brokerReq.send(str);
                    poller.socket = static_cast<void*>(brokerReq);
                    poller.events = ZMQ_POLLIN;
                    int rc = 0;
                    while (rc == 0) {
                        ++cnt2;
                        rc = zmq::poll(&poller, 1, connectionTimeout);
                        if (rc < 0) {
                            logError("ZMQ broker connection error (2)");
                            setTxStatus(connection_status::error);
                            break;
                        }
                        if (rc == 0) {
                            if (requestDisconnect.load(std::memory_order::memory_order_acquire)) {
                                ActionMessage M(CMD_PROTOCOL);
                                M.messageID = DISCONNECT;
                                controlSocket.send(M.to_string());
                                return (-3);
                            }
                            if (cnt2 == 1) {
                                logWarning("zmq broker connection timed out, trying again (2)");
                                logWarning(std::string("sending message to ") +
                                           makePortAddress(brokerTargetAddress, brokerPort + 1));
                            } else if (cnt2 > maxRetries) {
                                logError(
                                    "zmq broker connection timed out after trying 5 times (2)");
                                setTxStatus(connection_status::error);
                                break;
                            }
                            // try to reestablish the connection
                            brokerReq.close();
                            brokerReq = zmq::socket_t(ctx->getContext(), ZMQ_REQ);
                            brokerReq.setsockopt(ZMQ_LINGER, 50);
                            try {
                                brokerReq.connect(
                                    makePortAddress(brokerTargetAddress, brokerPort + 1));
                                poller.socket = static_cast<void*>(brokerReq);
                            }
                            catch (zmq::error_t& ze) {
                                logError(std::string("unable to connect with broker at ") +
                                         makePortAddress(brokerTargetAddress, brokerPort + 1) +
                                         ":(" + name + ")" + ze.what());
                                setTxStatus(connection_status::error);
                                ActionMessage M(CMD_PROTOCOL);
                                M.messageID = DISCONNECT_ERROR;
                                controlSocket.send(M.to_string());
                                return (-1);
                            }
                            break;
                        }
                    }

                    if (getTxStatus() == connection_status::error) {
                        ActionMessage M(CMD_PROTOCOL);
                        M.messageID = DISCONNECT_ERROR;
                        controlSocket.send(M.to_string());
                        return (-1);
                    }
                    if (rc > 0) {
                        brokerReq.recv(msg);

                        ActionMessage rxcmd(static_cast<char*>(msg.data()), msg.size());
                        if (isProtocolCommand(rxcmd)) {
                            if (rxcmd.messageID == PORT_DEFINITIONS) {
                                controlSocket.send(msg, zmq::send_flags::none);
                                return 0;
                            }
                            if (rxcmd.messageID == DISCONNECT) {
                                controlSocket.send(msg, zmq::send_flags::none);
                                setTxStatus(connection_status::terminated);
                                return (-3);
                            }
                            if (rxcmd.messageID == DISCONNECT_ERROR) {
                                controlSocket.send(msg, zmq::send_flags::none);
                                setTxStatus(connection_status::error);
                                return (-4);
                            }
                            if (rxcmd.messageID == NEW_BROKER_INFORMATION) {
                                logMessage("got new broker information");
                                brokerReq.disconnect(
                                    makePortAddress(brokerTargetAddress, brokerPort + 1));
                                auto brkprt = extractInterfaceandPort(rxcmd.getString(0));
                                brokerPort = brkprt.second;
                                if (brkprt.first != "?") {
                                    brokerTargetAddress = brkprt.first;
                                }
                                try {
                                    brokerReq.connect(
                                        makePortAddress(brokerTargetAddress, brokerPort + 1));
                                }
                                catch (zmq::error_t& ze) {
                                    logError(std::string("unable to connect with broker at ") +
                                             makePortAddress(brokerTargetAddress, brokerPort + 1) +
                                             ":(" + name + ")" + ze.what());
                                    setTxStatus(connection_status::error);
                                    ActionMessage M(CMD_PROTOCOL);
                                    M.messageID = DISCONNECT_ERROR;
                                    controlSocket.send(M.to_string());
                                    return (-1);
                                }
                            } else if (rxcmd.messageID == DELAY_CONNECTION) {
                                std::this_thread::sleep_for(std::chrono::seconds(2));
                            }
                        }
                    }
                    ++cnt;
                    if (cnt > maxRetries) {
                        // we can't get the broker to respond with port numbers
                        setTxStatus(connection_status::error);
                        return (-1);
                    }
                }
            }
        } else {
            if ((PortNumber < 0)) {
                PortNumber = DEFAULT_ZMQ_BROKER_PORT_NUMBER;
                ActionMessage setPorts(CMD_PROTOCOL);
                setPorts.messageID = PORT_DEFINITIONS;
                setPorts.setExtraData(PortNumber);
                auto str = setPorts.to_string();
                controlSocket.send(str);
            }
        }
        return 0;
    }

    void ZmqComms::queue_tx_function()
    {
        std::vector<char> buffer;
        if (!brokerTargetAddress.empty()) {
            hasBroker = true;
        }
        auto ctx = ZmqContextManager::getContextPointer();
        // Setup the control socket for comms with the receiver
        zmq::socket_t controlSocket(ctx->getContext(), ZMQ_PAIR);
        controlSocket.setsockopt(ZMQ_LINGER, 200);
        std::string controlsockString =
            std::string("inproc://") + name + '_' + getRandomID() + "_control";
        controlSocket.connect(controlsockString);
        try {
            auto res = initializeBrokerConnections(controlSocket);
            if (res < 0) {
                setTxStatus((res != -3) ? connection_status::error : connection_status::terminated);

                controlSocket.close();
                return;
            }
        }
        catch (const zmq::error_t&) {
            controlSocket.close();
            return;
        }

        zmq::socket_t brokerPushSocket(ctx->getContext(), ZMQ_PUSH);
        brokerPushSocket.setsockopt(ZMQ_LINGER, 200);
        std::map<route_id, zmq::socket_t> routes;  // for all the other possible routes
        // ZmqRequestSets priority_routes;  //!< object to handle the management of the priority
        // routes

        if (hasBroker) {
            //   priority_routes.addRoutes (0, makePortAddress (brokerTargetAddress, brokerPort+1));
            brokerPushSocket.connect(makePortAddress(brokerTargetAddress, brokerPort));
        }
        setTxStatus(connection_status::connected);
        zmq::message_t msg;
        bool continueProcessing{true};
        while (continueProcessing) {
            route_id rid;
            ActionMessage cmd;

            std::tie(rid, cmd) = txQueue.pop();
            bool processed = false;
            if (isProtocolCommand(cmd)) {
                if (control_route == rid) {
                    switch (cmd.messageID) {
                        case RECONNECT_TRANSMITTER:
                            setTxStatus(connection_status::connected);
                            break;
                        case NEW_BROKER_INFORMATION:
                            brokerPushSocket.close();
                            brokerPushSocket = zmq::socket_t(ctx->getContext(), ZMQ_PUSH);
                            brokerPushSocket.setsockopt(ZMQ_LINGER, 200);
                            brokerTargetAddress = cmd.payload;
                            brokerPort = cmd.getExtraData();
                            brokerPushSocket.connect(
                                makePortAddress(brokerTargetAddress, brokerPort));
                            break;
                        case NEW_ROUTE: {
                            try {
                                auto interfaceAndPort = extractInterfaceandPort(cmd.payload);

                                auto zsock = zmq::socket_t(ctx->getContext(), ZMQ_PUSH);
                                zsock.setsockopt(ZMQ_LINGER, 100);
                                zsock.connect(makePortAddress(interfaceAndPort.first,
                                                              interfaceAndPort.second));
                                routes.emplace(route_id{cmd.getExtraData()}, std::move(zsock));
                            }
                            catch (const zmq::error_t& e) {
                                // TODO(PT): do something???
                                logError(std::string("unable to connect route") + cmd.payload +
                                         "::" + e.what());
                            }
                            processed = true;
                        } break;
                        case REMOVE_ROUTE:
                            routes.erase(route_id{cmd.getExtraData()});
                            processed = true;
                            break;
                        case DISCONNECT:
                            continueProcessing = false;
                            processed = true;
                            break;
                    }
                }
            }
            if (processed) {
                continue;
            }
            cmd.to_vector(buffer);
            if (rid == parent_route_id) {
                if (hasBroker) {
                    brokerPushSocket.send(zmq::const_buffer(buffer.data(), buffer.size()),
                                          zmq::send_flags::none);
                } else {
                    logWarning("no route to broker for message");
                }
            } else if (rid == control_route) {  // send to rx thread loop
                try {
                    controlSocket.send(zmq::const_buffer(buffer.data(), buffer.size()),
                                       zmq::send_flags::dontwait);
                }
                catch (const zmq::error_t& e) {
                    if ((getRxStatus() == connection_status::terminated) ||
                        (getRxStatus() == connection_status::error)) {
                        break;
                    }
                    logError(e.what());
                }
                continue;
            } else {
                auto rt_find = routes.find(rid);
                if (rt_find != routes.end()) {
                    rt_find->second.send(zmq::const_buffer(buffer.data(), buffer.size()));
                } else {
                    if (hasBroker) {
                        brokerPushSocket.send(zmq::const_buffer(buffer.data(), buffer.size()));
                    } else {
                        if (!isDisconnectCommand(cmd)) {
                            logWarning(
                                std::string("unknown route and no broker, dropping message ") +
                                prettyPrintString(cmd));
                        }
                    }
                }
            }
        }
        brokerPushSocket.close();

        routes.clear();
        if (getRxStatus() == connection_status::connected) {
            try {
                controlSocket.send(std::string("close"), zmq::send_flags::dontwait);
            }
            catch (const zmq::error_t&) {
                // this probably just means it got closed simultaneously which would be unusual but
                // not impossible
            }
        }

        controlSocket.close();

        setTxStatus(connection_status::terminated);
    }

    void ZmqComms::closeReceiver()
    {
        switch (getTxStatus()) {
            case connection_status::startup:
            case connection_status::connected: {
                ActionMessage cmd(CMD_PROTOCOL);
                cmd.messageID = CLOSE_RECEIVER;
                transmit(control_route, cmd);
            }

            break;
            default:
                if (!disconnecting) {
                    // try connecting with the receivers push socket
                    auto ctx = ZmqContextManager::getContextPointer();
                    zmq::socket_t pushSocket(ctx->getContext(), ZMQ_PUSH);
                    pushSocket.setsockopt(ZMQ_LINGER, 200);
                    if (localTargetAddress == "tcp://*") {
                        pushSocket.connect(makePortAddress("tcp://127.0.0.1", PortNumber));
                    } else {
                        pushSocket.connect(makePortAddress(localTargetAddress, PortNumber));
                    }

                    ActionMessage cmd(CMD_PROTOCOL);
                    cmd.messageID = CLOSE_RECEIVER;
                    pushSocket.send(cmd.to_string());
                }
                break;
        }
    }

}  // namespace zeromq
}  // namespace helics
