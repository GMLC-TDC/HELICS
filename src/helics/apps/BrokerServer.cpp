/*
Copyright � 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "BrokerServer.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "../common/zmqContextManager.h"
#include "../core/ActionMessage.hpp"
#include "../core/NetworkBrokerData.hpp"
#include "../core/helicsCLI11.hpp"
#include "cppzmq/zmq.hpp"

namespace helics
{
BrokerServer::BrokerServer () noexcept : zmq_server{true} {}

BrokerServer::BrokerServer (int argc, char *argv[])
{
    auto app = generateArgProcessing ();
    app->parse (argc, argv);
}

BrokerServer::BrokerServer (const std::string &configFile) : configFile_ (configFile) {}

BrokerServer::~BrokerServer () {}

void BrokerServer::startServers ()
{
    if (zmq_server)
    {
        startZMQserver ();
    }
}

std::unique_ptr<helicsCLI11App> BrokerServer::generateArgProcessing ()
{
    auto app = std::make_unique<helicsCLI11App> (
      "The Broker server is a helics broker coordinator that can generate brokers on request", "broker_server");

    app->add_flag ("--zmq,-z", zmq_server, "start a broker-server for the zmq comms in helics");
    app->add_flag ("--zmqss", zmq_ss_server, "start a broker-server for the zmq single socket comms in helics");
    app->add_flag ("--tcp,-t", tcp_server, "start a broker-server for the tcp comms in helics");
    app->add_flag ("--udp,-u", udp_server, "start a broker-server for the udp comms in helics");
    app->add_flag ("--mpi", mpi_server, "start a broker-server for the mpi comms in helics");

    app->add_option ("--config,--config-file", configFile_, "load a config file for the broker server");
    return app;
}

void BrokerServer::startZMQserver () {}
}  // namespace helics
