/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. 
SPDX-License-Identifier: BSD-3-Clause
*/

#include "UdpCore.h"
#include "UdpComms.h"
#include "../NetworkCore_impl.hpp"

namespace helics
{
    template class NetworkCore<udp::UdpComms, interface_type::udp>;
}  // namespace helics
