/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "TcpCommsCommon.h"

#include "../../common/AsioContextManager.h"
#include "../../core/ActionMessage.hpp"
#include "../CommsInterface.hpp"
#include "../NetworkBrokerData.hpp"
#include "TcpHelperClasses.h"

#include <memory>
#include <string>

namespace helics {
namespace tcp {
    TcpConnection::pointer makeConnection(asio::io_context& io_context,
                                          const std::string& connection,
                                          const std::string& port,
                                          size_t bufferSize,
                                          std::chrono::milliseconds timeOut)
    {
        using std::chrono::milliseconds;
        using std::chrono::steady_clock;

        using namespace std::chrono_literals;  // NOLINT
        auto tick = steady_clock::now();
        milliseconds timeRemaining(timeOut);
        milliseconds timeRemPrev(timeOut);
        TcpConnection::pointer connectionPtr =
            TcpConnection::create(io_context, connection, port, bufferSize);
        int trycnt = 1;
        while (!connectionPtr->waitUntilConnected(timeRemaining)) {
            auto tock = steady_clock::now();
            timeRemaining =
                milliseconds(timeOut) - std::chrono::duration_cast<milliseconds>(tock - tick);
            if ((timeRemaining < 0ms) && (trycnt > 1)) {
                connectionPtr = nullptr;
                break;
            }
            // make sure we slow down and sleep for a little bit
            if (timeRemPrev - timeRemaining < 100ms) {
                std::this_thread::sleep_for(200ms);
            }
            timeRemPrev = timeRemaining;
            if (timeRemaining < 0ms) {
                timeRemaining = 400ms;
            }

            // lets try to connect again
            ++trycnt;
            connectionPtr = TcpConnection::create(io_context, connection, port, bufferSize);
        }
        return connectionPtr;
    }

    bool commErrorHandler(CommsInterface* comm,
                          TcpConnection* /*connection*/,
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
