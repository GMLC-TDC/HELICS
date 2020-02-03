/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "MultiBroker.hpp"

#include "../core/CommsInterface.hpp"

#include <atomic>
#include <mutex>
#include <thread>
#ifdef ENABLE_UDP_CORE
#    include "../core/udp/UdpComms.h"
#endif
#ifdef ENABLE_TCP_CORE
#    include "../core/tcp/TcpComms.h"
#    include "../core/tcp/TcpCommsSS.h"
#endif
#ifdef ENABLE_ZMQ_CORE
#    include "../core/zmq/ZmqComms.h"
#    include "../core/zmq/ZmqCommsSS.h"
#endif
#ifdef ENABLE_MPI_CORE
#    include "../core/mpi/MpiComms.h"
#endif
#ifdef ENABLE_IPC_CORE
#    include "../core/ipc/IpcComms.h"
#endif
#include "../core/NetworkBrokerData.hpp"
#include "../core/helicsCLI11.hpp"

using namespace std::string_literals;

namespace helics {
static void loadTypeSpecificArgs(
    helics::core_type ctype,
    CommsInterface* comm,
    std::vector<std::string> args)
{
    if (comm == nullptr) {
        return;
    }
    switch (ctype) {
#ifdef ENABLE_TCP_CORE
        case core_type::TCP_SS: {
            auto cm = dynamic_cast<tcp::TcpCommsSS*>(comm);
            helicsCLI11App tsparse;
            tsparse.add_option_function<std::vector<std::string>>(
                "--connections",
                [cm](const std::vector<std::string>& conns) { cm->addConnections(conns); },
                "target link connections");
            tsparse.allow_extras();
            tsparse.helics_parse(std::move(args));
        } break;
#endif
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
    switch (ctype) {
        case core_type::TCP:
#ifdef ENABLE_TCP_CORE
            comm = std::make_unique<tcp::TcpComms>();
#endif
            break;
        case core_type::DEFAULT:
        case core_type::ZMQ:
#ifdef ENABLE_ZMQ_CORE
            comm = std::make_unique<zeromq::ZmqComms>();
#endif
            break;
        case core_type::ZMQ_SS:
#ifdef ENABLE_ZMQ_CORE
            comm = std::make_unique<zeromq::ZmqCommsSS>();
#endif
            break;
        case core_type::TCP_SS:
#ifdef ENABLE_TCP_CORE
            comm = std::make_unique<tcp::TcpCommsSS>();
            loadTypeSpecificArgs(ctype, comm.get(), parser->remaining_for_passthrough());
#endif
            break;
        case core_type::UDP:
#ifdef ENABLE_UDP_CORE
            comm = std::make_unique<udp::UdpComms>();
#endif
            break;
        case core_type::IPC:
        case core_type::INTERPROCESS:
#ifdef ENABLE_IPC_CORE
            comm = std::make_unique<ipc::IpcComms>();
#endif
            break;
        case core_type::MPI:
#ifdef ENABLE_MPI_CORE
            comm = std::make_unique<mpi::MpiComms>();
            break;
#endif
        case core_type::HTTP:
        case core_type::TEST:
        case core_type::INPROC:
        case core_type::NNG:
        case core_type::UNRECOGNIZED:
        case core_type::WEBSOCKET:
            break;
    }
    if (comm) {
        comm->loadNetworkInfo(nbdata);
    }
    return comm;
}

MultiBroker::MultiBroker() noexcept
{
    loadComms();
}

MultiBroker::MultiBroker(int /*argc*/, char* /*argv*/[])
{
    loadComms();
}

MultiBroker::MultiBroker(const std::string& /*configFile*/)
{
    loadComms();
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
        masterComm->disconnect();
        disconnectionStage = 2;
    }
}

bool MultiBroker::tryReconnect()
{
    return masterComm->reconnect();
}

void MultiBroker::transmit(route_id rid, const ActionMessage& cmd)
{
    masterComm->transmit(rid, cmd);
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
