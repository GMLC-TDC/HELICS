/*
Copyright (c) 2017-2025,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "CommsInterface.hpp"

#include "../core/core-exceptions.hpp"
#include "NetworkBrokerData.hpp"
#include "gmlc/utilities/stringOps.h"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

namespace helics {

namespace {
    bool debugCleanupEnabled()
    {
        static const bool enabled = []() {
            const auto* env = std::getenv("HELICS_DEBUG_CLEANUP");
            return env != nullptr && env[0] != '\0' && env[0] != '0';
        }();
        return enabled;
    }

    void cleanupTrace(std::string_view commName, std::string_view stage, int rx, int tx)
    {
        if (!debugCleanupEnabled()) {
            return;
        }
        std::cerr << "[helics-cleanup][comms] " << commName << ' ' << stage << " rx=" << rx
                  << " tx=" << tx << " tid=" << std::this_thread::get_id() << '\n';
    }

    /*** class to hold the set of builders for comm interfaces
       @details this doesn't work as a global since it tends to get initialized after some of the
       things that call it so it needs to be a static member of function call*/
    class MasterCommBuilder {
      public:
        using BuilderData = std::tuple<int, std::string, std::shared_ptr<CommFactory::CommBuilder>>;

        static void addBuilder(std::shared_ptr<CommFactory::CommBuilder> builder,
                               std::string_view name,
                               int code)
        {
            instance()->builders.emplace_back(code, name, std::move(builder));
        }
        static const std::shared_ptr<CommFactory::CommBuilder>& getBuilder(int code)
        {
            for (auto& builder : instance()->builders) {
                if (std::get<0>(builder) == code) {
                    return std::get<2>(builder);
                }
            }
            throw(HelicsException("comm type is not available"));
        }

        static const std::shared_ptr<CommFactory::CommBuilder>& getBuilder(std::string_view type)
        {
            for (auto& builder : instance()->builders) {
                if (std::get<1>(builder) == type) {
                    return std::get<2>(builder);
                }
            }
            throw(HelicsException("comm type is not available"));
        }
        static const std::shared_ptr<CommFactory::CommBuilder>& getIndexedBuilder(std::size_t index)
        {
            const auto& blder = instance();
            if (blder->builders.size() <= index) {
                throw(HelicsException("comm type index is not available"));
            }
            return std::get<2>(blder->builders[index]);
        }
        static const std::shared_ptr<MasterCommBuilder>& instance()
        {
            static const std::shared_ptr<MasterCommBuilder> iptr(new MasterCommBuilder());
            return iptr;
        }

      private:
        /** private constructor since we only really want one of them
        accessed through the instance static member*/
        MasterCommBuilder() = default;
        std::vector<BuilderData> builders;  //!< container for the different builders
    };
}  // namespace

namespace CommFactory {

    void defineCommBuilder(std::shared_ptr<CommBuilder> builder, std::string_view name, int code)
    {
        MasterCommBuilder::addBuilder(std::move(builder), name, code);
    }

    std::unique_ptr<CommsInterface> create(CoreType type)
    {
        const auto& builder = MasterCommBuilder::getBuilder(static_cast<int>(type));
        return builder->build();
    }

    std::unique_ptr<CommsInterface> create(std::string_view type)
    {
        const auto& builder = MasterCommBuilder::getBuilder(type);
        return builder->build();
    }

}  // namespace CommFactory

CommsInterface::CommsInterface(thread_generation threads):
    singleThread(threads == thread_generation::single)
{
}

/** destructor*/
CommsInterface::~CommsInterface()
{
    join_tx_rx_thread();
}

void CommsInterface::loadNetworkInfo(const NetworkBrokerData& netInfo)
{
    if (propertyLock()) {
        localTargetAddress = netInfo.localInterface;
        brokerTargetAddress = netInfo.brokerAddress;
        brokerName = netInfo.brokerName;
        interfaceNetwork = netInfo.interfaceNetwork;
        maxMessageSize = netInfo.maxMessageSize;
        maxMessageCount = netInfo.maxMessageCount;
        brokerInitString = netInfo.brokerInitString;
        autoBroker = netInfo.autobroker;
        observer = netInfo.observer;
        switch (netInfo.server_mode) {
            case NetworkBrokerData::ServerModeOptions::SERVER_ACTIVE:
            case NetworkBrokerData::ServerModeOptions::SERVER_DEFAULT_ACTIVE:
                serverMode = true;
                break;
            case NetworkBrokerData::ServerModeOptions::SERVER_DEACTIVATED:
            case NetworkBrokerData::ServerModeOptions::SERVER_DEFAULT_DEACTIVATED:
                serverMode = false;
                break;
            case NetworkBrokerData::ServerModeOptions::UNSPECIFIED:
                break;
        }
        if (mRequireBrokerConnection) {
            if (brokerTargetAddress.empty() && !netInfo.connectionAddress.empty()) {
                brokerTargetAddress = netInfo.connectionAddress;
            }
        } else {
            if (localTargetAddress.empty() && !netInfo.connectionAddress.empty()) {
                localTargetAddress = netInfo.connectionAddress;
            }
        }
        propertyUnLock();
    }
}

void CommsInterface::loadTargetInfo(std::string_view localTarget,
                                    std::string_view brokerTarget,
                                    gmlc::networking::InterfaceNetworks targetNetwork)
{
    if (propertyLock()) {
        localTargetAddress = localTarget;
        brokerTargetAddress = brokerTarget;
        interfaceNetwork = targetNetwork;
        propertyUnLock();
    }
}

bool CommsInterface::propertyLock()
{
    bool exp = false;
    while (!operating.compare_exchange_weak(exp, true)) {
        if (txStatus != ConnectionStatus::STARTUP) {
            return false;
        }
    }
    return true;
}

void CommsInterface::propertyUnLock()
{
    bool exp = true;
    operating.compare_exchange_strong(exp, false);
}

void CommsInterface::transmit(route_id rid, const ActionMessage& cmd)
{
    if (isPriorityCommand(cmd)) {
        txQueue.emplacePriority(rid, cmd);
    } else {
        txQueue.emplace(rid, cmd);
    }
}

void CommsInterface::transmit(route_id rid, ActionMessage&& cmd)
{
    if (isPriorityCommand(cmd)) {
        txQueue.emplacePriority(rid, std::move(cmd));
    } else {
        txQueue.emplace(rid, std::move(cmd));
    }
}

void CommsInterface::addRoute(route_id rid, std::string_view routeInfo)
{
    ActionMessage route(CMD_PROTOCOL_PRIORITY);
    route.payload = routeInfo;
    route.messageID = NEW_ROUTE;
    route.setExtraData(rid.baseValue());
    transmit(control_route, std::move(route));
}

void CommsInterface::removeRoute(route_id rid)
{
    ActionMessage route(CMD_PROTOCOL);
    route.messageID = REMOVE_ROUTE;
    route.setExtraData(rid.baseValue());
    transmit(control_route, route);
}

void CommsInterface::setTxStatus(ConnectionStatus status)
{
    if (txStatus == status) {
        return;
    }
    switch (status) {
        case ConnectionStatus::CONNECTED:
            if (txStatus == ConnectionStatus::STARTUP) {
                txStatus = status;
                (void)txTrigger.activate();
            }
            break;
        case ConnectionStatus::TERMINATED:
        case ConnectionStatus::ERRORED:
            if (txStatus == ConnectionStatus::STARTUP) {
                txStatus = status;
                (void)txTrigger.activate();
                (void)txTrigger.trigger();
            } else {
                txStatus = status;
                (void)txTrigger.trigger();
            }
            break;
        default:
            txStatus = status;
    }
}

void CommsInterface::setRxStatus(ConnectionStatus status)
{
    if (rxStatus == status) {
        return;
    }
    switch (status) {
        case ConnectionStatus::CONNECTED:
            if (rxStatus == ConnectionStatus::STARTUP) {
                rxStatus = status;
                (void)rxTrigger.activate();
            }
            break;
        case ConnectionStatus::TERMINATED:
        case ConnectionStatus::ERRORED:
            if (rxStatus == ConnectionStatus::STARTUP) {
                rxStatus = status;
                (void)rxTrigger.activate();
                (void)rxTrigger.trigger();
            } else {
                rxStatus = status;
                (void)rxTrigger.trigger();
            }

            break;
        default:
            rxStatus = status;
    }
}

bool CommsInterface::connect()
{
    if (isConnected()) {
        return true;
    }
    if (rxStatus != ConnectionStatus::STARTUP) {
        return false;
    }
    if (txStatus != ConnectionStatus::STARTUP) {
        return false;
    }
    // bool exp = false;
    if (!ActionCallback) {
        logError("no callback specified, the receiver cannot start");
        return false;
    }
    if (!propertyLock()) {
        // this will lock all the properties and should not be unlocked;
        return isConnected();
    }
    std::unique_lock<std::mutex> syncLock(threadSyncLock);
    if (name.empty()) {
        name = localTargetAddress;
    }
    if (localTargetAddress.empty()) {
        localTargetAddress = name;
    }
    if (randomID.empty()) {
        randomID = gmlc::utilities::randomString(10);
    }
    if (!singleThread) {
        queue_watcher = std::thread([this] {
            try {
                queue_rx_function();
            }
            catch (const std::exception& e) {
                rxStatus = ConnectionStatus::ERRORED;
                logError(std::string("error in receiver >") + e.what());
            }
        });
    }

    queue_transmitter = std::thread([this] {
        try {
            queue_tx_function();
        }
        catch (const std::exception& e) {
            txStatus = ConnectionStatus::ERRORED;
            logError(std::string("error in transmitter >") + e.what());
        }
    });
    syncLock.unlock();
    txTrigger.waitActivation();
    rxTrigger.waitActivation();
    if (rxStatus != ConnectionStatus::CONNECTED) {
        if (!requestDisconnect.load()) {
            logError("receiver connection failure");
        }

        if (txStatus == ConnectionStatus::CONNECTED) {
            syncLock.lock();
            if (queue_transmitter.joinable()) {
                syncLock.unlock();
                closeTransmitter();
                syncLock.lock();
                if (queue_transmitter.joinable()) {
                    queue_transmitter.join();
                }
            }
            syncLock.unlock();
        }
        if (!singleThread) {
            syncLock.lock();
            if (queue_watcher.joinable()) {
                queue_watcher.join();
            }
        }
        return false;
    }

    if (txStatus != ConnectionStatus::CONNECTED) {
        if (!requestDisconnect.load()) {
            logError("transmitter connection failure");
        }
        if (!singleThread) {
            if (rxStatus == ConnectionStatus::CONNECTED) {
                syncLock.lock();
                if (queue_watcher.joinable()) {
                    syncLock.unlock();
                    closeReceiver();
                    syncLock.lock();
                    if (queue_watcher.joinable()) {
                        queue_watcher.join();
                    }
                }
                syncLock.unlock();
            }
        }
        syncLock.lock();
        if (queue_transmitter.joinable()) {
            queue_transmitter.join();
        }
        return false;
    }
    return true;
}

void CommsInterface::setRequireBrokerConnection(bool requireBrokerConnection)
{
    if (propertyLock()) {
        mRequireBrokerConnection = requireBrokerConnection;
        propertyUnLock();
    }
}

void CommsInterface::setName(const std::string& commName)
{
    if (propertyLock()) {
        name = commName;
        propertyUnLock();
    }
}

void CommsInterface::disconnect()
{
    cleanupTrace(name,
                 "disconnect start",
                 static_cast<int>(rxStatus.load()),
                 static_cast<int>(txStatus.load()));
    if (!operating) {
        if (propertyLock()) {
            setRxStatus(ConnectionStatus::TERMINATED);
            setTxStatus(ConnectionStatus::TERMINATED);
            propertyUnLock();
            join_tx_rx_thread();
            cleanupTrace(name,
                         "disconnect early return",
                         static_cast<int>(rxStatus.load()),
                         static_cast<int>(txStatus.load()));
            return;
        }
    }
    requestDisconnect.store(true, std::memory_order_release);

    if (rxStatus.load() <= ConnectionStatus::CONNECTED) {
        closeReceiver();
    }
    if (txStatus.load() <= ConnectionStatus::CONNECTED) {
        closeTransmitter();
    }
    if (tripDetector.isTripped()) {
        setRxStatus(ConnectionStatus::TERMINATED);
        setTxStatus(ConnectionStatus::TERMINATED);
        join_tx_rx_thread();
        cleanupTrace(name,
                     "disconnect trip return",
                     static_cast<int>(rxStatus.load()),
                     static_cast<int>(txStatus.load()));
        return;
    }
    int cnt = 0;
    while (rxStatus.load() <= ConnectionStatus::CONNECTED) {
        if (rxTrigger.wait_for(std::chrono::milliseconds(800))) {
            continue;
        }
        ++cnt;
        if ((cnt % 4) == 0)  // call this every 2400 milliseconds
        {
            // try calling closeReceiver again
            closeReceiver();
        }
        if (cnt == 14)  // Eventually give up
        {
            logError("unable to terminate receiver connection");
            break;
        }
        // check the trip detector
        if (tripDetector.isTripped()) {
            rxStatus = ConnectionStatus::TERMINATED;
            txStatus = ConnectionStatus::TERMINATED;
            join_tx_rx_thread();
            cleanupTrace(name,
                         "disconnect rx trip return",
                         static_cast<int>(rxStatus.load()),
                         static_cast<int>(txStatus.load()));
            return;
        }
    }
    cnt = 0;
    while (txStatus.load() <= ConnectionStatus::CONNECTED) {
        if (txTrigger.wait_for(std::chrono::milliseconds(800))) {
            continue;
        }
        ++cnt;
        if ((cnt % 4) == 0)  // call this every 2400 milliseconds
        {
            // try calling closeTransmitter again
            closeTransmitter();
        }
        if (cnt == 14)  // Eventually give up
        {
            logError("unable to terminate transmit connection");
            break;
        }
        // check the trip detector
        if (tripDetector.isTripped()) {
            rxStatus = ConnectionStatus::TERMINATED;
            txStatus = ConnectionStatus::TERMINATED;
            join_tx_rx_thread();
            cleanupTrace(name,
                         "disconnect tx trip return",
                         static_cast<int>(rxStatus.load()),
                         static_cast<int>(txStatus.load()));
            return;
        }
    }
    join_tx_rx_thread();
    cleanupTrace(name,
                 "disconnect complete",
                 static_cast<int>(rxStatus.load()),
                 static_cast<int>(txStatus.load()));
}

void CommsInterface::join_tx_rx_thread()
{
    cleanupTrace(name,
                 "join start",
                 static_cast<int>(rxStatus.load()),
                 static_cast<int>(txStatus.load()));
    const std::scoped_lock<std::mutex> syncLock(threadSyncLock);
    if (!singleThread) {
        if (queue_watcher.joinable()) {
            queue_watcher.join();
        }
    }
    if (queue_transmitter.joinable()) {
        queue_transmitter.join();
    }
    cleanupTrace(name,
                 "join complete",
                 static_cast<int>(rxStatus.load()),
                 static_cast<int>(txStatus.load()));
}

bool CommsInterface::reconnect()
{
    rxStatus = ConnectionStatus::RECONNECTING;
    txStatus = ConnectionStatus::RECONNECTING;
    reconnectReceiver();
    reconnectTransmitter();
    int cnt = 0;
    while (rxStatus.load() == ConnectionStatus::RECONNECTING) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        ++cnt;
        if (cnt == 400)  // Eventually give up
        {
            logError("unable to reconnect");
            break;
        }
    }
    cnt = 0;
    while (txStatus.load() == ConnectionStatus::RECONNECTING) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        ++cnt;
        if (cnt == 400) {
            logError("unable to reconnect");
            break;
        }
    }

    return ((rxStatus.load() == ConnectionStatus::CONNECTED) &&
            (txStatus.load() == ConnectionStatus::CONNECTED));
}

void CommsInterface::setCallback(std::function<void(ActionMessage&&)> callback)
{
    if (propertyLock()) {
        ActionCallback = std::move(callback);
        propertyUnLock();
    }
}

void CommsInterface::setLoggingCallback(
    std::function<void(int level, std::string_view name, std::string_view message)> callback)
{
    if (propertyLock()) {
        loggingCallback = std::move(callback);
        propertyUnLock();
    }
}

void CommsInterface::setMessageSize(int maxMsgSize, int maxCount)
{
    if (propertyLock()) {
        if (maxMsgSize > 0) {
            maxMessageSize = maxMsgSize;
        }
        if (maxCount > 0) {
            maxMessageCount = maxCount;
        }
        propertyUnLock();
    }
}

void CommsInterface::setFlag(std::string_view flag, bool val)
{
    if (flag == "server_mode") {
        setServerMode(val);
    } else {
        std::string message("unrecognized flag :");
        message.append(flag);
        logWarning(message);
    }
}

void CommsInterface::setTimeout(std::chrono::milliseconds timeOut)
{
    if (propertyLock()) {
        connectionTimeout = timeOut;
        propertyUnLock();
    }
}

void CommsInterface::setServerMode(bool serverActive)
{
    if (propertyLock()) {
        serverMode = serverActive;
        propertyUnLock();
    }
}

bool CommsInterface::isConnected() const
{
    return ((txStatus == ConnectionStatus::CONNECTED) && (rxStatus == ConnectionStatus::CONNECTED));
}

void CommsInterface::logMessage(std::string_view message) const
{
    if (loggingCallback) {
        loggingCallback(HELICS_LOG_LEVEL_INTERFACES, "commMessage||" + name, message);
    } else {
        std::cout << "commMessage||" << name << ":" << message << '\n';
    }
}

void CommsInterface::logWarning(std::string_view message) const
{
    if (loggingCallback) {
        loggingCallback(HELICS_LOG_LEVEL_WARNING, "commWarning||" + name, message);
    } else {
        std::cerr << "commWarning||" << name << ":" << message << '\n';
    }
}

void CommsInterface::logError(std::string_view message) const
{
    if (loggingCallback) {
        loggingCallback(HELICS_LOG_LEVEL_ERROR, "commERROR||" + name, message);
    } else {
        std::cerr << "commERROR||" << name << ":" << message << '\n';
    }
}

void CommsInterface::closeTransmitter()
{
    ActionMessage close(CMD_PROTOCOL);
    close.messageID = DISCONNECT;
    transmit(control_route, close);
}

void CommsInterface::closeReceiver()
{
    ActionMessage cmd(CMD_PROTOCOL);
    cmd.messageID = CLOSE_RECEIVER;
    transmit(control_route, cmd);
}

void CommsInterface::reconnectTransmitter()
{
    ActionMessage reconnect(CMD_PROTOCOL);
    reconnect.messageID = RECONNECT_TRANSMITTER;
    transmit(control_route, reconnect);
}

void CommsInterface::reconnectReceiver()
{
    ActionMessage cmd(CMD_PROTOCOL);
    cmd.messageID = RECONNECT_RECEIVER;
    transmit(control_route, cmd);
}

}  // namespace helics
