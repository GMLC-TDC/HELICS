/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "TestComms.h"

#include "../../common/fmt_format.h"
#include "../../core/ActionMessage.hpp"
#include "../../core/BrokerFactory.hpp"
#include "../../core/CommonCore.hpp"
#include "../../core/CoreBroker.hpp"
#include "../../core/CoreFactory.hpp"
#include "../NetworkBrokerData.hpp"

#include <map>
#include <memory>
#include <queue>
#include <string>
#include <utility>

namespace helics {
namespace testcore {
    TestComms::TestComms(): CommsInterface(CommsInterface::thread_generation::single) {}

    /** destructor*/
    TestComms::~TestComms() { disconnect(); }

    void TestComms::loadNetworkInfo(const NetworkBrokerData& netInfo)
    {
        CommsInterface::loadNetworkInfo(netInfo);
        if (!propertyLock()) {
            return;
        }
        // brokerPort = netInfo.brokerPort;
        // PortNumber = netInfo.portNumber;
        if (localTargetAddress.empty()) {
            localTargetAddress = name;
        }

        // if (PortNumber > 0)
        //{
        //    autoPortNumber = false;
        //}
        propertyUnLock();
    }

    void TestComms::queue_rx_function() {}

    void TestComms::queue_tx_function()
    {
        using std::chrono::milliseconds;
        bool bufferData{false};
        int32_t allowedSend{0};
        std::queue<std::pair<route_id, ActionMessage>> buffer;
        // make sure the link to the localTargetAddress is in place
        if (name != localTargetAddress) {
            if (!brokerName.empty()) {
                if (!CoreFactory::copyCoreIdentifier(name, localTargetAddress)) {
                    if (!BrokerFactory::copyBrokerIdentifier(name, localTargetAddress)) {
                        setRxStatus(connection_status::error);
                        setTxStatus(connection_status::error);
                        return;
                    }
                }
            } else {
                if (!BrokerFactory::copyBrokerIdentifier(name, localTargetAddress)) {
                    setRxStatus(connection_status::error);
                    setTxStatus(connection_status::error);
                    return;
                }
            }
        }
        setRxStatus(connection_status::connected);
        std::shared_ptr<CoreBroker> tbroker;

        if (brokerName.empty()) {
            if (!brokerTargetAddress.empty()) {
                brokerName = brokerTargetAddress;
            }
        }

        if (!brokerName.empty()) {
            milliseconds totalSleep(0);
            while (!tbroker) {
                auto broker = BrokerFactory::findBroker(brokerName);
                tbroker = std::dynamic_pointer_cast<CoreBroker>(broker);
                if (!tbroker) {
                    if (autoBroker) {
                        tbroker = std::static_pointer_cast<CoreBroker>(
                            BrokerFactory::create(core_type::TEST, brokerName, brokerInitString));
                        tbroker->connect();
                    } else {
                        if (totalSleep > connectionTimeout) {
                            setTxStatus(connection_status::error);
                            setRxStatus(connection_status::error);
                            return;
                        }
                        std::this_thread::sleep_for(milliseconds(200));
                        totalSleep += milliseconds(200);
                    }
                } else {
                    if (!tbroker->isOpenToNewFederates()) {
                        logError("broker is not open to new federates " + brokerName);
                        tbroker = nullptr;
                        broker = nullptr;
                        BrokerFactory::cleanUpBrokers(milliseconds(200));
                        totalSleep += milliseconds(200);
                        if (totalSleep > milliseconds(connectionTimeout)) {
                            setTxStatus(connection_status::error);
                            setRxStatus(connection_status::error);
                            return;
                        }
                    }
                }
            }
        } else if (!serverMode) {
            milliseconds totalSleep(0);
            while (!tbroker) {
                auto broker = BrokerFactory::findJoinableBrokerOfType(core_type::TEST);
                tbroker = std::dynamic_pointer_cast<CoreBroker>(broker);
                if (!tbroker) {
                    if (autoBroker) {
                        tbroker = std::static_pointer_cast<CoreBroker>(
                            BrokerFactory::create(core_type::TEST, "", brokerInitString));
                        tbroker->connect();
                    } else {
                        if (totalSleep > connectionTimeout) {
                            setTxStatus(connection_status::error);
                            setRxStatus(connection_status::error);
                            return;
                        }
                        std::this_thread::sleep_for(milliseconds(200));
                        totalSleep += milliseconds(200);
                    }
                }
            }
        }
        if (tbroker) {
            if (tbroker->getIdentifier() == localTargetAddress) {
                logError("broker == target");
            }
            if (!tbroker->isOpenToNewFederates()) {
                logError("broker is not open to new federates");
            }
        }

        setTxStatus(connection_status::connected);
        std::map<route_id, std::shared_ptr<BrokerBase>> routes;
        bool haltLoop{false};
        while (!haltLoop) {
            route_id rid;
            ActionMessage cmd;
            if (!buffer.empty() && (!bufferData || allowedSend > 0)) {
                auto& pr = buffer.front();
                rid = pr.first;
                cmd = std::move(pr.second);
                buffer.pop();
                --allowedSend;
            } else {
                std::tie(rid, cmd) = txQueue.pop();
            }

            bool processed = false;
            if (isProtocolCommand(cmd)) {
                if (rid == control_route) {
                    switch (cmd.messageID) {
                        case NEW_ROUTE: {
                            auto& newroute = cmd.payload;
                            bool foundRoute = false;
                            auto core = CoreFactory::findCore(newroute);
                            if (core) {
                                auto tcore = std::dynamic_pointer_cast<CommonCore>(core);
                                if (tcore) {
                                    routes.emplace(route_id{cmd.getExtraData()}, std::move(tcore));
                                    foundRoute = true;
                                }
                            }
                            if (!foundRoute) {
                                auto brk = BrokerFactory::findBroker(newroute);

                                if (brk) {
                                    auto cbrk = std::dynamic_pointer_cast<CoreBroker>(brk);
                                    if (cbrk) {
                                        routes.emplace(route_id{cmd.getExtraData()},
                                                       std::move(cbrk));
                                        foundRoute = true;
                                    }
                                }
                            }
                            if (!foundRoute) {
                                logError(std::string("unable to establish Route to ") + newroute);
                            }
                            processed = true;
                        } break;
                        case REMOVE_ROUTE:
                            routes.erase(route_id{cmd.getExtraData()});
                            processed = true;
                            break;
                        case CLOSE_RECEIVER:
                            setRxStatus(connection_status::terminated);
                            processed = true;
                            break;
                        case DISCONNECT:
                            haltLoop = true;
                            continue;
                        case PAUSE_TRANSMITTER:
                            bufferData = true;
                            allowedSend = 0;
                            continue;
                        case UNPAUSE_TRANSMITTER:
                            bufferData = false;
                            allowedSend = 0;
                            continue;
                        case ALLOW_MESSAGES:
                            allowedSend += cmd.getExtraData();
                            continue;
                    }
                }
            }
            if (processed) {
                continue;
            }
            if (bufferData) {
                if (allowedSend == 0) {
                    buffer.emplace(rid, std::move(cmd));
                    continue;
                } else {
                    --allowedSend;
                }
            }
            if (rid == parent_route_id) {
                if (tbroker) {
                    tbroker->addActionMessage(std::move(cmd));
                } else {
                    logWarning(fmt::format(
                        "message directed to broker of comm system with no broker, message dropped {}",
                        prettyPrintString(cmd)));
                }
            } else {
                auto rt_find = routes.find(rid);
                if (rt_find != routes.end()) {
                    rt_find->second->addActionMessage(std::move(cmd));
                } else {
                    if (tbroker) {
                        tbroker->addActionMessage(std::move(cmd));
                    } else {
                        if (!isDisconnectCommand(cmd)) {
                            logWarning(std::string("unknown route, message dropped ") +
                                       prettyPrintString(cmd));
                        }
                    }
                }
            }
        }  // while (!haltLoop)

        routes.clear();
        tbroker = nullptr;

        setTxStatus(connection_status::terminated);
    }

    void TestComms::haltComms()
    {
        if (getRxStatus() == connection_status::connected) {
            ActionMessage cmd(CMD_PROTOCOL);
            cmd.messageID = CLOSE_RECEIVER;
            transmit(control_route, cmd);
        }
        if (getTxStatus() == connection_status::connected) {
            ActionMessage cmd(CMD_PROTOCOL);
            cmd.messageID = DISCONNECT;
            transmit(control_route, cmd);
        }
    }

    void TestComms::pauseComms(bool paused)
    {
        if (getTxStatus() == connection_status::connected) {
            ActionMessage cmd(CMD_PROTOCOL);
            cmd.messageID = (paused) ? PAUSE_TRANSMITTER : UNPAUSE_TRANSMITTER;
            transmit(control_route, cmd);
        }
    }

    void TestComms::allowMessages(int count)
    {
        if (getTxStatus() == connection_status::connected) {
            ActionMessage cmd(CMD_PROTOCOL);
            cmd.setExtraData(count);
            transmit(control_route, cmd);
        }
    }

    std::string TestComms::getAddress() const { return localTargetAddress; }

}  // namespace testcore

}  // namespace helics
