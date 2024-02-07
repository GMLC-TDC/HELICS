/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "GlobalFederateId.hpp"
#include "gmlc/containers/MapTraits.hpp"

/** override the is_easily_hashable type_trait of InterfaceHandle for use in DualMappedVector and
 * some other types that may optionally use std::map or std::unordered_map*/
template<>
struct is_easily_hashable<helics::InterfaceHandle> {
    static constexpr bool value = true;
};

/** override the is_easily_hashable type_trait for use in DualMappedVector and some other types that
 * may optionally use std::map or std::unordered_map*/
template<>
struct is_easily_hashable<helics::GlobalFederateId> {
    static constexpr bool value = true;
};

/** override the is_easily_hashable type_trait for use in DualMappedVector and some other types that
 * may optionally use std::map or std::unordered_map*/
template<>
struct is_easily_hashable<helics::LocalFederateId> {
    static constexpr bool value = true;
};

/** override the is_easily_hashable type_trait for use in DualMappedVector and some other types that
 * may optionally use std::map or std::unordered_map*/
template<>
struct is_easily_hashable<helics::GlobalBrokerId> {
    static constexpr bool value = true;
};

/** override the is_easily_hashable type_trait for use in DualMappedVector and some other types that
 * may optionally use std::map or std::unordered_map*/
template<>
struct is_easily_hashable<helics::route_id> {
    static constexpr bool value = true;
};

/** override the is_easily_hashable type_trait for use in DualMappedVector and some other types that
 * may optionally use std::map or std::unordered_map*/
template<>
struct is_easily_hashable<helics::GlobalHandle> {
    static constexpr bool value = true;
};
