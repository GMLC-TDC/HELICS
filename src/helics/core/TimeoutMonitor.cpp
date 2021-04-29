/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "TimeoutMonitor.h"

#include "CommonCore.hpp"
#include "CoreBroker.hpp"
#include "loggingHelper.hpp"

#include <string>
#include <utility>

namespace helics {
void TimeoutMonitor::tick(CommonCore* core)
{
    if (parentConnection.waitingForPingReply) {
        auto now = std::chrono::steady_clock::now();
        if (now - parentConnection.lastPing > timeout) {
            // try to reset the connection to the broker
            // brokerReconnect()
            const std::string message{"core lost connection with broker"};
            core->sendToLogger(core->global_broker_id_local,
                               log_level::error,
                               core->getIdentifier(),
                               message);
            core->sendErrorToFederates(-5, message);
            core->processDisconnect();
            core->brokerState = BrokerBase::broker_state_t::errored;
            core->addActionMessage(CMD_STOP);
        } else {  // ping again
            ActionMessage png(CMD_PING_PRIORITY);
            png.source_id = core->global_broker_id_local;
            png.dest_id = core->higher_broker_id;
            core->transmit(parent_route_id, png);
        }
    } else if ((core->isConnected()) && (core->global_broker_id_local.isValid()) &&
               (core->global_broker_id_local != parent_broker_id)) {
        // if (allFedWaiting())
        //{
        if (core->higher_broker_id.isValid()) {
            ActionMessage png(CMD_PING_PRIORITY);
            png.source_id = core->global_broker_id_local;
            png.dest_id = core->higher_broker_id;
            core->transmit(parent_route_id, png);
            parentConnection.lastPing = std::chrono::steady_clock::now();
            parentConnection.waitingForPingReply = true;
        }
        //}
    } else if ((core->isConnected()) &&
               ((!core->global_broker_id_local.isValid()) ||
                (core->global_broker_id_local == parent_broker_id))) {
        ActionMessage rsend(CMD_RESEND);
        rsend.messageID = static_cast<int32_t>(CMD_REG_BROKER);
        core->processCommand(std::move(rsend));
    } else if ((core->brokerState == BrokerBase::broker_state_t::terminated) ||
               (core->brokerState == BrokerBase::broker_state_t::errored)) {
        if (waitingForConnection) {
            auto now = std::chrono::steady_clock::now();
            if (now - startWaiting > timeout) {
                ActionMessage png(CMD_CHECK_CONNECTIONS);
                png.source_id = core->global_broker_id_local;
                core->addActionMessage(png);
            }
        } else {
            waitingForConnection = true;
            startWaiting = std::chrono::steady_clock::now();
        }
    } else {
        if (waitingForConnection) {
            auto now = std::chrono::steady_clock::now();
            if (now - startWaiting > timeout) {
                ActionMessage png(CMD_CHECK_CONNECTIONS);
                png.source_id = core->global_broker_id_local;
                core->addActionMessage(png);
            }
        } else {
            waitingForConnection = true;
            startWaiting = std::chrono::steady_clock::now();
        }
    }
}

void TimeoutMonitor::tick(CoreBroker* brk)
{
    // check the connections

    bool waiting = parentConnection.waitingForPingReply;
    auto now = std::chrono::steady_clock::now();

    if (!brk->isRoot() && parentConnection.waitingForPingReply) {
        if (now - parentConnection.lastPing > timeout) {
            // try to reset the connection to the broker
            // brokerReconnect()
            brk->sendToLogger(brk->global_broker_id_local,
                              log_level::error,
                              brk->getIdentifier(),
                              "broker lost connection with parent");
            brk->sendErrorToImmediateBrokers(-5);
            brk->processDisconnect();
            brk->brokerState = BrokerBase::broker_state_t::errored;
            brk->addActionMessage(CMD_STOP);
        } else {  // ping again
            ActionMessage png(CMD_PING_PRIORITY);
            png.source_id = brk->global_broker_id_local;
            png.dest_id = brk->higher_broker_id;
            brk->transmit(parent_route_id, png);
        }
    }

    for (auto& conn : connections) {
        if (conn.waitingForPingReply) {
            waiting = true;
            if (now - conn.lastPing > timeout) {
                ActionMessage cerror(CMD_CONNECTION_ERROR);
                cerror.dest_id = brk->global_broker_id_local;
                cerror.source_id = conn.connection;
                brk->addActionMessage(cerror);
            } else {  // ping again
                ActionMessage png(CMD_PING);
                png.source_id = brk->global_broker_id_local;
                png.dest_id = conn.connection;
                brk->addActionMessage(png);
            }
        }
    }
    if (!waiting) {
        if (!brk->isRoot()) {
            if ((brk->isConnected()) && (brk->global_broker_id_local.isValid()) &&
                (brk->global_broker_id_local != parent_broker_id)) {
                // if (allFedWaiting())
                //{
                if (brk->higher_broker_id.isValid()) {
                    ActionMessage png(CMD_PING_PRIORITY);
                    png.source_id = brk->global_broker_id_local;
                    png.dest_id = brk->higher_broker_id;
                    brk->transmit(parent_route_id, png);
                    parentConnection.lastPing = now;
                    parentConnection.waitingForPingReply = true;
                }
                //}
            } else if ((brk->brokerState == BrokerBase::broker_state_t::terminated) ||
                       (brk->brokerState == BrokerBase::broker_state_t::errored)) {
                if (waitingForConnection) {
                    if (now - startWaiting > timeout) {
                        ActionMessage png(CMD_CHECK_CONNECTIONS);
                        png.source_id = brk->global_broker_id_local;
                        brk->addActionMessage(png);
                    }
                } else {
                    waitingForConnection = true;
                    startWaiting = now;
                }
            } else {
                if (waitingForConnection) {
                    if (now - startWaiting > timeout) {
                        ActionMessage png(CMD_CHECK_CONNECTIONS);
                        png.source_id = brk->global_broker_id_local;
                        brk->addActionMessage(png);
                    }
                } else {
                    waitingForConnection = true;
                    startWaiting = now;
                }
            }
        } else {  // brk is a root broker
            pingSub(brk);
        }
    }
}

void TimeoutMonitor::pingSub(CoreBroker* brk)
{
    auto now = std::chrono::steady_clock::now();
    bool activePing = false;
    for (auto& brkr : brk->_brokers) {
        size_t cindex = connections.size();
        for (size_t ii = 0; ii < cindex; ++ii) {
            if (connections[ii].connection == brkr.global_id) {
                cindex = ii;
                break;
            }
        }
        if (cindex == connections.size()) {
            connections.emplace_back();
            connections[cindex].connection = brkr.global_id;
            connections[cindex].disablePing = brkr._disable_ping;
        }

        if (brkr.state < connection_state::error) {
            if (connections[cindex].disablePing) {
                continue;
            }
            connections[cindex].activeConnection = true;
            connections[cindex].waitingForPingReply = true;
            connections[cindex].lastPing = now;
            // send the ping
            ActionMessage png(brkr._core ? CMD_PING : CMD_BROKER_PING);
            png.source_id = brk->global_broker_id_local;
            png.dest_id = brkr.global_id;
            brk->transmit(brkr.route, png);
            activePing = true;
        } else {
            connections[cindex].activeConnection = false;
        }
    }
    if (activePing) {
        ActionMessage tickf(CMD_BROKER_CONFIGURE);
        tickf.dest_id = global_federate_id(brk->global_id);
        tickf.source_id = global_federate_id(brk->global_id);
        tickf.messageID = REQUEST_TICK_FORWARDING;
        setActionFlag(tickf, indicator_flag);
        brk->addActionMessage(tickf);
    }
}

void TimeoutMonitor::reset()
{
    parentConnection.waitingForPingReply = false;
    waitingForConnection = false;
    for (auto& conn : connections) {
        conn.waitingForPingReply = false;
    }
}

void TimeoutMonitor::pingReply(const ActionMessage& cmd, CoreBroker* brk)
{
    if (cmd.source_id == global_federate_id(parentConnection.connection)) {
        parentConnection.waitingForPingReply = false;
        waitingForConnection = false;
    } else {
        bool waiting{false};
        bool waswaiting{false};
        for (auto& conn : connections) {
            if (cmd.source_id == global_federate_id(conn.connection)) {
                waswaiting = conn.waitingForPingReply;
                conn.waitingForPingReply = false;
            }
            if (conn.waitingForPingReply) {
                waiting = true;
            }
        }
        if (!waiting && waswaiting) {
            if (brk != nullptr) {
                ActionMessage noforward(CMD_BROKER_CONFIGURE);
                noforward.dest_id = cmd.dest_id;
                noforward.source_id = cmd.dest_id;
                noforward.messageID = REQUEST_TICK_FORWARDING;
                brk->addActionMessage(noforward);
            }
        }
    }
}

}  // namespace helics
