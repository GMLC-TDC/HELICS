/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "config.h"
#include "core/CommsBroker.hpp"
#include "core/CommsBroker_impl.hpp"

#include "core/ipc/IpcComms.h"
#include "core/zmq/ZmqComms.h"
#include "core/CommonCore.h"
#include "core/CoreBroker.h"

#if HELICS_HAVE_MPI
#include "core/mpi/MpiComms.h"
#endif

namespace helics
{
    template class CommsBroker<IpcComms, CoreBroker>;
    template class CommsBroker<IpcComms, CommonCore>;
    template class CommsBroker<ZmqComms, CoreBroker>;
    template class CommsBroker<ZmqComms, CommonCore>;

#if HELICS_HAVE_MPI
    template class CommsBroker<MpiComms, CoreBroker>;
    template class CommsBroker<MpiComms, CommonCore>;
#endif
}
