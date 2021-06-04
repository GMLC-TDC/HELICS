/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "../core/Broker.hpp"

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

namespace Json {
class Value;
}

namespace helics {
class ActionMessage;
class helicsCLI11App;

using portData = std::vector<std::tuple<int, bool, std::shared_ptr<Broker>>>;

namespace apps {
    class TypedBrokerServer;
    /** helper class defining some common functionality for brokers and cores that use different
communication methods*/
    class BrokerServer {
      protected:
      public:
        /** default constructor*/
        BrokerServer() noexcept;
        /** construct from command line arguments
    @param argc the number of arguments
    @param argv the strings in the input
    */
        BrokerServer(int argc, char* argv[]);
        /** construct from command line arguments contained in a vector
    @param args the number of arguments
    */
        explicit BrokerServer(std::vector<std::string> args);
        /** construct from command line arguments parsed as a single string
    @param configFile a configuration file for the broker Server
    */
        explicit BrokerServer(const std::string& configFile);
        /** destructor*/
        ~BrokerServer();
        /** start the broker servers*/
        void startServers();
        /** check if there are any active Brokers running*/
        static bool hasActiveBrokers();
        /** force terminate all running brokers*/
        void forceTerminate();
        /** close the broker server from creating new brokers*/
        void closeServers();

      private:
        /** generate an argument processing app*/
        std::unique_ptr<helicsCLI11App> generateArgProcessing();

      private:
        bool zmq_server{false};  //!< activate the ZMQ broker server
        bool zmq_ss_server{false};  //!< activate the ZMQ SS broker server
        bool tcp_server{false};  //!< activate the TCP broker server
        bool udp_server{false};  //!< activate the UDP broker server
        // bool mpi_server{false}; //!< activate the MPI broker server
        bool http_server{false};  //!< activate the HTTP web server REST API
        bool websocket_server{false};  //!< activate the websocket API
        std::atomic<bool> exitall{false};
        std::vector<std::unique_ptr<TypedBrokerServer>> servers;
        std::string configFile_;
        std::string server_name_;
        std::unique_ptr<Json::Value> config_;
        std::string mHttpArgs;
        std::string mWebSocketArgs;
        std::string mZmqArgs;
        std::string mTcpArgs;
        std::string mUdpArgs;
        std::string mMpiArgs;

      public:
    };
}  // namespace apps
}  // namespace helics
