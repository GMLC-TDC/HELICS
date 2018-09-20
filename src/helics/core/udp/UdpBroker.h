/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#pragma once

#include "../NetworkBroker.hpp"

namespace helics
{
namespace udp
{
class UdpComms;
using UdpBroker = NetworkBroker<UdpComms, NetworkBrokerData::interface_type::udp, 7>;

}  // namespace udp
}  // namespace helics

