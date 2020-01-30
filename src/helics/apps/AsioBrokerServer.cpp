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
        void AsioBrokerServer::startServer(const Json::Value *val)
        {
            std::cerr << "starting asio broker server\n";
            config_ = val;

            std::lock_guard<std::mutex> tlock(threadGuard);
            mainLoopThread = std::thread([this]() { mainLoop(); });
        }

        void AsioBrokerServer::stopServer()
        {
            std::lock_guard<std::mutex> tlock(threadGuard);
            mainLoopThread.join();
        }

        void AsioBrokerServer::mainLoop()
        {

        }
    }
}
