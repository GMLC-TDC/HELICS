/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
namespace helics {

namespace network {
    // For ZMQ Comms
    constexpr int DEFAULT_ZMQ_PORT = 23404;

    constexpr int BACKUP_ZMQ_PORT = 23406;

    constexpr int DEFAULT_ZMQSS_PORT = 23414;
    constexpr int BACKUP_ZMQSS_PORT = 23814;

    // for TCP comms
    constexpr int DEFAULT_TCP_PORT = 24160;
    constexpr int BACKUP_TCP_PORT = 24161;
    constexpr int DEFAULT_TCPSS_PORT = 33133;

    // for UDP comms
    constexpr int DEFAULT_UDP_PORT = 23901;
    constexpr int BACKUP_UDP_PORT = 23902;
}  // namespace network

int getDefaultPort(int coreType);

}  // namespace helics
