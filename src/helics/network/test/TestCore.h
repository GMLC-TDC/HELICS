/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../NetworkCore.hpp"

namespace helics {
namespace testcore {
    class TestComms;
    /** implementation for the core that can only communicate in process*/
    using TestCore = NetworkCore<TestComms, interface_type::inproc>;

}  // namespace testcore
}  // namespace helics
