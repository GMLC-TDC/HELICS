/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../core/helicsCLI11.hpp"
#include "NetworkCore.hpp"

#include <memory>
#include <string>

namespace helics {
constexpr const char* defBrokerInterface[] = {"127.0.0.1",
                                              "127.0.0.1",
                                              "tcp://127.0.0.1",
                                              "_ipc_broker",
                                              ""};
constexpr const char* defLocalInterface[] = {"127.0.0.1", "127.0.0.1", "tcp://127.0.0.1", "", ""};

template<class COMMS, InterfaceTypes baseline>
NetworkCore<COMMS, baseline>::NetworkCore() noexcept
{
    netInfo.server_mode = NetworkBrokerData::ServerModeOptions::SERVER_DEFAULT_DEACTIVATED;
}

template<class COMMS, InterfaceTypes baseline>
NetworkCore<COMMS, baseline>::NetworkCore(std::string_view coreName):
    CommsBroker<COMMS, CommonCore>(coreName)
{
    netInfo.server_mode = NetworkBrokerData::ServerModeOptions::SERVER_DEFAULT_DEACTIVATED;
}

template<class COMMS, InterfaceTypes baseline>
std::shared_ptr<helicsCLI11App> NetworkCore<COMMS, baseline>::generateCLI()
{
    auto app = CommonCore::generateCLI();
    CLI::App_p netApp =
        netInfo.commandLineParser(defLocalInterface[static_cast<int>(baseline)], false);
    app->add_subcommand(netApp);
    return app;
}

template<class COMMS, InterfaceTypes baseline>
bool NetworkCore<COMMS, baseline>::brokerConnect()
{
    std::lock_guard<std::mutex> lock(dataMutex);
    if (netInfo.brokerAddress.empty())  // cores require a broker
    {
        netInfo.brokerAddress = defBrokerInterface[static_cast<int>(baseline)];
    }
    CommsBroker<COMMS, CommonCore>::comms->setRequireBrokerConnection(true);
    netInfo.useJsonSerialization = BrokerBase::useJsonSerialization;
    netInfo.observer = BrokerBase::observer;
    CommsBroker<COMMS, CommonCore>::comms->setName(CommonCore::getIdentifier());
    CommsBroker<COMMS, CommonCore>::comms->loadNetworkInfo(netInfo);
    CommsBroker<COMMS, CommonCore>::comms->setTimeout(BrokerBase::networkTimeout.to_ms());
    auto res = CommsBroker<COMMS, CommonCore>::comms->connect();
    if (res) {
        if (netInfo.portNumber < 0) {
            netInfo.portNumber = CommsBroker<COMMS, CommonCore>::comms->getPort();
        }
    }
    return res;
}

template<class COMMS, InterfaceTypes baseline>
std::string NetworkCore<COMMS, baseline>::generateLocalAddressString() const
{
    std::string add;
    if (CommsBroker<COMMS, CommonCore>::comms->isConnected()) {
        add = CommsBroker<COMMS, CommonCore>::comms->getAddress();
    } else {
        std::lock_guard<std::mutex> lock(dataMutex);
        switch (baseline) {
            case InterfaceTypes::TCP:
            case InterfaceTypes::IP:
            case InterfaceTypes::UDP:
                if (!netInfo.localInterface.empty() && (netInfo.localInterface.back() == '*')) {
                    add = gmlc::networking::makePortAddress(
                        netInfo.localInterface.substr(0, netInfo.localInterface.size() - 1),
                        netInfo.portNumber);
                } else {
                    add = gmlc::networking::makePortAddress(netInfo.localInterface,
                                                            netInfo.portNumber);
                }
                break;
            case InterfaceTypes::INPROC:
            case InterfaceTypes::IPC:
            default:
                if (!netInfo.localInterface.empty()) {
                    add = netInfo.localInterface;
                } else {
                    add = CommonCore::getIdentifier();
                }
                break;
        }
    }
    return add;
}

}  // namespace helics
