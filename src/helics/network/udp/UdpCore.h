/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../NetworkCore.hpp"

namespace helics {
namespace udp {
    class UdpComms;
    using UdpCore = NetworkCore<UdpComms, interface_type::udp>;

}  // namespace udp
}  // namespace helics
