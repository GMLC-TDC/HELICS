/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "TypedBrokerServer.hpp"

#include "../core/ActionMessage.hpp"
#include "../core/BrokerFactory.hpp"
#include "../network/NetworkBrokerData.hpp"
#include "spdlog/spdlog.h"

#include <utility>

namespace helics {
namespace apps {

    static ActionMessage generatePortRequestReply(const ActionMessage& /*cmd*/,
                                                  std::shared_ptr<Broker>& brk)
    {
        ActionMessage rep(CMD_PROTOCOL);
        rep.messageID = NEW_BROKER_INFORMATION;
        rep.name = brk->getIdentifier();
        auto brkptr = extractInterfaceandPortString(brk->getAddress());
        rep.setString(0, std::string("?:") + brkptr.second);
        return rep;
    }

    /** find an existing broker or start a new one*/
    static std::pair<std::shared_ptr<Broker>, bool>
        findBroker(const ActionMessage& rx, core_type ctype, int startPort)
    {
        std::string brkname;
        std::string brkinit;
        bool newbrk{false};
        const auto& strs = rx.getStringData();
        if (!strs.empty()) {
            brkname = strs[0];
        }
        if (strs.size() > 1) {
            brkinit = strs[1] + " --external --localport=" + std::to_string(startPort);
        } else {
            brkinit = "--external --localport=" + std::to_string(startPort);
        }
        std::shared_ptr<Broker> brk;
        if (brkname.empty()) {
            brk = BrokerFactory::findJoinableBrokerOfType(ctype);
            if (!brk) {
                brk = BrokerFactory::create(ctype, brkinit);
                newbrk = true;
                brk->connect();
            }
        } else {
            brk = BrokerFactory::findBroker(brkname);
            if (!brk) {
                brk = BrokerFactory::create(ctype, brkname, brkinit);
                newbrk = true;
                brk->connect();
            }
        }
        return {brk, newbrk};
    }

    ActionMessage TypedBrokerServer::generateMessageResponse(const ActionMessage& rxcmd,
                                                             portData& pdata,
                                                             core_type ctype)
    {
        //   std::cout << "received data length " << msg.size () << std::endl;
        switch (rxcmd.action()) {
            case CMD_PROTOCOL:
            case CMD_PROTOCOL_PRIORITY:
            case CMD_PROTOCOL_BIG:
                switch (rxcmd.messageID) {
                    case REQUEST_PORTS:
                    case CONNECTION_INFORMATION: {
                        auto pt = getOpenPort(pdata);
                        if (pt > 0) {
                            auto nbrk = findBroker(rxcmd, ctype, pt);
                            if (nbrk.second) {
                                assignPort(pdata, pt, nbrk.first);
                            }
                            return generatePortRequestReply(rxcmd, nbrk.first);
                        }
                        ActionMessage rep(CMD_PROTOCOL);
                        rep.messageID = DELAY_CONNECTION;
                        return rep;
                    } break;
                }
                break;
            default:
                // std::cout << "received unknown message " << msg.size() << std::endl;
                // repSocket.send("ignored");
                break;
        }
        return CMD_IGNORE;
    }

    void TypedBrokerServer::processArgs(const std::string& /*unused*/) {}

    /** get an open port for broker to start*/
    int TypedBrokerServer::getOpenPort(portData& pd)
    {
        for (auto& pdi : pd) {
            if (!std::get<1>(pdi)) {
                return std::get<0>(pdi);
            }
        }
        for (auto& pdi : pd) {
            if (!std::get<2>(pdi)->isConnected()) {
                std::get<2>(pdi) = nullptr;
                std::get<1>(pdi) = false;
                return std::get<0>(pdi);
            }
        }
        return -1;
    }

    void TypedBrokerServer::assignPort(portData& pd, int pnumber, std::shared_ptr<Broker>& brk)
    {
        for (auto& pdi : pd) {
            if (std::get<0>(pdi) == pnumber) {
                std::get<1>(pdi) = true;
                std::get<2>(pdi) = brk;
                break;
            }
        }
    }

    void TypedBrokerServer::logMessage(const std::string& message) { spdlog::info(message); }
}  // namespace apps
}  // namespace helics
