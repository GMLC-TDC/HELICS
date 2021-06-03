/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "TypedBrokerServer.hpp"

#include <memory>
#include <string>
#include <utility>

#if defined(ENABLE_TCP_CORE) || defined(ENABLE_UDP_CORE)

#    include "helics/common/AsioContextManager.h"

#    include <mutex>
#    include <thread>

namespace helics {
#    ifdef ENABLE_TCP_CORE
namespace tcp {
    class TcpServer;
    class TcpConnection;
}  // namespace tcp
#    endif
#    ifdef ENABLE_UDP_CORE
namespace udp {
    class UdpServer;
}
#    endif
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
        virtual void processArgs(const std::string& args) override;
        void enableTcpServer(bool enabled) { tcp_enabled_ = enabled; }
        void enableUdpServer(bool enabled) { udp_enabled_ = enabled; }

      private:
        void mainLoop();
#    ifdef ENABLE_TCP_CORE
        std::shared_ptr<tcp::TcpServer> loadTCPserver(asio::io_context& ioctx);
        void loadTCPServerData(portData& pdata);
        std::size_t tcpDataReceive(std::shared_ptr<tcp::TcpConnection> connection,
                                   const char* data,
                                   size_t bytes_received);
        std::shared_ptr<tcp::TcpServer> tcpserver;
        portData tcpPortData;
#    endif
#    ifdef ENABLE_UDP_CORE
        std::shared_ptr<udp::UdpServer> loadUDPserver(asio::io_context& ioctx);
        void loadUDPServerData(portData& pdata);

        bool udpDataReceive(std::shared_ptr<udp::UdpServer> server,
                            const char* data,
                            size_t bytes_received);
        std::shared_ptr<udp::UdpServer> udpserver;
        portData udpPortData;
#    endif

        std::thread mainLoopThread;
        std::mutex threadGuard;

        const Json::Value* config_{nullptr};
        const std::string name_;
        bool tcp_enabled_{false};
        bool udp_enabled_{false};
    };
}  // namespace apps
}  // namespace helics

#else
namespace helics {
namespace apps {
    /** a virtual class to use as a base for broker servers of various types*/
    class AsioBrokerServer: public TypedBrokerServer {
      public:
        AsioBrokerServer() = default;
        explicit AsioBrokerServer(std::string /*server_name*/) {}
        void enableTcpServer(bool /*enabled*/) {}
        void enableUdpServer(bool /*enabled*/) {}
        /** start the server*/
        virtual void startServer(const Json::Value* /*val*/) override {}
        /** stop the server*/
        virtual void stopServer() override {}
    };
}  // namespace apps
}  // namespace helics
#endif
