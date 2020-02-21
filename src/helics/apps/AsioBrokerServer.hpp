/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "TypedBrokerServer.hpp"
#include "helics/common/AsioContextManager.h"

#include <mutex>
#include <thread>

namespace helics {
namespace tcp {
    class TcpServer;
    class TcpConnection;
} // namespace tcp
namespace udp {
    class UdpServer;
}
class Broker;
namespace apps {

    /** a virtual class to use as a base for broker servers of various types*/
    class AsioBrokerServer: public TypedBrokerServer {
      public:
        AsioBrokerServer() = default;
        explicit AsioBrokerServer(std::string server_name): name_(std::move(server_name)) {}
        /** start the server*/
        virtual void startServer(const Json::Value* val) override;
        /** stop the server*/
        virtual void stopServer() override;
        void enableTcpServer(bool enabled) { tcp_enabled_ = enabled; }
        void enableUdpServer(bool enabled) { udp_enabled_ = enabled; };

      private:
        void mainLoop();
        std::shared_ptr<tcp::TcpServer> loadTCPserver(asio::io_context& ioctx);
        std::shared_ptr<udp::UdpServer> loadUDPserver(asio::io_context& ioctx);

        void loadTCPServerData(portData& pdata);
        void loadUDPServerData(portData& pdata);

        //std::string generateResponseToMessage(zmq::message_t &msg, portData &pdata, core_type ctype);
        std::size_t tcpDataReceive(
            std::shared_ptr<tcp::TcpConnection> connection,
            const char* data,
            size_t bytes_received);
        bool udpDataReceive(
            std::shared_ptr<udp::UdpServer> server,
            const char* data,
            size_t bytes_received);

        std::thread mainLoopThread;
        std::mutex threadGuard;

        portData tcpPortData;
        std::shared_ptr<tcp::TcpServer> tcpserver;
        std::shared_ptr<udp::UdpServer> udpserver;
        portData udpPortData;
        const Json::Value* config_{nullptr};
        const std::string name_;
        bool tcp_enabled_{false};
        bool udp_enabled_{false};
    };
} // namespace apps
} // namespace helics
