/*
Copyright (c) 2017-2025,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "../core/CoreTypes.hpp"
#include "helics_cxx_export.h"

#include <string>

/** @file
@details function definitions for operations on core types
*/

namespace helics {
/**generate a string based on the core type*/
HELICS_CXX_EXPORT std::string to_string(CoreType type);

/** generate a core type value from a std::string
@param type a string describing the desired core type
@return a value of the helics::CoreType enumeration
helics::CoreType::unrecognized if the type is not valid
*/
HELICS_CXX_EXPORT CoreType coreTypeFromString(std::string_view type) noexcept;

/**
 * Returns true if core/broker type specified is available in current compilation.
 */
HELICS_CXX_EXPORT bool isCoreTypeAvailable(CoreType type) noexcept;

/** get a string with the system info*/
HELICS_CXX_EXPORT std::string systemInfo();

}  // namespace helics
