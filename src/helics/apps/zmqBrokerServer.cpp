/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "zmqBrokerServer.hpp"
#include <iostream>
#include "../common/JsonProcessingFunctions.hpp"
#include "../core/networkDefaults.hpp"
#include "../core/NetworkBrokerData.hpp"

#ifdef ENABLE_ZMQ_CORE
#    include "../common/zmqContextManager.h"
#    include "../core/zmq/ZmqCommsCommon.h"
#endif

namespace helics
{
    namespace apps
    {
        void zmqBrokerServer::startServer(const Json::Value *val)
        {
            std::cerr << "starting zmq broker server\n";
            config_ = val;
            
#ifdef ENABLE_ZMQ_CORE
            std::lock_guard<std::mutex> tlock(threadGuard);
            mainLoopThread = std::thread([this]() { mainLoop(); });
#endif
        }

        void zmqBrokerServer::stopServer()
        {
#ifdef ENABLE_ZMQ_CORE
            auto ctx = ZmqContextManager::getContextPointer();
            zmq::socket_t reqSocket(ctx->getContext(), ZMQ_REQ);
            reqSocket.setsockopt(ZMQ_LINGER, 300);
            std::string ext_interface = "tcp://127.0.0.1";
            int port = DEFAULT_ZMQ_BROKER_PORT_NUMBER + 1;
            if (config_->isMember("zmq")) {
                auto V = (*config_)["zmq"];
                replaceIfMember(V, "interface", ext_interface);
                replaceIfMember(V, "port", port);
            }
            try {
                reqSocket.connect(helics::makePortAddress(ext_interface, port));
                reqSocket.send(std::string("close_server:") + server_name_);
                reqSocket.close();
            }
            catch (const zmq::error_t&) {
            }

            std::lock_guard<std::mutex> tlock(threadGuard);
            mainLoopThread.join();
#endif
        }

        void zmqBrokerServer::mainLoop()
        {
#ifdef ENABLE_ZMQ_CORE
            std::string ext_interface = "tcp://*";
            int port = DEFAULT_ZMQ_BROKER_PORT_NUMBER + 1;
            std::chrono::milliseconds timeout(20000);
            if (config_->isMember("zmq")) {
                auto V = (*config_)["zmq"];
                replaceIfMember(V, "interface", ext_interface);
                replaceIfMember(V, "port", port);
            }
            auto ctx = ZmqContextManager::getContextPointer();
            zmq::socket_t repSocket(ctx->getContext(), ZMQ_REP);
            repSocket.setsockopt(ZMQ_LINGER, 500);
            auto bindsuccess = hzmq::bindzmqSocket(repSocket, ext_interface, port, timeout);
            if (!bindsuccess) {
                repSocket.close();
                std::cout << "ZMQ server failed to start\n";
                return;
            }
            zmq::pollitem_t poller;
            poller.socket = static_cast<void*>(repSocket);
            poller.events = ZMQ_POLLIN;

            portData pdata;
            for (int ii = 0; ii < 20; ++ii) {
                pdata.emplace_back(DEFAULT_ZMQ_BROKER_PORT_NUMBER + 4 + ii * 2, false, nullptr);
            }

            int rc = 0;
            while (rc >= 0) {
                rc = zmq::poll(&poller, 1, std::chrono::milliseconds(5000));
                if (rc < 0) {
                    std::cerr << "ZMQ broker connection error (2)" << std::endl;
                    break;
                }
                if (rc > 0) {
                    zmq::message_t msg;
                    repSocket.recv(msg);
                    auto sz = msg.size();
                    if (sz < 25) {
                        if (std::string(static_cast<char*>(msg.data()), msg.size()) ==
                            std::string("close_server:") + server_name_) {
                            //      std::cerr << "received close server message" << std::endl;
                            repSocket.send(msg, zmq::send_flags::none);
                            break;
                        }
                        else {
                            //    std::cerr << "received unrecognized message (ignoring)"
                            //              << std::string (static_cast<char *> (msg.data ()), msg.size ()) << std::endl;
                            repSocket.send("ignored");
                            continue;
                        }
                    }
                    else {
                        ActionMessage rxcmd(static_cast<char*>(msg.data()), msg.size());
                        auto str = generateMessageResponse(rxcmd, pdata);
                        if (!str.empty())
                        {
                            repSocket.send(str);
                        }
                        else
                        {
                            std::cout << "received unknown message " << msg.size() << std::endl;
                            repSocket.send("ignored");
                        }
                    }
                }
                if (exitall.load()) {
                    //    std::cerr << "exit all active" << std::endl;
                    break;
                }
            }
            repSocket.close();
            std::cerr << "exiting zmq broker server" << std::endl;

#endif
        }
    }
}
