/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "CoreTypes.hpp"

#include <string>
#include <string_view>

/** @file
@details function definitions for operations on core types
*/

namespace helics::core {

/**generate a string based on the core type*/
std::string to_string(CoreType type);
/** generate a core type value from a std::string
@param type a string describing the desired core type
@return a value of the helics::CoreType enumeration
helics::CoreType::UNRECOGNIZED if the type is not valid
*/
CoreType coreTypeFromString(std::string_view type) noexcept;

/**
 * Returns true if core/broker type specified is available in current compilation.
 */
bool isCoreTypeAvailable(CoreType type) noexcept;

/** check if two data types are compatible with each other
@param type1 the first type to match
@param type2 the second type to check
@return true if the types are compatible with each other
*/
bool matchingTypes(std::string_view type1, std::string_view type2);
}  // namespace helics::core
