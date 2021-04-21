/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "IpcComms.h"

#include "../../common/fmt_format.h"
#include "../../core/ActionMessage.hpp"
#include "../../core/helics_definitions.hpp"
#include "IpcQueueHelper.h"

#include <algorithm>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <cctype>
#include <map>
#include <memory>
#include <string>
#include <utility>

#define SET_TO_OPERATING 135111

using ipc_queue = boost::interprocess::message_queue;
using ipc_state = boost::interprocess::shared_memory_object;

namespace helics {
namespace ipc {
    IpcComms::IpcComms()
    {
        // override the default value for this comm system
        maxMessageCount = 256;
    }
    /** destructor*/
    IpcComms::~IpcComms() { disconnect(); }

    void IpcComms::loadNetworkInfo(const NetworkBrokerData& netInfo)
    {
        CommsInterface::loadNetworkInfo(netInfo);
        if (!propertyLock()) {
            return;
        }
        // brokerPort = netInfo.brokerPort;
        // PortNumber = netInfo.portNumber;
        if (localTargetAddress.empty()) {
            if (serverMode) {
                localTargetAddress = "_ipc_broker";
            } else {
                localTargetAddress = name;
            }
        }

        // if (PortNumber > 0)
        //{
        //    autoPortNumber = false;
        //}
        propertyUnLock();
    }

    void IpcComms::queue_rx_function()
    {
        OwnedQueue rxQueue;
        bool connected = rxQueue.connect(localTargetAddress, maxMessageCount, maxMessageSize);
        while (!connected) {
            std::this_thread::sleep_for(connectionTimeout);
            connected = rxQueue.connect(localTargetAddress, maxMessageCount, maxMessageSize);
            if (!connected) {
                disconnecting = true;
                ActionMessage err(CMD_ERROR);
                err.messageID = defs::errors::connection_failure;
                err.payload = rxQueue.getError();
                ActionCallback(std::move(err));
                setRxStatus(connection_status::error);  // the connection has failed
                rxQueue.changeState(queue_state_t::closing);
                return;
            }
        }
        setRxStatus(
            connection_status::connected);  // this is a atomic indicator that the rx queue is ready
        bool IPCoperating = false;
        while (true) {
            auto bc = ipcbackchannel.load();

            switch (bc) {
                case IPC_BACKCHANNEL_DISCONNECT:
                    ipcbackchannel = 0;
                    goto DISCONNECT_RX_QUEUE;
                case IPC_BACKCHANNEL_TRY_RESET:
                    connected =
                        rxQueue.connect(localTargetAddress, maxMessageCount, maxMessageSize);
                    if (!connected) {
                        disconnecting = true;
                        ActionMessage err(CMD_ERROR);
                        err.messageID = defs::errors::connection_failure;
                        err.payload = rxQueue.getError();
                        ActionCallback(std::move(err));
                        setRxStatus(connection_status::error);  // the connection has failed
                        rxQueue.changeState(queue_state_t::closing);
                        ipcbackchannel = 0;
                        return;
                    }
                    ipcbackchannel = 0;
                    break;
                default:
                    break;
            }
            auto cmdopt = rxQueue.getMessage(2000);
            if (!cmdopt) {
                continue;
            }
            if (isProtocolCommand(*cmdopt)) {
                if (cmdopt->messageID == CLOSE_RECEIVER) {
                    disconnecting = true;
                    break;
                }
                if (cmdopt->messageID == SET_TO_OPERATING) {
                    if (!IPCoperating) {
                        rxQueue.changeState(queue_state_t::operating);
                        IPCoperating = true;
                    }
                }
                continue;
            }
            if (cmdopt->action() == CMD_INIT_GRANT) {
                if (!IPCoperating) {
                    rxQueue.changeState(queue_state_t::operating);
                    IPCoperating = true;
                }
            }
            ActionCallback(std::move(*cmdopt));
        }
    DISCONNECT_RX_QUEUE:
        try {
            rxQueue.changeState(queue_state_t::closing);
        }
        catch (boost::interprocess::interprocess_exception const& ipe) {
            logError(std::string("error changing states:") + ipe.what());
        }
        setRxStatus(connection_status::terminated);
    }

    void IpcComms::queue_tx_function()
    {
        SendToQueue brokerQueue;  //!< the queue of the broker
        SendToQueue rxQueue;
        std::map<route_id, SendToQueue> routes;  //!< table of the routes to other brokers
        bool hasBroker = false;

        if (!brokerTargetAddress.empty()) {
            bool conn = brokerQueue.connect(brokerTargetAddress, true, 20);
            if (!conn) {
                std::this_thread::sleep_for(connectionTimeout);
                conn = brokerQueue.connect(brokerTargetAddress, true, 20);
                if (!conn) {
                    ActionMessage err(CMD_ERROR);
                    err.payload = fmt::format("Unable to open broker connection -> {}",
                                              brokerQueue.getError());
                    err.messageID = defs::errors::connection_failure;
                    ActionCallback(std::move(err));
                    setTxStatus(connection_status::error);
                    return;
                }
            }
            hasBroker = true;
        }
        // wait for the receiver to startup
        if (!rxTrigger.wait_forActivation(connectionTimeout)) {
            ActionMessage err(CMD_ERROR);
            err.messageID = defs::errors::connection_failure;
            err.payload = "Unable to link with receiver";
            ActionCallback(std::move(err));
            setTxStatus(connection_status::error);
            return;
        }
        if (getRxStatus() == connection_status::error) {
            setTxStatus(connection_status::error);
            return;
        }
        bool conn = rxQueue.connect(localTargetAddress, false, 0);
        if (!conn) {
            /** lets try a reset of the receiver*/
            ipcbackchannel = IPC_BACKCHANNEL_TRY_RESET;
            while (ipcbackchannel != 0) {
                if (getRxStatus() != connection_status::connected) {
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            if (getRxStatus() == connection_status::connected) {
                conn = rxQueue.connect(localTargetAddress, false, 0);
            }
            if (!conn) {
                ActionMessage err(CMD_ERROR);
                err.messageID = defs::errors::connection_failure;
                err.payload =
                    fmt::format("Unable to open receiver connection -> {}", rxQueue.getError());
                ActionCallback(std::move(err));
                setRxStatus(connection_status::error);
                return;
            }
        }

        setTxStatus(connection_status::connected);
        bool IPCoperating = false;
        bool continueLoop{true};
        while (continueLoop) {
            route_id rid;
            ActionMessage cmd;
            std::tie(rid, cmd) = txQueue.pop();
            if (isProtocolCommand(cmd)) {
                if (rid == control_route) {
                    switch (cmd.messageID) {
                        case NEW_ROUTE: {
                            SendToQueue newQueue;
                            bool newQconnected = newQueue.connect(cmd.payload, false, 3);
                            if (newQconnected) {
                                routes.emplace(route_id{cmd.getExtraData()}, std::move(newQueue));
                            }
                            continue;
                        }
                        case REMOVE_ROUTE:
                            routes.erase(route_id{cmd.getExtraData()});
                            continue;
                        case DISCONNECT:
                            continueLoop = false;
                            continue;
                    }
                }
            }
            if (cmd.action() == CMD_INIT_GRANT) {
                if (!IPCoperating) {
                    ActionMessage op(CMD_PROTOCOL);
                    op.messageID = SET_TO_OPERATING;
                    rxQueue.sendMessage(op, 3);
                }
            }
            std::string buffer = cmd.to_string();
            int priority = isPriorityCommand(cmd) ? 3 : 1;
            if (rid == parent_route_id) {
                if (hasBroker) {
                    brokerQueue.sendMessage(cmd, priority);
                }
            } else if (rid == control_route) {
                rxQueue.sendMessage(cmd, priority);
            } else {
                auto routeFnd = routes.find(rid);
                if (routeFnd != routes.end()) {
                    routeFnd->second.sendMessage(cmd, priority);
                } else {
                    if (hasBroker) {
                        brokerQueue.sendMessage(cmd, priority);
                    }
                }
            }
        }
        setTxStatus(connection_status::terminated);
    }

    void IpcComms::closeReceiver()
    {
        if ((getRxStatus() == connection_status::error) ||
            (getRxStatus() == connection_status::terminated)) {
            return;
        }
        ActionMessage cmd(CMD_PROTOCOL);
        cmd.messageID = CLOSE_RECEIVER;
        if (getTxStatus() == connection_status::connected) {
            transmit(control_route, cmd);
        } else if (!disconnecting) {
            try {
                auto rxQueue = std::make_unique<ipc_queue>(
                    boost::interprocess::open_only,
                    stringTranslateToCppName(localTargetAddress).c_str());
                std::string buffer = cmd.to_string();
                rxQueue->send(buffer.data(), buffer.size(), 3);
            }
            catch (boost::interprocess::interprocess_exception const&) {
                if (!disconnecting) {
                    ipcbackchannel.store(IPC_BACKCHANNEL_DISCONNECT);
                    //  std::cerr << "unable to send close message::" << ipe.what () << std::endl;
                }
            }
        }
    }

    std::string IpcComms::getAddress() const { return localTargetAddress; }

}  // namespace ipc
}  // namespace helics
