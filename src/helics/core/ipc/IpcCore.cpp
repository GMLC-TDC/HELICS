/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "IpcCore.h"
#include "IpcComms.h"
#include "../NetworkCore_impl.hpp"

namespace helics
{
template class NetworkCore<ipc::IpcComms, interface_type::ipc>;
}  // namespace helics
