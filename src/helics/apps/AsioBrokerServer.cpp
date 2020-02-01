/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "AsioBrokerServer.hpp"
#include <iostream>
#include "../common/JsonProcessingFunctions.hpp"
#include "../core/networkDefaults.hpp"
#include "../core/NetworkBrokerData.hpp"

#include "../core/tcp/TcpHelperClasses.h"

#include "../common/AsioContextManager.h"
#include <asio/ip/udp.hpp>

#include <array>

namespace helics
{
    namespace udp
    {
        class UdpServer: public std::enable_shared_from_this<UdpServer>
        {
        public:
            UdpServer(asio::io_context& io_context,int portNum)
                : socket_(io_context, asio::ip::udp::endpoint(asio::ip::udp::v4(), portNum))
            {
                
            }

        
            void start_receive()
            {
                socket_.async_receive_from(
                    asio::buffer(recv_buffer_), remote_endpoint_, [this](const asio::error_code& error, std::size_t bytes) {handle_receive(error, bytes); });
            }
        private:
            void handle_receive(const asio::error_code& error,
                std::size_t bytes_transferred)
            {
                if (!error)
                {
                    ActionMessage m(recv_buffer_.data(), static_cast<int>(bytes_transferred));
                    if (isProtocolCommand(m)) {
                        // if the reply is not ignored respond with it otherwise
                        // forward the original message on to the receiver to handle
                       // auto rep = generateMessageResponse(m, tcpPortData, core_type::UDP);
                      //  if (rep.action() != CMD_IGNORE) {
                      //      try {
                      //          socket_.send_to(asio::buffer(rep.to_string()), remote_endpoint_);
                       //     }
                       //     catch (const std::system_error&) {
                       //     }
                      //  }

                    }
                    start_receive();
                }
            }

            asio::ip::udp::socket socket_;
            asio::ip::udp::endpoint remote_endpoint_;
            std::array<char, 1024> recv_buffer_;
        };
    }
    namespace apps
    {

        std::size_t AsioBrokerServer::tcpDataReceive(
            std::shared_ptr<tcp::TcpConnection> connection,
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

        std::shared_ptr<tcp::TcpServer> AsioBrokerServer::loadTCPserver(asio::io_context &ioctx)
        {
            std::string ext_interface = "0.0.0.0";
            int tcpport = DEFAULT_TCP_BROKER_PORT_NUMBER;
            std::chrono::milliseconds timeout(20000);
            if (config_->isMember("tcp")) {
                auto V = (*config_)["tcp"];
                replaceIfMember(V, "interface", ext_interface);
                replaceIfMember(V, "port", tcpport);
            }
            auto server = helics::tcp::TcpServer::create(
                ioctx,
                ext_interface,
                static_cast<uint16_t>(tcpport),
                true,
                2048);
            return server;
        }

        void AsioBrokerServer::loadUDPsocket(asio::io_context& ioctx)
        {
            std::string ext_interface = "0.0.0.0";
            int zmqport = DEFAULT_UDP_BROKER_PORT_NUMBER;
            std::chrono::milliseconds timeout(20000);
            if (config_->isMember("udp")) {
                auto V = (*config_)["udp"];
                replaceIfMember(V, "interface", ext_interface);
                replaceIfMember(V, "port", zmqport);
            }
           
        }
        void AsioBrokerServer::loadTCPServerData(portData &pdata)
        {
            pdata.clear();
            for (int ii = 0; ii < 20; ++ii) {
                pdata.emplace_back(DEFAULT_TCP_BROKER_PORT_NUMBER + 4 + ii, false, nullptr);
            }
        }

        void AsioBrokerServer::loadUDPServerData(portData &pdata)
        {
            pdata.clear();
            for (int ii = 0; ii < 20; ++ii) {
                pdata.emplace_back(DEFAULT_ZMQSS_BROKER_PORT_NUMBER + 4 + ii, false, nullptr);
            }

        }


        void AsioBrokerServer::startServer(const Json::Value *val)
        {
            std::cout << "starting asio broker server\n";
            config_ = val;

            std::lock_guard<std::mutex> tlock(threadGuard);
            mainLoopThread = std::thread([this]() { mainLoop(); });
        }

        void AsioBrokerServer::stopServer()
        {
            std::lock_guard<std::mutex> tlock(threadGuard);
            if (tcp_enabled_)
            {
                tcpserver->close();
            }
            mainLoopThread.join();
            std::cout << "exiting asio broker server" << std::endl;
        }

        void AsioBrokerServer::mainLoop()
        {
            auto ioctx = AsioContextManager::getContextPointer();
            if (tcp_enabled_)
            {
                tcpserver = loadTCPserver(ioctx->getBaseContext());
                tcpserver->setDataCall(
                    [this](tcp::TcpConnection::pointer connection, const char* data, size_t datasize) {
                        return tcpDataReceive(connection, data, datasize);
                    });

                loadTCPServerData(tcpPortData);
                tcpserver->start();
            }
            if (udp_enabled_)
            {

            }
        }
    }
}
