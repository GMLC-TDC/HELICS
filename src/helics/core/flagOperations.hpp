/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
/**
@file
operations and helper functions for handling flags in helics
*/
#include <cstdint>

namespace helics {
/** flag definitions for the action Message Flag field*/
enum GeneralFlags : uint16_t {
    error_flag = 4,  //!< flag indicating an error condition associated with the command
    indicator_flag = 5,  //!< flag used for setting values
};

/// @brief  flags used in timing messages
enum TimingFlags : uint16_t {
    iteration_requested_flag = 0,  //!< indicator that an iteration has been requested
    /// flag to indicate the request is from a non-granting federate
    non_granting_flag = 7,

    /// flag to mark an interrupted event
    interrupted_flag = 8,
    /// flag to indicate the request is from federate with delayed timing
    delayed_timing_flag = 10,
    /// flag indicating the message is from a parent object
    parent_flag = 13,

    /// flag indicating a message is from a child object
    child_flag = 14
};

/// @brief flags used on handles
enum InterfaceFlags : uint16_t {
    /// indicator that the interface should buffer data
    buffer_data_flag = 0,
    /// indicator that the target is a destination target
    destination_target = 1,
    /// flag indicating that an action or match is required
    required_flag = 2,
    /// flag indicating that the interface accepts only a single connection
    single_connection_flag = 3,
    /// flag indicating that the values should only be updated on change
    only_update_on_change_flag = 6,
    /// flag indicating that the target is mapped
    reconnectable_flag = 7,
    /// flag indicating that a connection is optional and may not be matched
    optional_flag = 8,

    // 9,10,11,13,14 are interface specific

    /// indicator that the interface should only transmit on change
    only_transmit_on_change_flag = 12,

    /// flag to indicate an interface is nameless
    nameless_interface_flag = 15
};

/// @brief enumeration of endpoint specific flags
enum EndpointFlags {
    /// flag indicating an endpoint is targeted
    targeted_flag = 10,

    /// indicator that an endpoint or message has a source filter
    has_source_filter_flag = 11,
    /// indicator that an endpoint is source only
    source_only_flag = 13,
    /// indicator that an endpoint is receive only
    receive_only_flag = 14
};

/// @brief enumeration of filter specific flags
enum FilterFlags {
    /// flag indicating the filter is a clone filter or the data needs to be cloned
    clone_flag = 9,
    /// indicator that an endpoint or message has a destination filter
    has_dest_filter_flag = 13,
    /// indicator that the endpoint or filter has a destination filter that alters the message
    has_non_cloning_dest_filter_flag = 14,
};

/// @brief flags used when connecting a federate/core/broker to a federation
enum ConnectionFlags : uint16_t {
    /// flag indicating that message comes from a core vs a broker
    core_flag = 3,
    /// flag indicating to use global timing (overload of indicator flag)
    global_timing_flag = 5,

    /// flag to indicate it should use the json packetization
    use_json_serialization_flag = 6,
    /// flag indicating use of asynchronous timing on a global level
    async_timing_flag = 7,
    /// flag indicating that the connection is an observer only
    observer_flag = 8,
    /// flag indicating that the connection allows joining dynamically
    dynamic_join_flag = 9,
    /// flag indicating that the connection may be reentrant
    reentrant_flag = 10,
    /// flag indicating that global disconnect synchronize is in use
    global_disconnect_flag = 11,
    /// flag indicating that a broker/federate is disconnected
    disconnected_flag = 12,
    /// flag indicating this is a test connection
    test_connection_flag = 13,
    /// flag indicating a federate, core or broker is slow responding
    slow_responding_flag = 14,
    /// flag indicating that a federate should not count in any totals
    non_counting_flag = 15

};

/// @brief flags used for messages
enum MessageFlags : uint16_t {
    /// flag indicating that the message requires processing for filters yet
    filter_processing_required_flag = 7,
    /// custom message flag 1
    user_custom_message_flag1 = 10,
    /// flag indicating the message is for destination processing
    destination_processing_flag = 11,
    /// flag indicating the message is empty
    /// custom message flag 2
    user_custom_message_flag2 = 13,
    /// custom message flag 3
    user_custom_message_flag3 = 14,
    empty_flag = 15,
};

/// @brief general flags used for other operations
enum OperationFlags : uint16_t {
    /// flag indicating an operation is canceled
    cancel_flag = 13
};

// extra_flag1 = 7,  //!< extra flag

// extra_flag2 = 10,  //!< extra flag

// extra_flag3 = 13,  //!< extra flag
// extra_flag4 = 14,  //!< extra flag

/** template function to set a flag in an object containing a flags field
@tparam FlagContainer an object with a .flags field
@tparam FlagIndex a type that can be used as part of a shift to index into a flag object
@param M the container to set the flag in
@param flag the flag to set
*/
template<class FlagContainer, class FlagIndex>
inline void setActionFlag(FlagContainer& M, FlagIndex flag)
{
    M.flags |= (static_cast<decltype(M.flags)>((1U) << (static_cast<uint16_t>((flag)))));
}

/** check a flag value on a specified index*/
/** template function to check a flag in an object containing a flags field*/
template<class FlagIndex>
inline bool checkActionFlag(uint16_t flags, FlagIndex flag)
{
    return ((flags & (static_cast<uint16_t>((1U) << (static_cast<uint16_t>((flag)))))) != 0U);
}

/** template function to check a flag in an object containing a flags field*/
template<class FlagContainer, class FlagIndex>
inline bool checkActionFlag(const FlagContainer& M, FlagIndex flag)
{
    return ((M.flags & (static_cast<decltype(M.flags)>((1U) << (static_cast<uint16_t>((flag)))))) !=
            0U);
}

/** template function to clear a flag in an object containing a flags field*/
template<class FlagContainer, class FlagIndex>
inline void clearActionFlag(FlagContainer& M, FlagIndex flag)
{
    M.flags &= static_cast<decltype(M.flags)>(~((1U) << (static_cast<uint16_t>((flag)))));
}

/** template function to clear a flag in an object containing a flags field*/
template<class FlagContainer, class FlagIndex>
inline void toggleActionFlag(FlagContainer& M, FlagIndex flag)
{
    if (checkActionFlag(M, flag)) {
        clearActionFlag(M, flag);
    } else {
        setActionFlag(M, flag);
    }
}

/** helper function to facilitate make a flag variable*/
inline constexpr uint16_t make_flags(unsigned int flag)
{
    return static_cast<uint16_t>(1U << (flag));
}

/** helper function to facilitate make a flag variable out of two flags*/
inline constexpr uint16_t make_flags(unsigned int flag1, unsigned int flag2)
{
    return make_flags(flag1) | make_flags(flag2);
}

/** helper function to facilitate make a flag variable out of three flags*/
inline constexpr uint16_t make_flags(unsigned int flag1, unsigned int flag2, unsigned int flag3)
{
    return make_flags(flag1, flag2) | make_flags(flag3);
}

/** helper function to toggle a bit in a uint16_t value*/
inline constexpr uint16_t toggle_flag(uint16_t base, unsigned int flag)
{
    return base ^ make_flags(flag);
}
}  // namespace helics
