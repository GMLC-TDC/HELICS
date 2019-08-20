/*
Copyright � 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "BrokerServer.hpp"

#include "../common/JsonProcessingFunctions.hpp"

#include "../core/ActionMessage.hpp"
#include "../core/BrokerFactory.hpp"
#include "../core/NetworkBrokerData.hpp"
#include "../core/helicsCLI11.hpp"
#include "../core/networkDefaults.hpp"

#ifdef ENABLE_ZMQ_CORE
#include "../common/zmqContextManager.h"
#include "../core/zmq/ZmqCommsCommon.h"
#endif

namespace helics
{
namespace apps
{
BrokerServer::BrokerServer () noexcept : zmq_server{true} {}

BrokerServer::BrokerServer (int argc, char *argv[])
{
    auto app = generateArgProcessing ();
    app->parse (argc, argv);
}

BrokerServer::BrokerServer (std::vector<std::string> args)
{
    auto app = generateArgProcessing ();
    app->parse (args);
}

BrokerServer::BrokerServer (const std::string &configFile) : configFile_ (configFile) {}

BrokerServer::~BrokerServer () {}

void BrokerServer::startServers ()
{
    if (!configFile_.empty ())
    {
        config_ = std::make_unique<Json::Value> (loadJson (configFile_));
    }
    else
    {
        config_ = std::make_unique<Json::Value> ();
    }

#ifdef ENABLE_ZMQ_CORE
    if (zmq_server)
    {
        startZMQserver ();
    }
    if (zmq_ss_server)
    {
    }
#endif
}

bool BrokerServer::hasActiveBrokers () const { return false; }
/** force terminate all running brokers*/
void BrokerServer::forceTerminate () { closeServers (); }

void BrokerServer::closeServers ()
{
#ifdef ENABLE_ZMQ_CORE
    if (zmq_server)
    {
        closeZMQserver ();
    }
#endif
    exitall.store (false);
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

    app->add_option ("config,--config,--server-config", configFile_, "load a config file for the broker server");
    return app;
}

void BrokerServer::startZMQserver ()
{
#ifdef ENABLE_ZMQ_CORE
    std::string ext_interface = "*";
    int port = DEFAULT_ZMQ_BROKER_PORT_NUMBER;
    std::chrono::milliseconds timeout (20000);
    if (config_->isMember ("zmq"))
    {
        auto V = (*config_)["zmq"];
        replaceIfMember (V, "interface", ext_interface);
        replaceIfMember (V, "port", port);
    }
    auto ctx = ZmqContextManager::getContextPointer ();
    zmq::socket_t repSocket (ctx->getContext (), ZMQ_REP);
    repSocket.setsockopt (ZMQ_LINGER, 500);
    auto bindsuccess = hzmq::bindzmqSocket (repSocket, ext_interface, port, timeout);
    if (!bindsuccess)
    {
        repSocket.close ();
        return;
    }
#endif
}

void BrokerServer::closeZMQserver ()
{
#ifdef ENABLE_ZMQ_CORE
    auto ctx = ZmqContextManager::getContextPointer ();
    zmq::socket_t reqSocket (ctx->getContext (), ZMQ_REQ);

    std::string ext_interface = "*";
    int port = DEFAULT_ZMQ_BROKER_PORT_NUMBER;
    std::chrono::milliseconds timeout (20000);
    if (config_->isMember ("zmq"))
    {
        auto V = (*config_)["zmq"];
        replaceIfMember (V, "interface", ext_interface);
        replaceIfMember (V, "port", port);
    }
    try
    {
        reqSocket.connect (helics::makePortAddress (ext_interface, port));
        reqSocket.send ("close");
        zmq::message_t recv;
        reqSocket.recv (&recv);
    }
    catch (const zmq::error_t &)
    {
    }

#endif
}
}  // namespace apps
}  // namespace helics
