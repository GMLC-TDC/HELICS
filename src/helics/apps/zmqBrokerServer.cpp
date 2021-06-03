/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "zmqBrokerServer.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "../network/NetworkBrokerData.hpp"
#include "../network/networkDefaults.hpp"
#include "helics/external/CLI11/CLI11.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#ifdef ENABLE_ZMQ_CORE
#    include "../network/zmq/ZmqCommsCommon.h"
#    include "../network/zmq/ZmqContextManager.h"
#    include "cppzmq/zmq.hpp"
#endif

static const Json::Value null;

namespace helics {
namespace apps {

    void zmqBrokerServer::processArgs(const std::string& args)

    {
        CLI::App parser("zmq broker server parser");
        parser.allow_extras();
        parser.add_option("--zmq_port", mZmqPort, "specify the zmq port to use");
        parser.add_option("--zmq_interface",
                          mZmqInterface,
                          "specify the interface to use for connecting the zmq broker server");

        try {
            parser.parse(args);
        }
        catch (const CLI::Error& ce) {
            logMessage(std::string("error processing command line arguments for web server :") +
                       ce.what());
        }
    }

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
        int port = (mZmqPort != 0) ? mZmqPort :
                                     ((zmq_enabled_) ? DEFAULT_ZMQ_BROKER_PORT_NUMBER + 1 :
                                                       DEFAULT_ZMQSS_BROKER_PORT_NUMBER);
        if (zmq_enabled_) {
            if (config_->isMember("zmq")) {
                auto V = (*config_)["zmq"];
                replaceIfMember(V, "interface", mZmqInterface);
                replaceIfMember(V, "port", port);
            }
        } else {
            if (config_->isMember("zmqss")) {
                auto V = (*config_)["zmqss"];
                replaceIfMember(V, "interface", mZmqInterface);
                replaceIfMember(V, "port", port);
            }
        }

        try {
            reqSocket.connect(helics::makePortAddress(mZmqInterface, port));
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

#ifdef ENABLE_ZMQ_CORE

    std::pair<std::unique_ptr<zmq::socket_t>, int>
        zmqBrokerServer::loadZMQsocket(zmq::context_t& ctx)
    {
        std::pair<std::unique_ptr<zmq::socket_t>, int> retval{nullptr,
                                                              DEFAULT_ZMQ_BROKER_PORT_NUMBER + 1};
        std::string ext_interface = "tcp://*";
        std::chrono::milliseconds timeout(20000);
        if (config_->isMember("zmq")) {
            auto V = (*config_)["zmq"];
            replaceIfMember(V, "interface", ext_interface);
            replaceIfMember(V, "port", retval.second);
        }
        retval.first = std::make_unique<zmq::socket_t>(ctx, ZMQ_REP);
        retval.first->setsockopt(ZMQ_LINGER, 500);
        auto bindsuccess =
            zeromq::bindzmqSocket(*retval.first, ext_interface, retval.second, timeout);
        if (!bindsuccess) {
            retval.first->close();
            retval.first.reset();
            retval.second = 0;
            logMessage("ZMQ server failed to start");
        }
        return retval;
    }

    std::pair<std::unique_ptr<zmq::socket_t>, int>
        zmqBrokerServer::loadZMQSSsocket(zmq::context_t& ctx)
    {
        std::pair<std::unique_ptr<zmq::socket_t>, int> retval{nullptr,
                                                              DEFAULT_ZMQSS_BROKER_PORT_NUMBER};
        std::string ext_interface = "tcp://*";
        std::chrono::milliseconds timeout(20000);
        if (config_->isMember("zmqss")) {
            auto V = (*config_)["zmqss"];
            replaceIfMember(V, "interface", ext_interface);
            replaceIfMember(V, "port", retval.second);
        }
        retval.first = std::make_unique<zmq::socket_t>(ctx, ZMQ_ROUTER);
        retval.first->setsockopt(ZMQ_LINGER, 500);
        auto bindsuccess =
            zeromq::bindzmqSocket(*retval.first, ext_interface, retval.second, timeout);
        if (!bindsuccess) {
            retval.first->close();
            retval.first.reset();
            retval.second = 0;
            logMessage("ZMQSS server failed to start");
        }
        return retval;
    }

    zmqBrokerServer::zmqServerData zmqBrokerServer::generateServerData(int portNumber, int skip)
    {
        zmqServerData pdata;
        for (int ii = 0; ii < 20; ++ii) {
            pdata.ports.emplace_back(portNumber + ii * skip, false, nullptr);
        }
        return pdata;
    }

    std::string zmqBrokerServer::generateResponseToMessage(zmq::message_t& msg,
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

#endif

    void zmqBrokerServer::mainLoop()
    {
#ifdef ENABLE_ZMQ_CORE
        std::vector<std::unique_ptr<zmq::socket_t>> sockets;
        std::vector<zmqServerData> data;
        std::vector<std::function<void(zmq::socket_t*, portData&)>> handleMessage;

        auto ctx = ZmqContextManager::getContextPointer();

        if (zmq_enabled_) {
            auto sdata = loadZMQsocket(ctx->getContext());
            sockets.push_back(std::move(sdata.first));
            data.push_back(generateServerData(sdata.second + 3, 2));
            handleMessage.emplace_back([this](zmq::socket_t* skt, portData& pdata) {
                zmq::message_t msg;
                skt->recv(msg);
                std::string response = generateResponseToMessage(msg, pdata, core_type::ZMQ);
                skt->send(response);
            });
        }

        if (zmqss_enabled_) {
            auto sdata = loadZMQSSsocket(ctx->getContext());
            sockets.push_back(std::move(sdata.first));
            data.push_back(generateServerData(sdata.second + 4, 1));
            handleMessage.emplace_back([this](zmq::socket_t* skt, portData& pdata) {
                zmq::message_t msg1;
                zmq::message_t msg2;
                skt->recv(msg1);  // should be null
                skt->recv(msg2);
                std::string response = generateResponseToMessage(msg2, pdata, core_type::ZMQ_SS);
                skt->send(msg1, zmq::send_flags::sndmore);
                skt->send(std::string{}, zmq::send_flags::sndmore);
                skt->send(response, zmq::send_flags::dontwait);
            });
        }

        std::vector<zmq::pollitem_t> poller;
        for (auto& socket : sockets) {
            poller.emplace_back();
            poller.back().socket = static_cast<void*>(*socket);
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
                    if (zmq::has_message(poller[ii])) {
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
}  // namespace apps
}  // namespace helics
