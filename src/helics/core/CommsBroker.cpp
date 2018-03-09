/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
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

