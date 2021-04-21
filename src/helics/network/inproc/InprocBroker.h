/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "../NetworkBroker.hpp"

namespace helics {
namespace inproc {
    class InprocComms;

    /** implementation for the core that uses IPC messages to communicate*/
    using InprocBroker =
        NetworkBroker<InprocComms, interface_type::inproc, static_cast<int>(core_type::INPROC)>;

}  // namespace inproc
}  // namespace helics
