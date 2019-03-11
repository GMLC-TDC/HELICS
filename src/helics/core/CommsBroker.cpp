/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "CommsBroker.hpp"
#include "CommsBroker_impl.hpp"
#include "helics/helics-config.h"

#include "CommonCore.hpp"
#include "CoreBroker.hpp"

#ifndef DISABLE_IPC_CORE
#include "ipc/IpcComms.h"
#endif

#ifndef DISABLE_UDP_CORE
#include "udp/UdpComms.h"
#endif

#ifndef DISABLE_TEST_CORE
#include "test/TestComms.h"
#endif

#ifdef HELICS_HAVE_ZEROMQ
#include "zmq/ZmqComms.h"
#include "zmq/ZmqCommsSS.h"
#endif

#ifndef DISABLE_TCP_CORE
#include "tcp/TcpComms.h"
#include "tcp/TcpCommsSS.h"
#endif

#if HELICS_HAVE_MPI
#include "mpi/MpiComms.h"
#endif

namespace helics
{
#ifndef DISABLE_IPC_CORE
template class CommsBroker<ipc::IpcComms, CoreBroker>;
template class CommsBroker<ipc::IpcComms, CommonCore>;
#endif

#ifdef HELICS_HAVE_ZEROMQ
template class CommsBroker<zeromq::ZmqComms, CoreBroker>;
template class CommsBroker<zeromq::ZmqComms, CommonCore>;
template class CommsBroker<zeromq::ZmqCommsSS, CoreBroker>;
template class CommsBroker<zeromq::ZmqCommsSS, CommonCore>;
#endif

#ifndef DISABLE_UDP_CORE
template class CommsBroker<udp::UdpComms, CoreBroker>;
template class CommsBroker<udp::UdpComms, CommonCore>;
#endif

#ifndef DISABLE_TEST_CORE
template class CommsBroker<testcore::TestComms, CoreBroker>;
template class CommsBroker<testcore::TestComms, CommonCore>;
#endif

#ifndef DISABLE_TCP_CORE
template class CommsBroker<tcp::TcpComms, CommonCore>;
template class CommsBroker<tcp::TcpComms, CoreBroker>;
template class CommsBroker<tcp::TcpCommsSS, CommonCore>;
template class CommsBroker<tcp::TcpCommsSS, CoreBroker>;
#endif

#if HELICS_HAVE_MPI
template class CommsBroker<mpi::MpiComms, CoreBroker>;
template class CommsBroker<mpi::MpiComms, CommonCore>;
#endif
}  // namespace helics
