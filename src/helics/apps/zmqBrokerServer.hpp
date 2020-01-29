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

namespace helics {
    class Broker;
    namespace apps {

        /** a virtual class to use as a base for broker servers of various types*/
        class zmqBrokerServer :public TypedBrokerServer
        {
        public:
            zmqBrokerServer() = default;
            /** start the server*/
            virtual void startServer(const Json::Value *val) override;
            /** stop the server*/
            virtual void stopServer() override;
        private:
            void mainLoop();

            std::thread mainLoopThread;
            std::mutex threadGuard;
            const Json::Value *config_{ nullptr };
        };
    }
}
