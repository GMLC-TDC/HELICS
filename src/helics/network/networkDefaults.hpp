/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

// For ZMQ Comms
constexpr int DEFAULT_ZMQ_BROKER_PORT_NUMBER = 23404;

constexpr int BACKUP_ZMQ_BROKER_PORT_NUMBER = 23406;

constexpr int DEFAULT_ZMQSS_BROKER_PORT_NUMBER =
    23414;  // Todo define a different port number (change in HELICS 3.0)
constexpr int BACKUP_ZMQSS_BROKER_PORT_NUMBER = 23814;

// for TCP comms
constexpr int DEFAULT_TCP_BROKER_PORT_NUMBER = 24160;
constexpr int BACKUP_TCP_BROKER_PORT_NUMBER = 24161;
constexpr int DEFAULT_TCPSS_PORT = 33133;

// for UDP comms
static const int DEFAULT_UDP_BROKER_PORT_NUMBER = 23901;
static const int BACKUP_UDP_BROKER_PORT_NUMBER = 23902;
