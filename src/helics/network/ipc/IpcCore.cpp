/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "IpcCore.h"

#include "../NetworkCore_impl.hpp"
#include "IpcComms.h"

namespace helics {
template class NetworkCore<ipc::IpcComms, InterfaceTypes::IPC>;
}  // namespace helics
