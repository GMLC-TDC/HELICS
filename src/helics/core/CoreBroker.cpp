/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "CoreBroker.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "../common/fmt_format.h"
#include "../common/logger.h"
#include "BrokerFactory.hpp"
#include "ForwardingTimeCoordinator.hpp"
#include "TimeoutMonitor.h"
#include "fileConnections.hpp"
#include "gmlc/utilities/stringConversion.h"
#include "helicsCLI11.hpp"
#include "helics_definitions.hpp"
#include "loggingHelper.hpp"
#include "queryHelpers.hpp"

#include <iostream>

namespace helics {
using namespace std::string_literals;

constexpr char universalKey[] = "**";

CoreBroker::~CoreBroker()
{
    std::lock_guard<std::mutex> lock(name_mutex_);
    // make sure everything is synchronized
}

void CoreBroker::setIdentifier(const std::string& name)
{
    if (brokerState <= broker_state_t::connecting) // can't be changed after initialization
    {
        std::lock_guard<std::mutex> lock(name_mutex_);
        identifier = name;
    }
}

const std::string& CoreBroker::getAddress() const
{
    if ((brokerState != broker_state_t::connected) || (address.empty())) {
        address = generateLocalAddressString();
    }
    return address;
}

route_id CoreBroker::getRoute(global_federate_id fedid) const
{
    if ((fedid == parent_broker_id) || (fedid == higher_broker_id)) {
        return parent_route_id;
    }
    auto fnd = routing_table.find(fedid);
    return (fnd != routing_table.end()) ? fnd->second :
                                          parent_route_id; // zero is the default route
}

BasicBrokerInfo* CoreBroker::getBrokerById(global_broker_id brokerid)
{
    if (isRootc) {
        auto brkNum = brokerid.localIndex();
        return (isValidIndex(brkNum, _brokers) ? &_brokers[brkNum] : nullptr);
    }

    auto fnd = _brokers.find(brokerid);
    return (fnd != _brokers.end()) ? &(*fnd) : nullptr;
}

const BasicBrokerInfo* CoreBroker::getBrokerById(global_broker_id brokerid) const
{
    if (isRootc) {
        auto brkNum = brokerid.localIndex();
        return (isValidIndex(brkNum, _brokers)) ? &_brokers[brkNum] : nullptr;
    }

    auto fnd = _brokers.find(brokerid);
    return (fnd != _brokers.end()) ? &(*fnd) : nullptr;
}

void CoreBroker::setLoggingCallback(
    const std::function<void(int, const std::string&, const std::string&)>& logFunction)
{
    ActionMessage loggerUpdate(CMD_BROKER_CONFIGURE);
    loggerUpdate.messageID = UPDATE_LOGGING_CALLBACK;
    loggerUpdate.source_id = global_id.load();
    if (logFunction) {
        auto ii = getNextAirlockIndex();
        dataAirlocks[ii].load(logFunction);
        loggerUpdate.counter = ii;
    } else {
        setActionFlag(loggerUpdate, empty_flag);
    }

    actionQueue.push(loggerUpdate);
}

uint16_t CoreBroker::getNextAirlockIndex()
{
    uint16_t index = nextAirLock++;
    if (index >
        2) { // this is an atomic operation if the nextAirLock was not adjusted this could result in an out of bounds
        // exception if this check were not done
        index %= 2;
    }
    if (index == 2) {
        decltype(index) exp = 3;

        while (exp > 2) { // doing a lock free modulus we need to make sure the nextAirLock<4
            if (nextAirLock.compare_exchange_weak(exp, exp % 3)) {
                break;
            }
        }
    }
    return index;
}

bool CoreBroker::verifyBrokerKey(ActionMessage& mess) const
{
    if (mess.getStringData().size() > 1) {
        return verifyBrokerKey(mess.getString(1));
    }
    return brokerKey.empty();
}
/** verify the broker key contained in a string
@return false if the keys do not match*/
bool CoreBroker::verifyBrokerKey(const std::string& key) const
{
    return (key == brokerKey || brokerKey == universalKey);
}

void CoreBroker::makeConnections(const std::string& file)
{
    if (hasTomlExtension(file)) {
        makeConnectionsToml(this, file);
    } else {
        makeConnectionsJson(this, file);
    }
}

void CoreBroker::dataLink(const std::string& publication, const std::string& input)
{
    ActionMessage M(CMD_DATA_LINK);
    M.name = publication;
    M.setStringData(input);
    addActionMessage(std::move(M));
}

void CoreBroker::addSourceFilterToEndpoint(const std::string& filter, const std::string& endpoint)
{
    ActionMessage M(CMD_FILTER_LINK);
    M.name = filter;
    M.setStringData(endpoint);
    addActionMessage(std::move(M));
}

void CoreBroker::addDestinationFilterToEndpoint(
    const std::string& filter,
    const std::string& endpoint)
{
    ActionMessage M(CMD_FILTER_LINK);
    M.name = filter;
    M.setStringData(endpoint);
    setActionFlag(M, destination_target);
    addActionMessage(std::move(M));
}

route_id CoreBroker::fillMessageRouteInformation(ActionMessage& mess)
{
    auto& endpointName = mess.getString(targetStringLoc);
    auto eptInfo = handles.getEndpoint(endpointName);
    if (eptInfo != nullptr) {
        mess.setDestination(eptInfo->handle);
        return getRoute(eptInfo->handle.fed_id);
    }
    auto fnd2 = knownExternalEndpoints.find(endpointName);
    if (fnd2 != knownExternalEndpoints.end()) {
        return fnd2->second;
    }
    return parent_route_id;
}

bool CoreBroker::isOpenToNewFederates() const
{
    auto cstate = brokerState.load();
    return (
        (cstate != broker_state_t::created) && (cstate < broker_state_t::operating) &&
        (!haltOperations));
}

void CoreBroker::processPriorityCommand(ActionMessage&& command)
{
    // deal with a few types of message immediately
    LOG_TRACE(
        global_broker_id_local,
        getIdentifier(),
        fmt::format(
            "|| priority_cmd:{} from {}",
            prettyPrintString(command),
            command.source_id.baseValue()));
    switch (command.action()) {
        case CMD_PING_PRIORITY:
            if (command.dest_id == global_broker_id_local) {
                ActionMessage pngrep(CMD_PING_REPLY);
                pngrep.dest_id = command.source_id;
                pngrep.source_id = global_broker_id_local;
                routeMessage(pngrep);
            } else {
                routeMessage(command);
            }
            break;
        case CMD_BROKER_SETUP: {
            global_broker_id_local = global_id.load();
            isRootc = _isRoot.load();
            timeCoord->source_id = global_broker_id_local;
            connectionEstablished = true;
            if (!earlyMessages.empty()) {
                for (auto& M : earlyMessages) {
                    if (isPriorityCommand(M)) {
                        processPriorityCommand(std::move(M));
                    } else {
                        processCommand(std::move(M));
                    }
                }
                earlyMessages.clear();
            }
            break;
        }
        case CMD_REG_FED: {
            if (!connectionEstablished) {
                earlyMessages.push_back(std::move(command));
                break;
            }
            if (brokerState != broker_state_t::operating) {
                if (allInitReady()) {
                    ActionMessage noInit(CMD_INIT_NOT_READY);
                    noInit.source_id = global_broker_id_local;
                    transmit(parent_route_id, noInit);
                }
            } else // we are initialized already
            {
                ActionMessage badInit(CMD_FED_ACK);
                setActionFlag(badInit, error_flag);
                badInit.source_id = global_broker_id_local;
                badInit.messageID = 5;
                badInit.name = command.name;
                transmit(getRoute(command.source_id), badInit);
                return;
            }
            // this checks for duplicate federate names
            if (_federates.find(command.name) != _federates.end()) {
                ActionMessage badName(CMD_FED_ACK);
                setActionFlag(badName, error_flag);
                badName.source_id = global_broker_id_local;
                badName.messageID = 6;
                badName.name = command.name;
                transmit(getRoute(command.source_id), badName);
                return;
            }
            _federates.insert(command.name, no_search, command.name);
            _federates.back().route = getRoute(command.source_id);
            _federates.back().parent = command.source_id;
            if (!isRootc) {
                if (global_broker_id_local.isValid()) {
                    command.source_id = global_broker_id_local;
                    transmit(parent_route_id, command);
                } else {
                    // delay the response if we are not fully registered yet
                    delayTransmitQueue.push(command);
                }
            } else {
                _federates.back().global_id = global_federate_id(
                    static_cast<global_federate_id::base_type>(_federates.size()) - 1 +
                    global_federate_id_shift);
                _federates.addSearchTermForIndex(
                    _federates.back().global_id,
                    static_cast<size_t>(_federates.back().global_id.baseValue()) -
                        global_federate_id_shift);
                auto route_id = _federates.back().route;
                auto global_fedid = _federates.back().global_id;

                routing_table.emplace(global_fedid, route_id);
                // don't bother with the federate_table
                // transmit the response
                ActionMessage fedReply(CMD_FED_ACK);
                fedReply.source_id = global_broker_id_local;
                fedReply.dest_id = global_fedid;
                fedReply.name = command.name;
                transmit(route_id, fedReply);
                LOG_CONNECTIONS(
                    global_broker_id_local,
                    getIdentifier(),
                    fmt::format(
                        "registering federate {}({}) on route {}",
                        command.name,
                        global_fedid.baseValue(),
                        route_id.baseValue()));
            }
        } break;
        case CMD_REG_BROKER: {
            if (!connectionEstablished) {
                earlyMessages.push_back(std::move(command));
                break;
            }
            if (command.counter > 0) { // this indicates it is a resend
                auto brk = _brokers.find(command.name);
                if (brk != _brokers.end()) {
                    // we would get this if the ack didn't go through for some reason
                    brk->route = route_id{routeCount++};
                    addRoute(brk->route, command.getString(targetStringLoc));
                    routing_table[brk->global_id] = brk->route;

                    // sending the response message
                    ActionMessage brokerReply(CMD_BROKER_ACK);
                    brokerReply.source_id = global_broker_id_local; // source is global root
                    brokerReply.dest_id = brk->global_id; // the new id
                    brokerReply.name = command.name; // the identifier of the broker
                    if (no_ping) {
                        setActionFlag(brokerReply, slow_responding_flag);
                    }
                    transmit(brk->route, brokerReply);
                    return;
                }
            }
            if (brokerState != broker_state_t::operating) {
                if (allInitReady()) {
                    // send an init not ready as we were ready now we are not
                    ActionMessage noInit(CMD_INIT_NOT_READY);
                    noInit.source_id = global_broker_id_local;
                    transmit(parent_route_id, noInit);
                }
            } else // we are initialized already
            {
                route_id newroute;
                bool route_created = false;
                if ((!command.source_id.isValid()) || (command.source_id == parent_broker_id)) {
                    newroute = route_id(routeCount++);
                    addRoute(newroute, command.getString(targetStringLoc));
                    route_created = true;
                } else {
                    newroute = getRoute(command.source_id);
                }
                ActionMessage badInit(CMD_BROKER_ACK);
                setActionFlag(badInit, error_flag);
                badInit.source_id = global_broker_id_local;
                badInit.name = command.name;
                badInit.messageID = 5;
                transmit(newroute, badInit);

                if (route_created) {
                    removeRoute(newroute);
                }
                return;
            }
            if (!verifyBrokerKey(command)) {
                route_id newroute;
                bool route_created = false;
                if ((!command.source_id.isValid()) || (command.source_id == parent_broker_id)) {
                    newroute = route_id{routeCount++};
                    addRoute(newroute, command.getString(targetStringLoc));
                    route_created = true;
                } else {
                    newroute = getRoute(command.source_id);
                }
                ActionMessage badKey(CMD_BROKER_ACK);
                setActionFlag(badKey, error_flag);
                badKey.source_id = global_broker_id_local;
                badKey.messageID = mismatch_broker_key_error_code;
                badKey.name = command.name;
                badKey.setString(0, "broker key does not match");
                transmit(newroute, badKey);
                if (route_created) {
                    removeRoute(newroute);
                }
                return;
            }
            auto inserted = _brokers.insert(command.name, no_search, command.name);
            if (!inserted) {
                route_id newroute;
                bool route_created = false;
                if ((!command.source_id.isValid()) || (command.source_id == parent_broker_id)) {
                    newroute = route_id{routeCount++};
                    addRoute(newroute, command.getString(targetStringLoc));
                    route_created = true;
                } else {
                    newroute = getRoute(command.source_id);
                }
                ActionMessage badName(CMD_BROKER_ACK);
                setActionFlag(badName, error_flag);
                badName.source_id = global_broker_id_local;
                badName.messageID = duplicate_broker_name_error_code;
                badName.name = command.name;
                transmit(newroute, badName);
                if (route_created) {
                    removeRoute(newroute);
                }
                return;
            }
            if ((!command.source_id.isValid()) || (command.source_id == parent_broker_id)) {
                // TODO PT:: this will need to be updated when we enable mesh routing
                _brokers.back().route = route_id{routeCount++};
                addRoute(_brokers.back().route, command.getString(targetStringLoc));
                _brokers.back().parent = global_broker_id_local;
                _brokers.back()._nonLocal = false;
                _brokers.back()._route_key = true;
            } else {
                _brokers.back().route = getRoute(command.source_id);
                if (_brokers.back().route == parent_route_id) {
                    std::cout << " invalid route to parent broker or reg broker" << std::endl;
                }
                _brokers.back().parent = command.source_id;
                _brokers.back()._nonLocal = true;
            }
            _brokers.back()._core = checkActionFlag(command, core_flag);
            if (!isRootc) {
                if ((global_broker_id_local.isValid()) &&
                    (global_broker_id_local != parent_broker_id)) {
                    command.source_id = global_broker_id_local;
                    transmit(parent_route_id, command);
                } else {
                    // delay the response if we are not fully registered yet
                    delayTransmitQueue.push(command);
                }
            } else {
                _brokers.back().global_id = global_broker_id(
                    static_cast<global_broker_id::base_type>(_brokers.size()) - 1 +
                    global_broker_id_shift);
                _brokers.addSearchTermForIndex(_brokers.back().global_id, _brokers.size() - 1);
                auto global_brkid = _brokers.back().global_id;
                auto route = _brokers.back().route;
                if (checkActionFlag(command, slow_responding_flag)) {
                    _brokers.back()._disable_ping = true;
                }
                routing_table.emplace(global_brkid, route);
                // don't bother with the broker_table for root broker

                // sending the response message
                ActionMessage brokerReply(CMD_BROKER_ACK);
                brokerReply.source_id = global_broker_id_local; // source is global root
                brokerReply.dest_id = global_brkid; // the new id
                brokerReply.name = command.name; // the identifier of the broker
                if (no_ping) {
                    setActionFlag(brokerReply, slow_responding_flag);
                }
                transmit(route, brokerReply);
                LOG_CONNECTIONS(
                    global_broker_id_local,
                    getIdentifier(),
                    fmt::format(
                        "registering broker {}({}) on route {}",
                        command.name,
                        global_brkid.baseValue(),
                        route.baseValue()));
            }
        } break;
        case CMD_FED_ACK: { // we can't be root if we got one of these
            auto fed = _federates.find(command.name);
            if (fed != _federates.end()) {
                fed->global_id = command.dest_id;
                auto route = fed->route;
                _federates.addSearchTerm(command.dest_id, fed->name);
                transmit(route, command);
                routing_table.emplace(fed->global_id, route);
            } else {
                // this means we haven't seen this federate before for some reason
                _federates.insert(command.name, command.dest_id, command.name);
                _federates.back().route = getRoute(command.source_id);
                _federates.back().global_id = command.dest_id;
                routing_table.emplace(fed->global_id, _federates.back().route);
                // it also means we don't forward it
            }
        } break;
        case CMD_BROKER_ACK: { // we can't be root if we got one of these
            if (command.name == identifier) {
                if (checkActionFlag(command, error_flag)) {
                    // generate an error message
                    LOG_ERROR(
                        global_broker_id_local,
                        identifier,
                        fmt::format("unable to register broker {}", command.payload));
                    return;
                }

                global_broker_id_local = command.dest_id;
                global_id.store(global_broker_id_local);
                higher_broker_id = command.source_id;
                timeCoord->source_id = global_broker_id_local;
                transmitDelayedMessages();
                _brokers.apply([localid = global_broker_id_local](auto& brk) {
                    if (!brk._nonLocal) {
                        brk.parent = localid;
                    }
                });

                timeoutMon->setParentId(higher_broker_id);
                if (checkActionFlag(command, slow_responding_flag)) {
                    timeoutMon->disableParentPing();
                }
                timeoutMon->reset();
                return;
            }
            auto broker = _brokers.find(command.name);
            if (broker != _brokers.end()) {
                if (broker->global_id == global_broker_id(command.dest_id)) {
                    // drop the packet since we have seen this ack already
                    LOG_WARNING(global_broker_id_local, identifier, "repeated broker acks");
                    return;
                }
                broker->global_id = global_broker_id(command.dest_id);
                auto route = broker->route;
                _brokers.addSearchTerm(global_broker_id(command.dest_id), broker->name);
                routing_table.emplace(broker->global_id, route);
                command.source_id =
                    global_broker_id_local; // we want the intermediate broker to change the source_id
                transmit(route, command);
            } else {
                _brokers.insert(command.name, global_broker_id(command.dest_id), command.name);
                _brokers.back().route = getRoute(command.source_id);
                _brokers.back().global_id = global_broker_id(command.dest_id);
                routing_table.emplace(broker->global_id, _brokers.back().route);
            }
        } break;
        case CMD_PRIORITY_DISCONNECT: {
            auto brk = getBrokerById(global_broker_id(command.source_id));
            if (brk != nullptr) {
                brk->isDisconnected = true;
            }
            if (allDisconnected()) {
                if (!isRootc) {
                    ActionMessage dis(CMD_PRIORITY_DISCONNECT);
                    dis.source_id = global_broker_id_local;
                    transmit(parent_route_id, dis);
                }
                addActionMessage(CMD_STOP);
            }
        } break;
        case CMD_REG_ROUTE:
            break;
        case CMD_BROKER_QUERY:
            if (!connectionEstablished) {
                earlyMessages.push_back(std::move(command));
                break;
            }
            if (command.dest_id == global_broker_id_local) {
                processLocalQuery(command);
            } else if ((isRootc) && (command.dest_id == parent_broker_id)) {
                processLocalQuery(command);
            } else {
                routeMessage(command);
            }
            break;
        case CMD_QUERY:
            processQuery(command);
            break;
        case CMD_QUERY_REPLY:
            if (command.dest_id == global_broker_id_local) {
                processQueryResponse(command);
            } else {
                transmit(getRoute(command.dest_id), command);
            }
            break;
        case CMD_SET_GLOBAL:
            if (isRootc) {
                global_values[command.name] = command.getString(0);
            } else {
                if ((global_broker_id_local.isValid()) &&
                    (global_broker_id_local != parent_broker_id)) {
                    transmit(parent_route_id, command);
                } else {
                    // delay the response if we are not fully registered yet
                    delayTransmitQueue.push(command);
                }
            }
        default:
            // must not have been a priority command
            break;
    }
}

std::string CoreBroker::generateFederationSummary() const
{
    int pubs = 0;
    int epts = 0;
    int ipts = 0;
    int filt = 0;
    for (auto& hand : handles) {
        switch (hand.handleType) {
            case handle_type::publication:
                ++pubs;
                break;
            case handle_type::input:
                ++ipts;
                break;
            case handle_type::endpoint:
                ++epts;
                break;
            default:
                ++filt;
                break;
        }
    }
    std::string output = fmt::format(
        "Federation Summary> \n\t{} federates [min {}]\n\t{}/{} brokers/cores [min {}]\n\t{} "
        "publications\n\t{} inputs\n\t{} endpoints\n\t{} filters\n<<<<<<<<<",
        _federates.size(),
        minFederateCount,
        std::count_if(
            _brokers.begin(), _brokers.end(), [](auto& brk) { return brk._core == false; }),
        std::count_if(
            _brokers.begin(), _brokers.end(), [](auto& brk) { return brk._core == true; }),
        minBrokerCount,
        pubs,
        ipts,
        epts,
        filt);
    return output;
}

void CoreBroker::transmitDelayedMessages()
{
    auto msg = delayTransmitQueue.pop();
    while (msg) {
        msg->source_id = global_broker_id_local;
        transmit(parent_route_id, *msg);
        msg = delayTransmitQueue.pop();
    }
}

void CoreBroker::labelAsDisconnected(global_broker_id brkid)
{
    auto disconnect_procedure = [brkid](auto& obj) {
        if (obj.parent == brkid) {
            obj.isDisconnected = true;
        }
    };
    _brokers.apply(disconnect_procedure);
    _federates.apply(disconnect_procedure);
}

void CoreBroker::sendDisconnect()
{
    ActionMessage bye(CMD_DISCONNECT);
    bye.source_id = global_broker_id_local;
    _brokers.apply([this, &bye](auto& brk) {
        if (!brk.isDisconnected) {
            if (brk.parent == global_broker_id_local) {
                this->routeMessage(bye, brk.global_id);
                brk.isDisconnected = true;
            }
            if (hasTimeDependency) {
                timeCoord->removeDependency(brk.global_id);
                timeCoord->removeDependent(brk.global_id);
            }
        }
    });
    if (hasTimeDependency) {
        timeCoord->disconnect();
    }
}

void CoreBroker::sendErrorToImmediateBrokers(int error_code)
{
    ActionMessage errorCom(CMD_ERROR);
    errorCom.messageID = error_code;
    broadcast(errorCom);
}

void CoreBroker::processCommand(ActionMessage&& command)
{
    LOG_TRACE(
        global_broker_id_local,
        getIdentifier(),
        fmt::format(
            "|| cmd:{} from {} to {}",
            prettyPrintString(command),
            command.source_id.baseValue(),
            command.dest_id.baseValue()));
    switch (command.action()) {
        case CMD_IGNORE:
        case CMD_PROTOCOL:
            break;

        case CMD_TICK:
            if (brokerState == broker_state_t::operating) {
                timeoutMon->tick(this);
                LOG_SUMMARY(global_broker_id_local, getIdentifier(), " broker tick");
            }
            break;
        case CMD_PING:
            if (command.dest_id == global_broker_id_local) {
                ActionMessage pngrep(CMD_PING_REPLY);
                pngrep.dest_id = command.source_id;
                pngrep.source_id = global_broker_id_local;
                routeMessage(pngrep);
            } else {
                routeMessage(command);
            }
            break;
        case CMD_BROKER_PING:
            if (command.dest_id == global_broker_id_local) {
                ActionMessage pngrep(CMD_PING_REPLY);
                pngrep.dest_id = command.source_id;
                pngrep.source_id = global_broker_id_local;
                routeMessage(pngrep);
                timeoutMon->pingSub(this);
            } else {
                routeMessage(command);
            }
            break;
        case CMD_PING_REPLY:
            if (command.dest_id == global_broker_id_local) {
                timeoutMon->pingReply(command, this);
            } else {
                routeMessage(command);
            }
            break;
        case CMD_CHECK_CONNECTIONS:
            sendDisconnect();
            addActionMessage(CMD_STOP);
            LOG_WARNING(
                global_broker_id_local, getIdentifier(), "disconnecting from check connections");
            break;
        case CMD_CONNECTION_ERROR:
            // if anyone else as has terminated assume they finalized and the connection was lost
            if (command.dest_id == global_broker_id_local) {
                bool partDisconnected{false};
                bool ignore{false};
                for (auto& brk : _brokers) {
                    if (brk.isDisconnected) {
                        partDisconnected = true;
                    }
                    if (brk.isDisconnected && brk.global_id == command.source_id) {
                        // the broker in question is already disconnected, ignore this
                        ignore = true;
                        break;
                    }
                }
                if (ignore) {
                    break;
                }
                if (partDisconnected) { // we are going to assume it disconnected just assume broker even though it may be a core, there
                    // probably isn't any difference for this purpose
                    LOG_CONNECTIONS(
                        global_broker_id_local,
                        getIdentifier(),
                        fmt::format(
                            "disconnecting {} from communication timeout",
                            command.source_id.baseValue()));
                    command.setAction(CMD_DISCONNECT);
                    command.dest_id = parent_broker_id;
                    setActionFlag(command, error_flag);
                    processDisconnect(command);
                } else {
                    if (isRootc) {
                        std::string lcom =
                            fmt::format("lost comms with {}", command.source_id.baseValue());
                        LOG_ERROR(global_broker_id_local, getIdentifier(), lcom);
                        ActionMessage elink(CMD_ERROR);
                        elink.payload = lcom;
                        elink.messageID = defs::errors::connection_failure;
                        broadcast(elink);
                        brokerState = broker_state_t::errored;
                        addActionMessage(
                            CMD_USER_DISCONNECT); // TODO::PT this needs something better but this does
                        // what is needed for now
                    } else {
                        // pass it up the chain let the root deal with it
                        command.dest_id = parent_broker_id;
                        transmit(parent_route_id, command);
                    }
                }
            } else {
                routeMessage(command);
            }
            break;
        case CMD_INIT: {
            auto brk = getBrokerById(static_cast<global_broker_id>(command.source_id));
            if (brk != nullptr) {
                brk->_initRequested = true;
            }
            if (allInitReady()) {
                if (isRootc) {
                    LOG_TIMING(global_broker_id_local, "root", "entering initialization mode");
                    LOG_SUMMARY(global_broker_id_local, "root", generateFederationSummary());
                    executeInitializationOperations();
                } else {
                    LOG_TIMING(
                        global_broker_id_local, getIdentifier(), "entering initialization mode");
                    checkDependencies();
                    command.source_id = global_broker_id_local;
                    transmit(parent_route_id, command);
                }
            }
        } break;
        case CMD_INIT_NOT_READY: {
            if (allInitReady()) {
                transmit(parent_route_id, command);
            }
            auto brk = getBrokerById(global_broker_id(command.source_id));
            if (brk != nullptr) {
                brk->_initRequested = false;
            }
        } break;
        case CMD_INIT_GRANT:
            if (brokerKey == universalKey) {
                LOG_SUMMARY(
                    global_broker_id_local, getIdentifier(), " Broker started with universal key");
            }
            brokerState = broker_state_t::operating;
            for (auto& brk : _brokers) {
                transmit(brk.route, command);
            }
            {
                timeCoord->enteringExecMode();
                auto res = timeCoord->checkExecEntry();
                if (res == message_processing_result::next_step) {
                    enteredExecutionMode = true;
                }
            }
            break;
        case CMD_SEARCH_DEPENDENCY: {
            auto fed = _federates.find(command.name);
            if (fed != _federates.end()) {
                if (fed->global_id.isValid()) {
                    ActionMessage dep(CMD_ADD_DEPENDENCY, fed->global_id, command.source_id);
                    routeMessage(dep);
                    dep = ActionMessage(CMD_ADD_DEPENDENT, command.source_id, fed->global_id);
                    routeMessage(dep);
                    break;
                }
            }
            if (isRootc) {
                delayedDependencies.emplace_back(command.name, command.source_id);
            } else {
                routeMessage(command);
            }
            break;
        }
        case CMD_DATA_LINK: {
            auto pub = handles.getPublication(command.name);
            if (pub != nullptr) {
                command.name = command.getString(targetStringLoc);
                command.setAction(CMD_ADD_NAMED_INPUT);
                command.setSource(pub->handle);
                checkForNamedInterface(command);
            } else {
                auto input = handles.getInput(command.getString(targetStringLoc));
                if (input == nullptr) {
                    if (isRootc) {
                        unknownHandles.addDataLink(
                            command.name, command.getString(targetStringLoc));
                    } else {
                        routeMessage(command);
                    }
                } else {
                    command.setAction(CMD_ADD_NAMED_PUBLICATION);
                    command.setSource(input->handle);
                    checkForNamedInterface(command);
                }
            }
        } break;
        case CMD_FILTER_LINK: {
            auto filt = handles.getFilter(command.name);
            if (filt != nullptr) {
                command.name = command.getString(targetStringLoc);
                command.setAction(CMD_ADD_NAMED_ENDPOINT);
                command.setSource(filt->handle);
                if (checkActionFlag(*filt, clone_flag)) {
                    setActionFlag(command, clone_flag);
                }
                checkForNamedInterface(command);
            } else {
                auto ept = handles.getEndpoint(command.getString(targetStringLoc));
                if (ept == nullptr) {
                    if (isRootc) {
                        if (checkActionFlag(command, destination_target)) {
                            unknownHandles.addDestinationFilterLink(
                                command.name, command.getString(targetStringLoc));
                        } else {
                            unknownHandles.addSourceFilterLink(
                                command.name, command.getString(targetStringLoc));
                        }
                    } else {
                        routeMessage(command);
                    }
                } else {
                    command.setAction(CMD_ADD_NAMED_FILTER);
                    command.setSource(ept->handle);
                    checkForNamedInterface(command);
                }
            }
        } break;
        case CMD_DISCONNECT_NAME:
            if (command.dest_id == parent_broker_id) {
                auto brk = _brokers.find(command.payload);
                if (brk != _brokers.end()) {
                    command.source_id = brk->global_id;
                }
            }
            FALLTHROUGH
            /* FALLTHROUGH */
        case CMD_DISCONNECT:
        case CMD_DISCONNECT_CORE:
        case CMD_DISCONNECT_BROKER:
            processDisconnect(command);
            break;
        case CMD_DISCONNECT_BROKER_ACK:
            if ((command.dest_id == global_broker_id_local) &&
                (command.source_id == higher_broker_id)) {
                _brokers.apply([this](auto& brk) {
                    if (!brk._sent_disconnect_ack) {
                        ActionMessage dis(
                            (brk._core) ? CMD_DISCONNECT_CORE_ACK : CMD_DISCONNECT_BROKER_ACK);
                        dis.source_id = global_broker_id_local;
                        dis.dest_id = brk.global_id;
                        this->transmit(brk.route, dis);
                        brk._sent_disconnect_ack = true;
                        this->removeRoute(brk.route);
                    }
                });
                addActionMessage(CMD_STOP);
            }
            break;
        case CMD_USER_DISCONNECT:
            sendDisconnect();
            addActionMessage(CMD_STOP);
            break;
        case CMD_DISCONNECT_FED: {
            auto fed = _federates.find(command.source_id);
            if (fed != _federates.end()) {
                fed->isDisconnected = true;
            }
            if (!isRootc) {
                transmit(parent_route_id, command);
            } else if (brokerState < broker_state_t::operating) {
                command.setAction(CMD_BROADCAST_DISCONNECT);
                broadcast(command);
                unknownHandles.clearFederateUnknowns(command.source_id);
            }
        } break;
        case CMD_STOP:
            if (!allDisconnected()) { // only send a disconnect message if we haven't done so already
                timeCoord->disconnect();
                if (!isRootc) {
                    ActionMessage m(CMD_DISCONNECT);
                    m.source_id = global_broker_id_local;
                    transmit(parent_route_id, m);
                }
            }
            ActiveQueries.fulfillAllPromises("#disconnected");
            break;
        case CMD_BROADCAST_DISCONNECT: {
            timeCoord->processTimeMessage(command);
            broadcast(command);
        } break;
        case CMD_EXEC_REQUEST:
        case CMD_EXEC_GRANT:
            if (command.dest_id == global_broker_id_local) {
                timeCoord->processTimeMessage(command);
                if (!enteredExecutionMode) {
                    auto res = timeCoord->checkExecEntry();
                    if (res == message_processing_result::next_step) {
                        enteredExecutionMode = true;
                        LOG_TIMING(global_broker_id_local, getIdentifier(), "entering Exec Mode");
                    }
                }
            } else if (command.source_id == global_broker_id_local) {
                for (auto dep : timeCoord->getDependents()) {
                    routeMessage(command, dep);
                }
            } else {
                transmit(getRoute(command.dest_id), command);
            }

            break;
        case CMD_TIME_REQUEST:
        case CMD_TIME_GRANT:
            if ((command.source_id == global_broker_id_local) &&
                (command.dest_id == parent_broker_id)) {
                LOG_TIMING(
                    global_broker_id_local,
                    getIdentifier(),
                    fmt::format("time request update {}", prettyPrintString(command)));
                for (auto dep : timeCoord->getDependents()) {
                    routeMessage(command, dep);
                }
            } else if (command.dest_id == global_broker_id_local) {
                if (timeCoord->processTimeMessage(command)) {
                    timeCoord->updateTimeFactors();
                }
            } else {
                routeMessage(command);
            }

            break;
        case CMD_SEND_MESSAGE:
        case CMD_SEND_FOR_FILTER:
        case CMD_SEND_FOR_FILTER_AND_RETURN:
        case CMD_FILTER_RESULT:
        case CMD_NULL_MESSAGE:
            if (command.dest_id == parent_broker_id) {
                auto route = fillMessageRouteInformation(command);
                transmit(route, command);
            } else {
                transmit(getRoute(command.dest_id), command);
            }
            break;
        case CMD_PUB:
            transmit(getRoute(command.dest_id), command);
            break;

        case CMD_LOG:
            if (isRootc) {
                sendToLogger(command.source_id, command.counter, std::string(), command.payload);
            } else {
                transmit(parent_route_id, command);
            }
            break;
        case CMD_ERROR:
            if (isRootc || command.dest_id == global_broker_id_local) {
                sendToLogger(command.source_id, log_level::error, std::string(), command.payload);
                if (command.source_id == parent_broker_id) {
                    broadcast(command);
                }
                brokerState = broker_state_t::errored;
            } else {
                transmit(parent_route_id, command);
            }
            break;
        case CMD_REG_PUB:
            if ((!isRootc) && (command.dest_id != parent_broker_id)) {
                routeMessage(command);
                break;
            }
            addPublication(command);
            break;
        case CMD_REG_INPUT:
            if ((!isRootc) && (command.dest_id != parent_broker_id)) {
                routeMessage(command);
                break;
            }
            addInput(command);
            break;
        case CMD_REG_ENDPOINT:
            if ((!isRootc) && (command.dest_id != parent_broker_id)) {
                routeMessage(command);
                break;
            }
            addEndpoint(command);
            break;
        case CMD_REG_FILTER:
            if ((!isRootc) && (command.dest_id != parent_broker_id)) {
                routeMessage(command);
                break;
            }
            addFilter(command);
            break;
        case CMD_CLOSE_INTERFACE:
            if ((!isRootc) && (command.dest_id != parent_broker_id)) {
                routeMessage(command);
                break;
            }
            handles.removeHandle(command.getSource());
            break;
        case CMD_ADD_DEPENDENCY:
        case CMD_REMOVE_DEPENDENCY:
        case CMD_ADD_DEPENDENT:
        case CMD_REMOVE_DEPENDENT:
        case CMD_ADD_INTERDEPENDENCY:
        case CMD_REMOVE_INTERDEPENDENCY:
            if (command.dest_id != global_broker_id_local) {
                routeMessage(command);
            } else {
                timeCoord->processDependencyUpdateMessage(command);
                if (!hasTimeDependency) {
                    hasTimeDependency = true;
                }
            }
            break;
        case CMD_ADD_NAMED_ENDPOINT:
        case CMD_ADD_NAMED_PUBLICATION:
        case CMD_ADD_NAMED_INPUT:
        case CMD_ADD_NAMED_FILTER:
            checkForNamedInterface(command);
            break;
        case CMD_REMOVE_NAMED_ENDPOINT:
        case CMD_REMOVE_NAMED_PUBLICATION:
        case CMD_REMOVE_NAMED_INPUT:
        case CMD_REMOVE_NAMED_FILTER:
            removeNamedTarget(command);
            break;
        case CMD_BROKER_CONFIGURE:
            processBrokerConfigureCommands(command);
            break;
        default:
            if (command.dest_id != global_broker_id_local) {
                routeMessage(command);
            }
    }
}

void CoreBroker::processBrokerConfigureCommands(ActionMessage& cmd)
{
    switch (cmd.messageID) {
        case defs::flags::enable_init_entry:
            /*if (delayInitCounter <= 1)
        {
            delayInitCounter = 0;
            if (allInitReady())
            {
                broker_state_t exp = connected;
                if (brokerState.compare_exchange_strong(exp, broker_state_t::initializing))
                {  // make sure we only do this once
                    checkDependencies();
                    cmd.setAction(CMD_INIT);
                    cmd.source_id = global_broker_id_local;
                    cmd.dest_id = 0;
                    transmit(0, cmd);
                }
            }
        }
        else
        {
            --delayInitCounter;
        }
        break;
        */
        case defs::properties::log_level:
            setLogLevel(cmd.getExtraData());
            break;
        case UPDATE_LOGGING_CALLBACK:
            if (checkActionFlag(cmd, empty_flag)) {
                setLoggerFunction(nullptr);
            } else {
                auto op = dataAirlocks[cmd.counter].try_unload();
                if (op) {
                    auto M = stx::any_cast<
                        std::function<void(int, const std::string&, const std::string&)>>(
                        std::move(*op));
                    M(0, identifier, "logging callback activated");
                    setLoggerFunction(std::move(M));
                }
            }
            break;
        case REQUEST_TICK_FORWARDING:
            forwardTick = checkActionFlag(cmd, indicator_flag);
        default:
            break;
    }
}

void CoreBroker::checkForNamedInterface(ActionMessage& command)
{
    bool foundInterface = false;
    switch (command.action()) {
        case CMD_ADD_NAMED_PUBLICATION: {
            auto pub = handles.getPublication(command.name);
            if (pub != nullptr) {
                auto fed = _federates.find(pub->getFederateId());
                if (!fed->isDisconnected) {
                    command.setAction(CMD_ADD_SUBSCRIBER);
                    command.setDestination(pub->handle);
                    command.name.clear();
                    routeMessage(command);
                    command.setAction(CMD_ADD_PUBLISHER);
                    command.swapSourceDest();
                    command.name = pub->key;
                    command.setStringData(pub->type, pub->units);
                    routeMessage(command);
                } else {
                    command.setAction(CMD_ADD_PUBLISHER);
                    setActionFlag(command, error_flag);
                    command.swapSourceDest();
                    command.setSource(pub->handle);
                    command.clearStringData();
                    routeMessage(command);
                }
                foundInterface = true;
            }
        } break;
        case CMD_ADD_NAMED_INPUT: {
            auto inp = handles.getInput(command.name);
            if (inp != nullptr) {
                auto fed = _federates.find(inp->getFederateId());
                if (!fed->isDisconnected) {
                    command.setAction(CMD_ADD_PUBLISHER);
                    command.setDestination(inp->handle);
                    auto pub = handles.findHandle(command.getSource());
                    if (pub != nullptr) {
                        command.setStringData(pub->type, pub->units);
                    }
                    command.name.clear();
                    routeMessage(command);
                    command.setAction(CMD_ADD_SUBSCRIBER);
                    command.swapSourceDest();
                    command.clearStringData();
                    command.name = inp->key;
                    routeMessage(command);
                } else {
                    command.setAction(CMD_ADD_SUBSCRIBER);
                    setActionFlag(command, error_flag);
                    command.swapSourceDest();
                    command.setSource(inp->handle);
                    command.clearStringData();
                    routeMessage(command);
                }
                foundInterface = true;
            }
        } break;
        case CMD_ADD_NAMED_FILTER: {
            auto filt = handles.getFilter(command.name);
            if (filt != nullptr) {
                command.setAction(CMD_ADD_ENDPOINT);
                command.setDestination(filt->handle);
                command.name.clear();
                routeMessage(command);
                command.setAction(CMD_ADD_FILTER);
                command.swapSourceDest();
                if ((!filt->type_in.empty()) || (!filt->type_out.empty())) {
                    command.setStringData(filt->type_in, filt->type_out);
                }
                if (checkActionFlag(*filt, clone_flag)) {
                    setActionFlag(command, clone_flag);
                }
                routeMessage(command);
                foundInterface = true;
            }
        } break;
        case CMD_ADD_NAMED_ENDPOINT: {
            auto ept = handles.getEndpoint(command.name);
            if (ept != nullptr) {
                auto fed = _federates.find(ept->getFederateId());
                if (!fed->isDisconnected) {
                    command.setAction(CMD_ADD_FILTER);
                    command.setDestination(ept->handle);
                    command.name.clear();
                    auto filt = handles.findHandle(command.getSource());
                    if (filt != nullptr) {
                        if ((!filt->type_in.empty()) || (!filt->type_out.empty())) {
                            command.setStringData(filt->type_in, filt->type_out);
                        }
                        if (checkActionFlag(*filt, clone_flag)) {
                            setActionFlag(command, clone_flag);
                        }
                    }
                    routeMessage(command);
                    command.setAction(CMD_ADD_ENDPOINT);
                    command.swapSourceDest();
                    command.clearStringData();
                    routeMessage(command);
                } else {
                    command.setAction(CMD_ADD_ENDPOINT);
                    setActionFlag(command, error_flag);
                    command.swapSourceDest();
                    command.setSource(ept->handle);
                    command.clearStringData();
                    routeMessage(command);
                }
                foundInterface = true;
            }
        } break;
        default:
            break;
    }
    if (!foundInterface) {
        if (isRootc) {
            switch (command.action()) {
                case CMD_ADD_NAMED_PUBLICATION:
                    unknownHandles.addUnknownPublication(
                        command.name, command.getSource(), command.flags);
                    break;
                case CMD_ADD_NAMED_INPUT:
                    unknownHandles.addUnknownInput(
                        command.name, command.getSource(), command.flags);
                    if (!command.getStringData().empty()) {
                        auto pub = handles.findHandle(command.getSource());
                        if (pub == nullptr) {
                            // an anonymous publisher is adding an input
                            auto& apub = handles.addHandle(
                                command.source_id,
                                command.source_handle,
                                handle_type::publication,
                                std::string(),
                                command.getString(typeStringLoc),
                                command.getString(unitStringLoc));

                            addLocalInfo(apub, command);
                        }
                    }
                    break;
                case CMD_ADD_NAMED_ENDPOINT:
                    unknownHandles.addUnknownEndpoint(
                        command.name, command.getSource(), command.flags);
                    if (!command.getStringData().empty()) {
                        auto filt = handles.findHandle(command.getSource());
                        if (filt == nullptr) {
                            // an anonymous filter is adding an endpoint
                            auto& afilt = handles.addHandle(
                                command.source_id,
                                command.source_handle,
                                handle_type::filter,
                                std::string(),
                                command.getString(typeStringLoc),
                                command.getString(typeOutStringLoc));

                            addLocalInfo(afilt, command);
                        }
                    }
                    break;
                case CMD_ADD_NAMED_FILTER:
                    unknownHandles.addUnknownFilter(
                        command.name, command.getSource(), command.flags);
                    break;
                default:
                    LOG_WARNING(
                        global_broker_id_local,
                        getIdentifier(),
                        "unknown command in interface addition code section\n");
                    break;
            }
        } else {
            routeMessage(command);
        }
    }
}

void CoreBroker::removeNamedTarget(ActionMessage& command)
{
    bool foundInterface = false;
    switch (command.action()) {
        case CMD_REMOVE_NAMED_PUBLICATION: {
            auto pub = handles.getPublication(command.name);
            if (pub != nullptr) {
                command.setAction(CMD_REMOVE_SUBSCRIBER);
                command.setDestination(pub->handle);
                command.name.clear();
                routeMessage(command);
                command.setAction(CMD_REMOVE_PUBLICATION);
                command.swapSourceDest();
                routeMessage(command);
                foundInterface = true;
            }
        } break;
        case CMD_REMOVE_NAMED_INPUT: {
            auto inp = handles.getInput(command.name);
            if (inp != nullptr) {
                command.setAction(CMD_REMOVE_PUBLICATION);
                command.setDestination(inp->handle);
                command.name.clear();
                routeMessage(command);
                command.setAction(CMD_REMOVE_SUBSCRIBER);
                command.swapSourceDest();
                routeMessage(command);
                foundInterface = true;
            }
        } break;
        case CMD_REMOVE_NAMED_FILTER: {
            auto filt = handles.getFilter(command.name);
            if (filt != nullptr) {
                command.setAction(CMD_REMOVE_ENDPOINT);
                command.setDestination(filt->handle);
                command.name.clear();
                routeMessage(command);
                command.setAction(CMD_REMOVE_FILTER);
                command.swapSourceDest();
                routeMessage(command);
                foundInterface = true;
            }
        } break;
        case CMD_REMOVE_NAMED_ENDPOINT: {
            auto ept = handles.getEndpoint(command.name);
            if (ept != nullptr) {
                command.setAction(CMD_REMOVE_FILTER);
                command.setDestination(ept->handle);
                command.name.clear();
                routeMessage(command);
                command.setAction(CMD_ADD_ENDPOINT);
                command.swapSourceDest();
                routeMessage(command);

                foundInterface = true;
            }
        } break;
        default:
            break;
    }
    if (!foundInterface) {
        if (isRootc) {
            LOG_WARNING(
                global_broker_id_local,
                getIdentifier(),
                fmt::format("attempt to remove unrecognized target {} ", command.name));
        } else {
            routeMessage(command);
        }
    }
}

void CoreBroker::addLocalInfo(BasicHandleInfo& handleInfo, const ActionMessage& m)
{
    auto res = global_id_translation.find(m.source_id);
    if (res != global_id_translation.end()) {
        handleInfo.local_fed_id = res->second;
    }
    handleInfo.flags = m.flags;
}

void CoreBroker::addPublication(ActionMessage& m)
{
    // detect duplicate publications
    if (handles.getPublication(m.name) != nullptr) {
        ActionMessage eret(CMD_ERROR, global_broker_id_local, m.source_id);
        eret.dest_handle = m.source_handle;
        eret.messageID = defs::errors::registration_failure;
        eret.payload = "Duplicate publication names (" + m.name + ")";
        routeMessage(eret);
        return;
    }
    auto& pub = handles.addHandle(
        m.source_id,
        m.source_handle,
        handle_type::publication,
        m.name,
        m.getString(0),
        m.getString(1));

    addLocalInfo(pub, m);
    if (!isRootc) {
        transmit(parent_route_id, m);
    } else {
        FindandNotifyPublicationTargets(pub);
    }
}
void CoreBroker::addInput(ActionMessage& m)
{
    // detect duplicate publications
    if (handles.getInput(m.name) != nullptr) {
        ActionMessage eret(CMD_ERROR, global_broker_id_local, m.source_id);
        eret.dest_handle = m.source_handle;
        eret.messageID = defs::errors::registration_failure;
        eret.payload = "Duplicate input names (" + m.name + ")";
        routeMessage(eret);
        return;
    }
    auto& inp = handles.addHandle(
        m.source_id, m.source_handle, handle_type::input, m.name, m.getString(0), m.getString(1));

    addLocalInfo(inp, m);
    if (!isRootc) {
        transmit(parent_route_id, m);
    } else {
        FindandNotifyInputTargets(inp);
    }
}

void CoreBroker::addEndpoint(ActionMessage& m)
{
    // detect duplicate endpoints
    if (handles.getEndpoint(m.name) != nullptr) {
        ActionMessage eret(CMD_ERROR, global_broker_id_local, m.source_id);
        eret.dest_handle = m.source_handle;
        eret.messageID = defs::errors::registration_failure;
        eret.payload = "Duplicate endpoint names (" + m.name + ")";
        routeMessage(eret);
        return;
    }
    auto& ept = handles.addHandle(
        m.source_id,
        m.source_handle,
        handle_type::endpoint,
        m.name,
        m.getString(typeStringLoc),
        m.getString(unitStringLoc));

    addLocalInfo(ept, m);

    if (!isRootc) {
        transmit(parent_route_id, m);
        if (!hasTimeDependency) {
            if (timeCoord->addDependency(higher_broker_id)) {
                hasTimeDependency = true;
                ActionMessage add(
                    CMD_ADD_INTERDEPENDENCY, global_broker_id_local, higher_broker_id);
                transmit(parent_route_id, add);

                timeCoord->addDependent(higher_broker_id);
            }
        }
    } else {
        FindandNotifyEndpointTargets(ept);
    }
}
void CoreBroker::addFilter(ActionMessage& m)
{
    // detect duplicate endpoints
    if (handles.getFilter(m.name) != nullptr) {
        ActionMessage eret(CMD_ERROR, global_broker_id_local, m.source_id);
        eret.dest_handle = m.source_handle;
        eret.messageID = defs::errors::registration_failure;
        eret.payload = "Duplicate filter names (" + m.name + ")";
        routeMessage(eret);
        return;
    }

    auto& filt = handles.addHandle(
        m.source_id,
        m.source_handle,
        handle_type::filter,
        m.name,
        m.getString(typeStringLoc),
        m.getString(typeOutStringLoc));
    addLocalInfo(filt, m);

    if (!isRootc) {
        transmit(parent_route_id, m);
        if (!hasFilters) {
            hasFilters = true;
            if (timeCoord->addDependent(higher_broker_id)) {
                hasTimeDependency = true;
                ActionMessage add(CMD_ADD_DEPENDENCY, global_broker_id_local, higher_broker_id);
                transmit(parent_route_id, add);
            }
        }
    } else {
        FindandNotifyFilterTargets(filt);
    }
}

CoreBroker::CoreBroker(bool setAsRootBroker) noexcept:
    _isRoot(setAsRootBroker), isRootc(setAsRootBroker), timeoutMon(new TimeoutMonitor)
{
}

CoreBroker::CoreBroker(const std::string& broker_name):
    BrokerBase(broker_name), timeoutMon(new TimeoutMonitor)
{
}

void CoreBroker::configure(const std::string& configureString)
{
    broker_state_t exp = broker_state_t::created;
    if (brokerState.compare_exchange_strong(exp, broker_state_t::configuring)) {
        auto result = parseArgs(configureString);
        if (result != 0) {
            brokerState = broker_state_t::created;
            if (result < 0) {
                throw(helics::InvalidParameter("invalid arguments in configure string"));
            }
            return;
        }
        configureBase();
    }
}

void CoreBroker::configureFromArgs(int argc, char* argv[])
{
    broker_state_t exp = broker_state_t::created;
    if (brokerState.compare_exchange_strong(exp, broker_state_t::configuring)) {
        auto result = parseArgs(argc, argv);
        if (result != 0) {
            brokerState = broker_state_t::created;
            if (result < 0) {
                throw(helics::InvalidParameter("invalid arguments in command line"));
            }
            return;
        }
        configureBase();
    }
}

void CoreBroker::configureFromVector(std::vector<std::string> args)
{
    broker_state_t exp = broker_state_t::created;
    if (brokerState.compare_exchange_strong(exp, broker_state_t::configuring)) {
        auto result = parseArgs(std::move(args));
        if (result != 0) {
            brokerState = broker_state_t::created;
            if (result < 0) {
                throw(helics::InvalidParameter("invalid arguments in command line"));
            }
            return;
        }
        configureBase();
    }
}

std::shared_ptr<helicsCLI11App> CoreBroker::generateCLI()
{
    auto app = std::make_shared<helicsCLI11App>("Option for Broker");
    app->remove_helics_specifics();
    app->add_flag_callback(
        "--root", [this]() { setAsRoot(); }, "specify whether the broker is a root");
    return app;
}

void CoreBroker::setAsRoot()
{
    if (brokerState < broker_state_t::connected) {
        _isRoot = true;
        global_id = global_broker_id(1);
    }
}

bool CoreBroker::connect()
{
    if (brokerState < broker_state_t::connected) {
        broker_state_t exp = broker_state_t::configured;
        if (brokerState.compare_exchange_strong(exp, broker_state_t::connecting)) {
            LOG_CONNECTIONS(parent_broker_id, getIdentifier(), "connecting");
            timeoutMon->setTimeout(std::chrono::milliseconds(timeout));
            auto res = brokerConnect();
            if (res) {
                disconnection.activate();
                brokerState = broker_state_t::connected;
                ActionMessage setup(CMD_BROKER_SETUP);
                addActionMessage(setup);
                if (!_isRoot) {
                    ActionMessage m(CMD_REG_BROKER);
                    m.source_id = global_federate_id{};
                    m.name = getIdentifier();
                    if (no_ping) {
                        setActionFlag(m, slow_responding_flag);
                    }
                    if (!brokerKey.empty() && brokerKey != universalKey) {
                        m.setStringData(getAddress(), brokerKey);
                    } else {
                        m.setStringData(getAddress());
                    }
                    transmit(parent_route_id, m);
                }
                LOG_CONNECTIONS(
                    parent_broker_id,
                    getIdentifier(),
                    fmt::format("||connected on {}", getAddress()));
            } else {
                brokerState = broker_state_t::configured;
            }
            return res;
        }
        if (brokerState.load() == broker_state_t::connecting) {
            while (brokerState.load() == broker_state_t::connecting) {
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
        }
    }
    return isConnected();
}

bool CoreBroker::isConnected() const
{
    auto state = brokerState.load(std::memory_order_acquire);
    return ((state == broker_state_t::operating) || (state == broker_state_t::connected));
}

bool CoreBroker::waitForDisconnect(std::chrono::milliseconds msToWait) const
{
    if (msToWait <= std::chrono::milliseconds(0)) {
        disconnection.wait();
        return true;
    }
    return disconnection.wait_for(msToWait);
}

void CoreBroker::processDisconnect(bool skipUnregister)
{
    if ((brokerState == broker_state_t::terminating) ||
        (brokerState == broker_state_t::terminated)) {
        return;
    }
    if (brokerState > broker_state_t::configured) {
        LOG_CONNECTIONS(parent_broker_id, getIdentifier(), "||disconnecting");
        brokerState = broker_state_t::terminating;
        brokerDisconnect();
    }
    brokerState = broker_state_t::terminated;

    if (!skipUnregister) {
        unregister();
    }
    disconnection.trigger();
}

void CoreBroker::unregister()
{
    /*We need to ensure that the destructor is not called immediately upon calling unregister
    otherwise this would be a mess and probably cause segmentation faults so we capture it in a local variable
    that will be destroyed on function exit
    */
    auto keepBrokerAlive = BrokerFactory::findBroker(identifier);
    if (keepBrokerAlive) {
        BrokerFactory::unregisterBroker(identifier);
    }
    if (!previous_local_broker_identifier.empty()) {
        auto keepBrokerAlive2 = BrokerFactory::findBroker(previous_local_broker_identifier);
        if (keepBrokerAlive2) {
            BrokerFactory::unregisterBroker(previous_local_broker_identifier);
        }
    }
}

void CoreBroker::disconnect()
{
    ActionMessage udisconnect(CMD_USER_DISCONNECT);
    addActionMessage(udisconnect);
    int cnt{0};
    while (!waitForDisconnect(std::chrono::milliseconds(200))) {
        ++cnt;
        LOG_WARNING(
            global_id.load(),
            getIdentifier(),
            "waiting on disconnect: current state=" + brokerStateName(brokerState.load()));
        if (cnt % 4 == 0) {
            if (!isRunning()) {
                LOG_WARNING(
                    global_id.load(),
                    getIdentifier(),
                    "main loop is stopped but have not received disconnect notice, assuming disconnected");
                return;
            } else {
                LOG_WARNING(
                    global_id.load(),
                    getIdentifier(),
                    fmt::format(
                        "sending disconnect again; total message count = {}",
                        currentMessageCounter()));
            }
            addActionMessage(udisconnect);
        }
    }
}

void CoreBroker::transmitToParent(ActionMessage&& cmd)
{
    if (isRoot()) {
        addActionMessage(std::move(cmd));
    } else {
        if (!global_id.load().isValid()) {
            delayTransmitQueue.push(std::move(cmd));
        } else {
            transmit(parent_route_id, std::move(cmd));
        }
    }
}

void CoreBroker::routeMessage(ActionMessage& cmd, global_federate_id dest)
{
    if (dest == global_federate_id{}) {
        return;
    }
    cmd.dest_id = dest;
    if ((dest == parent_broker_id) || (dest == higher_broker_id)) {
        transmit(parent_route_id, cmd);
    } else {
        auto route = getRoute(dest);
        transmit(route, cmd);
    }
}

void CoreBroker::routeMessage(const ActionMessage& cmd)
{
    if ((cmd.dest_id == parent_broker_id) || (cmd.dest_id == higher_broker_id)) {
        transmit(parent_route_id, cmd);
    } else {
        auto route = getRoute(cmd.dest_id);
        transmit(route, cmd);
    }
}

void CoreBroker::routeMessage(ActionMessage&& cmd, global_federate_id dest)
{
    if (!dest.isValid()) {
        return;
    }
    cmd.dest_id = dest;
    if ((dest == parent_broker_id) || (dest == higher_broker_id)) {
        transmit(parent_route_id, std::move(cmd));
    } else {
        auto route = getRoute(dest);
        transmit(route, std::move(cmd));
    }
}

void CoreBroker::routeMessage(ActionMessage&& cmd)
{
    if ((cmd.dest_id == parent_broker_id) || (cmd.dest_id == higher_broker_id)) {
        transmit(parent_route_id, std::move(cmd));
    } else {
        auto route = getRoute(cmd.dest_id);
        transmit(route, std::move(cmd));
    }
}

void CoreBroker::broadcast(ActionMessage& cmd)
{
    for (auto& broker : _brokers) {
        if ((!broker._nonLocal) && (!broker.isDisconnected)) {
            cmd.dest_id = broker.global_id;
            transmit(broker.route, cmd);
        }
    }
}

void CoreBroker::executeInitializationOperations()
{
    if (brokerKey == universalKey) {
        LOG_SUMMARY(global_broker_id_local, getIdentifier(), " Broker started with universal key");
    }
    checkDependencies();

    if (unknownHandles.hasUnknowns()) {
        if (unknownHandles.hasNonOptionalUnknowns()) {
            if (unknownHandles.hasRequiredUnknowns()) {
                ActionMessage eMiss(CMD_ERROR);
                eMiss.source_id = global_broker_id_local;
                eMiss.messageID = defs::errors::connection_failure;
                unknownHandles.processRequiredUnknowns(
                    [this, &eMiss](const std::string& target, char type, global_handle handle) {
                        switch (type) {
                            case 'p':
                                eMiss.payload = fmt::format(
                                    "Unable to connect to required publication target {}", target);
                                LOG_ERROR(parent_broker_id, getIdentifier(), eMiss.payload);
                                break;
                            case 'i':
                                eMiss.payload = fmt::format(
                                    "Unable to connect to required input target {}", target);
                                LOG_ERROR(parent_broker_id, getIdentifier(), eMiss.payload);
                                break;
                            case 'f':
                                eMiss.payload = fmt::format(
                                    "Unable to connect to required filter target {}", target);
                                LOG_ERROR(parent_broker_id, getIdentifier(), eMiss.payload);
                                break;
                            case 'e':
                                eMiss.payload = fmt::format(
                                    "Unable to connect to required endpoint target {}", target);
                                LOG_ERROR(parent_broker_id, getIdentifier(), eMiss.payload);
                                break;
                        }
                        eMiss.setDestination(handle);
                        routeMessage(eMiss);
                    });
                eMiss.payload = "Missing required connections";
                eMiss.dest_handle = interface_handle{};
                broadcast(eMiss);
                sendDisconnect();
                addActionMessage(CMD_STOP);
                return;
            }
            ActionMessage wMiss(CMD_WARNING);
            wMiss.source_id = global_broker_id_local;
            wMiss.messageID = defs::errors::connection_failure;
            unknownHandles.processNonOptionalUnknowns(
                [this, &wMiss](const std::string& target, char type, global_handle handle) {
                    switch (type) {
                        case 'p':
                            wMiss.payload =
                                fmt::format("Unable to connect to publication target {}", target);
                            LOG_WARNING(parent_broker_id, getIdentifier(), wMiss.payload);
                            break;
                        case 'i':
                            wMiss.payload =
                                fmt::format("Unable to connect to input target {}", target);
                            LOG_WARNING(parent_broker_id, getIdentifier(), wMiss.payload);
                            break;
                        case 'f':
                            wMiss.payload =
                                fmt::format("Unable to connect to filter target {}", target);
                            LOG_WARNING(parent_broker_id, getIdentifier(), wMiss.payload);
                            break;
                        case 'e':
                            wMiss.payload =
                                fmt::format("Unable to connect to endpoint target {}", target);
                            LOG_WARNING(parent_broker_id, getIdentifier(), wMiss.payload);
                            break;
                    }
                    wMiss.setDestination(handle);
                    routeMessage(wMiss);
                });
        }
    }

    ActionMessage m(CMD_INIT_GRANT);
    m.source_id = global_broker_id_local;
    brokerState = broker_state_t::operating;
    broadcast(m);
    timeCoord->enteringExecMode();
    auto res = timeCoord->checkExecEntry();
    if (res == message_processing_result::next_step) {
        enteredExecutionMode = true;
    }
    loggingObj->flush();
}

void CoreBroker::FindandNotifyInputTargets(BasicHandleInfo& handleInfo)
{
    auto Handles = unknownHandles.checkForInputs(handleInfo.key);
    for (auto target : Handles) {
        // notify the publication about its subscriber
        ActionMessage m(CMD_ADD_SUBSCRIBER);

        m.setDestination(target.first);
        m.setSource(handleInfo.handle);
        m.payload = handleInfo.type;
        m.flags = handleInfo.flags;
        transmit(getRoute(m.dest_id), m);

        // notify the subscriber about its publisher
        m.setAction(CMD_ADD_PUBLISHER);
        m.setSource(target.first);
        m.setDestination(handleInfo.handle);
        m.flags = target.second;
        auto pub = handles.findHandle(target.first);
        if (pub != nullptr) {
            m.setStringData(pub->type, pub->units);
        }

        transmit(getRoute(m.dest_id), std::move(m));
    }
    if (!Handles.empty()) {
        unknownHandles.clearInput(handleInfo.key);
    }
}

void CoreBroker::FindandNotifyPublicationTargets(BasicHandleInfo& handleInfo)
{
    auto subHandles = unknownHandles.checkForPublications(handleInfo.key);
    for (auto sub : subHandles) {
        // notify the publication about its subscriber
        ActionMessage m(CMD_ADD_SUBSCRIBER);
        m.setSource(sub.first);
        m.setDestination(handleInfo.handle);
        m.flags = sub.second;

        transmit(getRoute(m.dest_id), m);

        // notify the subscriber about its publisher
        m.setAction(CMD_ADD_PUBLISHER);
        m.setDestination(sub.first);
        m.setSource(handleInfo.handle);
        m.payload = handleInfo.type;
        m.flags = handleInfo.flags;
        m.setStringData(handleInfo.type, handleInfo.units);
        transmit(getRoute(m.dest_id), std::move(m));
    }

    auto Pubtargets = unknownHandles.checkForLinks(handleInfo.key);
    for (auto sub : Pubtargets) {
        ActionMessage m(CMD_ADD_NAMED_INPUT);
        m.name = sub;
        m.setSource(handleInfo.handle);
        checkForNamedInterface(m);
    }
    if (!(subHandles.empty() && Pubtargets.empty())) {
        unknownHandles.clearPublication(handleInfo.key);
    }
}

void CoreBroker::FindandNotifyEndpointTargets(BasicHandleInfo& handleInfo)
{
    auto Handles = unknownHandles.checkForEndpoints(handleInfo.key);
    for (auto target : Handles) {
        // notify the filter about its endpoint
        ActionMessage m(CMD_ADD_ENDPOINT);
        m.setSource(handleInfo.handle);
        m.setDestination(target.first);
        m.flags = target.second;
        transmit(getRoute(m.dest_id), m);

        // notify the endpoint about its filter
        m.setAction(CMD_ADD_FILTER);
        m.swapSourceDest();
        m.flags = target.second;
        transmit(getRoute(m.dest_id), m);
    }

    if (!Handles.empty()) {
        unknownHandles.clearEndpoint(handleInfo.key);
    }
}

void CoreBroker::FindandNotifyFilterTargets(BasicHandleInfo& handleInfo)
{
    auto Handles = unknownHandles.checkForFilters(handleInfo.key);
    for (auto target : Handles) {
        // notify the endpoint about a filter
        ActionMessage m(CMD_ADD_FILTER);
        m.setSource(handleInfo.handle);
        m.flags = target.second;
        if (checkActionFlag(handleInfo, clone_flag)) {
            setActionFlag(m, clone_flag);
        }
        m.setDestination(target.first);
        if ((!handleInfo.type_in.empty()) || (!handleInfo.type_out.empty())) {
            m.setStringData(handleInfo.type_in, handleInfo.type_out);
        }
        transmit(getRoute(m.dest_id), m);

        // notify the filter about an endpoint
        m.setAction(CMD_ADD_ENDPOINT);
        m.swapSourceDest();
        m.clearStringData();
        transmit(getRoute(m.dest_id), m);
    }

    auto FiltDestTargets = unknownHandles.checkForFilterDestTargets(handleInfo.key);
    for (auto target : FiltDestTargets) {
        ActionMessage m(CMD_ADD_NAMED_ENDPOINT);
        m.name = target;
        m.setSource(handleInfo.handle);
        m.flags = handleInfo.flags;
        setActionFlag(m, destination_target);
        if (checkActionFlag(handleInfo, clone_flag)) {
            setActionFlag(m, clone_flag);
        }
        checkForNamedInterface(m);
    }

    auto FiltSourceTargets = unknownHandles.checkForFilterSourceTargets(handleInfo.key);
    for (auto target : FiltSourceTargets) {
        ActionMessage m(CMD_ADD_NAMED_ENDPOINT);
        m.name = target;
        m.flags = handleInfo.flags;
        m.setSource(handleInfo.handle);
        if (checkActionFlag(handleInfo, clone_flag)) {
            setActionFlag(m, clone_flag);
        }
        checkForNamedInterface(m);
    }
    if (!(Handles.empty() && FiltDestTargets.empty() && FiltSourceTargets.empty())) {
        unknownHandles.clearFilter(handleInfo.key);
    }
}

void CoreBroker::processDisconnect(ActionMessage& command)
{
    auto brk = getBrokerById(global_broker_id(command.source_id));
    switch (command.action()) {
        case CMD_DISCONNECT:
        case CMD_PRIORITY_DISCONNECT:
            if (command.dest_id == global_broker_id_local) {
                // deal with the time implications of the message
                if (hasTimeDependency) {
                    if (!enteredExecutionMode) {
                        timeCoord->processTimeMessage(command);
                        auto res = timeCoord->checkExecEntry();
                        if (res == message_processing_result::next_step) {
                            enteredExecutionMode = true;
                        }
                    } else {
                        if (timeCoord->processTimeMessage(command)) {
                            timeCoord->updateTimeFactors();
                        }
                    }
                }
            } else if (command.dest_id == parent_broker_id) {
                if (!isRootc) // we got a disconnect from up above
                {
                    LOG_CONNECTIONS(
                        parent_broker_id, getIdentifier(), "got disconnect from parent");
                    if (command.source_id == higher_broker_id) {
                        sendDisconnect();
                        addActionMessage(CMD_STOP);
                        return;
                    }
                }

                if (brk != nullptr) {
                    LOG_CONNECTIONS(
                        parent_broker_id,
                        getIdentifier(),
                        fmt::format(
                            "got disconnect from {}({})",
                            brk->name,
                            command.source_id.baseValue()));
                    disconnectBroker(*brk);
                }

                if (allDisconnected()) {
                    timeCoord->disconnect();
                    if (!isRootc) {
                        ActionMessage dis(CMD_DISCONNECT);
                        dis.source_id = global_broker_id_local;
                        transmit(parent_route_id, dis);
                    } else {
                        if ((brk != nullptr) && (!brk->_nonLocal)) {
                            if (!checkActionFlag(command, error_flag)) {
                                ActionMessage dis(
                                    (brk->_core) ? CMD_DISCONNECT_CORE_ACK :
                                                   CMD_DISCONNECT_BROKER_ACK);
                                dis.source_id = global_broker_id_local;
                                dis.dest_id = brk->global_id;
                                transmit(brk->route, dis);
                            }
                            brk->_sent_disconnect_ack = true;
                            removeRoute(brk->route);
                        }
                        addActionMessage(CMD_STOP);
                    }
                } else {
                    if ((brk != nullptr) && (!brk->_nonLocal)) {
                        if (!checkActionFlag(command, error_flag)) {
                            ActionMessage dis(
                                (brk->_core) ? CMD_DISCONNECT_CORE_ACK : CMD_DISCONNECT_BROKER_ACK);
                            dis.source_id = global_broker_id_local;
                            dis.dest_id = brk->global_id;
                            transmit(brk->route, dis);
                        }
                        brk->_sent_disconnect_ack = true;
                        if ((!isRootc) && (brokerState < broker_state_t::operating)) {
                            command.setAction(
                                (brk->_core) ? CMD_DISCONNECT_CORE : CMD_DISCONNECT_BROKER);
                            transmit(parent_route_id, command);
                        }
                        removeRoute(brk->route);
                    } else {
                        if ((!isRootc) && (brokerState < broker_state_t::operating)) {
                            if (brk != nullptr) {
                                command.setAction(
                                    (brk->_core) ? CMD_DISCONNECT_CORE : CMD_DISCONNECT_BROKER);
                                transmit(parent_route_id, command);
                            }
                        }
                    }
                }
            } else {
                transmit(getRoute(command.dest_id), command);
            }
            break;
        case CMD_DISCONNECT_CORE:
        case CMD_DISCONNECT_BROKER:
            if (brk != nullptr) {
                disconnectBroker(*brk);
                if (!isRootc) {
                    transmit(parent_route_id, command);
                }
            }
            break;
        default:
            break;
    }
}

void CoreBroker::disconnectBroker(BasicBrokerInfo& brk)
{
    brk.isDisconnected = true;
    if (brokerState < broker_state_t::operating) {
        if (isRootc) {
            ActionMessage dis(CMD_BROADCAST_DISCONNECT);
            dis.source_id = brk.global_id;
            broadcast(dis);
            unknownHandles.clearFederateUnknowns(brk.global_id);
            if (!brk._core) {
                for (auto& subbrk : _brokers) {
                    if ((subbrk.parent == brk.global_id) && (subbrk._core)) {
                        unknownHandles.clearFederateUnknowns(subbrk.global_id);
                    }
                }
            }
        }
    }
}

void CoreBroker::setLoggingLevel(int logLevel)
{
    ActionMessage cmd(CMD_BROKER_CONFIGURE);
    cmd.dest_id = global_id.load();
    cmd.messageID = defs::properties::log_level;
    cmd.setExtraData(logLevel);
    addActionMessage(cmd);
}

void CoreBroker::setLogFile(const std::string& lfile)
{
    setLoggingFile(lfile);
}

// public query function
std::string CoreBroker::query(const std::string& target, const std::string& queryStr)
{
    auto gid = global_id.load();
    if ((target == "broker") || (target == getIdentifier())) {
        ActionMessage querycmd(CMD_BROKER_QUERY);
        querycmd.source_id = querycmd.dest_id = gid;
        auto index = ++queryCounter;
        querycmd.messageID = index;
        querycmd.payload = queryStr;
        auto queryResult = ActiveQueries.getFuture(index);
        addActionMessage(std::move(querycmd));
        auto ret = queryResult.get();
        ActiveQueries.finishedWithValue(index);
        return ret;
    } else if (target == "parent") {
        if (isRootc) {
            return "#na";
        }
        ActionMessage querycmd(CMD_BROKER_QUERY);
        querycmd.source_id = gid;
        querycmd.messageID = ++queryCounter;
        querycmd.payload = queryStr;
        auto queryResult = ActiveQueries.getFuture(querycmd.messageID);
        addActionMessage(querycmd);
        auto ret = queryResult.get();
        ActiveQueries.finishedWithValue(querycmd.messageID);
        return ret;
    } else if ((target == "root") || (target == "rootbroker")) {
        ActionMessage querycmd(CMD_BROKER_QUERY);
        querycmd.source_id = gid;
        auto index = ++queryCounter;
        querycmd.messageID = index;
        querycmd.payload = queryStr;
        auto queryResult = ActiveQueries.getFuture(querycmd.messageID);
        transmitToParent(std::move(querycmd));

        auto ret = queryResult.get();
        ActiveQueries.finishedWithValue(index);
        return ret;
    } else {
        ActionMessage querycmd(CMD_QUERY);
        querycmd.source_id = gid;
        auto index = ++queryCounter;
        querycmd.messageID = index;
        querycmd.payload = queryStr;
        querycmd.setStringData(target);
        auto queryResult = ActiveQueries.getFuture(querycmd.messageID);
        transmitToParent(std::move(querycmd));

        auto ret = queryResult.get();
        ActiveQueries.finishedWithValue(index);
        return ret;
    }
    //  return "#invalid";
}

void CoreBroker::setGlobal(const std::string& valueName, const std::string& value)
{
    ActionMessage querycmd(CMD_SET_GLOBAL);
    querycmd.source_id = global_id.load();
    querycmd.payload = valueName;
    querycmd.setStringData(value);
    transmitToParent(std::move(querycmd));
}

std::string CoreBroker::generateQueryAnswer(const std::string& request)
{
    if (request == "isinit") {
        return (brokerState >= broker_state_t::operating) ? std::string("true") :
                                                            std::string("false");
    }
    if (request == "isconnected") {
        return (isConnected()) ? std::string("true") : std::string("false");
    }
    if (request == "name") {
        return getIdentifier();
    }
    if ((request == "queries") || (request == "available_queries")) {
        return "[isinit;isconnected;name;address;queries;address;counts;summary;federates;brokers;inputs;endpoints;"
               "publications;filters;federate_map;dependency_graph;dependencies;dependson;dependents]";
    }
    if (request == "address") {
        return getAddress();
    }
    if (request == "counts") {
        std::string cnts = "{\"brokers\":";
        cnts += std::to_string(_brokers.size());
        cnts += ",\n";
        cnts += "\"federates\":";
        cnts += std::to_string(_federates.size());
        cnts += ",\n";
        cnts += "\"handles\":";
        cnts += std::to_string(handles.size());
        cnts += '}';
        return cnts;
    }
    if (request == "summary") {
        return generateFederationSummary();
    }
    if (request == "federates") {
        return generateStringVector(_federates, [](auto& fed) { return fed.name; });
    }
    if (request == "brokers") {
        return generateStringVector(_brokers, [](auto& brk) { return brk.name; });
    }
    if (request == "inputs") {
        return generateStringVector_if(
            handles,
            [](auto& handle) { return handle.key; },
            [](auto& handle) { return (handle.handleType == handle_type::input); });
    }
    if (request == "publications") {
        return generateStringVector_if(
            handles,
            [](auto& handle) { return handle.key; },
            [](auto& handle) { return (handle.handleType == handle_type::publication); });
    }
    if (request == "filters") {
        return generateStringVector_if(
            handles,
            [](auto& handle) { return handle.key; },
            [](auto& handle) { return (handle.handleType == handle_type::filter); });
    }
    if (request == "endpoints") {
        return generateStringVector_if(
            handles,
            [](auto& handle) { return handle.key; },
            [](auto& handle) { return (handle.handleType == handle_type::endpoint); });
    }
    if (request == "federate_map") {
        if (fedMap.isCompleted()) {
            return fedMap.generate();
        }
        if (fedMap.isActive()) {
            return "#wait";
        }
        initializeFederateMap();
        if (fedMap.isCompleted()) {
            return fedMap.generate();
        }
        return "#wait";
    }
    if (request == "dependency_graph") {
        if (depMap.isCompleted()) {
            return depMap.generate();
        }
        if (depMap.isActive()) {
            return "#wait";
        }
        initializeDependencyGraph();
        if (depMap.isCompleted()) {
            return depMap.generate();
        }
        return "#wait";
    }
    if (request == "dependson") {
        return generateStringVector(timeCoord->getDependencies(), [](const auto& dep) {
            return std::to_string(dep.baseValue());
        });
    }
    if (request == "dependents") {
        return generateStringVector(timeCoord->getDependents(), [](const auto& dep) {
            return std::to_string(dep.baseValue());
        });
    }
    if (request == "dependencies") {
        Json::Value base;
        base["name"] = getIdentifier();
        base["id"] = global_broker_id_local.baseValue();
        if (!isRootc) {
            base["parent"] = higher_broker_id.baseValue();
        }
        base["dependents"] = Json::arrayValue;
        for (auto& dep : timeCoord->getDependents()) {
            base["dependents"].append(dep.baseValue());
        }
        base["dependencies"] = Json::arrayValue;
        for (auto& dep : timeCoord->getDependencies()) {
            base["dependencies"].append(dep.baseValue());
        }
        return generateJsonString(base);
    }
    return "#invalid";
}

std::string CoreBroker::getNameList(std::string gidString) const
{
    if (gidString.back() == ']') {
        gidString.pop_back();
    }
    if (gidString.front() == '[') {
        gidString = gidString.substr(1);
    }
    auto val = gmlc::utilities::str2vector<int>(gidString, -23, ";:");
    gidString.clear();
    gidString.push_back('[');
    size_t index = 0;
    while (index + 1 < val.size()) {
        auto info = handles.findHandle(
            global_handle(global_federate_id(val[index]), interface_handle(val[index + 1])));
        if (info != nullptr) {
            gidString.append(info->key);
            gidString.push_back(';');
        }

        index += 2;
    }
    if (gidString.back() == ';') {
        gidString.pop_back();
    }
    gidString.push_back(']');
    return gidString;
}
void CoreBroker::initializeFederateMap()
{
    Json::Value& base = fedMap.getJValue();
    base["name"] = getIdentifier();
    base["id"] = global_broker_id_local.baseValue();
    if (!isRootc) {
        base["parent"] = higher_broker_id.baseValue();
    }
    base["brokers"] = Json::arrayValue;
    ActionMessage queryReq(CMD_BROKER_QUERY);
    queryReq.payload = "federate_map";
    queryReq.source_id = global_broker_id_local;
    queryReq.counter = 2; // indicating which processing to use
    bool hasCores = false;
    for (auto& broker : _brokers) {
        if (broker.parent == global_broker_id_local) {
            int index;
            if (broker._core) {
                if (!hasCores) {
                    hasCores = true;
                    base["cores"] = Json::arrayValue;
                }
                index = fedMap.generatePlaceHolder("cores");
            } else {
                index = fedMap.generatePlaceHolder("brokers");
            }
            queryReq.messageID = index;
            queryReq.dest_id = broker.global_id;
            transmit(broker.route, queryReq);
        }
    }
}

void CoreBroker::initializeDependencyGraph()
{
    Json::Value& base = depMap.getJValue();
    base["name"] = getIdentifier();
    base["id"] = global_broker_id_local.baseValue();
    if (!isRootc) {
        base["parent"] = higher_broker_id.baseValue();
    }
    base["brokers"] = Json::arrayValue;
    ActionMessage queryReq(CMD_BROKER_QUERY);
    queryReq.payload = "dependency_graph";
    queryReq.source_id = global_broker_id_local;
    queryReq.counter = 4; // indicating which processing to use
    bool hasCores = false;
    for (auto& broker : _brokers) {
        int index;
        if (broker._core) {
            if (!hasCores) {
                hasCores = true;
                base["cores"] = Json::arrayValue;
            }
            index = depMap.generatePlaceHolder("cores");
        } else {
            index = depMap.generatePlaceHolder("brokers");
        }
        queryReq.messageID = index;
        queryReq.dest_id = broker.global_id;
        transmit(broker.route, queryReq);
    }

    base["dependents"] = Json::arrayValue;
    for (auto& dep : timeCoord->getDependents()) {
        base["dependents"].append(dep.baseValue());
    }
    base["dependencies"] = Json::arrayValue;
    for (auto& dep : timeCoord->getDependencies()) {
        base["dependencies"].append(dep.baseValue());
    }
}

void CoreBroker::initializeDataFlowGraph()
{
    Json::Value& base = depMap.getJValue();
    base["name"] = getIdentifier();
    base["id"] = global_broker_id_local.baseValue();
    if (!isRootc) {
        base["parent"] = higher_broker_id.baseValue();
    }
    base["brokers"] = Json::arrayValue;
    ActionMessage queryReq(CMD_BROKER_QUERY);
    queryReq.payload = "dependency_graph";
    queryReq.source_id = global_broker_id_local;
    queryReq.counter = 4; // indicating which processing to use
    bool hasCores = false;
    for (auto& broker : _brokers) {
        int index;
        if (broker._core) {
            if (!hasCores) {
                hasCores = true;
                base["cores"] = Json::arrayValue;
            }
            index = depMap.generatePlaceHolder("cores");
        } else {
            index = depMap.generatePlaceHolder("brokers");
        }
        queryReq.messageID = index;
        queryReq.dest_id = broker.global_id;
        transmit(broker.route, queryReq);
    }

    base["dependents"] = Json::arrayValue;
    for (auto& dep : timeCoord->getDependents()) {
        base["dependents"].append(dep.baseValue());
    }
    base["dependencies"] = Json::arrayValue;
    for (auto& dep : timeCoord->getDependencies()) {
        base["dependencies"].append(dep.baseValue());
    }
}

void CoreBroker::processLocalQuery(const ActionMessage& m)
{
    ActionMessage queryRep(CMD_QUERY_REPLY);
    queryRep.source_id = global_broker_id_local;
    queryRep.dest_id = m.source_id;
    queryRep.messageID = m.messageID;
    queryRep.payload = generateQueryAnswer(m.payload);
    queryRep.counter = m.counter;
    if (queryRep.payload == "#wait") {
        if (m.payload == "dependency_graph") {
            depMapRequestors.push_back(queryRep);
        } else if (m.payload == "federate_map") {
            fedMapRequestors.push_back(queryRep);
        } else if (m.payload == "data_flow_graph") {
            dataflowMapRequestors.push_back(queryRep);
        }
    } else if (queryRep.dest_id == global_broker_id_local) {
        ActiveQueries.setDelayedValue(m.messageID, queryRep.payload);
    } else {
        routeMessage(std::move(queryRep), m.source_id);
    }
}

void CoreBroker::processQuery(const ActionMessage& m)
{
    auto& target = m.getString(targetStringLoc);
    if ((target == getIdentifier()) || (target == "broker")) {
        processLocalQuery(m);
    } else if ((isRootc) && ((target == "root") || (target == "federation"))) {
        processLocalQuery(m);
    } else if (target == "gid_to_name") {
        ActionMessage queryResp(CMD_QUERY_REPLY);
        queryResp.dest_id = m.source_id;
        queryResp.source_id = global_broker_id_local;
        queryResp.messageID = m.messageID;
        queryResp.payload = getNameList(m.payload);
        if (queryResp.dest_id == global_broker_id_local) {
            ActiveQueries.setDelayedValue(m.messageID, queryResp.payload);
        } else {
            transmit(getRoute(queryResp.dest_id), queryResp);
        }
    } else if (target == "global") {
        ActionMessage queryResp(CMD_QUERY_REPLY);
        queryResp.dest_id = m.source_id;
        queryResp.source_id = global_broker_id_local;
        queryResp.messageID = m.messageID;

        auto gfind = global_values.find(m.payload);
        if (gfind != global_values.end()) {
            queryResp.payload = gfind->second;
        } else if (m.payload == "list") {
            queryResp.payload =
                generateStringVector(global_values, [](const auto& gv) { return gv.first; });
        } else if (m.payload == "all") {
            JsonMapBuilder globalSet;
            auto& jv = globalSet.getJValue();
            for (auto& val : global_values) {
                jv[val.first] = val.second;
            }
            queryResp.payload = globalSet.generate();
        } else {
            queryResp.payload = "#invalid";
        }
        if (queryResp.dest_id == global_broker_id_local) {
            ActiveQueries.setDelayedValue(m.messageID, queryResp.payload);
        } else {
            transmit(getRoute(queryResp.dest_id), queryResp);
        }
    } else {
        route_id route = parent_route_id;
        auto fed = _federates.find(target);
        if (fed != _federates.end()) {
            route = fed->route;
        } else {
            auto broker = _brokers.find(target);
            if (broker != _brokers.end()) {
                route = broker->route;
            }
        }
        if ((route == parent_route_id) && (isRootc)) {
            ActionMessage queryResp(CMD_QUERY_REPLY);
            queryResp.dest_id = m.source_id;
            queryResp.source_id = global_broker_id_local;
            queryResp.messageID = m.messageID;

            queryResp.payload = "#invalid";
            if (queryResp.dest_id == global_broker_id_local) {
                ActiveQueries.setDelayedValue(m.messageID, queryResp.payload);
            } else {
                transmit(getRoute(queryResp.dest_id), queryResp);
            }
        } else {
            transmit(route, m);
        }
    }
}

void CoreBroker::processQueryResponse(const ActionMessage& m)
{
    switch (m.counter) {
        case 0:
        default:
            ActiveQueries.setDelayedValue(m.messageID, m.payload);
            break;
        case 2:
            if (fedMap.addComponent(m.payload, m.messageID)) {
                if (fedMapRequestors.size() == 1) {
                    if (fedMapRequestors.front().dest_id == global_broker_id_local) {
                        ActiveQueries.setDelayedValue(
                            fedMapRequestors.front().messageID, fedMap.generate());
                    } else {
                        fedMapRequestors.front().payload = fedMap.generate();
                        routeMessage(fedMapRequestors.front());
                    }
                } else {
                    auto str = fedMap.generate();
                    for (auto& resp : fedMapRequestors) {
                        if (resp.dest_id == global_broker_id_local) {
                            ActiveQueries.setDelayedValue(resp.messageID, str);
                        } else {
                            resp.payload = str;
                            routeMessage(resp);
                        }
                    }
                }
                fedMapRequestors.clear();
            }
            break;
        case 4:
            if (depMap.addComponent(m.payload, m.messageID)) {
                if (depMapRequestors.size() == 1) {
                    if (depMapRequestors.front().dest_id == global_broker_id_local) {
                        ActiveQueries.setDelayedValue(
                            depMapRequestors.front().messageID, depMap.generate());
                    } else {
                        depMapRequestors.front().payload = depMap.generate();
                        routeMessage(std::move(depMapRequestors.front()));
                    }
                } else {
                    auto str = depMap.generate();
                    for (auto& resp : depMapRequestors) {
                        if (resp.dest_id == global_broker_id_local) {
                            ActiveQueries.setDelayedValue(resp.messageID, str);
                        } else {
                            resp.payload = str;
                            routeMessage(std::move(resp));
                        }
                    }
                }
                depMapRequestors.clear();
            }
            break;
    }
}

void CoreBroker::checkDependencies()
{
    if (isRootc) {
        for (const auto& newdep : delayedDependencies) {
            auto depfed = _federates.find(newdep.first);
            if (depfed != _federates.end()) {
                ActionMessage addDep(CMD_ADD_DEPENDENCY, newdep.second, depfed->global_id);
                routeMessage(addDep);
                addDep = ActionMessage(CMD_ADD_DEPENDENT, depfed->global_id, newdep.second);
                routeMessage(addDep);
            } else {
                ActionMessage logWarning(CMD_LOG, parent_broker_id, newdep.second);
                logWarning.messageID = warning;
                logWarning.payload =
                    "unable to locate " + newdep.first + " to establish dependency";
                routeMessage(logWarning);
            }
        }

        if (timeCoord->getDependents().size() == 1) { // if there is just one dependency remove it
            auto depid = timeCoord->getDependents()[0];
            auto dependencies = timeCoord->getDependencies();
            if (dependencies.size() == 1) {
                if (dependencies.front() != depid) {
                    ActionMessage adddep(CMD_ADD_DEPENDENT);
                    adddep.source_id = depid;
                    ActionMessage rmdep(CMD_REMOVE_DEPENDENT);
                    rmdep.source_id = global_broker_id_local;
                    routeMessage(adddep, dependencies.front());
                    routeMessage(rmdep, dependencies.front());

                    adddep.setAction(CMD_ADD_DEPENDENCY);
                    adddep.source_id = dependencies.front();
                    rmdep.setAction(CMD_REMOVE_DEPENDENCY);
                    routeMessage(adddep, depid);
                    routeMessage(rmdep, depid);

                    timeCoord->removeDependency(dependencies.front());
                    timeCoord->removeDependent(depid);
                } else {
                    ActionMessage rmdep(CMD_REMOVE_INTERDEPENDENCY);
                    rmdep.source_id = global_broker_id_local;

                    routeMessage(rmdep, depid);
                    timeCoord->removeDependency(depid);
                    timeCoord->removeDependent(depid);
                }
            }
        }
    } else {
        // if there is more than 2 dependents(higher broker + 2 or more other objects then we need to be a
        // timeCoordinator
        if (timeCoord->getDependents().size() > 2) {
            return;
        }

        global_federate_id fedid;
        int localcnt = 0;
        for (auto& dep : timeCoord->getDependents()) {
            if (dep != higher_broker_id) {
                ++localcnt;
                fedid = dep;
            }
        }
        if (localcnt != 1) {
            return;
        }
        // remove the core from the time dependency chain
        timeCoord->removeDependency(higher_broker_id);
        timeCoord->removeDependency(fedid);
        timeCoord->removeDependent(higher_broker_id);
        timeCoord->removeDependent(fedid);

        ActionMessage rmdep(CMD_REMOVE_INTERDEPENDENCY);

        rmdep.source_id = global_broker_id_local;
        routeMessage(rmdep, higher_broker_id);
        routeMessage(rmdep, fedid);

        ActionMessage adddep(CMD_ADD_INTERDEPENDENCY);
        adddep.source_id = fedid;
        routeMessage(adddep, higher_broker_id);
        adddep.source_id = higher_broker_id;
        routeMessage(adddep, fedid);
    }
}
bool CoreBroker::allInitReady() const
{
    // the federate count must be greater than the min size
    if (static_cast<decltype(minFederateCount)>(_federates.size()) < minFederateCount) {
        return false;
    }
    if (static_cast<decltype(minBrokerCount)>(_brokers.size()) < minBrokerCount) {
        return false;
    }

    return std::all_of(_brokers.begin(), _brokers.end(), [](const auto& brk) {
        return ((brk._nonLocal) || (brk._initRequested));
    });
}

bool CoreBroker::allDisconnected() const
{
    return std::all_of(_brokers.begin(), _brokers.end(), [](const auto& brk) {
        return ((brk._nonLocal) || (brk.isDisconnected));
    });
}

} // namespace helics
