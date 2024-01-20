/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "networkDefaults.hpp"

#include "../helics_enums.h"

namespace helics {
int getDefaultPort(int coreType)
{
    switch (coreType) {
        case HELICS_CORE_TYPE_TCP:
            return network::DEFAULT_TCP_PORT;
        case HELICS_CORE_TYPE_TCP_SS:
            return network::DEFAULT_TCPSS_PORT;
        case HELICS_CORE_TYPE_ZMQ:
            return network::DEFAULT_ZMQ_PORT;
        case HELICS_CORE_TYPE_ZMQ_SS:
            return network::DEFAULT_ZMQSS_PORT;
        case HELICS_CORE_TYPE_UDP:
            return network::DEFAULT_UDP_PORT;
        case HELICS_CORE_TYPE_HTTP:
        case HELICS_CORE_TYPE_WEBSOCKET:
            return 80;
        default:
            return (-1);
    }
}
}  // namespace helics
