/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "core-types.hpp"

#include <string>

/** @file
@details function definitions for operations on core types
*/

namespace helics {
namespace core {
    /**generate a string based on the core type*/
    std::string to_string(core_type type);

    /** generate a core type value from a std::string
@param type a string describing the desired core type
@return a value of the helics_core_type enumeration
helics::core_type::unrecognized if the type is not valid
*/
    core_type coreTypeFromString(std::string type) noexcept;

    /**
     * Returns true if core/broker type specified is available in current compilation.
     */
    bool isCoreTypeAvailable(core_type type) noexcept;

    /** check if two data types are compatible with each other
@param type1 the first type to match
@param type2 the second type to check
@return true if the types are compatible with each other
*/
    bool matchingTypes(const std::string& type1, const std::string& type2);
}  // namespace core
}  // namespace helics
