/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "InprocCore.h"

#include "../NetworkCore_impl.hpp"
#include "InprocComms.h"

namespace helics {
template class NetworkCore<inproc::InprocComms, interface_type::inproc>;
}  // namespace helics
