/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "IpcBroker.h"
#include "IpcComms.h"
#include "../NetworkBroker_impl.hpp"

namespace helics
{
template class NetworkBroker<ipc::IpcComms, interface_type::ipc, static_cast<int>(core_type::IPC)>;
} //namespace helics
