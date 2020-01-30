/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "zmqBrokerServer.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "../core/NetworkBrokerData.hpp"
#include "../core/networkDefaults.hpp"

#include <iostream>

#ifdef ENABLE_ZMQ_CORE
#    include "../common/zmqContextManager.h"
#    include "../core/zmq/ZmqCommsCommon.h"
#endif

namespace helics {
namespace apps {
    void zmqBrokerServer::startServer(const Json::Value* val)
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
        exitAll.store(true);
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
            reqSocket.send(std::string("close_server:") + name_);
            reqSocket.close();
        }
        catch (const zmq::error_t&) {
        }

        std::lock_guard<std::mutex> tlock(threadGuard);
        mainLoopThread.join();
#endif
    }

    std::unique_ptr<zmq::socket_t> zmqBrokerServer::loadZMQsocket()
    {
        std::string ext_interface = "tcp://*";
        int zmqport = DEFAULT_ZMQ_BROKER_PORT_NUMBER + 1;
        std::chrono::milliseconds timeout(20000);
        if (config_->isMember("zmq")) {
            auto V = (*config_)["zmq"];
            replaceIfMember(V, "interface", ext_interface);
            replaceIfMember(V, "port", zmqport);
        }
        auto ctx = ZmqContextManager::getContextPointer();
        auto repSocket=std::make_unique<zmq::socket_t>(ctx->getContext(), ZMQ_REP);
        repSocket->setsockopt(ZMQ_LINGER, 500);
        auto bindsuccess = hzmq::bindzmqSocket(*repSocket, ext_interface, zmqport, timeout);
        if (!bindsuccess) {
            repSocket->close();
            std::cout << "ZMQ server failed to start\n";
            return nullptr;
        }
        return repSocket;
    }

    std::unique_ptr<zmq::socket_t> zmqBrokerServer::loadZMQSSsocket()
    {
        std::string ext_interface = "tcp://*";
        int zmqport = DEFAULT_ZMQSS_BROKER_PORT_NUMBER;
        std::chrono::milliseconds timeout(20000);
        if (config_->isMember("zmqss")) {
            auto V = (*config_)["zmqss"];
            replaceIfMember(V, "interface", ext_interface);
            replaceIfMember(V, "port", zmqport);
        }
        auto ctx = ZmqContextManager::getContextPointer();
        auto repSocket = std::make_unique<zmq::socket_t>(ctx->getContext(), ZMQ_ROUTER);
        repSocket->setsockopt(ZMQ_LINGER, 500);
        auto bindsuccess = hzmq::bindzmqSocket(*repSocket, ext_interface, zmqport, timeout);
        if (!bindsuccess) {
            repSocket->close();
            std::cout << "ZMQSS server failed to start\n";
            return nullptr;
        }
        return repSocket;
    }

    zmqBrokerServer::zmqServerData zmqBrokerServer::generateZMQServerData()
    {
        zmqServerData pdata;
        for (int ii = 0; ii < 20; ++ii) {
            pdata.ports.emplace_back(DEFAULT_ZMQ_BROKER_PORT_NUMBER + 4 + ii * 2, false, nullptr);
        }
        return pdata;
    }

    zmqBrokerServer::zmqServerData zmqBrokerServer::generateZMQSSServerData()
    {
        zmqServerData pdata;
        for (int ii = 0; ii < 20; ++ii) {
            pdata.ports.emplace_back(DEFAULT_ZMQSS_BROKER_PORT_NUMBER + 4 + ii, false, nullptr);
        }
        return pdata;
    }

    std::string zmqBrokerServer::generateResponseToMessage(zmq::message_t& msg, portData& pdata)
    {
        auto sz = msg.size();
        if (sz < 25) {
            if (std::string(static_cast<char*>(msg.data()), msg.size()) ==
                std::string("close_server:") + name_) {
                //      std::cerr << "received close server message" << std::endl;
                return std::string("close_server:") + name_;
            }
        } else {
            ActionMessage rxcmd(static_cast<char*>(msg.data()), msg.size());
            auto str = generateMessageResponse(rxcmd, pdata);
            if (!str.empty()) {
                return str;
            }
        }
        std::cout << "received unknown message " << msg.size() << std::endl;
        return "ignored";
    }
    void zmqBrokerServer::mainLoop()
    {
#ifdef ENABLE_ZMQ_CORE
        std::vector<std::unique_ptr<zmq::socket_t>> sockets;
        std::vector<zmqServerData> data;
        std::vector<std::function<void(zmq::socket_t*, portData&)>> handleMessage;

        if (zmq_enabled_) {
            sockets.push_back(loadZMQsocket());
            data.push_back(generateZMQServerData());
            handleMessage.push_back([this](zmq::socket_t* skt, portData& pdata) {
                zmq::message_t msg;
                skt->recv(msg);
                std::string response = generateResponseToMessage(msg, pdata);
                skt->send(response);
            });
        }

        if (zmqss_enabled_) {
            sockets.push_back(loadZMQSSsocket());
            data.push_back(generateZMQSSServerData());
            handleMessage.push_back([this](zmq::socket_t* skt, portData& pdata) {
                zmq::message_t msg1;
                zmq::message_t msg2;
                zmq::message_t msg3;

                skt->recv(msg1); //should be routing
                skt->recv(msg2); //should be null
                skt->recv(msg3); //should be actual message
                std::string response = generateResponseToMessage(msg3, pdata);
                skt->send(msg1, zmq::send_flags::sndmore);
                skt->send(msg2, zmq::send_flags::sndmore);
                skt->send(response, zmq::send_flags::dontwait);
                });
        }

        std::vector<zmq::pollitem_t> poller;
        for (size_t ii = 0; ii < sockets.size(); ++ii) {
            poller.emplace_back();
            poller.back().socket = sockets[ii].get();
            poller.back().events = ZMQ_POLLIN;
        }

        int rc = 0;
        while (rc >= 0) {
            rc = zmq::poll(poller.data(), poller.size(), std::chrono::milliseconds(5000));
            if (rc < 0) {
                std::cerr << "ZMQ broker connection error (2)" << std::endl;
                break;
            }
            if (rc > 0) {
                zmq::message_t msg;
                for (int ii = 0; ii < poller.size(); ++ii) {
                    if ((poller[ii].revents & ZMQ_POLLIN) != 0) {
                        handleMessage[ii](sockets[ii].get(), data[ii].ports);
                    }
                }
            }
              if (exitAll.load()) {
                std::cerr << "exit all active" << std::endl;
                 break;
            }
        }

        for (auto &skt : sockets)
        {
            skt->close();
        }
        sockets.clear();
        std::cerr << "exiting zmq broker server" << std::endl;

#endif
    }
} // namespace apps
} // namespace helics
