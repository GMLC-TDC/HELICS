/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause

*/
#include "ZmqComms.h"

#include "../../core/ActionMessage.hpp"
#include "../../core/flagOperations.hpp"
#include "../NetworkBrokerData.hpp"
#include "../networkDefaults.hpp"
#include "ZmqCommsCommon.h"
#include "ZmqContextManager.h"
#include "ZmqHelper.h"
#include "ZmqRequestSets.h"
#include "zmqSocketDescriptor.h"

#include <algorithm>
#include <csignal>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace helics::zeromq {

using gmlc::networking::makePortAddress;

void ZmqComms::loadNetworkInfo(const NetworkBrokerData& netInfo)
{
    NetworkCommsInterface::loadNetworkInfo(netInfo);
    if (!propertyLock()) {
        return;
    }
    if (!brokerTargetAddress.empty()) {
        gmlc::networking::insertProtocol(brokerTargetAddress,
                                         gmlc::networking::InterfaceTypes::TCP);
    }
    if (!localTargetAddress.empty()) {
        gmlc::networking::insertProtocol(localTargetAddress, gmlc::networking::InterfaceTypes::TCP);
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

ZmqComms::ZmqComms() noexcept: NetworkCommsInterface(gmlc::networking::InterfaceTypes::IP) {}

/** destructor*/
ZmqComms::~ZmqComms()
{
    disconnect();
}

int ZmqComms::getDefaultBrokerPort() const
{
    return getDefaultPort(HELICS_CORE_TYPE_ZMQ);
}

int ZmqComms::processIncomingMessage(zmq::message_t& msg)
{
    if (msg.size() == 5) {
        std::string str(static_cast<char*>(msg.data()), msg.size());
        if (str == "close") {
            return (-1);
        }
    }
    ActionMessage M(static_cast<std::byte*>(msg.data()), msg.size());
    if (!isValidCommand(M)) {
        logError("invalid command received");
        ActionMessage Q(static_cast<std::byte*>(msg.data()), msg.size());
        return 0;
    }
    if (isProtocolCommand(M)) {
        switch (M.messageID) {
            case CLOSE_RECEIVER:
                return (-1);
            case RECONNECT_RECEIVER:
                setRxStatus(ConnectionStatus::CONNECTED);
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
    ActionMessage M(static_cast<std::byte*>(msg.data()), msg.size());
    bool useJson = checkActionFlag(M, use_json_serialization_flag);
    if (isProtocolCommand(M)) {
        if (M.messageID == CLOSE_RECEIVER) {
            return (-1);
        }
        auto reply = generateReplyToIncomingMessage(M);
        auto str = (useJson) ? reply.to_json_string() : reply.to_string();

        sock.send(str);
        return 0;
    }

    ActionCallback(std::move(M));
    ActionMessage resp(CMD_PRIORITY_ACK);
    auto str = (useJson) ? resp.to_json_string() : resp.to_string();
    sock.send(str);
    return 0;
}

void ZmqComms::queue_rx_function()
{
    auto ctx = ZmqContextManager::getContextPointer();
    zmq::socket_t pullSocket(ctx->getBaseContext(), ZMQ_PULL);
    pullSocket.setsockopt(ZMQ_LINGER, 200);
    zmq::socket_t controlSocket(ctx->getBaseContext(), ZMQ_PAIR);
    std::string controlsockString =
        std::string("inproc://") + name + '_' + getRandomID() + "_control";
    try {
        controlSocket.bind(controlsockString.c_str());
    }
    catch (const zmq::error_t& e) {
        logError(std::string("binding error on internal comms socket:") + e.what());
        setRxStatus(ConnectionStatus::ERRORED);
        return;
    }
    controlSocket.setsockopt(ZMQ_LINGER, 200);

    zmq::socket_t repSocket(ctx->getBaseContext(), ZMQ_REP);
    if (serverMode) {
        repSocket.setsockopt(ZMQ_LINGER, 500);
    }

    while (PortNumber == -1) {
        zmq::message_t msg;
        controlSocket.recv(msg);
        if (msg.size() < 10) {
            continue;
        }
        ActionMessage M(static_cast<std::byte*>(msg.data()), msg.size());

        if (isProtocolCommand(M)) {
            if (M.messageID == PORT_DEFINITIONS) {
                loadPortDefinitions(M);
            } else if (M.messageID == NAME_NOT_FOUND) {
                logError(std::string("broker name ") + brokerName +
                         " does not match broker connection");
                disconnecting = true;
                setRxStatus(ConnectionStatus::ERRORED);
                return;
            } else if (M.messageID == DISCONNECT || M.messageID == CLOSE_RECEIVER) {
                disconnecting = true;
                setRxStatus(ConnectionStatus::TERMINATED);
                return;
            } else if (M.messageID == DISCONNECT_ERROR) {
                disconnecting = true;
                setRxStatus(ConnectionStatus::ERRORED);
                return;
            }
        }
    }
    if (serverMode) {
        auto bindsuccess =
            bindzmqSocket(repSocket, localTargetAddress, PortNumber + 1, connectionTimeout);
        if (!bindsuccess) {
            if (forceConnection) {
                logWarning("attempting to override existing broker and force network connection");
                zmq::socket_t brokerReqTerm(ctx->getBaseContext(), ZMQ_REQ);
                brokerReqTerm.setsockopt(ZMQ_LINGER, 50);
                try {
                    repSocket.close();
                    brokerReqTerm.connect(makePortAddress(localTargetAddress, PortNumber + 1));
                    ActionMessage term(CMD_GLOBAL_ERROR);
                    term.messageID = HELICS_ERROR_TERMINATED;
                    term.payload = "force termination for new broker";
                    auto str = (useJsonSerialization) ? term.to_json_string() : term.to_string();

                    brokerReqTerm.send(str);
                    zmq::pollitem_t poller;
                    poller.socket = static_cast<void*>(brokerReqTerm);
                    poller.events = ZMQ_POLLIN;
                    int rc = zmq::poll(&poller, 1, connectionTimeout);
                    if (rc != 0) {
                        zmq::message_t msg;
                        brokerReqTerm.recv(msg);

                        brokerReqTerm.close();
                        // this sleep is to exceed the linger timeout on the port
                        std::this_thread::sleep_for(std::chrono::milliseconds(1050));
                    } else {
                        brokerReqTerm.close();
                    }
                    bool notConnected = true;
                    int cnt = 0;
                    while (notConnected) {
                        ++cnt;
                        repSocket = zmq::socket_t(ctx->getBaseContext(), ZMQ_REP);
                        if (serverMode) {
                            repSocket.setsockopt(ZMQ_LINGER, 500);
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        bindsuccess = bindzmqSocket(repSocket,
                                                    localTargetAddress,
                                                    PortNumber + 1,
                                                    connectionTimeout);
                        if (bindsuccess) {
                            notConnected = false;
                        } else {
                            repSocket.close();
                            if (cnt >= 10) {
                                break;
                            }
                            std::this_thread::sleep_for(connectionTimeout);
                        }
                    }

                    if (!bindsuccess) {
                        pullSocket.close();
                        repSocket.close();
                        disconnecting = true;
                        logError(
                            std::string("Unable to bind zmq reply even after force termination ") +
                            makePortAddress(localTargetAddress, PortNumber + 1));
                        setRxStatus(ConnectionStatus::ERRORED);
                        return;
                    }
                }
                catch (zmq::error_t& ze) {
                    pullSocket.close();
                    repSocket.close();
                    disconnecting = true;
                    logError(
                        std::string(
                            "unable to make connection with existing server, force override failed with error ") +
                        std::string(ze.what()) + " on " +
                        makePortAddress(localTargetAddress, PortNumber + 1));
                    setRxStatus(ConnectionStatus::ERRORED);
                    return;
                }
            } else {
                pullSocket.close();
                repSocket.close();
                disconnecting = true;
                logError(std::string("Unable to bind zmq reply socket giving up ") +
                         makePortAddress(localTargetAddress, PortNumber + 1));
                setRxStatus(ConnectionStatus::ERRORED);
                return;
            }
        }
    }

    auto bindsuccess = bindzmqSocket(pullSocket, localTargetAddress, PortNumber, connectionTimeout);

    if (!bindsuccess) {
        pullSocket.close();
        repSocket.close();
        disconnecting = true;
        logError(std::string("Unable to bind zmq pull socket giving up ") +
                 makePortAddress(localTargetAddress, PortNumber));
        setRxStatus(ConnectionStatus::ERRORED);
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
    setRxStatus(ConnectionStatus::CONNECTED);

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
        if (requestDisconnect.load(std::memory_order_acquire)) {
            break;
        }
    }
    disconnecting = true;
    setRxStatus(ConnectionStatus::TERMINATED);
}

int ZmqComms::initializeBrokerConnections(zmq::socket_t& controlSocket)
{
    zmq::pollitem_t poller;
    if (hasBroker) {
        auto ctx = ZmqContextManager::getContextPointer();
        if (brokerPort < 0) {
            brokerPort = getDefaultBrokerPort();
        }

        zmq::socket_t brokerReq(ctx->getBaseContext(), ZMQ_REQ);
        brokerReq.setsockopt(ZMQ_LINGER, 50);
        try {
            brokerReq.connect(makePortAddress(brokerTargetAddress, brokerPort + 1));
        }
        catch (zmq::error_t& ze) {
            logError(std::string("unable to connect with broker at ") +
                     makePortAddress(brokerTargetAddress, brokerPort + 1) + ":(" + name + ")" +
                     ze.what());
            setTxStatus(ConnectionStatus::ERRORED);
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
                if (requestDisconnect.load(std::memory_order_acquire)) {
                    ActionMessage M(CMD_PROTOCOL);
                    M.messageID = DISCONNECT;
                    controlSocket.send(M.to_string());
                    return (-3);
                }
                ActionMessage getPorts = generatePortRequest((serverMode) ? 2 : 1);
                if (useJsonSerialization) {
                    setActionFlag(getPorts, use_json_serialization_flag);
                }
                auto str =
                    (useJsonSerialization) ? getPorts.to_json_string() : getPorts.to_string();

                brokerReq.send(str);
                poller.socket = static_cast<void*>(brokerReq);
                poller.events = ZMQ_POLLIN;
                int rc = 0;
                while (rc == 0) {
                    ++cnt2;
                    rc = zmq::poll(&poller, 1, connectionTimeout);
                    if (rc < 0) {
                        logError("ZMQ broker connection error (2)");
                        setTxStatus(ConnectionStatus::ERRORED);
                        break;
                    }
                    if (rc == 0) {
                        if (requestDisconnect.load(std::memory_order_acquire)) {
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
                            logError("zmq broker connection timed out after trying " +
                                     std::to_string(maxRetries) + " times (2)");
                            setTxStatus(ConnectionStatus::ERRORED);
                            break;
                        }
                        // try to reestablish the connection
                        brokerReq.close();
                        brokerReq = zmq::socket_t(ctx->getBaseContext(), ZMQ_REQ);
                        brokerReq.setsockopt(ZMQ_LINGER, 50);
                        try {
                            brokerReq.connect(makePortAddress(brokerTargetAddress, brokerPort + 1));
                            poller.socket = static_cast<void*>(brokerReq);
                        }
                        catch (zmq::error_t& ze) {
                            logError(std::string("unable to connect with broker at ") +
                                     makePortAddress(brokerTargetAddress, brokerPort + 1) + ":(" +
                                     name + ")" + ze.what());
                            setTxStatus(ConnectionStatus::ERRORED);
                            ActionMessage M(CMD_PROTOCOL);
                            M.messageID = DISCONNECT_ERROR;
                            controlSocket.send(M.to_string());
                            return (-1);
                        }
                        break;
                    }
                }

                if (getTxStatus() == ConnectionStatus::ERRORED) {
                    ActionMessage M(CMD_PROTOCOL);
                    M.messageID = DISCONNECT_ERROR;
                    controlSocket.send(M.to_string());
                    return (-1);
                }
                if (rc > 0) {
                    brokerReq.recv(msg);

                    ActionMessage rxcmd(static_cast<std::byte*>(msg.data()), msg.size());
                    if (isProtocolCommand(rxcmd)) {
                        if (rxcmd.messageID == PORT_DEFINITIONS) {
                            controlSocket.send(msg, zmq::send_flags::none);
                            return 0;
                        }
                        if (rxcmd.messageID == DISCONNECT) {
                            controlSocket.send(msg, zmq::send_flags::none);
                            setTxStatus(ConnectionStatus::TERMINATED);
                            return (-3);
                        }
                        if (rxcmd.messageID == DISCONNECT_ERROR) {
                            controlSocket.send(msg, zmq::send_flags::none);
                            setTxStatus(ConnectionStatus::ERRORED);
                            return (-4);
                        }
                        if (rxcmd.messageID == NEW_BROKER_INFORMATION) {
                            logMessage("got new broker information");
                            brokerReq.disconnect(
                                makePortAddress(brokerTargetAddress, brokerPort + 1));
                            auto brkprt =
                                gmlc::networking::extractInterfaceAndPort(rxcmd.getString(0));
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
                                setTxStatus(ConnectionStatus::ERRORED);
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
                    setTxStatus(ConnectionStatus::ERRORED);
                    return (-1);
                }
            }
        }
    } else {
        if ((PortNumber < 0)) {
            PortNumber = getDefaultBrokerPort();
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
    zmq::socket_t controlSocket(ctx->getBaseContext(), ZMQ_PAIR);
    controlSocket.setsockopt(ZMQ_LINGER, 200);
    std::string controlsockString =
        std::string("inproc://") + name + '_' + getRandomID() + "_control";
    controlSocket.connect(controlsockString);
    try {
        auto res = initializeBrokerConnections(controlSocket);
        if (res < 0) {
            setTxStatus((res != -3) ? ConnectionStatus::ERRORED : ConnectionStatus::TERMINATED);

            controlSocket.close();
            return;
        }
    }
    catch (const zmq::error_t&) {
        controlSocket.close();
        return;
    }

    zmq::socket_t brokerPushSocket(ctx->getBaseContext(), ZMQ_PUSH);
    brokerPushSocket.setsockopt(ZMQ_LINGER, 200);
    std::map<route_id, zmq::socket_t> routes;  // for all the other possible routes
    // ZmqRequestSets priority_routes;  //!< object to handle the management of the priority
    // routes

    if (hasBroker) {
        //   priority_routes.addRoutes (0, makePortAddress (brokerTargetAddress, brokerPort+1));
        brokerPushSocket.connect(makePortAddress(brokerTargetAddress, brokerPort));
    }
    setTxStatus(ConnectionStatus::CONNECTED);
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
                        setTxStatus(ConnectionStatus::CONNECTED);
                        break;
                    case NEW_BROKER_INFORMATION:
                        brokerPushSocket.close();
                        brokerPushSocket = zmq::socket_t(ctx->getBaseContext(), ZMQ_PUSH);
                        brokerPushSocket.setsockopt(ZMQ_LINGER, 200);
                        brokerTargetAddress = cmd.payload.to_string();
                        brokerPort = cmd.getExtraData();
                        brokerPushSocket.connect(makePortAddress(brokerTargetAddress, brokerPort));
                        break;
                    case NEW_ROUTE: {
                        try {
                            auto interfaceAndPort = gmlc::networking::extractInterfaceAndPort(
                                std::string(cmd.payload.to_string()));

                            auto zsock = zmq::socket_t(ctx->getBaseContext(), ZMQ_PUSH);
                            zsock.setsockopt(ZMQ_LINGER, 100);
                            zsock.connect(
                                makePortAddress(interfaceAndPort.first, interfaceAndPort.second));
                            routes.emplace(route_id{cmd.getExtraData()}, std::move(zsock));
                        }
                        catch (const zmq::error_t& e) {
                            // TODO(PT): do something???
                            logError(std::string("unable to connect route") +
                                     std::string(cmd.payload.to_string()) + "::" + e.what());
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
        if (getRouteTypeCode(rid) == json_route_code || useJsonSerialization) {
            auto str = cmd.to_json_string();
            buffer.resize(str.size());
            std::copy(str.begin(), str.end(), buffer.begin());

        } else {
            cmd.to_vector(buffer);
        }
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
                if ((getRxStatus() == ConnectionStatus::TERMINATED) ||
                    (getRxStatus() == ConnectionStatus::ERRORED)) {
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
                    if (!isIgnoreableCommand(cmd)) {
                        logWarning(std::string("unknown route and no broker, dropping message ") +
                                   prettyPrintString(cmd));
                    }
                }
            }
        }
    }
    brokerPushSocket.close();

    routes.clear();
    if (getRxStatus() == ConnectionStatus::CONNECTED) {
        try {
            controlSocket.send(std::string("close"), zmq::send_flags::dontwait);
        }
        catch (const zmq::error_t&) {
            // this probably just means it got closed simultaneously which would be unusual but
            // not impossible
        }
    }

    controlSocket.close();

    setTxStatus(ConnectionStatus::TERMINATED);
}

void ZmqComms::closeReceiver()
{
    switch (getTxStatus()) {
        case ConnectionStatus::STARTUP:
        case ConnectionStatus::CONNECTED: {
            ActionMessage cmd(CMD_PROTOCOL);
            cmd.messageID = CLOSE_RECEIVER;
            transmit(control_route, cmd);
        }

        break;
        default:
            if (!disconnecting) {
                // try connecting with the receivers push socket
                auto ctx = ZmqContextManager::getContextPointer();
                zmq::socket_t pushSocket(ctx->getBaseContext(), ZMQ_PUSH);
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

}  // namespace helics::zeromq
