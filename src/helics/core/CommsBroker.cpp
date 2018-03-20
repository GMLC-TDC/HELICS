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
#include "zmq/ZmqComms.h"

#ifndef DISABLE_TCP_CORE
#include "tcp/TcpComms.h"
#endif

#if HELICS_HAVE_MPI
#include "mpi/MpiComms.h"
#endif

namespace helics
{
template class CommsBroker<ipc::IpcComms, CoreBroker>;
template class CommsBroker<ipc::IpcComms, CommonCore>;
template class CommsBroker<zeromq::ZmqComms, CoreBroker>;
template class CommsBroker<zeromq::ZmqComms, CommonCore>;
template class CommsBroker<udp::UdpComms, CoreBroker>;
template class CommsBroker<udp::UdpComms, CommonCore>;

#ifndef DISABLE_TCP_CORE
template class CommsBroker<tcp::TcpComms, CommonCore>;
template class CommsBroker<tcp::TcpComms, CoreBroker>;
#endif

#if HELICS_HAVE_MPI
template class CommsBroker<mpi::MpiComms, CoreBroker>;
template class CommsBroker<mpi::MpiComms, CommonCore>;
#endif
}  // namespace helics

