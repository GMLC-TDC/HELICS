/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "typeOperations.hpp"

#include "../core/coreTypeOperations.hpp"

namespace helics {
std::string to_string(CoreType type)
{
    return core::to_string(type);
}

CoreType coreTypeFromString(std::string_view type) noexcept
{
    return core::coreTypeFromString(type);
}

bool isCoreTypeAvailable(CoreType type) noexcept
{
    return core::isCoreTypeAvailable(type);
}
}  // namespace helics
