/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC. See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "MpiComms.h"

#include "../../common/fmt_format.h"
#include "../../core/ActionMessage.hpp"
#include "MpiService.h"

#include <boost/scope_exit.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <utility>

namespace helics {
namespace mpi {
    MpiComms::MpiComms()
    {
        auto& mpi_service = MpiService::getInstance();
        localTargetAddress = mpi_service.addMpiComms(this);
        logMessage(fmt::format("MpiComms() - commAddress = {}", localTargetAddress));
    }

    /** destructor*/
    MpiComms::~MpiComms() { disconnect(); }

    void MpiComms::setBrokerAddress(const std::string& address)
    {
        if (propertyLock()) {
            brokerTargetAddress = address;
            propertyUnLock();
        }
    }

    int MpiComms::processIncomingMessage(ActionMessage& cmd)
    {
        if (isProtocolCommand(cmd)) {
            switch (cmd.messageID) {
                case CLOSE_RECEIVER:
                    return (-1);
                default:
                    break;
            }
        }
        ActionCallback(std::move(cmd));
        return 0;
    }

    void MpiComms::queue_rx_function()
    {
        BOOST_SCOPE_EXIT_ALL(this)
        {
            logMessage(fmt::format("Shutdown RX Loop for {}", localTargetAddress));
            shutdown = true;
            setRxStatus(connection_status::terminated);
        };
        setRxStatus(connection_status::connected);

        while (true) {
            auto M = rxMessageQueue.pop(std::chrono::milliseconds(2000));

            if (M) {
                if (!isValidCommand(M.value())) {
                    logError("invalid command received");
                    continue;
                }

                if (isProtocolCommand(M.value())) {
                    if (M->messageID == CLOSE_RECEIVER) {
                        return;
                    }
                }

                auto res = processIncomingMessage(M.value());
                if (res < 0) {
                    return;
                }
            }

            if (shutdown) {
                return;
            }
        }
    }

    void MpiComms::queue_tx_function()
    {
        setTxStatus(connection_status::connected);

        auto& mpi_service = MpiService::getInstance();

        std::map<route_id, std::pair<int, int>> routes;  // for all the other possible routes

        std::pair<int, int> brokerLocation;
        if (!brokerTargetAddress.empty()) {
            hasBroker = true;
            auto addr_delim_pos = brokerTargetAddress.find_last_of(':');
            brokerLocation.first = std::stoi(brokerTargetAddress.substr(0, addr_delim_pos));
            brokerLocation.second = std::stoi(
                brokerTargetAddress.substr(addr_delim_pos + 1, brokerTargetAddress.length()));
        }

        while (true) {
            route_id rid;
            ActionMessage cmd;

            std::tie(rid, cmd) = txQueue.pop();
            bool processed = false;
            if (isProtocolCommand(cmd)) {
                if (control_route == rid) {
                    switch (cmd.messageID) {
                        case NEW_ROUTE: {
                            // cmd.payload would be the MPI rank of the destination
                            std::pair<int, int> routeLoc;
                            auto addr_delim_pos = cmd.payload.find_last_of(':');
                            routeLoc.first = std::stoi(cmd.payload.substr(0, addr_delim_pos));
                            routeLoc.second = std::stoi(
                                cmd.payload.substr(addr_delim_pos + 1, cmd.payload.length()));

                            routes.emplace(route_id{cmd.getExtraData()}, routeLoc);
                            processed = true;
                        } break;
                        case REMOVE_ROUTE:
                            routes.erase(route_id{cmd.getExtraData()});
                            processed = true;
                            break;
                        case DISCONNECT:
                            rxMessageQueue.push(cmd);
                            goto CLOSE_TX_LOOP;  // break out of loop
                    }
                }
            }
            if (processed) {
                continue;
            }

            if (rid == parent_route_id) {
                if (hasBroker) {
                    // Send using MPI to broker
                    // std::cout << "send msg to brkr rt: " << prettyPrintString(cmd) << std::endl;
                    mpi_service.sendMessage(brokerLocation, cmd.to_vector());
                }
            } else if (rid == control_route) {  // send to rx thread loop
                // Send to ourself -- may need command line option to enable for openmpi
                // std::cout << "send msg to self" << prettyPrintString(cmd) << std::endl;
                rxMessageQueue.push(cmd);
            } else {
                auto rt_find = routes.find(rid);
                if (rt_find != routes.end()) {
                    // Send using MPI to rank given by route
                    // std::cout << "send msg to rt: " << prettyPrintString(cmd) << std::endl;
                    mpi_service.sendMessage(rt_find->second, cmd.to_vector());
                } else {
                    if (hasBroker) {
                        // Send using MPI to broker
                        // std::cout << "send msg to brkr: " << prettyPrintString(cmd) << std::endl;
                        mpi_service.sendMessage(brokerLocation, cmd.to_vector());
                    } else {
                        if (!isDisconnectCommand(cmd)) {
                            logWarning(
                                fmt::format("unknown route and no broker, dropping message {}",
                                            prettyPrintString(cmd)));
                        }
                    }
                }
            }
        }
    CLOSE_TX_LOOP:
        logMessage(std::string("Shutdown TX Loop for ") + localTargetAddress);
        routes.clear();
        if (getRxStatus() == connection_status::connected) {
            shutdown = true;
        }
        setTxStatus(connection_status::terminated);
        mpi_service.removeMpiComms(this);
    }

    void MpiComms::closeReceiver()
    {
        shutdown = true;
        ActionMessage cmd(CMD_PROTOCOL);
        cmd.messageID = CLOSE_RECEIVER;
        rxMessageQueue.push(cmd);
    }
}  // namespace mpi
}  // namespace helics
