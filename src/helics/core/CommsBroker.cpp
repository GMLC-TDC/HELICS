/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "CommsBroker.hpp"
#include "CommsBroker_impl.hpp"
#include "helics/helics-config.h"

#include "CommonCore.h"
#include "CoreBroker.h"
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
template class CommsBroker<IpcComms, CoreBroker>;
template class CommsBroker<IpcComms, CommonCore>;
template class CommsBroker<ZmqComms, CoreBroker>;
template class CommsBroker<ZmqComms, CommonCore>;
template class CommsBroker<UdpComms, CoreBroker>;
template class CommsBroker<UdpComms, CommonCore>;

#ifndef DISABLE_TCP_CORE
template class CommsBroker<TcpComms, CommonCore>;
template class CommsBroker<TcpComms, CoreBroker>;
#endif

#if HELICS_HAVE_MPI
template class CommsBroker<MpiComms, CoreBroker>;
template class CommsBroker<MpiComms, CommonCore>;
#endif
}  // namespace helics
