/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

/** @file
@details function in this file are common function used between the different TCP comms */

#include <chrono>
#include <string>
#include <system_error>

namespace gmlc::networking {
class TcpConnection;
}

namespace helics {
class CommsInterface;

namespace tcp {

    /** do some checking and logging about errors if the interface is connected*/
    bool commErrorHandler(CommsInterface* comm,
                          gmlc::networking::TcpConnection* connection,
                          const std::error_code& error);
}  // namespace tcp
}  // namespace helics
