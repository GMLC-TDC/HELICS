/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "TypedBrokerServer.hpp"

#include <memory>
#include <string>
#include <utility>

#if defined(HELICS_ENABLE_TCP_CORE) || defined(HELICS_ENABLE_UDP_CORE)

#    include "gmlc/networking/AsioContextManager.h"

#    include <mutex>
#    include <thread>

#    ifdef HELICS_ENABLE_TCP_CORE
namespace gmlc::networking {
class TcpServer;
class TcpConnection;
}  // namespace gmlc::networking
#    endif

namespace helics {

#    ifdef HELICS_ENABLE_UDP_CORE
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
        explicit AsioBrokerServer(std::string_view server_name): name_(server_name) {}
        /** start the server*/
        virtual void startServer(const nlohmann::json* val,
                                 const std::shared_ptr<TypedBrokerServer>& ptr) override;
        /** stop the server*/
        virtual void stopServer() override;
        virtual void processArgs(std::string_view args) override;
        void enableTcpServer(bool enabled) { tcp_enabled_ = enabled; }
        void enableUdpServer(bool enabled) { udp_enabled_ = enabled; }

      private:
        void mainLoop();
#    ifdef HELICS_ENABLE_TCP_CORE
        std::shared_ptr<gmlc::networking::TcpServer> loadTCPserver(asio::io_context& ioctx);
        static void loadTCPServerData(portData& pdata);
        std::size_t
            tcpDataReceive(const std::shared_ptr<gmlc::networking::TcpConnection>& connection,
                           const char* data,
                           size_t bytes_received);
        std::shared_ptr<gmlc::networking::TcpServer> tcpserver;
        portData tcpPortData;
#    endif
#    ifdef HELICS_ENABLE_UDP_CORE
        std::shared_ptr<udp::UdpServer> loadUDPserver(asio::io_context& ioctx);
        static void loadUDPServerData(portData& pdata);

        bool udpDataReceive(const std::shared_ptr<udp::UdpServer>& server,
                            const char* data,
                            size_t bytes_received);
        std::shared_ptr<udp::UdpServer> udpserver;
        portData udpPortData;
#    endif

        std::thread mainLoopThread;
        std::mutex threadGuard;

        const nlohmann::json* config_{nullptr};
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
        explicit AsioBrokerServer(std::string_view /*server_name*/) {}
        void enableTcpServer(bool /*enabled*/) {}
        void enableUdpServer(bool /*enabled*/) {}
        /** start the server*/
        virtual void startServer(const nlohmann::json* /*val*/,
                                 const std::shared_ptr<TypedBrokerServer>& /*ptr*/) override
        {
        }
        /** stop the server*/
        virtual void stopServer() override {}
    };
}  // namespace apps
}  // namespace helics
#endif
