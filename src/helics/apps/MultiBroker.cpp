/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "MultiBroker.hpp"

#include "../network/CommsInterface.hpp"

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "../core/helicsCLI11.hpp"
#include "../network/NetworkBrokerData.hpp"

#ifdef ENABLE_TCP_CORE
#include "../network/tcp/TcpCommsSS.h"
#endif 

namespace helics {
static void loadTypeSpecificArgs(
    helics::core_type ctype,
    CommsInterface* comm,
    std::vector<std::string> args)
{
    if (comm == nullptr) {
        return;
    }
    (void)args;
    switch (ctype) {
    case core_type::TCP_SS:
#ifdef ENABLE_TCP_CORE
         {
            auto cm = dynamic_cast<tcp::TcpCommsSS*>(comm);
            helicsCLI11App tsparse;
            tsparse.add_option_function<std::vector<std::string>>(
                "--connections",
                [cm](const std::vector<std::string>& conns) { cm->addConnections(conns); },
                "target link connections");
            tsparse.allow_extras();
            tsparse.helics_parse(std::move(args));
        }    
#endif
        break;
        case core_type::MPI:
            break;
        default:
            break;
    }
}

static std::unique_ptr<CommsInterface>
    generateComms(const std::string& type, const std::string& initString = std::string{})
{
    auto ctype = coreTypeFromString(type);

    NetworkBrokerData nbdata;
    auto parser = nbdata.commandLineParser("localhost");
    parser->helics_parse(initString);

    std::unique_ptr<CommsInterface> comm;
    
    if (comm) {
        comm->loadNetworkInfo(nbdata);
    }
    return comm;
}

MultiBroker::MultiBroker(const std::string &brokerName, int argc, char* argv[]):CoreBroker(brokerName)
{
    configureFromArgs(argc, argv);
}

MultiBroker::MultiBroker(const std::string &brokerName, const std::string& configString):CoreBroker(brokerName)
{
    configure(configString);
}

MultiBroker::MultiBroker() noexcept
{
    
}

MultiBroker::MultiBroker(int argc, char* argv[]) :MultiBroker(std::string{},argc,argv)
{

}

MultiBroker::MultiBroker(const std::string& configFile) : MultiBroker(std::string{},configFile)
{
    
}



void MultiBroker::loadComms()
{
    masterComm = generateComms("def");
    masterComm->setCallback(
        [this](ActionMessage&& M) { BrokerBase::addActionMessage(std::move(M)); });
}

MultiBroker::~MultiBroker()
{
    BrokerBase::haltOperations = true;
    int exp = 2;
    while (!disconnectionStage.compare_exchange_weak(exp, 3)) {
        if (exp == 0) {
            commDisconnect();
            exp = 1;
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    masterComm =
        nullptr; // need to ensure the comms are deleted before the callbacks become invalid
    BrokerBase::joinAllThreads();
}

void MultiBroker::brokerDisconnect()
{
    commDisconnect();
}

void MultiBroker::commDisconnect()
{
    int exp = 0;
    if (disconnectionStage.compare_exchange_strong(exp, 1)) {
        if (masterComm)
        {
            masterComm->disconnect();
        }
        for (auto &comm : comms)
        {
            comm->disconnect();
        }
        disconnectionStage = 2;
    }
}

bool MultiBroker::tryReconnect()
{
    return masterComm->reconnect();
}

void MultiBroker::transmit(route_id rid, const ActionMessage& cmd)
{
    if (rid == parent_route_id)
    {
        if (masterComm)
        {
            masterComm->transmit(rid, cmd);
            return;
        }
        else
        {
            
        }
    }
   
}

void MultiBroker::transmit(route_id rid, ActionMessage&& cmd)
{
    masterComm->transmit(rid, std::move(cmd));
}

void MultiBroker::addRoute(route_id rid, const std::string& routeInfo)
{
    masterComm->addRoute(rid, routeInfo);
}

} // namespace helics
