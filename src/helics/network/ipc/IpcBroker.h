/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../NetworkBroker.hpp"

namespace helics {
namespace ipc {
    class IpcComms;

    /** implementation for the core that uses IPC messages to communicate*/
    using IpcBroker =
        NetworkBroker<IpcComms, interface_type::ipc, static_cast<int>(core_type::IPC)>;

}  // namespace ipc
}  // namespace helics
