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

#ifdef HELICS_ENABLE_ZMQ_CORE
namespace zmq {
class socket_t;
class message_t;
class context_t;
}  // namespace zmq

#endif

namespace helics {
class Broker;
namespace apps {

    /** a virtual class to use as a base for broker servers of various types*/
    class zmqBrokerServer: public TypedBrokerServer {
      public:
        zmqBrokerServer() = default;
        explicit zmqBrokerServer(std::string_view server_name): name_(server_name) {}
        /** start the server*/
        virtual void startServer(const nlohmann::json* val,
                                 const std::shared_ptr<TypedBrokerServer>& ptr) override;
        /** stop the server*/
        virtual void stopServer() override;

        virtual void processArgs(std::string_view args) override;
        void enableZmqServer(bool enabled) { zmq_enabled_ = enabled; }
        void enableZmqSsServer(bool enabled) { zmqss_enabled_ = enabled; }

      private:
        struct zmqServerData {
            portData ports;
        };

        void mainLoop();
#ifdef HELICS_ENABLE_ZMQ_CORE
        std::pair<std::unique_ptr<zmq::socket_t>, int> loadZMQsocket(zmq::context_t& ctx);
        std::pair<std::unique_ptr<zmq::socket_t>, int> loadZMQSSsocket(zmq::context_t& ctx);

        static zmqServerData generateServerData(int portNumber, int skip);

        std::string generateResponseToMessage(zmq::message_t& msg, portData& pdata, CoreType ctype);
#endif
        std::thread mainLoopThread;
        std::mutex threadGuard;
        const nlohmann::json* config_{nullptr};
        const std::string name_;
        bool zmq_enabled_{false};
        bool zmqss_enabled_{false};
        std::atomic_bool exitAll{false};
        int mZmqPort{0};
        std::string mZmqInterface{"tcp://127.0.0.1"};
    };
}  // namespace apps
}  // namespace helics
