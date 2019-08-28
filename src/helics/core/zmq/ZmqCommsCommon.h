/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

/** @file
@details function in this file are common function used between the different TCP comms */

#include "cppzmq/zmq.hpp"
#include <chrono>
class AsioContextManager;

namespace helics
{
namespace hzmq
{
/** bind a zmq socket, with a timeout and timeout period*/
bool bindzmqSocket (zmq::socket_t &socket,
                    const std::string &address,
                    int port,
                    std::chrono::milliseconds timeout,
                    std::chrono::milliseconds period = std::chrono::milliseconds (200));
}  // namespace hzmq
}  // namespace helics
