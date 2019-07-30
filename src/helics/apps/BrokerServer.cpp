/*
Copyright � 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "BrokerServer.hpp"

#include "../core/NetworkBrokerData.hpp"
#include "../core/helicsCLI11.hpp"

using namespace std::string_literals;

namespace helics
{
BrokerServer::BrokerServer () noexcept { loadComms (); }

BrokerServer::BrokerServer (int /*argc*/, char * /*argv*/[]) { loadComms (); }

BrokerServer::BrokerServer (const std::string & /*configFile*/) { loadComms (); }

void BrokerServer::loadComms ()
{
    masterComm = generateComms ("def");
    masterComm->setCallback ([this] (ActionMessage &&M) { BrokerBase::addActionMessage (std::move (M)); });
}

BrokerServer::~BrokerServer ()
{
    BrokerBase::haltOperations = true;
    int exp = 2;
    while (!disconnectionStage.compare_exchange_weak (exp, 3))
    {
        if (exp == 0)
        {
            commDisconnect ();
            exp = 1;
        }
        else
        {
            std::this_thread::sleep_for (std::chrono::milliseconds (50));
        }
    }
    masterComm = nullptr;  // need to ensure the comms are deleted before the callbacks become invalid
    BrokerBase::joinAllThreads ();
}

void BrokerServer::brokerDisconnect () { commDisconnect (); }

void BrokerServer::commDisconnect ()
{
    int exp = 0;
    if (disconnectionStage.compare_exchange_strong (exp, 1))
    {
        masterComm->disconnect ();
        disconnectionStage = 2;
    }
}

bool BrokerServer::tryReconnect () { return masterComm->reconnect (); }

void BrokerServer::transmit (route_id rid, const ActionMessage &cmd) { masterComm->transmit (rid, cmd); }

void BrokerServer::transmit (route_id rid, ActionMessage &&cmd) { masterComm->transmit (rid, std::move (cmd)); }

void BrokerServer::addRoute (route_id rid, const std::string &routeInfo) { masterComm->addRoute (rid, routeInfo); }

}  // namespace helics
