/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "BrokerServer.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "../core/ActionMessage.hpp"
#include "../core/BrokerFactory.hpp"
#include "../core/helicsCLI11.hpp"
#include "AsioBrokerServer.hpp"
#include "gmlc/utilities/stringOps.h"
#include "zmqBrokerServer.hpp"

#include <utility>
#ifdef HELICS_ENABLE_WEBSERVER
#    include "helicsWebServer.hpp"
#endif

namespace helics {
namespace apps {
    BrokerServer::BrokerServer() noexcept:
        zmq_server{true}, server_name_{gmlc::utilities::randomString(5)}
    {
    }

    BrokerServer::BrokerServer(int argc, char* argv[]):
        server_name_{gmlc::utilities::randomString(5)}
    {
        auto app = generateArgProcessing();
        app->helics_parse(argc, argv);
    }

    BrokerServer::BrokerServer(std::vector<std::string> args):
        server_name_{gmlc::utilities::randomString(5)}
    {
        auto app = generateArgProcessing();
        app->helics_parse(args);
    }

    BrokerServer::BrokerServer(const std::string& configFile):
        configFile_(configFile), server_name_{gmlc::utilities::randomString(5)}
    {
    }

    BrokerServer::~BrokerServer() { closeServers(); }

    void BrokerServer::startServers()
    {
        if (!configFile_.empty()) {
            config_ = std::make_unique<Json::Value>(loadJson(configFile_));
        } else {
            config_ = std::make_unique<Json::Value>();
        }
        if (zmq_server || zmq_ss_server) {
            auto zmqs = std::make_unique<zmqBrokerServer>(server_name_);
            if (zmq_server) {
                zmqs->enableZmqServer(true);
            }
            if (zmq_ss_server) {
                zmqs->enableZmqSsServer(true);
            }
            if (!mZmqArgs.empty()) {
                zmqs->processArgs(mZmqArgs);
            }
            servers.push_back(std::move(zmqs));
        }
        if (tcp_server || udp_server) {
            auto asios = std::make_unique<AsioBrokerServer>(server_name_);
            if (tcp_server) {
                asios->enableTcpServer(true);
                if (!mTcpArgs.empty()) {
                    asios->processArgs(mTcpArgs);
                }
            }
            if (udp_server) {
                asios->enableUdpServer(true);
                if (!mUdpArgs.empty()) {
                    asios->processArgs(mUdpArgs);
                }
            }
            servers.push_back(std::move(asios));
        }

        if (http_server || websocket_server) {
#ifdef HELICS_ENABLE_WEBSERVER
            auto webs = std::make_unique<WebServer>(server_name_);
            if (http_server) {
                webs->enableHttpServer(true);
                if (!mHttpArgs.empty()) {
                    webs->processArgs(mHttpArgs);
                }
            }
            if (websocket_server) {
                webs->enableWebSocketServer(true);
                if (!mWebSocketArgs.empty()) {
                    webs->processArgs(mWebSocketArgs);
                }
            }
            servers.push_back(std::move(webs));
#else
            std::cout << "Webserver not enabled" << std::endl;
#endif
        }
        for (auto& server : servers) {
            server->startServer(config_.get());
        }
    }

    bool BrokerServer::hasActiveBrokers() { return BrokerFactory::brokersActive(); }
    /** force terminate all running brokers*/
    void BrokerServer::forceTerminate()
    {
        closeServers();
        auto brokerList = BrokerFactory::getAllBrokers();
        for (auto& brk : brokerList) {
            if (!brk) {
                continue;
            }
            if (brk->isConnected()) {
                brk->disconnect();
            }
        }
    }

    void BrokerServer::closeServers()
    {
        for (auto& server : servers) {
            server->stopServer();
        }
        servers.clear();
    }

    std::unique_ptr<helicsCLI11App> BrokerServer::generateArgProcessing()
    {
        auto app = std::make_unique<helicsCLI11App>(
            "The Broker server is a helics broker coordinator that can generate brokers on request",
            "broker_server");
#ifdef ENABLE_ZMQ_CORE
        app->add_flag("--zmq,-z", zmq_server, "start a broker-server for the zmq comms in helics");
        app->add_flag("--zmqss",
                      zmq_ss_server,
                      "start a broker-server for the zmq single socket comms in helics");
        app->add_option("--zmq_server_args",
                        mZmqArgs,
                        "command line arguments for the zmq servers");

#endif
#ifdef ENABLE_TCP_CORE
        app->add_flag("--tcp,-t", tcp_server, "start a broker-server for the tcp comms in helics");
        app->add_option("--tcp_server_args", mTcpArgs, "command line arguments for the tcp server");
#endif
#ifdef ENABLE_UDP_CORE
        app->add_flag("--udp,-u", udp_server, "start a broker-server for the udp comms in helics");
        app->add_option("--udp_server_args", mUdpArgs, "command line arguments for the udp server");
#endif
#ifdef ENABLE_MPI_CORE
        // app->add_flag("--mpi", mpi_server, "start a broker-server for the mpi comms in helics");
#endif
#ifdef HELICS_ENABLE_WEBSERVER
        app->add_flag("--http,--web",
                      http_server,
                      "start a webserver to respond to http rest api requests");
        app->add_flag("--websocket",
                      websocket_server,
                      "start a websocket to respond to api requests");
        app->add_option("--http_server_args",
                        mHttpArgs,
                        "command line arguments for the http server");
        app->add_option("--websocket_server_args",
                        mWebSocketArgs,
                        "command line arguments for the websocket server");
#endif
        app->set_config();
        app->add_option("config,--config,--server-config",
                        configFile_,
                        "load a config file for the broker server");
        return app;
    }

}  // namespace apps
}  // namespace helics
