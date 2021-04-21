/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "InprocBroker.h"

#include "../NetworkBroker_impl.hpp"
#include "InprocComms.h"

namespace helics {
template class NetworkBroker<inproc::InprocComms,
                             interface_type::inproc,
                             static_cast<int>(core_type::INPROC)>;
}  // namespace helics
