/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "CoreBroker.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "../common/fmt_format.h"
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
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace helics {

constexpr char universalKey[] = "**";

const std::string& state_string(connection_state state)
{
    static const std::string c1{"connected"};
    static const std::string init{"init_requested"};
    static const std::string operating{"operating"};
    static const std::string estate{"error"};
    static const std::string dis{"disconnected"};
    switch (state) {
        case connection_state::operating:
            return operating;
        case connection_state::init_requested:
            return init;
        case connection_state::connected:
            return c1;
        case connection_state::request_disconnect:
        case connection_state::disconnected:
            return dis;
        case connection_state::error:
        default:
            return estate;
    }
}

CoreBroker::~CoreBroker()
{
    std::lock_guard<std::mutex> lock(name_mutex_);
    // make sure everything is synchronized
}

void CoreBroker::setIdentifier(const std::string& name)
{
    if (getBrokerState() <= broker_state_t::connecting)  // can't be changed after initialization
    {
        std::lock_guard<std::mutex> lock(name_mutex_);
        identifier = name;
    }
}

const std::string& CoreBroker::getAddress() const
{
    if ((getBrokerState() != broker_state_t::connected) || (address.empty())) {
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
                                          parent_route_id;  // zero is the default route
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
    if (index > 2) {  // this is an atomic operation if the nextAirLock was not adjusted this could
                      // result in an out of bounds
        // exception if this check were not done
        index %= 2;
    }
    if (index == 2) {
        decltype(index) exp = 3;

        while (exp > 2) {  // doing a lock free modulus we need to make sure the nextAirLock<4
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

void CoreBroker::addDestinationFilterToEndpoint(const std::string& filter,
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
    const auto& endpointName = mess.getString(targetStringLoc);
    auto* eptInfo = handles.getEndpoint(endpointName);
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
    auto cstate = getBrokerState();
    return (cstate != broker_state_t::created && cstate < broker_state_t::operating &&
            !haltOperations &&
            (maxFederateCount == (std::numeric_limits<int32_t>::max)() ||
             getCountableFederates() < maxFederateCount));
}

void CoreBroker::processPriorityCommand(ActionMessage&& command)
{
    // deal with a few types of message immediately
    LOG_TRACE(global_broker_id_local,
              getIdentifier(),
              fmt::format("|| priority_cmd:{} from {}",
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
            if (!checkActionFlag(command, non_counting_flag) &&
                getCountableFederates() >= maxFederateCount) {
                ActionMessage badInit(CMD_FED_ACK);
                setActionFlag(badInit, error_flag);
                badInit.source_id = global_broker_id_local;
                badInit.messageID = max_federate_count_exceeded;
                badInit.name = command.name;
                transmit(getRoute(command.source_id), badInit);
                return;
            }
            if (getBrokerState() != broker_state_t::operating) {
                if (allInitReady()) {
                    ActionMessage noInit(CMD_INIT_NOT_READY);
                    noInit.source_id = global_broker_id_local;
                    transmit(parent_route_id, noInit);
                }
            } else {
                // we are initialized already
                ActionMessage badInit(CMD_FED_ACK);
                setActionFlag(badInit, error_flag);
                badInit.source_id = global_broker_id_local;
                badInit.messageID = already_init_error_code;
                badInit.name = command.name;
                transmit(getRoute(command.source_id), badInit);
                return;
            }
            // this checks for duplicate federate names
            if (_federates.find(command.name) != _federates.end()) {
                ActionMessage badName(CMD_FED_ACK);
                setActionFlag(badName, error_flag);
                badName.source_id = global_broker_id_local;
                badName.messageID = duplicate_federate_name_error_code;
                badName.name = command.name;
                transmit(getRoute(command.source_id), badName);
                return;
            }
            _federates.insert(command.name, no_search, command.name);
            _federates.back().route = getRoute(command.source_id);
            _federates.back().parent = command.source_id;
            if (checkActionFlag(command, non_counting_flag)) {
                _federates.back().nonCounting = true;
            }
            if (checkActionFlag(command, child_flag)) {
                _federates.back().global_id = global_federate_id(command.getExtraData());
                _federates.addSearchTermForIndex(_federates.back().global_id,
                                                 _federates.size() - 1);
            } else if (isRootc) {
                _federates.back().global_id = global_federate_id(
                    static_cast<global_federate_id::base_type>(_federates.size()) - 1 +
                    global_federate_id_shift);
                _federates.addSearchTermForIndex(_federates.back().global_id,
                                                 static_cast<size_t>(
                                                     _federates.back().global_id.baseValue()) -
                                                     global_federate_id_shift);
            }
            if (!isRootc) {
                if (global_broker_id_local.isValid()) {
                    command.source_id = global_broker_id_local;
                    transmit(parent_route_id, command);
                } else {
                    // delay the response if we are not fully registered yet
                    delayTransmitQueue.push(command);
                }
            } else {
                auto route_id = _federates.back().route;
                auto global_fedid = _federates.back().global_id;

                routing_table.emplace(global_fedid, route_id);
                // don't bother with the federate_table
                // transmit the response
                ActionMessage fedReply(CMD_FED_ACK);
                fedReply.source_id = global_broker_id_local;
                fedReply.dest_id = global_fedid;
                fedReply.name = command.name;
                if (checkActionFlag(command, child_flag)) {
                    setActionFlag(fedReply, child_flag);
                }
                transmit(route_id, fedReply);
                LOG_CONNECTIONS(global_broker_id_local,
                                getIdentifier(),
                                fmt::format("registering federate {}({}) on route {}",
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
            if (command.counter > 0) {  // this indicates it is a resend
                auto brk = _brokers.find(command.name);
                if (brk != _brokers.end()) {
                    // we would get this if the ack didn't go through for some reason
                    brk->route = route_id{routeCount++};
                    addRoute(brk->route,
                             command.getExtraData(),
                             command.getString(targetStringLoc));
                    routing_table[brk->global_id] = brk->route;

                    // sending the response message
                    ActionMessage brokerReply(CMD_BROKER_ACK);
                    brokerReply.source_id = global_broker_id_local;  // source is global root
                    brokerReply.dest_id = brk->global_id;  // the new id
                    brokerReply.name = command.name;  // the identifier of the broker
                    if (no_ping) {
                        setActionFlag(brokerReply, slow_responding_flag);
                    }
                    transmit(brk->route, brokerReply);
                    return;
                }
            }
            if (static_cast<decltype(maxBrokerCount)>(_brokers.size()) >= maxBrokerCount) {
                route_id newroute;
                bool route_created = false;
                if ((!command.source_id.isValid()) || (command.source_id == parent_broker_id)) {
                    newroute = route_id(routeCount++);
                    addRoute(newroute, command.getExtraData(), command.getString(targetStringLoc));
                    route_created = true;
                } else {
                    newroute = getRoute(command.source_id);
                }
                ActionMessage badInit(CMD_BROKER_ACK);
                setActionFlag(badInit, error_flag);
                badInit.source_id = global_broker_id_local;
                badInit.name = command.name;
                badInit.messageID = max_broker_count_exceeded;
                transmit(newroute, badInit);

                if (route_created) {
                    removeRoute(newroute);
                }
                return;
            }
            if (getBrokerState() != broker_state_t::operating) {
                if (allInitReady()) {
                    // send an init not ready as we were ready now we are not
                    ActionMessage noInit(CMD_INIT_NOT_READY);
                    noInit.source_id = global_broker_id_local;
                    transmit(parent_route_id, noInit);
                }
            } else {
                // we are initialized already
                route_id newroute;
                bool route_created = false;
                if ((!command.source_id.isValid()) || (command.source_id == parent_broker_id)) {
                    newroute = route_id(routeCount++);
                    addRoute(newroute, command.getExtraData(), command.getString(targetStringLoc));
                    route_created = true;
                } else {
                    newroute = getRoute(command.source_id);
                }
                ActionMessage badInit(CMD_BROKER_ACK);
                setActionFlag(badInit, error_flag);
                badInit.source_id = global_broker_id_local;
                badInit.name = command.name;
                badInit.messageID = already_init_error_code;
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
                    addRoute(newroute, command.getExtraData(), command.getString(targetStringLoc));
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
                    addRoute(newroute, command.getExtraData(), command.getString(targetStringLoc));
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
                // TODO(PT): this will need to be updated when we enable mesh routing
                _brokers.back().route = route_id{routeCount++};
                addRoute(_brokers.back().route,
                         command.getExtraData(),
                         command.getString(targetStringLoc));
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
                _brokers.back().global_id =
                    global_broker_id(static_cast<global_broker_id::base_type>(_brokers.size()) - 1 +
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
                brokerReply.source_id = global_broker_id_local;  // source is global root
                brokerReply.dest_id = global_brkid;  // the new id
                brokerReply.name = command.name;  // the identifier of the broker
                if (no_ping) {
                    setActionFlag(brokerReply, slow_responding_flag);
                }
                transmit(route, brokerReply);
                LOG_CONNECTIONS(global_broker_id_local,
                                getIdentifier(),
                                fmt::format("registering broker {}({}) on route {}",
                                            command.name,
                                            global_brkid.baseValue(),
                                            route.baseValue()));
            }
        } break;
        case CMD_FED_ACK: {  // we can't be root if we got one of these
            auto fed = _federates.find(command.name);
            if (fed != _federates.end()) {
                auto route = fed->route;
                if (!fed->global_id.isValid()) {
                    fed->global_id = command.dest_id;
                    _federates.addSearchTerm(command.dest_id, fed->name);
                }
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
        case CMD_BROKER_ACK: {  // we can't be root if we got one of these
            if (command.name == identifier) {
                if (checkActionFlag(command, error_flag)) {
                    // generate an error message
                    LOG_ERROR(global_broker_id_local,
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
                command.source_id = global_broker_id_local;  // we want the intermediate broker to
                                                             // change the source_id
                transmit(route, command);
            } else {
                _brokers.insert(command.name, global_broker_id(command.dest_id), command.name);
                _brokers.back().route = getRoute(command.source_id);
                _brokers.back().global_id = global_broker_id(command.dest_id);
                routing_table.emplace(broker->global_id, _brokers.back().route);
            }
        } break;
        case CMD_PRIORITY_DISCONNECT: {
            auto* brk = getBrokerById(global_broker_id(command.source_id));
            if (brk != nullptr) {
                brk->state = connection_state::disconnected;
            }
            if (getAllConnectionState() >= connection_state::disconnected) {
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
        case CMD_QUERY:
        case CMD_QUERY_REPLY:
        case CMD_SET_GLOBAL:
            processQueryCommand(command);
            break;

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
    for (const auto& hand : handles) {
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
        getCountableFederates(),
        minFederateCount,
        std::count_if(_brokers.begin(),
                      _brokers.end(),
                      [](auto& brk) { return !static_cast<bool>(brk._core); }),
        std::count_if(_brokers.begin(),
                      _brokers.end(),
                      [](auto& brk) { return static_cast<bool>(brk._core); }),
        minBrokerCount,
        pubs,
        ipts,
        epts,
        filt);
    return output;
}

void CoreBroker::generateTimeBarrier(ActionMessage& m)
{
    if (checkActionFlag(m, cancel_flag)) {
        ActionMessage cancelBarrier(CMD_TIME_BARRIER_CLEAR);
        cancelBarrier.source_id = global_broker_id_local;
        cancelBarrier.messageID = global_broker_id_local.baseValue();
        broadcast(cancelBarrier);
        return;
    }
    m.setAction(CMD_TIME_BARRIER);
    m.source_id = global_broker_id_local;
    m.messageID = global_broker_id_local.baseValue();
    // time should already be set
    broadcast(m);
}

int CoreBroker::generateMapObjectCounter() const
{
    int result = static_cast<int>(getBrokerState());
    for (const auto& brk : _brokers) {
        result += static_cast<int>(brk.state);
    }
    for (const auto& fed : _federates) {
        result += static_cast<int>(fed.state);
    }
    result += static_cast<int>(handles.size());
    return result;
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
            obj.state = connection_state::disconnected;
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
        if (brk.state < connection_state::disconnected) {
            if (brk.parent == global_broker_id_local) {
                this->routeMessage(bye, brk.global_id);
                brk.state = connection_state::disconnected;
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

void CoreBroker::sendErrorToImmediateBrokers(int errorCode)
{
    ActionMessage errorCom(CMD_ERROR);
    errorCom.messageID = errorCode;
    broadcast(errorCom);
}

void CoreBroker::processCommand(ActionMessage&& command)
{
    LOG_TRACE(global_broker_id_local,
              getIdentifier(),
              fmt::format("|| cmd:{} from {} to {}",
                          prettyPrintString(command),
                          command.source_id.baseValue(),
                          command.dest_id.baseValue()));
    switch (command.action()) {
        case CMD_IGNORE:
        case CMD_PROTOCOL:
            break;

        case CMD_TICK:
            if (getBrokerState() == broker_state_t::operating) {
                timeoutMon->tick(this);
                LOG_SUMMARY(global_broker_id_local, getIdentifier(), " broker tick");
            }
            checkQueryTimeouts();
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
            LOG_WARNING(global_broker_id_local,
                        getIdentifier(),
                        "disconnecting from check connections");
            break;
        case CMD_CONNECTION_ERROR:
            // if anyone else as has terminated assume they finalized and the connection was
            // lost
            if (command.dest_id == global_broker_id_local) {
                bool partDisconnected{false};
                bool ignore{false};
                for (const auto& brk : _brokers) {
                    if (brk.state == connection_state::disconnected) {
                        partDisconnected = true;
                    }
                    if (brk.state == connection_state::disconnected &&
                        brk.global_id == command.source_id) {
                        // the broker in question is already disconnected, ignore this
                        ignore = true;
                        break;
                    }
                }
                if (ignore) {
                    break;
                }
                if (partDisconnected) {  // we are going to assume it disconnected just assume
                                         // broker even though it may be a core, there
                    // probably isn't any difference for this purpose
                    LOG_CONNECTIONS(global_broker_id_local,
                                    getIdentifier(),
                                    fmt::format("disconnecting {} from communication timeout",
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
                        setBrokerState(broker_state_t::errored);
                        addActionMessage(CMD_USER_DISCONNECT);
                        // TODO(PT): this needs something better but this does
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
            auto* brk = getBrokerById(static_cast<global_broker_id>(command.source_id));
            if (brk != nullptr) {
                brk->state = connection_state::init_requested;
            }
            if (allInitReady()) {
                if (isRootc) {
                    LOG_TIMING(global_broker_id_local, "root", "entering initialization mode");
                    LOG_SUMMARY(global_broker_id_local, "root", generateFederationSummary());
                    executeInitializationOperations();
                } else {
                    LOG_TIMING(global_broker_id_local,
                               getIdentifier(),
                               "entering initialization mode");
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
            auto* brk = getBrokerById(global_broker_id(command.source_id));
            if (brk != nullptr) {
                brk->state = connection_state::connected;
            }
        } break;
        case CMD_INIT_GRANT:
            if (brokerKey == universalKey) {
                LOG_SUMMARY(global_broker_id_local,
                            getIdentifier(),
                            " Broker started with universal key");
            }
            setBrokerState(broker_state_t::operating);
            for (const auto& brk : _brokers) {
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
            auto* pub = handles.getPublication(command.name);
            if (pub != nullptr) {
                command.name = command.getString(targetStringLoc);
                command.setAction(CMD_ADD_NAMED_INPUT);
                command.setSource(pub->handle);
                checkForNamedInterface(command);
            } else {
                auto* input = handles.getInput(command.getString(targetStringLoc));
                if (input == nullptr) {
                    if (isRootc) {
                        unknownHandles.addDataLink(command.name,
                                                   command.getString(targetStringLoc));
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
            auto* filt = handles.getFilter(command.name);
            if (filt != nullptr) {
                command.name = command.getString(targetStringLoc);
                command.setAction(CMD_ADD_NAMED_ENDPOINT);
                command.setSource(filt->handle);
                if (checkActionFlag(*filt, clone_flag)) {
                    setActionFlag(command, clone_flag);
                }
                checkForNamedInterface(command);
            } else {
                auto* ept = handles.getEndpoint(command.getString(targetStringLoc));
                if (ept == nullptr) {
                    if (isRootc) {
                        if (checkActionFlag(command, destination_target)) {
                            unknownHandles.addDestinationFilterLink(command.name,
                                                                    command.getString(
                                                                        targetStringLoc));
                        } else {
                            unknownHandles.addSourceFilterLink(command.name,
                                                               command.getString(targetStringLoc));
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
                        ActionMessage dis((brk._core) ? CMD_DISCONNECT_CORE_ACK :
                                                        CMD_DISCONNECT_BROKER_ACK);
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
                fed->state = connection_state::disconnected;
            }
            if (!isRootc) {
                transmit(parent_route_id, command);
            } else if (getBrokerState() < broker_state_t::operating) {
                command.setAction(CMD_BROADCAST_DISCONNECT);
                broadcast(command);
                unknownHandles.clearFederateUnknowns(command.source_id);
            }
        } break;
        case CMD_STOP:
            if ((getAllConnectionState() <
                 connection_state::disconnected)) {  // only send a disconnect message if we
                                                     // haven't done so already
                timeCoord->disconnect();
                if (!isRootc) {
                    ActionMessage m(CMD_DISCONNECT);
                    m.source_id = global_broker_id_local;
                    transmit(parent_route_id, m);
                }
            }
            activeQueries.fulfillAllPromises("#disconnected");
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
        case CMD_TIME_BARRIER:
        case CMD_TIME_BARRIER_CLEAR:
            broadcast(command);
            break;
        case CMD_TIME_BARRIER_REQUEST:
            generateTimeBarrier(command);
            break;
        case CMD_TIME_REQUEST:
        case CMD_TIME_GRANT:
            if ((command.source_id == global_broker_id_local) &&
                (command.dest_id == parent_broker_id)) {
                LOG_TIMING(global_broker_id_local,
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
                if (route == parent_route_id && isRootc && command.action() == CMD_SEND_MESSAGE) {
                    bool optional_flag_set = checkActionFlag(command, optional_flag);
                    bool required_flag_set = checkActionFlag(command, required_flag);

                    if (!optional_flag_set) {
                        ActionMessage warn(required_flag_set ? CMD_ERROR : CMD_WARNING);
                        warn.source_id = global_broker_id_local;
                        warn.dest_id = command.source_id;
                        warn.payload =
                            fmt::format("{} is not a known endpoint, message from {} dropped",
                                        command.getString(targetStringLoc),
                                        command.getString(sourceStringLoc));
                        transmit(getRoute(warn.dest_id), warn);
                    }

                } else {
                    transmit(route, command);
                }
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
        case CMD_LOCAL_ERROR:
        case CMD_GLOBAL_ERROR:
            processError(command);
            break;
        case CMD_REG_PUB:
            if ((!isRootc) && (command.dest_id != parent_broker_id)) {
                routeMessage(command);
                // break;
            }
            addPublication(command);
            break;
        case CMD_REG_INPUT:
            if ((!isRootc) && (command.dest_id != parent_broker_id)) {
                routeMessage(command);
                // break;
            }
            addInput(command);
            break;
        case CMD_REG_ENDPOINT:
            if ((!isRootc) && (command.dest_id != parent_broker_id)) {
                routeMessage(command);
                // break;
            }
            addEndpoint(command);
            break;
        case CMD_REG_FILTER:
            if ((!isRootc) && (command.dest_id != parent_broker_id)) {
                routeMessage(command);
                // break;
            }
            addFilter(command);
            break;
        case CMD_CLOSE_INTERFACE:
            if ((!isRootc) && (command.dest_id != parent_broker_id)) {
                routeMessage(command);
                // break;
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
        case CMD_BROKER_QUERY_ORDERED:
        case CMD_QUERY_ORDERED:
        case CMD_QUERY_REPLY_ORDERED:
            processQueryCommand(command);
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
            if (checkActionFlag(cmd, indicator_flag)) {
                setTickForwarding(TickForwardingReasons::ping_response, true);
            }
            break;
        default:
            break;
    }
}

void CoreBroker::checkForNamedInterface(ActionMessage& command)
{
    bool foundInterface = false;
    switch (command.action()) {
        case CMD_ADD_NAMED_PUBLICATION: {
            auto* pub = handles.getPublication(command.name);
            if (pub != nullptr) {
                auto fed = _federates.find(pub->getFederateId());
                if (fed->state < connection_state::error) {
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
            auto* inp = handles.getInput(command.name);
            if (inp != nullptr) {
                auto fed = _federates.find(inp->getFederateId());
                if (fed->state < connection_state::error) {
                    command.setAction(CMD_ADD_PUBLISHER);
                    command.setDestination(inp->handle);
                    auto* pub = handles.findHandle(command.getSource());
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
            auto* filt = handles.getFilter(command.name);
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
            auto* ept = handles.getEndpoint(command.name);
            if (ept != nullptr) {
                auto fed = _federates.find(ept->getFederateId());
                if (fed->state < connection_state::error) {
                    command.setAction(CMD_ADD_FILTER);
                    command.setDestination(ept->handle);
                    command.name.clear();
                    auto* filt = handles.findHandle(command.getSource());
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
                    unknownHandles.addUnknownPublication(command.name,
                                                         command.getSource(),
                                                         command.flags);
                    break;
                case CMD_ADD_NAMED_INPUT:
                    unknownHandles.addUnknownInput(command.name,
                                                   command.getSource(),
                                                   command.flags);
                    if (!command.getStringData().empty()) {
                        auto* pub = handles.findHandle(command.getSource());
                        if (pub == nullptr) {
                            // an anonymous publisher is adding an input
                            auto& apub = handles.addHandle(command.source_id,
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
                    unknownHandles.addUnknownEndpoint(command.name,
                                                      command.getSource(),
                                                      command.flags);
                    if (!command.getStringData().empty()) {
                        auto* filt = handles.findHandle(command.getSource());
                        if (filt == nullptr) {
                            // an anonymous filter is adding an endpoint
                            auto& afilt = handles.addHandle(command.source_id,
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
                    unknownHandles.addUnknownFilter(command.name,
                                                    command.getSource(),
                                                    command.flags);
                    break;
                default:
                    LOG_WARNING(global_broker_id_local,
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
            auto* pub = handles.getPublication(command.name);
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
            auto* inp = handles.getInput(command.name);
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
            auto* filt = handles.getFilter(command.name);
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
            auto* ept = handles.getEndpoint(command.name);
            if (ept != nullptr) {
                command.setAction(CMD_REMOVE_FILTER);
                command.setDestination(ept->handle);
                command.name.clear();
                routeMessage(command);
                command.setAction(CMD_REMOVE_ENDPOINT);
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
            LOG_WARNING(global_broker_id_local,
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

void CoreBroker::propagateError(ActionMessage&& cmd)
{
    LOG_ERROR(global_broker_id_local, getIdentifier(), cmd.payload);
    if (cmd.action() == CMD_LOCAL_ERROR) {
        if (terminate_on_error) {
            LOG_ERROR(global_broker_id_local,
                      getIdentifier(),
                      "Error Escalation: Federation terminating");
            cmd.setAction(CMD_GLOBAL_ERROR);
            setErrorState(cmd.messageID, cmd.payload);
            broadcast(cmd);
            transmitToParent(std::move(cmd));
            return;
        }
    }
    routeMessage(std::move(cmd));
}

void CoreBroker::addPublication(ActionMessage& m)
{
    // detect duplicate publications
    if (handles.getPublication(m.name) != nullptr) {
        ActionMessage eret(CMD_LOCAL_ERROR, global_broker_id_local, m.source_id);
        eret.dest_handle = m.source_handle;
        eret.messageID = defs::errors::registration_failure;
        eret.payload = "Duplicate publication names (" + m.name + ")";
        propagateError(std::move(eret));
        return;
    }
    auto& pub = handles.addHandle(m.source_id,
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
        ActionMessage eret(CMD_LOCAL_ERROR, global_broker_id_local, m.source_id);
        eret.dest_handle = m.source_handle;
        eret.messageID = defs::errors::registration_failure;
        eret.payload = "Duplicate input names (" + m.name + ")";
        propagateError(std::move(eret));
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
        ActionMessage eret(CMD_LOCAL_ERROR, global_broker_id_local, m.source_id);
        eret.dest_handle = m.source_handle;
        eret.messageID = defs::errors::registration_failure;
        eret.payload = "Duplicate endpoint names (" + m.name + ")";
        propagateError(std::move(eret));
        return;
    }
    auto& ept = handles.addHandle(m.source_id,
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
                ActionMessage add(CMD_ADD_INTERDEPENDENCY,
                                  global_broker_id_local,
                                  higher_broker_id);
                setActionFlag(add, child_flag);
                transmit(parent_route_id, add);

                timeCoord->addDependent(higher_broker_id);
                timeCoord->setAsParent(higher_broker_id);
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
        ActionMessage eret(CMD_LOCAL_ERROR, global_broker_id_local, m.source_id);
        eret.dest_handle = m.source_handle;
        eret.messageID = defs::errors::registration_failure;
        eret.payload = "Duplicate filter names (" + m.name + ")";
        propagateError(std::move(eret));
        return;
    }

    auto& filt = handles.addHandle(m.source_id,
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
                setActionFlag(add, child_flag);
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
    if (transitionBrokerState(broker_state_t::created, broker_state_t::configuring)) {
        auto result = parseArgs(configureString);
        if (result != 0) {
            setBrokerState(broker_state_t::created);
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
    if (transitionBrokerState(broker_state_t::created, broker_state_t::configuring)) {
        auto result = parseArgs(argc, argv);
        if (result != 0) {
            setBrokerState(broker_state_t::created);
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
    if (transitionBrokerState(broker_state_t::created, broker_state_t::configuring)) {
        auto result = parseArgs(std::move(args));
        if (result != 0) {
            setBrokerState(broker_state_t::created);
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
    if (getBrokerState() < broker_state_t::connected) {
        _isRoot = true;
        global_id = global_broker_id(1);
    }
}

bool CoreBroker::connect()
{
    if (getBrokerState() < broker_state_t::connected) {
        if (transitionBrokerState(broker_state_t::configured, broker_state_t::connecting)) {
            LOG_CONNECTIONS(parent_broker_id, getIdentifier(), "connecting");
            timeoutMon->setTimeout(std::chrono::milliseconds(timeout));
            auto res = brokerConnect();
            if (res) {
                disconnection.activate();
                setBrokerState(broker_state_t::connected);
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
                LOG_CONNECTIONS(parent_broker_id,
                                getIdentifier(),
                                fmt::format("||connected on {}", getAddress()));
            } else {
                setBrokerState(broker_state_t::configured);
            }
            return res;
        }
        if (getBrokerState() == broker_state_t::connecting) {
            while (getBrokerState() == broker_state_t::connecting) {
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
        }
    }
    return isConnected();
}

void CoreBroker::setTimeBarrier(Time barrierTime)
{
    if (barrierTime == Time::maxVal()) {
        return clearTimeBarrier();
    }
    ActionMessage tbarrier(CMD_TIME_BARRIER_REQUEST);
    tbarrier.source_id = global_id.load();
    tbarrier.actionTime = barrierTime;
    addActionMessage(tbarrier);
}

void CoreBroker::clearTimeBarrier()
{
    ActionMessage tbarrier(CMD_TIME_BARRIER_REQUEST);
    tbarrier.source_id = global_id.load();
    tbarrier.actionTime = Time::maxVal();
    setActionFlag(tbarrier, cancel_flag);
    addActionMessage(tbarrier);
}

void CoreBroker::globalError(int32_t errorCode, const std::string& errorString)
{
    ActionMessage m(CMD_GLOBAL_ERROR);
    m.source_id = getGlobalId();
    m.messageID = errorCode;
    m.payload = errorString;
    addActionMessage(m);
}

bool CoreBroker::isConnected() const
{
    auto state = getBrokerState();
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
    auto cBrokerState = getBrokerState();
    if ((cBrokerState == broker_state_t::terminating) ||
        (cBrokerState == broker_state_t::terminated)) {
        return;
    }
    if (cBrokerState > broker_state_t::configured) {
        LOG_CONNECTIONS(parent_broker_id, getIdentifier(), "||disconnecting");
        setBrokerState(broker_state_t::terminating);
        brokerDisconnect();
    }
    setBrokerState(broker_state_t::terminated);

    if (!skipUnregister) {
        unregister();
    }
    disconnection.trigger();
}

void CoreBroker::unregister()
{
    /*We need to ensure that the destructor is not called immediately upon calling unregister
    otherwise this would be a mess and probably cause segmentation faults so we capture it in a
    local variable that will be destroyed on function exit
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
        LOG_WARNING(global_id.load(),
                    getIdentifier(),
                    "waiting on disconnect: current state=" + brokerStateName(getBrokerState()));
        if (cnt % 4 == 0) {
            if (!isRunning()) {
                LOG_WARNING(
                    global_id.load(),
                    getIdentifier(),
                    "main loop is stopped but have not received disconnect notice, assuming disconnected");
                return;
            }
            LOG_WARNING(global_id.load(),
                        getIdentifier(),
                        fmt::format("sending disconnect again; total message count = {}",
                                    currentMessageCounter()));
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
    for (const auto& broker : _brokers) {
        if ((!broker._nonLocal) && (broker.state < connection_state::disconnected)) {
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
                unknownHandles.processRequiredUnknowns([this, &eMiss](const std::string& target,
                                                                      char type,
                                                                      global_handle handle) {
                    switch (type) {
                        case 'p':
                            eMiss.payload =
                                fmt::format("Unable to connect to required publication target {}",
                                            target);
                            LOG_ERROR(parent_broker_id, getIdentifier(), eMiss.payload);
                            break;
                        case 'i':
                            eMiss.payload =
                                fmt::format("Unable to connect to required input target {}",
                                            target);
                            LOG_ERROR(parent_broker_id, getIdentifier(), eMiss.payload);
                            break;
                        case 'f':
                            eMiss.payload =
                                fmt::format("Unable to connect to required filter target {}",
                                            target);
                            LOG_ERROR(parent_broker_id, getIdentifier(), eMiss.payload);
                            break;
                        case 'e':
                            eMiss.payload =
                                fmt::format("Unable to connect to required endpoint target {}",
                                            target);
                            LOG_ERROR(parent_broker_id, getIdentifier(), eMiss.payload);
                            break;
                        default:
                            // LCOV_EXCL_START
                            eMiss.payload =
                                fmt::format("Unable to connect to required unknown target {}",
                                            target);
                            LOG_ERROR(parent_broker_id, getIdentifier(), eMiss.payload);
                            break;
                            // LCOV_EXCL_STOP
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
                        default:
                            // LCOV_EXCL_START
                            wMiss.payload =
                                fmt::format("Unable to connect to undefined target {}", target);
                            LOG_WARNING(parent_broker_id, getIdentifier(), wMiss.payload);
                            break;
                            // LCOV_EXCL_STOP
                    }
                    wMiss.setDestination(handle);
                    routeMessage(wMiss);
                });
        }
    }

    ActionMessage m(CMD_INIT_GRANT);
    m.source_id = global_broker_id_local;
    setBrokerState(broker_state_t::operating);
    broadcast(m);
    timeCoord->enteringExecMode();
    auto res = timeCoord->checkExecEntry();
    if (res == message_processing_result::next_step) {
        enteredExecutionMode = true;
    }
    logFlush();
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
        auto* pub = handles.findHandle(target.first);
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
    for (const auto& sub : subHandles) {
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
    for (const auto& sub : Pubtargets) {
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
    for (const auto& target : Handles) {
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
    for (const auto& target : Handles) {
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
    for (const auto& target : FiltDestTargets) {
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
    for (const auto& target : FiltSourceTargets) {
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

void CoreBroker::processError(ActionMessage& command)
{
    sendToLogger(command.source_id, log_level::error, std::string(), command.payload);
    if (command.source_id == global_broker_id_local) {
        setBrokerState(broker_state_t::errored);
        broadcast(command);
        if (!isRootc) {
            command.setAction(CMD_LOCAL_ERROR);
            transmit(parent_route_id, std::move(command));
        }
        return;
    }

    if (command.source_id == parent_broker_id || command.source_id == root_broker_id) {
        setBrokerState(broker_state_t::errored);
        broadcast(command);
    }

    auto* brk = getBrokerById(global_broker_id(command.source_id));
    if (brk == nullptr) {
        auto fed = _federates.find(command.source_id);
        if (fed != _federates.end()) {
            fed->state = connection_state::error;
        }
    } else {
        brk->state = connection_state::error;
    }

    switch (command.action()) {
        case CMD_LOCAL_ERROR:
        case CMD_ERROR:
            if (terminate_on_error) {
                // upgrade the error to a global error and reprocess
                command.setAction(CMD_GLOBAL_ERROR);
                processError(command);
                return;
            }
            if (!(isRootc || command.dest_id == global_broker_id_local ||
                  command.dest_id == parent_broker_id)) {
                transmit(parent_route_id, command);
            }
            if (hasTimeDependency) {
                timeCoord->processTimeMessage(command);
            }
            break;
        case CMD_GLOBAL_ERROR:
            setErrorState(command.messageID, command.payload);
            if (!(isRootc || command.dest_id == global_broker_id_local ||
                  command.dest_id == parent_broker_id)) {
                transmit(parent_route_id, command);
            } else {
                command.source_id = global_broker_id_local;
                broadcast(command);
            }
            break;
        default:
            break;
    }
}

void CoreBroker::processDisconnect(ActionMessage& command)
{
    auto* brk = getBrokerById(global_broker_id(command.source_id));
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
                if (!isRootc) {
                    if (command.source_id == higher_broker_id) {
                        LOG_CONNECTIONS(parent_broker_id,
                                        getIdentifier(),
                                        "got disconnect from parent");
                        sendDisconnect();
                        addActionMessage(CMD_STOP);
                        return;
                    }
                }

                if (brk != nullptr) {
                    LOG_CONNECTIONS(parent_broker_id,
                                    getIdentifier(),
                                    fmt::format("got disconnect from {}({})",
                                                brk->name,
                                                command.source_id.baseValue()));
                    disconnectBroker(*brk);
                }

                if ((getAllConnectionState() >= connection_state::disconnected)) {
                    timeCoord->disconnect();
                    if (!isRootc) {
                        ActionMessage dis(CMD_DISCONNECT);
                        dis.source_id = global_broker_id_local;
                        transmit(parent_route_id, dis);
                    } else {
                        if ((brk != nullptr) && (!brk->_nonLocal)) {
                            if (!checkActionFlag(command, error_flag)) {
                                ActionMessage dis((brk->_core) ? CMD_DISCONNECT_CORE_ACK :
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
                            ActionMessage dis((brk->_core) ? CMD_DISCONNECT_CORE_ACK :
                                                             CMD_DISCONNECT_BROKER_ACK);
                            dis.source_id = global_broker_id_local;
                            dis.dest_id = brk->global_id;
                            transmit(brk->route, dis);
                        }
                        brk->_sent_disconnect_ack = true;
                        if ((!isRootc) && (getBrokerState() < broker_state_t::operating)) {
                            command.setAction((brk->_core) ? CMD_DISCONNECT_CORE :
                                                             CMD_DISCONNECT_BROKER);
                            transmit(parent_route_id, command);
                        }
                        removeRoute(brk->route);
                    } else {
                        if ((!isRootc) && (getBrokerState() < broker_state_t::operating)) {
                            if (brk != nullptr) {
                                command.setAction((brk->_core) ? CMD_DISCONNECT_CORE :
                                                                 CMD_DISCONNECT_BROKER);
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

void CoreBroker::checkInFlightQueries(global_broker_id brkid)
{
    for (auto& mb : mapBuilders) {
        auto& builder = std::get<0>(mb);
        auto& requestors = std::get<1>(mb);
        if (builder.isCompleted()) {
            return;
        }
        if (builder.clearComponents(brkid.baseValue())) {
            auto str = builder.generate();
            for (int ii = 0; ii < static_cast<int>(requestors.size()) - 1; ++ii) {
                if (requestors[ii].dest_id == global_broker_id_local) {
                    activeQueries.setDelayedValue(requestors[ii].messageID, str);
                } else {
                    requestors[ii].payload = str;
                    routeMessage(std::move(requestors[ii]));
                }
            }
            if (requestors.back().dest_id == global_broker_id_local) {
                // TODO(PT) add rvalue reference method
                activeQueries.setDelayedValue(requestors.back().messageID, str);
            } else {
                requestors.back().payload = std::move(str);
                routeMessage(std::move(requestors.back()));
            }

            requestors.clear();
            if (std::get<2>(mb)) {
                builder.reset();
            }
        }
    }
}

void CoreBroker::markAsDisconnected(global_broker_id brkid)
{
    // using regular loop here since dual mapped vector shouldn't produce a modifiable lvalue
    for (size_t ii = 0; ii < _brokers.size(); ++ii) {  // NOLINT
        auto& brk = _brokers[ii];
        if (brk.global_id == brkid) {
            if (brk.state != connection_state::error) {
                brk.state = connection_state::disconnected;
            }
        }
        if (brk.parent == brkid) {
            if (brk.state != connection_state::error) {
                brk.state = connection_state::disconnected;
                markAsDisconnected(brk.global_id);
            }
        }
    }
    for (size_t ii = 0; ii < _federates.size(); ++ii) {  // NOLINT
        auto& fed = _federates[ii];

        if (fed.parent == brkid) {
            if (fed.state != connection_state::error) {
                fed.state = connection_state::disconnected;
            }
        }
    }
}

void CoreBroker::disconnectBroker(BasicBrokerInfo& brk)
{
    markAsDisconnected(brk.global_id);
    checkInFlightQueries(brk.global_id);
    if (getBrokerState() < broker_state_t::operating) {
        if (isRootc) {
            ActionMessage dis(CMD_BROADCAST_DISCONNECT);
            dis.source_id = brk.global_id;
            broadcast(dis);
            unknownHandles.clearFederateUnknowns(brk.global_id);
            if (!brk._core) {
                for (const auto& subbrk : _brokers) {
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
std::string CoreBroker::query(const std::string& target,
                              const std::string& queryStr,
                              helics_sequencing_mode mode)
{
    auto gid = global_id.load();
    if (target == "broker" || target == getIdentifier() || target.empty()) {
        ActionMessage querycmd(mode == helics_sequencing_mode_fast ? CMD_BROKER_QUERY :
                                                                     CMD_BROKER_QUERY_ORDERED);
        querycmd.source_id = querycmd.dest_id = gid;
        auto index = ++queryCounter;
        querycmd.messageID = index;
        querycmd.payload = queryStr;
        auto queryResult = activeQueries.getFuture(index);
        addActionMessage(std::move(querycmd));
        auto ret = queryResult.get();
        activeQueries.finishedWithValue(index);
        return ret;
    }
    if (target == "parent") {
        if (isRootc) {
            return "#na";
        }
        ActionMessage querycmd(mode == helics_sequencing_mode_fast ? CMD_BROKER_QUERY :
                                                                     CMD_BROKER_QUERY_ORDERED);
        querycmd.source_id = gid;
        querycmd.messageID = ++queryCounter;
        querycmd.payload = queryStr;
        auto queryResult = activeQueries.getFuture(querycmd.messageID);
        addActionMessage(querycmd);
        auto ret = queryResult.get();
        activeQueries.finishedWithValue(querycmd.messageID);
        return ret;
    }
    if ((target == "root") || (target == "rootbroker")) {
        ActionMessage querycmd(mode == helics_sequencing_mode_fast ? CMD_BROKER_QUERY :
                                                                     CMD_BROKER_QUERY_ORDERED);
        querycmd.source_id = gid;
        auto index = ++queryCounter;
        querycmd.messageID = index;
        querycmd.payload = queryStr;
        auto queryResult = activeQueries.getFuture(querycmd.messageID);
        transmitToParent(std::move(querycmd));

        auto ret = queryResult.get();
        activeQueries.finishedWithValue(index);
        return ret;
    }

    ActionMessage querycmd(mode == helics_sequencing_mode_fast ? CMD_QUERY : CMD_QUERY_ORDERED);
    querycmd.source_id = gid;
    auto index = ++queryCounter;
    querycmd.messageID = index;
    querycmd.payload = queryStr;
    querycmd.setStringData(target);
    auto queryResult = activeQueries.getFuture(querycmd.messageID);
    transmitToParent(std::move(querycmd));

    auto ret = queryResult.get();
    activeQueries.finishedWithValue(index);
    return ret;

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
// enumeration of subqueries that cascade and need multiple levels of processing
enum subqueries : std::uint16_t {
    general_query = 0,
    federate_map = 1,
    current_time_map = 2,
    dependency_graph = 3,
    data_flow_graph = 4,
    version_all = 5,
    global_state = 6,
    global_time_debugging = 7,
    global_flush = 8,
    global_status = 9
};

static const std::map<std::string, std::pair<std::uint16_t, bool>> mapIndex{
    {"global_time", {current_time_map, true}},
    {"federate_map", {federate_map, false}},
    {"dependency_graph", {dependency_graph, false}},
    {"data_flow_graph", {data_flow_graph, false}},
    {"version_all", {version_all, false}},
    {"global_state", {global_state, true}},
    {"global_time_debugging", {global_time_debugging, true}},
    {"global_status", {global_status, true}},
    {"global_flush", {global_flush, true}}};

std::string CoreBroker::generateQueryAnswer(const std::string& request, bool force_ordering)
{
    if (request == "isinit") {
        return (getBrokerState() >= broker_state_t::operating) ? std::string("true") :
                                                                 std::string("false");
    }
    if (request == "isconnected") {
        return (isConnected()) ? std::string("true") : std::string("false");
    }
    if (request == "name" || request == "identifier") {
        return getIdentifier();
    }
    if (request == "exists") {
        return "true";
    }
    if ((request == "queries") || (request == "available_queries")) {
        return "[isinit;isconnected;name;identifier;address;queries;address;counts;summary;federates;brokers;inputs;endpoints;"
               "publications;filters;federate_map;dependency_graph;counter;data_flow_graph;dependencies;dependson;dependents;"
               "current_time;current_state;global_state;status;global_status;global_time;version;version_all;exists;global_flush]";
    }
    if (request == "address") {
        return getAddress();
    }
    if (request == "version") {
        return versionString;
    }
    if (request == "counter") {
        return fmt::format("{}", generateMapObjectCounter());
    }
    if (request == "status") {
        Json::Value base;
        base["name"] = getIdentifier();
        if (uuid_like) {
            base["uuid"] = getIdentifier();
        }
        base["state"] = brokerStateName(getBrokerState());
        base["status"] = isConnected();
        return generateJsonString(base);
    }
    if (request == "counts") {
        Json::Value base;
        base["name"] = getIdentifier();
        if (uuid_like) {
            base["uuid"] = getIdentifier();
        }
        base["id"] = global_broker_id_local.baseValue();
        if (!isRootc) {
            base["parent"] = higher_broker_id.baseValue();
        }
        base["brokers"] = static_cast<int>(_brokers.size());
        base["federates"] = static_cast<int>(_federates.size());
        base["countable_federates"] = getCountableFederates();
        base["handles"] = static_cast<int>(handles.size());
        return generateJsonString(base);
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
    if (request == "current_state") {
        Json::Value base;
        base["name"] = getIdentifier();
        if (uuid_like) {
            base["uuid"] = getIdentifier();
        }
        base["id"] = global_broker_id_local.baseValue();
        if (!isRootc) {
            base["parent"] = higher_broker_id.baseValue();
        }
        base["state"] = brokerStateName(getBrokerState());
        base["status"] = isConnected();
        base["federates"] = Json::arrayValue;
        for (const auto& fed : _federates) {
            Json::Value fedstate;
            fedstate["name"] = fed.name;
            fedstate["state"] = state_string(fed.state);
            fedstate["id"] = fed.global_id.baseValue();
            base["federates"].append(std::move(fedstate));
        }
        base["cores"] = Json::arrayValue;
        base["brokers"] = Json::arrayValue;
        for (const auto& brk : _brokers) {
            Json::Value brkstate;
            brkstate["state"] = state_string(brk.state);
            brkstate["name"] = brk.name;
            brkstate["id"] = brk.global_id.baseValue();
            if (brk._core) {
                base["cores"].append(std::move(brkstate));
            } else {
                base["brokers"].append(std::move(brkstate));
            }
        }
        return generateJsonString(base);
    }
    if (request == "current_time") {
        if (!hasTimeDependency) {
            return "{}";
        }
        return timeCoord->printTimeStatus();
    }
    if (request == "global_status") {
        if (!isConnected()) {
            Json::Value gs;
            gs["status"] = "disconnected";
            gs["timestep"] = -1;
            return generateJsonString(gs);
        }
    }
    auto mi = mapIndex.find(request);
    if (mi != mapIndex.end()) {
        auto index = mi->second.first;
        if (isValidIndex(index, mapBuilders) && !mi->second.second) {
            auto& builder = std::get<0>(mapBuilders[index]);
            if (builder.isCompleted()) {
                auto center = generateMapObjectCounter();
                if (center == builder.getCounterCode()) {
                    return builder.generate();
                }
                builder.reset();
            }
            if (builder.isActive()) {
                return "#wait";
            }
        }

        initializeMapBuilder(request, index, mi->second.second, force_ordering);
        if (std::get<0>(mapBuilders[index]).isCompleted()) {
            if (!mi->second.second) {
                auto center = generateMapObjectCounter();
                std::get<0>(mapBuilders[index]).setCounterCode(center);
            }
            if (index == global_status) {
                return generateGlobalStatus(std::get<0>(mapBuilders[index]));
            }
            return std::get<0>(mapBuilders[index]).generate();
        }
        return "#wait";
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
        if (uuid_like) {
            base["uuid"] = getIdentifier();
        }
        base["id"] = global_broker_id_local.baseValue();
        if (!isRootc) {
            base["parent"] = higher_broker_id.baseValue();
        }
        base["dependents"] = Json::arrayValue;
        for (const auto& dep : timeCoord->getDependents()) {
            base["dependents"].append(dep.baseValue());
        }
        base["dependencies"] = Json::arrayValue;
        for (const auto& dep : timeCoord->getDependencies()) {
            base["dependencies"].append(dep.baseValue());
        }
        return generateJsonString(base);
    }
    return "#invalid";
}

std::string CoreBroker::generateGlobalStatus(JsonMapBuilder& builder)
{
    auto cstate = generateQueryAnswer("current_state", false);
    auto jv = loadJsonStr(cstate);
    std::string state;
    if (jv["federates"][0].isObject()) {
        state = jv["state"].asString();
    } else {
        state = "init_requested";
    }

    if (state != "operating") {
        Json::Value v;
        v["status"] = state;
        v["timestep"] = -1;
        return generateJsonString(v);
    }
    Time mv{Time::maxVal()};
    if (!builder.getJValue()["cores"][0].isObject()) {
        state = "init_requested";
    }
    for (auto& cr : builder.getJValue()["cores"]) {
        for (auto fed : cr["federates"]) {
            auto dv = fed["granted_time"].asDouble();
            if (dv < mv) {
                mv = dv;
            }
        }
    }
    std::string tste = (mv >= timeZero) ? std::string("operating") : std::string("init_requested");

    Json::Value v;

    if (tste != "operating") {
        v["status"] = tste;
        v["timestep"] = -1;
    } else {
        v["status"] = jv;
        v["timestep"] = builder.getJValue();
    }

    return generateJsonString(v);
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
        const auto* info = handles.findHandle(
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

void CoreBroker::initializeMapBuilder(const std::string& request,
                                      std::uint16_t index,
                                      bool reset,
                                      bool force_ordering)
{
    if (!isValidIndex(index, mapBuilders)) {
        mapBuilders.resize(index + 1);
    }
    std::get<2>(mapBuilders[index]) = reset;
    auto& builder = std::get<0>(mapBuilders[index]);
    builder.reset();
    Json::Value& base = builder.getJValue();
    base["name"] = getIdentifier();
    if (uuid_like) {
        base["uuid"] = getIdentifier();
    }
    base["id"] = global_broker_id_local.baseValue();
    if (!isRootc) {
        base["parent"] = higher_broker_id.baseValue();
    }
    base["brokers"] = Json::arrayValue;
    ActionMessage queryReq(force_ordering ? CMD_BROKER_QUERY_ORDERED : CMD_BROKER_QUERY);
    if (index == global_flush) {
        queryReq.setAction(CMD_BROKER_QUERY_ORDERED);
    }
    queryReq.payload = request;
    queryReq.source_id = global_broker_id_local;
    queryReq.counter = index;  // indicating which processing to use
    bool hasCores = false;
    bool hasBrokers = false;
    for (const auto& broker : _brokers) {
        if (broker.parent == global_broker_id_local) {
            switch (broker.state) {
                case connection_state::connected:
                case connection_state::init_requested:
                case connection_state::operating: {
                    int brkindex;
                    if (broker._core) {
                        if (!hasCores) {
                            hasCores = true;
                            base["cores"] = Json::arrayValue;
                        }
                        brkindex =
                            builder.generatePlaceHolder("cores", broker.global_id.baseValue());
                    } else {
                        if (!hasBrokers) {
                            hasBrokers = true;
                            base["brokers"] = Json::arrayValue;
                        }
                        brkindex =
                            builder.generatePlaceHolder("brokers", broker.global_id.baseValue());
                    }
                    queryReq.messageID = brkindex;
                    queryReq.dest_id = broker.global_id;
                    transmit(broker.route, queryReq);
                } break;
                case connection_state::error:
                case connection_state::disconnected:
                case connection_state::request_disconnect:
                    if (index == global_state) {
                        Json::Value brkstate;
                        brkstate["state"] = state_string(broker.state);
                        brkstate["name"] = broker.name;
                        brkstate["id"] = broker.global_id.baseValue();
                        if (broker._core) {
                            if (!hasCores) {
                                base["cores"] = Json::arrayValue;
                                hasCores = true;
                            }
                            base["cores"].append(std::move(brkstate));
                        } else {
                            if (!hasBrokers) {
                                base["brokers"] = Json::arrayValue;
                                hasBrokers = true;
                            }
                            base["brokers"].append(std::move(brkstate));
                        }
                    }
                    break;
            }
        }
    }
    switch (index) {
        case federate_map:
        case current_time_map:
        case global_status:
        case data_flow_graph:
        case global_flush:
        default:
            break;
        case dependency_graph: {
            base["dependents"] = Json::arrayValue;
            for (const auto& dep : timeCoord->getDependents()) {
                base["dependents"].append(dep.baseValue());
            }
            base["dependencies"] = Json::arrayValue;
            for (const auto& dep : timeCoord->getDependencies()) {
                base["dependencies"].append(dep.baseValue());
            }
        } break;
        case version_all:
            base["version"] = versionString;
            break;
        case global_state:
            base["state"] = brokerStateName(getBrokerState());
            base["status"] = isConnected();
            break;
        case global_time_debugging:
            base["state"] = brokerStateName(getBrokerState());
            if (timeCoord && !timeCoord->empty()) {
                base["time"] = Json::Value();
                timeCoord->generateDebuggingTimeInfo(base["time"]);
            }
            break;
    }
}

void CoreBroker::processLocalQuery(const ActionMessage& m)
{
    bool force_ordered =
        (m.action() == CMD_QUERY_ORDERED || m.action() == CMD_BROKER_QUERY_ORDERED);
    ActionMessage queryRep(force_ordered ? CMD_QUERY_REPLY_ORDERED : CMD_QUERY_REPLY);
    queryRep.source_id = global_broker_id_local;
    queryRep.dest_id = m.source_id;
    queryRep.messageID = m.messageID;
    queryRep.payload = generateQueryAnswer(m.payload, force_ordered);
    queryRep.counter = m.counter;
    if (queryRep.payload == "#wait") {
        if (queryRep.dest_id == global_broker_id_local) {
            if (queryTimeouts.empty()) {
                setTickForwarding(TickForwardingReasons::query_timeout, true);
            }
            queryTimeouts.emplace_back(queryRep.messageID, std::chrono::steady_clock::now());
        }
        std::get<1>(mapBuilders[mapIndex.at(m.payload).first]).push_back(queryRep);
    } else if (queryRep.dest_id == global_broker_id_local) {
        activeQueries.setDelayedValue(m.messageID, queryRep.payload);
    } else {
        routeMessage(std::move(queryRep), m.source_id);
    }
}

/** check for fed queries that can be answered by the broker*/
static std::string checkFedQuery(const BasicFedInfo& fed, const std::string& query)
{
    std::string response;
    if (query == "exists") {
        response = "true";
    } else if (query == "isconnected") {
        response =
            (fed.state >= connection_state::connected && fed.state <= connection_state::operating) ?
            "true" :
            "false";
    } else if (query == "state") {
        response = state_string(fed.state);
    } else if (query == "isinit") {
        if (fed.state >= connection_state::operating) {
            response = "true";
            // if it is false we need to actually go check the federate directly
        }
    }
    return response;
}
/** check for broker queries that can be answered by the broker*/
static std::string checkBrokerQuery(const BasicBrokerInfo& brk, const std::string& query)
{
    std::string response;
    if (query == "exists") {
        response = "true";
    } else if (query == "isconnected") {
        response =
            (brk.state >= connection_state::connected && brk.state <= connection_state::operating) ?
            "true" :
            "false";
    } else if (query == "state") {
        response = state_string(brk.state);
    } else if (query == "isinit") {
        if (brk.state >= connection_state::operating) {
            response = "true";
            // if it is false we need to actually go check the federate directly
        }
    }
    return response;
}

void CoreBroker::processQueryCommand(ActionMessage& cmd)
{
    switch (cmd.action()) {
        case CMD_BROKER_QUERY:
        case CMD_BROKER_QUERY_ORDERED:
            if (!connectionEstablished) {
                earlyMessages.push_back(std::move(cmd));
                break;
            }
            if (cmd.dest_id == global_broker_id_local ||
                (isRootc && cmd.dest_id == parent_broker_id)) {
                processLocalQuery(cmd);
            } else {
                routeMessage(cmd);
            }
            break;
        case CMD_QUERY:
        case CMD_QUERY_ORDERED:
            processQuery(cmd);
            break;
        case CMD_QUERY_REPLY:
        case CMD_QUERY_REPLY_ORDERED:
            if (cmd.dest_id == global_broker_id_local) {
                processQueryResponse(cmd);
            } else {
                transmit(getRoute(cmd.dest_id), cmd);
            }
            break;
        case CMD_SET_GLOBAL:
            if (isRootc) {
                global_values[cmd.name] = cmd.getString(0);
            } else {
                if ((global_broker_id_local.isValid()) &&
                    (global_broker_id_local != parent_broker_id)) {
                    transmit(parent_route_id, cmd);
                } else {
                    // delay the response if we are not fully registered yet
                    delayTransmitQueue.push(cmd);
                }
            }
            break;
        default:
            break;
    }
}

void CoreBroker::processQuery(ActionMessage& m)
{
    bool force_ordered =
        (m.action() == CMD_QUERY_ORDERED || m.action() == CMD_BROKER_QUERY_ORDERED);
    const auto& target = m.getString(targetStringLoc);
    if ((target == getIdentifier() || target == "broker") ||
        (isRootc && (target == "root" || target == "federation"))) {
        processLocalQuery(m);
    } else if (isRootc && target == "gid_to_name") {
        ActionMessage queryResp(force_ordered ? CMD_QUERY_REPLY_ORDERED : CMD_QUERY_REPLY);
        queryResp.dest_id = m.source_id;
        queryResp.source_id = global_broker_id_local;
        queryResp.messageID = m.messageID;
        queryResp.payload = getNameList(m.payload);
        if (queryResp.dest_id == global_broker_id_local) {
            activeQueries.setDelayedValue(m.messageID, queryResp.payload);
        } else {
            transmit(getRoute(queryResp.dest_id), queryResp);
        }
    } else if ((isRootc) && (target == "global")) {
        ActionMessage queryResp(force_ordered ? CMD_QUERY_REPLY_ORDERED : CMD_QUERY_REPLY);
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
            activeQueries.setDelayedValue(m.messageID, queryResp.payload);
        } else {
            transmit(getRoute(queryResp.dest_id), queryResp);
        }
    } else {
        route_id route = parent_route_id;
        auto fed = _federates.find(target);
        std::string response;
        if (fed != _federates.end()) {
            route = fed->route;
            m.dest_id = fed->parent;
            response = checkFedQuery(*fed, m.payload);
            if (response.empty() && fed->state >= connection_state::error) {
                route = parent_route_id;
                switch (fed->state) {
                    case connection_state::error:
                        response = "#errored";
                        break;
                    case connection_state::disconnected:
                    case connection_state::request_disconnect:
                        response = "#disconnected";
                        break;
                    default:
                        break;
                }
            }
        } else {
            auto broker = _brokers.find(target);
            if (broker != _brokers.end()) {
                route = broker->route;
                m.dest_id = broker->global_id;
                response = checkBrokerQuery(*broker, m.payload);
                if (response.empty() && broker->state >= connection_state::error) {
                    route = parent_route_id;
                    switch (broker->state) {
                        case connection_state::error:
                            response = "#errored";
                            break;
                        case connection_state::disconnected:
                        case connection_state::request_disconnect:
                            response = "#disconnected";
                            break;
                        default:
                            break;
                    }
                }
            } else if (isRootc && m.payload == "exists") {
                response = "false";
            }
        }
        if (((route == parent_route_id) && (isRootc)) || !response.empty()) {
            if (response.empty()) {
                response = "#invalid";
            }
            ActionMessage queryResp(force_ordered ? CMD_QUERY_REPLY_ORDERED : CMD_QUERY_REPLY);
            queryResp.dest_id = m.source_id;
            queryResp.source_id = global_broker_id_local;
            queryResp.messageID = m.messageID;

            queryResp.payload = response;
            if (queryResp.dest_id == global_broker_id_local) {
                activeQueries.setDelayedValue(m.messageID, queryResp.payload);
            } else {
                transmit(getRoute(queryResp.dest_id), queryResp);
            }
        } else {
            if (m.source_id == global_broker_id_local) {
                if (queryTimeouts.empty()) {
                    setTickForwarding(TickForwardingReasons::query_timeout, true);
                }
                queryTimeouts.emplace_back(m.messageID, std::chrono::steady_clock::now());
            }
            transmit(route, m);
        }
    }
}

void CoreBroker::checkQueryTimeouts()
{
    if (!queryTimeouts.empty()) {
        auto ctime = std::chrono::steady_clock::now();
        for (auto& qt : queryTimeouts) {
            if (activeQueries.isRecognized(qt.first) && !activeQueries.isCompleted(qt.first)) {
                if (Time(ctime - qt.second) > queryTimeout) {
                    activeQueries.setDelayedValue(qt.first, "#timeout");
                    qt.first = 0;
                }
            }
        }
        while (!queryTimeouts.empty() && queryTimeouts.front().first == 0) {
            queryTimeouts.pop_front();
        }
        if (queryTimeouts.empty()) {
            setTickForwarding(TickForwardingReasons::query_timeout, false);
        }
    }
}

void CoreBroker::processQueryResponse(const ActionMessage& m)
{
    if (m.counter == general_query) {
        activeQueries.setDelayedValue(m.messageID, m.payload);
        return;
    }
    if (isValidIndex(m.counter, mapBuilders)) {
        auto& builder = std::get<0>(mapBuilders[m.counter]);
        auto& requestors = std::get<1>(mapBuilders[m.counter]);
        if (builder.addComponent(m.payload, m.messageID)) {
            std::string str;
            switch (m.counter) {
                case global_status:
                    str = generateGlobalStatus(builder);
                    break;
                case global_flush:
                    str = "{\"status\":true}";
                    break;
                default:
                    str = builder.generate();
                    break;
            }

            for (int ii = 0; ii < static_cast<int>(requestors.size()) - 1; ++ii) {
                if (requestors[ii].dest_id == global_broker_id_local) {
                    activeQueries.setDelayedValue(requestors[ii].messageID, str);
                } else {
                    requestors[ii].payload = str;
                    routeMessage(std::move(requestors[ii]));
                }
            }
            if (requestors.back().dest_id == global_broker_id_local) {
                // TODO(PT) add rvalue reference method
                activeQueries.setDelayedValue(requestors.back().messageID, str);
            } else {
                requestors.back().payload = std::move(str);
                routeMessage(std::move(requestors.back()));
            }

            requestors.clear();
            if (std::get<2>(mapBuilders[m.counter])) {
                builder.reset();
            } else {
                builder.setCounterCode(generateMapObjectCounter());
            }
        }
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

        if (timeCoord->getDependents().size() == 1) {  // if there is just one dependency remove it
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
        // if there is more than 2 dependents(higher broker + 2 or more other objects
        // then we need to be a timeCoordinator
        if (timeCoord->getDependents().size() > 2) {
            return;
        }

        global_federate_id fedid;
        int localcnt = 0;
        for (const auto& dep : timeCoord->getDependents()) {
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
        setActionFlag(adddep, child_flag);
        routeMessage(adddep, higher_broker_id);
        adddep.source_id = higher_broker_id;
        clearActionFlag(adddep, child_flag);
        setActionFlag(adddep, parent_flag);
        routeMessage(adddep, fedid);
    }
}

connection_state CoreBroker::getAllConnectionState() const
{
    connection_state res = connection_state::disconnected;
    int cnt{0};
    for (const auto& brk : _brokers) {
        if (brk._nonLocal) {
            continue;
        }
        ++cnt;
        if (brk.state < res) {
            res = brk.state;
        }
    }
    return (cnt > 0) ? res : connection_state::connected;
}

int CoreBroker::getCountableFederates() const
{
    int cnt{0};
    for (const auto& fed : _federates) {
        if (!fed.nonCounting) {
            ++cnt;
        }
    }
    return cnt;
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
    bool initReady = (getAllConnectionState() >= connection_state::init_requested);
    if (initReady) {
        // now do a more formal count of federates as there may be non-counting ones
        return (getCountableFederates() >= minFederateCount);
    }
    return false;
    // return std::all_of(_brokers.begin(), _brokers.end(), [](const auto& brk) {
    //   return ((brk._nonLocal) || (brk.state==connection_state::init_requested));
    //});
}

}  // namespace helics
