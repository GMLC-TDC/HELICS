/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "CommsBroker.hpp"
#include "CommsBroker_impl.hpp"
#include "helics/helics-config.h"

#include "CommonCore.hpp"
#include "CoreBroker.hpp"
#include "ipc/IpcComms.h"
#include "udp/UdpComms.h"
#include "test/TestComms.h"

#ifdef HELICS_HAVE_ZEROMQ
#include "zmq/ZmqComms.h"
#endif

#ifndef DISABLE_TCP_CORE
#include "tcp/TcpCommsSS.h"
#include "tcp/TcpComms.h"
#endif

#if HELICS_HAVE_MPI
#include "mpi/MpiComms.h"
#endif

namespace helics
{
template class CommsBroker<ipc::IpcComms, CoreBroker>;
template class CommsBroker<ipc::IpcComms, CommonCore>;
#ifdef HELICS_HAVE_ZEROMQ
template class CommsBroker<zeromq::ZmqComms, CoreBroker>;
template class CommsBroker<zeromq::ZmqComms, CommonCore>;
#endif
template class CommsBroker<udp::UdpComms, CoreBroker>;
template class CommsBroker<udp::UdpComms, CommonCore>;

template class CommsBroker<testcore::TestComms, CoreBroker>;
template class CommsBroker<testcore::TestComms, CommonCore>;

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
