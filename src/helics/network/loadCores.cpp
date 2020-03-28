/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "loadCores.hpp"

#include "../core/BrokerFactory.hpp"
#include "../core/CoreFactory.hpp"
#include "helics/helics-config.h"

#ifdef ENABLE_IPC_CORE
#    include "ipc/IpcBroker.h"
#    include "ipc/IpcCore.h"
#endif

#ifdef ENABLE_UDP_CORE
#    include "udp/UdpBroker.h"
#    include "udp/UdpCore.h"
#endif

#ifdef ENABLE_TEST_CORE
#    include "test/TestBroker.h"
#    include "test/TestCore.h"
#endif

#ifdef ENABLE_ZMQ_CORE
#    include "zmq/ZmqBroker.h"
#    include "zmq/ZmqCore.h"
#endif

#ifdef ENABLE_TCP_CORE
#    include "tcp/TcpBroker.h"
#    include "tcp/TcpCore.h"
#endif

#ifdef ENABLE_INPROC_CORE
#    include "inproc/InprocBroker.h"
#    include "inproc/InprocCore.h"
#endif

#ifdef ENABLE_MPI_CORE
#    include "mpi/MpiBroker.h"
#    include "mpi/MpiCore.h"
#endif

namespace helics {

//order determines priority here

#ifdef ENABLE_ZMQ_CORE
static auto zmqc =
    CoreFactory::addCoreType<zeromq::ZmqCore>("zmq", static_cast<int>(core_type::ZMQ));
static auto zmqb =
    BrokerFactory::addBrokerType<zeromq::ZmqBroker>("zmq", static_cast<int>(core_type::ZMQ));
static auto zmqssc =
    CoreFactory::addCoreType<zeromq::ZmqCoreSS>("zmqss", static_cast<int>(core_type::ZMQ_SS));
static auto zmqssb =
    BrokerFactory::addBrokerType<zeromq::ZmqBrokerSS>("zmqss", static_cast<int>(core_type::ZMQ_SS));
#endif

#ifdef ENABLE_TCP_CORE
static auto tcpc = CoreFactory::addCoreType<tcp::TcpCore>("tcp", static_cast<int>(core_type::TCP));
static auto tcpb =
    BrokerFactory::addBrokerType<tcp::TcpBroker>("tcp", static_cast<int>(core_type::TCP));
static auto tcpssc =
    CoreFactory::addCoreType<tcp::TcpCoreSS>("tcpss", static_cast<int>(core_type::TCP_SS));
static auto tcpssb =
    BrokerFactory::addBrokerType<tcp::TcpBrokerSS>("tcpss", static_cast<int>(core_type::TCP_SS));
#endif

#ifdef ENABLE_MPI_CORE
static auto mpic = CoreFactory::addCoreType<mpi::MpiCore>("mpi", static_cast<int>(core_type::MPI));
static auto mpib =
    BrokerFactory::addBrokerType<mpi::MpiBroker>("mpi", static_cast<int>(core_type::MPI));
#endif

#ifdef ENABLE_UDP_CORE
static auto udpc = CoreFactory::addCoreType<udp::UdpCore>("udp", static_cast<int>(core_type::UDP));
static auto udpb =
    BrokerFactory::addBrokerType<udp::UdpBroker>("udp", static_cast<int>(core_type::UDP));
#endif

#ifdef ENABLE_IPC_CORE
static auto ipcc = CoreFactory::addCoreType<ipc::IpcCore>("ipc", static_cast<int>(core_type::IPC));
static auto ipcb =
    BrokerFactory::addBrokerType<ipc::IpcBroker>("ipc", static_cast<int>(core_type::IPC));
static auto ipcc2 = CoreFactory::addCoreType<ipc::IpcCore>(
    "interprocess",
    static_cast<int>(core_type::INTERPROCESS));
static auto ipcb2 = BrokerFactory::addBrokerType<ipc::IpcBroker>(
    "interprocess",
    static_cast<int>(core_type::INTERPROCESS));
#endif

#ifdef ENABLE_INPROC_CORE
static auto iprcc =
    CoreFactory::addCoreType<inproc::InprocCore>("inproc", static_cast<int>(core_type::INPROC));
static auto iprcb = BrokerFactory::addBrokerType<inproc::InprocBroker>(
    "inproc",
    static_cast<int>(core_type::INPROC));
#endif

#ifdef ENABLE_TEST_CORE
static auto testc =
    CoreFactory::addCoreType<testcore::TestCore>("test", static_cast<int>(core_type::TEST));
static auto testb =
    BrokerFactory::addBrokerType<testcore::TestBroker>("test", static_cast<int>(core_type::TEST));
#endif

bool loadCores()
{
    return true;
}
} // namespace helics
