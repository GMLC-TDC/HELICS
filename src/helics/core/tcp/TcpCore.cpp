/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "TcpCore.h"

#include "../NetworkCore_impl.hpp"
#include "../helicsCLI11.hpp"
#include "TcpComms.h"
#include "TcpCommsSS.h"

namespace helics {
template class NetworkCore<tcp::TcpComms, interface_type::tcp>;
namespace tcp {
    TcpCoreSS::TcpCoreSS() noexcept {}

    TcpCoreSS::TcpCoreSS(const std::string& core_name): NetworkCore(core_name) {}

    std::shared_ptr<helicsCLI11App> TcpCoreSS::generateCLI()
    {
        auto hApp = NetworkCore::generateCLI();
        hApp->description("TCP Single Socket Core ");
        hApp->add_option("--connections", connections, "target link connections");
        hApp->add_flag(
                "--no_outgoing_connection", no_outgoing_connections, "disable outgoing connections")
            ->ignore_underscore();
        return hApp;
    }

    bool TcpCoreSS::brokerConnect()
    {
        std::unique_lock<std::mutex> lock(dataMutex);
        if (!connections.empty()) {
            comms->addConnections(connections);
        }
        if (no_outgoing_connections) {
            comms->setFlag("allow_outgoing", false);
        }
        lock.unlock();
        return NetworkCore::brokerConnect();
    }

} // namespace tcp
} // namespace helics
