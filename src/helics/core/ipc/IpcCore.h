/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "../NetworkCore.hpp"

namespace helics
{
namespace ipc
{
class IpcComms;
class IpcCommsSS;
/** implementation for the core that uses tcp messages to communicate*/
using IpcCore = NetworkCore<IpcComms, interface_type::ipc>;

}  // namespace ipc
}  // namespace helics
