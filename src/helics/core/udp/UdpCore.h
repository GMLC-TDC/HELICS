/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "../NetworkCore.hpp"

namespace helics
{
namespace udp {
class UdpComms;
using UdpCore = NetworkCore<UdpComms, interface_type::udp>;

} // namespace udp
}  // namespace helics

