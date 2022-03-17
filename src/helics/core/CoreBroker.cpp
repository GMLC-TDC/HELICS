/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "CoreBroker.hpp"

#include "../common/JsonGeneration.hpp"
#include "../common/JsonProcessingFunctions.hpp"
#include "../common/fmt_format.h"
#include "../common/logging.hpp"
#include "BrokerFactory.hpp"
#include "ForwardingTimeCoordinator.hpp"
#include "LogManager.hpp"
#include "TimeoutMonitor.h"
#include "fileConnections.hpp"
#include "gmlc/utilities/stringConversion.h"
#include "gmlc/utilities/string_viewConversion.h"
#include "gmlc/utilities/string_viewOps.h"
#include "helics/common/LogBuffer.hpp"
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
    if (getBrokerState() <= BrokerState::connecting)  // can't be changed after initialization
    {
        std::lock_guard<std::mutex> lock(name_mutex_);
        identifier = name;
    }
}

const std::string& CoreBroker::getAddress() const
{
    if ((getBrokerState() != BrokerState::connected) || (address.empty())) {
        address = generateLocalAddressString();
    }
    return address;
}

route_id CoreBroker::getRoute(GlobalFederateId fedid) const
{
    if ((fedid == parent_broker_id) || (fedid == higher_broker_id)) {
        return parent_route_id;
    }
    auto fnd = routing_table.find(fedid);
    return (fnd != routing_table.end()) ? fnd->second :
                                          parent_route_id;  // zero is the default route
}

BasicBrokerInfo* CoreBroker::getBrokerById(GlobalBrokerId brokerid)
{
    if (isRootc) {
        auto brkNum = brokerid.localIndex();
        return (isValidIndex(brkNum, mBrokers) ? &mBrokers[brkNum] : nullptr);
    }

    auto fnd = mBrokers.find(brokerid);
    return (fnd != mBrokers.end()) ? &(*fnd) : nullptr;
}

const BasicBrokerInfo* CoreBroker::getBrokerById(GlobalBrokerId brokerid) const
{
    if (isRootc) {
        auto brkNum = brokerid.localIndex();
        return (isValidIndex(brkNum, mBrokers)) ? &mBrokers[brkNum] : nullptr;
    }

    auto fnd = mBrokers.find(brokerid);
    return (fnd != mBrokers.end()) ? &(*fnd) : nullptr;
}

void CoreBroker::setLoggingCallback(
    const std::function<void(int, std::string_view, std::string_view)>& logFunction)
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
    if (fileops::hasTomlExtension(file)) {
        fileops::makeConnectionsToml(this, file);
    } else {
        fileops::makeConnectionsJson(this, file);
    }
}

void CoreBroker::linkEndpoints(const std::string& source, const std::string& target)
{
    ActionMessage M(CMD_ENDPOINT_LINK);
    M.name(source);
    M.setStringData(target);
    addActionMessage(std::move(M));
}

void CoreBroker::dataLink(const std::string& publication, const std::string& input)
{
    ActionMessage M(CMD_DATA_LINK);
    M.name(publication);
    M.setStringData(input);
    addActionMessage(std::move(M));
}

void CoreBroker::addSourceFilterToEndpoint(const std::string& filter, const std::string& endpoint)
{
    ActionMessage M(CMD_FILTER_LINK);
    M.name(filter);
    M.setStringData(endpoint);
    addActionMessage(std::move(M));
}

void CoreBroker::addDestinationFilterToEndpoint(const std::string& filter,
                                                const std::string& endpoint)
{
    ActionMessage M(CMD_FILTER_LINK);
    M.name(filter);
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
    return (cstate != BrokerState::created && cstate < BrokerState::operating && !haltOperations &&
            (maxFederateCount == (std::numeric_limits<int32_t>::max)() ||
             getCountableFederates() < maxFederateCount));
}

void CoreBroker::brokerRegistration(ActionMessage&& command)
{
    if (!connectionEstablished) {
        earlyMessages.push_back(std::move(command));
        return;
    }
    bool jsonReply = checkActionFlag(command, use_json_serialization_flag);
    if (command.counter > 0) {  // this indicates it is a resend
        auto brk = mBrokers.find(std::string(command.name()));
        if (brk != mBrokers.end()) {
            // we would get this if the ack didn't go through for some reason
            brk->route = generateRouteId(jsonReply ? json_route_code : 0, routeCount++);
            addRoute(brk->route, command.getExtraData(), command.getString(targetStringLoc));
            routing_table[brk->global_id] = brk->route;

            // sending the response message
            ActionMessage brokerReply(CMD_BROKER_ACK);
            brokerReply.source_id = global_broker_id_local;  // source is global root
            brokerReply.dest_id = brk->global_id;  // the new id
            brokerReply.name(command.name());  // the identifier of the broker
            if (no_ping) {
                setActionFlag(brokerReply, slow_responding_flag);
            }
            transmit(brk->route, brokerReply);
            return;
        }
    }
    // check the max broker count
    if (static_cast<decltype(maxBrokerCount)>(mBrokers.size()) >= maxBrokerCount) {
        route_id newroute;
        bool route_created = false;
        if ((!command.source_id.isValid()) || (command.source_id == parent_broker_id)) {
            newroute = generateRouteId(jsonReply ? json_route_code : 0, routeCount++);
            addRoute(newroute, command.getExtraData(), command.getString(targetStringLoc));
            route_created = true;
        } else {
            newroute = getRoute(command.source_id);
        }
        ActionMessage badInit(CMD_BROKER_ACK);
        setActionFlag(badInit, error_flag);
        badInit.source_id = global_broker_id_local;
        badInit.name(command.name());
        badInit.messageID = max_broker_count_exceeded;
        transmit(newroute, badInit);

        if (route_created) {
            removeRoute(newroute);
        }
        return;
    }

    auto currentBrokerState = getBrokerState();
    if (currentBrokerState < BrokerState::operating) {
        if (allInitReady()) {
            // send an init not ready as we were ready now we are not
            ActionMessage noInit(CMD_INIT_NOT_READY);
            noInit.source_id = global_broker_id_local;
            transmit(parent_route_id, noInit);
        }
    } else if (currentBrokerState == BrokerState::operating) {
        // we are initialized already
        if (!checkActionFlag(command, observer_flag)) {
            route_id newroute;
            bool route_created = false;
            if ((!command.source_id.isValid()) || (command.source_id == parent_broker_id)) {
                newroute = generateRouteId(jsonReply ? json_route_code : 0, routeCount++);
                addRoute(newroute, command.getExtraData(), command.getString(targetStringLoc));
                route_created = true;
            } else {
                newroute = getRoute(command.source_id);
            }
            ActionMessage badInit(CMD_BROKER_ACK);
            setActionFlag(badInit, error_flag);
            badInit.source_id = global_broker_id_local;
            badInit.name(command.name());
            badInit.messageID = already_init_error_code;
            transmit(newroute, badInit);

            if (route_created) {
                removeRoute(newroute);
            }
            return;
        }
        // can't add a non observer federate in operating mode
    } else {
        route_id newroute;
        bool route_created = false;
        if ((!command.source_id.isValid()) || (command.source_id == parent_broker_id)) {
            newroute = generateRouteId(jsonReply ? json_route_code : 0, routeCount++);
            addRoute(newroute, command.getExtraData(), command.getString(targetStringLoc));
            route_created = true;
        } else {
            newroute = getRoute(command.source_id);
        }
        ActionMessage badInit(CMD_BROKER_ACK);
        setActionFlag(badInit, error_flag);
        badInit.source_id = global_broker_id_local;
        badInit.name(command.name());
        badInit.setString(0, "broker is terminating");
        badInit.messageID = broker_terminating;
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
            newroute = generateRouteId(jsonReply ? json_route_code : 0, routeCount++);
            addRoute(newroute, command.getExtraData(), command.getString(targetStringLoc));
            route_created = true;
        } else {
            newroute = getRoute(command.source_id);
        }
        ActionMessage badKey(CMD_BROKER_ACK);
        setActionFlag(badKey, error_flag);
        badKey.source_id = global_broker_id_local;
        badKey.messageID = mismatch_broker_key_error_code;
        badKey.name(command.name());
        badKey.setString(0, "broker key does not match");
        transmit(newroute, badKey);
        if (route_created) {
            removeRoute(newroute);
        }
        return;
    }
    auto inserted = mBrokers.insert(std::string(command.name()), no_search, command.name());
    if (!inserted) {
        route_id newroute;
        bool route_created = false;
        if ((!command.source_id.isValid()) || (command.source_id == parent_broker_id)) {
            newroute = generateRouteId(jsonReply ? json_route_code : 0, routeCount++);
            addRoute(newroute, command.getExtraData(), command.getString(targetStringLoc));
            route_created = true;
        } else {
            newroute = getRoute(command.source_id);
        }
        ActionMessage badName(CMD_BROKER_ACK);
        setActionFlag(badName, error_flag);
        badName.source_id = global_broker_id_local;
        badName.messageID = duplicate_broker_name_error_code;
        badName.name(command.name());
        transmit(newroute, badName);
        if (route_created) {
            removeRoute(newroute);
        }
        return;
    }
    if ((!command.source_id.isValid()) || (command.source_id == parent_broker_id)) {
        // TODO(PT): this will need to be updated when we enable mesh routing
        mBrokers.back().route = generateRouteId(jsonReply ? json_route_code : 0, routeCount++);
        addRoute(mBrokers.back().route, command.getExtraData(), command.getString(targetStringLoc));
        mBrokers.back().parent = global_broker_id_local;
        mBrokers.back()._nonLocal = false;
        mBrokers.back()._route_key = true;
        mBrokers.back()._observer = checkActionFlag(command, observer_flag);
    } else {
        mBrokers.back().route = getRoute(command.source_id);
        if (mBrokers.back().route == parent_route_id) {
            std::cout << " invalid route to parent broker or reg broker" << std::endl;
        }
        mBrokers.back().parent = command.source_id;
        mBrokers.back()._nonLocal = true;
        mBrokers.back()._observer = checkActionFlag(command, observer_flag);
    }
    mBrokers.back()._core = checkActionFlag(command, core_flag);
    if (!isRootc) {
        if ((global_broker_id_local.isValid()) && (global_broker_id_local != parent_broker_id)) {
            command.source_id = global_broker_id_local;
            transmit(parent_route_id, command);
        } else {
            // delay the response if we are not fully registered yet
            delayTransmitQueue.push(command);
        }
    } else {
        mBrokers.back().global_id = GlobalBrokerId(
            static_cast<GlobalBrokerId::BaseType>(mBrokers.size()) - 1 + gGlobalBrokerIdShift);
        mBrokers.addSearchTermForIndex(mBrokers.back().global_id, mBrokers.size() - 1);
        auto global_brkid = mBrokers.back().global_id;
        auto route = mBrokers.back().route;
        if (checkActionFlag(command, slow_responding_flag)) {
            mBrokers.back()._disable_ping = true;
        }
        routing_table.emplace(global_brkid, route);
        // don't bother with the broker_table for root broker

        // sending the response message
        ActionMessage brokerReply(CMD_BROKER_ACK);
        brokerReply.source_id = global_broker_id_local;  // source is global root
        brokerReply.dest_id = global_brkid;  // the new id
        brokerReply.name(command.name());  // the identifier of the broker
        if (no_ping) {
            setActionFlag(brokerReply, slow_responding_flag);
        }
        transmit(route, brokerReply);
        LOG_CONNECTIONS(global_broker_id_local,
                        getIdentifier(),
                        fmt::format("registering broker {}({}) on route {}",
                                    command.name(),
                                    global_brkid.baseValue(),
                                    route.baseValue()));
    }
}

// Handle the registration of new federates;
void CoreBroker::fedRegistration(ActionMessage&& command)
{
    if (!connectionEstablished) {
        earlyMessages.push_back(std::move(command));
        return;
    }
    if (!checkActionFlag(command, non_counting_flag) &&
        getCountableFederates() >= maxFederateCount) {
        ActionMessage badInit(CMD_FED_ACK);
        setActionFlag(badInit, error_flag);
        badInit.source_id = global_broker_id_local;
        badInit.messageID = max_federate_count_exceeded;
        badInit.name(command.name());
        transmit(getRoute(command.source_id), badInit);
        return;
    }
    if (getBrokerState() < BrokerState::operating) {
        if (allInitReady()) {
            ActionMessage noInit(CMD_INIT_NOT_READY);
            noInit.source_id = global_broker_id_local;
            transmit(parent_route_id, noInit);
        }
    } else if (getBrokerState() == BrokerState::operating) {
        if (!checkActionFlag(command, observer_flag)) {
            // we are initialized already
            ActionMessage badInit(CMD_FED_ACK);
            setActionFlag(badInit, error_flag);
            badInit.source_id = global_broker_id_local;
            badInit.messageID = already_init_error_code;
            badInit.name(command.name());
            transmit(getRoute(command.source_id), badInit);
            return;
        }
    } else {
        // we are initialized already
        ActionMessage badInit(CMD_FED_ACK);
        setActionFlag(badInit, error_flag);
        badInit.source_id = global_broker_id_local;
        badInit.messageID = broker_terminating;
        badInit.name(command.name());
        transmit(getRoute(command.source_id), badInit);
        return;
    }
    // this checks for duplicate federate names
    if (mFederates.find(std::string(command.name())) != mFederates.end()) {
        ActionMessage badName(CMD_FED_ACK);
        setActionFlag(badName, error_flag);
        badName.source_id = global_broker_id_local;
        badName.messageID = duplicate_federate_name_error_code;
        badName.name(command.name());
        transmit(getRoute(command.source_id), badName);
        return;
    }
    mFederates.insert(std::string(command.name()), no_search, std::string(command.name()));
    mFederates.back().route = getRoute(command.source_id);
    mFederates.back().parent = command.source_id;
    if (checkActionFlag(command, non_counting_flag)) {
        mFederates.back().nonCounting = true;
    }
    if (checkActionFlag(command, child_flag)) {
        mFederates.back().global_id = GlobalFederateId(command.getExtraData());
        mFederates.addSearchTermForIndex(mFederates.back().global_id, mFederates.size() - 1);
    } else if (isRootc) {
        mFederates.back().global_id =
            GlobalFederateId(static_cast<GlobalFederateId::BaseType>(mFederates.size()) - 1 +
                             gGlobalFederateIdShift);
        mFederates.addSearchTermForIndex(mFederates.back().global_id,
                                         static_cast<size_t>(
                                             mFederates.back().global_id.baseValue()) -
                                             gGlobalFederateIdShift);
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
        auto route_id = mFederates.back().route;
        auto global_fedid = mFederates.back().global_id;

        routing_table.emplace(global_fedid, route_id);
        // don't bother with the federate_table
        // transmit the response
        ActionMessage fedReply(CMD_FED_ACK);
        fedReply.source_id = global_broker_id_local;
        fedReply.dest_id = global_fedid;
        fedReply.name(command.name());
        if (checkActionFlag(command, child_flag)) {
            setActionFlag(fedReply, child_flag);
        }
        transmit(route_id, fedReply);
        LOG_CONNECTIONS(global_broker_id_local,
                        getIdentifier(),
                        fmt::format("registering federate {}({}) on route {}",
                                    command.name(),
                                    global_fedid.baseValue(),
                                    route_id.baseValue()));
        if (enable_profiling) {
            ActionMessage fedEnableProfiling(CMD_SET_PROFILER_FLAG,
                                             global_broker_id_local,
                                             global_fedid);
            setActionFlag(fedEnableProfiling, indicator_flag);
            transmit(route_id, fedEnableProfiling);
        }
    }
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
        case CMD_REG_FED:
            fedRegistration(std::move(command));
            break;
        case CMD_REG_BROKER:
            brokerRegistration(std::move(command));
            break;
        case CMD_FED_ACK: {  // we can't be root if we got one of these
            auto fed = mFederates.find(std::string(command.name()));
            if (fed != mFederates.end()) {
                auto route = fed->route;
                if (!fed->global_id.isValid()) {
                    fed->global_id = command.dest_id;
                    mFederates.addSearchTerm(command.dest_id, fed->name);
                }
                transmit(route, command);
                routing_table.emplace(fed->global_id, route);
                if (enable_profiling) {
                    ActionMessage fedEnableProfiling(CMD_SET_PROFILER_FLAG,
                                                     global_broker_id_local,
                                                     command.dest_id);
                    setActionFlag(fedEnableProfiling, indicator_flag);
                    transmit(route, fedEnableProfiling);
                }
            } else {
                // this means we haven't seen this federate before for some reason
                mFederates.insert(std::string(command.name()),
                                  command.dest_id,
                                  std::string(command.name()));
                mFederates.back().route = getRoute(command.source_id);
                mFederates.back().global_id = command.dest_id;
                routing_table.emplace(fed->global_id, mFederates.back().route);
                // it also means we don't forward it
            }

        } break;
        case CMD_BROKER_ACK: {  // we can't be root if we got one of these
            if (command.name() == identifier) {
                if (checkActionFlag(command, error_flag)) {
                    // generate an error message
                    LOG_ERROR(global_broker_id_local,
                              identifier,
                              fmt::format("unable to register broker {}",
                                          command.payload.to_string()));
                    return;
                }

                global_broker_id_local = command.dest_id;
                global_id.store(global_broker_id_local);
                higher_broker_id = command.source_id;
                timeCoord->source_id = global_broker_id_local;
                transmitDelayedMessages();
                mBrokers.apply([localid = global_broker_id_local](auto& brk) {
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
            auto broker = mBrokers.find(std::string(command.name()));
            if (broker != mBrokers.end()) {
                if (broker->global_id == GlobalBrokerId(command.dest_id)) {
                    // drop the packet since we have seen this ack already
                    LOG_WARNING(global_broker_id_local, identifier, "repeated broker acks");
                    return;
                }
                broker->global_id = GlobalBrokerId(command.dest_id);
                auto route = broker->route;
                mBrokers.addSearchTerm(GlobalBrokerId(command.dest_id), broker->name);
                routing_table.emplace(broker->global_id, route);
                command.source_id = global_broker_id_local;  // we want the intermediate broker to
                                                             // change the source_id
                transmit(route, command);
            } else {
                mBrokers.insert(std::string(command.name()),
                                GlobalBrokerId(command.dest_id),
                                command.name());
                mBrokers.back().route = getRoute(command.source_id);
                mBrokers.back().global_id = GlobalBrokerId(command.dest_id);
                routing_table.emplace(broker->global_id, mBrokers.back().route);
            }
        } break;
        case CMD_PRIORITY_DISCONNECT: {
            auto* brk = getBrokerById(GlobalBrokerId(command.source_id));
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
        case CMD_SEND_COMMAND:
            processCommandInstruction(command);
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
            case InterfaceType::PUBLICATION:
                ++pubs;
                break;
            case InterfaceType::INPUT:
                ++ipts;
                break;
            case InterfaceType::ENDPOINT:
                ++epts;
                break;
            default:
                ++filt;
                break;
        }
    }
    Json::Value summary;
    Json::Value block;
    block["federates"] = static_cast<int>(mFederates.size());
    block["min_federates"] = minFederateCount;
    block["brokers"] =
        static_cast<int>(std::count_if(mBrokers.begin(), mBrokers.end(), [](auto& brk) {
            return !static_cast<bool>(brk._core);
        }));
    block["cores"] =
        static_cast<int>(std::count_if(mBrokers.begin(), mBrokers.end(), [](auto& brk) {
            return static_cast<bool>(brk._core);
        }));
    block["min_brokers"] = minBrokerCount;
    block["publications"] = pubs;
    block["inputs"] = ipts;
    block["filters"] = filt;
    block["endpoints"] = epts;
    summary["summary"] = block;

    return fileops::generateJsonString(summary);
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
    for (const auto& brk : mBrokers) {
        result += static_cast<int>(brk.state);
    }
    for (const auto& fed : mFederates) {
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

void CoreBroker::labelAsDisconnected(GlobalBrokerId brkid)
{
    auto disconnect_procedure = [brkid](auto& obj) {
        if (obj.parent == brkid) {
            obj.state = connection_state::disconnected;
        }
    };
    mBrokers.apply(disconnect_procedure);
    mFederates.apply(disconnect_procedure);
}

void CoreBroker::loadTimeMonitor(bool firstLoad, std::string_view newFederate)
{
    if (!newFederate.empty() && newFederate == mTimeMonitorFederate) {
        return;
    }
    if (!firstLoad && mTimeMonitorFederateId.isValid() && newFederate.empty()) {
        // do a disconnect
        ActionMessage timeMarkerRem(CMD_REMOVE_DEPENDENT);
        timeMarkerRem.dest_id = mTimeMonitorFederateId;
        timeMarkerRem.source_id = mTimeMonitorLocalFederateId;
        routeMessage(timeMarkerRem);
        mTimeMonitorFederateId = GlobalFederateId{};
        LOG_SUMMARY(global_broker_id_local, getIdentifier(), " disconnected time monitor federate");
        mTimeMonitorFederate = newFederate;
        return;
    }
    auto cState = getBrokerState();
    if (cState == BrokerState::operating || firstLoad) {
        if (cState == BrokerState::operating && !firstLoad) {
            if (mTimeMonitorFederateId.isValid()) {
                // do a disconnect
                ActionMessage timeMarkerRem(CMD_REMOVE_DEPENDENT);
                timeMarkerRem.dest_id = mTimeMonitorFederateId;
                timeMarkerRem.source_id = mTimeMonitorLocalFederateId;
                routeMessage(timeMarkerRem);
                mTimeMonitorFederateId = GlobalFederateId{};
                LOG_SUMMARY(global_broker_id_local,
                            getIdentifier(),
                            fmt::format(" changing time monitor federate from {} to {}",
                                        mTimeMonitorFederate,
                                        newFederate));
            }
        }
        if (!newFederate.empty()) {
            mTimeMonitorFederate = newFederate;
        }
        auto fed = mFederates.find(mTimeMonitorFederate);
        if (fed != mFederates.end()) {
            ActionMessage timeMarker(CMD_ADD_DEPENDENT);
            timeMarker.dest_id = fed->global_id;

            mTimeMonitorFederateId = fed->global_id;
            mTimeMonitorLastLogTime = Time::minVal();
            mTimeMonitorLocalFederateId = getSpecialFederateId(global_broker_id_local, 0);
            timeMarker.source_id = mTimeMonitorLocalFederateId;
            routeMessage(timeMarker);
        } else {
            LOG_WARNING(global_broker_id_local,
                        getIdentifier(),
                        fmt::format(" unrecognized timing federate {}", mTimeMonitorFederate));
        }
    } else if (cState < BrokerState::operating) {
        if (!newFederate.empty()) {
            mTimeMonitorFederate = newFederate;
        }
    }
}

void CoreBroker::processTimeMonitorMessage(ActionMessage& m)
{
    if (m.source_id != mTimeMonitorFederateId) {
        return;
    }
    switch (m.action()) {
        case CMD_EXEC_GRANT:
            mTimeMonitorLastLogTime = timeZero;
            mTimeMonitorCurrentTime = timeZero;

            simTime.store(static_cast<double>(mTimeMonitorCurrentTime));
            LOG_SUMMARY(m.source_id, mTimeMonitorFederate, "TIME: exec granted");
            break;
        case CMD_TIME_GRANT:
            mTimeMonitorCurrentTime = m.actionTime;
            simTime.store(static_cast<double>(mTimeMonitorCurrentTime));
            if (mTimeMonitorCurrentTime - mTimeMonitorPeriod >= mTimeMonitorLastLogTime) {
                LOG_SUMMARY(m.source_id,
                            mTimeMonitorFederate,
                            fmt::format("TIME: granted time={}",
                                        static_cast<double>(mTimeMonitorCurrentTime)));
                mTimeMonitorLastLogTime = mTimeMonitorCurrentTime;
            }
            break;
        case CMD_DISCONNECT:
            LOG_SUMMARY(m.source_id,
                        mTimeMonitorFederate,
                        fmt::format("TIME: disconnected, last time {}",
                                    static_cast<double>(mTimeMonitorCurrentTime)));
            mTimeMonitorCurrentTime = Time::maxVal();
            mTimeMonitorLastLogTime = Time::maxVal();
            simTime.store(static_cast<double>(mTimeMonitorCurrentTime));
            break;
        default:
            break;
    }
}

void CoreBroker::sendDisconnect(action_message_def::action_t disconnectType)
{
    ActionMessage bye(disconnectType);
    bye.source_id = global_broker_id_local;
    mBrokers.apply([this, &bye](auto& brk) {
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
    if (enable_profiling) {
        writeProfilingData();
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
            if (getBrokerState() == BrokerState::operating) {
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
            sendDisconnect(CMD_GLOBAL_DISCONNECT);
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
                for (const auto& brk : mBrokers) {
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
                        elink.messageID = defs::Errors::CONNECTION_FAILURE;
                        broadcast(elink);
                        setBrokerState(BrokerState::errored);
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
        case CMD_INIT:
        case CMD_INIT_NOT_READY:
        case CMD_INIT_GRANT:
            processInitCommand(command);
            break;
        case CMD_SEARCH_DEPENDENCY: {
            auto fed = mFederates.find(std::string(command.name()));
            if (fed != mFederates.end()) {
                if (fed->global_id.isValid()) {
                    ActionMessage dep(CMD_ADD_DEPENDENCY, fed->global_id, command.source_id);
                    routeMessage(dep);
                    dep = ActionMessage(CMD_ADD_DEPENDENT, command.source_id, fed->global_id);
                    routeMessage(dep);
                    break;
                }
            }
            if (isRootc) {
                delayedDependencies.emplace_back(command.name(), command.source_id);
            } else {
                routeMessage(command);
            }
            break;
        }
        case CMD_DATA_LINK:
        case CMD_ENDPOINT_LINK:
        case CMD_FILTER_LINK:
            linkInterfaces(command);
            break;
        case CMD_DISCONNECT_NAME:
            if (command.dest_id == parent_broker_id) {
                auto brk = mBrokers.find(std::string(command.name()));
                if (brk != mBrokers.end()) {
                    command.source_id = brk->global_id;
                }
            }
            [[fallthrough]];
        case CMD_DISCONNECT:
        case CMD_DISCONNECT_CORE:
        case CMD_DISCONNECT_BROKER:
            processDisconnect(command);
            break;
        case CMD_DISCONNECT_BROKER_ACK:
            if ((command.dest_id == global_broker_id_local) &&
                (command.source_id == higher_broker_id)) {
                mBrokers.apply([this](auto& brk) {
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
        case CMD_GLOBAL_DISCONNECT:
            sendDisconnect(CMD_GLOBAL_DISCONNECT);
            addActionMessage(CMD_STOP);
            break;
        case CMD_TIMEOUT_DISCONNECT:
            if (command.dest_id == parent_broker_id || command.dest_id == global_broker_id_local) {
                if (isConnected()) {
                    if (timeCoord->hasActiveTimeDependencies()) {
                        Json::Value base;
                        base["id"] = global_broker_id_local.baseValue();
                        timeCoord->generateDebuggingTimeInfo(base);
                        auto debugString = fileops::generateJsonString(base);
                        debugString.insert(0, "TIME DEBUGGING::");
                        LOG_WARNING(global_broker_id_local, getIdentifier(), debugString);
                    }
                    if (command.source_id == global_broker_id_local) {
                        LOG_ERROR(global_broker_id_local, getIdentifier(), "timeout disconnect");
                    } else {
                        LOG_ERROR(global_broker_id_local,
                                  getIdentifier(),
                                  "received timeout disconnect");
                    }

                    if (getBrokerState() <
                        BrokerState::terminating) {  // only send a disconnect message
                                                     // if we haven't done so already
                        sendDisconnect(CMD_TIMEOUT_DISCONNECT);
                    }
                } else if (getBrokerState() ==
                           BrokerState::errored) {  // we are disconnecting in an error state
                    sendDisconnect(CMD_TIMEOUT_DISCONNECT);
                }
                addActionMessage(CMD_USER_DISCONNECT);
            } else {
                routeMessage(command);
            }
            break;
        case CMD_DISCONNECT_FED: {
            auto fed = mFederates.find(command.source_id);
            if (fed != mFederates.end()) {
                fed->state = connection_state::disconnected;
            }
            if (!isRootc) {
                transmit(parent_route_id, command);
            } else if (getBrokerState() < BrokerState::operating) {
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
                    if (res == MessageProcessingResult::NEXT_STEP) {
                        enteredExecutionMode = true;
                        LOG_TIMING(global_broker_id_local, getIdentifier(), "entering Exec Mode");
                    }
                }
            } else if (command.source_id == global_broker_id_local) {
                for (auto& dep : timeCoord->getDependents()) {
                    routeMessage(command, dep);
                }
            } else if (command.dest_id == mTimeMonitorLocalFederateId) {
                processTimeMonitorMessage(command);
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
        case CMD_REQUEST_CURRENT_TIME:
            if ((command.source_id == global_broker_id_local) &&
                (command.dest_id == parent_broker_id)) {
                LOG_TIMING(global_broker_id_local,
                           getIdentifier(),
                           fmt::format("time request update {}", prettyPrintString(command)));
                for (auto& dep : timeCoord->getDependents()) {
                    routeMessage(command, dep);
                }
            } else if (command.dest_id == global_broker_id_local) {
                if (timeCoord->processTimeMessage(command)) {
                    if (enteredExecutionMode) {
                        timeCoord->updateTimeFactors();
                    } else {
                        auto res = timeCoord->checkExecEntry();
                        if (res == MessageProcessingResult::NEXT_STEP) {
                            enteredExecutionMode = true;
                            LOG_TIMING(global_broker_id_local,
                                       getIdentifier(),
                                       "entering Exec Mode");
                        }
                    }
                }
            } else if (command.dest_id == mTimeMonitorLocalFederateId) {
                processTimeMonitorMessage(command);
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
        case CMD_GRANT_TIMEOUT_CHECK:
            if (command.dest_id == global_broker_id_local ||
                (isRootc && command.dest_id == parent_broker_id)) {
                auto v = timeCoord->grantTimeoutCheck(command);
                if (!v.isNull()) {
                    auto debugString = fileops::generateJsonString(v);
                    debugString.insert(0, "TIME DEBUGGING::");
                    LOG_WARNING(global_broker_id_local, "broker", debugString);
                }
            } else {
                routeMessage(std::move(command));
            }
            break;
        case CMD_PUB:
            transmit(getRoute(command.dest_id), command);
            break;

        case CMD_LOG:
        case CMD_REMOTE_LOG:
            if (command.dest_id == global_broker_id_local || command.dest_id == parent_broker_id) {
                sendToLogger(command.source_id,
                             command.messageID,
                             command.getString(0),
                             command.name(),
                             (command.action() == CMD_REMOTE_LOG));

            } else {
                routeMessage(command);
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
        case CMD_SEND_COMMAND_ORDERED:
            processCommandInstruction(command);
            break;
        case CMD_PROFILER_DATA:
            if (enable_profiling) {
                saveProfilingData(command.payload.to_string());
            } else {
                if (isRootc) {
                    saveProfilingData(command.payload.to_string());
                } else {
                    routeMessage(std::move(command), parent_broker_id);
                }
            }
            break;
        case CMD_SET_PROFILER_FLAG:
            routeMessage(command);
            break;
        default:
            if (command.dest_id != global_broker_id_local) {
                routeMessage(command);
            }
    }
}

void CoreBroker::processInitCommand(ActionMessage& cmd)
{
    switch (cmd.action()) {
        case CMD_INIT: {
            auto* brk = getBrokerById(static_cast<GlobalBrokerId>(cmd.source_id));

            if (brk == nullptr) {
                break;
            }
            brk->state = connection_state::init_requested;
            if (brk->_observer && getBrokerState() >= BrokerState::operating) {
                if (isRootc) {
                    ActionMessage grant(CMD_INIT_GRANT, global_broker_id_local, cmd.source_id);
                    setActionFlag(grant, observer_flag);
                    transmit(brk->route, grant);
                } else {
                    transmit(parent_route_id, cmd);
                }
            } else {
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
                        cmd.source_id = global_broker_id_local;
                        transmit(parent_route_id, cmd);
                    }
                }
            }

        } break;
        case CMD_INIT_NOT_READY: {
            if (allInitReady()) {
                transmit(parent_route_id, cmd);
            }
            auto* brk = getBrokerById(GlobalBrokerId(cmd.source_id));
            if (brk != nullptr) {
                brk->state = connection_state::connected;
            }
        } break;
        case CMD_INIT_GRANT:
            if (!checkActionFlag(cmd, observer_flag)) {
                if (brokerKey == universalKey) {
                    LOG_SUMMARY(global_broker_id_local,
                                getIdentifier(),
                                "Broker started with universal key");
                }
                setBrokerState(BrokerState::operating);
                for (const auto& brk : mBrokers) {
                    transmit(brk.route, cmd);
                }
                {
                    timeCoord->enteringExecMode();
                    auto res = timeCoord->checkExecEntry();
                    if (res == MessageProcessingResult::NEXT_STEP) {
                        enteredExecutionMode = true;
                    }
                }
            } else {
                routeMessage(std::move(cmd));
            }
            break;
        default:
            break;
    }
}

void CoreBroker::processBrokerConfigureCommands(ActionMessage& cmd)
{
    switch (cmd.messageID) {
        case defs::Flags::ENABLE_INIT_ENTRY:
            /*if (delayInitCounter <= 1)
        {
            delayInitCounter = 0;
            if (allInitReady())
            {
                BrokerState exp = connected;
                if (brokerState.compare_exchange_strong(exp, BrokerState::initializing))
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
        case defs::Properties::LOG_LEVEL:
            setLogLevel(cmd.getExtraData());
            break;
        case UPDATE_LOGGING_CALLBACK:
            if (checkActionFlag(cmd, empty_flag)) {
                setLoggerFunction(nullptr);
            } else {
                auto op = dataAirlocks[cmd.counter].try_unload();
                if (op) {
                    try {
                        auto M = std::any_cast<
                            std::function<void(int, std::string_view, std::string_view)>>(
                            std::move(*op));
                        M(0, identifier, "logging callback activated");
                        setLoggerFunction(std::move(M));
                    }
                    catch (const std::bad_any_cast&) {
                        // This shouldn't really happen unless someone is being malicious so just
                        // ignore it for now.
                    }
                }
            }
            break;
        case UPDATE_LOGGING_FILE:
            setLoggingFile(cmd.payload.to_string());
            break;
        case REQUEST_TICK_FORWARDING:
            if (checkActionFlag(cmd, indicator_flag)) {
                setTickForwarding(TickForwardingReasons::PING_RESPONSE, true);
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
            auto* pub = handles.getPublication(command.name());
            if (pub != nullptr) {
                auto fed = mFederates.find(pub->getFederateId());
                if (fed->state < connection_state::error) {
                    command.setAction(CMD_ADD_SUBSCRIBER);
                    command.setDestination(pub->handle);
                    command.payload.clear();
                    routeMessage(command);
                    command.setAction(CMD_ADD_PUBLISHER);
                    command.swapSourceDest();
                    command.name(pub->key);
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
            auto* inp = handles.getInput(command.name());
            if (inp != nullptr) {
                auto fed = mFederates.find(inp->getFederateId());
                if (fed->state < connection_state::error) {
                    command.setAction(CMD_ADD_PUBLISHER);
                    command.setDestination(inp->handle);
                    auto* pub = handles.findHandle(command.getSource());
                    if (pub != nullptr) {
                        command.setStringData(pub->type, pub->units);
                    }
                    command.payload.clear();
                    routeMessage(command);
                    command.setAction(CMD_ADD_SUBSCRIBER);
                    command.swapSourceDest();
                    command.clearStringData();
                    command.name(inp->key);
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
            auto* filt = handles.getFilter(command.name());
            if (filt != nullptr) {
                command.setAction(CMD_ADD_ENDPOINT);
                command.setDestination(filt->handle);
                command.payload.clear();
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
            auto* ept = handles.getEndpoint(command.name());
            if (ept != nullptr) {
                auto fed = mFederates.find(ept->getFederateId());
                if (fed->state < connection_state::error) {
                    if (command.counter == static_cast<uint16_t>(InterfaceType::ENDPOINT)) {
                        command.setAction(CMD_ADD_ENDPOINT);
                        toggleActionFlag(command, destination_target);
                    } else {
                        command.setAction(CMD_ADD_FILTER);
                        auto* filt = handles.findHandle(command.getSource());
                        if (filt != nullptr) {
                            if ((!filt->type_in.empty()) || (!filt->type_out.empty())) {
                                command.setStringData(filt->type_in, filt->type_out);
                            }
                            if (checkActionFlag(*filt, clone_flag)) {
                                setActionFlag(command, clone_flag);
                            }
                        }
                    }
                    command.setDestination(ept->handle);
                    routeMessage(command);
                    command.setAction(CMD_ADD_ENDPOINT);
                    if (command.counter == static_cast<uint16_t>(InterfaceType::ENDPOINT)) {
                        toggleActionFlag(command, destination_target);
                        command.name(ept->key);
                        command.setString(typeStringLoc, ept->type);
                    }
                    command.swapSourceDest();
                    // command.setSource(ept->handle);

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
                    unknownHandles.addUnknownPublication(std::string(command.name()),
                                                         command.getSource(),
                                                         command.flags);
                    break;
                case CMD_ADD_NAMED_INPUT:
                    unknownHandles.addUnknownInput(std::string(command.name()),
                                                   command.getSource(),
                                                   command.flags);
                    if (!command.getStringData().empty()) {
                        auto* pub = handles.findHandle(command.getSource());
                        if (pub == nullptr) {
                            // an anonymous publisher is adding an input
                            auto& apub = handles.addHandle(command.source_id,
                                                           command.source_handle,
                                                           InterfaceType::PUBLICATION,
                                                           std::string(),
                                                           command.getString(typeStringLoc),
                                                           command.getString(unitStringLoc));

                            addLocalInfo(apub, command);
                        }
                    }
                    break;
                case CMD_ADD_NAMED_ENDPOINT:
                    unknownHandles.addUnknownEndpoint(std::string(command.name()),
                                                      command.getSource(),
                                                      command.flags);
                    if (!command.getStringData().empty()) {
                        auto* filt = handles.findHandle(command.getSource());
                        if (filt == nullptr) {
                            // an anonymous filter is adding an endpoint
                            auto& afilt = handles.addHandle(command.source_id,
                                                            command.source_handle,
                                                            InterfaceType::FILTER,
                                                            std::string(),
                                                            command.getString(typeStringLoc),
                                                            command.getString(typeOutStringLoc));

                            addLocalInfo(afilt, command);
                        }
                    }
                    break;
                case CMD_ADD_NAMED_FILTER:
                    unknownHandles.addUnknownFilter(std::string(command.name()),
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
            auto* pub = handles.getPublication(command.name());
            if (pub != nullptr) {
                command.setAction(CMD_REMOVE_SUBSCRIBER);
                command.setDestination(pub->handle);
                command.payload.clear();
                routeMessage(command);
                command.setAction(CMD_REMOVE_PUBLICATION);
                command.swapSourceDest();
                routeMessage(command);
                foundInterface = true;
            }
        } break;
        case CMD_REMOVE_NAMED_INPUT: {
            auto* inp = handles.getInput(command.name());
            if (inp != nullptr) {
                command.setAction(CMD_REMOVE_PUBLICATION);
                command.setDestination(inp->handle);
                command.payload.clear();
                routeMessage(command);
                command.setAction(CMD_REMOVE_SUBSCRIBER);
                command.swapSourceDest();
                routeMessage(command);
                foundInterface = true;
            }
        } break;
        case CMD_REMOVE_NAMED_FILTER: {
            auto* filt = handles.getFilter(command.name());
            if (filt != nullptr) {
                command.setAction(CMD_REMOVE_ENDPOINT);
                command.setDestination(filt->handle);
                command.payload.clear();
                routeMessage(command);
                command.setAction(CMD_REMOVE_FILTER);
                command.swapSourceDest();
                routeMessage(command);
                foundInterface = true;
            }
        } break;
        case CMD_REMOVE_NAMED_ENDPOINT: {
            auto* ept = handles.getEndpoint(command.name());
            if (ept != nullptr) {
                command.setAction(CMD_REMOVE_FILTER);
                command.setDestination(ept->handle);
                command.payload.clear();
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
                        fmt::format("attempt to remove unrecognized target {} ", command.name()));
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
    LOG_ERROR(global_broker_id_local, getIdentifier(), cmd.payload.to_string());
    if (cmd.action() == CMD_LOCAL_ERROR) {
        if (terminate_on_error) {
            LOG_ERROR(global_broker_id_local,
                      getIdentifier(),
                      "Error Escalation: Federation terminating");
            cmd.setAction(CMD_GLOBAL_ERROR);
            setErrorState(cmd.messageID, cmd.payload.to_string());
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
    if (handles.getPublication(m.name()) != nullptr) {
        ActionMessage eret(CMD_LOCAL_ERROR, global_broker_id_local, m.source_id);
        eret.dest_handle = m.source_handle;
        eret.messageID = defs::Errors::REGISTRATION_FAILURE;
        eret.payload = fmt::format("Duplicate PUBLICATION names ({})", m.name());
        propagateError(std::move(eret));
        return;
    }
    auto& pub = handles.addHandle(m.source_id,
                                  m.source_handle,
                                  InterfaceType::PUBLICATION,
                                  std::string(m.name()),
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
    if (handles.getInput(m.name()) != nullptr) {
        ActionMessage eret(CMD_LOCAL_ERROR, global_broker_id_local, m.source_id);
        eret.dest_handle = m.source_handle;
        eret.messageID = defs::Errors::REGISTRATION_FAILURE;
        eret.payload = fmt::format("Duplicate input names ({})", m.name());
        propagateError(std::move(eret));
        return;
    }
    auto& inp = handles.addHandle(m.source_id,
                                  m.source_handle,
                                  InterfaceType::INPUT,
                                  std::string(m.name()),
                                  m.getString(0),
                                  m.getString(1));

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
    if (handles.getEndpoint(m.name()) != nullptr) {
        ActionMessage eret(CMD_LOCAL_ERROR, global_broker_id_local, m.source_id);
        eret.dest_handle = m.source_handle;
        eret.messageID = defs::Errors::REGISTRATION_FAILURE;
        eret.payload = fmt::format("Duplicate endpoint names ({})", m.name());
        propagateError(std::move(eret));
        return;
    }
    auto& ept = handles.addHandle(m.source_id,
                                  m.source_handle,
                                  InterfaceType::ENDPOINT,
                                  std::string(m.name()),
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
    if (handles.getFilter(m.name()) != nullptr) {
        ActionMessage eret(CMD_LOCAL_ERROR, global_broker_id_local, m.source_id);
        eret.dest_handle = m.source_handle;
        eret.messageID = defs::Errors::REGISTRATION_FAILURE;
        eret.payload = fmt::format("Duplicate filter names ({})", m.name());
        propagateError(std::move(eret));
        return;
    }

    auto& filt = handles.addHandle(m.source_id,
                                   m.source_handle,
                                   InterfaceType::FILTER,
                                   std::string(m.name()),
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

void CoreBroker::linkInterfaces(ActionMessage& command)
{
    switch (command.action()) {
        case CMD_DATA_LINK: {
            auto* pub = handles.getPublication(command.name());
            if (pub != nullptr) {
                command.name(command.getString(targetStringLoc));
                command.setAction(CMD_ADD_NAMED_INPUT);
                command.setSource(pub->handle);
                checkForNamedInterface(command);
            } else {
                auto* input = handles.getInput(command.getString(targetStringLoc));
                if (input == nullptr) {
                    if (isRootc) {
                        unknownHandles.addDataLink(std::string(command.name()),
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
        case CMD_ENDPOINT_LINK: {
            auto* ept = handles.getEndpoint(command.name());
            if (ept != nullptr) {
                command.name(command.getString(targetStringLoc));
                command.setAction(CMD_ADD_NAMED_ENDPOINT);
                setActionFlag(command, destination_target);
                command.counter = static_cast<uint16_t>(InterfaceType::ENDPOINT);
                command.setSource(ept->handle);
                checkForNamedInterface(command);
            } else {
                auto* target = handles.getEndpoint(command.getString(targetStringLoc));
                if (target == nullptr) {
                    if (isRootc) {
                        unknownHandles.addEndpointLink(std::string(command.name()),
                                                       command.getString(targetStringLoc));
                    } else {
                        routeMessage(command);
                    }
                } else {
                    command.setAction(CMD_ADD_NAMED_ENDPOINT);
                    command.setSource(target->handle);
                    command.counter = static_cast<uint16_t>(InterfaceType::ENDPOINT);
                    checkForNamedInterface(command);
                }
            }
        } break;
        case CMD_FILTER_LINK: {
            auto* filt = handles.getFilter(command.name());
            if (filt != nullptr) {
                command.payload = command.getString(targetStringLoc);
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
                            unknownHandles.addDestinationFilterLink(std::string(command.name()),
                                                                    command.getString(
                                                                        targetStringLoc));
                        } else {
                            unknownHandles.addSourceFilterLink(std::string(command.name()),
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
        default:
            break;
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
    if (transitionBrokerState(BrokerState::created, BrokerState::configuring)) {
        auto result = parseArgs(configureString);
        if (result != 0) {
            setBrokerState(BrokerState::created);
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
    if (transitionBrokerState(BrokerState::created, BrokerState::configuring)) {
        auto result = parseArgs(argc, argv);
        if (result != 0) {
            setBrokerState(BrokerState::created);
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
    if (transitionBrokerState(BrokerState::created, BrokerState::configuring)) {
        auto result = parseArgs(std::move(args));
        if (result != 0) {
            setBrokerState(BrokerState::created);
            if (result < 0) {
                throw(helics::InvalidParameter("invalid arguments in command line"));
            }
            return;
        }
        configureBase();
    }
}

double CoreBroker::getSimulationTime() const
{
    return static_cast<double>(simTime.load());
}

std::shared_ptr<helicsCLI11App> CoreBroker::generateCLI()
{
    auto app = std::make_shared<helicsCLI11App>("Option for Broker");
    app->remove_helics_specifics();
    app->add_flag_callback(
        "--root", [this]() { setAsRoot(); }, "specify whether the broker is a root");
    auto* tfed = app->add_option(
        "--timemonitor",
        mTimeMonitorFederate,
        "specify a federate to use as the primary time monitor for logging and indicator purpose, it has no actual impact on the cosimulation");
    app->add_option("--timemonitorperiod",
                    mTimeMonitorPeriod,
                    "period to display logs of times from the time monitor federate")
        ->needs(tfed);
    return app;
}

void CoreBroker::setAsRoot()
{
    if (getBrokerState() < BrokerState::connected) {
        _isRoot = true;
        global_id = gRootBrokerID;
    }
}

bool CoreBroker::connect()
{
    if (getBrokerState() < BrokerState::connected) {
        if (transitionBrokerState(BrokerState::configured, BrokerState::connecting)) {
            LOG_CONNECTIONS(parent_broker_id, getIdentifier(), "connecting");
            timeoutMon->setTimeout(std::chrono::milliseconds(timeout));
            auto res = brokerConnect();
            if (res) {
                disconnection.activate();
                setBrokerState(BrokerState::connected);
                ActionMessage setup(CMD_BROKER_SETUP);
                addActionMessage(setup);
                if (!_isRoot) {
                    ActionMessage m(CMD_REG_BROKER);
                    m.source_id = GlobalFederateId{};
                    m.name(getIdentifier());
                    if (no_ping) {
                        setActionFlag(m, slow_responding_flag);
                    }
                    if (useJsonSerialization) {
                        setActionFlag(m, use_json_serialization_flag);
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
                setBrokerState(BrokerState::configured);
            }
            return res;
        }
        if (getBrokerState() == BrokerState::connecting) {
            while (getBrokerState() == BrokerState::connecting) {
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
    return ((state >= BrokerState::connected) && (state < BrokerState::terminating));
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
    if (cBrokerState >= BrokerState::terminating) {
        return;
    }
    if (cBrokerState > BrokerState::configured) {
        LOG_CONNECTIONS(parent_broker_id, getIdentifier(), "||disconnecting");
        setBrokerState(BrokerState::terminating);
        brokerDisconnect();
    }
    if (cBrokerState!=BrokerState::connected_error) {
        setBrokerState(BrokerState::terminated);
    } else {
        setBrokerState(BrokerState::errored);
    }

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
        if (cnt % 13 == 0) {
            std::cerr << "waiting on disconnect " << std::endl;
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

void CoreBroker::routeMessage(ActionMessage& cmd, GlobalFederateId dest)
{
    if (dest == GlobalFederateId{}) {
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

void CoreBroker::routeMessage(ActionMessage&& cmd, GlobalFederateId dest)
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
    for (const auto& broker : mBrokers) {
        if ((!broker._nonLocal) && (broker.state < connection_state::disconnected)) {
            cmd.dest_id = broker.global_id;
            transmit(broker.route, cmd);
        }
    }
}

void CoreBroker::executeInitializationOperations()
{
    if (brokerKey == universalKey) {
        LOG_SUMMARY(global_broker_id_local, getIdentifier(), "Broker started with universal key");
    }
    checkDependencies();
    if (!mTimeMonitorFederate.empty()) {
        loadTimeMonitor(true, std::string{});
    }
    if (unknownHandles.hasUnknowns()) {
        if (unknownHandles.hasNonOptionalUnknowns()) {
            if (unknownHandles.hasRequiredUnknowns()) {
                ActionMessage eMiss(CMD_ERROR);
                eMiss.source_id = global_broker_id_local;
                eMiss.messageID = defs::Errors::CONNECTION_FAILURE;
                unknownHandles.processRequiredUnknowns([this, &eMiss](const std::string& target,
                                                                      char type,
                                                                      GlobalHandle handle) {
                    switch (type) {
                        case 'p':
                            eMiss.payload =
                                fmt::format("Unable to connect to required publication target {}",
                                            target);
                            LOG_ERROR(parent_broker_id, getIdentifier(), eMiss.payload.to_string());
                            break;
                        case 'i':
                            eMiss.payload =
                                fmt::format("Unable to connect to required input target {}",
                                            target);
                            LOG_ERROR(parent_broker_id, getIdentifier(), eMiss.payload.to_string());
                            break;
                        case 'f':
                            eMiss.payload =
                                fmt::format("Unable to connect to required filter target {}",
                                            target);
                            LOG_ERROR(parent_broker_id, getIdentifier(), eMiss.payload.to_string());
                            break;
                        case 'e':
                            eMiss.payload =
                                fmt::format("Unable to connect to required endpoint target {}",
                                            target);
                            LOG_ERROR(parent_broker_id, getIdentifier(), eMiss.payload.to_string());
                            break;
                        default:
                            // LCOV_EXCL_START
                            eMiss.payload =
                                fmt::format("Unable to connect to required unknown target {}",
                                            target);
                            LOG_ERROR(parent_broker_id, getIdentifier(), eMiss.payload.to_string());
                            break;
                            // LCOV_EXCL_STOP
                    }
                    eMiss.setDestination(handle);
                    routeMessage(eMiss);
                });
                eMiss.payload = "Missing required connections";
                eMiss.dest_handle = InterfaceHandle{};
                broadcast(eMiss);
                sendDisconnect(CMD_GLOBAL_DISCONNECT);
                addActionMessage(CMD_STOP);
                return;
            }
            ActionMessage wMiss(CMD_WARNING);
            wMiss.source_id = global_broker_id_local;
            wMiss.messageID = defs::Errors::CONNECTION_FAILURE;
            unknownHandles.processNonOptionalUnknowns([this, &wMiss](const std::string& target,
                                                                     char type,
                                                                     GlobalHandle handle) {
                switch (type) {
                    case 'p':
                        wMiss.payload =
                            fmt::format("Unable to connect to publication target {}", target);
                        LOG_WARNING(parent_broker_id, getIdentifier(), wMiss.payload.to_string());
                        break;
                    case 'i':
                        wMiss.payload = fmt::format("Unable to connect to input target {}", target);
                        LOG_WARNING(parent_broker_id, getIdentifier(), wMiss.payload.to_string());
                        break;
                    case 'f':
                        wMiss.payload =
                            fmt::format("Unable to connect to filter target {}", target);
                        LOG_WARNING(parent_broker_id, getIdentifier(), wMiss.payload.to_string());
                        break;
                    case 'e':
                        wMiss.payload =
                            fmt::format("Unable to connect to endpoint target {}", target);
                        LOG_WARNING(parent_broker_id, getIdentifier(), wMiss.payload.to_string());
                        break;
                    default:
                        // LCOV_EXCL_START
                        wMiss.payload =
                            fmt::format("Unable to connect to undefined target {}", target);
                        LOG_WARNING(parent_broker_id, getIdentifier(), wMiss.payload.to_string());
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
    setBrokerState(BrokerState::operating);
    broadcast(m);
    timeCoord->enteringExecMode();
    auto res = timeCoord->checkExecEntry();
    if (res == MessageProcessingResult::NEXT_STEP) {
        enteredExecutionMode = true;
    }
    logFlush();
}

void CoreBroker::FindandNotifyInputTargets(BasicHandleInfo& handleInfo)
{
    auto Handles = unknownHandles.checkForInputs(handleInfo.key);
    for (auto& target : Handles) {
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
        m.name(sub);
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
        // notify the filter or endpoint about its target
        ActionMessage m(CMD_ADD_ENDPOINT);
        m.setSource(handleInfo.handle);
        m.setDestination(target.first);
        m.flags = target.second;
        m.name(handleInfo.key);
        if (!handleInfo.type.empty()) {
            m.setString(typeStringLoc, handleInfo.type);
        }
        transmit(getRoute(m.dest_id), m);

        const auto* iface = handles.findHandle(target.first);
        if (iface->handleType == InterfaceType::ENDPOINT) {
            // notify the endpoint about its endpoint
            m.setAction(CMD_ADD_ENDPOINT);
            m.name(iface->key);
            if (!iface->type.empty()) {
                m.setString(typeStringLoc, iface->type);
            }
            toggleActionFlag(m, destination_target);
        } else {
            // notify the endpoint about its filter
            m.setAction(CMD_ADD_FILTER);
        }

        m.swapSourceDest();
        m.flags = target.second;
        transmit(getRoute(m.dest_id), m);
    }
    auto EptTargets = unknownHandles.checkForEndpointLinks(handleInfo.key);
    for (const auto& ept : EptTargets) {
        ActionMessage m(CMD_ADD_NAMED_ENDPOINT);
        m.name(ept);
        m.setSource(handleInfo.handle);
        setActionFlag(m, destination_target);
        m.counter = static_cast<uint16_t>(InterfaceType::ENDPOINT);
        checkForNamedInterface(m);
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
        m.name(target);
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
        m.name(target);
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
    sendToLogger(command.source_id,
                 LogLevels::ERROR_LEVEL,
                 std::string(),
                 command.payload.to_string());
    if (command.source_id == global_broker_id_local) {
        setBrokerState(BrokerState::errored);
        if (command.action() == CMD_GLOBAL_ERROR) {
            setErrorState(command.messageID, command.payload.to_string());
        }
        broadcast(command);
        if (!isRootc) {
            command.setAction(CMD_LOCAL_ERROR);
            transmit(parent_route_id, std::move(command));
        }
        return;
    }

    if (command.source_id == parent_broker_id || command.source_id == gRootBrokerID) {
        setBrokerState(BrokerState::errored);
        if (command.action() == CMD_GLOBAL_ERROR) {
            setErrorState(command.messageID, command.payload.to_string());
        }
        broadcast(command);
        return;
    }

    auto* brk = getBrokerById(GlobalBrokerId(command.source_id));
    if (brk == nullptr) {
        auto fed = mFederates.find(command.source_id);
        if (fed != mFederates.end()) {
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
            setErrorState(command.messageID, command.payload.to_string());
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
    auto* brk = getBrokerById(GlobalBrokerId(command.source_id));
    switch (command.action()) {
        case CMD_DISCONNECT:
        case CMD_PRIORITY_DISCONNECT:
            if (command.dest_id == global_broker_id_local) {
                // deal with the time implications of the message
                if (hasTimeDependency) {
                    if (!enteredExecutionMode) {
                        timeCoord->processTimeMessage(command);
                        auto res = timeCoord->checkExecEntry();
                        if (res == MessageProcessingResult::NEXT_STEP) {
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
                        sendDisconnect(CMD_GLOBAL_DISCONNECT);
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
                        if ((!isRootc) && (getBrokerState() < BrokerState::operating)) {
                            command.setAction((brk->_core) ? CMD_DISCONNECT_CORE :
                                                             CMD_DISCONNECT_BROKER);
                            transmit(parent_route_id, command);
                        }
                        removeRoute(brk->route);
                    } else {
                        if ((!isRootc) && (getBrokerState() < BrokerState::operating)) {
                            if (brk != nullptr) {
                                command.setAction((brk->_core) ? CMD_DISCONNECT_CORE :
                                                                 CMD_DISCONNECT_BROKER);
                                transmit(parent_route_id, command);
                            }
                        }
                    }
                }
            } else if (command.dest_id == mTimeMonitorLocalFederateId) {
                processTimeMonitorMessage(command);
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

void CoreBroker::checkInFlightQueries(GlobalBrokerId brkid)
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

void CoreBroker::markAsDisconnected(GlobalBrokerId brkid)
{
    // using regular loop here since dual mapped vector shouldn't produce a modifiable lvalue
    for (size_t ii = 0; ii < mBrokers.size(); ++ii) {  // NOLINT
        auto& brk = mBrokers[ii];
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
    for (size_t ii = 0; ii < mFederates.size(); ++ii) {  // NOLINT
        auto& fed = mFederates[ii];

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
    if (getBrokerState() < BrokerState::operating) {
        if (isRootc) {
            ActionMessage dis(CMD_BROADCAST_DISCONNECT);
            dis.source_id = brk.global_id;
            broadcast(dis);
            unknownHandles.clearFederateUnknowns(brk.global_id);
            if (!brk._core) {
                for (const auto& subbrk : mBrokers) {
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
    cmd.messageID = defs::Properties::LOG_LEVEL;
    cmd.setExtraData(logLevel);
    addActionMessage(cmd);
}

void CoreBroker::setLogFile(const std::string& lfile)
{
    ActionMessage cmd(CMD_BROKER_CONFIGURE);
    cmd.dest_id = global_id.load();
    cmd.messageID = UPDATE_LOGGING_FILE;
    cmd.payload = lfile;
    addActionMessage(cmd);
}

// public query function
std::string CoreBroker::query(const std::string& target,
                              const std::string& queryStr,
                              HelicsSequencingModes mode)
{
    if (getBrokerState() >= BrokerState::terminating) {
        if (target == "broker" || target == getIdentifier() || target.empty() ||
            (target == "root" && _isRoot) || (target == "federation" && _isRoot)) {
            auto res = quickBrokerQueries(queryStr);
            if (!res.empty()) {
                return res;
            }
            if (queryStr == "logs") {
                Json::Value base;
                base["name"] = getIdentifier();
                if (uuid_like) {
                    base["uuid"] = getIdentifier();
                }
                base["id"] = global_id.load().baseValue();
                bufferToJson(mLogManager->getLogBuffer(), base);
                return fileops::generateJsonString(base);
            }
        }
        return generateJsonErrorResponse(JsonErrorCodes::DISCONNECTED, "Broker has terminated");
    }
    auto gid = global_id.load();
    if (target == "broker" || target == getIdentifier() || target.empty()) {
        auto res = quickBrokerQueries(queryStr);
        if (!res.empty()) {
            return res;
        }
        if (queryStr == "address") {
            res = generateJsonQuotedString(getAddress());
            return res;
        }
        ActionMessage querycmd(mode == HELICS_SEQUENCING_MODE_FAST ? CMD_BROKER_QUERY :
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
            return generateJsonErrorResponse(JsonErrorCodes::NOT_FOUND,
                                             "broker has no parent");  // LCOV_EXCL_LINE
        }
        ActionMessage querycmd(mode == HELICS_SEQUENCING_MODE_FAST ? CMD_BROKER_QUERY :
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
        ActionMessage querycmd(mode == HELICS_SEQUENCING_MODE_FAST ? CMD_BROKER_QUERY :
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

    ActionMessage querycmd(mode == HELICS_SEQUENCING_MODE_FAST ? CMD_QUERY : CMD_QUERY_ORDERED);
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
}

void CoreBroker::setGlobal(const std::string& valueName, const std::string& value)
{
    ActionMessage querycmd(CMD_SET_GLOBAL);
    querycmd.source_id = global_id.load();
    querycmd.payload = valueName;
    querycmd.setStringData(value);
    transmitToParent(std::move(querycmd));
}

void CoreBroker::sendCommand(const std::string& target,
                             const std::string& commandStr,
                             HelicsSequencingModes mode)
{
    if (commandStr == "flush") {
        query(target, "global_flush", HelicsSequencingModes::HELICS_SEQUENCING_MODE_ORDERED);
        return;
    }
    ActionMessage cmdcmd(mode == HELICS_SEQUENCING_MODE_ORDERED ? CMD_SEND_COMMAND_ORDERED :
                                                                  CMD_SEND_COMMAND);
    cmdcmd.source_id = global_id.load();
    cmdcmd.payload = commandStr;
    cmdcmd.setString(targetStringLoc, target);
    cmdcmd.setString(sourceStringLoc, getIdentifier());
    transmitToParent(std::move(cmdcmd));
}

static const std::map<std::string, std::pair<std::uint16_t, bool>> mapIndex{
    {"global_time", {CURRENT_TIME_MAP, true}},
    {"federate_map", {FEDERATE_MAP, false}},
    {"dependency_graph", {DEPENDENCY_GRAPH, false}},
    {"data_flow_graph", {DATA_FLOW_GRAPH, false}},
    {"version_all", {VERSION_ALL, false}},
    {"global_state", {GLOBAL_STATE, true}},
    {"global_time_debugging", {GLOBAL_TIME_DEBUGGING, true}},
    {"global_status", {GLOBAL_STATUS, true}},
    {"global_flush", {GLOBAL_FLUSH, true}}};

std::string CoreBroker::quickBrokerQueries(const std::string& request) const
{
    if (request == "isinit") {
        return (getBrokerState() >= BrokerState::operating) ? std::string("true") :
                                                              std::string("false");
    }
    if (request == "isconnected") {
        return (isConnected()) ? std::string("true") : std::string("false");
    }
    if (request == "name" || request == "identifier") {
        return std::string{"\""} + getIdentifier() + '"';
    }
    if (request == "exists") {
        return "true";
    }
    if ((request == "queries") || (request == "available_queries")) {
        return "[\"isinit\",\"isconnected\",\"name\",\"identifier\",\"address\",\"queries\",\"address\",\"counts\",\"summary\",\"federates\",\"brokers\",\"inputs\",\"endpoints\","
               "\"publications\",\"filters\",\"federate_map\",\"dependency_graph\",\"data_flow_graph\",\"dependencies\",\"dependson\",\"dependents\","
               "\"monitor\",\"logs\","
               "\"current_time\",\"current_state\",\"global_state\",\"status\",\"global_time\",\"global_status\",\"version\",\"version_all\",\"exists\",\"global_flush\"]";
    }
    if (request == "address") {
        return std::string{"\""} + getAddress() + '"';
    }
    if (request == "version") {
        return std::string{"\""} + versionString + '"';
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
        return fileops::generateJsonString(base);
    }
    return {};
}

std::string CoreBroker::generateQueryAnswer(const std::string& request, bool force_ordering)
{
    auto res = quickBrokerQueries(request);
    if (!res.empty()) {
        return res;
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
        base["brokers"] = static_cast<int>(mBrokers.size());
        base["federates"] = static_cast<int>(mFederates.size());
        base["countable_federates"] = getCountableFederates();
        base["handles"] = static_cast<int>(handles.size());
        return fileops::generateJsonString(base);
    }
    if (request == "summary") {
        return generateFederationSummary();
    }
    if (request == "monitor") {
        return std::string{"\""} + mTimeMonitorFederate + '"';
    }
    if (request == "logs") {
        Json::Value base;
        base["name"] = getIdentifier();
        if (uuid_like) {
            base["uuid"] = getIdentifier();
        }
        base["id"] = global_broker_id_local.baseValue();
        bufferToJson(mLogManager->getLogBuffer(), base);
        return fileops::generateJsonString(base);
    }
    if (request == "federates") {
        return generateStringVector(mFederates, [](auto& fed) { return fed.name; });
    }
    if (request == "brokers") {
        return generateStringVector(mBrokers, [](auto& brk) { return brk.name; });
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
        for (const auto& fed : mFederates) {
            Json::Value fedstate;
            fedstate["name"] = fed.name;
            fedstate["state"] = state_string(fed.state);
            fedstate["id"] = fed.global_id.baseValue();
            base["federates"].append(std::move(fedstate));
        }
        base["cores"] = Json::arrayValue;
        base["brokers"] = Json::arrayValue;
        for (const auto& brk : mBrokers) {
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
        return fileops::generateJsonString(base);
    }
    if (request == "current_time") {
        if (!hasTimeDependency) {
            return "{}";
        }
        return timeCoord->printTimeStatus();
    }
    if (request == "time_monitor") {
        if (mTimeMonitorFederateId.isValid()) {
            return fmt::format(R"raw({{"time":{}, "federate":{}}})raw",
                               static_cast<double>(mTimeMonitorCurrentTime),
                               mTimeMonitorFederate);
        }
        return "{}";
    }
    if (request == "global_status") {
        if (!isConnected()) {
            Json::Value gs;
            gs["status"] = "disconnected";
            gs["timestep"] = -1;
            return fileops::generateJsonString(gs);
        }
    }
    auto mi = mapIndex.find(std::string(request));
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
            if (index == GLOBAL_STATUS) {
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
            [](auto& handle) { return (handle.handleType == InterfaceType::INPUT); });
    }
    if (request == "publications") {
        return generateStringVector_if(
            handles,
            [](auto& handle) { return handle.key; },
            [](auto& handle) { return (handle.handleType == InterfaceType::PUBLICATION); });
    }
    if (request == "filters") {
        return generateStringVector_if(
            handles,
            [](auto& handle) { return handle.key; },
            [](auto& handle) { return (handle.handleType == InterfaceType::FILTER); });
    }
    if (request == "endpoints") {
        return generateStringVector_if(
            handles,
            [](auto& handle) { return handle.key; },
            [](auto& handle) { return (handle.handleType == InterfaceType::ENDPOINT); });
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
        return fileops::generateJsonString(base);
    }
    return generateJsonErrorResponse(JsonErrorCodes::BAD_REQUEST, "unrecognized broker query");
}

std::string CoreBroker::generateGlobalStatus(fileops::JsonMapBuilder& builder)
{
    auto cstate = generateQueryAnswer("current_state", false);
    auto jv = fileops::loadJsonStr(cstate);
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
        return fileops::generateJsonString(v);
    }
    Time mv{Time::maxVal()};
    if (!builder.getJValue()["cores"][0].isObject()) {
        state = "init_requested";
    }
    for (auto& cr : builder.getJValue()["cores"]) {
        for (auto& fed : cr["federates"]) {
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

    return fileops::generateJsonString(v);
}

std::string CoreBroker::getNameList(std::string gidString) const
{
    if (gidString.back() == ']') {
        gidString.pop_back();
    }
    if (gidString.front() == '[') {
        gidString = gidString.substr(1);
    }
    auto val = gmlc::utilities::str2vector<int>(gidString, -23, ",:;");
    gidString.clear();
    gidString.push_back('[');
    size_t index = 0;
    while (index + 1 < val.size()) {
        const auto* info = handles.findHandle(
            GlobalHandle(GlobalFederateId(val[index]), InterfaceHandle(val[index + 1])));
        if (info != nullptr) {
            gidString.append(generateJsonQuotedString(info->key));
            gidString.push_back(',');
        }

        index += 2;
    }
    if (gidString.back() == ',') {
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
        mapBuilders.resize(static_cast<size_t>(index) + 1);
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
    if (index == GLOBAL_FLUSH) {
        queryReq.setAction(CMD_BROKER_QUERY_ORDERED);
    }
    queryReq.payload = request;
    queryReq.source_id = global_broker_id_local;
    queryReq.counter = index;  // indicating which processing to use
    bool hasCores = false;
    bool hasBrokers = false;
    for (const auto& broker : mBrokers) {
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
                    if (index == GLOBAL_STATE) {
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
        case FEDERATE_MAP:
        case CURRENT_TIME_MAP:
        case GLOBAL_STATUS:
        case DATA_FLOW_GRAPH:
        case GLOBAL_FLUSH:
        default:
            break;
        case DEPENDENCY_GRAPH: {
            base["dependents"] = Json::arrayValue;
            for (const auto& dep : timeCoord->getDependents()) {
                base["dependents"].append(dep.baseValue());
            }
            base["dependencies"] = Json::arrayValue;
            for (const auto& dep : timeCoord->getDependencies()) {
                base["dependencies"].append(dep.baseValue());
            }
        } break;
        case VERSION_ALL:
            base["version"] = versionString;
            break;
        case GLOBAL_STATE:
            base["state"] = brokerStateName(getBrokerState());
            base["status"] = isConnected();
            break;
        case GLOBAL_TIME_DEBUGGING:
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
    queryRep.payload = generateQueryAnswer(std::string(m.payload.to_string()), force_ordered);
    queryRep.counter = m.counter;
    if (queryRep.payload.to_string() == "#wait") {
        if (queryRep.dest_id == global_broker_id_local) {
            if (queryTimeouts.empty()) {
                setTickForwarding(TickForwardingReasons::QUERY_TIMEOUT, true);
            }
            queryTimeouts.emplace_back(queryRep.messageID, std::chrono::steady_clock::now());
        }
        std::get<1>(mapBuilders[mapIndex.at(std::string(m.payload.to_string())).first])
            .push_back(queryRep);
    } else if (queryRep.dest_id == global_broker_id_local) {
        activeQueries.setDelayedValue(m.messageID, std::string(queryRep.payload.to_string()));
    } else {
        routeMessage(std::move(queryRep), m.source_id);
    }
}

/** check for fed queries that can be answered by the broker*/
static std::string checkFedQuery(const BasicFedInfo& fed, std::string_view query)
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
        response.push_back('"');
        response.append(state_string(fed.state));
        response.push_back('"');
    } else if (query == "isinit") {
        if (fed.state >= connection_state::operating) {
            response = "true";
            // if it is false we need to actually go check the federate directly
        }
    }
    return response;
}
/** check for broker queries that can be answered by the broker*/
static std::string checkBrokerQuery(const BasicBrokerInfo& brk, std::string_view query)
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
                global_values[std::string(cmd.payload.to_string())] = cmd.getString(0);
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
        queryResp.payload = getNameList(std::string(m.payload.to_string()));
        if (queryResp.dest_id == global_broker_id_local) {
            activeQueries.setDelayedValue(m.messageID, std::string(queryResp.payload.to_string()));
        } else {
            transmit(getRoute(queryResp.dest_id), queryResp);
        }
    } else if ((isRootc) && (target == "global" || target == "global_value")) {
        ActionMessage queryResp(force_ordered ? CMD_QUERY_REPLY_ORDERED : CMD_QUERY_REPLY);
        queryResp.dest_id = m.source_id;
        queryResp.source_id = global_broker_id_local;
        queryResp.messageID = m.messageID;

        auto gfind = global_values.find(std::string(m.payload.to_string()));
        if (gfind != global_values.end()) {
            if (target == "global_value") {
                queryResp.payload = gfind->second;
            } else {
                Json::Value v;
                v["name"] = std::string(m.payload.to_string());
                v["value"] = gfind->second;
                queryResp.payload = fileops::generateJsonString(v);
            }
        } else if (m.payload.to_string() == "list") {
            queryResp.payload =
                generateStringVector(global_values, [](const auto& gv) { return gv.first; });
        } else if (m.payload.to_string() == "all") {
            fileops::JsonMapBuilder globalSet;
            auto& jv = globalSet.getJValue();
            for (auto& val : global_values) {
                jv[val.first] = val.second;
            }
            queryResp.payload = globalSet.generate();
        } else {
            queryResp.payload =
                generateJsonErrorResponse(JsonErrorCodes::NOT_FOUND, "Global value not found");
        }
        if (queryResp.dest_id == global_broker_id_local) {
            activeQueries.setDelayedValue(m.messageID, std::string(queryResp.payload.to_string()));
        } else {
            transmit(getRoute(queryResp.dest_id), queryResp);
        }
    } else {
        route_id route = parent_route_id;
        auto fed = mFederates.find(target);
        std::string response;
        if (fed != mFederates.end()) {
            route = fed->route;
            m.dest_id = fed->parent;
            response = checkFedQuery(*fed, m.payload.to_string());
            if (response.empty() && fed->state >= connection_state::error) {
                route = parent_route_id;
                switch (fed->state) {
                    case connection_state::error:
                        response = generateJsonErrorResponse(JsonErrorCodes::SERVICE_UNAVAILABLE,
                                                             "federate is in error state");
                        break;
                    case connection_state::disconnected:
                    case connection_state::request_disconnect:
                        response = generateJsonErrorResponse(JsonErrorCodes::SERVICE_UNAVAILABLE,
                                                             "federate is disconnected");
                        break;
                    default:
                        break;
                }
            }
        } else {
            auto broker = mBrokers.find(target);
            if (broker != mBrokers.end()) {
                route = broker->route;
                m.dest_id = broker->global_id;
                response = checkBrokerQuery(*broker, m.payload.to_string());
                if (response.empty() && broker->state >= connection_state::error) {
                    route = parent_route_id;
                    switch (broker->state) {
                        case connection_state::error:
                            response =
                                generateJsonErrorResponse(JsonErrorCodes::SERVICE_UNAVAILABLE,
                                                          "target broker is in error state");
                            break;
                        case connection_state::disconnected:
                        case connection_state::request_disconnect:
                            response =
                                generateJsonErrorResponse(JsonErrorCodes::SERVICE_UNAVAILABLE,
                                                          "federate is disconnected");
                            break;
                        default:
                            break;
                    }
                }
            } else if (isRootc && m.payload.to_string() == "exists") {
                response = "false";
            }
        }
        if (((route == parent_route_id) && (isRootc)) || !response.empty()) {
            if (response.empty()) {
                response = generateJsonErrorResponse(JsonErrorCodes::NOT_FOUND, "query not valid");
            }
            ActionMessage queryResp(force_ordered ? CMD_QUERY_REPLY_ORDERED : CMD_QUERY_REPLY);
            queryResp.dest_id = m.source_id;
            queryResp.source_id = global_broker_id_local;
            queryResp.messageID = m.messageID;

            queryResp.payload = response;
            if (queryResp.dest_id == global_broker_id_local) {
                activeQueries.setDelayedValue(m.messageID,
                                              std::string(queryResp.payload.to_string()));
            } else {
                transmit(getRoute(queryResp.dest_id), queryResp);
            }
        } else {
            if (m.source_id == global_broker_id_local) {
                if (queryTimeouts.empty()) {
                    setTickForwarding(TickForwardingReasons::QUERY_TIMEOUT, true);
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
                    activeQueries.setDelayedValue(
                        qt.first,
                        generateJsonErrorResponse(JsonErrorCodes::GATEWAY_TIMEOUT,
                                                  "query timeout"));
                    qt.first = 0;
                }
            }
        }
        while (!queryTimeouts.empty() && queryTimeouts.front().first == 0) {
            queryTimeouts.pop_front();
        }
        if (queryTimeouts.empty()) {
            setTickForwarding(TickForwardingReasons::QUERY_TIMEOUT, false);
        }
    }
}

void CoreBroker::processQueryResponse(const ActionMessage& m)
{
    if (m.counter == GENERAL_QUERY) {
        activeQueries.setDelayedValue(m.messageID, std::string(m.payload.to_string()));
        return;
    }
    if (isValidIndex(m.counter, mapBuilders)) {
        auto& builder = std::get<0>(mapBuilders[m.counter]);
        auto& requestors = std::get<1>(mapBuilders[m.counter]);
        if (builder.addComponent(std::string(m.payload.to_string()), m.messageID)) {
            std::string str;
            switch (m.counter) {
                case GLOBAL_STATUS:
                    str = generateGlobalStatus(builder);
                    break;
                case GLOBAL_FLUSH:
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

void CoreBroker::processLocalCommandInstruction(ActionMessage& m)
{
    auto [processed, res] = processBaseCommands(m);
    if (processed) {
        return;
    }

    if (res[0] == "monitor") {
        switch (res.size()) {
            case 1:
                break;
            case 2:
                if (res[1] == "stop" || res[1] == "off") {
                    loadTimeMonitor(false, "");
                } else {
                    loadTimeMonitor(false, res[1]);
                }
                break;
            case 3:
                mTimeMonitorPeriod = loadTimeFromString(std::string(res[2]), time_units::sec);
                loadTimeMonitor(false, res[1]);
                break;
            case 4:
            default:
                mTimeMonitorPeriod =
                    loadTimeFromString(std::string(
                                           gmlc::utilities::string_viewOps::merge(res[2], res[3])),
                                       time_units::sec);
                loadTimeMonitor(false, res[1]);
                break;
        }
    } else {
        auto warnString = fmt::format(" unrecognized command instruction \"{}\"", res[0]);
        LOG_WARNING(global_broker_id_local, getIdentifier(), warnString);
        if (m.source_id != global_broker_id_local) {
            ActionMessage warn(CMD_WARNING, global_broker_id_local, m.source_id);
            warn.payload = std::move(warnString);
            warn.messageID = HELICS_LOG_LEVEL_WARNING;
            warn.setString(0, getIdentifier());
            routeMessage(warn);
        }
    }
}

void CoreBroker::processCommandInstruction(ActionMessage& m)
{
    if (m.dest_id == global_broker_id_local) {
        processLocalCommandInstruction(m);
    } else if (m.dest_id == parent_broker_id) {
        const auto& target = m.getString(targetStringLoc);
        if (target == "broker" || target == getIdentifier()) {
            processLocalCommandInstruction(m);
        } else if (isRootc) {
            if (target == "federation" || target == "root") {
                processLocalCommandInstruction(m);
            } else {
                route_id route = parent_route_id;
                auto fed = mFederates.find(target);
                if (fed != mFederates.end()) {
                    route = fed->route;
                    m.dest_id = fed->global_id;
                    transmit(route, std::move(m));
                } else {
                    auto broker = mBrokers.find(target);
                    if (broker != mBrokers.end()) {
                        route = broker->route;
                        m.dest_id = broker->global_id;
                        transmit(route, std::move(m));
                    } else {
                        m.swapSourceDest();
                        m.source_id = global_broker_id_local;
                        m.setAction(CMD_ERROR);
                        m.payload = "unable to locate target for command";
                        transmit(getRoute(m.dest_id), std::move(m));
                    }
                }
            }
        } else {
            route_id route = parent_route_id;
            auto fed = mFederates.find(target);
            if (fed != mFederates.end()) {
                route = fed->route;
                m.dest_id = fed->global_id;
                transmit(route, std::move(m));
            } else {
                auto broker = mBrokers.find(target);
                if (broker != mBrokers.end()) {
                    route = broker->route;
                    m.dest_id = broker->global_id;
                    transmit(route, std::move(m));
                } else {
                    transmit(parent_route_id, std::move(m));
                }
            }
        }
    } else {
        transmit(getRoute(m.dest_id), std::move(m));
    }
}

void CoreBroker::checkDependencies()
{
    if (isRootc) {
        for (const auto& newdep : delayedDependencies) {
            auto depfed = mFederates.find(newdep.first);
            if (depfed != mFederates.end()) {
                ActionMessage addDep(CMD_ADD_DEPENDENCY, newdep.second, depfed->global_id);
                routeMessage(addDep);
                addDep = ActionMessage(CMD_ADD_DEPENDENT, depfed->global_id, newdep.second);
                routeMessage(addDep);
            } else {
                ActionMessage logWarning(CMD_LOG, parent_broker_id, newdep.second);
                logWarning.messageID = WARNING;
                logWarning.payload =
                    "unable to locate " + newdep.first + " to establish dependency";
                logWarning.setString(0, getIdentifier());
                routeMessage(logWarning);
            }
        }

        if (timeCoord->getDependents().size() == 1) {  // if there is just one dependency remove it
            auto depid{timeCoord->getDependents()[0]};
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
        // if there is more than 2 dependents(higher broker + 2 or more other objects then we
        // need to be a timeCoordinator
        if (timeCoord->getDependents().size() > 2) {
            return;
        }

        GlobalFederateId fedid;
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
    for (const auto& brk : mBrokers) {
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
    for (const auto& fed : mFederates) {
        if (!fed.nonCounting) {
            ++cnt;
        }
    }
    return cnt;
}

bool CoreBroker::allInitReady() const
{
    // the federate count must be greater than the min size
    if (static_cast<decltype(minFederateCount)>(mFederates.size()) < minFederateCount) {
        return false;
    }
    if (static_cast<decltype(minBrokerCount)>(mBrokers.size()) < minBrokerCount) {
        return false;
    }
    if (minChildCount > 0) {
        decltype(minChildCount) children{0U};
        for (const auto& brk : mBrokers) {
            if (brk.parent == global_broker_id_local) {
                ++children;
            }
        }
        if (children < minChildCount) {
            return false;
        }
    }
    bool initReady = (getAllConnectionState() >= connection_state::init_requested);
    if (initReady) {
        // now do a more formal count of federates as there may be non-counting ones
        return (getCountableFederates() >= minFederateCount);
    }
    return false;
    // return std::all_of(mBrokers.begin(), mBrokers.end(), [](const auto& brk) {
    //   return ((brk._nonLocal) || (brk.state==connection_state::init_requested));
    //});
}

}  // namespace helics
