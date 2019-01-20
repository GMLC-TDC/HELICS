/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "UdpBroker.h"
#include "UdpComms.h"
#include "../NetworkBroker_impl.hpp"

namespace helics
{
template class NetworkBroker<udp::UdpComms, interface_type::udp, 7>;
}  // namespace helics
