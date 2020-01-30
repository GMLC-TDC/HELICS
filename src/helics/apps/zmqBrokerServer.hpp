/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once


#include "TypedBrokerServer.hpp"
#include <thread>
#include <mutex>
#include <atomic>

namespace zmq
{
    class socket_t;
    class message_t;
}

namespace helics {
    class Broker;
    namespace apps {
        
        /** a virtual class to use as a base for broker servers of various types*/
        class zmqBrokerServer :public TypedBrokerServer
        {
        public:
            zmqBrokerServer() = default;
            zmqBrokerServer(std::string server_name) :name_(std::move(server_name)) {}
            /** start the server*/
            virtual void startServer(const Json::Value *val) override;
            /** stop the server*/
            virtual void stopServer() override;
            void enableZmqServer(bool enabled) { zmq_enabled_ = enabled; }
            void enableZmqSsServer(bool enabled) { zmqss_enabled_ = enabled; };
        private:
            struct zmqServerData
            {
                portData ports;
            };

            void mainLoop();
            std::unique_ptr<zmq::socket_t> zmqBrokerServer::loadZMQsocket();
            std::unique_ptr<zmq::socket_t> zmqBrokerServer::loadZMQSSsocket();

            zmqServerData generateZMQServerData();
            zmqServerData generateZMQSSServerData();

            std::string generateResponseToMessage(zmq::message_t &msg, portData &pdata);

            std::thread mainLoopThread;
            std::mutex threadGuard;
            const Json::Value *config_{ nullptr };
            const std::string name_;
            bool zmq_enabled_{ false };
            bool zmqss_enabled_{ false };
            std::atomic_bool exitAll{ false };
        };
    }
}
