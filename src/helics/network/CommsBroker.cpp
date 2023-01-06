/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "CommsBroker.hpp"

#include "CommsBroker_impl.hpp"
#include "helics/core/CommonCore.hpp"
#include "helics/core/CoreBroker.hpp"
#include "helics/helics-config.h"

#ifdef HELICS_ENABLE_IPC_CORE
#    include "ipc/IpcComms.h"
#endif

#ifdef HELICS_ENABLE_UDP_CORE
#    include "udp/UdpComms.h"
#endif

#ifdef HELICS_ENABLE_TEST_CORE
#    include "test/TestComms.h"
#endif

#ifdef HELICS_ENABLE_ZMQ_CORE
#    include "zmq/ZmqComms.h"
#    include "zmq/ZmqCommsSS.h"
#endif

#ifdef HELICS_ENABLE_TCP_CORE
#    include "tcp/TcpComms.h"
#    include "tcp/TcpCommsSS.h"
#endif

#ifdef HELICS_ENABLE_INPROC_CORE
#    include "inproc/InprocComms.h"
#endif

#ifdef HELICS_ENABLE_MPI_CORE
#    include "mpi/MpiComms.h"
#endif

namespace helics {
#ifdef HELICS_ENABLE_IPC_CORE
template class CommsBroker<ipc::IpcComms, CoreBroker>;
template class CommsBroker<ipc::IpcComms, CommonCore>;
#endif

#ifdef HELICS_ENABLE_ZMQ_CORE
template class CommsBroker<zeromq::ZmqComms, CoreBroker>;
template class CommsBroker<zeromq::ZmqComms, CommonCore>;
template class CommsBroker<zeromq::ZmqCommsSS, CoreBroker>;
template class CommsBroker<zeromq::ZmqCommsSS, CommonCore>;
#endif

#ifdef HELICS_ENABLE_UDP_CORE
template class CommsBroker<udp::UdpComms, CoreBroker>;
template class CommsBroker<udp::UdpComms, CommonCore>;
#endif

#ifdef HELICS_ENABLE_TEST_CORE
template class CommsBroker<testcore::TestComms, CoreBroker>;
template class CommsBroker<testcore::TestComms, CommonCore>;
#endif

#ifdef HELICS_ENABLE_TCP_CORE
template class CommsBroker<tcp::TcpComms, CommonCore>;
template class CommsBroker<tcp::TcpComms, CoreBroker>;
template class CommsBroker<tcp::TcpCommsSS, CommonCore>;
template class CommsBroker<tcp::TcpCommsSS, CoreBroker>;
#endif

#ifdef HELICS_ENABLE_INPROC_CORE
template class CommsBroker<inproc::InprocComms, CommonCore>;
template class CommsBroker<inproc::InprocComms, CoreBroker>;
#endif

#ifdef HELICS_ENABLE_MPI_CORE
template class CommsBroker<mpi::MpiComms, CoreBroker>;
template class CommsBroker<mpi::MpiComms, CommonCore>;
#endif
}  // namespace helics
