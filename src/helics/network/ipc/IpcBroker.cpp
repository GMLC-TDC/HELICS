/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "IpcBroker.h"

#include "../NetworkBroker_impl.hpp"
#include "IpcComms.h"

namespace helics {
template class NetworkBroker<ipc::IpcComms,
                             gmlc::networking::InterfaceTypes::IPC,
                             static_cast<int>(CoreType::IPC)>;
}  // namespace helics
