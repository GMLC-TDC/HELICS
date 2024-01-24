/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../NetworkCore.hpp"

namespace helics {
namespace inproc {
    class InprocComms;
    /** implementation for the core that can only communicate in process*/
    using InprocCore = NetworkCore<InprocComms, InterfaceTypes::INPROC>;

}  // namespace inproc
}  // namespace helics
