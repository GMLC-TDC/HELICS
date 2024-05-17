/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "TypedBrokerServer.hpp"

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>

namespace helics::apps {

class IocWrapper;

/** a virtual class to use as a base for broker servers of various types*/
class WebServer: public TypedBrokerServer {
  public:
    /// @brief default port for the HTTP web server
    static constexpr int defaultHttpPort{43542};
    /// @brief default port for the Websocket server
    static constexpr int defaultWebSocketPort{43543};
    WebServer() = default;
    explicit WebServer(std::string_view server_name): mName(server_name) {}
    /** start the server*/
    virtual void startServer(const nlohmann::json* val,
                             const std::shared_ptr<TypedBrokerServer>& ptr) override;
    /** stop the server*/
    virtual void stopServer() override;
    /** process any command line arguments*/
    virtual void processArgs(std::string_view args) override;
    /** enable the HTTP server*/
    void enableHttpServer(bool enabled) { mHttpEnabled = enabled; }
    /** enable the websocket server*/
    void enableWebSocketServer(bool enabled) { mWebsocketEnabled = enabled; }

  private:
    void mainLoop(std::shared_ptr<WebServer> keepAlive);
    std::atomic<bool> running{false};
    std::shared_ptr<IocWrapper> context;
    std::thread mainLoopThread;
    std::mutex threadGuard;

    const nlohmann::json* config{nullptr};
    const std::string mName;
    std::string mArgs;
    std::string mHttpAddress;
    int mHttpPort{defaultHttpPort};
    std::string mWebsocketAddress;
    int mWebsocketPort{defaultWebSocketPort};
    bool mHttpEnabled{false};
    bool mWebsocketEnabled{false};
    int mInterfaceNetwork{0};
    std::atomic<bool> executing{false};
};
}  // namespace helics::apps
