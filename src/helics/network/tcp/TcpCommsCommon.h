/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

/** @file
@details function in this file are common function used between the different TCP comms */

#include "TcpHelperClasses.h"

#include <chrono>
#include <string>

class AsioContextManager;
namespace asio {
class io_context;
}  // namespace asio

namespace helics {
class CommsInterface;

namespace tcp {
    /** establish a connection to a server by as associated timeout*/
    TcpConnection::pointer makeConnection(asio::io_context& io_context,
                                          const std::string& connection,
                                          const std::string& port,
                                          size_t bufferSize,
                                          std::chrono::milliseconds timeOut);

    /** do some checking and logging about errors if the interface is connected*/
    bool commErrorHandler(CommsInterface* comm,
                          TcpConnection* connection,
                          const std::error_code& error);
}  // namespace tcp
}  // namespace helics
