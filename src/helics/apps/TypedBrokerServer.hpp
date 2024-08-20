/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "../core/ActionMessage.hpp"

#include <memory>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <tuple>
#include <vector>

namespace helics {
class Broker;
namespace apps {

    using portData = std::vector<std::tuple<int, bool, std::shared_ptr<Broker>>>;
    /** a virtual class to use as a base for broker servers of various types*/
    class TypedBrokerServer {
      public:
        virtual ~TypedBrokerServer() = default;
        /** start the server, the server may require a shared pointer to keep the data alive */
        virtual void startServer(const nlohmann::json* val,
                                 const std::shared_ptr<TypedBrokerServer>& ptr) = 0;
        /** stop the server*/
        virtual void stopServer() = 0;
        /** process some potential command line arguments for the typed server*/
        virtual void processArgs(std::string_view args);

      protected:
        /** generate a reply to a message*/
        static ActionMessage
            generateMessageResponse(const ActionMessage& rxcmd, portData& pdata, CoreType ctype);
        /** get an open port for broker to start*/
        static int getOpenPort(portData& pd);
        /* assign a port in the portData structure*/
        static void assignPort(portData& pd, int pnumber, std::shared_ptr<Broker>& brk);
        /* log a message to the console */
        static void logMessage(std::string_view message);
    };
}  // namespace apps
}  // namespace helics
