/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. 
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "../NetworkBroker.hpp"

namespace helics
{
namespace udp
{
class UdpComms;
using UdpBroker = NetworkBroker<UdpComms, interface_type::udp, 7>;

}  // namespace udp
}  // namespace helics

