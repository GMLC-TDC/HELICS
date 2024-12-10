/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "CoreBroker.hpp"

#include "../common/JsonGeneration.hpp"
#include "../common/JsonProcessingFunctions.hpp"
#include "../common/logging.hpp"
#include "BaseTimeCoordinator.hpp"
#include "BrokerFactory.hpp"
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

#include <fmt/format.h>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace helics {

constexpr char universalKey[] = "**";

const std::string& stateString(ConnectionState state)
{
    static const std::string connected{"connected"};
    static const std::string init{"init_requested"};
    static const std::string operating{"operating"};
    static const std::string estate{"error"};
    static const std::string dis{"disconnected"};
    switch (state) {
        case ConnectionState::OPERATING:
            return operating;
        case ConnectionState::INIT_REQUESTED:
            return init;
        case ConnectionState::CONNECTED:
            return connected;
        case ConnectionState::REQUEST_DISCONNECT:
        case ConnectionState::DISCONNECTED:
            return dis;
        case ConnectionState::ERROR_STATE:
        default:
            return estate;
    }
}

CoreBroker::~CoreBroker()
{
    const std::lock_guard<std::mutex> lock(name_mutex_);
    // make sure everything is synchronized
}

void CoreBroker::setIdentifier(std::string_view name)
{
    if (getBrokerState() <= BrokerState::CONNECTING)  // can't be changed after initialization
    {
        const std::lock_guard<std::mutex> lock(name_mutex_);
        identifier.assign(name);
    }
}

const std::string& CoreBroker::getAddress() const
{
    if ((getBrokerState() != BrokerState::CONNECTED) || (address.empty())) {
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
    std::function<void(int, std::string_view, std::string_view)> logFunction)
{
    ActionMessage loggerUpdate(CMD_BROKER_CONFIGURE);
    loggerUpdate.messageID = UPDATE_LOGGING_CALLBACK;
    loggerUpdate.source_id = global_id.load();
    if (logFunction) {
        auto index = getNextAirlockIndex();
        dataAirlocks[index].load(std::move(logFunction));
        loggerUpdate.counter = index;
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
bool CoreBroker::verifyBrokerKey(std::string_view key) const
{
    return (key == brokerKey || brokerKey == universalKey);
}

void CoreBroker::makeConnections(const std::string& file)
{
    auto type = fileops::getConfigType(file);

    switch (type) {
        case fileops::ConfigType::JSON_FILE:
        case fileops::ConfigType::JSON_STRING:
            fileops::makeConnectionsJson(this, file);
            break;
        case fileops::ConfigType::TOML_FILE:
        case fileops::ConfigType::TOML_STRING:
            fileops::makeConnectionsToml(this, file);
            break;
        case fileops::ConfigType::CMD_LINE:
        case fileops::ConfigType::NONE:
            // with NONE there are default command line and environment possibilities
            break;
    }
}

void CoreBroker::linkEndpoints(std::string_view source, std::string_view target)
{
    ActionMessage link(CMD_ENDPOINT_LINK);
    link.name(source);
    link.setStringData(target);
    addActionMessage(std::move(link));
}

void CoreBroker::dataLink(std::string_view publication, std::string_view input)
{
    ActionMessage link(CMD_DATA_LINK);
    link.name(publication);
    link.setStringData(input);
    addActionMessage(std::move(link));
}

void CoreBroker::addSourceFilterToEndpoint(std::string_view filter, std::string_view endpoint)
{
    ActionMessage link(CMD_FILTER_LINK);
    link.name(filter);
    link.setStringData(endpoint);
    addActionMessage(std::move(link));
}

void CoreBroker::addDestinationFilterToEndpoint(std::string_view filter, std::string_view endpoint)
{
    ActionMessage link(CMD_FILTER_LINK);
    link.name(filter);
    link.setStringData(endpoint);
    setActionFlag(link, destination_target);
    addActionMessage(std::move(link));
}

void CoreBroker::addAlias(std::string_view interfaceKey, std::string_view alias)
{
    ActionMessage link(CMD_ADD_ALIAS);
    link.name(interfaceKey);
    link.setStringData(alias);
    addActionMessage(std::move(link));
}

route_id CoreBroker::fillMessageRouteInformation(ActionMessage& mess)
{
    const auto& endpointName = mess.getString(targetStringLoc);
    auto* eptInfo = handles.getInterfaceHandle(endpointName, InterfaceType::ENDPOINT);
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
    if (cstate > BrokerState::OPERATING) {
        return false;
    }
    if ((maxFederateCount == (std::numeric_limits<int32_t>::max)() ||
         getCountableFederates() < maxFederateCount) &&
        !haltOperations) {
        if (!dynamicFederation) {
            return (cstate < BrokerState::OPERATING);
        }
        return true;
    }
    return false;
}

void CoreBroker::sendBrokerErrorAck(ActionMessage& command, std::int32_t errorCode)
{
    route_id newroute;
    bool route_created = false;
    const bool jsonReply = checkActionFlag(command, use_json_serialization_flag);
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
    badInit.messageID = errorCode;
    switch (errorCode) {
        case broker_terminating:
            badInit.setString(0, "broker is terminating");
            break;
        case mismatch_broker_key_error_code:
            badInit.setString(0, "broker key does not match");
            break;
        default:
            break;
    }
    transmit(newroute, badInit);

    if (route_created) {
        removeRoute(newroute);
    }
}

void CoreBroker::brokerRegistration(ActionMessage&& command)
{
    if (!connectionEstablished) {
        earlyMessages.push_back(std::move(command));
        return;
    }
    const bool jsonReply = checkActionFlag(command, use_json_serialization_flag);
    if (command.counter > 0) {  // this indicates it is a resend
        auto brk = mBrokers.find(command.name());
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
            if (globalTime || asyncTime) {
                setActionFlag(brokerReply, global_timing_flag);
                if (asyncTime) {
                    setActionFlag(brokerReply, async_timing_flag);
                }
            }
            transmit(brk->route, brokerReply);
            return;
        }
    }
    // check the max broker count
    if (static_cast<decltype(maxBrokerCount)>(mBrokers.size()) >= maxBrokerCount) {
        sendBrokerErrorAck(command, max_broker_count_exceeded);
        return;
    }

    auto currentBrokerState = getBrokerState();
    if (currentBrokerState < BrokerState::OPERATING) {
        if (allInitReady()) {
            // send an init not ready as we were ready now we are not
            ActionMessage noInit(CMD_INIT_NOT_READY);
            noInit.source_id = global_broker_id_local;
            transmit(parent_route_id, noInit);
        }
    } else if (currentBrokerState == BrokerState::OPERATING) {
        // we are initialized already
        if (!checkActionFlag(command, observer_flag) && !dynamicFederation) {
            sendBrokerErrorAck(command, already_init_error_code);
            return;
        }
        // can't add a non observer federate in OPERATING mode unless this is a dynamicFederation
    } else {
        sendBrokerErrorAck(command, broker_terminating);
        return;
    }
    if (!verifyBrokerKey(command)) {
        sendBrokerErrorAck(command, mismatch_broker_key_error_code);
        return;
    }
    if (checkActionFlag(command, test_connection_flag)) {
        route_id newroute;
        bool route_created = false;
        if ((!command.source_id.isValid()) || (command.source_id == parent_broker_id)) {
            newroute = generateRouteId(jsonReply ? json_route_code : 0, routeCount++);
            addRoute(newroute, command.getExtraData(), command.getString(targetStringLoc));
            route_created = true;
        } else {
            newroute = getRoute(command.source_id);
        }
        ActionMessage testInit(CMD_BROKER_ACK);
        setActionFlag(testInit, error_flag);
        setActionFlag(testInit, test_connection_flag);
        testInit.source_id = global_broker_id_local;
        testInit.name(command.name());
        testInit.messageID = CONNECTION_TEST;
        transmit(newroute, testInit);

        if (route_created) {
            removeRoute(newroute);
        }
        // this was a test connection and should not be added
        return;
    }
    auto inserted = mBrokers.insert(command.name(), no_search, command.name());
    if (!inserted) {
        sendBrokerErrorAck(command, duplicate_broker_name_error_code);
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

        // sending the response message
        ActionMessage brokerReply(CMD_BROKER_ACK);
        brokerReply.source_id = global_broker_id_local;  // source is global root
        brokerReply.dest_id = global_brkid;  // the new id
        brokerReply.name(command.name());  // the identifier of the broker
        if (no_ping) {
            setActionFlag(brokerReply, slow_responding_flag);
        }
        if (globalTime || asyncTime) {
            setActionFlag(brokerReply, global_timing_flag);
            if (asyncTime) {
                setActionFlag(brokerReply, async_timing_flag);
            }
        }
        if (globalDisconnect) {
            setActionFlag(brokerReply, global_disconnect_flag);
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

void CoreBroker::sendFedErrorAck(ActionMessage& command, std::int32_t errorCode)
{
    ActionMessage badInit(CMD_FED_ACK);
    setActionFlag(badInit, error_flag);
    badInit.source_id = global_broker_id_local;
    badInit.messageID = errorCode;
    badInit.name(command.name());
    transmit(getRoute(command.source_id), badInit);
}

std::string CoreBroker::generateRename(std::string_view name)
{
    std::string newName{name};
    auto cntLoc = newName.find("${#}");
    if (cntLoc != std::string::npos) {
        auto renamer = renamers.find(newName);
        if (renamer != renamers.end()) {
            newName.replace(cntLoc, 4, std::to_string(renamer->second + 1));
            renamer->second++;
        } else {
            newName.replace(cntLoc, 4, "1");
            renamers.emplace(name, 1);
        }
    }
    return newName;
}

// Handle the registration of new federates;
void CoreBroker::fedRegistration(ActionMessage&& command)
{
    if (!connectionEstablished) {
        earlyMessages.push_back(std::move(command));
        return;
    }
    const bool countable = !checkActionFlag(command, non_counting_flag);
    bool dynamicFed{false};
    if (countable && getCountableFederates() >= maxFederateCount) {
        sendFedErrorAck(command, max_federate_count_exceeded);
        return;
    }
    if (getBrokerState() < BrokerState::OPERATING) {
        if (countable && allInitReady()) {
            ActionMessage noInit(CMD_INIT_NOT_READY);
            noInit.source_id = global_broker_id_local;
            transmit(parent_route_id, noInit);
        }
    } else if (getBrokerState() == BrokerState::OPERATING) {
        const bool allowed =
            dynamicFederation || !countable || checkActionFlag(command, observer_flag);
        if (!allowed) {
            // we are initialized already
            sendFedErrorAck(command, already_init_error_code);
            return;
        }
        dynamicFed = true;
    } else {
        // we are in an ERROR_STATE and terminating
        sendFedErrorAck(command, broker_terminating);
        return;
    }
    auto fedName = command.name();
    bool newFed{true};
    if (dynamicFed && checkActionFlag(command, reentrant_flag)) {
        auto fedLoc = mFederates.find(fedName);
        if (fedLoc != mFederates.end()) {
            if (fedLoc->reentrant) {
                fedLoc->route = getRoute(command.source_id);
                fedLoc->parent = command.source_id;
                fedLoc->state = ConnectionState::CONNECTED;
                newFed = false;
            } else {
                sendFedErrorAck(command, duplicate_federate_name_error_code);
                return;
            }
        }
    }

    if (newFed) {
        // this checks for duplicate federate names
        if (mFederates.find(fedName) != mFederates.end()) {
            sendFedErrorAck(command, duplicate_federate_name_error_code);
            return;
        }
        mFederates.insert(fedName, no_search, fedName);
        mFederates.back().route = getRoute(command.source_id);
        mFederates.back().parent = command.source_id;
        if (checkActionFlag(command, non_counting_flag)) {
            mFederates.back().nonCounting = true;
        }
        if (checkActionFlag(command, observer_flag)) {
            mFederates.back().observer = true;
        }
        if (checkActionFlag(command, reentrant_flag)) {
            mFederates.back().reentrant = true;
        }
        mFederates.back().dynamic = dynamicFed;
        auto lookupIndex = mFederates.size() - 1;
        if (checkActionFlag(command, child_flag)) {
            mFederates.back().global_id = GlobalFederateId(command.getExtraData());
            if (!mFederates.addSearchTermForIndex(mFederates.back().global_id, lookupIndex)) {
                sendFedErrorAck(command, duplicate_federate_id);
                return;
            }
        } else if (isRootc) {
            if (command.counter > 0 && command.counter <= 16) {
                mFederates.back().global_id = GlobalFederateId(
                    static_cast<GlobalFederateId::BaseType>(lookupIndex) + gGlobalFederateIdShift +
                    command.counter * gGlobalPriorityBlockSize);
            } else {
                mFederates.back().global_id = GlobalFederateId(
                    static_cast<GlobalFederateId::BaseType>(lookupIndex) + gGlobalFederateIdShift);
            }
            mFederates.addSearchTermForIndex(mFederates.back().global_id, lookupIndex);
        }
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
        auto route_id = (newFed) ? mFederates.back().route : mFederates.find(fedName)->route;
        auto global_fedid =
            (newFed) ? mFederates.back().global_id : mFederates.find(fedName)->global_id;
        auto res = routing_table.emplace(global_fedid, route_id);
        if (!res.second && !newFed) {
            res.first->second = route_id;
        }

        // don't bother with the federate_table
        // transmit the response
        ActionMessage fedReply(CMD_FED_ACK);
        fedReply.source_id = global_broker_id_local;
        fedReply.dest_id = global_fedid;
        fedReply.name(fedName);
        if (checkActionFlag(command, child_flag)) {
            setActionFlag(fedReply, child_flag);
        }
        if (globalTime || asyncTime) {
            setActionFlag(fedReply, global_timing_flag);
            if (asyncTime) {
                setActionFlag(fedReply, async_timing_flag);
            }
            if (!checkActionFlag(command, non_counting_flag)) {
                timeCoord->addDependency(global_fedid);
                timeCoord->addDependent(global_fedid);
                timeCoord->setAsChild(global_fedid);
            }
        }
        if (globalDisconnect) {
            setActionFlag(fedReply, global_disconnect_flag);
        }
        transmit(route_id, fedReply);
        LOG_CONNECTIONS(global_broker_id_local,
                        getIdentifier(),
                        fmt::format("registering federate {}({}) on route {}",
                                    fedName,
                                    global_fedid.baseValue(),
                                    route_id.baseValue()));
        if (enable_profiling) {
            ActionMessage fedEnableProfiling(CMD_SET_PROFILER_FLAG,
                                             global_broker_id_local,
                                             global_fedid);
            setActionFlag(fedEnableProfiling, indicator_flag);
            transmit(route_id, fedEnableProfiling);
        }
        if (mNextTimeBarrier < Time::maxVal()) {
            ActionMessage timeBarrier(CMD_TIME_BARRIER, global_broker_id_local, global_fedid);
            timeBarrier.actionTime = mNextTimeBarrier;
            timeBarrier.messageID = global_broker_id_local.baseValue();
            transmit(route_id, timeBarrier);
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
            timeCoord->setSourceId(global_broker_id_local);
            connectionEstablished = true;
            if (!earlyMessages.empty()) {
                for (auto& message : earlyMessages) {
                    if (isPriorityCommand(message)) {
                        processPriorityCommand(std::move(message));
                    } else {
                        processCommand(std::move(message));
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
            auto fed = mFederates.find(command.name());
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
                mFederates.insert(command.name(), command.dest_id, command.name());
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
                if (checkActionFlag(command, global_timing_flag)) {
                    globalTime = true;
                    if (checkActionFlag(command, async_timing_flag)) {
                        asyncTime = true;
                    }
                }
                timeCoord->setSourceId(global_broker_id_local);
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
                if (checkActionFlag(command, global_disconnect_flag)) {
                    globalDisconnect = true;
                }
                return;
            }
            auto broker = mBrokers.find(command.name());
            if (broker != mBrokers.end()) {
                if (broker->global_id == GlobalBrokerId{command.dest_id}) {
                    // drop the packet since we have seen this ack already
                    LOG_WARNING(global_broker_id_local, identifier, "repeated broker acks");
                    return;
                }
                broker->global_id = GlobalBrokerId{command.dest_id};
                auto route = broker->route;
                mBrokers.addSearchTerm(GlobalBrokerId{command.dest_id}, broker->name);
                routing_table.emplace(broker->global_id, route);
                command.source_id = global_broker_id_local;  // we want the intermediate broker to
                                                             // change the source_id
                transmit(route, command);
            } else {
                mBrokers.insert(command.name(), GlobalBrokerId{command.dest_id}, command.name());
                mBrokers.back().route = getRoute(command.source_id);
                mBrokers.back().global_id = GlobalBrokerId{command.dest_id};
                routing_table.emplace(broker->global_id, mBrokers.back().route);
            }
        } break;
        case CMD_PRIORITY_DISCONNECT: {
            auto* brk = getBrokerById(GlobalBrokerId{command.source_id});
            if (brk != nullptr) {
                brk->state = ConnectionState::DISCONNECTED;
            }
            if (getAllConnectionState() >= ConnectionState::DISCONNECTED) {
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
    int translators = 0;
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
            case InterfaceType::TRANSLATOR:
                ++translators;
                break;
            default:
                ++filt;
                break;
        }
    }
    nlohmann::json summary;
    nlohmann::json block;
    block["federates"] = static_cast<int>(mFederates.size());
    block["allowed_federates"][0] = minFederateCount;
    block["allowed_federates"][1] = maxFederateCount;
    block["countable_federates"] = getCountableFederates();
    block["brokers"] =
        static_cast<int>(std::count_if(mBrokers.begin(), mBrokers.end(), [](auto& brk) {
            return !static_cast<bool>(brk._core);
        }));
    block["cores"] =
        static_cast<int>(std::count_if(mBrokers.begin(), mBrokers.end(), [](auto& brk) {
            return static_cast<bool>(brk._core);
        }));
    block["allowed_brokers"][0] = minBrokerCount;
    block["allowed_brokers"][1] = maxBrokerCount;
    block["publications"] = pubs;
    block["inputs"] = ipts;
    block["filters"] = filt;
    block["endpoints"] = epts;
    block["translators"] = translators;
    summary["summary"] = block;
    addBaseInformation(summary, isRootc);
    return fileops::generateJsonString(summary);
}

void CoreBroker::generateTimeBarrier(ActionMessage& message)
{
    if (checkActionFlag(message, cancel_flag)) {
        ActionMessage cancelBarrier(CMD_TIME_BARRIER_CLEAR);
        cancelBarrier.source_id = global_broker_id_local;
        if (cancelBarrier.messageID == 0) {
            cancelBarrier.messageID = global_broker_id_local.baseValue();
        }
        mNextTimeBarrier = Time::maxVal();
        broadcast(cancelBarrier);
        return;
    }
    message.setAction(CMD_TIME_BARRIER);
    message.source_id = global_broker_id_local;
    if (message.messageID == 0) {
        message.messageID = global_broker_id_local.baseValue();
    }
    mNextTimeBarrier = message.actionTime;
    // time should already be set
    broadcast(message);
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
            obj.state = ConnectionState::DISCONNECTED;
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
    if (cState == BrokerState::OPERATING || firstLoad) {
        if (cState == BrokerState::OPERATING && !firstLoad) {
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
    } else if (cState < BrokerState::OPERATING) {
        if (!newFederate.empty()) {
            mTimeMonitorFederate = newFederate;
        }
    }
}

void CoreBroker::processTimeMonitorMessage(ActionMessage& message)
{
    if (message.source_id != mTimeMonitorFederateId) {
        return;
    }
    switch (message.action()) {
        case CMD_EXEC_GRANT:
            mTimeMonitorLastLogTime = timeZero;
            mTimeMonitorCurrentTime = timeZero;

            simTime.store(static_cast<double>(mTimeMonitorCurrentTime));
            LOG_SUMMARY(message.source_id, mTimeMonitorFederate, "TIME: exec granted");
            break;
        case CMD_TIME_GRANT:
            mTimeMonitorCurrentTime = message.actionTime;
            simTime.store(static_cast<double>(mTimeMonitorCurrentTime));
            if (mTimeMonitorCurrentTime - mTimeMonitorPeriod >= mTimeMonitorLastLogTime) {
                LOG_SUMMARY(message.source_id,
                            mTimeMonitorFederate,
                            fmt::format("TIME: granted time={}",
                                        static_cast<double>(mTimeMonitorCurrentTime)));
                mTimeMonitorLastLogTime = mTimeMonitorCurrentTime;
            }
            break;
        case CMD_DISCONNECT:
            LOG_SUMMARY(message.source_id,
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
        if (brk.state < ConnectionState::DISCONNECTED) {
            if (brk.parent == global_broker_id_local) {
                this->routeMessage(bye, brk.global_id);
                brk.state = ConnectionState::DISCONNECTED;
                brk._sent_disconnect_ack = true;
            }
            if (hasTimeDependency) {
                timeCoord->removeDependency(brk.global_id);
                timeCoord->removeDependent(brk.global_id);
            }
        } else if (brk.state == ConnectionState::DISCONNECTED) {
            if (brk._sent_disconnect_ack == false) {
                ActionMessage dis((brk._core) ? CMD_DISCONNECT_CORE_ACK :
                                                CMD_DISCONNECT_BROKER_ACK);
                dis.source_id = global_broker_id_local;
                dis.dest_id = brk.global_id;
                transmit(brk.route, dis);
                brk._sent_disconnect_ack = true;
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

// NOLINTNEXTLINE
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
            if (getBrokerState() == BrokerState::OPERATING) {
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
                    if (brk.state == ConnectionState::DISCONNECTED) {
                        partDisconnected = true;
                    }
                    if (brk.state == ConnectionState::DISCONNECTED &&
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
                    processDisconnectCommand(command);
                } else {
                    if (isRootc) {
                        const std::string lcom =
                            fmt::format("lost comms with {}", command.source_id.baseValue());
                        LOG_ERROR(global_broker_id_local, getIdentifier(), lcom);
                        ActionMessage elink(CMD_ERROR);
                        elink.payload = lcom;
                        elink.messageID = defs::Errors::CONNECTION_FAILURE;
                        broadcast(elink);
                        setBrokerState(BrokerState::ERRORED);
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
            auto fed = mFederates.find(command.name());
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
        case CMD_ADD_ALIAS:
            linkInterfaces(command);
            break;
        case CMD_DISCONNECT_NAME:
            if (command.dest_id == parent_broker_id) {
                auto brk = mBrokers.find(command.name());
                if (brk != mBrokers.end()) {
                    command.source_id = brk->global_id;
                }
            }
            [[fallthrough]];
        case CMD_DISCONNECT:
        case CMD_DISCONNECT_CORE:
        case CMD_DISCONNECT_BROKER:
            processDisconnectCommand(command);
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
                        nlohmann::json base;
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
                        BrokerState::TERMINATING) {  // only send a disconnect message
                                                     // if we haven't done so already
                        sendDisconnect(CMD_TIMEOUT_DISCONNECT);
                    }
                } else if (getBrokerState() ==
                           BrokerState::ERRORED) {  // we are disconnecting in an error state
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
                fed->state = ConnectionState::DISCONNECTED;
                if (fed->reentrant) {
                    // for reentrant federates the interface can be recreated later so needs to be
                    // removed
                    handles.removeFederateHandles(command.source_id);
                }
            }
            if (!isRootc) {
                transmit(parent_route_id, command);
            } else if (getBrokerState() < BrokerState::OPERATING) {
                command.setAction(CMD_BROADCAST_DISCONNECT);
                broadcast(command);
                unknownHandles.clearFederateUnknowns(command.source_id);
            }
        } break;
        case CMD_STOP:
            if ((getAllConnectionState() <
                 ConnectionState::DISCONNECTED)) {  // only send a disconnect message if we
                                                    // haven't done so already
                timeCoord->disconnect();
                if (!isRootc) {
                    ActionMessage disconnect(CMD_DISCONNECT);
                    disconnect.source_id = global_broker_id_local;
                    transmit(parent_route_id, disconnect);
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
                if (timeCoord->processTimeMessage(command) != TimeProcessingResult::NOT_PROCESSED) {
                    if (!enteredExecutionMode) {
                        if (getBrokerState() >= BrokerState::OPERATING) {
                            auto res = timeCoord->checkExecEntry(command.source_id);
                            if (res == MessageProcessingResult::NEXT_STEP) {
                                enteredExecutionMode = true;
                                LOG_TIMING(global_broker_id_local,
                                           getIdentifier(),
                                           "entering Exec Mode");
                            } else {
                                timeCoord->updateTimeFactors();
                            }
                        }
                    }
                }

            } else if (command.source_id == global_broker_id_local) {
                if (command.dest_id.isValid()) {
                    transmit(getRoute(command.dest_id), command);
                } else {
                    for (auto& dep : timeCoord->getDependents()) {
                        routeMessage(command, dep);
                    }
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
                if (timeCoord->processTimeMessage(command) != TimeProcessingResult::NOT_PROCESSED) {
                    if (enteredExecutionMode) {
                        timeCoord->updateTimeFactors();
                    } else {
                        if (getBrokerState() >= BrokerState::OPERATING) {
                            auto res = timeCoord->checkExecEntry(command.source_id);
                            if (res == MessageProcessingResult::NEXT_STEP) {
                                enteredExecutionMode = true;
                                LOG_TIMING(global_broker_id_local,
                                           getIdentifier(),
                                           "entering Exec Mode");
                            }
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
                    const bool optional_flag_set = checkActionFlag(command, optional_flag);
                    const bool required_flag_set = checkActionFlag(command, required_flag);

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
                auto json = timeCoord->grantTimeoutCheck(command);
                if (!json.is_null()) {
                    auto debugString = fileops::generateJsonString(json);
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
        case CMD_REG_TRANSLATOR:
            if ((!isRootc) && (command.dest_id != parent_broker_id)) {
                routeMessage(command);
                // break;
            }
            addTranslator(command);
            break;
        case CMD_REG_DATASINK:
            if ((!isRootc) && (command.dest_id != parent_broker_id)) {
                routeMessage(command);
                // break;
            }
            addDataSink(command);
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
        case CMD_TIMING_INFO:
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
            if (brk->state == ConnectionState::CONNECTED) {
                brk->state = ConnectionState::INIT_REQUESTED;
            }

            if ((dynamicFederation || brk->_observer) &&
                getBrokerState() >= BrokerState::OPERATING) {
                if (isRootc) {
                    ActionMessage grant(CMD_INIT_GRANT, global_broker_id_local, cmd.source_id);
                    if (checkActionFlag(cmd, iteration_requested_flag)) {
                        setActionFlag(grant, iteration_requested_flag);
                    }
                    if (brk->_observer) {
                        setActionFlag(grant, observer_flag);
                    } else {
                        setActionFlag(grant, dynamic_join_flag);
                    }
                    transmit(brk->route, grant);
                } else {
                    transmit(parent_route_id, cmd);
                }
            } else {
                if (checkActionFlag(cmd, iteration_requested_flag)) {
                    brk->initIterating = true;
                    initIterating = true;
                }
                if (allInitReady()) {
                    if (isRootc) {
                        if (initIterating) {
                            executeInitializationOperations(true);
                        } else {
                            LOG_TIMING(global_broker_id_local,
                                       "root",
                                       "entering initialization mode");
                            LOG_SUMMARY(global_broker_id_local,
                                        "root",
                                        generateFederationSummary());
                            executeInitializationOperations(false);
                        }

                    } else {
                        LOG_TIMING(global_broker_id_local,
                                   getIdentifier(),
                                   "entering initialization mode");
                        checkDependencies();
                        cmd.source_id = global_broker_id_local;
                        if (initIterating) {
                            setActionFlag(cmd, iteration_requested_flag);
                        }
                        transmit(parent_route_id, cmd);
                    }
                }
            }

        } break;
        case CMD_INIT_NOT_READY: {
            if (allInitReady()) {
                if (isRootc) {
                    LOG_WARNING(global_broker_id_local,
                                getIdentifier(),
                                "received init not ready but already init");
                    break;
                }
                transmit(parent_route_id, cmd);
            }
            auto* brk = getBrokerById(GlobalBrokerId{cmd.source_id});
            if (brk != nullptr) {
                brk->state = ConnectionState::CONNECTED;
                brk->initIterating = false;
            }
        } break;
        case CMD_INIT_GRANT:
            if (!(checkActionFlag(cmd, observer_flag) || checkActionFlag(cmd, dynamic_join_flag))) {
                if (checkActionFlag(cmd, iteration_requested_flag)) {
                    executeInitializationOperations(true);
                } else {
                    if (brokerKey == universalKey) {
                        LOG_SUMMARY(global_broker_id_local,
                                    getIdentifier(),
                                    "Broker started with universal key");
                    }
                    setBrokerState(BrokerState::OPERATING);
                    broadcast(cmd);
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
                BrokerState exp = CONNECTED;
                if (brokerState.compare_exchange_strong(exp, BrokerState::INITIALIZING))
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
                auto locker = dataAirlocks[cmd.counter].try_unload();
                if (locker) {
                    try {
                        auto callback = std::any_cast<
                            std::function<void(int, std::string_view, std::string_view)>>(
                            std::move(*locker));
                        callback(0, identifier, "logging callback activated");
                        setLoggerFunction(std::move(callback));
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
    if (checkActionFlag(command, reconnectable_flag)) {
        if (isRootc) {
            switch (command.action()) {
                case CMD_ADD_NAMED_PUBLICATION:
                    unknownHandles.addReconnectablePublication(command.name(),
                                                               command.getSource(),
                                                               command.flags);
                    break;
                case CMD_ADD_NAMED_INPUT:
                    unknownHandles.addReconnectableInput(command.name(),
                                                         command.getSource(),
                                                         command.flags);

                    break;
                case CMD_ADD_NAMED_ENDPOINT:
                    unknownHandles.addReconnectableEndpoint(command.name(),
                                                            command.getSource(),
                                                            command.flags);

                    break;
                case CMD_ADD_NAMED_FILTER:
                    unknownHandles.addReconnectableFilter(command.name(),
                                                          command.getSource(),
                                                          command.flags);
                    break;
                default:
                    break;
            }
        }
    }
    switch (command.action()) {
        case CMD_ADD_NAMED_PUBLICATION: {
            auto* pub = handles.getInterfaceHandle(command.name(), InterfaceType::PUBLICATION);
            if (pub != nullptr) {
                auto fed = mFederates.find(pub->getFederateId());
                if (fed->state < ConnectionState::ERROR_STATE) {
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
            auto* inp = handles.getInterfaceHandle(command.name(), InterfaceType::INPUT);
            if (inp != nullptr) {
                auto fed = mFederates.find(inp->getFederateId());
                if (fed->state < ConnectionState::ERROR_STATE) {
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
            auto* filt = handles.getInterfaceHandle(command.name(), InterfaceType::FILTER);
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
            auto* ept = handles.getInterfaceHandle(command.name(), InterfaceType::ENDPOINT);
            if (ept != nullptr) {
                auto fed = mFederates.find(ept->getFederateId());
                if (fed->state < ConnectionState::ERROR_STATE) {
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
                    unknownHandles.addUnknownPublication(command.name(),
                                                         command.getSource(),
                                                         command.flags);
                    break;
                case CMD_ADD_NAMED_INPUT:
                    unknownHandles.addUnknownInput(command.name(),
                                                   command.getSource(),
                                                   command.flags);
                    if (!command.getStringData().empty()) {
                        auto* pub = handles.findHandle(command.getSource());
                        if (pub == nullptr) {
                            // an anonymous publisher is adding an input
                            auto& apub = handles.addHandle(command.source_id,
                                                           command.source_handle,
                                                           InterfaceType::PUBLICATION,
                                                           std::string_view(),
                                                           command.getString(typeStringLoc),
                                                           command.getString(unitStringLoc));

                            addLocalInfo(apub, command);
                        }
                    }
                    break;
                case CMD_ADD_NAMED_ENDPOINT:
                    unknownHandles.addUnknownEndpoint(command.name(),
                                                      command.getSource(),
                                                      command.flags);
                    if (!command.getStringData().empty()) {
                        auto* filt = handles.findHandle(command.getSource());
                        if (filt == nullptr) {
                            // an anonymous filter is adding an endpoint
                            auto& afilt = handles.addHandle(command.source_id,
                                                            command.source_handle,
                                                            InterfaceType::FILTER,
                                                            "",
                                                            command.getString(typeStringLoc),
                                                            command.getString(typeOutStringLoc));

                            addLocalInfo(afilt, command);
                        }
                    }
                    break;
                case CMD_ADD_NAMED_FILTER:
                    unknownHandles.addUnknownFilter(command.name(),
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
            auto* pub = handles.getInterfaceHandle(command.name(), InterfaceType::PUBLICATION);
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
            auto* inp = handles.getInterfaceHandle(command.name(), InterfaceType::INPUT);
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
            auto* filt = handles.getInterfaceHandle(command.name(), InterfaceType::FILTER);
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
            auto* ept = handles.getInterfaceHandle(command.name(), InterfaceType::ENDPOINT);
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

void CoreBroker::addLocalInfo(BasicHandleInfo& handleInfo, const ActionMessage& message)
{
    auto res = global_id_translation.find(message.source_id);
    if (res != global_id_translation.end()) {
        handleInfo.local_fed_id = res->second;
    }
    handleInfo.flags = message.flags;
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
            if (!isRoot()) {
                transmitToParent(std::move(cmd));
            } else {
            }
            return;
        }
    }
    routeMessage(std::move(cmd));
}

bool CoreBroker::checkInterfaceCreation(ActionMessage& message, InterfaceType type)
{
    bool existingName{false};
    if (type == InterfaceType::TRANSLATOR) {
        existingName =
            (handles.getInterfaceHandle(message.name(), InterfaceType::ENDPOINT) != nullptr ||
             handles.getInterfaceHandle(message.name(), InterfaceType::INPUT) != nullptr ||
             handles.getInterfaceHandle(message.name(), InterfaceType::PUBLICATION) != nullptr);
    } else {
        existingName = (handles.getInterfaceHandle(message.name(), type) != nullptr);
    }

    // detect duplicate InterfaceName;
    if (existingName) {
        ActionMessage eret(CMD_LOCAL_ERROR, global_broker_id_local, message.source_id);
        eret.dest_handle = message.source_handle;
        eret.messageID = defs::Errors::REGISTRATION_FAILURE;
        eret.payload =
            fmt::format("Duplicate {} names ({})", interfaceTypeName(type), message.name());
        propagateError(std::move(eret));
        return false;
    }
    if (disableDynamicSources && type != InterfaceType::INPUT) {
        if (getBrokerState() == BrokerState::OPERATING) {
            auto fed = mFederates.find(message.source_id);
            if (fed == mFederates.end()) {
                ActionMessage eret(CMD_LOCAL_ERROR, global_broker_id_local, message.source_id);
                eret.dest_handle = message.source_handle;
                eret.messageID = defs::Errors::REGISTRATION_FAILURE;
                eret.payload =
                    fmt::format("Source {} not allowed after entering initializing mode ({})",
                                interfaceTypeName(type),
                                message.name());
                propagateError(std::move(eret));
                return false;
            }
            if (fed->observer || !fed->dynamic || fed->state != ConnectionState::CONNECTED) {
                ActionMessage eret(CMD_LOCAL_ERROR, global_broker_id_local, message.source_id);
                eret.dest_handle = message.source_handle;
                eret.messageID = defs::Errors::REGISTRATION_FAILURE;
                eret.payload = fmt::format(
                    "Source {} from {} not allowed after entering initializing mode ({})",
                    interfaceTypeName(type),
                    fed->name,
                    message.name());
                propagateError(std::move(eret));
                return false;
            }
        }
    }
    return true;
}

void CoreBroker::addPublication(ActionMessage& message)
{
    if (!checkInterfaceCreation(message, InterfaceType::PUBLICATION)) {
        return;
    }
    auto& pub = handles.addHandle(message.source_id,
                                  message.source_handle,
                                  InterfaceType::PUBLICATION,
                                  message.name(),
                                  message.getString(typeStringLoc),
                                  message.getString(unitStringLoc));

    addLocalInfo(pub, message);
    if (!isRootc) {
        transmit(parent_route_id, message);
    } else {
        findAndNotifyPublicationTargets(pub, pub.key);
    }
}
void CoreBroker::addInput(ActionMessage& message)
{
    if (!checkInterfaceCreation(message, InterfaceType::INPUT)) {
        return;
    }
    auto& inp = handles.addHandle(message.source_id,
                                  message.source_handle,
                                  InterfaceType::INPUT,
                                  message.name(),
                                  message.getString(typeStringLoc),
                                  message.getString(unitStringLoc));

    addLocalInfo(inp, message);
    if (!isRootc) {
        transmit(parent_route_id, message);
    } else {
        findAndNotifyInputTargets(inp, inp.key);
    }
}

void CoreBroker::addEndpoint(ActionMessage& message)
{
    if (!checkInterfaceCreation(message, InterfaceType::ENDPOINT)) {
        return;
    }
    auto& ept = handles.addHandle(message.source_id,
                                  message.source_handle,
                                  InterfaceType::ENDPOINT,
                                  message.name(),
                                  message.getString(typeStringLoc),
                                  message.getString(unitStringLoc));

    addLocalInfo(ept, message);

    if (!isRootc) {
        transmit(parent_route_id, message);
        if (!hasTimeDependency && !globalTime && !asyncTime) {
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
        findAndNotifyEndpointTargets(ept, ept.key);
    }
}
void CoreBroker::addFilter(ActionMessage& message)
{
    if (!checkInterfaceCreation(message, InterfaceType::FILTER)) {
        return;
    }
    auto& filt = handles.addHandle(message.source_id,
                                   message.source_handle,
                                   InterfaceType::FILTER,
                                   message.name(),
                                   message.getString(typeStringLoc),
                                   message.getString(typeOutStringLoc));
    addLocalInfo(filt, message);

    if (!isRootc) {
        transmit(parent_route_id, message);
    } else {
        findAndNotifyFilterTargets(filt, filt.key);
    }
}

void CoreBroker::addTranslator(ActionMessage& message)
{
    if (!checkInterfaceCreation(message, InterfaceType::TRANSLATOR)) {
        return;
    }
    auto& trans = handles.addHandle(message.source_id,
                                    message.source_handle,
                                    InterfaceType::TRANSLATOR,
                                    message.name(),
                                    message.getString(typeStringLoc),
                                    message.getString(typeOutStringLoc));
    addLocalInfo(trans, message);

    if (!isRootc) {
        transmit(parent_route_id, message);
        if (!hasFilters) {
            hasFilters = true;
            if (!globalTime && !asyncTime) {
                if (timeCoord->addDependent(higher_broker_id)) {
                    hasTimeDependency = true;
                    ActionMessage add(CMD_ADD_DEPENDENCY, global_broker_id_local, higher_broker_id);
                    setActionFlag(add, child_flag);
                    transmit(parent_route_id, add);
                }
            }
        }
    } else {
        findAndNotifyInputTargets(trans, trans.key);
        findAndNotifyPublicationTargets(trans, trans.key);
        findAndNotifyEndpointTargets(trans, trans.key);
    }
}

void CoreBroker::addDataSink(ActionMessage& message)
{
    if (!checkInterfaceCreation(message, InterfaceType::SINK)) {
        return;
    }
    auto& sink = handles.addHandle(message.source_id,
                                   message.source_handle,
                                   InterfaceType::SINK,
                                   message.name(),
                                   message.getString(typeStringLoc),
                                   message.getString(unitStringLoc));
    addLocalInfo(sink, message);

    if (!isRootc) {
        transmit(parent_route_id, message);
    } else {
        findAndNotifyInputTargets(sink, sink.key);
        findAndNotifyEndpointTargets(sink, sink.key);
    }
}

void CoreBroker::linkInterfaces(ActionMessage& command)
{
    switch (command.action()) {
        case CMD_DATA_LINK: {
            auto* pub = handles.getInterfaceHandle(command.name(), InterfaceType::PUBLICATION);
            if (pub != nullptr) {
                command.name(command.getString(targetStringLoc));
                command.setAction(CMD_ADD_NAMED_INPUT);
                command.setSource(pub->handle);
                checkForNamedInterface(command);
            } else {
                auto* input = handles.getInterfaceHandle(command.getString(targetStringLoc),
                                                         InterfaceType::INPUT);
                if (input == nullptr) {
                    if (isRootc) {
                        unknownHandles.addDataLink(command.name(),
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
            auto* ept = handles.getInterfaceHandle(command.name(), InterfaceType::ENDPOINT);
            if (ept != nullptr) {
                command.name(command.getString(targetStringLoc));
                command.setAction(CMD_ADD_NAMED_ENDPOINT);
                setActionFlag(command, destination_target);
                command.counter = static_cast<uint16_t>(InterfaceType::ENDPOINT);
                command.setSource(ept->handle);
                checkForNamedInterface(command);
            } else {
                auto* target = handles.getInterfaceHandle(command.getString(targetStringLoc),
                                                          InterfaceType::ENDPOINT);
                if (target == nullptr) {
                    if (isRootc) {
                        unknownHandles.addEndpointLink(command.name(),
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
            auto* filt = handles.getInterfaceHandle(command.name(), InterfaceType::FILTER);
            if (filt != nullptr) {
                command.payload = command.getString(targetStringLoc);
                command.setAction(CMD_ADD_NAMED_ENDPOINT);
                command.setSource(filt->handle);
                if (checkActionFlag(*filt, clone_flag)) {
                    setActionFlag(command, clone_flag);
                }
                checkForNamedInterface(command);
            } else {
                auto* ept = handles.getInterfaceHandle(command.getString(targetStringLoc),
                                                       InterfaceType::ENDPOINT);
                if (ept == nullptr) {
                    if (isRootc) {
                        if (checkActionFlag(command, destination_target)) {
                            unknownHandles.addDestinationFilterLink(command.name(),
                                                                    command.getString(
                                                                        targetStringLoc));
                        } else {
                            unknownHandles.addSourceFilterLink(command.name(),
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
        case CMD_ADD_ALIAS:
            try {
                handles.addAlias(command.payload.to_string(), command.getString(targetStringLoc));
                if (!isRootc) {
                    routeMessage(std::move(command), parent_broker_id);
                }
            }
            catch (const std::runtime_error& rtError) {
                ActionMessage error(CMD_ERROR);
                error.setDestination(command.getSource());
                error.source_id = global_broker_id_local;
                error.payload = rtError.what();
                routeMessage(std::move(error));
            }
            break;
        default:
            break;
    }
}

CoreBroker::CoreBroker(bool setAsRootBroker) noexcept:
    _isRoot(setAsRootBroker), isRootc(setAsRootBroker),
    timeoutMon(std::make_unique<TimeoutMonitor>())
{
}

CoreBroker::CoreBroker(std::string_view broker_name):
    BrokerBase(broker_name), timeoutMon(std::make_unique<TimeoutMonitor>())
{
}

void CoreBroker::configure(std::string_view configureString)
{
    if (transitionBrokerState(BrokerState::CREATED, BrokerState::CONFIGURING)) {
        auto result = parseArgs(configureString);
        if (result != 0) {
            setBrokerState(BrokerState::CREATED);
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
    if (transitionBrokerState(BrokerState::CREATED, BrokerState::CONFIGURING)) {
        auto result = parseArgs(argc, argv);
        if (result != 0) {
            setBrokerState(BrokerState::CREATED);
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
    if (transitionBrokerState(BrokerState::CREATED, BrokerState::CONFIGURING)) {
        auto result = parseArgs(std::move(args));
        if (result != 0) {
            setBrokerState(BrokerState::CREATED);
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
    if (getBrokerState() < BrokerState::CONNECTED) {
        _isRoot = true;
        global_id = gRootBrokerID;
    }
}

bool CoreBroker::connect()
{
    if (getBrokerState() < BrokerState::CONNECTED) {
        if (transitionBrokerState(BrokerState::CONFIGURED, BrokerState::CONNECTING)) {
            LOG_CONNECTIONS(parent_broker_id, getIdentifier(), "connecting");
            timeoutMon->setTimeout(std::chrono::milliseconds(timeout));
            auto res = brokerConnect();
            if (res) {
                disconnection.activate();
                setBrokerState(BrokerState::CONNECTED);
                addActionMessage(CMD_BROKER_SETUP);
                if (!_isRoot) {
                    ActionMessage reg(CMD_REG_BROKER);
                    reg.source_id = GlobalFederateId{};
                    reg.name(getIdentifier());
                    if (no_ping) {
                        setActionFlag(reg, slow_responding_flag);
                    }
                    if (useJsonSerialization) {
                        setActionFlag(reg, use_json_serialization_flag);
                    }
                    if (!brokerKey.empty() && brokerKey != universalKey) {
                        reg.setStringData(getAddress(), brokerKey);
                    } else {
                        reg.setStringData(getAddress());
                    }
                    transmit(parent_route_id, reg);
                }
                LOG_SUMMARY(parent_broker_id,
                            getIdentifier(),
                            fmt::format("Broker {} connected on {}",
                                        getIdentifier(),
                                        getAddress()));
                if (!configString.empty()) {
                    makeConnections(configString);
                }
            } else {
                setBrokerState(BrokerState::CONFIGURED);
            }
            return res;
        }
        if (getBrokerState() == BrokerState::CONNECTING) {
            while (getBrokerState() == BrokerState::CONNECTING) {
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

void CoreBroker::globalError(int32_t errorCode, std::string_view errorString)
{
    ActionMessage error(CMD_GLOBAL_ERROR);
    error.source_id = getGlobalId();
    error.messageID = errorCode;
    error.payload = errorString;
    addActionMessage(error);
}

bool CoreBroker::isConnected() const
{
    auto state = getBrokerState();
    return ((state >= BrokerState::CONNECTED) && (state < BrokerState::TERMINATING));
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
    if (cBrokerState >= BrokerState::TERMINATING) {
        return;
    }
    if (cBrokerState > BrokerState::CONFIGURED) {
        LOG_CONNECTIONS(parent_broker_id, getIdentifier(), "||disconnecting");
        setBrokerState(BrokerState::TERMINATING);
        brokerDisconnect();
    }
    setBrokerState(BrokerState::TERMINATED);

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
    if (!mPreviousLocalBrokerIdentifier.empty()) {
        auto keepBrokerAlive2 = BrokerFactory::findBroker(mPreviousLocalBrokerIdentifier);
        if (keepBrokerAlive2) {
            BrokerFactory::unregisterBroker(mPreviousLocalBrokerIdentifier);
        }
    }
}

void CoreBroker::disconnect()
{
    addActionMessage(CMD_USER_DISCONNECT);
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
            addActionMessage(CMD_USER_DISCONNECT);
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
        if ((!broker._nonLocal) && (broker.state < ConnectionState::DISCONNECTED)) {
            cmd.dest_id = broker.global_id;
            transmit(broker.route, cmd);
        }
    }
}
// get the action associated with an interfaceType
static action_message_def::action_t getAction(InterfaceType type)
{
    switch (type) {
        case InterfaceType::FILTER:
            return CMD_ADD_FILTER;
        case InterfaceType::PUBLICATION:
            return CMD_ADD_PUBLISHER;
        case InterfaceType::INPUT:
            return CMD_ADD_SUBSCRIBER;
        default:
            return CMD_ADD_ENDPOINT;
    }
}

// get the matching action associated with an interfaceType
static action_message_def::action_t getMatchAction(InterfaceType type, InterfaceType destType)
{
    switch (type) {
        case InterfaceType::FILTER:
            return CMD_ADD_ENDPOINT;
        case InterfaceType::PUBLICATION:
            return CMD_ADD_SUBSCRIBER;
        case InterfaceType::INPUT:
            return CMD_ADD_PUBLISHER;
        default:
            return (destType == InterfaceType::FILTER) ? CMD_ADD_FILTER : CMD_ADD_ENDPOINT;
    }
}

// get the matching type associated with an interfaceType
static InterfaceType getMatchType(InterfaceType type)
{
    switch (type) {
        case InterfaceType::FILTER:
            return InterfaceType::ENDPOINT;
        case InterfaceType::PUBLICATION:
            return InterfaceType::INPUT;
        case InterfaceType::INPUT:
            return InterfaceType::PUBLICATION;
        default:
            return InterfaceType::ENDPOINT;
    }
}
void CoreBroker::connectInterfaces(
    const BasicHandleInfo& origin,
    uint32_t originFlags,
    const BasicHandleInfo& target,
    uint32_t targetFlags,
    std::pair<action_message_def::action_t, action_message_def::action_t> actions)
{
    // notify the target about an origin
    ActionMessage connect(actions.first);
    connect.setSource(origin.handle);
    connect.setDestination(target.handle);
    connect.flags = originFlags;
    connect.name(origin.key);
    if (!origin.type.empty()) {
        connect.setString(typeStringLoc, origin.type);
    }
    if (!origin.units.empty()) {
        connect.setString(unitStringLoc, origin.units);
    }
    transmit(getRoute(connect.dest_id), connect);

    connect.setAction(actions.second);
    connect.name(target.key);
    connect.clearStringData();
    if (!target.type.empty()) {
        connect.setString(typeStringLoc, target.type);
    }
    if (!target.units.empty()) {
        connect.setString(unitStringLoc, target.units);
    }

    connect.flags = targetFlags;

    connect.swapSourceDest();
    transmit(getRoute(connect.dest_id), connect);
}

void CoreBroker::findRegexMatch(const std::string& target,
                                InterfaceType type,
                                GlobalHandle handle,
                                uint16_t flags)
{
    const auto* dest = handles.findHandle(handle);

    try {
        auto matches = handles.regexSearch(target, type);
        for (auto& mtch : matches) {
            const auto* hnd = handles.findHandle(mtch);
            if (hnd == nullptr) {
                continue;
            }
            auto destFlags = flags;
            if (dest != nullptr && dest->handleType == InterfaceType::FILTER) {
                if (checkActionFlag(*dest, clone_flag)) {
                    destFlags |= make_flags(clone_flag);
                    flags |= make_flags(clone_flag);
                }
            }
            if (type == InterfaceType::ENDPOINT &&
                (dest == nullptr || dest->handleType != InterfaceType::FILTER)) {
                destFlags = toggle_flag(destFlags, destination_target);
            }
            connectInterfaces(*hnd,
                              flags,
                              (dest != nullptr) ? *dest :
                                                  BasicHandleInfo(handle, getMatchType(type)),

                              destFlags,
                              std::make_pair(getAction(type),
                                             getMatchAction(type,
                                                            (dest != nullptr) ?
                                                                dest->handleType :
                                                                getMatchType(type))));
        }
    }
    catch (const std::invalid_argument& ia) {
        LOG_WARNING(global_id.load(),
                    getIdentifier(),
                    fmt::format("invalid regular expression processing {}", ia.what()));
    }
}

static constexpr auto regexKey = "REGEX:";

void CoreBroker::executeInitializationOperations(bool iterating)
{
    if (iterating) {
        ActionMessage init(CMD_INIT_GRANT);
        init.source_id = global_broker_id_local;
        setActionFlag(init, iteration_requested_flag);
        setBrokerState(BrokerState::CONNECTED);
        mBrokers.apply([&init, this](auto& broker) {
            if ((!broker._nonLocal) && (broker.state < ConnectionState::DISCONNECTED)) {
                if (broker.initIterating) {
                    broker.initIterating = false;
                    broker.state = ConnectionState::CONNECTED;
                    init.dest_id = broker.global_id;
                    transmit(broker.route, init);
                }
            }
        });
        initIterating = false;
        return;
    }
    if (brokerKey == universalKey) {
        LOG_SUMMARY(global_broker_id_local, getIdentifier(), "Broker started with universal key");
    }
    checkDependencies();
    if (!mTimeMonitorFederate.empty()) {
        loadTimeMonitor(true, std::string_view{});
    }
    if (unknownHandles.hasUnknowns()) {
        unknownHandles.processUnknownLinks([this](const std::string& origin,
                                                  InterfaceType originType,
                                                  const std::string& target,
                                                  InterfaceType targetType) {
            const auto* originHandle = handles.getInterfaceHandle(origin, originType);
            if (originHandle != nullptr) {
                const auto* targetHandle = handles.getInterfaceHandle(target, targetType);
                if (targetHandle != nullptr) {
                    if (originType == InterfaceType::PUBLICATION) {
                        ActionMessage datalink(CMD_DATA_LINK);
                        datalink.name(originHandle->key);
                        datalink.setString(targetStringLoc, targetHandle->key);
                        linkInterfaces(datalink);
                    } else if (originType == InterfaceType::ENDPOINT &&
                               targetType == InterfaceType::ENDPOINT) {
                        ActionMessage eptlink(CMD_ENDPOINT_LINK);
                        eptlink.name(originHandle->key);
                        eptlink.setString(targetStringLoc, targetHandle->key);
                        linkInterfaces(eptlink);
                    }
                    // TODO(PT):: make this work for filters
                }
            }
        });
        std::vector<std::vector<std::string>> foundAliasHandles;
        foundAliasHandles.resize(4);
        bool useRegex{false};
        unknownHandles.processUnknowns(
            [this, &foundAliasHandles, &useRegex](const std::string& target,
                                                  InterfaceType type,
                                                  UnknownHandleManager::TargetInfo /*target*/) {
                const auto* info = handles.getInterfaceHandle(target, type);
                if (info == nullptr) {
                    if (!useRegex) {
                        if (target.compare(0, 6, regexKey) == 0) {
                            useRegex = true;
                        }
                    }
                    return;
                }
                switch (type) {
                    case InterfaceType::PUBLICATION:
                        foundAliasHandles[0].emplace_back(target);
                        break;
                    case InterfaceType::INPUT:
                        foundAliasHandles[1].emplace_back(target);
                        break;
                    case InterfaceType::ENDPOINT:
                        foundAliasHandles[2].emplace_back(target);
                        break;
                    case InterfaceType::FILTER:
                        foundAliasHandles[3].emplace_back(target);
                        break;
                    default:
                        break;
                }
            });
        if (!foundAliasHandles[0].empty()) {
            for (const auto& target : foundAliasHandles[0]) {
                auto* info = handles.getInterfaceHandle(target, InterfaceType::PUBLICATION);
                findAndNotifyPublicationTargets(*info, target);
            }
        }
        if (!foundAliasHandles[1].empty()) {
            for (const auto& target : foundAliasHandles[1]) {
                auto* info = handles.getInterfaceHandle(target, InterfaceType::INPUT);
                findAndNotifyInputTargets(*info, target);
            }
        }
        if (!foundAliasHandles[2].empty()) {
            for (const auto& target : foundAliasHandles[2]) {
                auto* info = handles.getInterfaceHandle(target, InterfaceType::ENDPOINT);
                findAndNotifyEndpointTargets(*info, target);
            }
        }
        if (!foundAliasHandles[3].empty()) {
            for (const auto& target : foundAliasHandles[3]) {
                auto* info = handles.getInterfaceHandle(target, InterfaceType::FILTER);
                findAndNotifyFilterTargets(*info, target);
            }
        }
        if (useRegex) {
            unknownHandles.processUnknowns([this](const std::string& target,
                                                  InterfaceType type,
                                                  UnknownHandleManager::TargetInfo tinfo) {
                if (target.compare(0, 6, regexKey) == 0) {
                    findRegexMatch(target, type, tinfo.first, tinfo.second);
                }
            });
            unknownHandles.clearUnknownsIf([](const std::string& target,
                                              InterfaceType /*type*/,
                                              UnknownHandleManager::TargetInfo /*tinfo*/) {
                return (target.compare(0, 6, regexKey) == 0);
            });
        }
        /** now do a check on the unknownLinks*/

        if (errorOnUnmatchedConnections) {
            if (unknownHandles.hasUnknowns()) {
                int unmatchedCount{0};
                ActionMessage eMiss(CMD_ERROR);
                eMiss.source_id = global_broker_id_local;
                eMiss.messageID = defs::Errors::CONNECTION_FAILURE;
                std::string errorString{"unmatched connections"};

                unknownHandles.processUnknowns(
                    [&errorString, &unmatchedCount](const std::string& target,
                                                    InterfaceType type,
                                                    UnknownHandleManager::TargetInfo /*tinfo*/) {
                        errorString.append(fmt::format("\nUnable to connect {} to target {}",
                                                       interfaceTypeName(type),
                                                       target));
                        ++unmatchedCount;
                    });

                unknownHandles.processUnknownLinks(
                    [this, &errorString, &unmatchedCount](const std::string& origin,
                                                          InterfaceType originType,
                                                          const std::string& target,
                                                          InterfaceType targetType) {
                        const auto* originHandle = handles.getInterfaceHandle(origin, originType);
                        if (originHandle != nullptr) {
                            const auto* targetHandle =
                                handles.getInterfaceHandle(target, targetType);
                            if (targetHandle != nullptr) {
                                return;
                            }
                        }
                        ++unmatchedCount;
                        errorString.append(
                            fmt::format("\nUnable to make link between {} and {}", origin, target));
                    });
                if (unmatchedCount > 0) {
                    LOG_ERROR(parent_broker_id, getIdentifier(), errorString);
                    eMiss.payload = errorString;
                    eMiss.dest_handle = InterfaceHandle{};
                    broadcast(eMiss);
                    sendDisconnect(CMD_GLOBAL_DISCONNECT);
                    addActionMessage(CMD_STOP);
                    return;
                }
            }
        }
        if (unknownHandles.hasNonOptionalUnknowns()) {
            if (unknownHandles.hasRequiredUnknowns()) {
                ActionMessage eMiss(CMD_ERROR);
                eMiss.source_id = global_broker_id_local;
                eMiss.messageID = defs::Errors::CONNECTION_FAILURE;
                unknownHandles.processRequiredUnknowns(
                    [this, &eMiss](const std::string& target,
                                   InterfaceType type,
                                   UnknownHandleManager::TargetInfo tinfo) {
                        eMiss.payload = fmt::format("Unable to connect to required {} target {}",
                                                    interfaceTypeName(type),
                                                    target);
                        LOG_ERROR(parent_broker_id, getIdentifier(), eMiss.payload.to_string());

                        eMiss.setDestination(tinfo.first);
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
            unknownHandles.processNonOptionalUnknowns(
                [this, &wMiss](const std::string& target,
                               InterfaceType type,
                               UnknownHandleManager::TargetInfo tinfo) {
                    wMiss.payload = fmt::format("Unable to connect to {} target {}",
                                                interfaceTypeName(type),
                                                target);
                    LOG_WARNING(parent_broker_id, getIdentifier(), wMiss.payload.to_string());

                    wMiss.setDestination(tinfo.first);
                    routeMessage(wMiss);
                });
        }
    }

    ActionMessage init(CMD_INIT_GRANT);
    init.source_id = global_broker_id_local;
    setBrokerState(BrokerState::OPERATING);
    broadcast(init);
    timeCoord->enteringExecMode();
    auto res = timeCoord->checkExecEntry();
    if (res == MessageProcessingResult::NEXT_STEP) {
        enteredExecutionMode = true;
    }
    logFlush();
}

void CoreBroker::findAndNotifyInputTargets(BasicHandleInfo& handleInfo, const std::string& key)
{
    auto Handles = unknownHandles.checkForInputs(key);
    for (auto& target : Handles) {
        auto* pub = handles.findHandle(target.first);
        if (pub == nullptr) {
            connectInterfaces(handleInfo,
                              handleInfo.flags,
                              BasicHandleInfo(target.first.fed_id,
                                              target.first.handle,
                                              InterfaceType::PUBLICATION),

                              target.second,
                              std::make_pair(CMD_ADD_SUBSCRIBER, CMD_ADD_PUBLISHER));
        } else {
            connectInterfaces(handleInfo,
                              handleInfo.flags,
                              *pub,

                              target.second,
                              std::make_pair(CMD_ADD_SUBSCRIBER, CMD_ADD_PUBLISHER));
        }
    }
    if (!Handles.empty()) {
        unknownHandles.clearInput(key);
    }
    if (getBrokerState() == BrokerState::OPERATING) {
        Handles = unknownHandles.checkForReconnectionInputs(key);
        for (auto& target : Handles) {
            auto* pub = handles.findHandle(target.first);
            if (pub == nullptr) {
                connectInterfaces(handleInfo,
                                  handleInfo.flags,
                                  BasicHandleInfo(target.first.fed_id,
                                                  target.first.handle,
                                                  InterfaceType::PUBLICATION),

                                  target.second,
                                  std::make_pair(CMD_ADD_SUBSCRIBER, CMD_ADD_PUBLISHER));
            } else {
                connectInterfaces(handleInfo,
                                  handleInfo.flags,
                                  *pub,

                                  target.second,
                                  std::make_pair(CMD_ADD_SUBSCRIBER, CMD_ADD_PUBLISHER));
            }
        }
    }
}

void CoreBroker::findAndNotifyPublicationTargets(BasicHandleInfo& handleInfo,
                                                 const std::string& key)
{
    auto subHandles = unknownHandles.checkForPublications(key);
    for (const auto& sub : subHandles) {
        connectInterfaces(handleInfo,
                          sub.second,
                          BasicHandleInfo(sub.first.fed_id, sub.first.handle, InterfaceType::INPUT),

                          handleInfo.flags,
                          std::make_pair(CMD_ADD_PUBLISHER, CMD_ADD_SUBSCRIBER));
    }

    auto Pubtargets = unknownHandles.checkForLinks(key);
    for (const auto& sub : Pubtargets) {
        ActionMessage link(CMD_ADD_NAMED_INPUT);
        link.name(sub);
        link.setSource(handleInfo.handle);
        checkForNamedInterface(link);
    }
    if (!(subHandles.empty() && Pubtargets.empty())) {
        unknownHandles.clearPublication(key);
    }
    if (getBrokerState() == BrokerState::OPERATING) {
        subHandles = unknownHandles.checkForReconnectionPublications(key);
        for (const auto& sub : subHandles) {
            connectInterfaces(handleInfo,
                              sub.second,
                              BasicHandleInfo(sub.first.fed_id,
                                              sub.first.handle,
                                              InterfaceType::INPUT),

                              handleInfo.flags,
                              std::make_pair(CMD_ADD_PUBLISHER, CMD_ADD_SUBSCRIBER));
        }
    }
}

void CoreBroker::findAndNotifyEndpointTargets(BasicHandleInfo& handleInfo, const std::string& key)
{
    auto uHandles = unknownHandles.checkForEndpoints(key);
    for (const auto& target : uHandles) {
        const auto* iface = handles.findHandle(target.first);
        auto destFlags = target.second;
        if (iface->handleType != InterfaceType::FILTER) {
            destFlags = toggle_flag(destFlags, destination_target);
        }

        connectInterfaces(handleInfo,
                          target.second,
                          *iface,

                          destFlags,
                          std::make_pair(CMD_ADD_ENDPOINT,
                                         (iface->handleType != InterfaceType::FILTER) ?
                                             CMD_ADD_ENDPOINT :
                                             CMD_ADD_FILTER));
    }

    auto eptTargets = unknownHandles.checkForEndpointLinks(key);
    for (const auto& ept : eptTargets) {
        ActionMessage link(CMD_ADD_NAMED_ENDPOINT);
        link.name(ept);
        link.setSource(handleInfo.handle);
        setActionFlag(link, destination_target);
        link.counter = static_cast<uint16_t>(InterfaceType::ENDPOINT);
        checkForNamedInterface(link);
    }

    if (!(uHandles.empty() && eptTargets.empty())) {
        unknownHandles.clearEndpoint(key);
    }

    if (getBrokerState() == BrokerState::OPERATING) {
        uHandles = unknownHandles.checkForReconnectionEndpoints(key);
        for (const auto& target : uHandles) {
            const auto* iface = handles.findHandle(target.first);
            auto destFlags = target.second;
            if (iface->handleType != InterfaceType::FILTER) {
                destFlags = toggle_flag(destFlags, destination_target);
            }

            connectInterfaces(handleInfo,
                              target.second,
                              *iface,

                              destFlags,
                              std::make_pair(CMD_ADD_ENDPOINT,
                                             (iface->handleType != InterfaceType::FILTER) ?
                                                 CMD_ADD_ENDPOINT :
                                                 CMD_ADD_FILTER));
        }
    }
}

void CoreBroker::findAndNotifyFilterTargets(BasicHandleInfo& handleInfo, const std::string& key)
{
    auto Handles = unknownHandles.checkForFilters(key);
    for (const auto& target : Handles) {
        auto flags = target.second;
        if (checkActionFlag(handleInfo, clone_flag)) {
            flags |= make_flags(clone_flag);
        }
        connectInterfaces(handleInfo,
                          flags,
                          BasicHandleInfo(target.first.fed_id,
                                          target.first.handle,
                                          InterfaceType::ENDPOINT),

                          flags,
                          std::make_pair(CMD_ADD_FILTER, CMD_ADD_ENDPOINT));
    }

    auto FiltDestTargets = unknownHandles.checkForFilterDestTargets(key);
    for (const auto& target : FiltDestTargets) {
        ActionMessage link(CMD_ADD_NAMED_ENDPOINT);
        link.name(target);
        link.setSource(handleInfo.handle);
        link.flags = handleInfo.flags;
        setActionFlag(link, destination_target);
        if (checkActionFlag(handleInfo, clone_flag)) {
            setActionFlag(link, clone_flag);
        }
        checkForNamedInterface(link);
    }

    auto FiltSourceTargets = unknownHandles.checkForFilterSourceTargets(key);
    for (const auto& target : FiltSourceTargets) {
        ActionMessage link(CMD_ADD_NAMED_ENDPOINT);
        link.name(target);
        link.flags = handleInfo.flags;
        link.setSource(handleInfo.handle);
        if (checkActionFlag(handleInfo, clone_flag)) {
            setActionFlag(link, clone_flag);
        }
        checkForNamedInterface(link);
    }
    if (!(Handles.empty() && FiltDestTargets.empty() && FiltSourceTargets.empty())) {
        unknownHandles.clearFilter(key);
    }
    if (getBrokerState() == BrokerState::OPERATING) {
        Handles = unknownHandles.checkForFilters(key);
        for (const auto& target : Handles) {
            auto flags = target.second;
            if (checkActionFlag(handleInfo, clone_flag)) {
                flags |= make_flags(clone_flag);
            }
            connectInterfaces(handleInfo,
                              flags,
                              BasicHandleInfo(target.first.fed_id,
                                              target.first.handle,
                                              InterfaceType::ENDPOINT),

                              flags,
                              std::make_pair(CMD_ADD_FILTER, CMD_ADD_ENDPOINT));
        }
    }
}

void CoreBroker::processError(ActionMessage& command)
{
    sendToLogger(command.source_id,
                 LogLevels::ERROR_LEVEL,
                 std::string_view(),
                 command.payload.to_string());
    if (command.source_id == global_broker_id_local) {
        setBrokerState(BrokerState::ERRORED);
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
        setBrokerState(BrokerState::ERRORED);
        if (command.action() == CMD_GLOBAL_ERROR) {
            setErrorState(command.messageID, command.payload.to_string());
        }
        broadcast(command);
        return;
    }

    auto* brk = getBrokerById(GlobalBrokerId{command.source_id});
    if (brk == nullptr) {
        auto fed = mFederates.find(command.source_id);
        if (fed != mFederates.end()) {
            fed->state = ConnectionState::ERROR_STATE;
        }
    } else {
        brk->state = ConnectionState::ERROR_STATE;
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

void CoreBroker::disconnectTiming(ActionMessage& command)
{
    if (hasTimeDependency) {
        if (!enteredExecutionMode) {
            if (getBrokerState() >= BrokerState::OPERATING) {
                if (timeCoord->processTimeMessage(command) != TimeProcessingResult::NOT_PROCESSED) {
                    auto res = timeCoord->checkExecEntry();
                    if (res == MessageProcessingResult::NEXT_STEP) {
                        enteredExecutionMode = true;
                    }
                }
            }
        } else {
            if (timeCoord->processTimeMessage(command) != TimeProcessingResult::NOT_PROCESSED) {
                timeCoord->updateTimeFactors();
            }
        }
    }
}

void CoreBroker::processBrokerDisconnect(ActionMessage& command, BasicBrokerInfo* brk)
{
    if (!isRootc) {
        if (command.source_id == higher_broker_id) {
            LOG_CONNECTIONS(parent_broker_id, getIdentifier(), "got disconnect from parent");
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

    if ((getAllConnectionState() >= ConnectionState::DISCONNECTED)) {
        timeCoord->disconnect();
        if (!isRootc) {
            ActionMessage dis(CMD_DISCONNECT);
            dis.source_id = global_broker_id_local;
            transmit(parent_route_id, dis);
        } else {
            if ((brk != nullptr) && (!brk->_nonLocal)) {
                if (globalDisconnect) {
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
                } else {
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
            }
            addActionMessage(CMD_STOP);
        }
    } else {
        if ((brk != nullptr) && (!brk->_nonLocal)) {
            if (!globalDisconnect) {
                if (!checkActionFlag(command, error_flag)) {
                    ActionMessage dis((brk->_core) ? CMD_DISCONNECT_CORE_ACK :
                                                     CMD_DISCONNECT_BROKER_ACK);
                    dis.source_id = global_broker_id_local;
                    dis.dest_id = brk->global_id;
                    transmit(brk->route, dis);
                }
                brk->_sent_disconnect_ack = true;
            }
            if ((!isRootc) && (getBrokerState() < BrokerState::OPERATING)) {
                command.setAction((brk->_core) ? CMD_DISCONNECT_CORE : CMD_DISCONNECT_BROKER);
                transmit(parent_route_id, command);
            }
            if (!globalDisconnect) {
                removeRoute(brk->route);
            }
        } else {
            if ((!isRootc) && (getBrokerState() < BrokerState::OPERATING)) {
                if (brk != nullptr) {
                    command.setAction((brk->_core) ? CMD_DISCONNECT_CORE : CMD_DISCONNECT_BROKER);
                    transmit(parent_route_id, command);
                }
            }
        }
    }
}

void CoreBroker::processDisconnectCommand(ActionMessage& command)
{
    auto* brk = getBrokerById(GlobalBrokerId{command.source_id});
    switch (command.action()) {
        case CMD_DISCONNECT:
        case CMD_PRIORITY_DISCONNECT:
            if (command.dest_id == global_broker_id_local) {
                // deal with the time implications of the message
                disconnectTiming(command);
            } else if (command.dest_id == parent_broker_id) {
                processBrokerDisconnect(command, brk);
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
    for (auto& builderData : mapBuilders) {
        auto& builder = std::get<0>(builderData);
        auto& requesters = std::get<1>(builderData);
        if (builder.isCompleted()) {
            return;
        }
        if (builder.clearComponents(brkid.baseValue())) {
            auto str = builder.generate();
            for (int ii = 0; ii < static_cast<int>(requesters.size()) - 1; ++ii) {
                if (requesters[ii].dest_id == global_broker_id_local) {
                    activeQueries.setDelayedValue(requesters[ii].messageID, str);
                } else {
                    requesters[ii].payload = str;
                    routeMessage(std::move(requesters[ii]));
                }
            }
            if (requesters.back().dest_id == global_broker_id_local) {
                activeQueries.setDelayedValue(requesters.back().messageID, std::move(str));
            } else {
                requesters.back().payload = std::move(str);
                routeMessage(std::move(requesters.back()));
            }

            requesters.clear();
            if (std::get<2>(builderData) == QueryReuse::DISABLED) {
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
            if (brk.state != ConnectionState::ERROR_STATE) {
                brk.state = ConnectionState::DISCONNECTED;
            }
        }
        if (brk.parent == brkid) {
            if (brk.state != ConnectionState::ERROR_STATE) {
                brk.state = ConnectionState::DISCONNECTED;
                markAsDisconnected(brk.global_id);
            }
        }
    }
    for (size_t ii = 0; ii < mFederates.size(); ++ii) {  // NOLINT
        auto& fed = mFederates[ii];

        if (fed.parent == brkid) {
            if (fed.state != ConnectionState::ERROR_STATE) {
                fed.state = ConnectionState::DISCONNECTED;
                if (fed.reentrant) {
                    handles.removeFederateHandles(fed.global_id);
                }
            }
        }
    }
}

void CoreBroker::disconnectBroker(BasicBrokerInfo& brk)
{
    markAsDisconnected(brk.global_id);
    checkInFlightQueries(brk.global_id);
    if (getBrokerState() < BrokerState::OPERATING) {
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

void CoreBroker::setLogFile(std::string_view lfile)
{
    ActionMessage cmd(CMD_BROKER_CONFIGURE);
    cmd.dest_id = global_id.load();
    cmd.messageID = UPDATE_LOGGING_FILE;
    cmd.payload = lfile;
    addActionMessage(cmd);
}

// public query function
std::string CoreBroker::query(std::string_view target,
                              std::string_view queryStr,
                              HelicsSequencingModes mode)
{
    if (getBrokerState() >= BrokerState::TERMINATING) {
        if (target == "broker" || target == getIdentifier() || target.empty() ||
            (target == "root" && _isRoot) || (target == "federation" && _isRoot)) {
            auto res = quickBrokerQueries(queryStr);
            if (!res.empty()) {
                return res;
            }
            if (queryStr == "logs") {
                nlohmann::json base;
                addBaseInformation(base, !isRoot());
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

void CoreBroker::setGlobal(std::string_view valueName, std::string_view value)
{
    ActionMessage querycmd(CMD_SET_GLOBAL);
    querycmd.source_id = global_id.load();
    querycmd.payload = valueName;
    querycmd.setStringData(value);
    transmitToParent(std::move(querycmd));
}

void CoreBroker::sendCommand(std::string_view target,
                             std::string_view commandStr,
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

    if (target == "broker" || target == getIdentifier() || target.empty() ||
        (target == "root" && _isRoot) || (target == "federation" && _isRoot)) {
        addActionMessage(std::move(cmdcmd));
    } else {
        transmitToParent(std::move(cmdcmd));
    }
}

static const std::set<std::string> querySet{"isinit",
                                            "isconnected",
                                            "exists",
                                            "name",
                                            "identifier",
                                            "address",
                                            "queries",
                                            "address",
                                            "counts",
                                            "summary",
                                            "federates",
                                            "brokers",
                                            "inputs",
                                            "barriers",
                                            "input_details",
                                            "endpoints",
                                            "endpoint_details",
                                            "publications",
                                            "publication_details",
                                            "filters",
                                            "filter_details",
                                            "interface_details",
                                            "version",
                                            "version_all",
                                            "federate_map",
                                            "dependency_graph",
                                            "data_flow_graph",
                                            "dependencies",
                                            "dependson",
                                            "logs",
                                            "monitor",
                                            "dependents",
                                            "status",
                                            "current_time",
                                            "global_time",
                                            "global_state",
                                            "global_flush",
                                            "current_state",
                                            "unconnected_interfaces",
                                            "logs"};

static const std::map<std::string_view, std::pair<std::uint16_t, QueryReuse>> mapIndex{
    {"global_time", {CURRENT_TIME_MAP, QueryReuse::DISABLED}},
    {"federate_map", {FEDERATE_MAP, QueryReuse::ENABLED}},
    {"dependency_graph", {DEPENDENCY_GRAPH, QueryReuse::ENABLED}},
    {"data_flow_graph", {DATA_FLOW_GRAPH, QueryReuse::ENABLED}},
    {"version_all", {VERSION_ALL, QueryReuse::ENABLED}},
    {"global_state", {GLOBAL_STATE, QueryReuse::DISABLED}},
    {"global_time_debugging", {GLOBAL_TIME_DEBUGGING, QueryReuse::DISABLED}},
    {"global_status", {GLOBAL_STATUS, QueryReuse::DISABLED}},
    {"barriers", {BARRIERS, QueryReuse::DISABLED}},
    {"unconnected_interfaces", {UNCONNECTED_INTERFACES, QueryReuse::DISABLED}},
    {"global_flush", {GLOBAL_FLUSH, QueryReuse::DISABLED}}};

std::string CoreBroker::quickBrokerQueries(std::string_view request) const
{
    if (request == "isinit") {
        return (getBrokerState() >= BrokerState::OPERATING) ? std::string("true") :
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
        return generateStringVector(querySet, [](const std::string& data) { return data; });
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
        nlohmann::json base;
        addBaseInformation(base, !isRootc);
        base["state"] = brokerStateName(getBrokerState());
        base["status"] = isConnected();
        return fileops::generateJsonString(base);
    }
    return {};
}

std::string CoreBroker::generateQueryAnswer(std::string_view request, bool force_ordering)
{
    auto addHeader = [this](nlohmann::json& base) { addBaseInformation(base, !isRootc); };

    auto res = quickBrokerQueries(request);
    if (!res.empty()) {
        return res;
    }
    if (request == "counts") {
        nlohmann::json base;
        addHeader(base);
        base["brokers"] = static_cast<int>(mBrokers.size());
        base["federates"] = static_cast<int>(mFederates.size());
        base["countable_federates"] = getCountableFederates();
        base["interfaces"] = static_cast<int>(handles.size());
        return fileops::generateJsonString(base);
    }
    if (request == "summary") {
        return generateFederationSummary();
    }
    if (request == "config") {
        nlohmann::json base;
        base["name"] = getIdentifier();
        if (uuid_like) {
            base["uuid"] = getIdentifier();
        }
        if (!isRootc) {
            base["parent"] = higher_broker_id.baseValue();
        }
        base["id"] = global_broker_id_local.baseValue();
        base["log_level"] = logLevelToString(static_cast<LogLevels>(maxLogLevel.load()));
        base["root"] = isRoot();
        return fileops::generateJsonString(base);
    }
    if (request == "monitor") {
        return std::string{"\""} + mTimeMonitorFederate + '"';
    }
    if (request == "logs") {
        nlohmann::json base;
        addBaseInformation(base, !isRootc);
        bufferToJson(mLogManager->getLogBuffer(), base);
        return fileops::generateJsonString(base);
    }
    if (request == "federates") {
        return generateStringVector(mFederates, [](auto& fed) { return fed.name; });
    }
    if (request == "brokers") {
        return generateStringVector(mBrokers, [](auto& brk) { return brk.name; });
    }
    if (request == "subbrokers") {
        return generateStringVector_if(
            mBrokers, [](auto& brk) { return brk.name; }, [](auto& brk) { return !brk._core; });
    }
    if (request == "cores") {
        return generateStringVector_if(
            mBrokers, [](auto& brk) { return brk.name; }, [](auto& brk) { return brk._core; });
    }
    if (request == "current_state") {
        nlohmann::json base;
        addBaseInformation(base, !isRootc);
        base["state"] = brokerStateName(getBrokerState());
        base["status"] = isConnected();
        base["federates"] = nlohmann::json::array();
        for (const auto& fed : mFederates) {
            nlohmann::json fedstate;
            fedstate["attributes"] = nlohmann::json::object();
            fedstate["attributes"]["name"] = fed.name;
            fedstate["state"] = stateString(fed.state);
            fedstate["attributes"]["id"] = fed.global_id.baseValue();
            fedstate["attributes"]["parent"] = fed.parent.baseValue();
            base["federates"].push_back(std::move(fedstate));
        }
        base["cores"] = nlohmann::json::array();
        base["brokers"] = nlohmann::json::array();
        for (const auto& brk : mBrokers) {
            nlohmann::json brkstate;
            brkstate["state"] = stateString(brk.state);
            brkstate["attributes"] = nlohmann::json::object();
            brkstate["attributes"]["name"] = brk.name;
            brkstate["attributes"]["id"] = brk.global_id.baseValue();
            brkstate["attributes"]["parent"] = brk.parent.baseValue();
            if (brk._core) {
                base["cores"].push_back(std::move(brkstate));
            } else {
                base["brokers"].push_back(std::move(brkstate));
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
            nlohmann::json json;
            addBaseInformation(json, !isRootc);
            json["status"] = "disconnected";
            json["timestep"] = -1;
            return fileops::generateJsonString(json);
        }
    }
    if (request.compare(0, 7, "rename:") == 0) {
        return generateRename(request.substr(7));
    }
    auto mapping = mapIndex.find(request);
    if (mapping != mapIndex.end()) {
        auto index = mapping->second.first;
        if (isValidIndex(index, mapBuilders) && mapping->second.second == QueryReuse::ENABLED) {
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

        initializeMapBuilder(request, index, mapping->second.second, force_ordering);
        if (std::get<0>(mapBuilders[index]).isCompleted()) {
            if (mapping->second.second == QueryReuse::ENABLED) {
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
    auto interfaceQueryResult =
        generateInterfaceQueryResults(request, handles, GlobalFederateId{}, addHeader);
    if (!interfaceQueryResult.empty()) {
        return interfaceQueryResult;
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
        nlohmann::json base;
        addBaseInformation(base, !isRootc);
        base["dependents"] = nlohmann::json::array();
        for (const auto& dep : timeCoord->getDependents()) {
            base["dependents"].push_back(dep.baseValue());
        }
        base["dependencies"] = nlohmann::json::array();
        for (const auto& dep : timeCoord->getDependencies()) {
            base["dependencies"].push_back(dep.baseValue());
        }
        return fileops::generateJsonString(base);
    }
    return generateJsonErrorResponse(JsonErrorCodes::BAD_REQUEST, "unrecognized broker query");
}

std::string CoreBroker::generateGlobalStatus(fileops::JsonMapBuilder& builder)
{
    auto cstate = generateQueryAnswer("current_state", false);
    auto jsonStatus = fileops::loadJsonStr(cstate);
    std::string state;
    if (jsonStatus["federates"][0].is_object()) {
        state = jsonStatus["state"].get<std::string>();
    } else {
        state = "init_requested";
    }

    if (state != "operating") {
        nlohmann::json json;
        json["status"] = state;
        json["timestep"] = -1;
        return fileops::generateJsonString(json);
    }
    Time minTime{Time::maxVal()};
    if (!builder.getJValue()["cores"][0].is_object()) {
        state = "init_requested";
    }
    for (auto& core : builder.getJValue()["cores"]) {
        for (auto& fed : core["federates"]) {
            auto granted = fed["granted_time"].get<double>();
            if (granted < minTime) {
                minTime = granted;
            }
        }
    }
    const std::string tste =
        (minTime >= timeZero) ? std::string("operating") : std::string("init_requested");

    nlohmann::json json;

    if (tste != "operating") {
        json["status"] = tste;
        json["timestep"] = -1;
    } else {
        json["status"] = jsonStatus;
        json["timestep"] = builder.getJValue();
    }

    return fileops::generateJsonString(json);
}

std::string CoreBroker::getNameList(std::string_view gidString) const
{
    if (gidString.back() == ']') {
        gidString.remove_suffix(1);
    }
    if (gidString.front() == '[') {
        gidString.remove_prefix(1);
    }
    auto val = gmlc::utilities::str2vector<int>(gidString, -23, ",:;");
    std::string nameString;
    nameString.push_back('[');
    size_t index = 0;
    while (index + 1 < val.size()) {
        const auto* info = handles.findHandle(
            GlobalHandle(GlobalFederateId(val[index]), InterfaceHandle(val[index + 1])));
        if (info != nullptr) {
            nameString.append(generateJsonQuotedString(info->key));
            nameString.push_back(',');
        }

        index += 2;
    }
    if (nameString.back() == ',') {
        nameString.pop_back();
    }
    nameString.push_back(']');
    return nameString;
}

void CoreBroker::initializeMapBuilder(std::string_view request,
                                      std::uint16_t index,
                                      QueryReuse reuse,
                                      bool force_ordering)
{
    if (!isValidIndex(index, mapBuilders)) {
        mapBuilders.resize(static_cast<size_t>(index) + 1);
    }
    std::get<2>(mapBuilders[index]) = reuse;
    auto& builder = std::get<0>(mapBuilders[index]);
    builder.reset();
    nlohmann::json& base = builder.getJValue();
    addBaseInformation(base, !isRootc);
    base["brokers"] = nlohmann::json::array();
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
                case ConnectionState::CONNECTED:
                case ConnectionState::INIT_REQUESTED:
                case ConnectionState::OPERATING: {
                    int brkindex;
                    if (broker._core) {
                        if (!hasCores) {
                            hasCores = true;
                            base["cores"] = nlohmann::json::array();
                        }
                        brkindex =
                            builder.generatePlaceHolder("cores", broker.global_id.baseValue());
                    } else {
                        if (!hasBrokers) {
                            hasBrokers = true;
                            base["brokers"] = nlohmann::json::array();
                        }
                        brkindex =
                            builder.generatePlaceHolder("brokers", broker.global_id.baseValue());
                    }
                    queryReq.messageID = brkindex;
                    queryReq.dest_id = broker.global_id;
                    transmit(broker.route, queryReq);
                } break;
                case ConnectionState::ERROR_STATE:
                case ConnectionState::DISCONNECTED:
                case ConnectionState::REQUEST_DISCONNECT:
                    if (index == GLOBAL_STATE) {
                        nlohmann::json brkstate;
                        brkstate["state"] = stateString(broker.state);
                        brkstate["attributes"] = nlohmann::json::object();
                        brkstate["attributes"]["name"] = broker.name;
                        brkstate["attributes"]["id"] = broker.global_id.baseValue();
                        brkstate["attributes"]["parent"] = broker.parent.baseValue();
                        if (broker._core) {
                            if (!hasCores) {
                                base["cores"] = nlohmann::json::array();
                                hasCores = true;
                            }
                            base["cores"].push_back(std::move(brkstate));
                        } else {
                            if (!hasBrokers) {
                                base["brokers"] = nlohmann::json::array();
                                hasBrokers = true;
                            }
                            base["brokers"].push_back(std::move(brkstate));
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
            base["dependents"] = nlohmann::json::array();
            for (const auto& dep : timeCoord->getDependents()) {
                base["dependents"].push_back(dep.baseValue());
            }
            base["dependencies"] = nlohmann::json::array();
            for (const auto& dep : timeCoord->getDependencies()) {
                base["dependencies"].push_back(dep.baseValue());
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
                base["time"] = nlohmann::json();
                timeCoord->generateDebuggingTimeInfo(base["time"]);
            }
            break;
        case UNCONNECTED_INTERFACES:
            if (!global_values.empty()) {
                nlohmann::json tagBlock = nlohmann::json::object();
                for (const auto& global : global_values) {
                    tagBlock[global.first] = global.second;
                }
                base["tags"] = tagBlock;
            }
            const auto& aliases = handles.getAliases();
            if (!aliases.empty()) {
                base["aliases"] = nlohmann::json::array();
                for (const auto& alias : aliases) {
                    const std::string_view interfaceName = alias.first;
                    const auto& aliasNames = alias.second;
                    for (const auto& aliasName : aliasNames) {
                        nlohmann::json aliasSet = nlohmann::json::array();
                        aliasSet.push_back(std::string(interfaceName));
                        aliasSet.push_back(std::string(aliasName));
                        base["aliases"].push_back(std::move(aliasSet));
                    }
                }
            }
            if (unknownHandles.hasUnknowns()) {
                base["unknown_publications"] = nlohmann::json::array();
                base["unknown_inputs"] = nlohmann::json::array();
                base["unknown_endpoints"] = nlohmann::json::array();
                auto unknownProcessor = [&base](const std::string& name,
                                                InterfaceType type,
                                                UnknownHandleManager::TargetInfo /*target*/) {
                    switch (type) {
                        case InterfaceType::INPUT:
                            base["unknown_inputs"].push_back(name);
                            break;
                        case InterfaceType::PUBLICATION:
                            base["unknown_publications"].push_back(name);
                            break;
                        case InterfaceType::ENDPOINT:
                            base["unknown_endpoints"].push_back(name);
                            break;
                        default:
                            break;
                    }
                };
                unknownHandles.processUnknowns(unknownProcessor);
                auto unknownLinkProcessor = [&base](const std::string& origin,
                                                    InterfaceType type1,
                                                    const std::string& target,
                                                    InterfaceType type2) {
                    switch (type2) {
                        case InterfaceType::INPUT:
                            base["unknown_inputs"].push_back(target);
                            base["unknown_publications"].push_back(origin);
                            break;
                        case InterfaceType::ENDPOINT:
                            base["unknown_endpoints"].push_back(target);
                            if (type1 == InterfaceType::ENDPOINT) {
                                base["unknown_endpoints"].push_back(origin);
                            }
                            break;
                        default:
                            break;
                    }
                };
                unknownHandles.processUnknownLinks(unknownLinkProcessor);
            }
            break;
    }
}

void CoreBroker::processLocalQuery(const ActionMessage& message)
{
    const bool force_ordered =
        (message.action() == CMD_QUERY_ORDERED || message.action() == CMD_BROKER_QUERY_ORDERED);
    ActionMessage queryRep(force_ordered ? CMD_QUERY_REPLY_ORDERED : CMD_QUERY_REPLY);
    queryRep.source_id = global_broker_id_local;
    queryRep.dest_id = message.source_id;
    queryRep.messageID = message.messageID;
    queryRep.payload = generateQueryAnswer(message.payload.to_string(), force_ordered);
    queryRep.counter = message.counter;
    if (queryRep.payload.to_string() == "#wait") {
        if (queryRep.dest_id == global_broker_id_local) {
            if (queryTimeouts.empty()) {
                setTickForwarding(TickForwardingReasons::QUERY_TIMEOUT, true);
            }
            queryTimeouts.emplace_back(queryRep.messageID, std::chrono::steady_clock::now());
        }
        std::get<1>(mapBuilders[mapIndex.at(message.payload.to_string()).first])
            .push_back(queryRep);
    } else if (queryRep.dest_id == global_broker_id_local) {
        activeQueries.setDelayedValue(message.messageID, std::string(queryRep.payload.to_string()));
    } else {
        routeMessage(std::move(queryRep), message.source_id);
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
            (fed.state >= ConnectionState::CONNECTED && fed.state <= ConnectionState::OPERATING) ?
            "true" :
            "false";
    } else if (query == "state") {
        response.push_back('"');
        response.append(stateString(fed.state));
        response.push_back('"');
    } else if (query == "isinit") {
        if (fed.state >= ConnectionState::OPERATING) {
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
            (brk.state >= ConnectionState::CONNECTED && brk.state <= ConnectionState::OPERATING) ?
            "true" :
            "false";
    } else if (query == "state") {
        response = stateString(brk.state);
    } else if (query == "isinit") {
        if (brk.state >= ConnectionState::OPERATING) {
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

void CoreBroker::processQuery(ActionMessage& message)
{
    const bool force_ordered =
        (message.action() == CMD_QUERY_ORDERED || message.action() == CMD_BROKER_QUERY_ORDERED);
    const auto& target = message.getString(targetStringLoc);
    if ((target == getIdentifier() || target == "broker") ||
        (isRootc && (target == "root" || target == "federation"))) {
        processLocalQuery(message);
    } else if (isRootc && target == "gid_to_name") {
        ActionMessage queryResp(force_ordered ? CMD_QUERY_REPLY_ORDERED : CMD_QUERY_REPLY);
        queryResp.dest_id = message.source_id;
        queryResp.source_id = global_broker_id_local;
        queryResp.messageID = message.messageID;
        queryResp.payload = getNameList(message.payload.to_string());
        if (queryResp.dest_id == global_broker_id_local) {
            activeQueries.setDelayedValue(message.messageID,
                                          std::string(queryResp.payload.to_string()));
        } else {
            transmit(getRoute(queryResp.dest_id), queryResp);
        }
    } else if ((isRootc) && (target == "global" || target == "global_value")) {
        ActionMessage queryResp(force_ordered ? CMD_QUERY_REPLY_ORDERED : CMD_QUERY_REPLY);
        queryResp.dest_id = message.source_id;
        queryResp.source_id = global_broker_id_local;
        queryResp.messageID = message.messageID;

        auto gfind = global_values.find(std::string(message.payload.to_string()));
        if (gfind != global_values.end()) {
            if (target == "global_value") {
                queryResp.payload = gfind->second;
            } else {
                nlohmann::json json;
                json["name"] = std::string(message.payload.to_string());
                json["value"] = gfind->second;
                queryResp.payload = fileops::generateJsonString(json);
            }
        } else if (message.payload.to_string() == "list") {
            queryResp.payload = generateStringVector(global_values, [](const auto& globalValue) {
                return globalValue.first;
            });
        } else if (message.payload.to_string() == "all") {
            fileops::JsonMapBuilder globalSet;
            auto& json = globalSet.getJValue();
            for (auto& val : global_values) {
                json[val.first] = val.second;
            }
            queryResp.payload = globalSet.generate();
        } else {
            queryResp.payload =
                generateJsonErrorResponse(JsonErrorCodes::NOT_FOUND, "Global value not found");
        }
        if (queryResp.dest_id == global_broker_id_local) {
            activeQueries.setDelayedValue(message.messageID,
                                          std::string(queryResp.payload.to_string()));
        } else {
            transmit(getRoute(queryResp.dest_id), queryResp);
        }
    } else {
        route_id route = parent_route_id;
        auto fed = mFederates.find(target);
        std::string response;
        if (fed != mFederates.end()) {
            route = fed->route;
            message.dest_id = fed->parent;
            response = checkFedQuery(*fed, message.payload.to_string());
            if (response.empty() && fed->state >= ConnectionState::ERROR_STATE) {
                route = parent_route_id;
                switch (fed->state) {
                    case ConnectionState::ERROR_STATE:
                        response = generateJsonErrorResponse(JsonErrorCodes::SERVICE_UNAVAILABLE,
                                                             "federate is in error state");
                        break;
                    case ConnectionState::DISCONNECTED:
                    case ConnectionState::REQUEST_DISCONNECT:
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
                message.dest_id = broker->global_id;
                response = checkBrokerQuery(*broker, message.payload.to_string());
                if (response.empty() && broker->state >= ConnectionState::ERROR_STATE) {
                    route = parent_route_id;
                    switch (broker->state) {
                        case ConnectionState::ERROR_STATE:
                            response =
                                generateJsonErrorResponse(JsonErrorCodes::SERVICE_UNAVAILABLE,
                                                          "target broker is in error state");
                            break;
                        case ConnectionState::DISCONNECTED:
                        case ConnectionState::REQUEST_DISCONNECT:
                            response =
                                generateJsonErrorResponse(JsonErrorCodes::SERVICE_UNAVAILABLE,
                                                          "federate is disconnected");
                            break;
                        default:
                            break;
                    }
                }
            } else if (isRootc && message.payload.to_string() == "exists") {
                response = "false";
            }
        }
        if (((route == parent_route_id) && (isRootc)) || !response.empty()) {
            if (response.empty()) {
                response = generateJsonErrorResponse(JsonErrorCodes::NOT_FOUND, "query not valid");
            }
            ActionMessage queryResp(force_ordered ? CMD_QUERY_REPLY_ORDERED : CMD_QUERY_REPLY);
            queryResp.dest_id = message.source_id;
            queryResp.source_id = global_broker_id_local;
            queryResp.messageID = message.messageID;

            queryResp.payload = response;
            if (queryResp.dest_id == global_broker_id_local) {
                activeQueries.setDelayedValue(message.messageID,
                                              std::string(queryResp.payload.to_string()));
            } else {
                transmit(getRoute(queryResp.dest_id), queryResp);
            }
        } else {
            if (message.source_id == global_broker_id_local) {
                if (queryTimeouts.empty()) {
                    setTickForwarding(TickForwardingReasons::QUERY_TIMEOUT, true);
                }
                queryTimeouts.emplace_back(message.messageID, std::chrono::steady_clock::now());
            }
            transmit(route, message);
        }
    }
}

void CoreBroker::checkQueryTimeouts()
{
    if (!queryTimeouts.empty()) {
        auto ctime = std::chrono::steady_clock::now();
        for (auto& qtimeout : queryTimeouts) {
            if (activeQueries.isRecognized(qtimeout.first) &&
                !activeQueries.isCompleted(qtimeout.first)) {
                if (Time(ctime - qtimeout.second) > queryTimeout) {
                    activeQueries.setDelayedValue(
                        qtimeout.first,
                        generateJsonErrorResponse(JsonErrorCodes::GATEWAY_TIMEOUT,
                                                  "query timeout"));
                    qtimeout.first = 0;
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

void CoreBroker::processQueryResponse(const ActionMessage& message)
{
    if (message.counter == GENERAL_QUERY) {
        activeQueries.setDelayedValue(message.messageID, std::string(message.payload.to_string()));
        return;
    }
    if (isValidIndex(message.counter, mapBuilders)) {
        auto& builder = std::get<0>(mapBuilders[message.counter]);
        auto& requesters = std::get<1>(mapBuilders[message.counter]);
        if (builder.addComponent(std::string(message.payload.to_string()), message.messageID)) {
            std::string str;
            switch (message.counter) {
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

            for (int ii = 0; ii < static_cast<int>(requesters.size()) - 1; ++ii) {
                if (requesters[ii].dest_id == global_broker_id_local) {
                    activeQueries.setDelayedValue(requesters[ii].messageID, str);
                } else {
                    requesters[ii].payload = str;
                    routeMessage(std::move(requesters[ii]));
                }
            }
            if (requesters.back().dest_id == global_broker_id_local) {
                activeQueries.setDelayedValue(requesters.back().messageID, std::move(str));
            } else {
                requesters.back().payload = std::move(str);
                routeMessage(std::move(requesters.back()));
            }

            requesters.clear();
            if (std::get<2>(mapBuilders[message.counter]) == QueryReuse::DISABLED) {
                builder.reset();
            } else {
                builder.setCounterCode(generateMapObjectCounter());
            }
        }
    }
}

void CoreBroker::processLocalCommandInstruction(ActionMessage& message)
{
    auto [processed, res] = processBaseCommands(message);
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
                mTimeMonitorPeriod = loadTimeFromString(res[2], time_units::sec);
                loadTimeMonitor(false, res[1]);
                break;
            case 4:
            default:
                mTimeMonitorPeriod =
                    loadTimeFromString(gmlc::utilities::string_viewOps::merge(res[2], res[3]),
                                       time_units::sec);
                loadTimeMonitor(false, res[1]);
                break;
        }
    } else if ((res[0] == "set") && (res.size() > 2 && res[1] == "barrier")) {
        ActionMessage barrier(CMD_TIME_BARRIER);
        barrier.actionTime = gmlc::utilities::numeric_conversionComplete<double>(res[2], 0.0);
        if (res.size() >= 4) {
            barrier.messageID =
                gmlc::utilities::numeric_conversionComplete<std::int32_t>(res[3], 0);
        }

        generateTimeBarrier(barrier);
    } else if ((res[0] == "clear") && (res.size() >= 2 && res[1] == "barrier")) {
        ActionMessage barrier(CMD_TIME_BARRIER_CLEAR);
        setActionFlag(barrier, cancel_flag);
        if (res.size() >= 3) {
            barrier.messageID =
                gmlc::utilities::numeric_conversionComplete<std::int32_t>(res[2], 0);
        }
        generateTimeBarrier(barrier);
    } else {
        auto warnString = fmt::format(" unrecognized command instruction \"{}\"", res[0]);
        LOG_WARNING(global_broker_id_local, getIdentifier(), warnString);
        if (message.source_id != global_broker_id_local) {
            ActionMessage warn(CMD_WARNING, global_broker_id_local, message.source_id);
            warn.payload = std::move(warnString);
            warn.messageID = HELICS_LOG_LEVEL_WARNING;
            warn.setString(0, getIdentifier());
            routeMessage(warn);
        }
    }
}

void CoreBroker::processCommandInstruction(ActionMessage& message)
{
    if (message.dest_id == global_broker_id_local) {
        processLocalCommandInstruction(message);
    } else if (message.dest_id == parent_broker_id) {
        const auto& target = message.getString(targetStringLoc);
        if (target == "broker" || target == getIdentifier()) {
            processLocalCommandInstruction(message);
        } else if (isRootc) {
            if (target == "federation" || target == "root") {
                processLocalCommandInstruction(message);
            } else {
                route_id route = parent_route_id;
                auto fed = mFederates.find(target);
                if (fed != mFederates.end()) {
                    route = fed->route;
                    message.dest_id = fed->global_id;
                    transmit(route, std::move(message));
                } else {
                    auto broker = mBrokers.find(target);
                    if (broker != mBrokers.end()) {
                        route = broker->route;
                        message.dest_id = broker->global_id;
                        transmit(route, std::move(message));
                    } else {
                        message.swapSourceDest();
                        message.source_id = global_broker_id_local;
                        message.setAction(CMD_ERROR);
                        message.payload = "unable to locate target for command";
                        transmit(getRoute(message.dest_id), std::move(message));
                    }
                }
            }
        } else {
            route_id route = parent_route_id;
            auto fed = mFederates.find(target);
            if (fed != mFederates.end()) {
                route = fed->route;
                message.dest_id = fed->global_id;
                transmit(route, std::move(message));
            } else {
                auto broker = mBrokers.find(target);
                if (broker != mBrokers.end()) {
                    route = broker->route;
                    message.dest_id = broker->global_id;
                    transmit(route, std::move(message));
                } else {
                    transmit(parent_route_id, std::move(message));
                }
            }
        }
    } else {
        transmit(getRoute(message.dest_id), std::move(message));
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

ConnectionState CoreBroker::getAllConnectionState() const
{
    ConnectionState res = ConnectionState::DISCONNECTED;
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
    return (cnt > 0) ? res : ConnectionState::CONNECTED;
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
    const bool initReady = (getAllConnectionState() >= ConnectionState::INIT_REQUESTED);
    if (initReady) {
        // now do a more formal count of federates as there may be non-counting ones
        return (getCountableFederates() >= minFederateCount);
    }
    return false;
}

}  // namespace helics
