/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "AsioBrokerServer.hpp"

#include "../common/AsioContextManager.h"
#include "../common/JsonProcessingFunctions.hpp"
#include "../network/NetworkBrokerData.hpp"
#include "../network/networkDefaults.hpp"
#include "helics/external/CLI11/CLI11.hpp"
#ifdef ENABLE_TCP_CORE
#    include "../network/tcp/TcpHelperClasses.h"
#endif
#ifdef ENABLE_UDP_CORE
#    include <asio/ip/udp.hpp>
#endif

#include <array>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

namespace helics {

#ifdef ENABLE_UDP_CORE
namespace udp {
    class UdpServer: public std::enable_shared_from_this<UdpServer> {
      public:
        UdpServer(asio::io_context& io_context, std::string& interface, std::uint16_t portNum):
            socket_(io_context)
        {
            socket_.open(asio::ip::udp::v4());
            socket_.bind(
                asio::ip::udp::endpoint(asio::ip::address::from_string(interface), portNum));
        }

        ~UdpServer()
        {
            stop_receive();
            socket_.close();
        }

        void start_receive()
        {
            socket_.async_receive_from(asio::buffer(recv_buffer_),
                                       remote_endpoint_,
                                       [this](const asio::error_code& error, std::size_t bytes) {
                                           handle_receive(error, bytes);
                                       });
        }
        void stop_receive() { socket_.cancel(); }
        /** set the callback for the data object*/
        void setDataCall(
            std::function<bool(std::shared_ptr<UdpServer>, const char*, size_t)> dataFunc)
        {
            dataCall = std::move(dataFunc);
        }

        void send_to(const std::string& message, asio::ip::udp::endpoint ept)
        {
            socket_.send_to(asio::buffer(message), ept);
        }
        void reply(const std::string& message)
        {
            socket_.send_to(asio::buffer(message), remote_endpoint_);
        }

      private:
        void handle_receive(const asio::error_code& error, std::size_t bytes_transferred)
        {
            if (!error) {
                if (dataCall) {
                    bool ret = dataCall(shared_from_this(), recv_buffer_.data(), bytes_transferred);
                    if (ret) {
                        start_receive();
                    }
                }
            }
        }

        asio::ip::udp::socket socket_;
        asio::ip::udp::endpoint remote_endpoint_;
        std::array<char, 1024> recv_buffer_;
        std::function<bool(std::shared_ptr<UdpServer>, const char*, size_t)> dataCall;
    };
}  // namespace udp
#endif

namespace apps {
#ifdef ENABLE_TCP_CORE
    std::size_t AsioBrokerServer::tcpDataReceive(std::shared_ptr<tcp::TcpConnection> connection,
                                                 const char* data,
                                                 std::size_t bytes_received)
    {
        std::size_t used_total = 0;
        while (used_total < bytes_received) {
            ActionMessage m;
            auto used =
                m.depacketize(data + used_total, static_cast<int>(bytes_received - used_total));
            if (used == 0) {
                break;
            }
            if (isProtocolCommand(m)) {
                // if the reply is not ignored respond with it otherwise
                // forward the original message on to the receiver to handle
                auto rep = generateMessageResponse(m, tcpPortData, core_type::TCP);
                if (rep.action() != CMD_IGNORE) {
                    try {
                        connection->send(rep.packetize());
                    }
                    catch (const std::system_error&) {
                    }
                }
            }
            used_total += used;
        }

        return used_total;
    }

    std::shared_ptr<tcp::TcpServer> AsioBrokerServer::loadTCPserver(asio::io_context& ioctx)
    {
        std::string ext_interface = "0.0.0.0";
        int tcpport = DEFAULT_TCP_BROKER_PORT_NUMBER;
        // std::chrono::milliseconds timeout(20000);
        if (config_->isMember("tcp")) {
            auto V = (*config_)["tcp"];
            replaceIfMember(V, "interface", ext_interface);
            replaceIfMember(V, "port", tcpport);
        }
        auto server = helics::tcp::TcpServer::create(
            ioctx, ext_interface, static_cast<uint16_t>(tcpport), true, 2048);
        return server;
    }

    void AsioBrokerServer::loadTCPServerData(portData& pdata)
    {
        pdata.clear();
        for (int ii = 0; ii < 20; ++ii) {
            pdata.emplace_back(DEFAULT_TCP_BROKER_PORT_NUMBER + 4 + ii, false, nullptr);
        }
    }
#endif  // ENABLE_TCP_CORE

#ifdef ENABLE_UDP_CORE
    bool AsioBrokerServer::udpDataReceive(std::shared_ptr<udp::UdpServer> server,
                                          const char* data,
                                          size_t bytes_received)
    {
        ActionMessage m(data, static_cast<int>(bytes_received));
        if (isProtocolCommand(m)) {
            // if the reply is not ignored respond with it otherwise
            // forward the original message on to the receiver to handle
            auto rep = generateMessageResponse(m, udpPortData, core_type::UDP);
            if (rep.action() != CMD_IGNORE) {
                try {
                    server->reply(rep.to_string());
                }
                catch (const std::system_error&) {
                    return false;
                }
            }
        } else if (bytes_received == 5) {
            if (std::string(data, bytes_received) == "close") {
                return false;
            }
        }

        return true;
    }

    std::shared_ptr<udp::UdpServer> AsioBrokerServer::loadUDPserver(asio::io_context& ioctx)
    {
        std::string ext_interface = "0.0.0.0";
        int udpport = DEFAULT_UDP_BROKER_PORT_NUMBER;
        // std::chrono::milliseconds timeout(20000);
        if (config_->isMember("udp")) {
            auto V = (*config_)["udp"];
            replaceIfMember(V, "interface", ext_interface);
            replaceIfMember(V, "port", udpport);
        }
        return std::make_shared<udp::UdpServer>(ioctx, ext_interface, udpport);
    }

    void AsioBrokerServer::loadUDPServerData(portData& pdata)
    {
        pdata.clear();
        for (int ii = 0; ii < 20; ++ii) {
            pdata.emplace_back(DEFAULT_ZMQSS_BROKER_PORT_NUMBER + 4 + ii, false, nullptr);
        }
    }

#endif  // ENABLE_UDP_CORE

    void AsioBrokerServer::processArgs(const std::string& args)

    {
        /*
         CLI::App parser("Asio broker server CLI parser");
         parser.allow_extras();
         parser.add_option("--tcp_port", tcpPort_, "specify the tcp port to use");
         parser.add_option("--tcp_interface",
                           tcpAddress_,
                           "specify the interface to use for connecting the tcp server");

         parser.add_option("--udp_port", udpPort_, "specify the udp port to use");
         parser.add_option("--udp_interface",
                           udpAddress_,
                           "specify the interface to use for connecting a udp server");

         try {
             parser.parse(args);
         }
         catch (const CLI::Error& ce) {
             logMessage(std::string("error processing command line arguments for asio broker server
         :") + ce.what());
         }
         */
    }

    static const Json::Value null;

    void AsioBrokerServer::startServer(const Json::Value* val)
    {
        config_ = (val != nullptr) ? val : &null;

        if (tcp_enabled_) {
            logMessage("starting tcp broker server");
        }
        if (udp_enabled_) {
            logMessage("starting udp broker server");
        }
        std::lock_guard<std::mutex> tlock(threadGuard);
        mainLoopThread = std::thread([this]() { mainLoop(); });
    }

    void AsioBrokerServer::stopServer()
    {
        std::lock_guard<std::mutex> tlock(threadGuard);
        if (tcp_enabled_) {
#ifdef ENABLE_TCP_CORE
            logMessage("stopping tcp broker server");
            tcpserver->close();
#endif
        }
        if (udp_enabled_) {
#ifdef ENABLE_UDP_CORE
            logMessage("stopping udp broker server");
            udpserver->stop_receive();
#endif
        }
        mainLoopThread.join();
    }

    void AsioBrokerServer::mainLoop()
    {
        auto ioctx = AsioContextManager::getContextPointer();
#ifdef ENABLE_TCP_CORE
        if (tcp_enabled_) {
            tcpserver = loadTCPserver(ioctx->getBaseContext());
            tcpserver->setDataCall(
                [this](tcp::TcpConnection::pointer connection, const char* data, size_t datasize) {
                    return tcpDataReceive(connection, data, datasize);
                });

            loadTCPServerData(tcpPortData);
            tcpserver->start();
        }
#endif
#ifdef ENABLE_UDP_CORE
        if (udp_enabled_) {
            udpserver = loadUDPserver(ioctx->getBaseContext());
            udpserver->setDataCall(
                [this](std::shared_ptr<udp::UdpServer> server, const char* data, size_t datasize) {
                    return udpDataReceive(server, data, datasize);
                });

            loadUDPServerData(udpPortData);
            udpserver->start_receive();
        }
#endif
    }
}  // namespace apps
}  // namespace helics
