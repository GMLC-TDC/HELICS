/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "NetworkCommsInterface.hpp"

#include "NetworkBrokerData.hpp"
#include "helics/core/ActionMessage.hpp"
#include "helics/helics-config.h"
#ifndef HELICS_ENABLE_ENCRYPTION
#    include <iostream>
#endif
#include <fmt/format.h>
#include <memory>
#include <string>

namespace helics {
static constexpr char localHostString[] = "localhost";

NetworkCommsInterface::NetworkCommsInterface(gmlc::networking::InterfaceTypes type,
                                             CommsInterface::thread_generation threads) noexcept:
    CommsInterface(threads), networkType(type)
{
}

NetworkCommsInterface::PortAllocator::PortAllocator()
{
    addNewHost(localHostString);
}

int NetworkCommsInterface::PortAllocator::findOpenPort(int count, std::string_view host)
{
    if ((host == "127.0.0.1") || (host == "::1")) {
        return findOpenPort(count, localHostString);
    }
    auto np = nextPorts.find(host);
    int nextPort = startingPort;
    if (np == nextPorts.end()) {
        host = addNewHost(host);
        nextPorts[host] = startingPort;
        nextPorts[host] += count;
    } else {
        nextPort = np->second;
        (np->second) += count;
    }
    if (isPortUsed(host, nextPort)) {
        ++nextPort;
        while (isPortUsed(host, nextPort)) {
            ++nextPort;
        }
        nextPorts[host] = nextPort + count;
    }
    for (int ii = 0; ii < count; ++ii) {
        addUsedPort(host, nextPort + ii);
    }
    return nextPort;
}

void NetworkCommsInterface::PortAllocator::addUsedPort(int port)
{
    usedPort[localHostString].insert(port);
}

void NetworkCommsInterface::PortAllocator::addUsedPort(std::string_view host, int port)
{
    auto hst = usedPort.find(host);
    if (hst != usedPort.end()) {
        hst->second.insert(port);
    } else {
        host = addNewHost(host);
    }
    usedPort[host].insert(port);
}
/*
    int startingPort;
    std::map<std::string, std::set<int>> usedPort;
    std::map<std::string, int> nextPorts;
    */
bool NetworkCommsInterface::PortAllocator::isPortUsed(std::string_view host, int port) const
{
    auto fnd = usedPort.find(host);
    if (fnd == usedPort.end()) {
        return false;
    }
    return (fnd->second.count(port) != 0);
}

std::string_view NetworkCommsInterface::PortAllocator::addNewHost(std::string_view host)
{
    auto [it, emplaced] = hosts.emplace(host);
    return std::string_view(*it);
}
/** load network information into the comms object*/
void NetworkCommsInterface::loadNetworkInfo(const NetworkBrokerData& netInfo)
{
    using gmlc::networking::InterfaceTypes;
    using gmlc::networking::removeProtocol;

    CommsInterface::loadNetworkInfo(netInfo);
    if (!propertyLock()) {
        return;
    }
    brokerPort = netInfo.brokerPort;
    PortNumber = netInfo.portNumber;
    maxRetries = netInfo.maxRetries;
    switch (networkType) {
        case InterfaceTypes::TCP:
        case InterfaceTypes::UDP:
            removeProtocol(brokerTargetAddress);
            removeProtocol(localTargetAddress);
            break;
        default:
            break;
    }
    if (localTargetAddress.empty()) {
        auto bTarget = gmlc::networking::stripProtocol(brokerTargetAddress);
        if ((bTarget == localHostString) || (bTarget == "127.0.0.1")) {
            localTargetAddress = localHostString;
        } else if (bTarget.empty()) {
            switch (interfaceNetwork) {
                case gmlc::networking::InterfaceNetworks::LOCAL:
                    localTargetAddress = localHostString;
                    break;
                default:
                    localTargetAddress.assign(1, '*');
                    break;
            }
        } else {
            localTargetAddress =
                generateMatchingInterfaceAddress(brokerTargetAddress, interfaceNetwork);
        }
    }

    if (netInfo.portStart > 0) {
        openPorts.setStartingPortNumber(netInfo.portStart);
    }

    if (mRequireBrokerConnection) {
        if (brokerPort < 0 && netInfo.connectionPort >= 0) {
            brokerPort = netInfo.connectionPort;
        }
    } else {
        if (PortNumber < 0 && netInfo.connectionPort >= 0) {
            PortNumber = netInfo.connectionPort;
        }
    }

    if (PortNumber > 0) {
        autoPortNumber = false;
    }
    useOsPortAllocation = netInfo.use_os_port;
    appendNameToAddress = netInfo.appendNameToAddress;
    noAckConnection = netInfo.noAckConnection;
    useJsonSerialization = netInfo.useJsonSerialization;
    encrypted = netInfo.encrypted;
    forceConnection = netInfo.forceConnection;
#ifndef HELICS_ENABLE_ENCRYPTION
    if (encrypted) {
        std::cerr
            << "encryption not enabled in HELICS, recompile with encryption enabled if required"
            << std::endl;
    }
#endif
    propertyUnLock();
}

void NetworkCommsInterface::setBrokerPort(int brokerPortNumber)
{
    if (propertyLock()) {
        brokerPort = brokerPortNumber;
        propertyUnLock();
    }
}

int NetworkCommsInterface::findOpenPort(int count, std::string_view host)
{
    if (openPorts.getDefaultStartingPort() < 0) {
        auto dport = PortNumber - getDefaultBrokerPort();
        auto start = (dport < 10 * count && dport >= 0) ?
            getDefaultBrokerPort() + 10 * count * (dport + 1) :
            PortNumber + 5 * count;
        openPorts.setStartingPortNumber(start);
    }
    return openPorts.findOpenPort(count, std::string(host));
}

void NetworkCommsInterface::setPortNumber(int localPortNumber)
{
    if (propertyLock()) {
        PortNumber = localPortNumber;
        if (PortNumber > 0) {
            autoPortNumber = false;
        }
        propertyUnLock();
    }
}

void NetworkCommsInterface::setAutomaticPortStartPort(int startingPort)
{
    if (propertyLock()) {
        openPorts.setStartingPortNumber(startingPort);
        propertyUnLock();
    }
}

void NetworkCommsInterface::setFlag(std::string_view flag, bool val)
{
    if (flag == "os_port") {
        if (propertyLock()) {
            useOsPortAllocation = val;
            propertyUnLock();
        }
    } else if (flag == "noack_connect") {
        if (propertyLock()) {
            noAckConnection = val;
            propertyUnLock();
        }
    } else {
        CommsInterface::setFlag(flag, val);
    }
}

ActionMessage NetworkCommsInterface::generateReplyToIncomingMessage(ActionMessage& cmd)
{
    if (isProtocolCommand(cmd)) {
        switch (cmd.messageID) {
            case QUERY_PORTS: {
                ActionMessage portReply(CMD_PROTOCOL);
                portReply.messageID = PORT_DEFINITIONS;
                portReply.setExtraData(PortNumber);
                return portReply;
            } break;
            case REQUEST_PORTS: {
                int cnt = (cmd.counter == 0) ? 2 : cmd.counter;
                auto openPort = (cmd.name().empty()) ? findOpenPort(cnt, localHostString) :
                                                       findOpenPort(cnt, std::string(cmd.name()));
                ActionMessage portReply(CMD_PROTOCOL);
                portReply.messageID = PORT_DEFINITIONS;
                portReply.source_id = GlobalFederateId(PortNumber);
                portReply.setExtraData(openPort);
                portReply.counter = cmd.counter;
                return portReply;
            } break;
            case CONNECTION_REQUEST: {
                ActionMessage connAck(CMD_PROTOCOL);
                connAck.messageID = CONNECTION_ACK;
                return connAck;
            } break;
            default:
                break;
        }
    }
    ActionMessage resp(CMD_IGNORE);
    return resp;
}

std::string NetworkCommsInterface::getAddress() const
{
    using gmlc::networking::makePortAddress;

    if ((PortNumber < 0) && (!serverMode)) {
        return name;
    }
    std::string address;
    if ((localTargetAddress == "tcp://*") || (localTargetAddress == "tcp://0.0.0.0")) {
        address = makePortAddress("tcp://127.0.0.1", PortNumber);
    } else if ((localTargetAddress == "*") || (localTargetAddress == "0.0.0.0")) {
        address = makePortAddress("127.0.0.1", PortNumber);
    } else {
        address = makePortAddress(localTargetAddress, PortNumber);
    }
    if (appendNameToAddress) {
        address.push_back('/');
        address.append(name);
    }
    return address;
}

ActionMessage NetworkCommsInterface::generatePortRequest(int cnt) const
{
    ActionMessage req(CMD_PROTOCOL);
    req.messageID = REQUEST_PORTS;
    req.payload = gmlc::networking::stripProtocol(localTargetAddress);
    req.counter = cnt;
    req.setStringData(brokerName, brokerInitString);
    return req;
}

void NetworkCommsInterface::loadPortDefinitions(const ActionMessage& cmd)
{
    if (cmd.action() == CMD_PROTOCOL) {
        if (cmd.messageID == PORT_DEFINITIONS) {
            PortNumber = cmd.getExtraData();
            if ((openPorts.getDefaultStartingPort() < 0)) {
                if (PortNumber < getDefaultBrokerPort() + 100) {
                    openPorts.setStartingPortNumber(getDefaultBrokerPort() + 100 +
                                                    (PortNumber - getDefaultBrokerPort() - 2) * 6);
                } else {
                    openPorts.setStartingPortNumber(getDefaultBrokerPort() + 110 +
                                                    (PortNumber - getDefaultBrokerPort() - 100) *
                                                        6);
                }
            }
        }
    }
}

}  // namespace helics
