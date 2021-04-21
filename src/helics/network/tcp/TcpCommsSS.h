/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../NetworkCommsInterface.hpp"

#include <atomic>
#include <set>
#include <string>
#include <vector>

class AsioContextManager;
namespace asio {
class io_context;
}  // namespace asio

namespace helics {
namespace tcp {
    class TcpConnection;

    /** implementation for the communication interface that uses TCP messages to communicate*/
    class TcpCommsSS final: public NetworkCommsInterface {
      public:
        /** default constructor*/
        TcpCommsSS() noexcept;
        /** destructor*/
        ~TcpCommsSS();

        /** add a connection to the connection list*/
        void addConnection(const std::string& newConn);
        /** add a vector of connections to the connection list*/
        void addConnections(const std::vector<std::string>& newConnections);
        /** allow outgoing connections*/
        virtual void setFlag(const std::string& flag, bool val) override;

      private:
        bool outgoingConnectionsAllowed{
            true};  //!< disable all outgoing connections- allow only incoming connections
        bool reuse_address{false};
        std::vector<std::string> connections;  //!< list of connections to make
        virtual int getDefaultBrokerPort() const override;
        virtual void queue_rx_function() override;  //!< the functional loop for the receive queue
        virtual void queue_tx_function() override;  //!< the loop for transmitting data

        /** process an incoming message
    return code for required action 0=NONE, -1 TERMINATE*/
        int processIncomingMessage(ActionMessage&& cmd);

        /** callback function for receiving data asynchronously from the socket
    @param connection pointer to the connection
    @param data the pointer to the data
    @param bytes_received the length of the received data
    @return a the number of bytes used by the function
    */
        size_t dataReceive(TcpConnection* connection, const char* data, size_t bytes_received);
        //  bool errorHandle()
    };

}  // namespace tcp
}  // namespace helics
