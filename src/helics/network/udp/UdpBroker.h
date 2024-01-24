/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "../NetworkBroker.hpp"

namespace helics {
namespace udp {
    class UdpComms;
    using UdpBroker = NetworkBroker<UdpComms, gmlc::networking::InterfaceTypes::UDP, 7>;

}  // namespace udp
}  // namespace helics
