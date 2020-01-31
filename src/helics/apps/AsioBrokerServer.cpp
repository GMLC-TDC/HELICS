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

#ifdef ENABLE_TCP_CORE
#    include "../core/tcp/TcpHelperClasses.h"
#endif

#include "../common/AsioContextManager.h"

namespace helics
{
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
