/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. 
SPDX-License-Identifier: BSD-3-Clause
*/
#include "IpcBroker.h"
#include "IpcComms.h"
#include "../NetworkBroker_impl.hpp"

namespace helics
{
template class NetworkBroker<ipc::IpcComms, interface_type::ipc, static_cast<int>(core_type::IPC)>;
} //namespace helics
