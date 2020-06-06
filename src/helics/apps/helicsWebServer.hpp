/*
Copyright (c) 2017-2020,
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

namespace helics {
namespace apps {

    class IocWrapper;

    /** a virtual class to use as a base for broker servers of various types*/
    class WebServer: public TypedBrokerServer {
      public:
        WebServer() = default;
        explicit WebServer(std::string server_name): name_(std::move(server_name)) {}
        /** start the server*/
        virtual void startServer(const Json::Value* val) override;
        /** stop the server*/
        virtual void stopServer() override;
        /** enable the HTTP server*/
        void enableHttpServer(bool enabled) { http_enabled_ = enabled; }
        /** enable the websocket server*/
        void enableWebSocketServer(bool enabled) { websocket_enabled_ = enabled; }

      private:
        void mainLoop();

        std::atomic<bool> running{false};
        std::shared_ptr<IocWrapper> context;
        std::thread mainLoopThread;
        std::mutex threadGuard;

        const Json::Value* config{nullptr};
        const std::string name_;
        std::string httpAddress_{"127.0.0.1"};
        int httpPort_{80};
        std::string websocketAddress_{"127.0.0.1"};
        int websocketPort_{80};
        bool http_enabled_{false};
        bool websocket_enabled_{false};
        std::atomic<bool> executing{false};
    };
}  // namespace apps
}  // namespace helics
