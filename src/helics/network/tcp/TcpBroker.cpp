/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "TcpBroker.h"

#include "../../core/helicsCLI11.hpp"
#include "../NetworkBroker_impl.hpp"
#include "TcpComms.h"
#include "TcpCommsSS.h"

#include <iostream>
#include <memory>
#include <string>

namespace helics {
template class NetworkBroker<tcp::TcpComms, interface_type::tcp, static_cast<int>(core_type::TCP)>;
namespace tcp {
    TcpBrokerSS::TcpBrokerSS(bool rootBroker) noexcept: NetworkBroker(rootBroker) {}

    TcpBrokerSS::TcpBrokerSS(const std::string& broker_name): NetworkBroker(broker_name) {}

    std::shared_ptr<helicsCLI11App> TcpBrokerSS::generateCLI()
    {
        auto hApp = NetworkBroker::generateCLI();
        hApp->description("TCP Single Socket Broker arguments");
        hApp->add_option("--connections", connections, "target link connections");
        hApp->add_flag("--no_outgoing_connection",
                       no_outgoing_connections,
                       "disable outgoing connections")
            ->ignore_underscore();
        return hApp;
    }

    bool TcpBrokerSS::brokerConnect()
    {
        std::unique_lock<std::mutex> lock(dataMutex);
        if (!connections.empty()) {
            comms->addConnections(connections);
        }
        if (no_outgoing_connections) {
            comms->setFlag("allow_outgoing", false);
        }
        lock.unlock();
        return NetworkBroker::brokerConnect();
    }

}  // namespace tcp
}  // namespace helics
