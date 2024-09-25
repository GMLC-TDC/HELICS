/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "UdpComms.h"

#include "../../core/ActionMessage.hpp"
#include "../NetworkBrokerData.hpp"
#include "../networkDefaults.hpp"
#include "gmlc/networking/AsioContextManager.h"

#include <asio/ip/udp.hpp>
#include <fmt/format.h>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace helics::udp {
using asio::ip::udp;
UdpComms::UdpComms(): NetworkCommsInterface(gmlc::networking::InterfaceTypes::UDP)
{
    futurePort = promisePort.get_future();
}

int UdpComms::getDefaultBrokerPort() const
{
    return getDefaultPort(HELICS_CORE_TYPE_UDP);
}

/** load network information into the comms object*/
void UdpComms::loadNetworkInfo(const NetworkBrokerData& netInfo)
{
    NetworkCommsInterface::loadNetworkInfo(netInfo);
    if (!propertyLock()) {
        return;
    }

    promisePort = std::promise<int>();
    futurePort = promisePort.get_future();

    propertyUnLock();
}
/** destructor*/
UdpComms::~UdpComms()
{
    disconnect();
}

static inline auto udpnet(gmlc::networking::InterfaceNetworks net)
{
    return (net != gmlc::networking::InterfaceNetworks::IPV6) ? udp::v4() : udp::v6();
}

void UdpComms::queue_rx_function()
{
    using gmlc::networking::makePortAddress;

    if (PortNumber < 0) {
        PortNumber = futurePort.get();
    }
    if (PortNumber < 0) {
        setRxStatus(ConnectionStatus::ERRORED);
        return;
    }
    auto ioctx = gmlc::networking::AsioContextManager::getContextPointer();
    udp::socket socket(ioctx->getBaseContext());
    socket.open(udpnet(interfaceNetwork));
    std::chrono::milliseconds t_cnt{0};
    bool bindsuccess = false;
    while (!bindsuccess) {
        try {
            socket.bind(udp::endpoint(udpnet(interfaceNetwork), static_cast<uint16_t>(PortNumber)));
            bindsuccess = true;
        }
        catch (const std::system_error& error) {
            if ((autoPortNumber) &&
                (hasBroker)) {  // If we failed and we are on an automatically assigned port
                                // number,  just try a different port
                int tries = 0;
                while (!bindsuccess) {
                    ++PortNumber;
                    try {
                        socket.bind(udp::endpoint(udpnet(interfaceNetwork),
                                                  static_cast<uint16_t>(PortNumber)));
                        bindsuccess = true;
                    }
                    catch (const std::system_error&) {
                        ++tries;
                        if (tries > 10) {
                            break;
                        }
                    }
                }
                if (!bindsuccess) {
                    disconnecting = true;
                    logError(fmt::format("unable to bind socket {} :{}",
                                         makePortAddress(localTargetAddress, PortNumber),
                                         error.what()));
                    socket.close();
                    setRxStatus(ConnectionStatus::ERRORED);
                    return;
                }
                continue;
            }
            if (t_cnt == std::chrono::milliseconds(0)) {
                logWarning(fmt::format("bind error on UDP socket {} :{}",
                                       makePortAddress(localTargetAddress, PortNumber),
                                       error.what()));
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            t_cnt += std::chrono::milliseconds(200);
            if (t_cnt > connectionTimeout) {
                disconnecting = true;
                logError(fmt::format("unable to bind socket {} :{}",
                                     makePortAddress(localTargetAddress, PortNumber),
                                     error.what()));
                socket.close();
                setRxStatus(ConnectionStatus::ERRORED);
                return;
            }
        }
    }

    std::vector<char> data(10192);
    udp::endpoint remote_endp;
    std::error_code error;
    std::error_code ignored_error;
    setRxStatus(ConnectionStatus::CONNECTED);
    while (true) {
        auto len = socket.receive_from(asio::buffer(data), remote_endp, 0, error);
        if (error) {
            setRxStatus(ConnectionStatus::ERRORED);
            return;
        }
        if (len == 5) {
            const std::string_view str(data.data(), len);
            if (str == "close") {
                break;
            }
        }
        ActionMessage cmd(reinterpret_cast<std::byte*>(data.data()), len);
        if (!isValidCommand(cmd)) {
            logWarning("invalid command received udp");
            continue;
        }
        if (isProtocolCommand(cmd)) {
            if (cmd.messageID == CLOSE_RECEIVER) {
                break;
            }
            auto reply = generateReplyToIncomingMessage(cmd);
            if (reply.messageID == DISCONNECT) {
                break;
            }
            if (reply.action() != CMD_IGNORE) {
                socket.send_to(asio::buffer(reply.to_string()), remote_endp, 0, ignored_error);
            }
        } else {
            ActionCallback(std::move(cmd));
        }
    }
    disconnecting = true;
    setRxStatus(ConnectionStatus::TERMINATED);
}

// NOLINTNEXTLINE
void UdpComms::queue_tx_function()
{
    auto ioctx = gmlc::networking::AsioContextManager::getContextPointer();
    udp::resolver resolver(ioctx->getBaseContext());
    bool closingRx = false;
    udp::socket transmitSocket(ioctx->getBaseContext());
    transmitSocket.open(udpnet(interfaceNetwork));
    if (PortNumber >= 0) {
        promisePort.set_value(PortNumber);
    }

    std::error_code error;
    std::map<route_id, udp::endpoint> routes;  // for all the other possible routes
    udp::endpoint broker_endpoint;

    if (!brokerTargetAddress.empty()) {
        hasBroker = true;
    }
    if (hasBroker) {
        if (brokerPort < 0) {
            brokerPort = getDefaultBrokerPort();
        }
        try {
            const udp::resolver::query query(udpnet(interfaceNetwork),
                                             brokerTargetAddress,
                                             std::to_string(brokerPort));
            // Setup the control socket for comms with the receiver
            broker_endpoint = *resolver.resolve(query);
            int retries{0};
            bool connectionEstablished{false};
            if (PortNumber.load() > 0 && NetworkCommsInterface::noAckConnection) {
                connectionEstablished = true;
            }
            const decltype(std::chrono::steady_clock::now()) startTime{
                std::chrono::steady_clock::now()};
            int errorCount{0};
            while (!connectionEstablished) {
                if (requestDisconnect.load(std::memory_order_acquire)) {
                    if (PortNumber.load() <= 0) {
                        PortNumber = -1;
                        promisePort.set_value(-1);
                    }
                    setTxStatus(ConnectionStatus::TERMINATED);
                    return;
                }
                ActionMessage cmd(CMD_PROTOCOL_PRIORITY);
                cmd.messageID = (PortNumber <= 0) ? REQUEST_PORTS : CONNECTION_REQUEST;
                cmd.setStringData(brokerName, brokerInitString);
                transmitSocket.send_to(asio::buffer(cmd.to_string()), broker_endpoint, 0, error);
                if (error) {
                    logError(fmt::format("error in initial send to broker {}", error.message()));
                    if (PortNumber.load() <= 0) {
                        PortNumber = -1;
                        promisePort.set_value(-1);
                    }

                    setTxStatus(ConnectionStatus::ERRORED);
                    return;
                }

                bool timeout{false};
                std::vector<char> receiveData(128);
                udp::endpoint brk;

                while ((transmitSocket.available() == 0) && (!timeout)) {
                    if (std::chrono::steady_clock::now() - startTime > connectionTimeout) {
                        timeout = true;
                    }
                    if (requestDisconnect.load(std::memory_order_acquire)) {
                        if (PortNumber.load() <= 0) {
                            PortNumber = -1;
                            promisePort.set_value(-1);
                        }
                        setTxStatus(ConnectionStatus::TERMINATED);
                        return;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
                if (timeout) {
                    ++retries;
                    if (retries > maxRetries) {
                        logError("the max number of retries has been exceeded");
                        if (PortNumber.load() <= 0) {
                            PortNumber = -1;
                            promisePort.set_value(-1);
                        }
                        setTxStatus(ConnectionStatus::ERRORED);
                        return;
                    }
                    continue;
                }
                auto len = transmitSocket.receive_from(asio::buffer(receiveData), brk, 0, error);
                if (error) {
                    if (errorCount++ == 0) {
                        logError(
                            fmt::format("error in initial receive broker {}", error.message()));
                    }
                    if (std::chrono::steady_clock::now() - startTime > connectionTimeout) {
                        if (PortNumber.load() <= 0) {
                            PortNumber = -1;
                            promisePort.set_value(-1);
                        }
                        setTxStatus(ConnectionStatus::ERRORED);
                        logError(fmt::format("timeerror in broker receive {}", error.message()));
                        return;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    continue;
                }
                cmd = ActionMessage(reinterpret_cast<std::byte*>(receiveData.data()), len);
                if (isProtocolCommand(cmd)) {
                    if (cmd.messageID == PORT_DEFINITIONS) {
                        loadPortDefinitions(cmd);
                        promisePort.set_value(PortNumber);
                        connectionEstablished = true;
                    } else if (cmd.messageID == CONNECTION_ACK) {
                        if (PortNumber.load() > 0) {
                            connectionEstablished = true;
                            continue;
                        }
                    } else if (cmd.messageID == NEW_BROKER_INFORMATION) {
                        logMessage("got new broker information");
                        auto brkprt = gmlc::networking::extractInterfaceAndPort(cmd.getString(0));
                        brokerPort = brkprt.second;
                        if (brkprt.first != "?") {
                            brokerTargetAddress = brkprt.first;
                        }
                        const udp::resolver::query localQuery(udpnet(interfaceNetwork),
                                                              brokerTargetAddress,
                                                              std::to_string(brokerPort));
                        // Setup the control socket for comms with the receiver
                        broker_endpoint = *resolver.resolve(localQuery);
                        continue;
                    } else if (cmd.messageID == DELAY_CONNECTION) {
                        std::this_thread::sleep_for(std::chrono::seconds(2));
                    } else if (cmd.messageID == DISCONNECT) {
                        if (PortNumber <= 0) {
                            PortNumber = -1;
                            promisePort.set_value(-1);
                        }
                        setTxStatus(ConnectionStatus::TERMINATED);
                        return;
                    }
                }
            }
        }
        catch (std::exception& err) {
            logError(fmt::format("error connecting to broker {}", err.what()));
            PortNumber = -1;
            promisePort.set_value(-1);
            setTxStatus(ConnectionStatus::ERRORED);
            return;
        }
    } else {
        if (PortNumber < 0) {
            PortNumber = getDefaultBrokerPort();
            promisePort.set_value(PortNumber);
        }
    }
    udp::endpoint rxEndpoint;
    if (localTargetAddress.empty() || localTargetAddress == "*" ||
        localTargetAddress == "udp://*") {
        const udp::resolver::query queryLocal(udpnet(interfaceNetwork),
                                              "127.0.0.1",
                                              std::to_string(PortNumber));
        auto result = resolver.resolve(queryLocal);
        rxEndpoint = *result;
    } else {
        const udp::resolver::query queryLocal(udpnet(interfaceNetwork),
                                              localTargetAddress,
                                              std::to_string(PortNumber));
        auto result = resolver.resolve(queryLocal, error);
        if (error) {
            logError(fmt::format("Unable to resolve: {}", localTargetAddress));
            setTxStatus(ConnectionStatus::ERRORED);
            return;
        }
        rxEndpoint = *result;
    }

    setTxStatus(ConnectionStatus::CONNECTED);
    bool continueProcessing{true};
    while (continueProcessing) {
        route_id rid;
        ActionMessage cmd;

        std::tie(rid, cmd) = txQueue.pop();
        bool processed = false;
        if (isProtocolCommand(cmd)) {
            if (rid == control_route) {
                switch (cmd.messageID) {
                    case NEW_ROUTE: {
                        try {
                            const std::string newroute(cmd.payload.to_string());
                            std::string interface;
                            std::string port;
                            std::tie(interface, port) =
                                gmlc::networking::extractInterfaceAndPortString(newroute);
                            const udp::resolver::query queryNew(udpnet(interfaceNetwork),
                                                                interface,
                                                                port);

                            routes.emplace(route_id{cmd.getExtraData()},
                                           *resolver.resolve(queryNew));
                        }
                        catch (const std::exception& err) {
                            logError(fmt::format("unable to resolve new route: {}", err.what()));
                        }
                        processed = true;
                    } break;
                    case REMOVE_ROUTE:
                        routes.erase(route_id{cmd.getExtraData()});
                        processed = true;
                        break;
                    case CLOSE_RECEIVER:
                        transmitSocket.send_to(asio::buffer(cmd.to_string()), rxEndpoint, 0, error);
                        if (error) {
                            logError(
                                fmt::format("transmit failure on sending 'close' to receiver  {}",
                                            error.message()));
                        }
                        closingRx = true;
                        processed = true;
                        break;
                    case DISCONNECT:
                        continueProcessing = false;
                        processed = true;
                        break;
                    default:
                        break;
                }
            }
        }
        if (processed) {
            continue;
        }

        if (rid == parent_route_id) {
            if (hasBroker) {
                transmitSocket.send_to(asio::buffer(cmd.to_string()), broker_endpoint, 0, error);
                if (error) {
                    logWarning(
                        fmt::format("transmit failure sending to broker  {}", error.message()));
                }
            } else {
                logWarning(fmt::format(
                    "message directed to broker of comm system with no broker, message dropped {}",
                    prettyPrintString(cmd)));
            }
        } else if (rid == control_route) {  // send to rx thread loop
            transmitSocket.send_to(asio::buffer(cmd.to_string()), rxEndpoint, 0, error);
            if (error) {
                logWarning(fmt::format("transmit failure sending control message to receiver  {}",
                                       error.message()));
            }
        } else {
            auto rt_find = routes.find(rid);
            if (rt_find != routes.end()) {
                transmitSocket.send_to(asio::buffer(cmd.to_string()), rt_find->second, 0, error);
                if (error) {
                    logWarning(fmt::format("transmit failure sending to route {}:{}",
                                           rid.baseValue(),
                                           error.message()));
                }
            } else {
                if (hasBroker) {
                    transmitSocket.send_to(asio::buffer(cmd.to_string()),
                                           broker_endpoint,
                                           0,
                                           error);
                    if (error) {
                        logWarning(
                            fmt::format("transmit failure sending to broker  {}", error.message()));
                    }
                } else {
                    if (!isDisconnectCommand(cmd)) {
                        logWarning(std::string("(udp) unknown route, message dropped ") +
                                   prettyPrintString(cmd));
                    }
                }
            }
        }
    }
    routes.clear();
    if (getRxStatus() == ConnectionStatus::CONNECTED) {
        if (closingRx) {
            if (!(rxTrigger.wait_for(std::chrono::milliseconds(3000)))) {
                std::string cls("close");
                transmitSocket.send_to(asio::buffer(cls), rxEndpoint, 0, error);
                if (error) {
                    logWarning(fmt::format("transmit failure sending close to receiver III:{}",
                                           error.message()));
                } else {
                    if (!(rxTrigger.wait_for(std::chrono::milliseconds(2000)))) {
                        logWarning("udp rx not responding to close signal");
                    }
                }
            }
        } else {
            std::string cls("close");
            transmitSocket.send_to(asio::buffer(cls), rxEndpoint, 0, error);
            if (error) {
                logWarning(fmt::format("transmit failure sending close to receiver II:{}",
                                       error.message()));
            }
        }
    }

    setTxStatus(ConnectionStatus::TERMINATED);
}

void UdpComms::closeReceiver()
{
    if (getTxStatus() == ConnectionStatus::CONNECTED) {
        ActionMessage cmd(CMD_PROTOCOL);
        cmd.messageID = CLOSE_RECEIVER;
        transmit(control_route, cmd);
    } else if (!disconnecting) {
        try {
            auto serv = gmlc::networking::AsioContextManager::getContextPointer();
            if (serv) {
                udp::endpoint rxEndpoint;

                if (localTargetAddress.empty() || localTargetAddress == "*" ||
                    localTargetAddress == "udp://*") {
                    // try connecting with the receiver socket
                    udp::resolver resolver(serv->getBaseContext());
                    const udp::resolver::query queryLocal(udpnet(interfaceNetwork),
                                                          "127.0.0.1",
                                                          std::to_string(PortNumber));
                    rxEndpoint = *resolver.resolve(queryLocal);
                } else {
                    // try connecting with the receiver socket
                    udp::resolver resolver(serv->getBaseContext());
                    const udp::resolver::query queryLocal(udpnet(interfaceNetwork),
                                                          localTargetAddress,
                                                          std::to_string(PortNumber));
                    rxEndpoint = *resolver.resolve(queryLocal);
                }

                udp::socket transmitter(serv->getBaseContext(),
                                        udp::endpoint(udpnet(interfaceNetwork), 0));
                std::string cls("close");
                std::error_code error;
                transmitter.send_to(asio::buffer(cls), rxEndpoint, 0, error);
                if (error) {
                    logWarning(fmt::format("transmit failure on disconnect:{}", error.message()));
                }
            }
        }
        // NOLINTNEXTLINE
        catch (...) {
            // ignore error here since we are already disconnecting
        }
    }
}
}  // namespace helics::udp
