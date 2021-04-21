/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "federate_id.hpp"

namespace helics {
/** a shift in the global federate id numbers to allow discrimination between local ids and global
   ones this value allows 131072 federates to be available in each core 1,878,917,120 allowable
   federates in the system and 268,435,455 brokers allowed  if we need more than that this, HELICS
   as a program has been phenomenally successful beyond all wildest imaginations and we can probably
   afford to change these to 64 bit numbers to accommodate.  Of the available federates there is 1
   federate number that can be defined per core for various purposes.  These are the upper number of
   federate id's so 268,435,455 reserved federate id's.  An ID of 1 is reserved for the root broker
   */
constexpr identififier_base_type global_federate_id_shift{0x0002'0000};
/** a shift in the global id index to discriminate between global ids of brokers vs federates*/
constexpr identififier_base_type global_broker_id_shift{0x7000'0000};

/** class holding a globally unique identifier for brokers
    @details the class is fully compatible with global_federate_id*/
class global_broker_id {
  public:
    using base_type = identififier_base_type;
    /** default constructor*/
    constexpr global_broker_id() = default;

    constexpr explicit global_broker_id(base_type val) noexcept: gid(val) {}
    /** extract the base value of the id code*/
    constexpr base_type baseValue() const { return gid; }
    /** equality operator*/
    bool operator==(global_broker_id id) const noexcept { return (gid == id.gid); }
    /** inequality operator*/
    bool operator!=(global_broker_id id) const noexcept { return (gid != id.gid); }
    /** less than operator for sorting*/
    bool operator<(global_broker_id id) const noexcept { return (gid < id.gid); }

    bool isFederate() const
    {
        return ((gid >= global_federate_id_shift) && (gid < global_broker_id_shift));
    }
    bool isBroker() const { return (gid >= global_broker_id_shift || gid == 1); }
    bool isValid() const
    {
        return (gid != invalid_global_broker_id && gid != detail::invalid_interface_handle);
    }
    base_type localIndex() const { return gid - global_broker_id_shift; }

  private:
    base_type gid = invalid_global_broker_id;  //!< the underlying index value
    friend class global_federate_id;  // for allowing comparison operators to work well
    static constexpr base_type invalid_global_broker_id{-2'010'000'000};
};

/** constant to use for indicating that a command is for the core itself from the Core Public API*/
constexpr global_broker_id parent_broker_id{0};
/** constant to use for indicating the id of the root broker*/
constexpr global_broker_id root_broker_id{1};

/** stream operator for a federate_id
 */
std::ostream& operator<<(std::ostream& os, global_broker_id id);
/** class holder a globally unique identifier for federates*/
class global_federate_id {
  public:
    using base_type = identififier_base_type;
    /** default constructor*/
    constexpr global_federate_id() = default;

    constexpr explicit global_federate_id(base_type val) noexcept: gid(val) {}
    /** implicit conversion from global_id*/
    constexpr global_federate_id(global_broker_id id) noexcept: gid(id.gid) {}  // NOLINT

    constexpr operator global_broker_id() const noexcept
    {
        return global_broker_id{gid};
    }  // NOLINT
    /** conversion to the base_type*/
    constexpr base_type baseValue() const { return gid; }
    /** equality operator*/
    bool operator==(global_federate_id id) const noexcept { return (gid == id.gid); }
    /** inequality operator*/
    bool operator!=(global_federate_id id) const noexcept { return (gid != id.gid); }
    /** less than operator for sorting*/
    bool operator<(global_federate_id id) const noexcept { return (gid < id.gid); }
    /** greater than operator for sorting*/
    bool operator>(global_federate_id id) const noexcept { return (gid > id.gid); }
    /** equality operator*/
    bool operator==(global_broker_id id) const noexcept { return (gid == id.gid); }
    /** inequality operator*/
    bool operator!=(global_broker_id id) const noexcept { return (gid != id.gid); }
    /** less than operator for sorting*/
    bool operator<(global_broker_id id) const noexcept { return (gid < id.gid); }
    /** greater than operator for sorting*/
    bool operator>(global_broker_id id) const noexcept { return (gid > id.gid); }
    /** return true if the broker_id is a valid federate id code*/
    bool isFederate() const
    {
        return ((gid >= global_federate_id_shift) && (gid < global_broker_id_shift));
    }
    /** return true if the broker_id is a valid broker id code*/
    bool isBroker() const { return (gid >= global_broker_id_shift || gid == 1); }
    /** return true if the broker_id is a valid broker id code*/
    bool isValid() const
    {
        return (gid != invalid_global_fed_id && gid != detail::invalid_interface_handle);
    }
    /** generate a local offset index
        @details the global_id is shifted by a certain amount*/
    constexpr base_type localIndex() const { return gid - global_federate_id_shift; }

  private:
    static constexpr base_type invalid_global_fed_id{-2'010'000'000};
    base_type gid{invalid_global_fed_id};  //!< the underlying index value
};
/** identifier to target the local core even if the global id hasn't been assigned yet*/
constexpr global_federate_id direct_core_id{-235262};

/** stream operator for a federate_id
 */
std::ostream& operator<<(std::ostream& os, global_federate_id id);

/** class merging a global id and handle together */
class global_handle {
  public:
    global_federate_id fed_id = global_federate_id{};  //!< the federate id component
    interface_handle handle = interface_handle{};  //!< the interface handle component
    /** default constructor*/
    constexpr global_handle() = default;
    /** construct directly from a federate_id and interface_handle*/
    constexpr global_handle(global_federate_id fed, interface_handle hand):
        fed_id(fed), handle(hand)
    {
    }
    /** convert to a uint64_t
        @details for use in maps and other things*/
    explicit operator uint64_t() const
    {
        auto key = static_cast<uint64_t>(fed_id.baseValue()) << 32U;
        key += static_cast<uint64_t>(handle.baseValue()) & (0x0000'0000'FFFF'FFFF);
        return key;
    }
    /** equality operator*/
    bool operator==(global_handle id) const noexcept
    {
        return ((fed_id == id.fed_id) && (handle == id.handle));
    }
    /** inequality operator*/
    bool operator!=(global_handle id) const noexcept
    {
        return ((fed_id != id.fed_id) || (handle != id.handle));
    }
    /** less than operator for sorting*/
    bool operator<(global_handle id) const noexcept
    {
        return (fed_id < id.fed_id) ? true : ((fed_id != id.fed_id) ? false : (handle < id.handle));
    }
    /** check if the handle points to a validly specified interface*/
    bool isValid() const { return fed_id.isValid() && handle.isValid(); }
};

/** stream operator for a federate_id
 */
std::ostream& operator<<(std::ostream& os, global_handle id);

/** class defining a specific type for labeling a route*/
class route_id {
  public:
    using base_type = identififier_base_type;
    /** default constructor*/
    constexpr route_id() = default;
    /** construct from a base_type value*/
    constexpr explicit route_id(base_type val) noexcept: rid(val) {}
    /** explicitly convert to a base value */
    constexpr base_type baseValue() const { return rid; }
    /** equality operator*/
    bool operator==(route_id id) const noexcept { return (rid == id.rid); }
    /** inequality operator*/
    bool operator!=(route_id id) const noexcept { return (rid != id.rid); }
    /** less than operator for sorting*/
    bool operator<(route_id id) const noexcept { return (rid < id.rid); }
    /** check if the route is valid*/
    bool isValid() const { return (rid != invalid_route_id); }

  private:
    static constexpr base_type invalid_route_id{-1'295'148'000};
    base_type rid{invalid_route_id};  //!< the underlying index value
};

constexpr route_id parent_route_id{0};
constexpr route_id control_route{-1};

/** stream operator for a route_id
 */
std::ostream& operator<<(std::ostream& os, route_id id);

}  // namespace helics

namespace std {
/** define a template specialization for hash function for global_federate_id so it can be used in
 * unordered_map*/
template<>
struct hash<helics::global_federate_id> {
    using argument_type = helics::global_federate_id;  //!< typedef for input type of hash
    using result_type = std::size_t;  //!< typedef for output result
    /** hash operator*/
    result_type operator()(argument_type const& key) const noexcept
    {
        return std::hash<helics::global_federate_id::base_type>{}(key.baseValue());
    }
};

/** define a template specialization for hash function for global_broker_id so it can be used in
 * unordered_map*/
template<>
struct hash<helics::global_broker_id> {
    using argument_type = helics::global_broker_id;  //!< typedef for input type of hash
    using result_type = std::size_t;  //!< typedef for output result
    /** hash operator*/
    result_type operator()(argument_type const& key) const noexcept
    {
        return std::hash<helics::global_broker_id::base_type>{}(key.baseValue());
    }
};

/** define a template specialization for hash function for route_id so it can be used in
 * unordered_map*/
template<>
struct hash<helics::route_id> {
    using argument_type = helics::route_id;  //!< typedef for input type of hash
    using result_type = std::size_t;  //!< typedef for output result
    /** hash operator*/
    result_type operator()(argument_type const& key) const noexcept
    {
        return std::hash<helics::route_id::base_type>{}(key.baseValue());
    }
};

/** define a template specialization for hash function for global_handle so it can be used in
unordered_map
@details based on hash of std::uint64_t*/
template<>
struct hash<helics::global_handle> {
    using argument_type = helics::global_handle;  //!< typedef for input type of hash
    using result_type = std::size_t;  //!< typedef for output result
    /** hash operator*/
    result_type operator()(argument_type const& key) const noexcept
    {
        return std::hash<uint64_t>{}(static_cast<uint64_t>(key));
    }
};

}  // namespace std
