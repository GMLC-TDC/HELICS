/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "global_federate_id.hpp"
#include "gmlc/containers/MapTraits.hpp"

/** override the is_easily_hashable type_trait of interface_handle for use in DualMappedVector and
 * some other types that may optionally use std::map or std::unordered_map*/
template<>
struct is_easily_hashable<helics::interface_handle> {
    static constexpr bool value = true;
};

/** override the is_easily_hashable type_trait for use in DualMappedVector and some other types that
 * may optionally use std::map or std::unordered_map*/
template<>
struct is_easily_hashable<helics::global_federate_id> {
    static constexpr bool value = true;
};

/** override the is_easily_hashable type_trait for use in DualMappedVector and some other types that
 * may optionally use std::map or std::unordered_map*/
template<>
struct is_easily_hashable<helics::local_federate_id> {
    static constexpr bool value = true;
};

/** override the is_easily_hashable type_trait for use in DualMappedVector and some other types that
 * may optionally use std::map or std::unordered_map*/
template<>
struct is_easily_hashable<helics::global_broker_id> {
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
struct is_easily_hashable<helics::global_handle> {
    static constexpr bool value = true;
};
