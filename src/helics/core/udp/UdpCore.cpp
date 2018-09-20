/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "UdpCore.h"
#include "UdpComms.h"
#include "../NetworkCore_impl.hpp"

namespace helics
{
    template class NetworkCore<udp::UdpComms, NetworkBrokerData::interface_type::udp>;
}  // namespace helics
