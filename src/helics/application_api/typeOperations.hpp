/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "../core/core-types.hpp"
#include "helics_cxx_export.h"

#include <string>

/** @file
@details function definitions for operations on core types
*/

namespace helics {
/**generate a string based on the core type*/
HELICS_CXX_EXPORT std::string to_string(core_type type);

/** generate a core type value from a std::string
@param type a string describing the desired core type
@return a value of the helics_core_type enumeration
helics::core_type::unrecognized if the type is not valid
*/
HELICS_CXX_EXPORT core_type coreTypeFromString(std::string type) noexcept;

/**
 * Returns true if core/broker type specified is available in current compilation.
 */
HELICS_CXX_EXPORT bool isCoreTypeAvailable(core_type type) noexcept;
}  // namespace helics
