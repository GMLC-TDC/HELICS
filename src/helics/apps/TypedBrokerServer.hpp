/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "../core/ActionMessage.hpp"

#include <json/forwards.h>
#include <memory>

namespace helics {
class Broker;
namespace apps {

    using portData = std::vector<std::tuple<int, bool, std::shared_ptr<Broker>>>;
    /** a virtual class to use as a base for broker servers of various types*/
    class TypedBrokerServer {
      public:
        virtual ~TypedBrokerServer() = default;
        /** start the server*/
        virtual void startServer(const Json::Value* val) = 0;
        /** stop the server*/
        virtual void stopServer() = 0;

      protected:
        /** generate a reply to a message*/
        static ActionMessage
            generateMessageResponse(const ActionMessage& rxcmd, portData& pdata, core_type ctype);
        /** get an open port for broker to start*/
        static int getOpenPort(portData& pd);
        /* assign a port in the portData structure*/
        static void assignPort(portData& pd, int pnumber, std::shared_ptr<Broker>& brk);
        /* log a message to the console */
        static void logMessage(std::string message);
    };
} // namespace apps
} // namespace helics
