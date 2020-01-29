/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once


#include "TypedBrokerServer.hpp"

namespace helics {
    class Broker;
    namespace apps {

        /** a virtual class to use as a base for broker servers of various types*/
        class tcpBrokerServer :public TypedBrokerServer
        {
        public:
            tcpBrokerServer() = default;
            /** start the server*/
            virtual void startServer(const Json::Value &val);
            /** stop the server*/
            virtual void stopServer();
        };
    }
}
