/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "../NetworkBroker.hpp"
#include "IpcComms.h"

namespace helics
{
namespace ipc
{
class IpcComms;

/** implementation for the core that uses IPC messages to communicate*/
using IpcBroker = NetworkBroker<IpcComms, interface_type::ipc, static_cast<int>(core_type::IPC)>;

} // namespace ipc
}  // namespace helics

