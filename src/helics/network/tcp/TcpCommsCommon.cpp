/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "TcpCommsCommon.h"

#include "../../core/ActionMessage.hpp"
#include "../CommsInterface.hpp"
#include "../NetworkBrokerData.hpp"
#include "gmlc/networking/TcpConnection.h"

#include <memory>
#include <string>

namespace helics {
namespace tcp {

    bool commErrorHandler(CommsInterface* comm,
                          gmlc::networking::TcpConnection* /*connection*/,
                          const std::error_code& error)
    {
        if (comm->isConnected()) {
            if ((error != asio::error::eof) && (error != asio::error::operation_aborted)) {
                if (error != asio::error::connection_reset) {
                    comm->logError("error message while connected " + error.message() + "code " +
                                   std::to_string(error.value()));
                }
            }
        }
        return false;
    }

}  // namespace tcp
}  // namespace helics
