/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. 
SPDX-License-Identifier: BSD-3-Clause
*/

#include "IpcCore.h"
#include "IpcComms.h"
#include "../NetworkCore_impl.hpp"

namespace helics
{
template class NetworkCore<ipc::IpcComms, interface_type::ipc>;
}  // namespace helics
