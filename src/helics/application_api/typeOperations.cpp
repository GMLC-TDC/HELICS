/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "typeOperations.hpp"

#include "../core/coreTypeOperations.hpp"

namespace helics {
std::string to_string(core_type type)
{
    return core::to_string(type);
}

core_type coreTypeFromString(std::string type) noexcept
{
    return core::coreTypeFromString(type);
}

bool isCoreTypeAvailable(core_type type) noexcept
{
    return core::isCoreTypeAvailable(type);
}
}  // namespace helics
