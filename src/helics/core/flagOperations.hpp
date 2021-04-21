/*
Copyright (c) 2017-2021,
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

/** flag definitions for the action Message Flag field*/
enum operation_flags : uint16_t {
    iteration_requested_flag = 0,  //!< indicator that an iteration has been requested
    destination_target = 1,  //!< indicator that the target is a destination target
    required_flag = 2,  //!< flag indicating that an action or match is required
    core_flag = 3,  //!< flag indicating that message comes from a core vs a broker
    error_flag = 4,  //!< flag indicating an error condition associated with the command
    indicator_flag = 5,  //!< flag used for setting values
    empty_flag = 6,  //!< flag indicating that the message is empty
    extra_flag1 = 7,  //!< extra flag
    optional_flag = 8,  //!< flag indicating that a connection is optional and may not be matched
    clone_flag =
        9,  //!< flag indicating the filter is a clone filter or the data needs to be cloned
    extra_flag2 = 10,  //!< extra flag
    destination_processing_flag =
        11,  //!< flag indicating the message is for destination processing
    disconnected_flag = 12,  //!< flag indicating that a broker/federate is disconnected
    extra_flag3 = 13,  //!< extra flag
    extra_flag4 = 14,  //!< extra flag
    nameless_interface_flag = 15,  //!< flag indicating the interface is nameless
};

/// overload of extra_flag4 indicating a federate, core or broker is slow responding
constexpr uint16_t slow_responding_flag = extra_flag4;

/// overload of extra_flag3 indicating an operation is canceled
constexpr uint16_t cancel_flag = extra_flag3;

/// overload of extra_flag3 indicating the message is from a parent object
constexpr uint16_t parent_flag = extra_flag3;

/// overload of extra_flag4 indicating a message is from a child object
constexpr uint16_t child_flag = extra_flag4;

/// overload of nameless_interface_flag indicating that a federate should not count in any totals
constexpr uint16_t non_counting_flag = nameless_interface_flag;

/// overload of extra_flag1 to indicate the request is from a non-granting federate
constexpr uint16_t non_granting_flag = extra_flag1;

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
