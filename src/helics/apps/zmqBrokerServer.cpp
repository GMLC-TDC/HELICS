/*
Copyright (c) 2017-2020,
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

static const Json::Value null;

namespace helics {
namespace apps {
    void zmqBrokerServer::startServer(const Json::Value* val)
    {
        config_ = (val != nullptr) ? val : &null;
#ifdef ENABLE_ZMQ_CORE
        if (zmq_enabled_) {
            logMessage("starting zmq broker server");
        }
        if (zmqss_enabled_) {
            logMessage("starting zmq ss broker server");
        }
        std::lock_guard<std::mutex> tlock(threadGuard);
        mainLoopThread = std::thread([this]() { mainLoop(); });
#endif
    }

    void zmqBrokerServer::stopServer()
    {
        exitAll.store(true);
#ifdef ENABLE_ZMQ_CORE
        if (!zmq_enabled_ && !zmqss_enabled_) {
            return;
        }
        auto ctx = ZmqContextManager::getContextPointer();
        zmq::socket_t reqSocket(ctx->getContext(), (zmq_enabled_) ? ZMQ_REQ : ZMQ_DEALER);
        reqSocket.setsockopt(ZMQ_LINGER, 300);
        std::string ext_interface = "tcp://127.0.0.1";
        int port =
            (zmq_enabled_) ? DEFAULT_ZMQ_BROKER_PORT_NUMBER + 1 : DEFAULT_ZMQSS_BROKER_PORT_NUMBER;
        if (zmq_enabled_) {
            if (config_->isMember("zmq")) {
                auto V = (*config_)["zmq"];
                replaceIfMember(V, "interface", ext_interface);
                replaceIfMember(V, "port", port);
            }
        } else {
            if (config_->isMember("zmqss")) {
                auto V = (*config_)["zmqss"];
                replaceIfMember(V, "interface", ext_interface);
                replaceIfMember(V, "port", port);
            }
        }

        try {
            reqSocket.connect(helics::makePortAddress(ext_interface, port));
            reqSocket.send(std::string("close_server:") + name_);
            reqSocket.close();
        }
        catch (const zmq::error_t&) {
        }

        std::lock_guard<std::mutex> tlock(threadGuard);
        if (zmq_enabled_) {
            logMessage("stopping zmq broker server");
        }
        if (zmqss_enabled_) {
            logMessage("stopping zmq ss broker server");
        }
        mainLoopThread.join();
#endif
    }

    std::unique_ptr<zmq::socket_t> zmqBrokerServer::loadZMQsocket(zmq::context_t& ctx)
    {
        std::string ext_interface = "tcp://*";
        int zmqport = DEFAULT_ZMQ_BROKER_PORT_NUMBER + 1;
        std::chrono::milliseconds timeout(20000);
        if (config_->isMember("zmq")) {
            auto V = (*config_)["zmq"];
            replaceIfMember(V, "interface", ext_interface);
            replaceIfMember(V, "port", zmqport);
        }
        auto repSocket = std::make_unique<zmq::socket_t>(ctx, ZMQ_REP);
        repSocket->setsockopt(ZMQ_LINGER, 500);
        auto bindsuccess = hzmq::bindzmqSocket(*repSocket, ext_interface, zmqport, timeout);
        if (!bindsuccess) {
            repSocket->close();
            logMessage("ZMQ server failed to start");
            return nullptr;
        }
        return repSocket;
    }

    std::unique_ptr<zmq::socket_t> zmqBrokerServer::loadZMQSSsocket(zmq::context_t& ctx)
    {
        std::string ext_interface = "tcp://*";
        int zmqport = DEFAULT_ZMQSS_BROKER_PORT_NUMBER;
        std::chrono::milliseconds timeout(20000);
        if (config_->isMember("zmqss")) {
            auto V = (*config_)["zmqss"];
            replaceIfMember(V, "interface", ext_interface);
            replaceIfMember(V, "port", zmqport);
        }
        auto repSocket = std::make_unique<zmq::socket_t>(ctx, ZMQ_ROUTER);
        repSocket->setsockopt(ZMQ_LINGER, 500);
        auto bindsuccess = hzmq::bindzmqSocket(*repSocket, ext_interface, zmqport, timeout);
        if (!bindsuccess) {
            repSocket->close();
            logMessage("ZMQSS server failed to start");
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

    std::string zmqBrokerServer::generateResponseToMessage(
        zmq::message_t& msg,
        portData& pdata,
        core_type ctype)
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
            auto rep = generateMessageResponse(rxcmd, pdata, ctype);
            if (rep.action() != CMD_IGNORE) {
                return rep.to_string();
            }
        }
        logMessage("received unknown message of length " + std::to_string(msg.size()));
        return "ignored";
    }
    void zmqBrokerServer::mainLoop()
    {
#ifdef ENABLE_ZMQ_CORE
        std::vector<std::unique_ptr<zmq::socket_t>> sockets;
        std::vector<zmqServerData> data;
        std::vector<std::function<void(zmq::socket_t*, portData&)>> handleMessage;

        auto ctx = ZmqContextManager::getContextPointer();

        if (zmq_enabled_) {
            sockets.push_back(loadZMQsocket(ctx->getContext()));
            data.push_back(generateZMQServerData());
            handleMessage.push_back([this](zmq::socket_t* skt, portData& pdata) {
                zmq::message_t msg;
                skt->recv(msg);
                std::string response = generateResponseToMessage(msg, pdata, core_type::ZMQ);
                skt->send(response);
            });
        }

        if (zmqss_enabled_) {
            sockets.push_back(loadZMQSSsocket(ctx->getContext()));
            data.push_back(generateZMQSSServerData());
            handleMessage.push_back([this](zmq::socket_t* skt, portData& pdata) {
                zmq::message_t msg1;
                zmq::message_t msg2;
                skt->recv(msg1); //should be null
                skt->recv(msg2);
                std::string response = generateResponseToMessage(msg2, pdata, core_type::ZMQ_SS);
                skt->send(msg1, zmq::send_flags::sndmore);
                skt->send(std::string{}, zmq::send_flags::sndmore);
                skt->send(response, zmq::send_flags::dontwait);
            });
        }

        std::vector<zmq::pollitem_t> poller;
        for (std::size_t ii = 0; ii < sockets.size(); ++ii) {
            poller.emplace_back();
            poller.back().socket = static_cast<void*>(*sockets[ii]);
            poller.back().events = ZMQ_POLLIN;
        }

        int rc = 0;
        while (rc >= 0) {
            try {
                rc = zmq::poll(poller, std::chrono::milliseconds(5000));
            }
            catch (const zmq::error_t& e) {
                logMessage(e.what());
                return;
            }
            if (rc < 0) {
                std::cerr << "ZMQ broker connection error (2)" << std::endl;
                break;
            }
            if (rc > 0) {
                zmq::message_t msg;
                for (std::size_t ii = 0; ii < poller.size(); ++ii) {
                    if ((poller[ii].revents & ZMQ_POLLIN) != 0) {
                        handleMessage[ii](sockets[ii].get(), data[ii].ports);
                    }
                }
            }
            if (exitAll.load()) {
                break;
            }
        }

        for (auto& skt : sockets) {
            skt->close();
        }
        sockets.clear();

#endif
    }
} // namespace apps
} // namespace helics
