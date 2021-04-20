/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

/** @file
@details function in this file are common function used between the different TCP comms */

#include <chrono>
#include <string>
class AsioContextManager;

namespace zmq {
class socket_t;
}

namespace helics {
namespace zeromq {
    static const std::chrono::milliseconds defaultPeriod(200);

    /** bind a zmq socket, with a timeout and timeout period*/
    bool bindzmqSocket(zmq::socket_t& socket,
                       const std::string& address,
                       int port,
                       std::chrono::milliseconds timeout,
                       std::chrono::milliseconds period = defaultPeriod);
    /** get the ZeroMQ version currently in use*/
    std::string getZMQVersion();
}  // namespace zeromq

}  // namespace helics
